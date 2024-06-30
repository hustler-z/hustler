/**
 * Hustler's Project
 *
 * File:  ttbl.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ARCH_TTBL_H
#define _ARCH_TTBL_H
// --------------------------------------------------------------
#include <asm/page.h>
#include <asm/acpi.h>

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
#define LEVEL_SIZE(n, lvl)     (_AT(paddr_t, 1) << (LEVEL_SHIFT(n, lvl)))
#define PGTBL_LEVEL_SHIFT(lvl) LEVEL_SHIFT(PAGE_SHIFT, lvl)
#define PGTBL_LEVEL_ORDER(lvl) LEVEL_ORDER(PAGE_SHIFT, lvl)
#define PGTBL_LEVEL_SIZE(lvl)  LEVEL_SIZE(PAGE_SHIFT, lvl)
#define PGTBL_LEVEL_MASK(lvl)  (~(PGTBL_LEVEL_SIZE(lvl) - 1))

/* --------------------------------------------------------------------
 * Translation Table (stage 1 and stage 2)
 *
 *
 * --------------------------------------------------------------------
 */

#define KB(_kb)                (_AC(_kb, UL) << 10)
#define MB(_mb)                (_AC(_mb, UL) << 20)
#define GB(_gb)                (_AC(_gb, UL) << 30)

/* Memory Layout
 * --------------------------------------------------------------------
 * DATA
 * FIXMAP
 * VMAP
 * BOOTMEM
 * DIRECTMAP
 * --------------------------------------------------------------------
 */
#define HYPOS_DATA_VIRT_START      PAGE_ALIGN(0x00000A0000000000)
#define HYPOS_DATA_VIRT_SIZE       _AT(vaddr_t, MB(8))
#define HYPOS_DATA_NR_ENTRIES(lvl) (HYPOS_DATA_VIRT_SIZE / PGTBL_LEVEL_SIZE(lvl))
// --------------------------------------------------------------


/* HYPOS FIXMAP
 * ------------------------- CONSOLE      (0)
 *  |
 * ------------------------- PERIPHERAL   (1)
 *  |
 * ------------------------- PFNMAP_START (2)
 *  |
 *  :
 *  |
 * ------------------------- PFNMAP_END   (9)
 */
#define HYPOS_FIXMAP_VIRT_START    (HYPOS_DATA_VIRT_START + HYPOS_DATA_VIRT_SIZE)
#define HYPOS_FIXMAP_VIRT_SIZE     _AT(vaddr_t, MB(2))
// --------------------------------------------------------------
#define HYPOS_FIXMAP_ADDR(n)       (HYPOS_FIXMAP_VIRT_START + (n) * PAGE_SIZE)
#define NUM_FIX_PFNMAP             8
#define FIX_CONSOLE                0
#define FIX_PERIPHERAL             1
#define FIX_PFNMAP_START           2
#define FIX_PFNMAP_END             (FIX_PFNMAP_START + NUM_FIX_PFNMAP - 1)
#define FIXADDR_START              HYPOS_FIXMAP_ADDR(0)
#define FIXADDR_END                HYPOS_FIXMAP_ADDR(FIX_PFNMAP_END)
// --------------------------------------------------------------
#define HYPOS_VMAP_VIRT_START      PAGE_ALIGN(FIXADDR_END + MB(32))
#define HYPOS_VMAP_VIRT_SIZE       _AT(vaddr_t, GB(1))
#define HYPOS_VMAP_VIRT_END        (HYPOS_VMAP_VIRT_START + HYPOS_VMAP_VIRT_SIZE)
/* Boot-time Memory Chunks (1MB)
 */
#define HYPOS_BOOTMEM_START       PAGE_ALIGN(HYPOS_VMAP_VIRT_END + MB(32))
#define HYPOS_BOOTMEM_SIZE        GB(1)
#define HYPOS_BOOTMEM_NR          (HYPOS_BOOTMEM_SIZE / sizeof(*page_head))
#define HYPOS_BOOTMEM_END         (HYPOS_BOOTMEM_START + HYPOS_BOOTMEM_SIZE)
// --------------------------------------------------------------
#define HYPOS_DIRECTMAP_START      PAGE_ALIGN(HYPOS_BOOTMEM_END + MB(32))
#define HYPOS_DIRECTMAP_SIZE       GB(4)
#define HYPOS_DIRECTMAP_END        (HYPOS_DIRECTMAP_START + HYPOS_DIRECTMAP_SIZE)

#ifndef __ASSEMBLY__
extern unsigned long directmap_virt_start;
#define HYPOS_HEAP_VIRT_START      directmap_virt_start
#endif /* __ASSEMBLY__ */

// --------------------------------------------------------------
#define HYPOS_VIRT_START           HYPOS_DATA_VIRT_START
#define HYPOS_VIRT_END             HYPOS_DIRECTMAP_END
#define SYMBOLS_ORIGIN             HYPOS_VIRT_START
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

#ifndef __ASSEMBLY__
// --------------------------------------------------------------
#include <asm/atomic.h>
#include <asm/sysregs.h>
#include <asm/barrier.h>
#include <common/ccattr.h>
#include <common/type.h>
#include <common/traps.h>
#include <lib/math.h>
#include <bsp/check.h>

