/**
 * Hustler's Project
 *
 * File:  cpu.h
 * Date:  2024/05/21
 * Usage:
 */

#ifndef _BSP_CPU_H
#define _BSP_CPU_H
// --------------------------------------------------------------

#ifdef __RK3568__
#include <rk3568/board.h>
#endif

#define NR_CPUS  NR_BOARD_CPU

#include <bsp/config.h>
#include <lib/list.h>

/* --------------------------------------------------------------
 * CPU NOTIFIER CHAINS
 * --------------------------------------------------------------
 */
struct notifier_block {
    int (*notifier_call)(struct notifier_block *nfb,
                         unsigned long action,
                         void *hcpu);
    struct list_head chain;
    int priority;
};

struct notifier_head {
    struct list_head head;
};

#define NOTIFIER_HEAD(name) \
    struct notifier_head name = { .head = LIST_HEAD_INIT((name).head) }

void notifier_chain_register(
    struct notifier_head *nh, struct notifier_block *n);
void notifier_chain_unregister(
    struct notifier_head *nh, struct notifier_block *n);

int notifier_call_chain(
    struct notifier_head *nh, unsigned long val, void *v,
    struct notifier_block **pcursor);

/* Notifier flag values: OR into @val passed to notifier_call_chain().
 */
#define NOTIFY_FORWARD 0x0000 /* Call chain highest-priority-first */
#define NOTIFY_REVERSE 0x8000 /* Call chain lowest-priority-first */

/* Handler completion values */
#define NOTIFY_DONE      0x0000
#define NOTIFY_STOP_MASK 0x8000
#define NOTIFY_STOP      (NOTIFY_STOP_MASK|NOTIFY_DONE)
#define NOTIFY_BAD       (NOTIFY_STOP_MASK|EINVAL)

/* Encapsulate (negative) errno value. */
static inline int notifier_from_errno(int err)
{
    return err ? (NOTIFY_STOP_MASK | -err) : NOTIFY_DONE;
}

/* Restore (negative) errno value from notify return value. */
static inline int notifier_to_errno(int ret)
{
    return (ret == NOTIFY_DONE) ? 0 : -(ret & ~NOTIFY_STOP_MASK);
}
// --------------------------------------------------------------


// --------------------------------------------------------------
#endif /* _BSP_CPU_H */
