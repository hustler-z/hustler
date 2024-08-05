/**
 * Hustler's Project
 *
 * File:  hypos.c
 * Date:  2024/07/10
 * Usage:
 */

#include <org/section.h>
#include <org/vcpu.h>
#include <bsp/sched.h>
#include <bsp/cpu.h>
#include <bsp/config.h>

// --------------------------------------------------------------
struct hypos *hypos_list;
struct vcpu  *hypos_vcpus[NR_CPUS] __read_mostly;
hid_t __read_mostly max_init_hid;

bool opt_hypos0_vcpus_pin;

/* Protect updates/reads (resp.) of hypos_list and hypos_hash. */
DEFINE_SPINLOCK(hidlist_update_lock);
DEFINE_RCU_READ_LOCK(hidlist_read_lock);

#define HYPAIN_HASH_SIZE  256
#define HYPAIN_HASH(_id)  ((int)(_id)&(HYPAIN_HASH_SIZE-1))
static struct hypos       *hypos_hash[HYPAIN_HASH_SIZE];
struct hypos              *hypos_list;
struct hypos              *hardware_hypos __read_mostly;

/* Private hypos structs for HYPID__, HYPID_IO, etc. */
struct hypos *__read_mostly h__;
struct hypos *__read_mostly h_io;
struct vcpu *idle_vcpu[NR_CPUS] __read_mostly;

bool __read_mostly vmtrace_available;
bool __read_mostly vpmu_is_available;

void __bootfunc hypos_setup(void)
{
    vcpu_sched_setup();
    set_current(hypos_vcpus[0]);
}

#if IS_IMPLEMENTED(__HYPOS_IMPL)
// --------------------------------------------------------------

/* XXX: hypos Implementation
 * --------------------------------------------------------------
 *
 * +-----------------+           +-----------------+
 * |     hypos 0     |           |      hypos x    |
 * +-----------------+           +-----------------+
 *          |                             |
 *          ▼                             ▼
 * XXX: The priviledged
 * one which have most  ------------------▶
 * things under control.     (Channel)
 *
 * --------------------------------------------------------------
 */
DEFINE_PERCPU(struct vcpu *, curr_vcpu);

static void do_idle(void)
{
    unsigned int cpu = smp_processor_id();

    rcu_idle_enter(cpu);
    /* rcu_idle_enter() can raise TIMER_SOFTIRQ. Process it now. */
    process_pending_softirqs();

    local_irq_disable();
    if (cpu_is_haltable(cpu)) {
        dsb(sy);
        wfi();
    }
    local_irq_enable();

    rcu_idle_exit(cpu);
}

static void noreturn idle_loop(void)
{
    unsigned int cpu = smp_processor_id();

    for ( ; ; ) {
        if (cpu_is_offline(cpu))
            stop_cpu();

        /* Are we here for running vcpu context tasklets, or for idling? */
        if (unlikely(tasklet_work_to_do(cpu))) {
            do_tasklet();
            /* Livepatch work is always kicked off via a tasklet. */
            check_for_livepatch_work();
        } else if ( !softirq_pending(cpu) && !scrub_free_pages() &&
                  !softirq_pending(cpu) )
            do_idle();

        do_softirq();
    }
}

static void ctxt_switch_from(struct vcpu *p)
{
    if (is_idle_vcpu(p))
        return;

    vttbl_save_state(p);

    /* CP 15 */
    p->arch.csselr = READ_SYSREG(CSSELR_EL1);

    /* VFP */
    vfp_save_state(p);

    /* Control Registers */
    p->arch.cpacr = READ_SYSREG(CPACR_EL1);

    p->arch.contextidr = READ_SYSREG(CONTEXTIDR_EL1);
    p->arch.tpidr_el0 = READ_SYSREG(TPIDR_EL0);
    p->arch.tpidrro_el0 = READ_SYSREG(TPIDRRO_EL0);
    p->arch.tpidr_el1 = READ_SYSREG(TPIDR_EL1);

    /* Arch timer */
    p->arch.cntkctl = READ_SYSREG(CNTKCTL_EL1);
    virt_timer_save(p);

    isb();

    /* MMU */
    p->arch.vbar = READ_SYSREG(VBAR_EL1);
    p->arch.ttbcr = READ_SYSREG(TCR_EL1);
    p->arch.ttbr0 = READ_SYSREG64(TTBR0_EL1);
    p->arch.ttbr1 = READ_SYSREG64(TTBR1_EL1);
    p->arch.par = read_sysreg_par();
    p->arch.mair = READ_SYSREG64(MAIR_EL1);
    p->arch.amair = READ_SYSREG64(AMAIR_EL1);

    /* Fault Status */
    p->arch.far = READ_SYSREG64(FAR_EL1);
    p->arch.esr = READ_SYSREG64(ESR_EL1);

    p->arch.afsr0 = READ_SYSREG(AFSR0_EL1);
    p->arch.afsr1 = READ_SYSREG(AFSR1_EL1);

    /* VGIC */
    gic_save_state(p);

    isb();
}

static void ctxt_switch_to(struct vcpu *n)
{
    register_t vpidr;

    if (is_idle_vcpu(n))
        return;

    vpidr = READ_SYSREG(MIDR_EL1);
    WRITE_SYSREG(vpidr, VPIDR_EL2);
    WRITE_SYSREG(n->arch.vmpidr, VMPIDR_EL2);

    /* VGIC */
    gic_restore_state(n);

    /* Fault Status */
    WRITE_SYSREG64(n->arch.far, FAR_EL1);
    WRITE_SYSREG64(n->arch.esr, ESR_EL1);
    WRITE_SYSREG(n->arch.afsr0, AFSR0_EL1);
    WRITE_SYSREG(n->arch.afsr1, AFSR1_EL1);

    /* MMU */
    WRITE_SYSREG(n->arch.vbar, VBAR_EL1);
    WRITE_SYSREG(n->arch.ttbcr, TCR_EL1);
    WRITE_SYSREG64(n->arch.ttbr0, TTBR0_EL1);
    WRITE_SYSREG64(n->arch.ttbr1, TTBR1_EL1);
    WRITE_SYSREG64(n->arch.par, PAR_EL1);
    WRITE_SYSREG64(n->arch.mair, MAIR_EL1);
    WRITE_SYSREG64(n->arch.amair, AMAIR_EL1);

    isb();

    vttbl_restore_state(n);

    WRITE_SYSREG(n->arch.cpacr, CPACR_EL1);
    WRITE_SYSREG(n->arch.contextidr, CONTEXTIDR_EL1);
    WRITE_SYSREG(n->arch.tpidr_el0, TPIDR_EL0);
    WRITE_SYSREG(n->arch.tpidrro_el0, TPIDRRO_EL0);
    WRITE_SYSREG(n->arch.tpidr_el1, TPIDR_EL1);
    WRITE_SYSREG(n->arch.cptr_el2, CPTR_EL2);
    isb();

    vfp_restore_state(n);

    WRITE_SYSREG(n->arch.csselr, CSSELR_EL1);

    isb();

    WRITE_SYSREG(n->arch.cntkctl, CNTKCTL_EL1);
    virt_timer_restore(n);

    WRITE_SYSREG(n->arch.mdcr_el2, MDCR_EL2);
}

static void schedule_tail(struct vcpu *prev)
{
    ASSERT(prev != current);

    ctxt_switch_from(prev);

    ctxt_switch_to(current);

    local_irq_enable();

    sched_context_switched(prev, current);

    update_runstate_area(current);

    /* Ensure that the vcpu has an up-to-date time base. */
    update_vcpu_system_time(current);
}

extern void noreturn return_to_new_vcpu64(void);

static void continue_new_vcpu(struct vcpu *prev)
{
    current->arch.actlr = READ_SYSREG(ACTLR_EL1);
    processor_vcpu_initialise(current);

    schedule_tail(prev);

    if (is_idle_vcpu(current))
        reset_stack_and_jump(idle_loop);
    else
        reset_stack_and_jump(return_to_new_vcpu64);
}

void context_switch(struct vcpu *prev, struct vcpu *next)
{
    ASSERT(local_irq_is_enabled());
    ASSERT(prev != next);
    ASSERT(!vcpu_cpu_dirty(next));

    update_runstate_area(prev);

    local_irq_disable();

    set_current(next);

    prev = __context_switch(prev, next);

    schedule_tail(prev);
}

void continue_running(struct vcpu *same)
{
    /* Nothing to do */
}

void sync_local_execstate(void)
{
    /* Nothing to do -- no lazy switching */
}

void sync_vcpu_execstate(struct vcpu *v)
{
    smp_mb();
}

#define NEXT_ARG(fmt, args)                                                 \
({                                                                          \
    unsigned long __arg;                                                    \
    switch (*(fmt)++) {                                                     \
    case 'i': __arg = (unsigned long)va_arg(args, unsigned int);  break;    \
    case 'l': __arg = (unsigned long)va_arg(args, unsigned long); break;    \
    case 'h': __arg = (unsigned long)va_arg(args, void *);        break;    \
    default:  goto bad_fmt;                                                 \
    }                                                                       \
    __arg;                                                                  \
})

