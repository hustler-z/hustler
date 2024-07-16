/**
 * Hustler's Project
 *
 * File:  timer.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _BSP_TIMER_H
#define _BSP_TIMER_H
// --------------------------------------------------------------
#include <bsp/compiler.h>
#include <bsp/percpu.h>
#include <bsp/type.h>
#include <bsp/panic.h>
#include <lib/list.h>

struct timer {
    /* System time expiry value (nanoseconds since boot). */
    stime_t expires;

    /* Position in active-timer data structure. */
    union {
        /* Timer-heap offset (TIMER_STATUS_IN_HEAP). */
        unsigned int heap_offset;
        /* Linked list (TIMER_STATUS_in_list). */
        struct timer *list_next;
        /* Linked list of inactive timers (TIMER_STATUS_INACTIVE). */
        struct list_head inactive;
    };

    /* On expiry, '(*function)(data)' will be executed
     * in softirq context.
     */
    void (*function)(void *data);
    void *data;

/* CPU on which this timer will be installed and executed. */
#define TIMER_CPU_STATUS_KILLED 0xFFFFU
    u16 cpu;

    /* Timer status. */
#define TIMER_STATUS_INVALID  0
#define TIMER_STATUS_INACTIVE 1
#define TIMER_STATUS_KILLED   2
#define TIMER_STATUS_IN_HEAP  3
#define TIMER_STATUS_IN_LIST  4
    u8 status;
};

// --------------------------------------------------------------
void set_timer(struct timer *timer, stime_t expires);
void stop_timer(struct timer *timer);
void init_timer(
    struct timer *timer,
    void        (*function)(void *data),
    void         *data,
    unsigned int  cpu);
bool timer_expires_before(struct timer *timer, stime_t t);

void migrate_timer(struct timer *timer, unsigned int new_cpu);
void kill_timer(struct timer *timer);

static inline bool timer_is_active(const struct timer *timer)
{
    ASSERT(timer->status <= TIMER_STATUS_IN_LIST);
    return timer->status >= TIMER_STATUS_IN_HEAP;
}

DECLARE_PERCPU(stime_t, timer_deadline);
int reprogram_timer(stime_t timeout);
stime_t align_timer(stime_t firsttick, u64 period);
// --------------------------------------------------------------
int timer_setup(void);

// --------------------------------------------------------------
#endif /* _BSP_TIMER_H */
