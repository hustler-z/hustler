/**
 * Hustler's Project
 *
 * File:  lock.c
 * Date:  2024/06/19
 * Usage:
 */

#include <asm-generic/spinlock.h>
#include <asm-generic/section.h>
#include <asm-generic/thread.h>
#include <bsp/cpu.h>
#include <bsp/panic.h>

// --------------------------------------------------------------

/* XXX: simple spinlock implementation stolen from optee.
 */
extern struct thread_percpu thread_percpu_local[NR_CPUS];

extern void __cpu_spinlock(unsigned int *lock);
extern unsigned int __cpu_spin_trylock(unsigned int *lock);
extern void __cpu_spinunlock(unsigned int *lock);

void spinlock_count_increment(void)
{
    struct thread_percpu *stat = get_thread_percpu();

    stat->locked_count++;
    ASSERT(stat->locked_count);
}

void spinlock_count_decrement(void)
{
    struct thread_percpu *stat = get_thread_percpu();

    ASSERT(stat->locked_count);
    stat->locked_count--;
}

void spinlock(unsigned int *lock)
{
    __cpu_spinlock(lock);
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

void spinunlock(unsigned int *lock)
{
    __cpu_spinunlock(lock);
    spinlock_count_decrement();
}

bool have_spinlock(void)
{
    struct thread_percpu *tp;

    if (!thread_foreign_intr_disabled())
        return false;

    tp = get_thread_percpu();

    return !!tp->locked_count;
}

static u64 __spinlock_xsave(unsigned int *lock)
{
    u64 exceptions = thread_mask_exceptions(THREAD_EXCP_ALL);

    spinlock(lock);

    return exceptions;
}

u64 spinlock_xsave(unsigned int *lock)
{
    return __spinlock_xsave(lock);
}

void spinunlock_xrestore(unsigned int *lock,
                          u64 exceptions)
{
    spinunlock(lock);
    thread_unmask_exceptions(exceptions);
}
// --------------------------------------------------------------