unsigned long hypercall_create_continuation(
    unsigned int op, const char *format, ...)
{
    struct mc_state *mcs = &current->mc_state;
    struct cpu_user_regs *regs;
    const char *p = format;
    unsigned long arg, rc;
    unsigned int i;
    va_list args;

    current->hcall_preempted = true;

    va_start(args, format);

    if (mcs->flags & MCSF_in_multicall) {
        for (i = 0; *p != '\0'; i++)
            mcs->call.args[i] = NEXT_ARG(p, args);

        rc = mcs->call.result;
    } else {
        regs = guest_cpu_user_regs();

        if (!is_32bit_hypos(current->hypos)) {
            regs->x16 = op;

            for (i = 0; *p != '\0'; i++) {
                arg = NEXT_ARG(p, args);

                switch (i) {
                case 0: regs->x0 = arg; break;
                case 1: regs->x1 = arg; break;
                case 2: regs->x2 = arg; break;
                case 3: regs->x3 = arg; break;
                case 4: regs->x4 = arg; break;
                case 5: regs->x5 = arg; break;
                }
            }

            /* Return value gets written back to x0 */
            rc = regs->x0;
        }
    }

    va_end(args);

    return rc;

 bad_fmt:
    va_end(args);
    gMSGH(_LOG_ERR, "Bad hypercall continuation format '%c'\n", *p);
    ASSERT_UNREACHABLE();
    hypos_crash(current->hypos);
    return 0;
}

#undef NEXT_ARG

void startup_cpu_idle_loop(void)
{
    struct vcpu *v = current;

    ASSERT(is_idle_vcpu(v));

    reset_stack_and_jump(idle_loop);
}

struct hypos *alloc_hypos_struct(void)
{
    struct hypos *d;
    BUILD_BUG_ON(sizeof(*d) > PAGE_SIZE);
    d = alloc__heap_pages(0, 0);
    if (d == NULL)
        return NULL;

    clear_page(d);
    return d;
}

void free_hypos_struct(struct hypos *d)
{
    free__heap_page(d);
}

void dump_pageframe_info(struct hypos *d)
{

}

#define MAX_PAGES_PER_VCPU  1

struct vcpu *alloc_vcpu_struct(const struct hypos *d)
{
    struct vcpu *v;

    BUILD_BUG_ON(sizeof(*v) > MAX_PAGES_PER_VCPU * PAGE_SIZE);
    v = alloc__heap_pages(get_order_from_bytes(sizeof(*v)), 0);
    if (v != NULL) {
        unsigned int i;

        for (i = 0; i < DIV_ROUND_UP(sizeof(*v), PAGE_SIZE); i++)
            clear_page((void *)v + i * PAGE_SIZE);
    }

    return v;
}

void free_vcpu_struct(struct vcpu *v)
{
    free__heap_pages(v, get_order_from_bytes(sizeof(*v)));
}

int arch_vcpu_create(struct vcpu *v)
{
    int rc = 0;

    BUILD_BUG_ON( sizeof(struct cpu_info) > STACK_SIZE );

    v->arch.stack = alloc__heap_pages(STACK_ORDER, MEMF_node(vcpu_to_node(v)));
    if (v->arch.stack == NULL)
        return -ENOMEM;

    v->arch.cpu_info = (struct cpu_info *)(v->arch.stack
                                           + STACK_SIZE
                                           - sizeof(struct cpu_info));
    memset(v->arch.cpu_info, 0, sizeof(*v->arch.cpu_info));

    v->arch.saved_context.sp = (register_t)v->arch.cpu_info;
    v->arch.saved_context.pc = (register_t)continue_new_vcpu;

    /* Idle VCPUs don't need the rest of this setup */
    if (is_idle_vcpu(v))
        return rc;

    v->arch.sctlr = SCTLR_GUEST_INIT;

    v->arch.vmpidr = MPIDR_SMP | vcpuid_to_vaffinity(v->vcpuid);

    v->arch.cptr_el2 = get_default_cptr_flags();
    if (is_sve_hypos(v->hypos)) {
        if ( (rc = sve_context_init(v)) != 0 )
            goto fail;
        v->arch.cptr_el2 &= ~HCPTR_CP(8);
    }

    v->arch.hcr_el2 = get_default_hcr_flags();

    v->arch.mdcr_el2 = HDCR_TDRA | HDCR_TDOSA | HDCR_TDA;
    if (!(v->hypos->options & __HYPCTL_CDF_vpmu))
        v->arch.mdcr_el2 |= HDCR_TPM | HDCR_TPMCR;

    if ((rc = vcpu_vgic_init(v)) != 0)
        goto fail;

    if ((rc = vcpu_vtimer_init(v)) != 0)
        goto fail;

    if (get_ssbd_state() == ARM_SSBD_RUNTIME)
        v->arch.cpu_info->flags |= CPUINFO_WORKAROUND_2_FLAG;

    return rc;

fail:
    arch_vcpu_destroy(v);
    return rc;
}

void arch_vcpu_destroy(struct vcpu *v)
{
    if (is_sve_hypos(v->hypos))
        sve_context_free(v);
    vcpu_timer_destroy(v);
    vcpu_vgic_free(v);
    free__heap_pages(v->arch.stack, STACK_ORDER);
}

void vcpu_switch_to_aarch64_mode(struct vcpu *v)
{
    v->arch.hcr_el2 |= HCR_RW;
}

int arch_sanitise_hypos_config(struct __hctl_createhypos *config)
{
    unsigned int max_vcpus;
    unsigned int flags_required = (__HYPCTL_CDF_hvm | __HYPCTL_CDF_hap);
    unsigned int flags_optional = (__HYPCTL_CDF_iommu | __HYPCTL_CDF_vpmu);
    unsigned int sve_vl_bits = sve_decode_vl(config->arch.sve_vl);

    if ((config->flags & ~flags_optional) != flags_required) {
        MSGH("Unsupported configuration %#x\n",
                config->flags);
        return -EINVAL;
    }

    /* Check feature flags */
    if (sve_vl_bits > 0) {
        unsigned int zcr_max_bits = get_sys_vl_len();

        if (!zcr_max_bits) {
            MSGH("SVE is unsupported on this machine.\n");
            return -EINVAL;
        }

        if (sve_vl_bits > zcr_max_bits) {
            MSGH("Requested SVE vector length (%u) > supported length (%u)\n",
                    sve_vl_bits, zcr_max_bits);
            return -EINVAL;
        }
    }

    if (config->iommu_opts & __HYPCTL_IOMMU_no_sharept) {
        MSGH("Unsupported iommu option: __HYPCTL_IOMMU_no_sharept\n");
        return -EINVAL;
    }

    if (config->arch.gic_version == __HYPCTL_CONFIG_GIC_NATIVE) {
        switch (gic_hw_version()) {
        case GIC_V3:
            config->arch.gic_version = __HYPCTL_CONFIG_GIC_V3;
            break;
        default:
            ASSERT_UNREACHABLE();
            return -EINVAL;
        }
    }

    max_vcpus = min(vgic_max_vcpus(config->arch.gic_version), MAX_VIRT_CPUS);

    if (max_vcpus == 0) {
        MSGH("Unsupported GIC version\n");
        return -EINVAL;
    }

    if (config->max_vcpus > max_vcpus) {
        MSGH("Requested vCPUs (%u) exceeds max (%u)\n",
                config->max_vcpus, max_vcpus);
        return -EINVAL;
    }

    if (config->arch.tee_type != __HYPCTL_CONFIG_TEE_NONE &&
        config->arch.tee_type != tee_get_type()) {
        MSGH("Unsupported TEE type\n");
        return -EINVAL;
    }

    return 0;
}

int arch_hypos_create(struct hypos *d,
                       struct __hctl_createhypos *config,
                       unsigned int flags)
{
    unsigned int count = 0;
    int rc;

    BUILD_BUG_ON(GUEST_MAX_VCPUS < MAX_VIRT_CPUS);

    if (is_idle_hypos(d))
        return 0;

    ASSERT(config != NULL);

    if ((rc = iommu_hypos_init(d, config->iommu_opts)) != 0)
        goto fail;

    if ((rc = vttbl_init(d)) != 0)
        goto fail;

    rc = -ENOMEM;
    if ((d->shared_info = alloc__heap_pages(0, 0)) == NULL)
        goto fail;

    clear_page(d->shared_info);
    share___page_with_guest(virt_to_page(d->shared_info), d, SHARE_rw);

    switch (config->arch.gic_version) {
    case __HYPCTL_CONFIG_GIC_V2:
        d->arch.vgic.version = GIC_V2;
        break;
    case __HYPCTL_CONFIG_GIC_V3:
        d->arch.vgic.version = GIC_V3;
        break;
    default:
        BUG();
    }

    if ( (rc = hypos_vgic_register(d, &count)) != 0 )
        goto fail;

    count += hypos_vpci_get_num_mmio_handlers(d);

    if ((rc = hypos_io_init(d, count + MAX_IO_HANDLER)) != 0)
        goto fail;

    if ((rc = hypos_vgic_init(d, config->arch.nr_spis)) != 0)
        goto fail;

    if ((rc = hypos_vtimer_init(d, &config->arch)) != 0)
        goto fail;

    if ((rc = tee_hypos_init(d, config->arch.tee_type)) != 0)
        goto fail;

    update_hypos_wallclock_time(d);

    if (!is_hardware_hypos(d)) {
        d->arch.evtchn_irq = GUEST_EVTCHN_PPI;
        /* At this stage vgic_reserve_virq should never fail */
        if (!vgic_reserve_virq(d, GUEST_EVTCHN_PPI))
            BUG();
    }

    if (is_hardware_hypos(d) && (rc = hypos_vuart_init(d)))
        goto fail;

    if ((rc = hypos_vpci_init(d)) != 0)
        goto fail;

    d->arch.sve_vl = config->arch.sve_vl;


    return 0;

fail:
    d->is_dying = HYPDYING_dead;
    arch_hypos_destroy(d);

    return rc;
}

