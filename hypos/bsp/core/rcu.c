/**
 * Hustler's Project
 *
 * File:  rcu.c
 * Date:  2024/07/11
 * Usage:
 */

#include <org/section.h>
#include <org/smp.h>
#include <org/irq.h>
#include <bsp/softirq.h>
#include <asm/atomic.h>
#include <asm/barrier.h>
#include <bsp/rcu.h>
#include <bsp/cpu.h>
#include <bsp/time.h>
#include <bsp/timer.h>
#include <bsp/debug.h>
#include <bsp/config.h>
#include <bsp/spinlock.h>

#if IS_IMPLEMENTED(__RCU_IMPL)
// --------------------------------------------------------------
#define timer_is_expired(t) timer_expires_before(t, NOW())

DEFINE_PERCPU(unsigned int, rcu_lock_cnt);

static struct rcu_ctrlblk {
    long cur;
    long completed;
    int  next_pending;

    spinlock_t  lock __cacheline_aligned;
    cpumask_t   cpumask;
    cpumask_t   idle_cpumask;
} __cacheline_aligned rcu_ctrlblk = {
    .cur = -300,
    .completed = -300,
    .lock = SPIN_LOCK_UNLOCKED,
};

struct rcu_data {
    /* 1) quiescent state handling : */
    long quiescbatch;
    int  qs_pending;

    /* 2) batch handling */
    long            batch;
    struct rcu_head *nxtlist;
    struct rcu_head **nxttail;
    long            qlen;
    struct rcu_head *curlist;
    struct rcu_head **curtail;
    struct rcu_head *donelist;
    struct rcu_head **donetail;
    long            blimit;
    int cpu;
    long            last_rs_qlen;

    /* 3) idle CPUs handling */
    struct timer idle_timer;
    bool idle_timer_active;

    bool process_callbacks;
    bool barrier_active;
};

#define IDLE_TIMER_PERIOD_MAX     MILLISECS(100)
#define IDLE_TIMER_PERIOD_DEFAULT MILLISECS(10)
#define IDLE_TIMER_PERIOD_MIN     MICROSECS(100)

static stime_t __read_mostly idle_timer_period;

#define IDLE_TIMER_PERIOD_INCR    MILLISECS(10)
#define IDLE_TIMER_PERIOD_DECR    MICROSECS(100)

static DEFINE_PERCPU(struct rcu_data, rcu_data);

static int blimit     = 10;
static int qhimark    = 10000;
static int qlowmark   = 100;
static int rsinterval = 1000;

static atomic_t cpu_count     = ATOMIC_INIT(0);
static atomic_t pending_count = ATOMIC_INIT(0);

static void rcu_barrier_callback(struct rcu_head *head)
{
    smp_wmb();
    atomic_dec(&cpu_count);
}

static void rcu_barrier_action(void)
{
    struct rcu_head head;
    call_rcu(&head, rcu_barrier_callback);

    while (atomic_read(&cpu_count)) {
        process_pending_softirqs();
        cpu_relax();
    }

    smp_mb__before_atomic();
    atomic_dec(&pending_count);
}

void rcu_barrier(void)
{
    unsigned int n_cpus;

    ASSERT(!in_irq() && local_irq_is_enabled());

    for ( ; ; ) {
        if (!atomic_read(&pending_count) && get_cpu_maps()) {
            n_cpus = num_online_cpus();

            if (atomic_cmpxchg(&pending_count, 0,
                        n_cpus + 1) == 0)
                break;

            put_cpu_maps();
        }

        process_pending_softirqs();
        cpu_relax();
    }

    atomic_set(&cpu_count, n_cpus);
    cpumask_raise_softirq(&cpu_online_map, RCU_SOFTIRQ);

    while (atomic_read(&pending_count) != 1) {
        process_pending_softirqs();
        cpu_relax();
    }

    atomic_set(&pending_count, 0);

    put_cpu_maps();
}

/* Is batch a before batch b ? */
static inline int rcu_batch_before(long a, long b)
{
    return (a - b) < 0;
}

static void force_quiescent_state(struct rcu_data *rdp,
                                  struct rcu_ctrlblk *rcp)
{
    cpumask_t cpumask;
    raise_softirq(RCU_SOFTIRQ);
    if (unlikely(rdp->qlen - rdp->last_rs_qlen > rsinterval)) {
        rdp->last_rs_qlen = rdp->qlen;

        cpumask_andnot(&cpumask, &rcp->cpumask,
                cpumask_of(rdp->cpu));
        cpumask_raise_softirq(&cpumask, RCU_SOFTIRQ);
    }
}

void call_rcu(struct rcu_head *head,
              void (*func)(struct rcu_head *rcu))
{
    unsigned long flags;
    struct rcu_data *rdp;

    head->func = func;
    head->next = NULL;
    local_irq_save(flags);
    rdp = &this_cpu(rcu_data);
    *rdp->nxttail = head;
    rdp->nxttail = &head->next;
    if (unlikely(++rdp->qlen > qhimark)) {
        rdp->blimit = INT_MAX;
        force_quiescent_state(rdp, &rcu_ctrlblk);
    }
    local_irq_restore(flags);
}

