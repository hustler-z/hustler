/**
 * Hustler's Project
 *
 * File:  time.h
 * Date:  2024/07/10
 * Usage:
 */

#ifndef _ORG_TIME_H
#define _ORG_TIME_H
// --------------------------------------------------------------
#include <org/bitops.h>
#include <asm/sysregs.h>
#include <asm/barrier.h>
#include <bsp/type.h>
#include <bsp/config.h>

/* Time counter hypervisor control register
 */

/* Kernel/user access to physical counter */
#define CNTHCTL_EL2_EL1PCTEN (1U << 0)
/* Kernel/user access to CNTP timer regs */
#define CNTHCTL_EL2_EL1PCEN  (1U << 1)

/* Time counter kernel control register */
/* Expose phys counters to EL0 */
#define CNTKCTL_EL1_EL0PCTEN (1U << 0)
/* Expose virt counters to EL0 */
#define CNTKCTL_EL1_EL0VCTEN (1U << 1)
/* Expose virt timer registers to EL0 */
#define CNTKCTL_EL1_EL0VTEN  (1U << 8)
/* Expose phys timer registers to EL0 */
#define CNTKCTL_EL1_EL0PTEN  (1U << 9)

/* Timer control registers */
#define CNTx_CTL_ENABLE      (1UL << 0) /* Enable timer */
#define CNTx_CTL_MASK        (1UL << 1) /* Mask IRQ */
#define CNTx_CTL_PENDING     (1UL << 2) /* IRQ pending */

#define CNTFRQ_MASK          GENMASK(31, 0)

typedef u64                  cycles_t;

/* This is for ARM64 */
#define read_cntpct_enforce_ordering(val) \
    do {                                  \
        u64 tmp, _val = (val);            \
        asm volatile(                     \
        "eor %0, %1, %1\n"                \
        "add %0, sp, %0\n"                \
        "ldr xzr, [%0]"                   \
        : "=r" (tmp) : "r" (_val));       \
    } while (0)

static inline cycles_t read_cntpct_stable(void)
{
    /*
     * ARM_WORKAROUND_858921: Cortex-A73 (all versions) counter
     * read can return a wrong value when the counter crosses a
     * 32bit boundary.
     */
    if (!IS_ENABLED(CFG_ARM_CORTEX_A73))
        return READ_SYSREG64(CNTPCT_EL0);
    else {
        /*
         * A recommended workaround for erratum 858921 is to:
         * 1 - Read twice CNTPCT.
         * 2 - Compare bit[32] of the two read values.
         *     - If bit[32] is different, keep the old value.
         *     - If bit[32] is the same, keep the new value.
         */
        cycles_t old, new;
        old = READ_SYSREG64(CNTPCT_EL0);
        new = READ_SYSREG64(CNTPCT_EL0);
        return (((old ^ new) >> 32) & 1) ? old : new;
    }
}

static inline cycles_t get_cycles(void)
{
    cycles_t cnt;

    isb();
    cnt = read_cntpct_stable();

    /*
     * If there is not any barrier here. When get_cycles being used in
     * some seqlock critical context in the future, the seqlock can be
     * speculated potentially.
     *
     * To prevent seqlock from being speculated silently, we add a barrier
     * here defensively. Normally, we just need an ISB here is enough, but
     * considering the minimum performance cost. We prefer to use enforce
     * order here.
     */
    read_cntpct_enforce_ordering(cnt);

    return cnt;
}

/* List of timer's IRQ */
enum timer_ppi
{
    TIMER_PHYS_SECURE_PPI = 0,
    TIMER_PHYS_NONSECURE_PPI = 1,
    TIMER_VIRT_PPI = 2,
    TIMER_HYP_PPI = 3,
    TIMER_HYP_VIRT_PPI = 4,
    MAX_TIMER_PPI = 5,
};

/* Get one of the timer IRQ number */
unsigned int timer_get_irq(enum timer_ppi ppi);

/* Set up the timer interrupt on this CPU */
extern void init_timer_interrupt(void);
extern unsigned long cpu_khz;
/* Counter value at boot time */
extern u64 boot_count;

extern stime_t ticks_to_ns(u64 ticks);
extern u64 ns_to_ticks(stime_t ns);

int time_preset(void);
// --------------------------------------------------------------
#endif /* _ORG_TIME_H */
