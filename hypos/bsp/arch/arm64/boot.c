/**
 * Hustler's Project
 *
 * File:  boot.c
 * Date:  2024/05/20
 * Usage:
 */

#include <asm/page.h>
#include <asm/exit.h>
#include <asm/setup.h>
#include <asm-generic/section.h>
#include <lib/strops.h>

// --------------------------------------------------------------

static unsigned char __initdata boot_cpu_stack[STACK_SIZE]
    __attribute__((__aligned__(STACK_SIZE)));

struct arch_stack boot_stack = {
    .stack = boot_cpu_stack,
    .cpuid = 0,
};

int __bootfunc arch_setup(void)
{
    /* TODO
     */
    return 0;
}

void arch_cpu_reboot(void)
{
    /* TODO
     */
}
// --------------------------------------------------------------
