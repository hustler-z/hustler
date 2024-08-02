/**
 * hustler's project
 *
 * file:  timer.c
 * date:  2024/05/22
 * usage:
 */

#include <org/section.h>
#include <org/globl.h>
#include <asm/barrier.h>
#include <bsp/softirq.h>
#include <bsp/time.h>
#include <bsp/rcu.h>
#include <bsp/timer.h>
#include <bsp/debug.h>
#include <bsp/config.h>
#include <bsp/memz.h>
#include <lib/math.h>
#include <lib/list.h>
#include <lib/strops.h>

#if IS_IMPLEMENTED(__TIMER_IMPL)
// --------------------------------------------------------------

static unsigned int timer_slop __read_mostly = 50000; /* 50 us */

struct timers {
    spinlock_t     lock;
    struct timer **heap;
    struct timer  *list;
    struct timer  *running;
    struct list_head inactive;
} __cacheline_aligned;

static DEFINE_PERCPU(struct timers, timers);

static DEFINE_RCU_READ_LOCK(timer_cpu_read_lock);

DEFINE_PERCPU(stime_t, timer_deadline);

struct heap_metadata {
    u16 size, limit;
};

static struct heap_metadata *heap_metadata(struct timer **heap)
{
    BUILD_BUG_ON(sizeof(struct heap_metadata)
                 > sizeof(struct timer *));

    return (struct heap_metadata *)&heap[0];
}

static void down_heap(struct timer **heap, unsigned int pos)
{
    unsigned int sz = heap_metadata(heap)->size, nxt;
    struct timer *t = heap[pos];

    while ((nxt = (pos << 1)) <= sz) {
        if (((nxt + 1) <= sz) && (heap[nxt+1]->expires
                                < heap[nxt]->expires))
            nxt++;
        if (heap[nxt]->expires > t->expires)
            break;
        heap[pos] = heap[nxt];
        heap[pos]->heap_offset = pos;
        pos = nxt;
    }

    heap[pos] = t;
    t->heap_offset = pos;
}

static void up_heap(struct timer **heap, unsigned int pos)
{
    struct timer *t = heap[pos];

    while ((pos > 1) && (t->expires < heap[pos >> 1]->expires)) {
        heap[pos] = heap[pos>>1];
        heap[pos]->heap_offset = pos;
        pos >>= 1;
    }

    heap[pos] = t;
    t->heap_offset = pos;
}

static int remove_from_heap(struct timer **heap,
                            struct timer *t)
{
    unsigned int sz = heap_metadata(heap)->size;
    unsigned int pos = t->heap_offset;

    if (unlikely(pos == sz)) {
        heap_metadata(heap)->size = sz - 1;
        goto out;
    }

    heap[pos] = heap[sz];
    heap[pos]->heap_offset = pos;

    heap_metadata(heap)->size = --sz;

    if ((pos > 1) && (heap[pos]->expires
                      < heap[pos >> 1]->expires))
        up_heap(heap, pos);
    else
        down_heap(heap, pos);

 out:
    return (pos == 1);
}

static int add_to_heap(struct timer **heap,
                       struct timer *t)
{
    unsigned int sz = heap_metadata(heap)->size;

    if (unlikely(sz == heap_metadata(heap)->limit))
        return 0;

    heap_metadata(heap)->size = ++sz;
    heap[sz] = t;
    t->heap_offset = sz;
    up_heap(heap, sz);

    return (t->heap_offset == 1);
}

static int remove_from_list(struct timer **pprev,
                            struct timer *t)
{
    struct timer *curr, **_pprev = pprev;

    while ((curr = *_pprev) != t)
        _pprev = &curr->list_next;

    *_pprev = t->list_next;

    return (_pprev == pprev);
}

static int add_to_list(struct timer **pprev, struct timer *t)
{
    struct timer *curr, **_pprev = pprev;

    while (((curr = *_pprev) != NULL) &&
           (curr->expires <= t->expires))
        _pprev = &curr->list_next;

    t->list_next = curr;
    *_pprev = t;

    return (_pprev == pprev);
}

static int remove_entry(struct timer *t)
{
    struct timers *timers = &percpu(timers, t->cpu);
    int rc;

    switch (t->status) {
    case TIMER_STATUS_IN_HEAP:
        rc = remove_from_heap(timers->heap, t);
        break;
    case TIMER_STATUS_IN_LIST:
        rc = remove_from_list(&timers->list, t);
        break;
    default:
        rc = 0;
        BUG();
    }

    t->status = TIMER_STATUS_INVALID;
    return rc;
}

