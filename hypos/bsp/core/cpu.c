/**
 * Hustler's Project
 *
 * File:  cpu.c
 * Date:  2024/07/10
 * Usage:
 */

#include <bsp/cpu.h>
#include <org/section.h>
// --------------------------------------------------------------
#if IS_IMPLEMENTED(__NOTIFIER_CHAIN_IMPL)
// --------------------------------------------------------------
void __bootfunc notifier_chain_register(
    struct notifier_head *nh, struct notifier_block *n)
{
    struct list_head *chain = &nh->head;
    struct notifier_block *nb;

    while (chain->next != &nh->head) {
        nb = list_entry(chain->next, struct notifier_block, chain);
        if (n->priority > nb->priority)
            break;
        chain = chain->next;
    }

    list_add(&n->chain, chain);
}

void __bootfunc notifier_chain_unregister(
    struct notifier_head *nh, struct notifier_block *n)
{
    list_del(&n->chain);
}

int notifier_call_chain(
    struct notifier_head *nh, unsigned long val, void *v,
    struct notifier_block **pcursor)
{
    int ret = NOTIFY_DONE;
    struct list_head *cursor;
    struct notifier_block *nb = NULL;
    bool reverse = val & NOTIFY_REVERSE;

    cursor = pcursor && *pcursor ? &(*pcursor)->chain : &nh->head;

    do {
        cursor = reverse ? cursor->prev : cursor->next;
        if (cursor == &nh->head)
            break;
        nb = list_entry(cursor, struct notifier_block, chain);
        ret = nb->notifier_call(nb, val, v);
    } while (!(ret & NOTIFY_STOP_MASK));

    if (pcursor)
        *pcursor = nb;

    return ret;
}
#endif /* __CPU_NOTIFIER_CHAIN_IMPL */
// --------------------------------------------------------------
