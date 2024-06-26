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

#define DEFINE_SPINLOCK(lock) \
unsigned int lock = SPINLOCK_UNLOCK

void spin_lock(unsigned int *lock);
unsigned int spin_trylock(unsigned int *lock);
void spin_unlock(unsigned int *lock);

// --------------------------------------------------------------
#endif /* _BSP_LOCK_H */
