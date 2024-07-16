/**
 * Hustler's Project
 *
 * File:  delay.h
 * Date:  2024/07/11
 * Usage:
 */

#ifndef _BSP_DELAY_H
#define _BSP_DELAY_H
// --------------------------------------------------------------

void udelay(unsigned long usecs);

static inline void mdelay(unsigned long msecs)
{
    while (msecs--)
        udelay(1000);
}

// --------------------------------------------------------------
#endif /* _BSP_DELAY_H */
