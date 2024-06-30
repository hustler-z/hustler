/**
 * Hustler's Project
 *
 * File:  thread.h
 * Date:  2024/06/27
 * Usage:
 */

#ifndef _ASM_GENERIC_THREAD_H
#define _ASM_GENERIC_THREAD_H
// --------------------------------------------------------------
#include <common/type.h>
#include <asm/sysregs.h>

#define THREAD_EXCP_FOREIGN_INTR     (DAIF_I >> DAIF_F_SHIFT)
#define THREAD_EXCP_NATIVE_INTR      (DAIF_F >> DAIF_F_SHIFT)
#define THREAD_EXCP_ALL              (THREAD_EXCP_FOREIGN_INTR | \
                                      THREAD_EXCP_NATIVE_INTR  | \
                                      (DAIF_A >> DAIF_F_SHIFT))

#define THREAD_ID_INVALID            -1

#define THREAD_CLF_TMP_SHIFT         0
#define THREAD_CLF_ABORT_SHIFT       1
#define THREAD_CLF_IRQ_SHIFT         2
#define THREAD_CLF_FIQ_SHIFT         3

#define THREAD_CLF_TMP               BIT(THREAD_CLF_TMP_SHIFT, UL)
#define THREAD_CLF_ABORT             BIT(THREAD_CLF_ABORT_SHIFT, UL)
#define THREAD_CLF_IRQ               BIT(THREAD_CLF_IRQ_SHIFT, UL)
#define THREAD_CLF_FIQ               BIT(THREAD_CLF_FIQ_SHIFT, UL)

u64 thread_get_exceptions(void);
void thread_set_exceptions(u64 exceptions);
u64 thread_mask_exceptions(u64 exceptions);
void thread_unmask_exceptions(u64 state);

static inline bool thread_foreign_intr_disabled(void)
{
    return !!(thread_get_exceptions() & THREAD_EXCP_FOREIGN_INTR);
}

enum thread_state {
    THREAD_STATE_FREE = 0,
    THREAD_STATE_SUSPENDED,
    THREAD_STATE_ACTIVE,
};

struct thread_context {
    enum thread_state state;
};

struct thread_percpu {
    unsigned long locked_count;
    vaddr_t stack_bottom;
    int current;
    unsigned long flags;
};

struct thread_percpu *get_thread_percpu(void);

int thread_get_id(void);

bool thread_is_in_normal_mode(void);
// --------------------------------------------------------------
#endif /* _ASM_GENERIC_THREAD_H */
