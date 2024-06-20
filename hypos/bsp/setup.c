/**
 * Hustler's Project
 *
 * File:  setup.c
 * Date:  2024/05/20
 * Usage: general initialization
 */

#include <asm/ttbl.h>
#include <asm/hcpu.h>
#include <asm/debug.h>
#include <asm-generic/globl.h>
#include <bsp/process.h>
#include <bsp/alloc.h>
#include <bsp/percpu.h>
#include <bsp/cpu.h>
#include <bsp/sdev.h>
#include <bsp/console.h>
#include <bsp/debug.h>
#include <bsp/period.h>
#include <generic/exit.h>
#include <generic/board.h>
#include <generic/gicv3.h>
#include <generic/memory.h>

unsigned long paddr_offset;

typedef int (*bootfunc_t)(void);

static int __bootchain(const bootfunc_t *boot_sequence)
{
    const bootfunc_t *boot_one;
    int ret;

    MSGH("Sequential %s() kicks\n", __func__);

    for (boot_one = boot_sequence; *boot_one; boot_one++) {
        ret = (*boot_one)();
        if (ret) {
            MSGH("Boot squence %p failed at call %p (err=%d)\n",
                    boot_sequence, (char *)(*boot_one), ret);
            return -1;
        }
    }

    return 0;
}

/* TODO
 * --------------------------------------------------------------
 * Boot Sequence Control
 *
 *
 * --------------------------------------------------------------
 */
static bootfunc_t hypos_boot_sequence[] = {
    percpu_setup,

    /* Hypervisor CPU Setup
     */
    hcpu_setup,

    /* Set up page table
     */
    ttbl_setup,

    /* Hypos Memory Setup
     */
    mem_setup,

    /* Interrupt Controller Setup
     */
    gicv3_setup,

    /* Generic Timer Setup
     */
    timer_setup,

    /* Board Setup
     */
    board_setup,

    /* Periodic Work List Setup
     */
    periodw_setup,

    /* Peripheral Setup
     */
    device_setup,

    /* Hypos Console Setup
     */
    console_setup,
};

void __setup(unsigned long phys_offset,
        unsigned long boot_args)
{
    early_debug("[hypos] Welcome to C world\n");

    paddr_offset = phys_offset;

    /* Normal booting process, initiate hypos services. my
     * goal here is to implement basic console and be able
     * to run basic virtual machine on this by executing a
     * simple command.
     */
    if (__bootchain(hypos_boot_sequence))
        hang();
}
