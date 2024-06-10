/**
 * Hustler's Project
 *
 * File:  setup.c
 * Date:  2024/05/20
 * Usage: general initialization
 */

#include <asm-generic/globl.h>
#include <bsp/process.h>
#include <bsp/alloc.h>
#include <bsp/cpu.h>
#include <bsp/sdev.h>
#include <bsp/console.h>
#include <generic/exit.h>
#include <generic/board.h>
#include <bsp/bootchain.h>

/* TODO
 * --------------------------------------------------------------
 * Boot Sequence Control
 *
 * (a) glb             - hypos global tracker setup
 * (b) hypos device    - hypos basic device setup
 * (c)
 *
 * --------------------------------------------------------------
 */
static boot_func_t hypos_boot_sequence[] = {
    /* should set up glb early before any use cases;
     */
    glb_setup,

    /* TODO architectual boot flow
     */


    console_setup,
};

void bsp_setup(unsigned long phys_offset,
        unsigned long boot_args)
{
    /* Some basic setups before normal booting process.
     */
    board_setup();

    /* Normal booting process, initiate hypos services. my
     * goal here is to implement basic console and be able
     * to run basic virtual machine on this by executing a
     * simple command.
     */
    if (bsp_bootchain(hypos_boot_sequence))
        hang();
}
