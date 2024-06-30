/**
 * Hustler's Project
 *
 * File:  lock.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ARCH_LOCK_H
#define _ARCH_LOCK_H
// --------------------------------------------------------------

#define SPINLOCK_LOCK        1
#define SPINLOCK_UNLOCK      0

#ifndef __ASSEMBLY__

void __cpu_spin_lock(unsigned int *lock);
unsigned int __cpu_spin_trylock(unsigned int *lock);
void __cpu_spin_unlock(unsigned int *lock);

#endif /* !__ASSEMBLY__ */

// --------------------------------------------------------------
#endif /* _ARCH_LOCK_H */
