/**
 * Hustler's Project
 *
 * File:  lock.c
 * Date:  2024/06/19
 * Usage:
 */

#include <asm-generic/section.h>
#include <bsp/percpu.h>
#include <bsp/cpu.h>
#include <bsp/check.h>
#include <bsp/lock.h>

// --------------------------------------------------------------

/* XXX: simple spin_lock implementation stolen from optee.
 */
extern struct percpu_stat percpu_stat_local[NR_CPUS];

extern void __cpu_spin_lock(unsigned int *lock);
extern unsigned int __cpu_spin_trylock(unsigned int *lock);
extern void __cpu_spin_unlock(unsigned int *lock);

void spinlock_count_increment(void)
{
    struct percpu_stat *stat = get_percpu_stat();

    stat->locked_count++;
    ASSERT(stat->locked_count);
}

void spinlock_count_decrement(void)
{
    struct percpu_stat *stat = get_percpu_stat();

    ASSERT(stat->locked_count);
    stat->locked_count--;
}

void spin_lock(unsigned int *lock)
{
    __cpu_spin_lock(lock);
    spinlock_count_increment();
}

unsigned int spin_trylock(unsigned int *lock)
{
    unsigned int ret;

    ret = __cpu_spin_trylock(lock);
    if (!ret)
        spinlock_count_increment();

    return !ret;
}

void spin_unlock(unsigned int *lock)
{
    __cpu_spin_unlock(lock);
    spinlock_count_decrement();
}

// --------------------------------------------------------------