int arch_hypos_teardown(struct hypos *d)
{
    int ret = 0;

    BUG_ON(!d->is_dying);

    switch (d->teardown.arch_val) {
#define PROGRESS(x)                             \
        d->teardown.arch_val = PROG_ ## x;      \
        fallthrough;                            \
    case PROG_ ## x

        enum {
            PROG_none,
            PROG_tee,
            PROG_done,
        };
    case PROG_none:
        BUILD_BUG_ON(PROG_none != 0);

    PROGRESS(tee):
        ret = tee_hypos_teardown(d);
        if ( ret )
            return ret;

    PROGRESS(done):
        break;

#undef PROGRESS

    default:
        BUG();
    }

    return 0;
}

void arch_hypos_destroy(struct hypos *d)
{
    iommu_hypos_destroy(d);
    vttbl_final_teardown(d);
    hypos_vgic_free(d);
    hypos_vuart_free(d);
    free__heap_page(d->shared_info);
    hypos_io_free(d);
}

void arch_hypos_shutdown(struct hypos *d)
{
}

void arch_hypos_pause(struct hypos *d)
{
}

void arch_hypos_unpause(struct hypos *d)
{
}

int arch_hypos_soft_reset(struct hypos *d)
{
    return -ENOSYS;
}

void arch_hypos_creation_finished(struct hypos *d)
{
    vttbl_hypos_creation_finished(d);
}

static int is_guest_pv64_psr(u64 psr)
{
    if (psr & PSR_MODE_BIT)
        return 0;

    switch (psr & PSR_MODE_MASK) {
    case PSR_MODE_EL1h:
    case PSR_MODE_EL1t:
    case PSR_MODE_EL0t:
        return 1;
    case PSR_MODE_EL3h:
    case PSR_MODE_EL3t:
    case PSR_MODE_EL2h:
    case PSR_MODE_EL2t:
    default:
        return 0;
    }
}

int arch_set_info_guest(
    struct vcpu *v, vcpu_guest_context_u c)
{
    struct vcpu_guest_context *ctxt = c.nat;
    struct vcpu_guest_core_regs *regs = &c.nat->user_regs;

    if (!is_guest_pv64_psr(regs->cpsr))
        return -EINVAL;

    if (regs->spsr_el1 && !is_guest_pv64_psr(regs->spsr_el1))
        return -EINVAL;

    vcpu_regs_user_to_hyp(v, regs);

    v->arch.sctlr = ctxt->sctlr;
    v->arch.ttbr0 = ctxt->ttbr0;
    v->arch.ttbr1 = ctxt->ttbr1;
    v->arch.ttbcr = ctxt->ttbcr;

    v->is_initialised = 1;

    if (ctxt->flags & VGCF_online)
        clear_bit(_VPF_down, &v->pause_flags);
    else
        set_bit(_VPF_down, &v->pause_flags);

    return 0;
}

int arch_initialise_vcpu(struct vcpu *v, __GUEST_HANDLE_PARAM(void) arg)
{
    ASSERT_UNREACHABLE();
    return -EOPNOTSUPP;
}

int arch_vcpu_reset(struct vcpu *v)
{
    vcpu_end_shutdown_deferral(v);
    return 0;
}

static int relinquish_memory(struct hypos *d, struct page_list_head *list)
{
    struct page_info *page, *tmp;
    int               ret = 0;

    rspin_lock(&d->page_alloc_lock);

    page_list_for_each_safe(page, tmp, list) {
        if (unlikely(!get_page(page, d)))
            continue;

        put_page_alloc_ref(page);
        put_page(page);

        if (hypercall_preempt_check()) {
            ret = -ERESTART;
            goto out;
        }
    }

  out:
    rspin_unlock(&d->page_alloc_lock);
    return ret;
}

enum {
    PROG_pci = 1,
    PROG_tee,
    PROG__,
    PROG_page,
    PROG_mapping,
    PROG_vttbl_root,
    PROG_vttbl,
    PROG_vttbl_pool,
    PROG_done,
};

#define PROGRESS(x)                         \
    d->arch.rel_priv = PROG_ ## x;          \
    /* Fallthrough */                       \
    case PROG_ ## x

int hypos_relinquish_resources(struct hypos *d)
{
    int ret = 0;

    switch (d->arch.rel_priv) {
    PROGRESS(tee):
        ret = tee_relinquish_resources(d);
        if (ret)
            return ret;
    PROGRESS(page):
        ret = relinquish_memory(d, &d->page_list);
        if (ret)
            return ret;
    PROGRESS(mapping):
        ret = relinquish_vttbl_mapping(d);
        if (ret)
            return ret;
    PROGRESS(vttbl_root):
        vttbl_clear_root_pages(&d->arch.vttbl);
    PROGRESS(vttbl):
        ret = vttbl_teardown(d);
        if (ret)
            return ret;
    PROGRESS(vttbl_pool):
        ret = vttbl_teardown_allocation(d);
        if(ret)
            return ret;
    PROGRESS(done):
        break;
    default:
        BUG();
    }

    return 0;
}

#undef PROGRESS

void arch_dump_hypos_info(struct hypos *d)
{
    vttbl_dump_info(d);
}


long do_vcpu_op(int cmd, unsigned int vcpuid, __GUEST_HANDLE_PARAM(void) arg)
{
    struct hypos *d = current->hypos;
    struct vcpu *v;

    if ((v = hypos_vcpu(d, vcpuid)) == NULL)
        return -ENOENT;

    switch (cmd) {
        case VCPUOP_register_vcpu_info:
        case VCPUOP_register_runstate_memory_area:
            return common_vcpu_op(cmd, v, arg);
        default:
            return -EINVAL;
    }
}

void arch_dump_vcpu_info(struct vcpu *v)
{
    gic_dump_info(v);
    gic_dump_vgic_info(v);
}

void vcpu_mark_events_pending(struct vcpu *v)
{
    bool already_pending = guest_test_and_set_bit(v->hypos,
        0, (unsigned long *)&vcpu_info(v, evtchn_upcall_pending));

    if (already_pending)
        return;

    vgic_inject_irq(v->hypos, v, v->hypos->arch.evtchn_irq, true);
}

void vcpu_update_evtchn_irq(struct vcpu *v)
{
    bool pending = vcpu_info(v, evtchn_upcall_pending);

    vgic_inject_irq(v->hypos, v, v->hypos->arch.evtchn_irq, pending);
}

void vcpu_block_unless_event_pending(struct vcpu *v)
{
    vcpu_block();
    if (local_events_need_delivery_nomask())
        vcpu_unblock(current);
}

void vcpu_kick(struct vcpu *v)
{
    bool running = v->is_running;

    vcpu_unblock(v);
    if (running && v != current) {
        perfc_incr(vcpu_kick);
        smp_send_event_check_mask(cpumask_of(v->processor));
    }
}

// --------------------------------------------------------------

static void __hypos_finalise_shutdown(struct hypos *d)
{
    struct vcpu *v;

    BUG_ON(!spin_is_locked(&d->shutdown_lock));

    if (d->is_shut_down)
        return;

    for_each_vcpu (d, v)
        if (!v->paused_for_shutdown)
            return;

    d->is_shut_down = 1;
    if ((d->shutdown_code == SHUTDOWN_suspend) && d->suspend_evtchn)
        evtchn_send(d, d->suspend_evtchn);
    else
        send_global_virq(VIRQ_HYP_EXC);
}

static void vcpu_check_shutdown(struct vcpu *v)
{
    struct hypos *d = v->hypos;

    spin_lock(&d->shutdown_lock);

    if (d->is_shutting_down) {
        if (!v->paused_for_shutdown)
            vcpu_pause_nosync(v);
        v->paused_for_shutdown = 1;
        v->defer_shutdown = 0;
        __hypos_finalise_shutdown(d);
    }

    spin_unlock(&d->shutdown_lock);
}

static void vcpu_info_reset(struct vcpu *v)
{
    struct hypos *d = v->hypos;

    v->vcpu_info_area.map =
        ((v->vcpuid < __LEGACY_MAX_VCPUS)
         ? (vcpu_info_t *)&shared_info(d, vcpu_info[v->vcpuid])
         : &dummy_vcpu_info);
}

static void vmtrace_free_buffer(struct vcpu *v)
{
    const struct hypos *d = v->hypos;
    struct page_info *pg = v->vmtrace.pg;
    unsigned int i;

    if (!pg)
        return;

    v->vmtrace.pg = NULL;

    for (i = 0; i < (d->vmtrace_size >> PAGE_SHIFT); i++) {
        put_page_alloc_ref(&pg[i]);
        put_page_and_type(&pg[i]);
    }
}

