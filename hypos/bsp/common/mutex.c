/**
 * Hustler's Project
 *
 * File:  mutex.c
 * Date:  2024/06/04
 * Usage: Mutex Implementation
 */

#include <bsp/mutex.h>
#include <bsp/wq.h>
#include <bsp/check.h>
#include <bsp/lockdep.h>
#include <bsp/alloc.h>
#include <bsp/debug.h>
#include <bsp/cpu.h>
#include <common/errno.h>
#include <asm-generic/spinlock.h>
#include <lib/strops.h>
#include <common/refcount.h>

// --------------------------------------------------------------
void mutex_init(struct mutex *m)
{
    *m = (struct mutex)MUTEX_INITIALIZER;
}

void mutex_init_recursive(struct recursive_mutex *m)
{
    *m = (struct recursive_mutex)RECURSIVE_MUTEX_INITIALIZER;
}

static void __mutex_lock(struct mutex *m, const char *fname,
                         int lineno)
{
    assert_have_no_spinlock();
    ASSERT(thread_get_id() != THREAD_ID_INVALID);
    ASSERT(thread_is_in_normal_mode());

    mutex_lock_check(m);

    while (true) {
        u32 old_itr_status;
        bool can_lock;
        struct wait_queue_elem wqe;

        old_itr_status = spinlock_xsave(&m->spinlock);

        can_lock = !m->state;
        if (!can_lock)
            wq_wait_init(&m->wq, &wqe, false);
        else
            m->state = -1;

        spinunlock_xrestore(&m->spinlock, old_itr_status);

        if (!can_lock)
            wq_wait_final(&m->wq, &wqe, m, fname, lineno);
        else
            return;
    }
}

static void __mutex_lock_recursive(struct recursive_mutex *m,
                                   const char *fname, int lineno)
{
    int ct = thread_get_id();

    assert_have_no_spinlock();
    ASSERT(thread_is_in_normal_mode());

    if (gnu_atomic_load_s32(&m->owner) == ct) {
        if (!refcount_inc(&m->lock_depth))
            BUG();
        return;
    }

    __mutex_lock(&m->m, fname, lineno);

    ASSERT(m->owner == THREAD_ID_INVALID);
    gnu_atomic_store_s32(&m->owner, ct);
    refcount_set(&m->lock_depth, 1);
}

static void __mutex_unlock(struct mutex *m, const char *fname,
                           int lineno)
{
    u32 old_itr_status;

    assert_have_no_spinlock();
    ASSERT(thread_get_id() != THREAD_ID_INVALID);

    mutex_unlock_check(m);

    old_itr_status = spinlock_xsave(&m->spinlock);

    if (!m->state)
        BUG();

    m->state = 0;

    spinunlock_xrestore(&m->spinlock, old_itr_status);

    wq_wake_next(&m->wq, m, fname, lineno);
}

static void __mutex_unlock_recursive(struct recursive_mutex *m,
				     const char *fname, int lineno)
{
    assert_have_no_spinlock();
    ASSERT(m->owner == thread_get_id());

    if (refcount_dec(&m->lock_depth)) {
        gnu_atomic_store_s32(&m->owner, THREAD_ID_INVALID);
        __mutex_unlock(&m->m, fname, lineno);
    }
}

static bool __mutex_trylock(struct mutex *m,
                            const char *fname, int lineno)
{
    u32 old_itr_status;
    bool can_lock_write;

    assert_have_no_spinlock();
    ASSERT(thread_get_id() != THREAD_ID_INVALID);

    old_itr_status = spinlock_xsave(&m->spinlock);

    can_lock_write = !m->state;
    if (can_lock_write)
        m->state = -1;

    spinunlock_xrestore(&m->spinlock, old_itr_status);

    if (can_lock_write)
        mutex_trylock_check(m);

    return can_lock_write;
}

static void __mutex_read_unlock(struct mutex *m,
                                const char *fname, int lineno)
{
    u32 old_itr_status;
    short new_state;

    assert_have_no_spinlock();
    ASSERT(thread_get_id() != THREAD_ID_INVALID);

    old_itr_status = spinlock_xsave(&m->spinlock);

    if (m->state <= 0)
        BUG();
    m->state--;
    new_state = m->state;

    spinunlock_xrestore(&m->spinlock, old_itr_status);

    /* Wake eventual waiters if the mutex was unlocked */
    if (!new_state)
        wq_wake_next(&m->wq, m, fname, lineno);
}

