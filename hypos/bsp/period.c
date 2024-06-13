/**
 * Hustler's Project
 *
 * File:  period.c
 * Date:  2024/06/05
 * Usage:
 */

#include <asm-generic/globl.h>
#include <generic/time.h>
#include <generic/timer.h>

#include <bsp/period.h>
#include <bsp/alloc.h>
#include <bsp/debug.h>
#include <lib/strops.h>
// --------------------------------------------------------------
#define PERIODIC_MAX_CPU_US         1000

extern struct hypos_globl *glb;

struct hlist_head *get_pw_global_list(void)
{
    return (struct hlist_head *)&glb->pw_list;
}

struct periodic_work *periodic_work_register(
        periodic_func_t func, u64 delay_us,
        const char *name, void *context)
{
    struct periodic_work *work;

    work = bcalloc(1, sizeof(struct periodic_work));
    if (!work) {
        DEBUG("Memory allocation error\n");
        return NULL;
    }

    /* Store values in struct */
    work->func = func;
    work->context = context;
    work->name = strdup(name);
    work->delay_us = delay_us;
    work->start_us = timer_get_us();
	hlist_add_head(&work->list, get_pw_global_list());

    return work;
}

int periodic_work_unregister(struct periodic_work *work)
{
    hlist_del(&work->list);
    bfree(work);

    return 0;
}

void periodic_work_run(void)
{
    struct periodic_work *work;
    struct hlist_node *tmp;
    u64 now, cpu_time;

    /* Prevent recursion */
    if (glb->flags & GLB_PERIODIC_RUNNING)
        return;

    glb->flags |= GLB_PERIODIC_RUNNING;
    hlist_for_each_entry_safe(work, tmp, get_pw_global_list(), list) {
        /*
         * Check if this work function needs to get called, e.g.
         * do not call the work func too often
         */
        now = timer_get_us();
        if (time_after_eq64(now, work->next_call)) {
            /* Call work function and account it's cpu-time */
            work->next_call = now + work->delay_us;
            work->func(work->context);
            work->run_cnt++;
            cpu_time = timer_get_us() - now;
            work->cpu_us += cpu_time;

            if ((cpu_time > PERIODIC_MAX_CPU_US) &&
                (!work->already_warned)) {
                DEBUG("periodic work %s took too long: %ldus vs %dus max\n",
                       work->name, cpu_time,
                       PERIODIC_MAX_CPU_US);

                work->already_warned = true;
            }
        }
    }
    glb->flags &= ~GLB_PERIODIC_RUNNING;
}

void schedule(void)
{
    if (glb_is_initialized())
        periodic_work_run();
}

int periodic_work_unregister_all(void)
{
    struct periodic_work *work;
    struct hlist_node *tmp;

    hlist_for_each_entry_safe(work, tmp, get_pw_global_list(), list)
        periodic_work_unregister(work);

    return 0;
}
// --------------------------------------------------------------
