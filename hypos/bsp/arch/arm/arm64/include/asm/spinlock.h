/**
 * Hustler's Project
 *
 * File:  lock.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ASM_LOCK_H
#define _ASM_LOCK_H
// --------------------------------------------------------------

#define SPINLOCK_LOCK        1
#define SPINLOCK_UNLOCK      0

#ifndef __ASSEMBLY__
#include <bsp/config.h>

#if IS_IMPLEMENTED(__SIMPLE_SPINLOCK_IMPL)
// --------------------------------------------------------------
void __cpu_spin_lock(unsigned int *lock);
unsigned int __cpu_spin_trylock(unsigned int *lock);
void __cpu_spin_unlock(unsigned int *lock);
#else
// --------------------------------------------------------------
#include <asm/barrier.h>

#define arch_lock_acquire_barrier() smp_mb()
#define arch_lock_release_barrier() smp_mb()

#define arch_lock_relax()  wfe()
#define arch_lock_signal() do { \
    dsb(ishst);                 \
    sev();                      \
} while(0)

#define arch_lock_signal_wmb()  arch_lock_signal()

// --------------------------------------------------------------
#endif

#endif /* !__ASSEMBLY__ */

// --------------------------------------------------------------
#endif /* _ASM_LOCK_H */
