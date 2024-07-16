/**
 * Hustler's Project
 *
 * File:  preempt.h
 * Date:  2024/07/10
 * Usage:
 */

#ifndef _BSP_PREEMPT_H
#define _BSP_PREEMPT_H
// --------------------------------------------------------------
#include <asm/barrier.h>
#include <bsp/percpu.h>
#include <bsp/type.h>

DECLARE_PERCPU(unsigned int, __preempt_count);

#define preempt_count()      (this_cpu(__preempt_count))

#define preempt_disable()  \
    do {                   \
        preempt_count()++; \
        barrier();         \
    } while(0)

#define preempt_enable()   \
    do {                   \
        barrier();         \
        preempt_count()--; \
    } while(0)

bool in_atomic(void);

void ASSERT_NOT_IN_ATOMIC(void);
// --------------------------------------------------------------
#endif /* _BSP_PREEMPT_H */
