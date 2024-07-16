/**
 * Hustler's Project
 *
 * File:  period.c
 * Date:  2024/06/05
 * Usage:
 */

#include <org/irq.h>
#include <asm/barrier.h>
#include <bsp/preempt.h>
#include <bsp/panic.h>
// --------------------------------------------------------------

DEFINE_PERCPU(unsigned int, __preempt_count);

bool in_atomic(void)
{
    return preempt_count() || in_irq() || !local_irq_is_enabled();
}

void ASSERT_NOT_IN_ATOMIC(void)
{
    ASSERT(!preempt_count());
    ASSERT(!in_irq());
    ASSERT(local_irq_is_enabled());
}
// --------------------------------------------------------------
