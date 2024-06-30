/**
 * Hustler's Project
 *
 * File:  wq.c
 * Date:  2024/06/27
 * Usage:
 */

#include <asm-generic/spinlock.h>
#include <asm-generic/thread.h>
#include <bsp/debug.h>
#include <bsp/wq.h>
#include <bsp/mutex.h>
#include <common/ccattr.h>

static unsigned wq_spinlock;

void wq_init(struct wait_queue *wq)
{
    *wq = (struct wait_queue)WAIT_QUEUE_INITIALIZER;
}

static void slist_add_tail(struct wait_queue *wq,
        struct wait_queue_elem *wqe)
{
    struct wait_queue_elem *wqe_iter;

    /* Add elem to end of wait queue */
    wqe_iter = SLIST_FIRST(wq);
    if (wqe_iter) {
        while (SLIST_NEXT(wqe_iter, link))
            wqe_iter = SLIST_NEXT(wqe_iter, link);
        SLIST_INSERT_AFTER(wqe_iter, wqe, link);
    } else
        SLIST_INSERT_HEAD(wq, wqe, link);
}

void wq_wait_init_condition(struct wait_queue *wq,
        struct wait_queue_elem *wqe,
        struct condition *cv, bool wait_read)
{
    u64 old_itr_status;

    wqe->handle = thread_get_id();
    wqe->done = false;
    wqe->wait_read = wait_read;
    wqe->cv = cv;

    old_itr_status = spinlock_xsave(&wq_spinlock);

    slist_add_tail(wq, wqe);

    spinunlock_xrestore(&wq_spinlock, old_itr_status);
}

void wq_wait_final(struct wait_queue *wq,
        struct wait_queue_elem *wqe,
        const void *sync_obj, const char *fname, int lineno)
{
    u64 old_itr_status;
    unsigned done;

    do {
        old_itr_status = spinlock_xsave(&wq_spinlock);

        done = wqe->done;
        if (done)
            SLIST_REMOVE(wq, wqe, wait_queue_elem, link);

        spinunlock_xrestore(&wq_spinlock, old_itr_status);
    } while (!done);
}

void wq_wake_next(struct wait_queue *wq, const void *sync_obj,
                  const char *fname, int lineno)
{
    u64 old_itr_status;
    struct wait_queue_elem *wqe;
    int handle __maybe_unused;
    bool do_wakeup = false;
    bool wake_type_assigned = false;
    bool wake_read = false;

    while (true) {
        old_itr_status = spinlock_xsave(&wq_spinlock);

        SLIST_FOREACH(wqe, wq, link) {
            if (wqe->cv)
                continue;
            if (wqe->done)
                continue;
            if (!wake_type_assigned) {
                wake_read = wqe->wait_read;
                wake_type_assigned = true;
            }

            if (wqe->wait_read != wake_read)
                continue;

            wqe->done = true;
            handle = wqe->handle;
            do_wakeup = true;
            break;
        }

        spinunlock_xrestore(&wq_spinlock, old_itr_status);

        if (!do_wakeup || !wake_read)
            break;
        do_wakeup = false;
    }
}

void wq_promote_condition(struct wait_queue *wq,
            struct condition *cv,
            bool only_one,
            const void *sync_obj,
            const char *fname,
            int lineno __maybe_unused)
{
    u64 old_itr_status;
    struct wait_queue_elem *wqe;

    if (!cv)
        return;

    old_itr_status = spinlock_xsave(&wq_spinlock);

    SLIST_FOREACH(wqe, wq, link) {
        if (wqe->cv == cv) {
            if (fname)
                MSGH("promote thread %u %p %s:%d",
                     wqe->handle, (void *)cv->mtx, fname, lineno);
            else
                MSGH("promote thread %u %p",
                     wqe->handle, (void *)cv->mtx);

            wqe->cv = NULL;
            if (only_one)
                break;
        }
    }

    spinunlock_xrestore(&wq_spinlock, old_itr_status);
}

bool wq_have_condition(struct wait_queue *wq, struct condition *cv)
{
    u64 old_itr_status;
    struct wait_queue_elem *wqe;
    bool rc = false;

    old_itr_status = spinlock_xsave(&wq_spinlock);

    SLIST_FOREACH(wqe, wq, link) {
        if (wqe->cv == cv) {
            rc = true;
            break;
        }
    }

    spinunlock_xrestore(&wq_spinlock, old_itr_status);

    return rc;
}

bool wq_is_empty(struct wait_queue *wq)
{
    u64 old_itr_status;
    bool ret;

    old_itr_status = spinlock_xsave(&wq_spinlock);

    ret = SLIST_EMPTY(wq);

    spinunlock_xrestore(&wq_spinlock, old_itr_status);

    return ret;
}