static void __mutex_read_lock(struct mutex *m,
                              const char *fname, int lineno)
{
    assert_have_no_spinlock();
    ASSERT(thread_get_id() != THREAD_ID_INVALID);
    ASSERT(thread_is_in_normal_mode());

    while (true) {
        u32 old_itr_status;
        bool can_lock;
        struct wait_queue_elem wqe;

        old_itr_status = spinlock_xsave(&m->spinlock);

        can_lock = m->state != -1;
        if (!can_lock)
            wq_wait_init(&m->wq, &wqe, true /* wait_read */);
        else
            m->state++;

        spinunlock_xrestore(&m->spinlock, old_itr_status);

        if (!can_lock)
            wq_wait_final(&m->wq, &wqe, m, fname, lineno);
        else
            return;
    }
}

static bool __mutex_read_trylock(struct mutex *m,
    const char *fname,
    int lineno)
{
    u32 old_itr_status;
    bool can_lock;

    assert_have_no_spinlock();
    ASSERT(thread_get_id() != THREAD_ID_INVALID);
    ASSERT(thread_is_in_normal_mode());

    old_itr_status = spinlock_xsave(&m->spinlock);

    can_lock = m->state != -1;
    if (can_lock)
        m->state++;

    spinunlock_xrestore(&m->spinlock, old_itr_status);

    return can_lock;
}
// --------------------------------------------------------------
void mutex_unlock(struct mutex *m)
{
    __mutex_unlock(m, NULL, -1);
}

void mutex_unlock_recursive(struct recursive_mutex *m)
{
    __mutex_unlock_recursive(m, NULL, -1);
}

void mutex_lock(struct mutex *m)
{
    __mutex_lock(m, NULL, -1);
}

void mutex_lock_recursive(struct recursive_mutex *m)
{
    __mutex_lock_recursive(m, NULL, -1);
}

bool mutex_trylock(struct mutex *m)
{
    return __mutex_trylock(m, NULL, -1);
}

void mutex_read_unlock(struct mutex *m)
{
    __mutex_read_unlock(m, NULL, -1);
}

void mutex_read_lock(struct mutex *m)
{
    __mutex_read_lock(m, NULL, -1);
}

bool mutex_read_trylock(struct mutex *m)
{
    return __mutex_read_trylock(m, NULL, -1);
}
// --------------------------------------------------------------
void mutex_destroy(struct mutex *m)
{
    if (m->state)
        BUG();
    if (!wq_is_empty(&m->wq))
        panic("waitqueue not empty");
    mutex_destroy_check(m);
}

void mutex_destroy_recursive(struct recursive_mutex *m)
{
    mutex_destroy(&m->m);
}

unsigned int mutex_get_recursive_lock_depth(struct recursive_mutex *m)
{
    assert_have_no_spinlock();
    ASSERT(m->owner == thread_get_id());

    return refcount_val(&m->lock_depth);
}

// --------------------------------------------------------------
#define NUM_THREADS                   (NR_CPUS)

static struct lockdep_node_head graph = TAILQ_HEAD_INITIALIZER(graph);
static DEFINE_SPINLOCK(graph_lock);
static struct lockdep_lock_head owned[NUM_THREADS];

void mutex_lockdep_init(void)
{
    int n = 0;

    for (n = 0; n < NUM_THREADS; n++)
        TAILQ_INIT(&owned[n]);

    MSGH("lockdep is enabled for mutexes\n");
}

void mutex_lock_check(struct mutex *m)
{
    int thread = thread_get_id();
    u64 exceptions = 0;

    exceptions = spinlock_xsave(&graph_lock);
    lockdep_lock_acquire(&graph, &owned[thread], (uptr_t)m);
    spinunlock_xrestore(&graph_lock, exceptions);
}

void mutex_trylock_check(struct mutex *m)
{
    int thread = thread_get_id();
    u64 exceptions = 0;

    exceptions = spinlock_xsave(&graph_lock);
    lockdep_lock_tryacquire(&graph, &owned[thread], (uptr_t)m);
    spinunlock_xrestore(&graph_lock, exceptions);
}