static int add_entry(struct timer *t)
{
    struct timers *timers = &percpu(timers, t->cpu);
    int rc;

    ASSERT(t->status == TIMER_STATUS_INVALID);

    t->heap_offset = 0;
    t->status = TIMER_STATUS_IN_HEAP;
    rc = add_to_heap(timers->heap, t);
    if (t->heap_offset != 0)
        return rc;

    t->status = TIMER_STATUS_IN_LIST;
    return add_to_list(&timers->list, t);
}

static inline void activate_timer(struct timer *timer)
{
    ASSERT(timer->status == TIMER_STATUS_INACTIVE);
    timer->status = TIMER_STATUS_INVALID;
    list_del(&timer->inactive);

    if (add_entry(timer))
        cpu_raise_softirq(timer->cpu, TIMER_SOFTIRQ);
}

static inline void deactivate_timer(struct timer *timer)
{
    if (remove_entry(timer))
        cpu_raise_softirq(timer->cpu, TIMER_SOFTIRQ);

    timer->status = TIMER_STATUS_INACTIVE;
    list_add(&timer->inactive,
             &percpu(timers, timer->cpu).inactive);
}

static DEFINE_RCU_READ_LOCK(timer_lock);

static inline bool timer_lock_unsafe(struct timer *timer)
{
    unsigned int cpu;

    rcu_read_lock(&timer_lock);

    for ( ; ; ) {
        cpu = read_atomic(&timer->cpu);
        if (unlikely(cpu == TIMER_CPU_STATUS_KILLED)) {
            rcu_read_unlock(&timer_lock);
            return 0;
        }

        _spin_lock(&percpu(timers, cpu).lock);
        if (likely(timer->cpu == cpu))
            break;
        spin_unlock(&percpu(timers, cpu).lock);
    }

    rcu_read_unlock(&timer_lock);

    return 1;
}

#define timer_lock_irqsave(t, flags) ({         \
    bool __x;                                   \
    local_irq_save(flags);                      \
    if (!(__x = timer_lock_unsafe(t)))          \
        local_irq_restore(flags);               \
    __x;                                        \
})

static inline void timer_unlock(struct timer *timer)
{
    spin_unlock(&percpu(timers, timer->cpu).lock);
}

#define timer_unlock_irqrestore(t, flags) ({    \
    timer_unlock(t);                            \
    local_irq_restore(flags);                   \
})


static bool active_timer(const struct timer *timer)
{
    ASSERT(timer->status >= TIMER_STATUS_INACTIVE);
    return timer_is_active(timer);
}


void init_timer(
    struct timer *timer,
    void        (*function)(void *data),
    void         *data,
    unsigned int  cpu)
{
    unsigned long flags;

    memset(timer, 0, sizeof(*timer));
    timer->function = function;
    timer->data = data;
    write_atomic(&timer->cpu, cpu);
    timer->status = TIMER_STATUS_INACTIVE;
    if (!timer_lock_irqsave(timer, flags))
        BUG();
    list_add(&timer->inactive, &percpu(timers, cpu).inactive);
    timer_unlock_irqrestore(timer, flags);
}


void set_timer(struct timer *timer, stime_t expires)
{
    unsigned long flags;

    if (!timer_lock_irqsave(timer, flags))
        return;

    if (active_timer(timer))
        deactivate_timer(timer);

    timer->expires = expires;

    activate_timer(timer);

    timer_unlock_irqrestore(timer, flags);
}


void stop_timer(struct timer *timer)
{
    unsigned long flags;

    if (!timer_lock_irqsave(timer, flags))
        return;

    if (active_timer(timer))
        deactivate_timer(timer);

    timer_unlock_irqrestore(timer, flags);
}

bool timer_expires_before(struct timer *timer, stime_t t)
{
    unsigned long flags;
    bool ret;

    if (!timer_lock_irqsave(timer, flags))
        return false;

    ret = active_timer(timer) && timer->expires <= t;

    timer_unlock_irqrestore(timer, flags);

    return ret;
}