static int vmtrace_alloc_buffer(struct vcpu *v)
{
    struct hypos *d = v->hypos;
    struct page_info *pg;
    unsigned int i;

    if (!d->vmtrace_size)
        return 0;

    pg = alloc_hheap_pages(d, get_order_from_bytes(d->vmtrace_size),
                             MEMF_no_refcount);
    if (!pg)
        return -ENOMEM;

    for (i = 0; i < (d->vmtrace_size >> PAGE_SHIFT); i++)
        if (unlikely(!get_page_and_type(&pg[i], d, PGT_writable_page)))
            goto refcnt_err;

    v->vmtrace.pg = pg;
    return 0;

 refcnt_err:
    while (i--) {
        put_page_alloc_ref(&pg[i]);
        put_page_and_type(&pg[i]);
    }

    return -ENODATA;
}

static int vcpu_teardown(struct vcpu *v)
{
    vmtrace_free_buffer(v);

    return 0;
}

static void vcpu_destroy(struct vcpu *v)
{
    free_vcpu_struct(v);
}

struct vcpu *vcpu_create(struct hypos *d, unsigned int vcpu_id)
{
    struct vcpu *v;

    if (vcpu_id >= d->max_vcpus || d->vcpu[vcpu_id] ||
        (!is_idle_hypos(d) && vcpu_id && !d->vcpu[vcpu_id - 1])) {
        ASSERT_UNREACHABLE();
        return NULL;
    }

    if ((v = alloc_vcpu_struct(d)) == NULL)
        return NULL;

    v->hypos = d;
    v->vcpuid = vcpu_id;
    v->dirty_cpu = VCPU_CPU_CLEAN;

    rwlock_init(&v->virq_lock);

    tasklet_init(&v->continue_hypercall_tasklet, NULL, NULL);

    grant_table_init_vcpu(v);

    if (is_idle_hypos(d)) {
        v->runstate.state = RUNSTATE_running;
        v->new_state = RUNSTATE_running;
    } else {
        v->runstate.state = RUNSTATE_offline;
        v->runstate.state_entry_time = NOW();
        set_bit(_VPF_down, &v->pause_flags);
        vcpu_info_reset(v);
        init_waitqueue_vcpu(v);
    }

    if (sched_init_vcpu(v) != 0)
        goto fail_wq;

    if (vmtrace_alloc_buffer(v) != 0)
        goto fail_wq;

    if (arch_vcpu_create(v) != 0)
        goto fail_sched;

    d->vcpu[vcpu_id] = v;
    if (vcpu_id != 0) {
        int prev_id = v->vcpuid - 1;
        while ((prev_id >= 0) && (d->vcpu[prev_id] == NULL))
            prev_id--;
        BUG_ON(prev_id < 0);
        v->next_in_list = d->vcpu[prev_id]->next_in_list;
        d->vcpu[prev_id]->next_in_list = v;
    }

    /* Must be called after making new vcpu visible to for_each_vcpu(). */
    vcpu_check_shutdown(v);

    return v;

 fail_sched:
    sched_destroy_vcpu(v);
 fail_wq:
    destroy_waitqueue_vcpu(v);

    /* Must not hit a continuation in this context. */
    if (vcpu_teardown(v))
        ASSERT_UNREACHABLE();

    vcpu_destroy(v);

    return NULL;
}

static int late_hwh_init(struct hypos *d)
{
    return 0;
}

#ifdef CONFIG_HAS_PIRQ

static unsigned int __read_mostly extra_hwh_irqs;
static unsigned int __read_mostly extra_hU_irqs = 32;

static int __init parse_extra_guest_irqs(const char *s)
{
    if (isdigit(*s))
        extra_hU_irqs = simple_strtoul(s, &s, 0);
    if (*s == ',' && isdigit(*++s))
        extra_hwh_irqs = simple_strtoul(s, &s, 0);

    return *s ? -EINVAL : 0;
}

#endif /* CONFIG_HAS_PIRQ */

static int __init parse_h0_param(const char *s)
{
    const char *ss;
    int rc = 0;

    do {
        int ret;

        ss = strchr(s, ',');
        if (!ss)
            ss = strchr(s, '\0');

        ret = parse_arch_h0_param(s, ss);
        if (ret && !rc)
            rc = ret;

        s = ss + 1;
    } while (*ss);

    return rc;
}

static int hypos_teardown(struct hypos *d)
{
    struct vcpu *v;
    int rc;

    BUG_ON(!d->is_dying);

    switch (d->teardown.val) {
#define PROGRESS(x)                             \
        d->teardown.val = PROG_ ## x;           \
        fallthrough;                            \
    case PROG_ ## x

#define PROGRESS_VCPU(x)                        \
        d->teardown.val = PROG_vcpu_ ## x;      \
        d->teardown.vcpu = v;                   \
        fallthrough;                            \
    case PROG_vcpu_ ## x:                       \
        v = d->teardown.vcpu

        enum {
            PROG_none,
            PROG_gnttab_mappings,
            PROG_vcpu_teardown,
            PROG_arch_teardown,
            PROG_done,
        };
    case PROG_none:
        BUILD_BUG_ON(PROG_none != 0);

    PROGRESS(gnttab_mappings):
        rc = gnttab_release_mappings(d);
        if ( rc )
            return rc;

        for_each_vcpu ( d, v )
        {
            /* SAF-5-safe MISRA C Rule 16.2: switch label enclosed by for loop */
            PROGRESS_VCPU(teardown);

            rc = vcpu_teardown(v);
            if ( rc )
                return rc;
        }
    PROGRESS(arch_teardown):
        rc = arch_hypos_teardown(d);
        if ( rc )
            return rc;

    PROGRESS(done):
        break;

#undef PROGRESS_VCPU
#undef PROGRESS

    default:
        BUG();
    }

    return 0;
}

static void _hypos_destroy(struct hypos *d)
{
    BUG_ON(!d->is_dying);
    BUG_ON(atomic_read(&d->refcnt) != HYPAIN_DESTROYED);

    xfree(d->pbuf);

    argo_destroy(d);

    rangeset_hypos_destroy(d);

    free_cpumask_var(d->dirty_cpumask);

    xsm_free_security_hypos(d);

    lock_profile_deregister_struct(LOCKPROF_TYPE_PERHYP, d);

    free_hypos_struct(d);
}

static int sanitise_hypos_config(struct __hctl_createhypos *config)
{
    bool hvm = config->flags & __HYPCTL_CDF_hvm;
    bool hap = config->flags & __HYPCTL_CDF_hap;
    bool iommu = config->flags & __HYPCTL_CDF_iommu;
    bool vpmu = config->flags & __HYPCTL_CDF_vpmu;

    if (config->flags &
        ~(__HYPCTL_CDF_hvm | __HYPCTL_CDF_hap |
        __HYPCTL_CDF_s3_integrity | __HYPCTL_CDF_oos_off |
        __HYPCTL_CDF_xs_hypos | __HYPCTL_CDF_iommu |
        __HYPCTL_CDF_nested_virt | __HYPCTL_CDF_vpmu)) {
        MSGH("Unknown CDF flags %#x\n", config->flags);
        return -EINVAL;
    }

    if (config->grant_opts & ~__HYPCTL_GRANT_version_mask) {
        MSGH("Unknown grant options %#x\n", config->grant_opts);
        return -EINVAL;
    }

    if (config->max_vcpus < 1) {
        MSGH("No vCPUS\n");
        return -EINVAL;
    }

    if (hap && !hvm) {
        MSGH("HAP requested for non-HVM guest\n");
        return -EINVAL;
    }

    if (iommu) {
        if (config->iommu_opts & ~__HYPCTL_IOMMU_no_sharept)
        {
            MSGH("Unknown IOMMU options %#x\n",
                    config->iommu_opts);
            return -EINVAL;
        }

        if (!iommu_enabled) {
            MSGH("IOMMU requested but not available\n");
            return -EINVAL;
        }
    } else {
        if (config->iommu_opts) {
            MSGH("IOMMU options specified but IOMMU not requested\n");
            return -EINVAL;
        }
    }

    if (config->vmtrace_size && !vmtrace_available) {
        MSGH("vmtrace requested but not available\n");
        return -EINVAL;
    }

    if (vpmu && !vpmu_is_available) {
        MSGH("vpmu requested but cannot be enabled this way\n");
        return -EINVAL;
    }

    return arch_sanitise_hypos_config(config);
}

