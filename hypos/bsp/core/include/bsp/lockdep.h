/**
 * Hustler's Project
 *
 * File:  lockdep.h
 * Date:  2024/06/28
 * Usage:
 */

#ifndef _BSP_LOCKDEP_H
#define _BSP_LOCKDEP_H
// --------------------------------------------------------------
#include <lib/queue.h>
#include <bsp/type.h>
#include <bsp/debug.h>

struct lockdep_node;

struct lockdep_edge {
    struct lockdep_node *to;
    uptr_t thread_id;
    hva_t *call_stack_from;
    hva_t *call_stack_to;
    STAILQ_ENTRY(lockdep_edge) link;
};

STAILQ_HEAD(lockdep_edge_head, lockdep_edge);

struct lockdep_node {
    uptr_t lock_id;
    struct lockdep_edge_head edges;
    TAILQ_ENTRY(lockdep_node) link;
    u8 flags;
};

TAILQ_HEAD(lockdep_node_head, lockdep_node);

struct lockdep_lock {
    struct lockdep_node *node;
    hva_t *call_stack;
    TAILQ_ENTRY(lockdep_lock) link;
};

TAILQ_HEAD(lockdep_lock_head, lockdep_lock);

int __lockdep_lock_acquire(struct lockdep_node_head *graph,
                struct lockdep_lock_head *owned,
                uptr_t id);
int __lockdep_lock_tryacquire(struct lockdep_node_head *graph,
                struct lockdep_lock_head *owned,
                uptr_t id);
int __lockdep_lock_release(struct lockdep_lock_head *owned,
                uptr_t id);

static inline void lockdep_lock_acquire(struct lockdep_node_head *graph,
                struct lockdep_lock_head *owned,
                uptr_t id)
{
    int res = __lockdep_lock_acquire(graph, owned, id);

    if (res) {
        MSGE("lockdep: error %d\n", res);
        BUG();
    }
}

static inline void lockdep_lock_tryacquire(struct lockdep_node_head *graph,
					   struct lockdep_lock_head *owned,
					   uptr_t id)
{
	int res = __lockdep_lock_tryacquire(graph, owned, id);

	if (res) {
		MSGE("lockdep: error %d\n", res);
		BUG();
	}
}

static inline void lockdep_lock_release(struct lockdep_lock_head *owned,
					uptr_t id)
{
	int res = __lockdep_lock_release(owned, id);

	if (res) {
		MSGE("lockdep: error %d\n", res);
		BUG();
	}
}

void lockdep_lock_destroy(struct lockdep_node_head *graph, uptr_t id);
// --------------------------------------------------------------
#endif /* _BSP_LOCKDEP_H */
