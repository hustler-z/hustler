/**
 * Hustler's Project
 *
 * File:  rcu.h
 * Date:  2024/07/11
 * Usage:
 */

#ifndef _BSP_RCU_H
#define _BSP_RCU_H
// --------------------------------------------------------------
#include <bsp/panic.h>
#include <bsp/percpu.h>
#include <bsp/preempt.h>
#include <asm/barrier.h>

DECLARE_PERCPU(unsigned int, rcu_lock_cnt);

static inline void rcu_quiesce_disable(void)
{
    preempt_disable();
    this_cpu(rcu_lock_cnt)++;
    barrier();
}

static inline void rcu_quiesce_enable(void)
{
    barrier();
    this_cpu(rcu_lock_cnt)--;
    preempt_enable();
}

static inline bool rcu_quiesce_allowed(void)
{
    return !this_cpu(rcu_lock_cnt);
}

struct rcu_head {
    struct rcu_head *next;
    void (*func)(struct rcu_head *head);
};

#define rcu_dereference(p)   (p)
#define rcu_assign_pointer(p, v) ({ smp_wmb(); (p) = (v); })

#define RCU_HEAD_INIT   { .next = NULL, .func = NULL }
#define RCU_HEAD(head) struct rcu_head head = RCU_HEAD_INIT
#define INIT_RCU_HEAD(ptr)  \
    do {                    \
        (ptr)->next = NULL; \
        (ptr)->func = NULL; \
    } while (0)

struct _rcu_read_lock {};
typedef struct _rcu_read_lock  rcu_read_lock_t;
#define DEFINE_RCU_READ_LOCK(x) rcu_read_lock_t x
#define RCU_READ_LOCK_INIT(x)

int rcu_pending(int cpu);
int rcu_needs_cpu(int cpu);

static inline void rcu_read_lock(void)
{
    rcu_quiesce_disable();
}

static inline void rcu_read_unlock(void)
{
    ASSERT(!rcu_quiesce_allowed());
    rcu_quiesce_enable();
}

void rcu_init(void);
void rcu_check_callbacks(int cpu);

void call_rcu(struct rcu_head *head,
              void (*func)(struct rcu_head *head));

void rcu_barrier(void);

void rcu_idle_enter(unsigned int cpu);
void rcu_idle_exit(unsigned int cpu);
// --------------------------------------------------------------
#endif /* _BSP_RCU_H */