struct hypos *hypos_create(hid_t hid,
                             struct __hctl_createhypos *config,
                             unsigned int flags)
{
    struct hypos *d, **pd, *old_hwh = NULL;
    enum { INIT_watchdog = 1u<<1,
           INIT_evtchn = 1u<<3, INIT_gnttab = 1u<<4, INIT_arch = 1u<<5 };
    int err, init_status = 0;

    if (config && (err = sanitise_hypos_config(config)))
        return ERR_PTR(err);

    if ((d = alloc_hypos_struct()) == NULL)
        return ERR_PTR(-ENOMEM);

    d->hypos_id = hid;

    d->cdf = flags;

    ASSERT(is_system_hypos(d) ? config == NULL : config != NULL);

    if (config) {
        d->options = config->flags;
        d->vmtrace_size = config->vmtrace_size;
    }

    /* Sort out our idea of is_control_hypos(). */
    d->is_privileged = flags & CDF_privileged;

    /* Sort out our idea of is_hardware_hypos(). */
    if (hid == 0 || hid == hardware_hid) {
        if (hardware_hid < 0 || hardware_hid >= HYPID_FIRST_RESERVED)
            panic("The value of hardware_h must be a valid hypos ID\n");

        old_hwh = hardware_hypos;
        hardware_hypos = d;
    }

    TRACE_1D(TRC_HYP0_HYP_ADD, d->hypos_id);

    lock_profile_register_struct(LOCKPROF_TYPE_PERHYP, d, hid);

    atomic_set(&d->refcnt, 1);
    RCU_READ_LOCK_INIT(&d->rcu_lock);
    rspin_lock_init_prof(d, hypos_lock);
    rspin_lock_init_prof(d, page_alloc_lock);
    spin_lock_init(&d->hypercall_deadlock_mutex);
    INIT_PAGE_LIST_HEAD(&d->page_list);
    INIT_PAGE_LIST_HEAD(&d->extra_page_list);
    INIT_PAGE_LIST_HEAD(&d->_page_list);

    spin_lock_init(&d->node_affinity_lock);
    d->node_affinity = NODE_MASK_ALL;
    d->auto_node_affinity = 1;

    spin_lock_init(&d->shutdown_lock);
    d->shutdown_code = SHUTDOWN_CODE_INVALID;

    spin_lock_init(&d->pbuf_lock);

    rwlock_init(&d->vnuma_rwlock);

#ifdef CONFIG_HAS_PCI
    INIT_LIST_HEAD(&d->pdev_list);
    rwlock_init(&d->pci_lock);
#endif

    if (!is_system_hypos(d)) {
        err = -ENOMEM;
        d->vcpu = zalloc_array(struct vcpu *, config->max_vcpus);
        if (!d->vcpu)
            goto fail;

        d->max_vcpus = config->max_vcpus;
    }

    if ((err = xsm_alloc_security_hypos(d)) != 0)
        goto fail;

    err = -ENOMEM;
    if (!zalloc_cpumask_var(&d->dirty_cpumask))
        goto fail;

    rangeset_hypos_initialise(d);

    if (is_system_hypos(d) && !is_idle_hypos(d))
        return d;

#ifdef CONFIG_HAS_PIRQ
    if (!is_idle_hypos(d)) {
        if ( !is_hardware_hypos(d) )
            d->nr_pirqs = nr_static_irqs + extra_hU_irqs;
        else
            d->nr_pirqs = extra_hwh_irqs ? nr_static_irqs + extra_hwh_irqs
                                           : arch_hwh_irqs(hid);
        d->nr_pirqs = min(d->nr_pirqs, nr_irqs);

        radix_tree_init(&d->pirq_tree);
    }
#endif

    if ((err = arch_hypos_create(d, config, flags)) != 0)
        goto fail;
    init_status |= INIT_arch;

    if (!is_idle_hypos(d)) {
        ASSERT(config);

        watchdog_hypos_init(d);
        init_status |= INIT_watchdog;

        err = -ENOMEM;
        d->iomem_caps = rangeset_new(d, "I/O Memory", RANGESETF_prettyprint_hex);
        d->irq_caps   = rangeset_new(d, "Interrupts", 0);
        if (!d->iomem_caps || !d->irq_caps)
            goto fail;

        if ((err = xsm_hypos_create(XSM_HOOK, d, config->ssidref)) != 0)
            goto fail;

        d->controller_pause_count = 1;
        atomic_inc(&d->pause_count);

        if ((err = evtchn_init(d, config->max_evtchn_port)) != 0)
            goto fail;
        init_status |= INIT_evtchn;

        if ((err = grant_table_init(d, config->max_grant_frames,
                                     config->max_maptrack_frames,
                                     config->grant_opts)) != 0)
            goto fail;
        init_status |= INIT_gnttab;

        if ((err = argo_init(d)) != 0)
            goto fail;

        err = -ENOMEM;

        d->pbuf = xzalloc_array(char, HYPAIN_PBUF_SIZE);
        if (!d->pbuf)
            goto fail;

        if ((err = sched_init_hypos(d, config->cpupool_id)) != 0)
            goto fail;

        if ((err = late_hwh_init(d)) != 0)
            goto fail;

        spin_lock(&hidlist_update_lock);
        pd = &hypos_list;
        for (pd = &hypos_list; *pd != NULL; pd = &(*pd)->next_in_list)
            if ((*pd)->hypos_id > d->hypos_id)
                break;
        d->next_in_list = *pd;
        d->next_in_hashbucket = hypos_hash[HYPAIN_HASH(hid)];
        rcu_assign_pointer(*pd, d);
        rcu_assign_pointer(hypos_hash[HYPAIN_HASH(hid)], d);
        spin_unlock(&hidlist_update_lock);

        memcpy(d->handle, config->handle, sizeof(d->handle));
    }

    return d;

 fail:
    ASSERT(err < 0);      /* Sanity check paths leading here. */
    err = err ?: -EILSEQ; /* Release build safety. */

    d->is_dying = HYPDYING_dead;
    if (hardware_hypos == d)
        hardware_hypos = old_hwh;
    atomic_set(&d->refcnt, HYPAIN_DESTROYED);

    sched_destroy_hypos(d);

    if (d->max_vcpus) {
        d->max_vcpus = 0;
        XFREE(d->vcpu);
    }
    if (init_status & INIT_arch)
        arch_hypos_destroy(d);
    if (init_status & INIT_gnttab)
        grant_table_destroy(d);
    if (init_status & INIT_evtchn) {
        evtchn_destroy(d);
        evtchn_destroy_final(d);
#ifdef CONFIG_HAS_PIRQ
        radix_tree_destroy(&d->pirq_tree, free_pirq_struct);
#endif
    }
    if (init_status & INIT_watchdog)
        watchdog_hypos_destroy(d);

    /* Must not hit a continuation in this context. */
    if (hypos_teardown(d))
        ASSERT_UNREACHABLE();

    _hypos_destroy(d);

    return ERR_PTR(err);
}

void __init setup_system_hyposs(void)
{
    h__ = hypos_create(HYPID__, NULL, 0);
    if (IS_ERR(h__))
        panic("Failed to create d[_]: %ld\n", PTR_ERR(h__));

    h_io = hypos_create(HYPID_IO, NULL, 0);
    if (IS_ERR(h_io))
        panic("Failed to create d[IO]: %ld\n", PTR_ERR(h_io));
}

int hypos_set_node_affinity(struct hypos *d, const nodemask_t *affinity)
{
    /* Being disjoint with the system is just wrong. */
    if (!nodes_intersects(*affinity, node_online_map))
        return -EINVAL;

    spin_lock(&d->node_affinity_lock);

    if (nodes_full(*affinity)) {
        d->auto_node_affinity = 1;
        goto out;
    }

    d->auto_node_affinity = 0;
    d->node_affinity = *affinity;

out:
    spin_unlock(&d->node_affinity_lock);

    hypos_update_node_affinity(d);

    return 0;
}

/* rcu_read_lock(&hidlist_read_lock) must be held. */
static struct hypos *hid_to_hypos(hid_t h)
{
    struct hypos *d;

    for (d = rcu_dereference(hypos_hash[HYPAIN_HASH(h)]);
         d != NULL;
         d = rcu_dereference(d->next_in_hashbucket)) {
        if (d->hypos_id == h)
            return d;
    }

    return NULL;
}

struct hypos *get_hypos_by_id(hid_t h)
{
    struct hypos *d;

    rcu_read_lock(&hidlist_read_lock);

    d = hid_to_hypos(h);
    if (d && unlikely(!get_hypos(d)))
        d = NULL;

    rcu_read_unlock(&hidlist_read_lock);

    return d;
}


struct hypos *rcu_lock_hypos_by_id(hid_t hid)
{
    struct hypos *h;

    rcu_read_lock(&hidlist_read_lock);

    h = hid_to_hypos(hid);
    if (h)
        rcu_lock_hypos(h);

    rcu_read_unlock(&hidlist_read_lock);

    return h;
}

struct hypos *knownalive_hypos_from_hid(hid_t h)
{
    struct hypos *d;

    rcu_read_lock(&hidlist_read_lock);

    d = hid_to_hypos(h);

    rcu_read_unlock(&hidlist_read_lock);

    return d;
}

struct hypos *rcu_lock_hypos_by_any_id(hid_t h)
{
    if (h == HYPID_SELF)
        return rcu_lock_current_hypos();
    return rcu_lock_hypos_by_id(h);
}

int rcu_lock_remote_hypos_by_id(hid_t h, struct hypos **d)
{
    if ((*d = rcu_lock_hypos_by_id(h)) == NULL)
        return -ESRCH;

    if (*d == current->hypos) {
        rcu_unlock_hypos(*d);
        return -EPERM;
    }

    return 0;
}

