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
#include <asm/ttbl.h>
#include <asm-generic/section.h>
#include <lib/strops.h>

// --------------------------------------------------------------

/* Boot-time Page Table Setup
 * --------------------------------------------------------------
 * boot_pgtbl<n> - 4-level page table used during boot time.
 *
 * --------------------------------------------------------------
 */
SET_BOOT_TTBL(boot_pgtbl0, 1);
SET_BOOT_TTBL(boot_pgtbl1, 1);
SET_BOOT_TTBL(boot_pgtbl2, 1);
SET_BOOT_TTBL(boot_pgtbl3, DATA_NR_ENTRIES(2));
SET_BOOT_TTBL(hypos_fixmap, 1);

SET_BOOT_TTBL(boot_idmap1, 1);
SET_BOOT_TTBL(boot_idmap2, 1);
SET_BOOT_TTBL(boot_idmap3, 1);

static unsigned char __initdata boot_cpu_stack[STACK_SIZE]
    __attribute__((__aligned__(STACK_SIZE)));

struct init_info init_stack = {
    .stack = boot_cpu_stack,
    .cpuid = 0,
};

void __bootfunc arch_setup(void)
{
    /* TODO
     */
}

void arch_cpu_reboot(void)
{
    /* TODO
     */
}
// --------------------------------------------------------------
