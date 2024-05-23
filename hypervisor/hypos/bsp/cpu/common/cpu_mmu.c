/**
 * Hustler's Project
 *
 * File:  cpu_mmu.c
 * Date:  2024/05/22
 * Usage: build 4-level page tables
 */

#include <cpu_mmu.h>
#include <arch_lpae.h>

/* This function defined in arch_mmu.S
 */
extern void boot_cpu_mem_setup(void);

SET_PGTBL(boot_pgtbl, 1);

/* To build 4-level page tables:
 * level 0 VA[47:39] point to level 1
 * level 1 VA[38:30] point to level 2
 * level 2 VA[29:21] point to level 3
 * level 3 VA[20:12] point to PA [11:0]
 */
void __init cpu_mmu_setup(void)
{

}