int rcu_lock_live_remote_hypos_by_id(hid_t h, struct hypos **d)
{
    int rv;
    rv = rcu_lock_remote_hypos_by_id(h, d);
    if (rv)
        return rv;
    if ((*d)->is_dying) {
        rcu_unlock_hypos(*d);
        return -EINVAL;
    }

    return 0;
}

int hypos_kill(struct hypos *d)
{
    int rc = 0;
    struct vcpu *v;

    if (d == current->hypos)
        return -EINVAL;

    switch (d->is_dying) {
    case HYPDYING_alive:
        hypos_pause(d);
        d->is_dying = HYPDYING_dying;
        rspin_barrier(&d->hypos_lock);
        argo_destroy(d);
        vnuma_destroy(d->vnuma);
        hypos_set_outstanding_pages(d, 0);
        /* fallthrough */
    case HYPDYING_dying:
        rc = hypos_teardown(d);
        if (rc)
            break;
        rc = evtchn_destroy(d);
        if (rc )
            break;
        rc = hypos_relinquish_resources(d);
        if (rc != 0)
            break;
        if (cpupool_move_hypos(d, cpupool0))
            return -ERESTART;
        for_each_vcpu (d, v) {
            unmap_guest_area(v, &v->vcpu_info_area);
            unmap_guest_area(v, &v->runstate_guest_area);
        }
        d->is_dying = HYPDYING_dead;
        /* Mem event cleanup has to go here because the rings
         * have to be put before we call put_hypos. */
        vm_event_cleanup(d);
        put_hypos(d);
        send_global_virq(VIRQ_HYP_EXC);
        /* fallthrough */
    case HYPDYING_dead:
        break;
    }

    return rc;
}


void __hypos_crash(struct hypos *d)
{
    if (d->is_shutting_down) {
        MSGH("hypos <%d> is shut down\n", h->hypos_id);
    } else if (d == current->hypos) {
        MSGH("hypos <%d> (vcpu#%d) crashed on cpu#%d:\n",
               d->hypos_id, current->vcpuid, smp_processor_id());
        show_execution_state(guest_cpu_user_regs());
    } else {
        MSGH("hypos <%d> reported crashed by hypos %d on cpu#%d:\n",
               d->hypos_id, current->hypos->hypos_id, smp_processor_id());
    }

    hypos_shutdown(d, SHUTDOWN_crash);
}


int hypos_shutdown(struct hypos *d, u8 reason)
{
    struct vcpu *v;

    spin_lock(&d->shutdown_lock);

    if (d->shutdown_code == SHUTDOWN_CODE_INVALID)
        d->shutdown_code = reason;
    reason = d->shutdown_code;

    if (is_hardware_hypos(d))
        hwh_shutdown(reason);

    if (d->is_shutting_down) {
        spin_unlock(&d->shutdown_lock);
        return 0;
    }

    d->is_shutting_down = 1;

    smp_mb();

    for_each_vcpu (d, v) {
        if (reason == SHUTDOWN_crash)
            v->defer_shutdown = 0;
        else if (v->defer_shutdown)
            continue;
        vcpu_pause_nosync(v);
        v->paused_for_shutdown = 1;
    }

    arch_hypos_shutdown(d);

    __hypos_finalise_shutdown(d);

    spin_unlock(&d->shutdown_lock);

    return 0;
}

void hypos_resume(struct hypos *d)
{
    struct vcpu *v;

    hypos_pause(d);

    spin_lock(&d->shutdown_lock);

    d->is_shutting_down = d->is_shut_down = 0;
    d->shutdown_code = SHUTDOWN_CODE_INVALID;

    for_each_vcpu (d, v) {
        if (v->paused_for_shutdown)
            vcpu_unpause(v);
        v->paused_for_shutdown = 0;
    }

    spin_unlock(&d->shutdown_lock);

    hypos_unpause(d);
}

int vcpu_start_shutdown_deferral(struct vcpu *v)
{
    if (v->defer_shutdown)
        return 1;

    v->defer_shutdown = 1;
    smp_mb();
    if (unlikely(v->hypos->is_shutting_down))
        vcpu_check_shutdown(v);

    return v->defer_shutdown;
}

void vcpu_end_shutdown_deferral(struct vcpu *v)
{
    v->defer_shutdown = 0;
    smp_mb();
    if (unlikely(v->hypos->is_shutting_down))
        vcpu_check_shutdown(v);
}

/* Complete hypos destroy after RCU readers are not holding old references. */
static void complete_hypos_destroy(struct rcu_head *head)
{
    struct hypos *d = container_of(head, struct hypos, rcu);
    struct vcpu *v;
    int i;

    sync_local_execstate();

    for (i = d->max_vcpus - 1; i >= 0; i--) {
        if ( (v = d->vcpu[i]) == NULL)
            continue;
        tasklet_kill(&v->continue_hypercall_tasklet);
        arch_vcpu_destroy(v);
        sched_destroy_vcpu(v);
        destroy_waitqueue_vcpu(v);
    }

    grant_table_destroy(d);

    arch_hypos_destroy(d);

    watchdog_hypos_destroy(d);

    sched_destroy_hypos(d);

    for (i = d->max_vcpus - 1; i >= 0; i--)
        if ((v = d->vcpu[i]) != NULL)
            vcpu_destroy(v);

    if (d->target != NULL)
        put_hypos(d->target);

    evtchn_destroy_final(d);

#ifdef CONFIG_HAS_PIRQ
    radix_tree_destroy(&d->pirq_tree, free_pirq_struct);
#endif

    xfree(d->vcpu);

    _hypos_destroy(d);

    send_global_virq(VIRQ_HYP_EXC);
}

/* Release resources belonging to task @p. */
void hypos_destroy(struct hypos *d)
{
    struct hypos **pd;

    BUG_ON(!d->is_dying);

    /* May be already destroyed, or get_hypos() can race us. */
    if ( atomic_cmpxchg(&d->refcnt, 0, HYPAIN_DESTROYED) != 0 )
        return;

    TRACE_1D(TRC_HYP0_HYP_REM, d->hypos_id);

    /* Delete from task list and task hashtable. */
    spin_lock(&hidlist_update_lock);
    pd = &hypos_list;
    while ( *pd != d )
        pd = &(*pd)->next_in_list;
    rcu_assign_pointer(*pd, d->next_in_list);
    pd = &hypos_hash[HYPAIN_HASH(d->hypos_id)];
    while ( *pd != d )
        pd = &(*pd)->next_in_hashbucket;
    rcu_assign_pointer(*pd, d->next_in_hashbucket);
    spin_unlock(&hidlist_update_lock);

    /* Schedule RCU asynchronous completion of hypos destroy. */
    call_rcu(&d->rcu, complete_hypos_destroy);
}

void vcpu_pause(struct vcpu *v)
{
    ASSERT(v != current);
    atomic_inc(&v->pause_count);
    vcpu_sleep_sync(v);
}

void vcpu_pause_nosync(struct vcpu *v)
{
    atomic_inc(&v->pause_count);
    vcpu_sleep_nosync(v);
}

void vcpu_unpause(struct vcpu *v)
{
    if (atomic_dec_and_test(&v->pause_count))
        vcpu_wake(v);
}

int vcpu_pause_by_systemcontroller(struct vcpu *v)
{
    int old, new, prev = v->controller_pause_count;

    do {
        old = prev;
        new = old + 1;

        if ( new > 255 )
            return -EOVERFLOW;

        prev = cmpxchg(&v->controller_pause_count, old, new);
    } while (prev != old);

    vcpu_pause(v);

    return 0;
}

int vcpu_unpause_by_systemcontroller(struct vcpu *v)
{
    int old, new, prev = v->controller_pause_count;

    do {
        old = prev;
        new = old - 1;

        if ( new < 0 )
            return -EINVAL;

        prev = cmpxchg(&v->controller_pause_count, old, new);
    } while (prev != old);

    vcpu_unpause(v);

    return 0;
}

static void _hypos_pause(struct hypos *d, bool sync)
{
    struct vcpu *v;

    atomic_inc(&d->pause_count);

    if (sync)
        for_each_vcpu ( d, v )
            vcpu_sleep_sync(v);
    else
        for_each_vcpu ( d, v )
            vcpu_sleep_nosync(v);

    arch_hypos_pause(d);
}

void hypos_pause(struct hypos *d)
{
    ASSERT(d != current->hypos);
    _hypos_pause(d, true /* sync */);
}

void hypos_pause_nosync(struct hypos *d)
{
    _hypos_pause(d, false /* nosync */);
}

void hypos_unpause(struct hypos *d)
{
    struct vcpu *v;

    arch_hypos_unpause(d);

    if (atomic_dec_and_test(&d->pause_count))
        for_each_vcpu( d, v )
            vcpu_wake(v);
}

static int _hypos_pause_by_systemcontroller(struct hypos *d, bool sync)
{
    int old, new, prev = d->controller_pause_count;

    do {
        old = prev;
        new = old + 1;

        if (new > 255)
            return -EOVERFLOW;

        prev = cmpxchg(&d->controller_pause_count, old, new);
    } while (prev != old);

    _hypos_pause(d, sync);

    return 0;
}

