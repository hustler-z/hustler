/**
 * Hustler's Project
 *
 * File:  smp.c
 * Date:  2024/06/26
 * Usage:
 */

#include <asm/sysregs.h>
#include <asm/setup.h>
#include <asm/barrier.h>
#include <asm/atomic.h>
#include <asm/map.h>
#include <org/smp.h>
#include <org/section.h>
#include <org/globl.h>
#include <org/cache.h>
#include <org/psci.h>
#include <org/cpu.h>
#include <org/vcpu.h>
#include <bsp/errno.h>
#include <bsp/softirq.h>
#include <bsp/tasklet.h>
#include <bsp/time.h>
#include <bsp/rwlock.h>
#include <bsp/debug.h>
#include <bsp/delay.h>
#include <bsp/panic.h>
#include <bsp/core.h>
#include <bsp/vmap.h>
#include <lib/math.h>

// --------------------------------------------------------------
unsigned int __read_mostly nr_cpu_ids = NR_CPUS;
unsigned long __section(".boot.ttbl") smpboot_cpu = MPIDR_INVALID;
struct arm_cpu cpu_data[NR_CPUS];

extern struct boot_setup boot_setup;
extern struct vcpu *hypos_vcpus[NR_CPUS];

static bool cpu_is_dead;

const cpumask_t cpumask_all = {
    .bits[0 ... (BITS_TO_LONGS(NR_CPUS) - 1)] = ~0UL
};

struct smp_enable_ops {
    int (*prepare_cpu)(int cpu);
};

static struct call_data_struct {
    void (*func) (void *info);
    void *info;
    int wait;
    cpumask_t selected;
} call_data;

enum halt_state {
    HALT_START,
    HALT_PREPARE,
    HALT_DISABLE_IRQ,
    HALT_INVOKE,
    HALT_EXIT
};

struct halt_data {
    unsigned int nr_cpus;

    enum halt_state state;
    atomic_t done;

    unsigned int fn_cpu;
    int fn_result;
    int (*fn)(void *data);
    void *fn_data;
};

cpumask_t cpu_online_map;
cpumask_t cpu_present_map;
cpumask_t cpu_possible_map;

register_t __cpu_logical_map[NR_CPUS] =
                        {[0 ... NR_CPUS-1] = MPIDR_INVALID};

#define MASK_DECLARE_1(x) [(x) + 1][0] = 1UL << (x)
#define MASK_DECLARE_2(x) MASK_DECLARE_1(x), MASK_DECLARE_1((x) + 1)
#define MASK_DECLARE_4(x) MASK_DECLARE_2(x), MASK_DECLARE_2((x) + 2)
#define MASK_DECLARE_8(x) MASK_DECLARE_4(x), MASK_DECLARE_4((x) + 4)

const unsigned long
cpu_bit_bitmap[BITS_PER_LONG + 1][BITS_TO_LONGS(NR_CPUS)] = {

    MASK_DECLARE_8(0),  MASK_DECLARE_8(8),
    MASK_DECLARE_8(16), MASK_DECLARE_8(24),
#if BITS_PER_LONG > 32
    MASK_DECLARE_8(32), MASK_DECLARE_8(40),
    MASK_DECLARE_8(48), MASK_DECLARE_8(56),
#endif
};

DEFINE_PERCPU_RO(cpumask_var_t, cpu_sibling_mask);
DEFINE_PERCPU_RO(cpumask_var_t, cpu_core_mask);

static paddr_t cpu_release_addr[NR_CPUS];
static struct smp_enable_ops smp_enable_ops[NR_CPUS];

static DEFINE_RWLOCK(cpu_add_remove_lock);
static DEFINE_SPINLOCK(call_lock);

static DEFINE_PERCPU(struct tasklet, halt_tasklet);
static struct halt_data halt_data;
static DEFINE_SPINLOCK(halt_lock);

// --------------------------------------------------------------

static void halt_set_state(enum halt_state state)
{
    atomic_set(&halt_data.done, 0);
    smp_wmb();
    halt_data.state = state;
}

static void halt_wait_state(void )
{
    while (atomic_read(&halt_data.done) != halt_data.nr_cpus)
        cpu_relax();
}

