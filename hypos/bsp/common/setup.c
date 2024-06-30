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
#include <asm-generic/section.h>
#include <asm-generic/smp.h>
#include <asm-generic/bootmem.h>
#include <bsp/process.h>
#include <bsp/alloc.h>
#include <bsp/percpu.h>
#include <bsp/cpu.h>
#include <bsp/sdev.h>
#include <bsp/console.h>
#include <bsp/debug.h>
#include <bsp/period.h>
#include <bsp/vmap.h>
#include <common/exit.h>
#include <common/board.h>
#include <common/gicv3.h>
#include <common/memory.h>
#include <common/symtbl.h>
#include <core/core.h>

// --------------------------------------------------------------
static void hypos_tag(void)
{
    MSGI("              ___  __  ____ \n");
    MSGI("    /\\_/\\/\\/\\/ _ \\/  \\/ __/ \n");
    MSGI("    \\  _ \\  / ___/ / /\\__ \\ \n");
    MSGI("     \\/ \\/_/\\/   \\__/ /___/ @PIECE OF SHiT\n");
    MSGI("\n");
}

typedef int (*bootfunc_t)(void);

static int __bootfunc __bootchain(const bootfunc_t *boot_sequence)
{
    const bootfunc_t *boot_one;
    int ret, boot_count = 0;
    unsigned long base;

    hypos_tag();

    for (boot_one = boot_sequence; *boot_one; boot_one++) {
        ret = (*boot_one)();

        boot_count++;

        if (ret) {
            MSGH("Boot squence %p failed at call %p (err=%d)\n",
                    boot_sequence, (char *)(*boot_one), ret);
            return -1;
        } else
            MSGH("[BOOT] <Function %2d> Finished.\n",
                    boot_count);
    }

    return 0;
}

/* XXX:
 * --------------------------------------------------------------
 * Boot Sequence FLow:
 *
 * Sub boot function should follow 'bootfunc_t'.
 * --------------------------------------------------------------
 */
static bootfunc_t hypos_boot_sequence[] = {
    percpu_setup,

    /* Hypervisor CPU Setup
     */
    hcpu_setup,

    /* Set up Translation Table
     */
    ttbl_setup,

    /* Hypervisor Memory Chuncks for Boot-time Memory
     */
    bootmem_setup,

    /* Hypos VMAP Setup
     */
    vmap_setup,

    /* Interrupt Controller Setup
     */
    gicv3_setup,

    /* Generic Timer Setup
     */
    timer_setup,

    /* Bring up SMP CPUs
     */
    smp_setup,

    /* Board Setup
     */
    board_setup,

    /* Periodic Work List Setup
     */
    periodw_setup,

    /* Peripheral Setup
     */
    device_setup,

    /* Hypos Main Process
     */
    process_setup,

    /* Virtualization Setup
     */
    core_setup,

    /* Hypos Console Setup
     */
    console_setup,
};

void __bootfunc __setup(unsigned long phys_offset,
        unsigned long boot_args)
{
    early_debug("[hypos] Welcome to C world\n");

    get_globl()->phys_offset = phys_offset;
    get_globl()->boot_param = boot_args;

    /* Normal booting process, initiate hypos services. my
     * goal here is to implement basic console and be able
     * to run basic virtual machine on this by executing a
     * simple command.
     */
    if (__bootchain(hypos_boot_sequence))
        hang();
}
// --------------------------------------------------------------
