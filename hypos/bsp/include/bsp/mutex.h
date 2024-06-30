/**
 * Hustler's Project
 *
 * File:  mutex.h
 * Date:  2024/06/28
 * Usage:
 */

#ifndef _BSP_MUTEX_H
#define _BSP_MUTEX_H
// --------------------------------------------------------------
#include <bsp/wq.h>
#include <common/refcount.h>
#include <asm-generic/thread.h>
#include <lib/queue.h>

struct mutex {
	unsigned int spinlock;
	struct wait_queue wq;
	int state; /* -1: write, 0: unlocked, > 0: readers */
};

#define MUTEX_INITIALIZER { .wq = WAIT_QUEUE_INITIALIZER }

struct recursive_mutex {
	struct mutex m;		/* used when lock_depth goes 0 -> 1 or 1 -> 0 */
	int owner;
	struct refcount lock_depth;
};

#define RECURSIVE_MUTEX_INITIALIZER { .m = MUTEX_INITIALIZER, \
				      .owner = THREAD_ID_INVALID }

// --------------------------------------------------------------
void mutex_lock_check(struct mutex *m);
void mutex_trylock_check(struct mutex *m);
void mutex_unlock_check(struct mutex *m);
void mutex_destroy_check(struct mutex *m);
// --------------------------------------------------------------
TAILQ_HEAD(mutex_head, mutex);

void mutex_init(struct mutex *m);
void mutex_destroy(struct mutex *m);

void mutex_init_recursive(struct recursive_mutex *m);
void mutex_destroy_recursive(struct recursive_mutex *m);
unsigned int mutex_get_recursive_lock_depth(struct recursive_mutex *m);

void mutex_unlock(struct mutex *m);
void mutex_lock(struct mutex *m);
bool mutex_trylock(struct mutex *m);
void mutex_read_unlock(struct mutex *m);
void mutex_read_lock(struct mutex *m);
bool mutex_read_trylock(struct mutex *m);

void mutex_unlock_recursive(struct recursive_mutex *m);
void mutex_lock_recursive(struct recursive_mutex *m);
// --------------------------------------------------------------
struct condition {
    unsigned int spinlock;
    struct mutex *mtx;
};

#define CONDITION_INITIALIZER { .mtx = NULL }

void condition_init(struct condition *cv);
void condition_destroy(struct condition *cv);
void condition_signal(struct condition *cv);
void condition_broadcast(struct condition *cv);
void condition_wait(struct condition *cv, struct mutex *m);
// --------------------------------------------------------------
#endif /* _BSP_MUTEX_H */