int halt_run(int (*fn)(void *data), void *data, unsigned int cpu)
{
    unsigned int i, nr_cpus;
    unsigned int this = smp_processor_id();
    int ret;

    BUG_ON(!local_irq_is_enabled());
    BUG_ON(!is_idle_vcpu(current));

    /* cpu_online_map must not change. */
    if (!get_cpu_maps())
        return -EBUSY;

    nr_cpus = num_online_cpus();
    if (cpu_online(this))
        nr_cpus--;

    /* Must not spin here as the holder will expect
     * us to be descheduled. */
    if (!spin_trylock(&halt_lock)) {
        put_cpu_maps();
        return -EBUSY;
    }

    halt_data.fn = fn;
    halt_data.fn_data = data;
    halt_data.nr_cpus = nr_cpus;
    halt_data.fn_cpu = cpu;
    halt_data.fn_result = 0;
    atomic_set(&halt_data.done, 0);
    halt_data.state = HALT_START;

    smp_wmb();

    for_each_online_cpu(i)
        if (i != this)
            tasklet_schedule_on_cpu(&percpu(halt_tasklet, i), i);

    halt_set_state(HALT_PREPARE);
    halt_wait_state();

    local_irq_disable();
    halt_set_state(HALT_DISABLE_IRQ);
    halt_wait_state();

    halt_set_state(HALT_INVOKE);
    if ((cpu == this) || (cpu == NR_CPUS)) {
        ret = (*fn)(data);
        if (ret)
            write_atomic(&halt_data.fn_result, ret);
    }
    halt_wait_state();
    ret = halt_data.fn_result;

    halt_set_state(HALT_EXIT);
    halt_wait_state();
    local_irq_enable();

    spin_unlock(&halt_lock);

    put_cpu_maps();

    return ret;
}

static void halt_action(void *data)
{
    unsigned int cpu = (unsigned long)data;
    enum halt_state state = HALT_START;

    BUG_ON(cpu != smp_processor_id());

    smp_mb();

    while (state != HALT_EXIT) {
        while (halt_data.state == state)
            cpu_relax();

        state = halt_data.state;
        switch (state) {
        case HALT_DISABLE_IRQ:
            local_irq_disable();
            break;
        case HALT_INVOKE:
            if ((halt_data.fn_cpu == smp_processor_id()) ||
                (halt_data.fn_cpu == NR_CPUS)) {
                int ret = halt_data.fn(halt_data.fn_data);

                if (ret)
                    write_atomic(&halt_data.fn_result, ret);
            }
            break;
        default:
            break;
        }

        smp_mb();
        atomic_inc(&halt_data.done);
    }

    local_irq_enable();
}

static int cpu_callback(
    struct notifier_block *nfb, unsigned long action, void *hcpu)
{
    unsigned int cpu = (unsigned long)hcpu;

    if (action == CPU_UP_PREPARE)
        tasklet_init(&percpu(halt_tasklet, cpu),
                     halt_action, hcpu);

    return NOTIFY_DONE;
}

static struct notifier_block cpu_nfb = {
    .notifier_call = cpu_callback
};

static int __bootfunc cpu_halt_init(void)
{
    unsigned int cpu;
    for_each_online_cpu (cpu) {
        void *hcpu = (void *)(long)cpu;
        cpu_callback(&cpu_nfb, CPU_UP_PREPARE, hcpu);
    }
    register_cpu_notifier(&cpu_nfb);

    return 0;
}
__bootcall(cpu_halt_init);

// --------------------------------------------------------------

void smp_send_event_check_mask(const cpumask_t *mask)
{
    send_sgi_mask(mask, GIC_SGI_EVENT_CHECK);
}

void smp_send_call_function_mask(const cpumask_t *mask)
{
    cpumask_t target_mask;

    cpumask_andnot(&target_mask, mask,
                        cpumask_of(smp_processor_id()));

    send_sgi_mask(&target_mask, GIC_SGI_CALL_FUNCTION);

    if (cpumask_test_cpu(smp_processor_id(), mask)) {
        local_irq_disable();
        smp_call_function_interrupt();
        local_irq_enable();
    }
}

void on_selected_cpus(
    const cpumask_t *selected,
    void (*func) (void *info),
    void *info,
    int wait)
{
    ASSERT(local_irq_is_enabled());
    ASSERT(cpumask_subset(selected, &cpu_online_map));

    spin_lock(&call_lock);

    cpumask_copy(&call_data.selected, selected);

    if (cpumask_empty(&call_data.selected))
        goto out;

    call_data.func = func;
    call_data.info = info;
    call_data.wait = wait;

    smp_send_call_function_mask(&call_data.selected);

    while (!cpumask_empty(&call_data.selected))
        cpu_relax();

out:
    spin_unlock(&call_lock);
}

void smp_call_function(
    void (*func) (void *info),
    void *info,
    int wait)
{
    cpumask_t allbutself;

    cpumask_andnot(&allbutself, &cpu_online_map,
                   cpumask_of(smp_processor_id()));
    on_selected_cpus(&allbutself, func, info, wait);
}

