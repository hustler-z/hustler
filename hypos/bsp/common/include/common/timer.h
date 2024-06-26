/**
 * Hustler's Project
 *
 * File:  timer.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _COMMON_TIMER_H
#define _COMMON_TIMER_H
// --------------------------------------------------------------
#include <common/ccattr.h>
#include <common/type.h>

u64 __notrace get_ticks(void);
u64 __notrace get_tbclk(void);
u64 __notrace timer_get_us(void);
u64 get_timer(u64 base);
u64 timer_get_boot_us(void);
void udelay(unsigned long usec);
int timer_setup(void);

// --------------------------------------------------------------
#endif /* _COMMON_TIMER_H */
