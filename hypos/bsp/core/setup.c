/**
 * Hustler's Project
 *
 * File:  setup.c
 * Date:  2024/05/20
 * Usage: general initialization
 */

#include <asm/xaddr.h>
#include <asm/ttbl.h>
#include <asm/hcpu.h>
#include <asm/debug.h>
#include <org/globl.h>
#include <org/section.h>
#include <org/membank.h>
#include <org/time.h>
#include <org/smp.h>
#include <org/vcpu.h>
#include <bsp/memz.h>
#include <bsp/percpu.h>
#include <bsp/cpu.h>
#include <bsp/sdev.h>
#include <bsp/bootcore.h>
#include <bsp/console.h>
#include <bsp/debug.h>
#include <bsp/vmap.h>
#include <bsp/board.h>
#include <bsp/symtbl.h>
#include <bsp/tasklet.h>

// --------------------------------------------------------------
typedef int (*bootfunc_t)(void);

static int __bootfunc __bootchain(const bootfunc_t *boot_sequence)
{
    const bootfunc_t *boot_one;
    int ret, boot_count = 0;
    unsigned long base;
    unsigned int boot_status = hypos_get(hypos_status);

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
            MSGQ(false, "Boot phase <%02d> done executing %ps() on %s\n",
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
    /* Set up percpu areas
     */
    percpu_setup,

    /* Set up Board Type
     */
    board_setup,

    /* Hypervisor CPU Setup
     */
    hcpu_setup,

    /* Set up Translation Table
     */
    ttbl_setup,

    smp_clear_cpu_maps,

    /* Processor Information
     */
    processor_setup,

    /* Time pre-set
     */
    time_preset,

    /* Hypervisor Memory Chuncks for Boot-time Memory
     */
    bootmem_setup,

    /* Hypos VMAP Setup
     */
    vmap_setup,

    /* Set up Buddy Allocators and Other Allocators
     */
    memz_setup,

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

    /* Extra Bootcalls
     */
    bootcalls,

    /* Peripheral Setup
     */
    device_setup,

    /* Hypos Console Setup
     */
    console_setup,
};

void asmlinkage __bootfunc __setup(unsigned long phys_offset)
{
    early_debug("[hypos] Welcome to C world\n");

    hypos_get(phys_offset) = phys_offset;

    /* Normal booting process, initiate hypos services. my
     * goal here is to implement basic console and be able
     * to run basic virtual machine on this by executing a
     * simple command.
     */
    if (__bootchain(hypos_boot_sequence))
        BUG();
}

static bootfunc_t hypos_smpboot_sequence[] = {
    post_secondary_cpu_setup,
};

void asmlinkage __bootfunc __smp_setup(void)
{
    hypos_get(hypos_status) = HYPOS_SMP_BOOT_STAGE;

    if (__bootchain(hypos_smpboot_sequence))
        BUG();
}
// --------------------------------------------------------------
