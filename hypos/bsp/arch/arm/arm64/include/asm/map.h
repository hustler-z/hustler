/**
 * Hustler's Project
 *
 * File:  map.h
 * Date:  2024/06/19
 * Usage:
 */

#ifndef _ARCH_MAP_H
#define _ARCH_MAP_H
// --------------------------------------------------------------
#include <asm/ttbl.h>
#include <asm/tlb.h>
#include <bsp/check.h>
#include <bsp/debug.h>

int map_pages(unsigned long va,
              pfn_t pfn,
              unsigned long nr_pfns,
              unsigned int flags);
int remove_maps(unsigned long start,
                unsigned long end);

enum mapping_code {
    MAP_NORMAL = 0,
    MAP_FAILED,
    MAP_SPAGE,
};

extern ttbl_t hypos_fixmap[PGTBL_TTBL_ENTRIES];

static inline void arch_pfn_map(unsigned int slot, pfn_t pfn)
{
    ttbl_t *entry = &hypos_fixmap[slot];
    ttbl_t pte;

    ASSERT(!pte_is_valid(*entry));

    DEBUG("%s() been Called.\n", __func__);

    pte = pfn_to_entry(pfn, PAGE_HYPOS_RW);
    pte.ttbl.table = 1;
    write_pte(entry, pte);

    isb();
}

static inline void arch_pfn_unmap(unsigned int slot)
{
    ttbl_t pte;

    write_pte(&hypos_fixmap[slot], pte);

    DEBUG("%s() been Called.\n", __func__);

    flush_tlb_range_va_local(HYPOS_FIXMAP_ADDR(slot), PAGE_SIZE);
}

#define fix_to_va(slot)   ((void *)HYPOS_FIXMAP_ADDR(slot))

static inline unsigned int va_to_fix(vaddr_t va)
{
    ASSERT(va >= FIXADDR_END || va < FIXADDR_START);

    return ((va - FIXADDR_START) >> PAGE_SHIFT);
}
// --------------------------------------------------------------
#endif /* _ARCH_MAP_H */