static void rcu_do_batch(struct rcu_data *rdp)
{
    struct rcu_head *next, *list;
    int count = 0;

    list = rdp->donelist;
    while (list) {
        next = rdp->donelist = list->next;
        list->func(list);
        list = next;
        rdp->qlen--;
        if (++count >= rdp->blimit)
            break;
    }
    if (rdp->blimit == INT_MAX && rdp->qlen <= qlowmark)
        rdp->blimit = blimit;
    if (!rdp->donelist)
        rdp->donetail = &rdp->donelist;
    else {
        rdp->process_callbacks = true;
        raise_softirq(RCU_SOFTIRQ);
    }
}

static void rcu_start_batch(struct rcu_ctrlblk *rcp)
{
    if (rcp->next_pending &&
        rcp->completed == rcp->cur) {
        rcp->next_pending = 0;

        smp_wmb();
        rcp->cur++;

        smp_mb();
        cpumask_andnot(&rcp->cpumask, &cpu_online_map,
                &rcp->idle_cpumask);
    }
}

static void cpu_quiet(int cpu, struct rcu_ctrlblk *rcp)
{
    cpumask_clear_cpu(cpu, &rcp->cpumask);
    if (cpumask_empty(&rcp->cpumask)) {
        /* batch completed ! */
        rcp->completed = rcp->cur;
        rcu_start_batch(rcp);
    }
}

static void rcu_check_quiescent_state(struct rcu_ctrlblk *rcp,
                                      struct rcu_data *rdp)
{
    if (rdp->quiescbatch != rcp->cur) {
        /* start new grace period: */
        rdp->qs_pending = 1;
        rdp->quiescbatch = rcp->cur;
        return;
    }

    if (!rdp->qs_pending)
        return;

    rdp->qs_pending = 0;

    spin_lock(&rcp->lock);

    if (likely(rdp->quiescbatch == rcp->cur))
        cpu_quiet(rdp->cpu, rcp);

    spin_unlock(&rcp->lock);
}

static void __rcu_process_callbacks(struct rcu_ctrlblk *rcp,
                                    struct rcu_data *rdp)
{
    if (rdp->curlist && !rcu_batch_before(rcp->completed,
                rdp->batch)) {
        *rdp->donetail = rdp->curlist;
        rdp->donetail = rdp->curtail;
        rdp->curlist = NULL;
        rdp->curtail = &rdp->curlist;
    }

    local_irq_disable();

    if (rdp->nxtlist && !rdp->curlist) {
        rdp->curlist = rdp->nxtlist;
        rdp->curtail = rdp->nxttail;
        rdp->nxtlist = NULL;
        rdp->nxttail = &rdp->nxtlist;
        local_irq_enable();

        rdp->batch = rcp->cur + 1;

        smp_rmb();

        if (!rcp->next_pending) {
            spin_lock(&rcp->lock);
            rcp->next_pending = 1;
            rcu_start_batch(rcp);
            spin_unlock(&rcp->lock);
        }
    } else {
        local_irq_enable();
    }

    rcu_check_quiescent_state(rcp, rdp);

    if (rdp->donelist)
        rcu_do_batch(rdp);
}

static void rcu_process_callbacks(void)
{
    struct rcu_data *rdp = &this_cpu(rcu_data);

    if (rdp->process_callbacks) {
        rdp->process_callbacks = false;
        __rcu_process_callbacks(&rcu_ctrlblk, rdp);
    }

    if (atomic_read(&cpu_count) && !rdp->barrier_active) {
        rdp->barrier_active = true;
        rcu_barrier_action();
        rdp->barrier_active = false;
    }
}

static int __rcu_pending(struct rcu_ctrlblk *rcp,
                         struct rcu_data *rdp)
{
    if (rdp->curlist && !rcu_batch_before(rcp->completed,
                rdp->batch))
        return 1;

    if (!rdp->curlist && rdp->nxtlist)
        return 1;

    if (rdp->donelist)
        return 1;

    if (rdp->quiescbatch != rcp->cur || rdp->qs_pending)
        return 1;

    return 0;
}

int rcu_pending(int cpu)
{
    return __rcu_pending(&rcu_ctrlblk, &percpu(rcu_data, cpu));
}

int rcu_needs_cpu(int cpu)
{
    struct rcu_data *rdp = &percpu(rcu_data, cpu);

    return (rdp->curlist && !rdp->idle_timer_active) || rcu_pending(cpu);
}

static void rcu_idle_timer_start(void)
{
    struct rcu_data *rdp = &this_cpu(rcu_data);

    if (likely(!rdp->curlist))
        return;

    set_timer(&rdp->idle_timer, NOW() + idle_timer_period);
    rdp->idle_timer_active = true;
}

static void rcu_idle_timer_stop(void)
{
    struct rcu_data *rdp = &this_cpu(rcu_data);

    if (likely(!rdp->idle_timer_active))
        return;

    rdp->idle_timer_active = false;

    if (!timer_is_expired(&rdp->idle_timer))
        stop_timer(&rdp->idle_timer);
}