void mutex_unlock_check(struct mutex *m)
{
    int thread = thread_get_id();
    u64 exceptions = 0;

    exceptions = spinlock_xsave(&graph_lock);
    lockdep_lock_release(&owned[thread], (uptr_t)m);
    spinunlock_xrestore(&graph_lock, exceptions);
}

void mutex_destroy_check(struct mutex *m)
{
    u64 exceptions = spinlock_xsave(&graph_lock);

    lockdep_lock_destroy(&graph, (uptr_t)m);
    spinunlock_xrestore(&graph_lock, exceptions);
}

// --------------------------------------------------------------
void condition_init(struct condition *cv)
{
    *cv = (struct condition)CONDITION_INITIALIZER;
}

void condition_destroy(struct condition *cv)
{
    if (cv->mtx && wq_have_condition(&cv->mtx->wq, cv))
        BUG();

    condition_init(cv);
}

static void cv_signal(struct condition *cv, bool only_one, const char *fname,
			int lineno)
{
    u32 old_itr_status;
    struct mutex *m;

    old_itr_status = spinlock_xsave(&cv->spinlock);
    m = cv->mtx;
    spinunlock_xrestore(&cv->spinlock, old_itr_status);

    if (m)
        wq_promote_condition(&m->wq, cv, only_one, m, fname, lineno);
}

void condition_signal(struct condition *cv)
{
    cv_signal(cv, true /* only one */, NULL, -1);
}

void condition_broadcast(struct condition *cv)
{
    cv_signal(cv, false /* all */, NULL, -1);
}

static void __condition_wait(struct condition *cv, struct mutex *m,
			const char *fname, int lineno)
{
    u32 old_itr_status;
    struct wait_queue_elem wqe;
    int old_state;
    int new_state;

    mutex_unlock_check(m);

    /* Link this condition to this mutex until reinitialized */
    old_itr_status = spinlock_xsave(&cv->spinlock);
    if (cv->mtx && cv->mtx != m)
        panic("invalid mutex");

    cv->mtx = m;
    spinunlock(&cv->spinlock);

    spinlock(&m->spinlock);

    if (!m->state)
        BUG();
    old_state = m->state;
    /* Add to mutex wait queue as a condition waiter */
    wq_wait_init_condition(&m->wq, &wqe, cv, m->state > 0);

    if (m->state > 1) {
        /* Multiple read locks, remove one */
        m->state--;
    } else {
        /* Only one lock (read or write), unlock the mutex */
        m->state = 0;
    }
    new_state = m->state;

    spinunlock_xrestore(&m->spinlock, old_itr_status);

    /* Wake eventual waiters if the mutex was unlocked */
    if (!new_state)
        wq_wake_next(&m->wq, m, fname, lineno);

    wq_wait_final(&m->wq, &wqe, m, fname, lineno);

    if (old_state > 0)
        mutex_read_lock(m);
    else
        mutex_lock(m);
}

void condition_wait(struct condition *cv, struct mutex *m)
{
    __condition_wait(cv, m, NULL, -1);
}
// --------------------------------------------------------------

#define LOCKDEP_NODE_TEMP_MARK		BIT(0, UL)
#define LOCKDEP_NODE_PERM_MARK		BIT(1, UL)
#define LOCKDEP_NODE_BFS_VISITED	BIT(2, UL)

static struct lockdep_node *lockdep_add_to_graph(
                struct lockdep_node_head *graph,
                uptr_t lock_id)
{
    struct lockdep_node *node = NULL;

    ASSERT(graph);

    TAILQ_FOREACH(node, graph, link)
        if (node->lock_id == lock_id)
            return node;

    node = hcalloc(1, sizeof(*node));
    if (!node)
        return NULL;

    node->lock_id = lock_id;
    STAILQ_INIT(&node->edges);
    TAILQ_INSERT_TAIL(graph, node, link);

    return node;
}

