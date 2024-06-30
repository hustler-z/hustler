/**
 * Hustler's Project
 *
 * File:  softirq.c
 * Date:  2024/06/27
 * Usage:
 */

#include <common/softirq.h>

// ------------------------------------------------------------------------

static void __do_softirq(unsigned long irqmask)
{

}

void do_softirq(void)
{
    __do_softirq(0);
}

void process_pending_softirqs(void)
{
    /* TODO */
}

// ------------------------------------------------------------------------
