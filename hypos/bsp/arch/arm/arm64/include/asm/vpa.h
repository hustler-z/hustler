/**
 * Hustler's Project
 *
 * File:  vpa.h
 * Date:  2024/06/19
 * Usage: Virtual Address <-> Physical Address Implementation
 */

#ifndef _ARCH_VPA_H
#define _ARCH_VPA_H
// --------------------------------------------------------------
#include <asm/ttbl.h>
#include <bsp/page.h>
/*
 * VFN -> IFN -> PFN (VA -> IPA -> PA)
 * vfn_t   -  Virtual Page Frame Number
 * ifn_t   -  Immediate Physical Page Frame Number
 * pfn_t   -  Physical Page Frame Number
 */
TYPE_SAFE(unsigned long, pfn);

#define pte_get_pfn(pte)      (pfn_set((pte).walk.base))
#define pte_set_pfn(pte, pfn) \
    (pte).walk.base = pfn_get(pfn)

#define INVALID_PFN_RAW   (~0UL)
#define INVALID_PFN       pfn_set(INVALID_PFN_RAW)
#define INVALID_PFN_INIT  { INVALID_PFN_RAW }

static inline pfn_t pfn_max(pfn_t x, pfn_t y)
{
    return pfn_set(max(pfn_get(x), pfn_get(y)));
}

static inline pfn_t pfn_min(pfn_t x, pfn_t y)
{
    return pfn_set(min(pfn_get(x), pfn_get(y)));
}

static inline bool pfn_eq(pfn_t x, pfn_t y)
{
    return pfn_get(x) == pfn_get(y);
}

static inline pfn_t pfn_add(pfn_t x, long i)
{
    return pfn_set(pfn_get(x) + i);
}

ttbl_t pfn_to_entry(pfn_t pfn, unsigned int attr);

// --------------------------------------------------------------
#define page_to_idx(pg)     ((pg) - page_head)
#define idx_to_page(idx)    (page_head + (idx))

#define pa_to_directmapoff(x) (x)
#define directmapoff_to_pa(x) (x)
// --------------------------------------------------------------

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

#define va_to_pa(va)     _va_to_pa((vaddr_t)(va))
#define va_to_pfn(va)    pfn_set((va_to_pa((vaddr_t)va) >> PAGE_SHIFT))

extern unsigned long directmap_virt_start;
extern unsigned long directmap_virt_end;
extern pfn_t directmap_pfn_start;
extern pfn_t directmap_pfn_end;
extern unsigned long directmap_base_idx;

#define __pa_to_pfn(pa)  ((unsigned long)(pa) >> PAGE_SHIFT)
#define pa_to_pfn(pa)    pfn_set(__pa_to_pfn(pa))

static inline void *pa_to_va(paddr_t pa)
{
    ASSERT((pfn_to_idx(pa_to_pfn(pa)) - directmap_base_idx)
           < (HYPOS_DIRECTMAP_SIZE >> PAGE_SHIFT));
    return (void *)(HYPOS_HEAP_VIRT_START -
                    (directmap_base_idx << PAGE_SHIFT) +
                    pa_to_directmapoff(pa));
}

#define pfn_to_va(pfn)   (pa_to_va(pfn_get(pfn) << PAGE_SHIFT))
#define pfn_to_pa(pfn)   ((paddr_t)(pfn_get(pfn) << PAGE_SHIFT))

#define __pa(x)          (va_to_pa(x))
#define __va(x)          (pa_to_va(x))
#define __va_to_pfn(va)  (va_to_pa(va) >> PAGE_SHIFT)
#define __pfn_to_va(pfn) (pa_to_va((paddr_t)(pfn) << PAGE_SHIFT))
// --------------------------------------------------------------
static inline struct page *va_to_page(const void *v)
{
    unsigned long va = (unsigned long)v;
    unsigned long idx;

    ASSERT(va >= HYPOS_VIRT_START);
    ASSERT(va < directmap_virt_end);

    idx = (va - HYPOS_HEAP_VIRT_START) >> PAGE_SHIFT;
    idx += pfn_to_idx(directmap_pfn_start);

    return page_head + idx - page_head_idx;
}

static inline void *page_to_va(const struct page *pg)
{
    return pfn_to_va(page_to_pfn(pg));
}
// --------------------------------------------------------------
TYPE_SAFE(unsigned long, ifn);

#define GV2P_READ        (0U << 0)
#define GV2P_WRITE       (1U << 0)
#define GV2P_EXEC        (1U << 1)

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
// --------------------------------------------------------------
#endif /* _ARCH_VPA_H */
