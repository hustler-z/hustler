/**
 * Hustler's Project
 *
 * File:  setup.c
 * Date:  2024/05/20
 * Usage: general initialization
 */

#include <asm/ttbl.h>
#include <asm/setup.h>
#include <asm-generic/globl.h>
#include <bsp/process.h>
#include <bsp/alloc.h>
#include <bsp/cpu.h>
#include <bsp/sdev.h>
#include <bsp/percpu.h>
#include <bsp/console.h>
#include <generic/exit.h>
#include <generic/board.h>
#include <generic/gicv3.h>
#include <generic/memory.h>
#include <bsp/bootchain.h>

unsigned long __percpu_offset[NR_CPU];

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
    arch_setup,

    board_setup,

    /* Set up page table
     */
    ttbl_setup,

    /* Interrupt Controller Setup
     */
    gicv3_setup,

    timer_setup,

    /* Hypos memory Setup
     */
    mem_setup,

    /* should set up glb early before any use cases;
     */
    glb_setup,

    /* Peripheral Setup
     */
    device_setup,

    console_setup,
};

void bsp_setup(unsigned long phys_offset,
        unsigned long boot_args)
{
    /* Normal booting process, initiate hypos services. my
     * goal here is to implement basic console and be able
     * to run basic virtual machine on this by executing a
     * simple command.
     */
    if (bsp_bootchain(hypos_boot_sequence))
        hang();
}
