/**
 * Hustler's Project
 *
 * File:  lock.h
 * Date:  2024/06/19
 * Usage:
 */

#ifndef _ASM_GENERIC_LOCK_H
#define _ASM_GENERIC_LOCK_H
// --------------------------------------------------------------
#include <asm/lock.h>

void spin_lock(unsigned int *lock);
unsigned int spin_trylock(unsigned int *lock);
void spin_unlock(unsigned int *lock);

// --------------------------------------------------------------
#endif /* _ASM_GENERIC_LOCK_H */
