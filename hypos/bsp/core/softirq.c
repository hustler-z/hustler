/**
 * Hustler's Project
 *
 * File:  softirq.c
 * Date:  2024/06/27
 * Usage:
 */

#include <org/bitops.h>
#include <org/irq.h>
#include <org/smp.h>
#include <asm/bitops.h>
#include <asm/barrier.h>
#include <bsp/softirq.h>
#include <bsp/preempt.h>
#include <bsp/percpu.h>
#include <bsp/panic.h>
#include <bsp/config.h>
#include <bsp/rcu.h>

// --------------------------------------------------------------

/* SoftIRQ Implementation
 *
 *
 */

// --------------------------------------------------------------
#if IS_IMPLEMENTED(__SOFTIRQ_IMPL)

static softirq_handler softirq_handlers[NR_SOFTIRQS];

static DEFINE_PERCPU(cpumask_t, batch_mask);
static DEFINE_PERCPU(unsigned int, batching);

static void __do_softirq(unsigned long ignore_mask)
{
    unsigned int i, cpu;
    unsigned long pending;
    bool rcu_allowed = !(ignore_mask & (1UL << RCU_SOFTIRQ));

    ASSERT(!rcu_allowed || rcu_quiesce_allowed());

    for (;;) {
        cpu = smp_processor_id();

        if (rcu_allowed && rcu_pending(cpu))
            rcu_check_callbacks(cpu);

        if (((pending = (softirq_pending(cpu)
            & ~ignore_mask)) == 0)
            || cpu_is_offline(cpu))
            break;

        i = find_first_set_bit(pending);
        clear_bit(i, &softirq_pending(cpu));
        (*softirq_handlers[i])();
    }
}

void process_pending_softirqs(void)
{
    unsigned long ignore_mask = (1UL << SCHEDULE_SOFTIRQ) |
                                (1UL << SCHED_SLAVE_SOFTIRQ);

    if (!rcu_quiesce_allowed())
        ignore_mask |= 1UL << RCU_SOFTIRQ;

    ASSERT(!in_irq() && local_irq_is_enabled());
    __do_softirq(ignore_mask);
}

void do_softirq(void)
{
    ASSERT_NOT_IN_ATOMIC();
    __do_softirq(0);
}

void open_softirq(int nr, softirq_handler handler)
{
    ASSERT(nr < NR_SOFTIRQS);
    softirq_handlers[nr] = handler;
}

void cpumask_raise_softirq(const cpumask_t *mask, unsigned int nr)
{
    unsigned int cpu, this_cpu = smp_processor_id();
    cpumask_t send_mask, *raise_mask;

    if (!percpu(batching, this_cpu) || in_irq()) {
        cpumask_clear(&send_mask);
        raise_mask = &send_mask;
    } else
        raise_mask = &percpu(batch_mask, this_cpu);

    for_each_cpu(cpu, mask)
        if (!test_and_set_bit(nr, &softirq_pending(cpu))
            && cpu != this_cpu)
            cpumask_set_cpu(cpu, raise_mask);

    if (raise_mask == &send_mask)
        smp_send_event_check_mask(raise_mask);
}

void cpu_raise_softirq(unsigned int cpu, unsigned int nr)
{
    unsigned int this_cpu = smp_processor_id();

    if (test_and_set_bit(nr, &softirq_pending(cpu))
        || (cpu == this_cpu))
        return;

    if (!percpu(batching, this_cpu) || in_irq())
        smp_send_event_check_cpu(cpu);
    else
        cpumask_set_cpu(cpu, &percpu(batch_mask, this_cpu));
}

void cpu_raise_softirq_batch_begin(void)
{
    ++this_cpu(batching);
}

void cpu_raise_softirq_batch_finish(void)
{
    unsigned int cpu, this_cpu = smp_processor_id();
    cpumask_t *mask = &percpu(batch_mask, this_cpu);

    ASSERT(percpu(batching, this_cpu));
    for_each_cpu (cpu, mask)
        if (!softirq_pending(cpu))
            cpumask_clear_cpu(cpu, mask);
    smp_send_event_check_mask(mask);
    cpumask_clear(mask);
    --percpu(batching, this_cpu);
}

void raise_softirq(unsigned int nr)
{
    set_bit(nr, &softirq_pending(smp_processor_id()));
}
// --------------------------------------------------------------
#endif
