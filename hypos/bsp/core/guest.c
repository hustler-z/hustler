/**
 * Hustler's Project
 *
 * File:  guest.c
 * Date:  2024/05/22
 * Usage:
 */

#include <org/vtimer.h>
#include <org/vcpu.h>
#include <org/vgic.h>
#include <bsp/vm.h>

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
