/**
 * Hustler's Project
 *
 * File:  boot.c
 * Date:  2024/05/20
 * Usage:
 */

#include <asm/page.h>
#include <asm/exit.h>
#include <asm/hcpu.h>
#include <asm/debug.h>
#include <asm-generic/section.h>
#include <lib/strops.h>
#include <bsp/debug.h>

// --------------------------------------------------------------

static unsigned char __initdata boot_cpu_stack[STACK_SIZE]
    __attribute__((__aligned__(STACK_SIZE)));

struct hcpu boot_hcpu = {
    .stack = boot_cpu_stack,
};

int __bootfunc hcpu_setup(void)
{
    MSGH("Physical CPU Setup\n");

    return 0;
}

void arch_cpu_reboot(void)
{
    /* TODO
     */
}
// --------------------------------------------------------------
