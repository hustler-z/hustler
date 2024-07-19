/**
 * Hustler's Project
 *
 * File:  ttbl.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ASM_TTBL_H
#define _ASM_TTBL_H
// --------------------------------------------------------------
#include <asm/page.h>

/*
 * Granularity | PAGE_SHIFT | TTBL_SHIFT
 * -------------------------------------
 * 4 K         | 12         | 9
 * 16K         | 14         | 11
 * 64K         | 16         | 13
 */
#define TTBL_SHIFT(n)          ((n) - 3)
#define TTBL_ENTRIES(n)        (_AC(1, U) << (TTBL_SHIFT(n)))
#define TTBL_ENTRY_MASK(n)     (TTBL_ENTRIES(n) - 1)
#define PGTBL_TTBL_SHIFT       TTBL_SHIFT(PAGE_SHIFT)
/* 512 Entries */
#define PGTBL_TTBL_ENTRIES     TTBL_ENTRIES(PAGE_SHIFT)
/* 0x01FF */
#define PGTBL_TTBL_ENTRY_MASK  TTBL_ENTRY_MASK(PAGE_SHIFT)

/* Page Table Settings
 * LEVEL             SHIFT           SIZE            MASK
 * --------------------------------------------------------------------
 * 0                 39              512GB           0xFFFFFF8000000000
 * 1                 30              1  GB           0xFFFFFFFFC0000000
 * 2                 21              2  MB           0xFFFFFFFFFFE00000
 * 3                 12              4  KB           0xFFFFFFFFFFFFF000
 */
#define LEVEL_ORDER(n, lvl)    ((3-(lvl)) * (TTBL_SHIFT(n)))
#define LEVEL_SHIFT(n, lvl)    (LEVEL_ORDER(n, lvl) + (n))
#define LEVEL_SIZE(n, lvl)     (_AT(hpa_t, 1) << (LEVEL_SHIFT(n, lvl)))
#define PGTBL_LEVEL_SHIFT(lvl) LEVEL_SHIFT(PAGE_SHIFT, lvl)
#define PGTBL_LEVEL_ORDER(lvl) LEVEL_ORDER(PAGE_SHIFT, lvl)
#define PGTBL_LEVEL_SIZE(lvl)  LEVEL_SIZE(PAGE_SHIFT, lvl)
#define PGTBL_LEVEL_MASK(lvl)  (~(PGTBL_LEVEL_SIZE(lvl) - 1))

// --------------------------------------------------------------

/* Calculate the offsets into the pagetbls for a given VA */
#define PGTBL0_LINEAR_OFFSET(va) ((va) >> PGTBL_LEVEL_SHIFT(0))
#define PGTBL1_LINEAR_OFFSET(va) ((va) >> PGTBL_LEVEL_SHIFT(1))
#define PGTBL2_LINEAR_OFFSET(va) ((va) >> PGTBL_LEVEL_SHIFT(2))
#define PGTBL3_LINEAR_OFFSET(va) ((va) >> PGTBL_LEVEL_SHIFT(3))

/* Page Table Levelx Index
 *  +--------------+--------------+--------------+--------------+
 *  | TBL0 [47:39] | TBL1 [38:30] | TBL2 [29:21] | TBL3 [20:12] | ...
 *  +--------------+--------------+--------------+--------------+
 */
#define PGTBL_OFFSET(ofs)    (_AT(unsigned int, ofs) & PGTBL_TTBL_ENTRY_MASK)
#define PGTBL0_OFFSET(va)    PGTBL_OFFSET(PGTBL0_LINEAR_OFFSET(va))
#define PGTBL1_OFFSET(va)    PGTBL_OFFSET(PGTBL1_LINEAR_OFFSET(va))
#define PGTBL2_OFFSET(va)    PGTBL_OFFSET(PGTBL2_LINEAR_OFFSET(va))
#define PGTBL3_OFFSET(va)    PGTBL_OFFSET(PGTBL3_LINEAR_OFFSET(va))

// --------------------------------------------------------------
#define TTBL_ENTRY_MEM  0xF7F /* nG=1 AF=1 SH=11 AP=01 NS=1 AI=111 T=1 V=1 */
#define TTBL_ENTRY_DEV  0xE73 /* nG=1 AF=1 SH=10 AP=01 NS=1 AI=100 T=1 V=1 */
// --------------------------------------------------------------
#endif /* _ASM_TTBL_H */