static void rcu_idle_timer_handler(void* data)
{
    if (!cpumask_empty(&rcu_ctrlblk.cpumask))
        idle_timer_period =
            min(idle_timer_period + IDLE_TIMER_PERIOD_INCR,
                IDLE_TIMER_PERIOD_MAX);
    else
        idle_timer_period =
            max(idle_timer_period - IDLE_TIMER_PERIOD_DECR,
                IDLE_TIMER_PERIOD_MIN);
}

void rcu_check_callbacks(int cpu)
{
    struct rcu_data *rdp = &this_cpu(rcu_data);

    rdp->process_callbacks = true;
    raise_softirq(RCU_SOFTIRQ);
}

static void rcu_move_batch(struct rcu_data *this_rdp,
                           struct rcu_head *list,
                           struct rcu_head **tail)
{
    local_irq_disable();
    *this_rdp->nxttail = list;
    if (list)
        this_rdp->nxttail = tail;
    local_irq_enable();
}

static void rcu_offline_cpu(struct rcu_data *this_rdp,
                            struct rcu_ctrlblk *rcp,
                            struct rcu_data *rdp)
{
    kill_timer(&rdp->idle_timer);

    spin_lock(&rcp->lock);
    if (rcp->cur != rcp->completed)
        cpu_quiet(rdp->cpu, rcp);
    spin_unlock(&rcp->lock);

    rcu_move_batch(this_rdp, rdp->donelist, rdp->donetail);
    rcu_move_batch(this_rdp, rdp->curlist, rdp->curtail);
    rcu_move_batch(this_rdp, rdp->nxtlist, rdp->nxttail);

    local_irq_disable();
    this_rdp->qlen += rdp->qlen;
    local_irq_enable();
}

static void rcu_init_percpu_data(int cpu,
                                 struct rcu_ctrlblk *rcp,
                                 struct rcu_data *rdp)
{
    memset(rdp, 0, sizeof(*rdp));
    rdp->curtail = &rdp->curlist;
    rdp->nxttail = &rdp->nxtlist;
    rdp->donetail = &rdp->donelist;
    rdp->quiescbatch = rcp->completed;
    rdp->qs_pending = 0;
    rdp->cpu = cpu;
    rdp->blimit = blimit;
    init_timer(&rdp->idle_timer, rcu_idle_timer_handler, rdp, cpu);
}

static int cpu_callback(
    struct notifier_block *nfb, unsigned long action, void *hcpu)
{
    unsigned int cpu = (unsigned long)hcpu;
    struct rcu_data *rdp = &percpu(rcu_data, cpu);

    switch (action) {
    case CPU_UP_PREPARE:
        rcu_init_percpu_data(cpu, &rcu_ctrlblk, rdp);
        break;
    case CPU_UP_CANCELED:
    case CPU_DEAD:
        rcu_offline_cpu(&this_cpu(rcu_data), &rcu_ctrlblk, rdp);
        break;
    default:
        break;
    }

    return NOTIFY_DONE;
}

static struct notifier_block cpu_nfb = {
    .notifier_call = cpu_callback
};

void __bootfunc rcu_init(void)
{
    void *cpu = (void *)(long)smp_processor_id();
    static unsigned int __initdata idle_timer_period_ms =
                IDLE_TIMER_PERIOD_DEFAULT / MILLISECS(1);

    if (idle_timer_period_ms == 0 ||
        idle_timer_period_ms > IDLE_TIMER_PERIOD_MAX / MILLISECS(1)) {
        idle_timer_period_ms = IDLE_TIMER_PERIOD_DEFAULT / MILLISECS(1);

        MSGH("rcu-idle-timer-period-ms outside of "
               "(0,%ld]. Resetting it to %u.\n",
               IDLE_TIMER_PERIOD_MAX / MILLISECS(1),
               idle_timer_period_ms);
    }
    idle_timer_period = MILLISECS(idle_timer_period_ms);

    cpumask_clear(&rcu_ctrlblk.idle_cpumask);
    cpu_callback(&cpu_nfb, CPU_UP_PREPARE, cpu);
    register_cpu_notifier(&cpu_nfb);
    open_softirq(RCU_SOFTIRQ, rcu_process_callbacks);
}

void rcu_idle_enter(unsigned int cpu)
{
    ASSERT(!cpumask_test_cpu(cpu, &rcu_ctrlblk.idle_cpumask));
    cpumask_set_cpu(cpu, &rcu_ctrlblk.idle_cpumask);

    smp_mb();

    rcu_idle_timer_start();
}

void rcu_idle_exit(unsigned int cpu)
{
    rcu_idle_timer_stop();
    ASSERT(cpumask_test_cpu(cpu, &rcu_ctrlblk.idle_cpumask));
    cpumask_clear_cpu(cpu, &rcu_ctrlblk.idle_cpumask);
}
// --------------------------------------------------------------
#endif