static vaddr_t *dup_call_stack(vaddr_t *stack)
{
    vaddr_t *nstack = NULL;
    int n = 0;

    if (!stack)
        return NULL;

    while (stack[n])
        n++;

    nstack = halloc((n + 1) * sizeof(vaddr_t));
    if (!nstack)
        return NULL;

    memcpy(nstack, stack, (n + 1) * sizeof(vaddr_t));

    return nstack;
}

static void lockdep_print_call_stack(vaddr_t *stack)
{
    vaddr_t *p = NULL;

    MSGI("Call Stack:\n");
    for (p = stack; p && *p; p++)
        MSGI(" %016lx", *p);
}

static int lockdep_add_edge(struct lockdep_node *from,
                            struct lockdep_node *to,
                            vaddr_t *call_stack_from,
                            vaddr_t *call_stack_to,
                            uptr_t thread_id)
{
    struct lockdep_edge *edge = NULL;

    STAILQ_FOREACH(edge, &from->edges, link)
        if (edge->to == to)
            return 0;

    edge = hcalloc(1, sizeof(*edge));
    if (!edge)
        return -ENOMEM;
    edge->to = to;
    edge->call_stack_from = dup_call_stack(call_stack_from);
    edge->call_stack_to = dup_call_stack(call_stack_to);
    edge->thread_id = thread_id;
    STAILQ_INSERT_TAIL(&from->edges, edge, link);

    return 0;
}

struct lockdep_bfs {
    struct lockdep_node *node;
    uptr_t *path;
    int pathlen;
    TAILQ_ENTRY(lockdep_bfs) link;
};

TAILQ_HEAD(lockdep_bfs_head, lockdep_bfs);

static void lockdep_bfs_queue_delete(struct lockdep_bfs_head *queue)
{
    struct lockdep_bfs *cur = NULL;
    struct lockdep_bfs *next = NULL;

    TAILQ_FOREACH_SAFE(cur, queue, link, next) {
        TAILQ_REMOVE(queue, cur, link);
        hfree(cur->path);
        hfree(cur);
    }
}

static uptr_t *lockdep_graph_get_shortest_cycle(struct lockdep_node *node)
{
    struct lockdep_bfs_head queue;
    struct lockdep_bfs *qe = NULL;
    uptr_t *ret = NULL;
    size_t nlen;

    TAILQ_INIT(&queue);
    node->flags |= LOCKDEP_NODE_BFS_VISITED;

    qe = hcalloc(1, sizeof(*qe));
    if (!qe)
        goto out;
    qe->node = node;
    qe->path = halloc(sizeof(uptr_t));
    if (!qe->path)
        goto out;
    qe->path[0] = node->lock_id;
    qe->pathlen = 1;
    TAILQ_INSERT_TAIL(&queue, qe, link);

    while (!TAILQ_EMPTY(&queue)) {
        struct lockdep_node *n = NULL;
        struct lockdep_edge *e = NULL;

        qe = TAILQ_FIRST(&queue);
        n = qe->node;
        TAILQ_REMOVE(&queue, qe, link);

        STAILQ_FOREACH(e, &n->edges, link) {
            if (e->to->lock_id == node->lock_id) {
                uptr_t *tmp = NULL;
                nlen = qe->pathlen + 1;

                tmp = hrealloc(qe->path,
                          nlen * sizeof(uptr_t));
                if (!tmp) {
                    MSGH("<%s> Out of memory\n", __func__);
                    hfree(qe->path);
                    ret = NULL;
                    goto out;
                }
                qe->path = tmp;
                qe->path[nlen - 1] = 0;
                ret = qe->path;
                goto out;
            }

            if (!(e->to->flags & LOCKDEP_NODE_BFS_VISITED)) {
                nlen = 0;
                struct lockdep_bfs *nqe = NULL;

                e->to->flags |= LOCKDEP_NODE_BFS_VISITED;

                nlen = qe->pathlen + 1;
                nqe = hcalloc(1, sizeof(*nqe));
                if (!nqe)
                    goto out;
                nqe->node = e->to;
                nqe->path = halloc(nlen * sizeof(uptr_t));
                if (!nqe->path)
                    goto out;
                nqe->pathlen = nlen;
                memcpy(nqe->path, qe->path,
                       qe->pathlen * sizeof(uptr_t));
                nqe->path[nlen - 1] = e->to->lock_id;
                TAILQ_INSERT_TAIL(&queue, nqe, link);
            }
        }

        hfree(qe->path);
        hfree(qe);
        qe = NULL;
    }

out:
    hfree(qe);
    lockdep_bfs_queue_delete(&queue);
    return ret;
}

