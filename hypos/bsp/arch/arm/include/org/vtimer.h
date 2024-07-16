/**
 * Hustler's Project
 *
 * File:  vtimer.h
 * Date:  2024/07/10
 * Usage:
 */

#ifndef _ORG_VTIMER_H
#define _ORG_VTIMER_H
// ------------------------------------------------------------------------
#include <bsp/type.h>
#include <bsp/timer.h>
#include <org/vcpu.h>

struct vtimer {
    struct vcpu *v;
    int irq;
    struct timer timer;
    register_t   ctl;
    u64          cval;
};

// ------------------------------------------------------------------------
#endif /* _ORG_VTIMER_H */
