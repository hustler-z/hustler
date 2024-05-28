/**
 * Hustler's Project
 *
 * File:  arch_pgtbl.h
 * Date:  2024/05/20
 * Usage: Arch setup for hypos
 */

#ifndef _ARCH_PGTBL_H
#define _ARCH_PGTBL_H
// ---------------------------------------------------------

#include <arch_page.h>

#ifndef __ASSEMBLY__
typedef unsigned long         vaddr_t;
typedef unsigned long         paddr_t;
#endif

/*
 * Granularity | PAGE_SHIFT | TTBL_SHIFT
 * -------------------------------------
 * 4K          | 12         | 9
 * 16K         | 14         | 11
 * 64K         | 16         | 13
 */
#define TTBL_SHIFT(n)          ((n) - 3)
#define TTBL_ENTRIES(n)        (_AC(1, U) << (TTBL_SHIFT(n)))
#define TTBL_ENTRY_MASK(n)     (TTBL_ENTRIES(n) - 1)
#define PGTBL_TTBL_SHIFT       TTBL_SHIFT(PAGE_SHIFT)
#define PGTBL_TTBL_ENTRIES     TTBL_ENTRIES(PAGE_SHIFT)
#define PGTBL_TTBL_ENTRY_MASK  TTBL_ENTRY_MASK(PAGE_SHIFT)

#define LEVEL_ORDER(n, lvl)    ((3-(lvl)) * (TTBL_SHIFT(n)))
#define LEVEL_SHIFT(n, lvl)    (LEVEL_ORDER(n, lvl) + (n))
#define LEVEL_SIZE(n, lvl)     (_AC(paddr_t, 1) << (LEVEL_SHIFT(n, lvl)))
#define PGTBL_LEVEL_SHIFT(lvl) LEVEL_SHIFT(PAGE_SHIFT, lvl)
#define PGTBL_LEVEL_ORDER(lvl) LEVEL_ORDER(PAGE_SHIFT, lvl)
#define PGTBL_LEVEL_SIZE(lvl)  LEVEL_SIZE(PAGE_SHIFT, lvl)
#define PGTBL_LEVEL_MASK(lvl)  (~(PGTBL_LEVEL_SIZE(lvl) - 1))

#define PGTBL0_SHIFT           PGTBL_LEVEL_SHIFT(0)
                               /* PGTBL_LEVEL_MASK(0) */
#define PGTBL0_MASK            _AC(0xFFFFFF8000000000, UL)
                               /* PGTBL_LEVEL_SIZE(0) (1<<39) 512GB */
#define PGTBL0_SIZE            _AC(0x0000008000000000, UL)

#define PGTBL1_SHIFT           PGTBL_LEVEL_SHIFT(1)
                               /* PGTBL_LEVEL_MASK(1) */
#define PGTBL1_MASK            _AC(0xFFFFFFFFC0000000, UL)
                               /* PGTBL_LEVEL_SIZE(1) (1<<30) 1GB */
#define PGTBL1_SIZE            _AC(0x0000000040000000, UL)

#define PGTBL2_SHIFT           PGTBL_LEVEL_SHIFT(2)
                               /* PGTBL_LEVEL_MASK(2) */
#define PGTBL2_MASK            _AC(0xFFFFFFFFFFE00000, UL)
                               /* PGTBL_LEVEL_SIZE(2) (1<<21) 2MB */
#define PGTBL2_SIZE            _AC(0x0000000000200000, UL)

#define PGTBL3_SHIFT           PGTBL_LEVEL_SHIFT(3)
                               /* PGTBL_LEVEL_MASK(3) */
#define PGTBL3_MASK            _AC(0xFFFFFFFFFFFFF000, UL)
                               /* PGTBL_LEVEL_SIZE(3) (1<<12) 4KB */
#define PGTBL3_SIZE            _AC(0x0000000000001000, UL)

