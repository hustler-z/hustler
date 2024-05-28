/**
 * Hustler's Project
 *
 * File:  arch_setup.c
 * Date:  2024/05/20
 * Usage: exception handler implementation in C
 */

#include <arch_ttbl.h>
#include <cpu_init.h>

SET_TTBL_ENTRIES(pgtbl0, 1);
SET_TTBL_ENTRIES(pgtbl1, 1);
SET_TTBL_ENTRIES(pgtbl2, 1);
SET_TTBL_ENTRIES(pgtbl3, 1);
SET_TTBL_ENTRIES(idmap1, 1);
SET_TTBL_ENTRIES(idmap2, 1);
SET_TTBL_ENTRIES(idmap3, 1);
SET_TTBL_ENTRIES(fixmap, 1);






void arch_setup_mm(void)
{

}
