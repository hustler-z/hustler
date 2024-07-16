/**
 * Hustler's Project
 *
 * File:  tasklet.h
 * Date:  2024/07/11
 * Usage:
 */

#ifndef _BSP_TASKLET_H
#define _BSP_TASKLET_H
// --------------------------------------------------------------
#include <bsp/percpu.h>
#include <lib/list.h>

struct tasklet {
    struct list_head list;
    int scheduled_on;
    bool is_softirq;
    bool is_running;
    bool is_dead;
    void (*func)(void *data);
    void *data;
};

#define _DECLARE_TASKLET(name, func, data, softirq)                     \
    struct tasklet name = {                                             \
        LIST_HEAD_INIT((name).list), -1, softirq, 0, 0, func, data }
#define DECLARE_TASKLET(name, func, data)               \
    _DECLARE_TASKLET(name, func, data, 0)
#define DECLARE_SOFTIRQ_TASKLET(name, func, data)       \
    _DECLARE_TASKLET(name, func, data, 1)

/* Indicates status of tasklet work on each CPU. */
DECLARE_PERCPU(unsigned long, tasklet_work_to_do);
#define _TASKLET_ENQUEUED  0 /* Tasklet work is enqueued for this CPU. */
#define _TASKLET_SCHEDULED 1 /* Scheduler has scheduled do_tasklet(). */
#define TASKLET_ENQUEUED   (1UL << _TASKLET_ENQUEUED)
#define TASKLET_SCHEDULED  (1UL << _TASKLET_SCHEDULED)

static inline bool tasklet_work_to_do(unsigned int cpu)
{
    return percpu(tasklet_work_to_do, cpu) == (TASKLET_ENQUEUED |
                                               TASKLET_SCHEDULED);
}

static inline bool tasklet_is_scheduled(const struct tasklet *t)
{
    return t->scheduled_on != -1;
}

void tasklet_schedule_on_cpu(struct tasklet *t, unsigned int cpu);
void tasklet_schedule(struct tasklet *t);
void do_tasklet(void);
void tasklet_kill(struct tasklet *t);
void tasklet_init(struct tasklet *t, void (*func)(void *data), void *data);
void softirq_tasklet_init(struct tasklet *t,
                          void (*func)(void *data), void *data);
int tasklet_setup(void);
// --------------------------------------------------------------
#endif /* _BSP_TASKLET_H */
