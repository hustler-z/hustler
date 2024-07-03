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
#include <asm-generic/smp.h>
#include <asm-generic/section.h>
#include <common/errno.h>
#include <common/softirq.h>
#include <common/timer.h>
#include <bsp/debug.h>
#include <bsp/panic.h>
#include <lib/math.h>
#include <core/vcpu.h>

unsigned int __read_mostly nr_cpu_ids = NR_CPUS;
extern struct boot_setup boot_setup;
extern struct vcpu *hypos_vcpus[NR_CPUS];

static bool cpu_is_dead;

cpumask_t cpu_online_map;
cpumask_t cpu_present_map;
cpumask_t cpu_possible_map;

register_t cpu_logical_map[NR_CPUS] = {[0 ... NR_CPUS-1] = MPIDR_INVALID};

int prep_smp_mm(int cpu)
{
    return 0;
}

int arch_cpu_up(int cpu)
{
    return 0;
}

static void set_smp_cpu_up(unsigned long mpidr)
{

}

/* Bring up a remote CPU */
int __cpu_up(unsigned int cpu)
{
    int rc;

    MSGH("Bringing up CPU%d\n", cpu);

    rc = prep_smp_mm(cpu);
    if (rc < 0)
        return rc;

    boot_setup.stack = hypos_vcpus[cpu]->arch.stack;

    boot_setup.cpuid = cpu;

    set_smp_cpu_up(cpu_logical_map[cpu]);

    rc = arch_cpu_up(cpu);

    if (rc < 0) {
        MSGH("Failed to bring up CPU%d\n", cpu);

        return rc;
    }

    while (!cpu_online(cpu))
    {
        barrier();
        process_pending_softirqs();
    }

    smp_rmb();

    boot_setup.stack = NULL;
    boot_setup.cpuid = ~0;

    set_smp_cpu_up(MPIDR_INVALID);

    if (!cpu_online(cpu)) {
        MSGH("CPU%d never came online\n", cpu);

        return -EIO;
    }

    return 0;
}

/* Wait for a remote CPU to die */
void __cpu_die(unsigned int cpu)
{
    unsigned int i = 0;

    while (!cpu_is_dead) {
        mdelay(100);

        barrier();

        process_pending_softirqs();

        if ((++i % 10) == 0)
            MSGH("CPU %u still not dead...\n", cpu);
        smp_mb();
    }

    cpu_is_dead = false;

    smp_mb();
}

int cpu_down(unsigned int cpu)
{
    int err;

    err = -EINVAL;
    if ((cpu >= nr_cpu_ids) || (cpu == 0))
        goto out;

    err = -EEXIST;
    if (!cpu_online(cpu))
        goto out;


    __cpu_die(cpu);
    err = cpu_online(cpu);
    BUG_ON(err);

    return 0;

out:
    return err;
}

int cpu_up(unsigned int cpu)
{
    int err;

    err = -EINVAL;
    if ((cpu >= nr_cpu_ids) || !cpu_present(cpu))
        goto out;

    err = -EEXIST;
    if (cpu_online(cpu))
        goto out;


    err = __cpu_up(cpu);
    if ( err < 0 )
        goto out;

    return 0;

out:
    return err;
}

static void __bootfunc smp_prep_cpus(void)
{

}

int __bootfunc smp_setup(void)
{
    int i, ret = -1;

    smp_prep_cpus();

    for_each_present_cpu(i) {
        if ((num_online_cpus() < nr_cpu_ids) &&
                !cpu_online(i)) {
            ret = cpu_up(i);

            if (ret)
                MSGH("Failed to bring up CPU %u [error %d]\n",
                        i, ret);
        }
    }

    MSGH("Brought up %ld CPUs\n", (long)num_online_cpus());

    return ret;
}
