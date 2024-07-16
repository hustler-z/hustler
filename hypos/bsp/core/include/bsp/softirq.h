/**
 * Hustler's Project
 *
 * File:  softirq.h
 * Date:  2024/06/27
 * Usage:
 */

#ifndef _BSP_SOFTIRQ_H
#define _BSP_SOFTIRQ_H
// ------------------------------------------------------------------------
#include <org/smp.h>

enum {
    TIMER_SOFTIRQ = 0,
    RCU_SOFTIRQ,
    SCHED_SLAVE_SOFTIRQ,
    SCHEDULE_SOFTIRQ,
    NEW_TLBFLUSH_CLOCK_PERIOD_SOFTIRQ,
    TASKLET_SOFTIRQ,
    NR_COMMON_SOFTIRQS
};

#define NR_SOFTIRQS (NR_COMMON_SOFTIRQS)

typedef void (*softirq_handler)(void);
void do_softirq(void);
void open_softirq(int nr, softirq_handler handler);
void cpumask_raise_softirq(const cpumask_t *mask, unsigned int nr);
void cpu_raise_softirq(unsigned int cpu, unsigned int nr);
void raise_softirq(unsigned int nr);

void process_pending_softirqs(void);

// ------------------------------------------------------------------------
#endif /* _BSP_SOFTIRQ_H */
