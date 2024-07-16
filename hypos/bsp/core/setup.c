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
#include <org/globl.h>
#include <org/section.h>
#include <org/membank.h>
#include <org/time.h>
#include <org/vcpu.h>
#include <asm/at.h>
#include <bsp/hypmem.h>
#include <bsp/percpu.h>
#include <bsp/cpu.h>
#include <bsp/sdev.h>
#include <bsp/console.h>
#include <bsp/debug.h>
#include <bsp/period.h>
#include <bsp/vmap.h>
#include <bsp/board.h>
#include <bsp/exit.h>
#include <bsp/board.h>
#include <bsp/symtbl.h>
#include <bsp/tasklet.h>

// --------------------------------------------------------------
static void hypos_tag(void)
{
    MSGI("              ___  __  ____ \n");
    MSGI("    /\\_/\\/\\/\\/ _ \\/  \\/ __/ \n");
    MSGI("    \\  _ \\  / ___/ / /\\__ \\ \n");
    MSGI("     \\/ \\/_/\\/   \\__/ /___/ @_<\n");
    MSGI("\n");
}

typedef int (*bootfunc_t)(void);

static int __bootfunc __bootchain(const bootfunc_t *boot_sequence)
{
    const bootfunc_t *boot_one;
    int ret, boot_count = 0;
    unsigned long base;
    unsigned int boot_status = hypos_get(hypos_status);

    hypos_tag();

    for (boot_one = boot_sequence; *boot_one; boot_one++) {
        ret = (*boot_one)();

        boot_count++;

        if (ret) {
            MSGH("Boot squence %p failed at call %ps() [err=%d] on %s\n",
                    boot_sequence, __void__(*boot_one), ret,
                    boot_status == HYPOS_SMP_BOOT_STAGE ?
                    "smp cpu" : "boot cpu");
            return -1;
        } else
            MSGH("Boot phase <%02d> done executing %ps() on %s @_<\n",
                    boot_count, __void__(*boot_one),
                    boot_status == HYPOS_SMP_BOOT_STAGE ?
                    "smp cpu" : "boot cpu");
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
    /* Set up Board Type
     */
    board_setup,

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

    /* Set up Buddy Allocators and Other Allocators
     */
    hypmem_setup,

    /* Time pre-set
     */
    time_preset,

    /* Interrupt Controller Setup
     */
    gic_setup,

    /* Tasklet Implementation
     */
    tasklet_setup,

    /* Generic Timer Setup
     */
    timer_setup,

    /* Bring up SMP CPUs
     */
    pre_secondary_cpu_setup,

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

void asmlinkage __bootfunc __setup(unsigned long phys_offset,
        unsigned long boot_args)
{
    early_debug("[hypos] Welcome to C world\n");

    hypos_get(phys_offset) = phys_offset;
    hypos_get(boot_param) = boot_args;

    early_debug("[hypos] Boot CPU start kicking\n");

    /* Normal booting process, initiate hypos services. my
     * goal here is to implement basic console and be able
     * to run basic virtual machine on this by executing a
     * simple command.
     */
    if (__bootchain(hypos_boot_sequence))
        hang();
}

static bootfunc_t hypos_smpboot_sequence[] = {
    post_secondary_cpu_setup,
};

void asmlinkage __bootfunc __smp_setup(void)
{
    get_globl()->hypos_status = HYPOS_SMP_BOOT_STAGE;

    if (__bootchain(hypos_smpboot_sequence))
        hang();
}
// --------------------------------------------------------------