void migrate_timer(struct timer *timer, unsigned int new_cpu)
{
    unsigned int old_cpu;
    bool active;
    unsigned long flags;

    rcu_read_lock(&timer_lock);

    for ( ; ; ) {
        old_cpu = read_atomic(&timer->cpu);
        if ((old_cpu == new_cpu) ||
            (old_cpu == TIMER_CPU_STATUS_KILLED)) {
            rcu_read_unlock(&timer_lock);
            return;
        }

        if (old_cpu < new_cpu) {
            spin_lock_irqsave(&percpu(timers, old_cpu).lock, flags);
            spin_lock(&percpu(timers, new_cpu).lock);
        } else {
            spin_lock_irqsave(&percpu(timers, new_cpu).lock, flags);
            spin_lock(&percpu(timers, old_cpu).lock);
        }

        if (likely(timer->cpu == old_cpu))
             break;

        spin_unlock(&percpu(timers, old_cpu).lock);
        spin_unlock_irqrestore(&percpu(timers, new_cpu).lock, flags);
    }

    rcu_read_unlock(&timer_lock);

    active = active_timer(timer);
    if (active)
        deactivate_timer(timer);

    list_del(&timer->inactive);
    write_atomic(&timer->cpu, new_cpu);
    list_add(&timer->inactive, &percpu(timers, new_cpu).inactive);

    if (active)
        activate_timer(timer);

    spin_unlock(&percpu(timers, old_cpu).lock);
    spin_unlock_irqrestore(&percpu(timers, new_cpu).lock, flags);
}


void kill_timer(struct timer *timer)
{
    unsigned int old_cpu, cpu;
    unsigned long flags;

    BUG_ON(this_cpu(timers).running == timer);

    if (!timer_lock_irqsave(timer, flags))
        return;

    if (active_timer(timer))
        deactivate_timer(timer);

    list_del(&timer->inactive);
    timer->status = TIMER_STATUS_KILLED;
    old_cpu = timer->cpu;
    write_atomic(&timer->cpu, TIMER_CPU_STATUS_KILLED);

    spin_unlock_irqrestore(&percpu(timers, old_cpu).lock, flags);

    for_each_online_cpu(cpu)
        while (percpu(timers, cpu).running == timer)
            cpu_relax();
}


static void execute_timer(struct timers *ts, struct timer *t)
{
    void (*fn)(void *data) = t->function;
    void *data = t->data;

    t->status = TIMER_STATUS_INACTIVE;
    list_add(&t->inactive, &ts->inactive);

    ts->running = t;
    spin_unlock_irq(&ts->lock);
    (*fn)(data);
    spin_lock_irq(&ts->lock);
    ts->running = NULL;
}


static void timer_softirq_action(void)
{
    struct timer  *t, **heap, *next;
    struct timers *ts;
    stime_t       now, deadline;

    ts = &this_cpu(timers);
    heap = ts->heap;

    if (unlikely(ts->list != NULL)) {
        unsigned int old_limit = heap_metadata(heap)->limit;
        unsigned int new_limit = ((old_limit + 1) << 4) - 1;
        struct timer **newheap = NULL;

        if (new_limit == (typeof(heap_metadata(heap)->limit))new_limit &&
            new_limit + 1)
            newheap = malloc_array(struct timer *, new_limit + 1);
        else
            MSGO("cpu%u: timer heap limit reached\n",
                 smp_processor_id());
        if (newheap != NULL) {
            spin_lock_irq(&ts->lock);
            memcpy(newheap, heap, (old_limit + 1) * sizeof(*heap));
            heap_metadata(newheap)->limit = new_limit;
            ts->heap = newheap;
            spin_unlock_irq(&ts->lock);
            if (old_limit != 0)
                mfree(heap);
            heap = newheap;
        }
    }

    spin_lock_irq(&ts->lock);

    now = NOW();

    while ((heap_metadata(heap)->size != 0) &&
           ((t = heap[1])->expires < now)) {
        remove_from_heap(heap, t);
        execute_timer(ts, t);
    }

    while (((t = ts->list) != NULL) && (t->expires < now)) {
        ts->list = t->list_next;
        execute_timer(ts, t);
    }

    next = ts->list;
    ts->list = NULL;
    while (unlikely((t = next) != NULL)) {
        next = t->list_next;
        t->status = TIMER_STATUS_INVALID;
        add_entry(t);
    }

    deadline = STIME_MAX;
    if (heap_metadata(heap)->size != 0)
        deadline = heap[1]->expires;
    if ((ts->list != NULL) && (ts->list->expires < deadline))
        deadline = ts->list->expires;
    now = NOW();
    this_cpu(timer_deadline) =
        (deadline == STIME_MAX) ?
                0 : max(deadline, now + timer_slop);

    if (!reprogram_timer(this_cpu(timer_deadline)))
        raise_softirq(TIMER_SOFTIRQ);

    spin_unlock_irq(&ts->lock);
}

