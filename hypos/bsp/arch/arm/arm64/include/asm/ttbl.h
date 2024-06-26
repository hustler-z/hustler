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
 * HYPOS_VIRT_START
 * IDENTITY MAPPING RESERVED
 * 8MB    DATA_VIRT_SIZE               - bss, data, etc.
 * 2MB    FIXMAP_VIRT_SIZE             - FIXMAP
 * 256MB  VMAP_VIRT_SIZE               - VMAP
 * 1GB    HYPER_HEAP_VIRT_SIZE         - HYPER HEAP
 * 2GB    GUEST_HEAP_VIRT_SIZE         - GUEST HEAP
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
#ifndef __ASSEMBLY__
#define HYPOS_MEMCHUNK_START       _AT(vaddr_t, MB(32))
#define HYPOS_MEMCHUNK_SIZE        MB(128 - 32)
#define HYPOS_MEMCHUNK_NR          (HYPOS_MEMCHUNK_SIZE / sizeof(*page_head))
#define HYPOS_VMAP_VIRT_START      _AT(vaddr_t, MB(256))
#define HYPOS_VMAP_VIRT_SIZE       _AT(vaddr_t, GB(1) - MB(256))

#define HYPOS_HEAP_VIRT_START      _AT(vaddr_t, GB(1))
#define HYPOS_HEAP_VIRT_SIZE       _AT(vaddr_t, GB(1))

#define GUEST_HEAP_VIRT_START      _AT(vaddr_t, GB(2))
#define GUEST_HEAP_VIRT_SIZE       _AT(vaddr_t, GB(2))

#endif /* !__ASSEMBLY__ */
// --------------------------------------------------------------
#define HYPOS_VIRT_START           HYPOS_DATA_VIRT_START
#define HYPOS_VIRT_END             (GUEST_HEAP_VIRT_START + GUEST_HEAP_VIRT_SIZE)
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
     * Execute attributes
     * uxn    - Unprivileged eXecute Never
     * pxn    - Privileged eXecute Never
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

#define GV2P_READ        (0U << 0)
#define GV2P_WRITE       (1U << 0)
#define GV2P_EXEC        (1U << 1)

/*
 * VFN -> IFN -> PFN (VA -> IPA -> PA)
 * vfn_t   -  Virtual Page Frame Number
 * ifn_t   -  Immediate Physical Page Frame Number
 * pfn_t   -  Physical Page Frame Number
 */

TYPE_SAFE(unsigned long, pfn);

#define pte_get_pfn(pte)      (to_pfn_t((pte).walk.base))
#define pte_set_pfn(pte, pfn) \
    (pte).walk.base = to_pfn(pfn)

#define INVALID_PFN_RAW   (~0UL)
#define INVALID_PFN       to_pfn_t(INVALID_PFN_RAW)
#define INVALID_PFN_INIT  { INVALID_PFN_RAW }

static inline pfn_t pfn_max(pfn_t x, pfn_t y)
{
    return to_pfn_t(max(to_pfn(x), to_pfn(y)));
}

static inline pfn_t pfn_min(pfn_t x, pfn_t y)
{
    return to_pfn_t(min(to_pfn(x), to_pfn(y)));
}

static inline bool pfn_eq(pfn_t x, pfn_t y)
{
    return to_pfn(x) == to_pfn(y);
}

static inline pfn_t pfn_add(pfn_t x, long i)
{
    return to_pfn_t(to_pfn(x) + i);
}

ttbl_t pfn_to_entry(pfn_t pfn, unsigned int attr);

static inline void *pa_to_va(paddr_t pa)
{
    return (void *)(HYPOS_HEAP_VIRT_START);
}

/* AT => Ask mmu to do <address translation>
 * --------------------------------------------------------------
 * @ at <at_op>, <Xt>
 * --------------------------------------------------------------
 * S1E1R
 * S1E1W
 * S1E0R
 * S1E0W
 * S1E1RP
 * S1E1WP
 * S1E1A
 * S1E2R
 * S1E2W
 * S12E1R
 * S12E1W
 * S12E0R
 * S12E0W
 * S1E2A
 * S1E3R
 * S1E3W
 * S1E3A
 * --------------------------------------------------------------
 */
static inline u64 __va_to_pa(vaddr_t va)
{
    u64 par, tmp = read_sysreg_par();
    asm volatile ("at s1e2r, %0;" : : "r"(va));
    isb();
    par = read_sysreg_par();
    WRITE_SYSREG64(tmp, PAR_EL1);
    return par;
}

#define PAR_INVALID         (_AC(1, UL) << 0)
void dump_ttbl_walk(vaddr_t va);

static inline paddr_t _va_to_pa(vaddr_t va) {
    u64 par = __va_to_pa(va);

    if (par & PAR_INVALID) {
        dump_ttbl_walk(va);
        panic_par(par);
    }

    return (paddr_t)((par & PADDR_MASK & PAGE_MASK) |
            (va & ~PAGE_MASK));
}

#define va_to_pa(va)    _va_to_pa((vaddr_t)(va))
#define va_to_pfn(va)   to_pfn_t((va_to_pa((vaddr_t)va) >> PAGE_SHIFT))
#define pfn_to_va(pfn)  (pa_to_va(to_pfn(pfn) << PAGE_SHIFT))

#define __pa_to_pfn(pa) ((unsigned long)(pa) >> PAGE_SHIFT)
#define pa_to_pfn(pa)   to_pfn_t(__pa_to_pfn(pa))
#define pfn_to_pa(pfn)  ((paddr_t)(to_pfn(pfn) << PAGE_SHIFT))

TYPE_SAFE(unsigned long, ifn);

static inline u64 __gva_to_pa(vaddr_t va, unsigned int flags)
{
    u64 par, tmp = read_sysreg_par();
    if (!flags & GV2P_WRITE)
        asm volatile ("at s12e1r, %0;" : : "r"(va));
    else
        asm volatile ("at s12e1w, %0;" : : "r"(va));
    isb();
    par = read_sysreg_par();
    WRITE_SYSREG64(tmp, PAR_EL1);
    return par;
}

static inline u64 __gva_to_ipa(vaddr_t va, unsigned int flags)
{
    u64 par, tmp = read_sysreg_par();
    if (!flags & GV2P_WRITE)
        asm volatile ("at s1e1r, %0;" : : "r"(va));
    else
        asm volatile ("at s1e1w, %0;" : : "r"(va));
    isb();
    par = read_sysreg_par();
    WRITE_SYSREG64(tmp, PAR_EL1);
    return par;
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
