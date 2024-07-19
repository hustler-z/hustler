/**
 * Hustler's Project
 *
 * File:  lock.c
 * Date:  2024/06/19
 * Usage:
 */

#include <asm/spinlock.h>
#include <asm/atomic.h>
#include <asm/cmpxchg.h>
#include <org/system.h>
#include <org/section.h>
#include <bsp/cpu.h>
#include <bsp/panic.h>
#include <bsp/spinlock.h>
#include <bsp/preempt.h>
#include <bsp/percpu.h>
#include <bsp/config.h>

#if IS_IMPLEMENTED(__COMPLEX_SPINLOCK_IMPL)
// --------------------------------------------------------------

static always_inline
spinlock_tickets_t observe_lock(spinlock_tickets_t *t)
{
    spinlock_tickets_t v;

    smp_rmb();
    v.head_tail = read_atomic(&t->head_tail);
    return v;
}

static always_inline u16 observe_head(spinlock_tickets_t *t)
{
    smp_rmb();
    return read_atomic(&t->head);
}

static void always_inline
spin_lock_common(spinlock_tickets_t *t,
                 void (*cb)(void *data), void *data)
{
    spinlock_tickets_t tickets = SPINLOCK_TICKET_INC;

    preempt_disable();
    tickets.head_tail = gnu_fetch_and_add(&t->head_tail,
                                          tickets.head_tail);
    while (tickets.tail != observe_head(t)) {
        if (cb)
            cb(data);
        arch_lock_relax();
    }
    arch_lock_acquire_barrier();
}

void _spin_lock(spinlock_t *lock)
{
    spin_lock_common(&lock->tickets, NULL, NULL);
}

void _spin_lock_cb(spinlock_t *lock, void (*cb)(void *data),
                   void *data)
{
    spin_lock_common(&lock->tickets, cb, data);
}

void _spin_lock_irq(spinlock_t *lock)
{
    ASSERT(local_irq_is_enabled());
    local_irq_disable();
    _spin_lock(lock);
}

unsigned long _spin_lock_irqsave(spinlock_t *lock)
{
    unsigned long flags;

    local_irq_save(flags);
    _spin_lock(lock);
    return flags;
}

static void always_inline
spin_unlock_common(spinlock_tickets_t *t)
{
    arch_lock_release_barrier();
    add_sized(&t->head, 1);
    arch_lock_signal();
    preempt_enable();
}

void _spin_unlock(spinlock_t *lock)
{
    spin_unlock_common(&lock->tickets);
}

void _spin_unlock_irq(spinlock_t *lock)
{
    _spin_unlock(lock);
    local_irq_enable();
}

void _spin_unlock_irqrestore(spinlock_t *lock,
                             unsigned long flags)
{
    _spin_unlock(lock);
    local_irq_restore(flags);
}

static bool always_inline
spin_is_locked_common(const spinlock_tickets_t *t)
{
    return t->head != t->tail;
}

bool _spin_is_locked(const spinlock_t *lock)
{
    return spin_is_locked_common(&lock->tickets);
}

static bool always_inline
spin_trylock_common(spinlock_tickets_t *t)
{
    spinlock_tickets_t old, new;

    preempt_disable();
    old = observe_lock(t);
    if (old.head != old.tail) {
        preempt_enable();
        return false;
    }
    new = old;
    new.tail++;
    if (cmpxchg(&t->head_tail, old.head_tail,
                new.head_tail) != old.head_tail) {
        preempt_enable();
        return false;
    }

    return true;
}

bool _spin_trylock(spinlock_t *lock)
{
    return spin_trylock_common(&lock->tickets);
}

static void always_inline
spin_barrier_common(spinlock_tickets_t *t)
{
    spinlock_tickets_t sample;

    smp_mb();
    sample = observe_lock(t);
    if (sample.head != sample.tail) {
        while (observe_head(t) == sample.head)
            arch_lock_relax();
    }
    smp_mb();
}

