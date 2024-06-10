/**
 * Hustler's Project
 *
 * File:  kickvm.c
 * Date:  2024/05/22
 * Usage:
 */

#include <hyp/vm.h>
#include <hyp/vcpu.h>
#include <hyp/vgic.h>
#include <hyp/vtimer.h>

// --------------------------------------------------------------
static int kick_guests_up(void)
{
    return 0;
}

void force_kick_guests_up(void)
{
    /* TODO
     */
    if (kick_guests_up()) {

    }
}

void man_kick_guests_up(void)
{
    /* TODO
     */
    if (kick_guests_up()) {

    }
}
// --------------------------------------------------------------