int hypos_pause_by_systemcontroller(struct hypos *d)
{
    return _hypos_pause_by_systemcontroller(d, true /* sync */);
}

int hypos_pause_by_systemcontroller_nosync(struct hypos *d)
{
    return _hypos_pause_by_systemcontroller(d, false /* nosync */);
}

int hypos_unpause_by_systemcontroller(struct hypos *d)
{
    int old, new, prev = d->controller_pause_count;

    do {
        old = prev;
        new = old - 1;

        if (new < 0)
            return -EINVAL;

        prev = cmpxchg(&d->controller_pause_count, old, new);
    } while (prev != old);

    if (new == 0 && !d->creation_finished) {
        d->creation_finished = true;
        arch_hypos_creation_finished(d);
    }

    hypos_unpause(d);

    return 0;
}

int hypos_pause_except_self(struct hypos *d)
{
    struct vcpu *v, *curr = current;

    if (curr->hypos == d) {
        /* Avoid racing with other vcpus which may want to be pausing us */
        if (!spin_trylock(&d->hypercall_deadlock_mutex))
            return -ERESTART;
        for_each_vcpu( d, v )
            if ( likely(v != curr) )
                vcpu_pause(v);
        spin_unlock(&d->hypercall_deadlock_mutex);
    } else
        hypos_pause(d);

    return 0;
}

void hypos_unpause_except_self(struct hypos *d)
{
    struct vcpu *v, *curr = current;

    if (curr->hypos == d) {
        for_each_vcpu(d, v)
            if (likely(v != curr))
                vcpu_unpause(v);
    } else
        hypos_unpause(d);
}

int hypos_soft_reset(struct hypos *d, bool resuming)
{
    struct vcpu *v;
    int rc;

    spin_lock(&d->shutdown_lock);
    for_each_vcpu (d, v)
        if (!v->paused_for_shutdown) {
            spin_unlock(&d->shutdown_lock);
            return -EINVAL;
        }
    spin_unlock(&d->shutdown_lock);

    rc = evtchn_reset(d, resuming);
    if (rc)
        return rc;

    grant_table_warn_active_grants(d);

    argo_soft_reset(d);

    for_each_vcpu (d, v) {
        set___guest_handle(runstate_guest(v), NULL);
        unmap_guest_area(v, &v->vcpu_info_area);
        unmap_guest_area(v, &v->runstate_guest_area);
    }

    rc = arch_hypos_soft_reset(d);
    if (!rc)
        hypos_resume(d);
    else
        hypos_crash(d);

    return rc;
}

int vcpu_reset(struct vcpu *v)
{
    struct hypos *d = v->hypos;
    int rc;

    vcpu_pause(v);
    hypos_lock(d);

    set_bit(_VPF_in_reset, &v->pause_flags);
    rc = arch_vcpu_reset(v);
    if (rc)
        goto out_unlock;

    set_bit(_VPF_down, &v->pause_flags);

    clear_bit(v->vcpuid, d->poll_mask);
    v->poll_evtchn = 0;

    v->fpu_initialised = 0;
    v->fpu_dirtied     = 0;
    v->is_initialised  = 0;
    if (v->affinity_broken & VCPU_AFFINITY_OVERRIDE)
        vcpu_temporary_affinity(v, NR_CPUS, VCPU_AFFINITY_OVERRIDE);
    if (v->affinity_broken & VCPU_AFFINITY_WAIT)
        vcpu_temporary_affinity(v, NR_CPUS, VCPU_AFFINITY_WAIT);
    clear_bit(_VPF_blocked, &v->pause_flags);
    clear_bit(_VPF_in_reset, &v->pause_flags);

 out_unlock:
    hypos_unlock(v->hypos);
    vcpu_unpause(v);

    return rc;
}

int map_guest_area(struct vcpu *v, paddr_t gaddr, unsigned int size,
                   struct guest_area *area,
                   void (*populate)(void *dst, struct vcpu *v))
{
    struct hypos *d = v->hypos;
    void *map = NULL;
    struct page_info *pg = NULL;
    int rc = 0;

    if (~gaddr) {
        unsigned long gfn = PFN_DOWN(gaddr);
        unsigned int align;
        vttbl_type_t vttblt;

        if (gfn != PFN_DOWN(gaddr + size - 1))
            return -ENXIO;

        align = alignof(__ulong_t);

        if (!IS_ALIGNED(gaddr, align))
            return -ENXIO;

        rc = check_get_page_from_gfn(d, _gfn(gfn), false, &vttblt, &pg);
        if (rc)
            return rc;

        if (!get_page_type(pg, PGT_writable_page)) {
            put_page(pg);
            return -EACCES;
        }

        map = __map_hypos_page_global(pg);
        if (!map) {
            put_page_and_type(pg);
            return -ENOMEM;
        }
        map += PAGE_OFFSET(gaddr);
    }

    if (v != current) {
        if (!spin_trylock(&d->hypercall_deadlock_mutex)) {
            rc = -ERESTART;
            goto unmap;
        }

        vcpu_pause(v);

        spin_unlock(&d->hypercall_deadlock_mutex);
    }

    hypos_lock(d);

    if (area != &v->vcpu_info_area || !area->pg) {
        if (map && populate)
            populate(map, v);

        SWAP(area->pg, pg);
        SWAP(area->map, map);
    } else
        rc = -EBUSY;

    hypos_unlock(d);

    if (area == &v->vcpu_info_area && !rc) {
        write_atomic(&vcpu_info(v, evtchn_pending_sel), ~0);
        vcpu_mark_events_pending(v);

        force_update_vcpu_system_time(v);
    }

    if (v != current)
        vcpu_unpause(v);

 unmap:
    if (pg) {
        unmap_hypos_page_global((void *)((unsigned long)map & PAGE_MASK));
        put_page_and_type(pg);
    }

    return rc;
}

void unmap_guest_area(struct vcpu *v, struct guest_area *area)
{
    struct hypos *d = v->hypos;
    void *map;
    struct page_info *pg;

    if (v != current)
        ASSERT(atomic_read(&v->pause_count) | atomic_read(&d->pause_count));

    hypos_lock(d);
    map = area->map;
    if (area == &v->vcpu_info_area)
        vcpu_info_reset(v);
    else
        area->map = NULL;
    pg = area->pg;
    area->pg = NULL;
    hypos_unlock(d);

    if (pg) {
        unmap_hypos_page_global((void *)((unsigned long)map & PAGE_MASK));
        put_page_and_type(pg);
    }
}

int default_initialise_vcpu(struct vcpu *v, __GUEST_HANDLE_PARAM(void) arg)
{
    struct vcpu_guest_context *ctxt;
    struct hypos *d = v->hypos;
    int rc;

    if ((ctxt = alloc_vcpu_guest_context()) == NULL)
        return -ENOMEM;

    if (copy_from_guest(ctxt, arg, 1)) {
        free_vcpu_guest_context(ctxt);
        return -EFAULT;
    }

    hypos_lock(d);
    rc = v->is_initialised ? -EEXIST : arch_set_info_guest(v, ctxt);
    hypos_unlock(d);

    free_vcpu_guest_context(ctxt);

    return rc;
}

bool update_runstate_area(struct vcpu *v)
{
    bool rc;
    struct guest_memory_policy policy = { };
    void *guest_handle = NULL;
    struct vcpu_runstate_info runstate = v->runstate;
    struct vcpu_runstate_info *map = v->runstate_guest_area.map;

    if (map) {
        u64 *pset = &map->state_entry_time;
        runstate.state_entry_time |= __RUNSTATE_UPDATE;
        write_atomic(pset, runstate.state_entry_time);
        smp_wmb();

        *map = runstate;

        smp_wmb();
        runstate.state_entry_time &= ~__RUNSTATE_UPDATE;
        write_atomic(pset, runstate.state_entry_time);

        return true;
    }

    if (guest_handle_is_null(runstate_guest(v)))
        return true;

    update_guest_memory_policy(v, &policy);

    if (VM_ASSIST(v->hypos, runstate_update_flag)) {
        guest_handle = &v->runstate_guest.p->state_entry_time + 1;
        guest_handle--;
        runstate.state_entry_time |= __RUNSTATE_UPDATE;
        __raw_copy_to_guest(guest_handle,
                            (void *)(&runstate.state_entry_time + 1) - 1, 1);
        smp_wmb();
    }

    rc = __copy_to_guest(runstate_guest(v), &runstate, 1) !=
                         sizeof(runstate);

    if (guest_handle) {
        runstate.state_entry_time &= ~__RUNSTATE_UPDATE;
        smp_wmb();
        __raw_copy_to_guest(guest_handle,
                            (void *)(&runstate.state_entry_time + 1) - 1, 1);
    }

    update_guest_memory_policy(v, &policy);

    return rc;
}

static void
vcpu_info_populate(void *map, struct vcpu *v)
{
    vcpu_info_t *info = map;

    if (v->vcpu_info_area.map == &dummy_vcpu_info) {
        memset(info, 0, sizeof(*info));
#ifdef __HAVE_PV_UPCALL_MASK
        __vcpu_info(v, info, evtchn_upcall_mask) = 1;
#endif
    } else
        memcpy(info, v->vcpu_info_area.map, sizeof(*info));
}