stime_t align_timer(stime_t firsttick, u64 period)
{
    if (!period)
        return firsttick;

    return firsttick + (period - 1) - ((firsttick - 1) % period);
}

static void dump_timer(struct timer *t, stime_t now)
{
    MSGI(BLANK_ALIGN"ex=%12ld us timer=%p cb=%ps(%p)\n",
         (t->expires - now) / 1000, t, t->function, t->data);
}

static void dump_timerq(unsigned char key)
{
    struct timer  *t;
    struct timers *ts;
    unsigned long  flags;
    stime_t now = NOW();
    unsigned int   i, j;

    MSGH("dumping timer queues:\n");

    for_each_online_cpu(i) {
        ts = &percpu(timers, i);

        MSGH("CPU%02d:\n", i);
        spin_lock_irqsave(&ts->lock, flags);
        for (j = 1; j <= heap_metadata(ts->heap)->size; j++)
            dump_timer(ts->heap[j], now);
        for (t = ts->list; t != NULL; t = t->list_next)
            dump_timer(t, now);
        spin_unlock_irqrestore(&ts->lock, flags);
    }
}

static void migrate_timers_from_cpu(unsigned int old_cpu)
{
    unsigned int new_cpu = cpumask_any(&cpu_online_map);
    struct timers *old_ts, *new_ts;
    struct timer *t;
    bool notify = false;

    ASSERT(!cpu_online(old_cpu) && cpu_online(new_cpu));

    old_ts = &percpu(timers, old_cpu);
    new_ts = &percpu(timers, new_cpu);

    if (old_cpu < new_cpu) {
        spin_lock_irq(&old_ts->lock);
        spin_lock(&new_ts->lock);
    } else {
        spin_lock_irq(&new_ts->lock);
        spin_lock(&old_ts->lock);
    }

    while ((t = heap_metadata(old_ts->heap)->size
           ? old_ts->heap[1] : old_ts->list) != NULL) {
        remove_entry(t);
        write_atomic(&t->cpu, new_cpu);
        notify |= add_entry(t);
    }

    while (!list_empty(&old_ts->inactive)) {
        t = list_entry(old_ts->inactive.next,
                       struct timer, inactive);
        list_del(&t->inactive);
        write_atomic(&t->cpu, new_cpu);
        list_add(&t->inactive, &new_ts->inactive);
    }

    spin_unlock(&old_ts->lock);
    spin_unlock_irq(&new_ts->lock);

    if (notify)
        cpu_raise_softirq(new_cpu, TIMER_SOFTIRQ);
}

static struct timer *dummy_heap[1];

static void free_percpu_timers(unsigned int cpu)
{
    struct timers *ts = &percpu(timers, cpu);

    ASSERT(heap_metadata(ts->heap)->size == 0);
    if (heap_metadata(ts->heap)->limit) {
        mfree(ts->heap);
        ts->heap = dummy_heap;
    } else
        ASSERT(ts->heap == dummy_heap);
}

static int cpu_callback(
    struct notifier_block *nfb, unsigned long action, void *hcpu)
{
    unsigned int cpu = (unsigned long)hcpu;
    struct timers *ts = &percpu(timers, cpu);

    switch (action) {
    case CPU_UP_PREPARE:
        if (!ts->heap) {
            INIT_LIST_HEAD(&ts->inactive);
            spin_lock_init(&ts->lock);
            ts->heap = dummy_heap;
        }
        break;
    case CPU_UP_CANCELED:
    case CPU_DEAD:
    case CPU_RESUME_FAILED:
        migrate_timers_from_cpu(cpu);
        if (get_globl()->hypos_status != HYPOS_SUSPEND_STATE)
            free_percpu_timers(cpu);
        break;
    case CPU_REMOVE:
        break;

    default:
        break;
    }

    return NOTIFY_DONE;
}

static struct notifier_block cpu_nfb = {
    .notifier_call = cpu_callback,
    .priority = 99
};

int __bootfunc timer_setup(void)
{
    void *cpu = (void *)(long)smp_processor_id();

    open_softirq(TIMER_SOFTIRQ, timer_softirq_action);

    cpu_callback(&cpu_nfb, CPU_UP_PREPARE, cpu);
    register_cpu_notifier(&cpu_nfb);

    return 0;
}
// --------------------------------------------------------------
#endif