void _spin_barrier(spinlock_t *lock)
{
    spin_barrier_common(&lock->tickets);
}

bool _rspin_is_locked(const rspinlock_t *lock)
{
    return lock->recurse_cpu == SPINLOCK_NO_CPU
           ? spin_is_locked_common(&lock->tickets)
           : lock->recurse_cpu == smp_processor_id();
}

void _rspin_barrier(rspinlock_t *lock)
{
    spin_barrier_common(&lock->tickets);
}

bool _rspin_trylock(rspinlock_t *lock)
{
    unsigned int cpu = smp_processor_id();

    /* Don't allow overflow of recurse_cpu field. */
    BUILD_BUG_ON(NR_CPUS > SPINLOCK_NO_CPU);
    BUILD_BUG_ON(SPINLOCK_CPU_BITS >
                 sizeof(lock->recurse_cpu) * 8);
    BUILD_BUG_ON(SPINLOCK_RECURSE_BITS < 3);
    BUILD_BUG_ON(SPINLOCK_MAX_RECURSE >
                 ((1u << SPINLOCK_RECURSE_BITS) - 1));

    if (likely(lock->recurse_cpu != cpu)) {
        if (!spin_trylock_common(&lock->tickets))
            return false;
        lock->recurse_cpu = cpu;
    }

    /* We support only fairly shallow recursion,
     * else the counter overflows. */
    ASSERT(lock->recurse_cnt < SPINLOCK_MAX_RECURSE);
    lock->recurse_cnt++;

    return true;
}

void _rspin_lock(rspinlock_t *lock)
{
    unsigned int cpu = smp_processor_id();

    if (likely(lock->recurse_cpu != cpu)) {
        spin_lock_common(&lock->tickets, NULL, NULL);
        lock->recurse_cpu = cpu;
    }

    /* We support only fairly shallow recursion,
     * else the counter overflows. */
    ASSERT(lock->recurse_cnt < SPINLOCK_MAX_RECURSE);
    lock->recurse_cnt++;
}

unsigned long _rspin_lock_irqsave(rspinlock_t *lock)
{
    unsigned long flags;

    local_irq_save(flags);
    _rspin_lock(lock);

    return flags;
}

void _rspin_unlock(rspinlock_t *lock)
{
    if (likely(--lock->recurse_cnt == 0)) {
        lock->recurse_cpu = SPINLOCK_NO_CPU;
        spin_unlock_common(&lock->tickets);
    }
}

void _rspin_unlock_irqrestore(rspinlock_t *lock,
                              unsigned long flags)
{
    _rspin_unlock(lock);
    local_irq_restore(flags);
}

bool _nrspin_trylock(rspinlock_t *lock)
{
    if (unlikely(lock->recurse_cpu != SPINLOCK_NO_CPU))
        return false;

    return spin_trylock_common(&lock->tickets);
}

void _nrspin_lock(rspinlock_t *lock)
{
    spin_lock_common(&lock->tickets, NULL, NULL);
}

void _nrspin_unlock(rspinlock_t *lock)
{
    spin_unlock_common(&lock->tickets);
}

void _nrspin_lock_irq(rspinlock_t *lock)
{
    ASSERT(local_irq_is_enabled());
    local_irq_disable();
    _nrspin_lock(lock);
}

void _nrspin_unlock_irq(rspinlock_t *lock)
{
    _nrspin_unlock(lock);
    local_irq_enable();
}

unsigned long _nrspin_lock_irqsave(rspinlock_t *lock)
{
    unsigned long flags;

    local_irq_save(flags);
    _nrspin_lock(lock);

    return flags;
}

void _nrspin_unlock_irqrestore(rspinlock_t *lock,
                               unsigned long flags)
{
    _nrspin_unlock(lock);
    local_irq_restore(flags);
}

// --------------------------------------------------------------
#endif