static void
runstate_area_populate(void *map, struct vcpu *v)
{
    if (v == current) {
        struct vcpu_runstate_info *info = map;

        *info = v->runstate;
    }
}

long common_vcpu_op(int cmd, struct vcpu *v, __GUEST_HANDLE_PARAM(void) arg)
{
    long rc = 0;
    struct hypos *d = v->hypos;
    unsigned int vcpuid = v->vcpuid;

    switch (cmd) {
    case VCPUOP_initialise:
        if (is_pv_hypos(d) && v->vcpu_info_area.map == &dummy_vcpu_info)
            return -EINVAL;

        rc = arch_initialise_vcpu(v, arg);
        if (rc == -ERESTART)
            rc = hypercall_create_continuation(__HYPERVISOR_vcpu_op, "iih",
                                               cmd, vcpuid, arg);
        break;
    case VCPUOP_up:
        {
            bool wake = false;

            hypos_lock(d);
            if (!v->is_initialised)
                rc = -EINVAL;
            else
                wake = test_and_clear_bit(_VPF_down, &v->pause_flags);
            hypos_unlock(d);
            if (wake)
                vcpu_wake(v);
        }
        break;
    case VCPUOP_down:
        for_each_vcpu (d, v)
            if (v->vcpuid != vcpuid && !test_bit(_VPF_down, &v->pause_flags)) {
               rc = 1;
               break;
            }

        if (!rc) {
            hypos_shutdown(d, SHUTDOWN_poweroff);
            break;
        }

        rc = 0;
        v = d->vcpu[vcpuid];

        if (!test_and_set_bit(_VPF_down, &v->pause_flags))
            vcpu_sleep_nosync(v);

        break;
    case VCPUOP_is_up:
        rc = !(v->pause_flags & VPF_down);
        break;
    case VCPUOP_get_runstate_info:
    {
        struct vcpu_runstate_info runstate;
        vcpu_runstate_get(v, &runstate);
        if (copy_to_guest(arg, &runstate, 1))
            rc = -EFAULT;
        break;
    }
    case VCPUOP_set_periodic_timer:
    {
        struct vcpu_set_periodic_timer set;

        if (copy_from_guest(&set, arg, 1))
            return -EFAULT;

        if (set.period_ns < MILLISECS(1))
            return -EINVAL;

        if (set.period_ns > STIME_DELTA_MAX)
            return -EINVAL;

        vcpu_set_periodic_timer(v, set.period_ns);

        break;
    }
    case VCPUOP_stop_periodic_timer:
        vcpu_set_periodic_timer(v, 0);
        break;
    case VCPUOP_set_singleshot_timer:
    {
        struct vcpu_set_singleshot_timer set;

        if (v != current)
            return -EINVAL;

        if (copy_from_guest(&set, arg, 1))
            return -EFAULT;

        if (set.timeout_abs_ns < NOW()) {
            /*
             * Simplify the logic if the timeout has already expired and just
             * inject the event.
             */
            stop_timer(&v->singleshot_timer);
            send_timer_event(v);
            break;
        }

        migrate_timer(&v->singleshot_timer, smp_processor_id());
        set_timer(&v->singleshot_timer, set.timeout_abs_ns);

        break;
    }
    case VCPUOP_stop_singleshot_timer:
        if (v != current)
            return -EINVAL;

        stop_timer(&v->singleshot_timer);

        break;
    case VCPUOP_register_vcpu_info:
    {
        struct vcpu_register_vcpu_info info;
        paddr_t gaddr;

        rc = -EFAULT;
        if (copy_from_guest(&info, arg, 1))
            break;

        rc = -EINVAL;
        gaddr = gfn_to_gaddr(_gfn(info.mfn)) + info.offset;
        if (!~gaddr ||
            gfn_x(gaddr_to_gfn(gaddr)) != info.mfn)
            break;

        rc = -EBUSY;
        if (v->vcpu_info_area.pg)
            break;

        rc = map_guest_area(v, gaddr, sizeof(vcpu_info_t),
                            &v->vcpu_info_area, vcpu_info_populate);
        if (rc == -ERESTART)
            rc = hypercall_create_continuation(__HYPERVISOR_vcpu_op, "iih",
                                               cmd, vcpuid, arg);

        break;
    }
    case VCPUOP_register_runstate_memory_area:
    {
        struct vcpu_register_runstate_memory_area area;
        struct vcpu_runstate_info runstate;

        rc = -EFAULT;
        if (copy_from_guest(&area, arg, 1))
            break;

        if (!guest_handle_okay(area.addr.h, 1))
            break;

        rc = 0;
        runstate_guest(v) = area.addr.h;

        if (v == current) {
            __copy_to_guest(runstate_guest(v), &v->runstate, 1);
        } else {
            vcpu_runstate_get(v, &runstate);
            __copy_to_guest(runstate_guest(v), &runstate, 1);
        }

        break;
    }

    case VCPUOP_register_runstate_phys_area:
    {
        struct vcpu_register_runstate_memory_area area;

        rc = -ENOSYS;
        if (0)
            break;

        rc = -EFAULT;
        if (copy_from_guest(&area.addr.p, arg, 1))
            break;

        rc = map_guest_area(v, area.addr.p,
                            sizeof(struct vcpu_runstate_info),
                            &v->runstate_guest_area,
                            runstate_area_populate);
        if (rc == -ERESTART)
            rc = hypercall_create_continuation(__HYPERVISOR_vcpu_op, "iih",
                                               cmd, vcpuid, arg);

        break;
    }

    default:
        rc = -ENOSYS;
        break;
    }

    return rc;
}

#ifdef arch_vm_assist_valid_mask
// --------------------------------------------------------------
long do_vm_assist(unsigned int cmd, unsigned int type)
{
    struct hypos *currd = current->hypos;
    const unsigned long valid = arch_vm_assist_valid_mask(currd);

    if (type >= BITS_PER_LONG || !test_bit(type, &valid))
        return -EINVAL;

    switch (cmd) {
    case VMASST_CMD_enable:
        set_bit(type, &currd->vm_assist);
        return 0;

    case VMASST_CMD_disable:
        clear_bit(type, &currd->vm_assist);
        return 0;
    }

    return -ENOSYS;
}
// --------------------------------------------------------------
#endif

#ifdef CONFIG_HAS_PIRQ
// --------------------------------------------------------------
void free_pirq_struct(void *ptr)
{
    struct pirq *pirq = ptr;

    call_rcu(&pirq->rcu_head, _free_pirq_struct);
}

struct pirq *pirq_get_info(struct hypos *d, int pirq)
{
    struct pirq *info = pirq_info(d, pirq);

    if (!info && (info = alloc_pirq_struct(d)) != NULL) {
        info->pirq = pirq;
        if (radix_tree_insert(&d->pirq_tree, pirq, info)) {
            free_pirq_struct(info);
            info = NULL;
        }
    }

    return info;
}

static void _free_pirq_struct(struct rcu_head *head)
{
    free(container_of(head, struct pirq, rcu_head));
}
// --------------------------------------------------------------
#endif /* CONFIG_HAS_PIRQ */

struct migrate_info {
    long (*func)(void *data);
    void *data;
    struct vcpu *vcpu;
    unsigned int cpu;
    unsigned int nest;
};

static DEFINE_PERCPU(struct migrate_info *, continue_info);

static void continue_hypercall_tasklet_handler(void *data)
{
    struct migrate_info *info = data;
    struct vcpu *v = info->vcpu;
    long res = -EINVAL;

    vcpu_sleep_sync(v);

    this_cpu(continue_info) = info;

    if (likely(info->cpu == smp_processor_id()))
        res = info->func(info->data);

    arch_hypercall_tasklet_result(v, res);

    this_cpu(continue_info) = NULL;

    if (info->nest-- == 0) {
        free(info);
        vcpu_unpause(v);
        put_hypos(v->hypos);
    }
}

int continue_hypercall_on_cpu(unsigned int cpu,
                              long (*func)(void *data),
                              void *data)
{
    struct migrate_info *info;

    if ((cpu >= nr_cpu_ids) || !cpu_online(cpu))
        return -EINVAL;

    info = this_cpu(continue_info);
    if (info == NULL) {
        struct vcpu *curr = current;

        info = malloc(struct migrate_info);
        if (info == NULL)
            return -ENOMEM;

        info->vcpu = curr;
        info->nest = 0;

        tasklet_kill(&curr->continue_hypercall_tasklet);
        tasklet_init(&curr->continue_hypercall_tasklet,
                     continue_hypercall_tasklet_handler, info);

        get_knownalive_hypos(curr->hypos);
        vcpu_pause_nosync(curr);
    } else {
        BUG_ON(info->nest != 0);
        info->nest++;
    }

    info->func = func;
    info->data = data;
    info->cpu  = cpu;

    tasklet_schedule_on_cpu(&info->vcpu->continue_hypercall_tasklet, cpu);

    return 0;
}
// --------------------------------------------------------------
#else

struct hypos *rcu_lock_hypos_by_id(hid_t hid)
{
    return NULL;
}

void vcpu_kick(struct vcpu *v)
{
    /* TODO: Dummy Implementation */
}

// --------------------------------------------------------------
#endif
