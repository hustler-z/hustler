/**
 * Hustler's Project
 *
 * File:  period.h
 * Date:  2024/06/05
 * Usage:
 */

#ifndef _BSP_PERIOD_H
#define _BSP_PERIOD_H
// --------------------------------------------------------------

#include <bsp/type.h>
#include <lib/list.h>

struct periodic_work {
    void (*func)(void *context);
    void *context;
    char *name;
    u64 delay_us;
    u64 start_us;
    u64 cpu_us;
    u64 run_cnt;
    u64 next_call;
    struct hlist_node list;
    bool already_warned;
};

typedef void (*periodic_func_t)(void *context);

void schedule(void);

int periodw_setup(void);
// --------------------------------------------------------------
#endif /* _BSP_PERIOD_H */