#define KB(_kb)                (_AC(_kb, UL) << 10)
#define MB(_mb)                (_AC(_mb, UL) << 20)
#define GB(_gb)                (_AC(_gb, UL) << 30)

/* Memory Layout
 * --------------------------------------------------------------------
 * HYPERVISOR_VIRT_START
 * 8MB    DATA_VIRT_SIZE               - bss, data, etc.
 * 2MB    FIXMAP_VIRT_SIZE             - FIXMAP
 * 256MB  VMAP_VIRT_SIZE               - VMAP
 * 1GB    HYPER_HEAP_VIRT_SIZE         - HYPER HEAP
 * 2GB    GUEST_HEAP_VIRT_SIZE         - GUEST HEAP
 * --------------------------------------------------------------------
 */

#define DATA_VIRT_START        _AT(vaddr_t, 0x00)
#define DATA_VIRT_SIZE         _AT(vaddr_t, MB(8))
#define DATA_NR_ENTRIES(lvl)   (DATA_VIRT_SIZE / PGTBL_LEVEL_SIZE(lvl))
#define DATA_PGTBL2_NR_ENTRIES (4)
#define FIXMAP_VIRT_START      (DATA_VIRT_START + DATA_VIRT_SIZE)
#define FIXMAP_VIRT_SIZE       _AT(vaddr_t, MB(2))
#define FIXMAP_ADDR(n)         (FIXMAP_VIRT_START + (n) * PAGE_SIZE)

#define VMAP_VIRT_START        (DATA_VIRT_START + MB(256))
#define VMAP_VIRT_SIZE         _AT(vaddr_t, MB(256))

#define HYPER_HEAP_VIRT_START  (VMAP_VIRT_START + VMAP_VIRT_SIZE)
#define HYPER_HEAP_VIRT_SIZE   _AT(vaddr_t, GB(1))

#define GUEST_HEAP_VIRT_START  (HYPER_HEAP_VIRT_START + HYPER_HEAP_VIRT_SIZE)
#define GUEST_HEAP_VIRT_SIZE   _AT(vaddr_t, GB(2))

#define HYPERVISOR_VIRT_START  DATA_VIRT_START
#define HYPERVISOR_VIRT_END    (GUEST_HEAP_VIRT_START + GUEST_HEAP_VIRT_SIZE)

/* Calculate the offsets into the pagetbls for a given VA */
#define PGTBL0_LINEAR_OFFSET(va) ((va) >> PGTBL_LEVEL_SHIFT(0))
#define PGTBL1_LINEAR_OFFSET(va) ((va) >> PGTBL_LEVEL_SHIFT(1))
#define PGTBL2_LINEAR_OFFSET(va) ((va) >> PGTBL_LEVEL_SHIFT(2))
#define PGTBL3_LINEAR_OFFSET(va) ((va) >> PGTBL_LEVEL_SHIFT(3))

#define PGTBL_OFFSET(offs)   (_AT(unsigned int, offs) & PGTBL_TTBL_ENTRY_MASK)
#define PGTBL0_OFFSET(va)    PGTBL_OFFSET(PGTBL0_LINEAR_OFFSET(va))
#define PGTBL1_OFFSET(va)    PGTBL_OFFSET(PGTBL1_LINEAR_OFFSET(va))
#define PGTBL2_OFFSET(va)    PGTBL_OFFSET(PGTBL2_LINEAR_OFFSET(va))
#define PGTBL3_OFFSET(va)    PGTBL_OFFSET(PGTBL3_LINEAR_OFFSET(va))

#define PGTBL0_SLOT          PGTBL0_OFFSET(HYPERVISOR_VIRT_START)
#define PGTBL1_SLOT          PGTBL1_OFFSET(HYPERVISOR_VIRT_START)
#define PGTBL2_SLOT          PGTBL2_OFFSET(HYPERVISOR_VIRT_START)
#define PGTBL3_SLOT          PGTBL3_OFFSET(HYPERVISOR_VIRT_START)
// ---------------------------------------------------------

#endif /* _ARCH_PGTBL_H */