void smp_call_function_interrupt(void)
{
    void (*func)(void *info) = call_data.func;
    void *info = call_data.info;
    unsigned int cpu = smp_processor_id();

    if (!cpumask_test_cpu(cpu, &call_data.selected))
        return;

    irq_enter();

    if (unlikely(!func)) {
        cpumask_clear_cpu(cpu, &call_data.selected);
    } else if (call_data.wait) {
        (*func)(info);
        smp_mb();
        cpumask_clear_cpu(cpu, &call_data.selected);
    } else {
        smp_mb();
        cpumask_clear_cpu(cpu, &call_data.selected);
        (*func)(info);
    }

    irq_exit();
}

// --------------------------------------------------------------

static int __bootfunc smp_spin_table_cpu_up(int cpu)
{
    paddr_t __iomem *release;

    if (!cpu_release_addr[cpu]) {
        MSGE("CPU%d: No release addr\n", cpu);
        return -ENODEV;
    }

    release = ioremap_nocache(cpu_release_addr[cpu], 8);
    if (!release) {
        MSGE("CPU%d: Unable to map release address\n", cpu);
        return -EFAULT;
    }

    writeq(__pa(smpboot_setup), release);

    iounmap(release);

    sev();

    return 0;
}

// --------------------------------------------------------------

bool get_cpu_maps(void)
{
    return read_trylock(&cpu_add_remove_lock);
}

void put_cpu_maps(void)
{
    read_unlock(&cpu_add_remove_lock);
}

void cpu_hotplug_begin(void)
{
    rcu_barrier();
    write_lock(&cpu_add_remove_lock);
}

void cpu_hotplug_done(void)
{
    write_unlock(&cpu_add_remove_lock);
}

static void set_smp_cpu_up(unsigned long mpidr)
{

    void *ptr = map_hypos_page(va_to_pfn(&smpboot_cpu));

    ptr += PAGE_OFFSET(&smpboot_cpu);

    *(unsigned long *)ptr = mpidr;

    clean_dcache_va_range(ptr, sizeof(unsigned long));

    unmap_hypos_page(ptr);
}

static NOTIFIER_HEAD(cpu_chain);

void __bootfunc register_cpu_notifier(struct notifier_block *nb)
{
    write_lock(&cpu_add_remove_lock);
    notifier_chain_register(&cpu_chain, nb);
    write_unlock(&cpu_add_remove_lock);
}

static int cpu_notifier_call_chain(unsigned int cpu,
                                   unsigned long action,
                                   struct notifier_block **nb,
                                   bool nofail)
{
    void *hcpu = (void *)(long)cpu;
    int notifier_rc = notifier_call_chain(&cpu_chain, action,
                                          hcpu, nb);
    int ret =  notifier_to_errno(notifier_rc);

    BUG_ON(ret && nofail);

    return ret;
}

void stop_cpu(void)
{
    local_irq_disable();
    cpu_is_dead = true;

    dsb(sy);
    isb();
    call_psci_cpu_off();

    while ( 1 )
        wfi();
}

int arch_cpu_up(int cpu)
{
    int rc;

    if (!smp_enable_ops[cpu].prepare_cpu)
        return -ENODEV;

    update_idmap(1);

    rc = smp_enable_ops[cpu].prepare_cpu(cpu);
    if (rc)
        update_idmap(0);

    return rc;
}

void arch_cpu_up_finish(void)
{
    update_idmap(0);
}

/* Bring up a remote CPU */
int __cpu_up(unsigned int cpu)
{
    int rc;
    stime_t deadline;

    MSGH("Bringing up CPU%d\n", cpu);

    rc = prep_smp_mm(cpu);
    if (rc < 0)
        return rc;

    boot_setup.stack = hypos_vcpus[cpu]->arch.stack;
    boot_setup.cpuid = cpu;

    /* Open the gate for this CPU */
    set_smp_cpu_up(cpu_logical_map(cpu));

    rc = arch_cpu_up(cpu);

    if (rc < 0) {
        MSGE("Failed to bring up CPU%d\n", cpu);
        return rc;
    }

    deadline = NOW() + MILLISECS(1000);

    while (!cpu_online(cpu) && NOW() < deadline) {
        cpu_relax();
        process_pending_softirqs();
    }

    smp_rmb();

    boot_setup.stack = NULL;
    boot_setup.cpuid = ~0;

    set_smp_cpu_up(MPIDR_INVALID);

    arch_cpu_up_finish();

    if (!cpu_online(cpu)) {
        MSGE("CPU%d never came online\n", cpu);
        return -EIO;
    }

    return 0;
}

