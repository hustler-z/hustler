/**
 * Hustler's Project
 *
 * File:  memory.c
 * Date:  2024/05/22
 * Usage:
 */

#include <asm-generic/section.h>
#include <generic/memory.h>
#include <asm/ttbl.h>
#include <bsp/debug.h>

extern ttbl_t boot_pgtbl0[PGTBL_TTBL_ENTRIES];
extern ttbl_t boot_pgtbl1[PGTBL_TTBL_ENTRIES];
extern ttbl_t boot_pgtbl2[PGTBL_TTBL_ENTRIES];
extern ttbl_t boot_pgtbl3[PGTBL_TTBL_ENTRIES * DATA_NR_ENTRIES(2)];
extern ttbl_t hypos_fixmap[PGTBL_TTBL_ENTRIES];

// --------------------------------------------------------------

/* To build 4-level page tables:
 * level 0 VA[47:39] point to level 1
 * level 1 VA[38:30] point to level 2
 * level 2 VA[29:21] point to level 3
 * level 3 VA[20:12] point to PA [11:0]
 */
int __bootfunc mem_setup(void)
{
    MSGH("Memory Allocator Setup\n");

    return 0;
}
// --------------------------------------------------------------
