/**
 * Hustler's Project
 *
 * File:  rwlock.c
 * Date:  2024/07/11
 * Usage: rwlock implementation
 */

#include <org/smp.h>
#include <org/irq.h>
#include <bsp/rwlock.h>
#include <bsp/config.h>

#if IS_IMPLEMENTED(__RWLOCK_IMPL)
// --------------------------------------------------------------
static inline void rspin_until_writer_unlock(rwlock_t *lock, u32 cnts)
{
    while ((cnts & _QW_WMASK) == _QW_LOCKED) {
        cpu_relax();
        smp_rmb();
        cnts = atomic_read(&lock->cnts);
    }
}

void queue_read_lock_slowpath(rwlock_t *lock)
{
    u32 cnts;

    atomic_sub(_QR_BIAS, &lock->cnts);

    _spin_lock(&lock->lock);

    while (atomic_read(&lock->cnts) & _QW_WMASK)
        cpu_relax();

    cnts = atomic_add_return(_QR_BIAS, &lock->cnts);
    rspin_until_writer_unlock(lock, cnts);

    spin_unlock(&lock->lock);
}

void queue_write_lock_slowpath(rwlock_t *lock)
{
    u32 cnts;

    _spin_lock(&lock->lock);

    if (!atomic_read(&lock->cnts) &&
        (atomic_cmpxchg(&lock->cnts, 0, _write_lock_val()) == 0))
        goto unlock;

    for ( ; ; ) {
        cnts = atomic_read(&lock->cnts);
        if (!(cnts & _QW_WMASK) &&
            (atomic_cmpxchg(&lock->cnts, cnts,
                             cnts | _QW_WAITING) == cnts))
            break;

        cpu_relax();
    }

    for ( ; ; ) {
        cnts = atomic_read(&lock->cnts);
        if ((cnts == _QW_WAITING) &&
            (atomic_cmpxchg(&lock->cnts, _QW_WAITING,
                             _write_lock_val()) == _QW_WAITING))
            break;

        cpu_relax();
    }

unlock:
    spin_unlock(&lock->lock);
}

static DEFINE_PERCPU(cpumask_t, percpu_rwlock_readers);

void _percpu_write_lock(percpu_rwlock_t **per_cpudata,
                percpu_rwlock_t *percpu_rwlock)
{
    unsigned int cpu;
    cpumask_t *rwlock_readers = &this_cpu(percpu_rwlock_readers);

    _percpu_rwlock_owner_check(per_cpudata, percpu_rwlock);

    _write_lock(&percpu_rwlock->rwlock);

    percpu_rwlock->writer_activating = 1;
    smp_mb();

    ASSERT(!in_irq());
    cpumask_copy(rwlock_readers, &cpu_online_map);

    for ( ; ; ) {
        for_each_cpu(cpu, rwlock_readers) {
            if (percpu_ptr(per_cpudata, cpu) != percpu_rwlock)
                cpumask_clear_cpu(cpu, rwlock_readers);
        }

        if (cpumask_empty(rwlock_readers))
            break;

        cpu_relax();
    };
}

// --------------------------------------------------------------
#endif