#define TTBL_OFFSETS(var, va)     \
    const unsigned int var[4] = { \
        PGTBL0_OFFSET(va),        \
        PGTBL1_OFFSET(va),        \
        PGTBL2_OFFSET(va),        \
        PGTBL3_OFFSET(va),        \
    }

/* PTE Shareability
 */
#define PTE_NOT_SHAREABLE 0x0
#define PTE_SHARE_UNDEF   0x1
#define PTE_SHARE_OUTER   0x2
#define PTE_SHARE_INNER   0x3

/* Always use the ARMv8-A long descriptor format in AArch64
 * execution state.
 *
 * When MMU is enabled, software uses virtual address to access
 * the page table. When using load/store instructions to change
 * a page table entry, MMU hardware translate the virtual address
 * before CPU LSU can access the physical memory of the page table
 * entry. As a result, some page table entries needs to describe
 * the memory attribute of the page table itself. When software
 * uses load/store instructions to access the page table, this
 * memory attribute is applied for accesses from CPU LSU.
 *
 * @LSU - Load-Store Unit
 *
 * page table entry format:
 *
 * +------------------+--+----------------------+--+------------------+
 * | Upper attributes |  | Output block address |  | Lower attributes |
 * +------------------+--+----------------------+--+------------------+
 *
 * Large Physical Address Extension (TTBL) - ARMv7-A Long Descriptor
 * format.
 *
 * @ttbl_t - translation table entry
 */

typedef struct __packed {
    unsigned long valid: 1;
    unsigned long table: 1;

    /* Memory Attributes
     *
     * lower attributes (10 bits)
     * [11:2]
     * ai  - attribute index to the MAIR_ELn
     *       0b111   Normal Memory
     *       0b100   Device Memory
     * sh  - shareable attibute
     * ap  - access permission
     *            Unprivileged(EL0)  Privileged(EL1/2/3)
     *       -------------------------------------------
     *       00   No access          Read & Write
     *       01   Read & Write       Read & Write
     *       10   No access          Read-only
     *       11   Read-only          Read-only
     *       -------------------------------------------
     * af  - access flag
     *       -------------------------------------------
     *       0    the block entry has not yet been used
     *       1    the block entry has been used
     *       -------------------------------------------
     * ng  - non-global
     *       -------------------------------------------
     *       1    associated with a specific task or app
     *       0    apply to all tasks
     *       -------------------------------------------
     * ns  - non-secure memory
     *       -------------------------------------------
     *       1    non-secure
     *       0    secure
     *       -------------------------------------------
     */
    unsigned long ai: 3;
    unsigned long ns: 1;
    unsigned long ap: 2;
    unsigned long sh: 2;
    unsigned long af: 1;
    unsigned long ng: 1;

    /* Base address of block or next table
     * [47:12]
     */
    unsigned long base: 36;

    /* [51:48] reserved set as zeros
     */
    unsigned long res1: 4;

    /* Upper attributes (16 bits)
     * contig - contiguous
     *
     * XXX: Execute attributes
     * HCR_EL2.{E2H, TGE} Trap General Exception
     *
     * uxn - User (EL0) Execute Never (Not used at EL3,
     *       or EL2 when HCR_EL2.E2H == 0)
     * pxn - Privileged Execute Never (Called XN at EL3,
     *       and EL2 when HCR_EL2.E2H == 0)
     */
    unsigned long contig: 1;
    unsigned long pxn: 1;
    unsigned long uxn: 1;
    unsigned long res2: 9;
} ttbl_entry_t;

/* For the purpose of page table walking
 */
typedef struct __packed {
    unsigned long valid: 1;
    unsigned long table: 1;
    unsigned long pad2: 10;
    unsigned long base: 36;
    unsigned long pad1: 16;
} ttbl_walk_t;

typedef union {
    unsigned long bits;
    ttbl_entry_t  ttbl;
    ttbl_walk_t   walk;
} ttbl_t;

int mmu_enabled(void);
int ttbl_setup(void);
void zero_page(ttbl_t *pte);

static inline void write_pte(ttbl_t *p, ttbl_t pte)
{
    dsb(sy);
    write_atomic(p, pte);
    dsb(sy);
}

static inline bool pte_is_valid(ttbl_t pte)
{
    return pte.walk.valid;
}

static inline bool pte_is_table(ttbl_t pte, unsigned int level)
{
    return (level < 3) && pte.walk.table;
}

static inline bool pte_is_mapped(ttbl_t pte, unsigned int level)
{
    if (level == 3)
        return pte.walk.table;
    else
        return !pte.walk.table;
}
#endif /* !__ASSEMBLY__ */
// --------------------------------------------------------------

#define SET_BOOT_TTBL(name, nr)                            \
ttbl_t __aligned(PAGE_SIZE) __section(".data.boot_pgtbl")  \
    name[PGTBL_TTBL_ENTRIES * (nr)]

#define SET_TTBL(name, nr)                                 \
ttbl_t __aligned(PAGE_SIZE) name[PGTBL_TTBL_ENTRIES * (nr)]

// --------------------------------------------------------------

#endif /* _ARCH_TTBL_H */
