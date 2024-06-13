/**
 * Hustler's Project
 *
 * File:  ttbl.c
 * Date:  2024/05/20
 * Usage: exception handler implementation in C
 */

#include <asm/ttbl.h>
#include <asm/sysregs.h>
#include <asm-generic/section.h>

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

int mmu_enabled(void)
{
    return READ_SYSREG(SCTLR_EL2) & SCTLR_Axx_ELx_M;
}

int __bootfunc ttbl_setup(void)
{
    return 0;
}

// --------------------------------------------------------------
