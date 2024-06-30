/**
 * Hustler's Project
 *
 * File:  lock.h
 * Date:  2024/06/19
 * Usage:
 */

#ifndef _BSP_LOCK_H
#define _BSP_LOCK_H
// --------------------------------------------------------------
#include <asm/lock.h>
#include <bsp/check.h>
#include <common/type.h>

#define DEFINE_SPINLOCK(lock) \
unsigned int lock = SPINLOCK_UNLOCK

void spinlock(unsigned int *lock);
unsigned int spin_trylock(unsigned int *lock);
void spinunlock(unsigned int *lock);
u64 spinlock_xsave(unsigned int *lock);
void spinunlock_xrestore(unsigned int *lock,
                          u64 exceptions);

bool have_spinlock(void);
static inline void assert_have_no_spinlock(void)
{
    ASSERT(!have_spinlock());
}
// --------------------------------------------------------------
#endif /* _BSP_LOCK_H */
