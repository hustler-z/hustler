/**
 * Hustler's Project
 *
 * File:  lock.h
 * Date:  2024/06/19
 * Usage:
 */

#ifndef _BSP_SPINLOCK_H
#define _BSP_SPINLOCK_H
// --------------------------------------------------------------
#include <asm/spinlock.h>
#include <bsp/compiler.h>
#include <bsp/panic.h>
#include <bsp/type.h>

#define SPINLOCK_CPU_BITS    16

/*
 * Head                   Tail
 * |◀-------- 16 --------▶|◀-------- 16 --------▶|
 *
 * When first got spinlock, head + 1 atomically.
 * compare the <tail> to <head>:
 * (a) if equals, continue on executing.
 * (b) if not equals, loop until lock been released.
 */
typedef union {
    u32 head_tail;
    struct {
        u16 head;
        u16 tail;
    };
} spinlock_tickets_t;

#define SPINLOCK_TICKET_INC { .head_tail = 0x10000, }

typedef struct spinlock {
    spinlock_tickets_t tickets;
} spinlock_t;

typedef struct rspinlock {
    spinlock_tickets_t tickets;
    u16 recurse_cpu;
#define SPINLOCK_NO_CPU        ((1u << SPINLOCK_CPU_BITS) - 1)
#define SPINLOCK_RECURSE_BITS  8
    u8  recurse_cnt;
#define SPINLOCK_MAX_RECURSE   15
} rspinlock_t;

#define SPIN_LOCK_UNLOCKED  {  }
#define RSPIN_LOCK_UNLOCKED {                   \
    .recurse_cpu = SPINLOCK_NO_CPU,             \
}
#define DEFINE_SPINLOCK(l)  spinlock_t l = SPIN_LOCK_UNLOCKED
#define DEFINE_RSPINLOCK(l) rspinlock_t l = RSPIN_LOCK_UNLOCKED

#define spin_lock_init(l) (*(l)  = (spinlock_t)SPIN_LOCK_UNLOCKED)
#define rspin_lock_init(l) (*(l) = (rspinlock_t)RSPIN_LOCK_UNLOCKED)

void _spin_lock(spinlock_t *lock);
void _spin_lock_cb(spinlock_t *lock, void (*cb)(void *data), void *data);
void _spin_lock_irq(spinlock_t *lock);
unsigned long _spin_lock_irqsave(spinlock_t *lock);

void _spin_unlock(spinlock_t *lock);
void _spin_unlock_irq(spinlock_t *lock);
void _spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags);

bool _spin_is_locked(const spinlock_t *lock);
bool _spin_trylock(spinlock_t *lock);
void _spin_barrier(spinlock_t *lock);

static always_inline void spin_lock(spinlock_t *l)
{
    _spin_lock(l);
}

static always_inline void spin_lock_cb(spinlock_t *l, void (*c)(void *data),
                                       void *d)
{
    _spin_lock_cb(l, c, d);
}

static always_inline void spin_lock_irq(spinlock_t *l)
{
    _spin_lock_irq(l);
}

#define spin_lock_irqsave(l, f)                                 \
    ({                                                          \
        BUILD_BUG_ON(sizeof(f) != sizeof(unsigned long));       \
        ((f) = _spin_lock_irqsave(l));                          \
    })

/* Conditionally take a spinlock in a speculation safe way. */
static always_inline void spin_lock_if(bool condition, spinlock_t *l)
{
    if (condition)
        _spin_lock(l);
}

#define spin_unlock(l)                _spin_unlock(l)
#define spin_unlock_irq(l)            _spin_unlock_irq(l)
#define spin_unlock_irqrestore(l, f)  _spin_unlock_irqrestore(l, f)

#define spin_is_locked(l)             _spin_is_locked(l)
#define spin_trylock(l)               lock_evaluate_nospec(_spin_trylock(l))

#define spin_trylock_irqsave(lock, flags)   \
({                                          \
    local_irq_save(flags);                  \
    spin_trylock(lock) ?                    \
    1 : ({ local_irq_restore(flags); 0; }); \
})

#define spin_lock_kick(l)             arch_lock_signal_wmb()

/* Ensure a lock is quiescent between two critical operations. */
#define spin_barrier(l)               _spin_barrier(l)

/*
 * rspin_[un]lock(): Use these forms when the lock can (safely!) be
 * reentered recursively on the same CPU. All critical regions that may form
 * part of a recursively-nested set must be protected by these forms. If there
 * are any critical regions that cannot form part of such a set, they can use
 * nrspin_[un]lock().
 * The nrspin_[un]lock() forms act the same way as normal spin_[un]lock()
 * calls, but operate on rspinlock_t locks. nrspin_lock() and rspin_lock()
 * calls are blocking to each other for a specific lock even on the same cpu.
 */
bool _rspin_trylock(rspinlock_t *lock);
void _rspin_lock(rspinlock_t *lock);
#define rspin_lock_irqsave(l, f)                          \
    ({                                                    \
        BUILD_BUG_ON(sizeof(f) != sizeof(unsigned long)); \
        (f) = _rspin_lock_irqsave(l);                     \
    })
unsigned long _rspin_lock_irqsave(rspinlock_t *lock);
void _rspin_unlock(rspinlock_t *lock);
void _rspin_unlock_irqrestore(rspinlock_t *lock, unsigned long flags);
bool _rspin_is_locked(const rspinlock_t *lock);
void _rspin_barrier(rspinlock_t *lock);

static always_inline void rspin_lock(rspinlock_t *lock)
{
    _rspin_lock(lock);
}

static always_inline bool lock_evaluate_nospec(bool condition)
{
    return condition;
}

#define rspin_trylock(l)              lock_evaluate_nospec(_rspin_trylock(l))
#define rspin_unlock(l)               _rspin_unlock(l)
#define rspin_unlock_irqrestore(l, f) _rspin_unlock_irqrestore(l, f)
#define rspin_barrier(l)              _rspin_barrier(l)
#define rspin_is_locked(l)            _rspin_is_locked(l)

bool _nrspin_trylock(rspinlock_t *lock);
void _nrspin_lock(rspinlock_t *lock);
#define nrspin_lock_irqsave(l, f)                         \
    ({                                                    \
        BUILD_BUG_ON(sizeof(f) != sizeof(unsigned long)); \
        (f) = _nrspin_lock_irqsave(l);                    \
    })
unsigned long _nrspin_lock_irqsave(rspinlock_t *lock);
void _nrspin_unlock(rspinlock_t *lock);
void _nrspin_lock_irq(rspinlock_t *lock);
void _nrspin_unlock_irq(rspinlock_t *lock);
void _nrspin_unlock_irqrestore(rspinlock_t *lock, unsigned long flags);

static always_inline void nrspin_lock(rspinlock_t *lock)
{
    _nrspin_lock(lock);
}

static always_inline void nrspin_lock_irq(rspinlock_t *l)
{
    _nrspin_lock_irq(l);
}

#define nrspin_trylock(l)              lock_evaluate_nospec(_nrspin_trylock(l))
#define nrspin_unlock(l)               _nrspin_unlock(l)
#define nrspin_unlock_irqrestore(l, f) _nrspin_unlock_irqrestore(l, f)
#define nrspin_unlock_irq(l)           _nrspin_unlock_irq(l)

// --------------------------------------------------------------
#endif /* _BSP_SPINLOCK_H */
