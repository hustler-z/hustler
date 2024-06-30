/**
 * Hustler's Project
 *
 * File:  wq.h
 * Date:  2024/06/27
 * Usage:
 */

#ifndef _BSP_WQ_H
#define _BSP_WQ_H
// --------------------------------------------------------------
#include <lib/queue.h>
#include <common/type.h>

struct wait_queue_elem;

SLIST_HEAD(wait_queue, wait_queue_elem);

#define WAIT_QUEUE_INITIALIZER { .slh_first = NULL }

struct wait_queue_elem {
	int  handle;
	bool done;
	bool wait_read;
	struct condition *cv;
	SLIST_ENTRY(wait_queue_elem) link;
};

void wq_init(struct wait_queue *wq);

void wq_wait_init_condition(struct wait_queue *wq, struct wait_queue_elem *wqe,
			struct condition *cv, bool wait_read);

static inline void wq_wait_init(struct wait_queue *wq,
			struct wait_queue_elem *wqe, bool wait_read)
{
	wq_wait_init_condition(wq, wqe, NULL, wait_read);
}

/* Waits for the wait queue element to the awakened. */
void wq_wait_final(struct wait_queue *wq, struct wait_queue_elem *wqe,
		   const void *sync_obj, const char *fname, int lineno);

/* Wakes up the first wait queue element in the wait queue, if there is one */
void wq_wake_next(struct wait_queue *wq, const void *sync_obj,
		const char *fname, int lineno);

/* Returns true if the wait queue doesn't contain any elements */
bool wq_is_empty(struct wait_queue *wq);

void wq_promote_condition(struct wait_queue *wq, struct condition *cv,
			bool only_one, const void *sync_obj, const char *fname,
			int lineno);
bool wq_have_condition(struct wait_queue *wq, struct condition *cv);

// --------------------------------------------------------------
#endif /* _BSP_WQ_H */
