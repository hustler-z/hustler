/**
 * Hustler's Project
 *
 * File:  timer.c
 * Date:  2024/05/22
 * Usage:
 */

#include <asm-generic/section.h>
#include <common/timer.h>
#include <asm/barrier.h>
#include <bsp/period.h>
#include <lib/math.h>
#include <bsp/debug.h>

// --------------------------------------------------------------
#define SYS_HZ       (1000)
#define WD_PERIOD    (10 * 1000 * 1000)

extern unsigned long get_phycnt_el0(void);
extern unsigned long get_cntfrq_el0(void);

unsigned long timer_read_counter(void)
{
    isb();
    return get_phycnt_el0();
}

u64 __notrace get_ticks(void)
{
    unsigned long ticks = timer_read_counter();
    return ticks;
}

u64 __notrace get_tbclk(void)
{
    return get_cntfrq_el0();
}

u32 __notrace get_tbclk_mhz(void)
{
    return get_tbclk() / 1000000;
}

u64 __notrace timer_get_us(void)
{
    return get_ticks() / get_tbclk_mhz();
}


static u64 __notrace tick_to_time(u64 tick)
{
    u64 div = get_tbclk();

    tick += SYS_HZ;

    do_div(tick, div);

    return tick;
}

u64 get_timer(u64 base)
{
    return tick_to_time(get_ticks()) - base;
}

u64 timer_get_boot_us(void)
{
    u64 val = get_ticks() * 1000000;
    return val / get_tbclk();
}
// --------------------------------------------------------------

/* Delay Implementation
 */
void __udelay(unsigned long usec)
{
    u64 now = get_ticks();
    u64 stop;

    stop = now + (u64)usec * get_tbclk_mhz();

    while ((u64)(stop - get_ticks()) > 0)
        ;
}

void udelay(unsigned long usec)
{
    unsigned long kv;

    do {
        schedule();
        kv = usec > WD_PERIOD ? WD_PERIOD : usec;
        __udelay(kv);
        usec -= kv;
    } while (usec);
}

void mdelay(u32 msec)
{
    __udelay(1000 * msec);
}
// --------------------------------------------------------------
int __bootfunc timer_setup(void)
{

    return 0;
}
// --------------------------------------------------------------