void __cpu_disable(void)
{
    unsigned int cpu = smp_processor_id();

    local_irq_disable();
    gic_disable_cpu();

    local_irq_enable();
    mdelay(1);
    local_irq_disable();

    cpumask_clear_cpu(cpu, &cpu_online_map);

    smp_mb();
}

static void _take_cpu_down(void *unused)
{
    cpu_notifier_call_chain(smp_processor_id(),
                            CPU_DYING, NULL, true);
    __cpu_disable();
}

static int take_cpu_down(void *arg)
{
    _take_cpu_down(arg);
    return 0;
}

/* Wait for a remote CPU to die */
void __cpu_die(unsigned int cpu)
{
    unsigned int i = 0;

    while (!cpu_is_dead) {
        mdelay(100);
        cpu_relax();
        process_pending_softirqs();
        if ((++i % 10) == 0)
            MSGE("CPU %u still not dead...\n", cpu);
        smp_mb();
    }

    cpu_is_dead = false;
    smp_mb();
}

int cpu_down(unsigned int cpu)
{
    int err;
    struct notifier_block *nb = NULL;

    cpu_hotplug_begin();

    err = -EINVAL;
    if ((cpu >= nr_cpu_ids) || (cpu == 0))
        goto out;

    err = -EEXIST;
    if (!cpu_online(cpu))
        goto out;

    err = cpu_notifier_call_chain(cpu, CPU_DOWN_PREPARE,
                                  &nb, false);
    if (err)
        goto fail;

    if (get_globl()->hypos_status < HYPOS_ACTIVE_STATE ||
            get_globl()->hypos_status == HYPOS_RESUME_STATE)
        on_selected_cpus(cpumask_of(cpu), _take_cpu_down,
                         NULL, true);
    else if ((err = halt_run(take_cpu_down, NULL, cpu)) < 0)
        goto fail;

    __cpu_die(cpu);
    err = cpu_online(cpu);
    BUG_ON(err);

    cpu_notifier_call_chain(cpu, CPU_DEAD, NULL, true);

    cpu_hotplug_done();

    return 0;

fail:
    cpu_notifier_call_chain(cpu, CPU_DOWN_FAILED, &nb, true);
out:
    cpu_hotplug_done();

    return err;
}

int cpu_up(unsigned int cpu)
{
    int err;
    struct notifier_block *nb = NULL;

    cpu_hotplug_begin();

    err = -EINVAL;
    if ((cpu >= nr_cpu_ids) || !cpu_present(cpu))
        goto out;

    err = -EEXIST;
    if (cpu_online(cpu))
        goto out;

    err = cpu_notifier_call_chain(cpu, CPU_UP_PREPARE,
                                  &nb, false);
    if (err)
        goto fail;

    err = __cpu_up(cpu);
    if ( err < 0 )
        goto fail;

    cpu_notifier_call_chain(cpu, CPU_ONLINE, NULL, true);

    cpu_hotplug_done();

    return 0;

fail:
    cpu_notifier_call_chain(cpu, CPU_UP_CANCELED, &nb, true);
out:
    cpu_hotplug_done();

    return err;
}

static int setup_cpu_sibling_map(int cpu)
{
    if (!zalloc_cpumask_var(&percpu(cpu_sibling_mask, cpu)) ||
        !zalloc_cpumask_var(&percpu(cpu_core_mask, cpu)))
        return -ENOMEM;

    cpumask_set_cpu(cpu, percpu(cpu_sibling_mask, cpu));
    cpumask_set_cpu(cpu, percpu(cpu_core_mask, cpu));

    return 0;
}

static void __bootfunc smp_prep_cpus(void)
{
    int rc;

    cpumask_copy(&cpu_present_map, &cpu_possible_map);

    rc = setup_cpu_sibling_map(0);
    if (rc)
        exec_panic("Unable to allocate CPU sibling/core maps");
}

int __bootfunc pre_secondary_cpu_setup(void)
{
    int i, ret = -1;

    psci_setup();

    smp_prep_cpus();

    for_each_present_cpu(i) {
        if ((num_online_cpus() < nr_cpu_ids) && !cpu_online(i)) {
            ret = cpu_up(i);

            if (ret)
                MSGH("Failed to bring up CPU %u [error %d]\n",
                        i, ret);
        }
    }

    MSGH("Brought up %ld CPUs\n", (long)num_online_cpus());

    return ret;
}

// --------------------------------------------------------------

/* XXX: Secondary CPUs Setup
 *
 *
 */
int __bootfunc post_secondary_cpu_setup(void)
{
    unsigned int cpuid = boot_setup.cpuid;

    identify_cpu(&current_cpu_data);

    return 0;
}

// --------------------------------------------------------------
