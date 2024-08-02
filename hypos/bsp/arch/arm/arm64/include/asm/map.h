/**
 * Hustler's Project
 *
 * File:  map.h
 * Date:  2024/06/19
 * Usage:
 */

#ifndef _ASM_MAP_H
#define _ASM_MAP_H
// --------------------------------------------------------------
#include <asm/xaddr.h>
#include <asm/tlb.h>
#include <bsp/panic.h>
#include <bsp/debug.h>

int map_pages(unsigned long va,
              hfn_t hfn,
              unsigned long nr_hfns,
              unsigned int flags);
int remove_maps(unsigned long start,
                unsigned long end);
void update_idmap(unsigned int ok);
ttbl_t *map_table(hfn_t hfn);
void unmap_table(const ttbl_t *table);

int populate_ttbl_range(unsigned long va,
                        unsigned long nr_hfns);
void set_fixmap(unsigned int map, hfn_t hfn,
                unsigned int flags);
void clear_fixmap(unsigned int map);

enum mapping_code {
    MAP_NORMAL = 0,
    MAP_FAILED,
    MAP_SPAGE,
};

extern ttbl_t hypos_fixmap[PGTBL_TTBL_ENTRIES];

static inline void arch_hfn_map(unsigned int slot, hfn_t hfn)
{
    ttbl_t *entry = &hypos_fixmap[slot];
    ttbl_t pte;

    ASSERT(!pte_is_valid(*entry));

    pte = hfn_to_entry(hfn, PAGE_HYPOS_RW);
    pte.ttbl.table = 1;
    write_pte(entry, pte);

    isb();
}

static inline void arch_hfn_unmap(unsigned int slot)
{
    ttbl_t pte;

    write_pte(&hypos_fixmap[slot], pte);

    flush_tlb_range_va_local(HYPOS_FIXMAP_ADDR(slot), PAGE_SIZE);
}

#define fix_to_va(slot)   ((void *)HYPOS_FIXMAP_ADDR(slot))

static inline unsigned int va_to_fix(hva_t va)
{
    ASSERT(va <= FIXADDR_END || va > FIXADDR_START);

    return ((va - FIXADDR_START) >> PAGE_SHIFT);
}
// --------------------------------------------------------------
#endif /* _ASM_MAP_H */