static int lockdep_visit(struct lockdep_node *node)
{
    struct lockdep_edge *e = NULL;
    int res;

    if (node->flags & LOCKDEP_NODE_PERM_MARK)
        return 0;

    if (node->flags & LOCKDEP_NODE_TEMP_MARK)
        return -ECRAP;

    node->flags |= LOCKDEP_NODE_TEMP_MARK;

    STAILQ_FOREACH(e, &node->edges, link) {
        res = lockdep_visit(e->to);
        if (res)
            return res;
    }

    node->flags |= LOCKDEP_NODE_PERM_MARK;

    return 0;
}

static int lockdep_graph_sort(struct lockdep_node_head *graph)
{
    struct lockdep_node *node = NULL;
    int res;

    TAILQ_FOREACH(node, graph, link) {
        if (!node->flags) {
            res = lockdep_visit(node);
            if (res)
                return res;
        }
    }

    TAILQ_FOREACH(node, graph, link)
        node->flags = 0;

    return 0;
}

static struct lockdep_edge *lockdep_find_edge(struct lockdep_node_head *graph,
					      uptr_t from, uptr_t to)
{
    struct lockdep_node *node = NULL;
    struct lockdep_edge *edge = NULL;

    TAILQ_FOREACH(node, graph, link)
        if (node->lock_id == from)
            STAILQ_FOREACH(edge, &node->edges, link)
                if (edge->to->lock_id == to)
                    return edge;
    return NULL;
}

static void lockdep_print_edge_info(uptr_t from __maybe_unused,
				    struct lockdep_edge *edge)
{
    uptr_t __maybe_unused to = edge->to->lock_id;
    const char __maybe_unused *at_msg = "";
    const char __maybe_unused *acq_msg = "";

    MSGI("-> Thread %016lx acquired lock %016lx %s",
         edge->thread_id, to, at_msg);
    lockdep_print_call_stack(edge->call_stack_to);
    MSGI("...while holding lock %016lx %s",
         from, acq_msg);
    lockdep_print_call_stack(edge->call_stack_from);
}

/*
 * Find cycle containing @node in the lock graph, then print full debug
 * information about each edge (thread that acquired the locks and call stacks)
 */
static void lockdep_print_cycle_info(struct lockdep_node_head *graph,
				     struct lockdep_node *node)
{
    struct lockdep_edge *edge = NULL;
    uptr_t *cycle = NULL;
    uptr_t *p = NULL;
    uptr_t from = 0;
    uptr_t to = 0;

    cycle = lockdep_graph_get_shortest_cycle(node);
    ASSERT(cycle && cycle[0]);

    MSGI("-> Shortest cycle:");
    for (p = cycle; *p; p++)
        MSGI(" Lock %016lx", *p);

    for (p = cycle; ; p++) {
        if (!*p) {
            ASSERT(p != cycle);
            from = to;
            to = cycle[0];
            edge = lockdep_find_edge(graph, from, to);
            lockdep_print_edge_info(from, edge);
            break;
        }
        if (p != cycle)
            from = to;
        to = *p;
        if (p != cycle) {
            edge = lockdep_find_edge(graph, from, to);
            lockdep_print_edge_info(from, edge);
        }
    }
    hfree(cycle);
}

int __lockdep_lock_acquire(struct lockdep_node_head *graph,
				  struct lockdep_lock_head *owned,
				  uptr_t id)
{
    struct lockdep_node *node = lockdep_add_to_graph(graph, id);
    struct lockdep_lock *lock = NULL;
    int res = 0;
    vaddr_t *acq_stack = NULL;

    if (!node)
        return -ENOMEM;

    TAILQ_FOREACH(lock, owned, link) {
        res = lockdep_add_edge(lock->node, node, lock->call_stack,
                       acq_stack, (uptr_t)owned);
        if (res)
            return res;
    }

    res = lockdep_graph_sort(graph);
    if (res) {
        MSGI("Potential deadlock detected!");
        MSGI("When trying to acquire lock 0x%016lx", id);
        lockdep_print_cycle_info(graph, node);
        return res;
    }

    lock = hcalloc(1, sizeof(*lock));
    if (!lock)
        return -ENOMEM;

    lock->node = node;
    lock->call_stack = acq_stack;
    TAILQ_INSERT_TAIL(owned, lock, link);

    return 0;
}

