/**
 * Hustler's Project
 *
 * File:  thread.c
 * Date:  2024/06/27
 * Usage:
 */

#include <asm-generic/thread.h>
#include <asm-generic/section.h>
#include <asm-generic/spinlock.h>
#include <asm/barrier.h>
#include <bsp/cpu.h>
#include <bsp/check.h>

// --------------------------------------------------------------
struct thread_percpu __thread_percpu thread_percpu_local[NR_CPUS];

extern unsigned long __get_percpu_pos(void);

static unsigned long get_percpu_pos(void)
{
    return __get_percpu_pos();
}

static struct thread_percpu *get_percpu_local(unsigned long  pos)
{
    ASSERT(pos < NR_CPUS);
    return &thread_percpu_local[pos];
}

struct thread_percpu *get_thread_percpu(void)
{
    unsigned long pos = get_percpu_pos();

    return get_percpu_local(pos);
}

u64 thread_get_exceptions(void)
{
    u64 daif = READ_SYSREG(DAIF);

    return (daif >> DAIF_F_SHIFT) & THREAD_EXCP_ALL;
}

void thread_set_exceptions(u64 exceptions)
{
    u64 daif = READ_SYSREG(DAIF);

    if (!(exceptions & THREAD_EXCP_FOREIGN_INTR))
        assert_have_no_spinlock();

    daif &= ~(THREAD_EXCP_ALL << DAIF_F_SHIFT);
    daif |= ((exceptions & THREAD_EXCP_ALL) << DAIF_F_SHIFT);

    barrier();
    WRITE_SYSREG(daif, DAIF);
    barrier();
}

u64 thread_mask_exceptions(u64 exceptions)
{
    u64 state = thread_get_exceptions();

    thread_set_exceptions(state | (exceptions & THREAD_EXCP_ALL));

    return state;
}

void thread_unmask_exceptions(u64 state)
{
    thread_set_exceptions(state & THREAD_EXCP_ALL);
}
// --------------------------------------------------------------
static int __thread_get_id(void)
{
	u64 exceptions = thread_mask_exceptions(THREAD_EXCP_FOREIGN_INTR);
	struct thread_percpu *tp = get_thread_percpu();
	int ct = tp->current;

	thread_unmask_exceptions(exceptions);
	return ct;
}

int thread_get_id(void)
{
    return __thread_get_id();
}

bool thread_is_in_normal_mode(void)
{
    u64 exceptions = thread_mask_exceptions(THREAD_EXCP_FOREIGN_INTR);
    struct thread_percpu *tp = get_thread_percpu();
    bool ret;

    ret = (tp->current != THREAD_ID_INVALID) &&
          !(tp->flags & ~THREAD_CLF_TMP);
    thread_mask_exceptions(exceptions);

    return ret;
}
// --------------------------------------------------------------