int __lockdep_lock_tryacquire(struct lockdep_node_head *graph,
				     struct lockdep_lock_head *owned,
				     uptr_t id)
{
    struct lockdep_node *node = lockdep_add_to_graph(graph, id);
    struct lockdep_lock *lock = NULL;
    vaddr_t *acq_stack = NULL;

    if (!node)
        return -ENOMEM;

    lock = hcalloc(1, sizeof(*lock));
    if (!lock)
        return -ENOMEM;

    lock->node = node;
    lock->call_stack = acq_stack;
    TAILQ_INSERT_TAIL(owned, lock, link);

    return 0;
}

int __lockdep_lock_release(struct lockdep_lock_head *owned, uptr_t id)
{
    struct lockdep_lock *lock = NULL;

    TAILQ_FOREACH_REVERSE(lock, owned, lockdep_lock_head, link) {
        if (lock->node->lock_id == id) {
            TAILQ_REMOVE(owned, lock, link);
            hfree(lock->call_stack);
            hfree(lock);

            return 0;
        }
    }

    MSGI("Thread %p does not own lock %016lx", (void *)owned, id);
    return -ENOFIND;
}

static void lockdep_free_edge(struct lockdep_edge *edge)
{
    hfree(edge->call_stack_from);
    hfree(edge->call_stack_to);
    hfree(edge);
}

static void lockdep_node_delete(struct lockdep_node *node)
{
    struct lockdep_edge *edge = NULL;
    struct lockdep_edge *next = NULL;

    STAILQ_FOREACH_SAFE(edge, &node->edges, link, next)
        lockdep_free_edge(edge);

    hfree(node);
}

void lockdep_graph_delete(struct lockdep_node_head *graph)
{
    struct lockdep_node *node = NULL;
    struct lockdep_node *next = NULL;

    TAILQ_FOREACH_SAFE(node, graph, link, next) {
        TAILQ_REMOVE(graph, node, link);
        lockdep_node_delete(node);
    }
}

void lockdep_queue_delete(struct lockdep_lock_head *owned)
{
    struct lockdep_lock *lock = NULL;
    struct lockdep_lock *next = NULL;

    TAILQ_FOREACH_SAFE(lock, owned, link, next) {
        TAILQ_REMOVE(owned, lock, link);
        hfree(lock);
    }
}

static void lockdep_node_destroy(struct lockdep_node_head *graph,
				 struct lockdep_node *node)
{
    struct lockdep_edge *edge = NULL;
    struct lockdep_edge *next = NULL;
    struct lockdep_node *from = NULL;

    TAILQ_REMOVE(graph, node, link);

    TAILQ_FOREACH(from, graph, link) {
        edge = STAILQ_FIRST(&from->edges);
        while (edge && edge->to == node) {
            STAILQ_REMOVE_HEAD(&from->edges, link);
            lockdep_free_edge(edge);
            edge = STAILQ_FIRST(&from->edges);
        }

        if (!edge)
            continue;

        next = STAILQ_NEXT(edge, link);
        while (next) {
            if (next->to == node) {
                STAILQ_REMOVE_AFTER(&from->edges, edge, link);
                lockdep_free_edge(next);
            } else {
                edge = next;
            }
            next = STAILQ_NEXT(edge, link);
        }
    }

    STAILQ_FOREACH_SAFE(edge, &node->edges, link, next)
        lockdep_free_edge(edge);

    hfree(node);
}

void lockdep_lock_destroy(struct lockdep_node_head *graph,
        uptr_t lock_id)
{
    struct lockdep_node *node = NULL;

    ASSERT(graph);

    TAILQ_FOREACH(node, graph, link) {
        if (node->lock_id == lock_id) {
            lockdep_node_destroy(graph, node);
            break;
        }
    }
}
// --------------------------------------------------------------
