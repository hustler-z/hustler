/**
 * Hustler's Project
 *
 * File:  map.c
 * Date:  2024/06/18
 * Usage: page mappings related implementation
 */

#include <asm/tlb.h>
#include <asm/map.h>
#include <asm/bitops.h>
#include <asm/xaddr.h>
#include <asm/io.h>
#include <org/section.h>
#include <org/globl.h>
#include <org/membank.h>
#include <lib/bitops.h>
#include <lib/math.h>
#include <lib/strops.h>
#include <bsp/debug.h>
#include <bsp/panic.h>
#include <bsp/spinlock.h>
#include <bsp/errno.h>
#include <bsp/type.h>
#include <bsp/vmap.h>

// --------------------------------------------------------------
static __initdata DECLARE_BITMAP(inuse, NUM_FIX_PFNMAP);

__bootfunc void *map_hfn(hfn_t hfn)
{
    unsigned int bit, slot;

    bit = find_first_zero_bit(inuse, NUM_FIX_PFNMAP);

    if (bit == NUM_FIX_PFNMAP)
        exec_panic("Out of hfn map slots\n");

    set_bit(bit, inuse);
    slot = bit + FIX_PFNMAP_START;

    ASSERT(slot >= FIX_PFNMAP_START &&
            slot <= FIX_PFNMAP_END);

    arch_hfn_map(slot, hfn);

    return fix_to_va(slot);
}

void __bootfunc unmap_hfn(const void *vp)
{
    unsigned int bit;
    unsigned int slot = va_to_fix((unsigned long)vp);

    ASSERT(slot >= FIX_PFNMAP_START &&
            slot <= FIX_PFNMAP_END);

    bit = slot - FIX_PFNMAP_START;
    clear_bit(bit, inuse);
    arch_hfn_unmap(slot);
}

ttbl_t *map_table(hfn_t hfn)
{
    if (hypos_get(hypos_status) == HYPOS_EARLY_BOOT_STAGE)
        return map_hfn(hfn);
    else {
        MSGH("Not Implemented yet.\n");
        return NULL;
    }
}

void unmap_table(const ttbl_t *table)
{
    if (hypos_get(hypos_status) == HYPOS_EARLY_BOOT_STAGE)
        unmap_hfn(table);
    else
        MSGH("Not Implemented yet.\n");
}

static int create_ttbl(ttbl_t *entry)
{
    hfn_t hfn;
    void *ptr;
    ttbl_t pte;

    if (hypos_get(hypos_status) != HYPOS_EARLY_BOOT_STAGE) {
        MSGH("Not Implemented yet\n");
    } else
        hfn = get_memchunks(1, 1);

    ptr = map_table(hfn);
    zero_page(ptr);
    unmap_table(ptr);

    pte = hfn_to_entry(hfn, MT_NORMAL);
    pte.ttbl.table = 1;
    write_pte(entry, pte);

    return 0;
}

static int ttbl_next_level(bool read_only,
                           unsigned int level,
                           ttbl_t **table,
                           unsigned int offset)
{
    ttbl_t *entry;
    int ret;
    hfn_t hfn;

    entry = *table + offset;

    if (!pte_is_valid(*entry)) {
        if (read_only)
            return MAP_FAILED;

        MSGQ(false, "This Entry (%p) ain't Valid, "
                    "Thus Create a New Entry @_<\n",
                entry);

        ret = create_ttbl(entry);

        if (ret)
            return MAP_FAILED;
    }

    if (pte_is_mapped(*entry, level))
        return MAP_SPAGE;

    hfn = pte_get_hfn(*entry);
    unmap_table(*table);
    *table = map_table(hfn);

    return MAP_NORMAL;
}

static bool ttbl_check_entry(ttbl_t entry,
                             hfn_t hfn,
                             unsigned int level,
                             unsigned int flags)
{
    if ((flags & _PAGE_PRESENT) && hfn_eq(hfn, INVALID_PFN)) {
        if (!pte_is_valid(entry)) {
            MSGH("Unable to modify the invalid entry.\n");
            return false;
        }

        if (!pte_is_mapped(entry, level)) {
            MSGH("Unable to modify the mapped entry.\n");
            return false;
        }

        if (entry.ttbl.ai != PAGE_AI_MASK(flags)) {
            MSGH("Unable to modify the memory attributes.\n");
            return false;
        }

        if (entry.ttbl.contig) {
            MSGH("Unable to modify entry with contiguous bit set.\n");
            return false;
        }
    } else if (flags & _PAGE_PRESENT) {
        ASSERT(!hfn_eq(hfn, INVALID_PFN));

        if (pte_is_valid(entry)) {
            if (pte_is_mapped(entry, level))
                MSGH("Uable to change PFN for a valid entry.\n");
            else
                MSGH("Trying to replace a table with a mapping.\n");
            return false;
        }
    } else if (!(flags & (_PAGE_PRESENT|_PAGE_POPULATE))) {
        ASSERT(hfn_eq(hfn, INVALID_PFN));

        if (pte_is_table(entry, level)) {
            MSGH("Uable to remove a table.\n");
            return false;
        }

        if (entry.ttbl.contig) {
            MSGH("Uable to remove entry with contiguous bit set.\n");
            return false;
        }
    } else {
        ASSERT(flags & _PAGE_POPULATE);
        ASSERT(hfn_eq(hfn, INVALID_PFN));
    }

    return true;
}

/* Update an entry at the level @target
 */
static int ttbl_update_entry(hfn_t root,
                             unsigned long va,
                             hfn_t hfn,
                             unsigned int target,
                             unsigned int flags)
{
    int ret;
    unsigned int level;
    ttbl_t *table;
    bool read_only = hfn_eq(hfn, INVALID_PFN)
            && !(flags & _PAGE_POPULATE);
    ttbl_t pte, *entry;

    TTBL_OFFSETS(offsets, (hpa_t)va);

    ASSERT((flags & (_PAGE_POPULATE|_PAGE_PRESENT))
            != (_PAGE_POPULATE|_PAGE_PRESENT));

    table = (ttbl_t *)map_table(root);

    for (level = 0; level < target; level++) {
        ret = ttbl_next_level(read_only, level, &table,
                offsets[level]);
        if (ret == MAP_FAILED) {
            if (flags & (_PAGE_PRESENT|_PAGE_POPULATE)) {
                MSGH("%s() Unable to Map Level %u\n",
                        __func__, level);
                ret = -ENOENT;
                goto out;
            } else {
                ret = 0;
                goto out;
            }
        } else if (ret != MAP_NORMAL)
            break;
    }

    if (level != target) {
        MSGH("Shit happened in %s()\n", __func__);
        ret = -EINVAL;
        goto out;
    }

    entry = table + offsets[level];

    ret = -EINVAL;

    if (!ttbl_check_entry(*entry, hfn, level, flags))
        goto out;

    ret = 0;

    if (flags & _PAGE_POPULATE)
        goto out;

    if (!(flags & _PAGE_PRESENT))
        memset(&pte, 0x00, sizeof(pte));
    else {
        if (!hfn_eq(hfn, INVALID_PFN)) {
            pte = hfn_to_entry(hfn, PAGE_AI_MASK(flags));
            pte.ttbl.table = (level == 3);
        } else
            pte = *entry;

        pte.ttbl.ap  = PAGE_AP_MASK(flags);
        pte.ttbl.uxn = PAGE_XN_MASK(flags);
        pte.ttbl.contig = !!(flags & _PAGE_CONTIG);
    }

    write_pte(entry, pte);

    ret = 0;

out:
    unmap_table(table);

    return ret;
}

static int ttbl_mapping_level(unsigned long vfn,
                              hfn_t hfn,
                              unsigned long nr,
                              unsigned int flags)
{
    unsigned int level;
    unsigned long mask;

    mask = !hfn_eq(hfn, INVALID_PFN) ? hfn_get(hfn) : 0;
    mask |= vfn;

    if (likely(!(flags & _PAGE_BLOCK)))
        level = 3;
    else if (!(mask & (BIT(PGTBL_LEVEL_ORDER(1), UL) - 1)) &&
            (nr >= BIT(PGTBL_LEVEL_ORDER(1), UL)))
        level = 1;
    else if (!(mask & (BIT(PGTBL_LEVEL_ORDER(2), UL) - 1)) &&
            (nr >= BIT(PGTBL_LEVEL_ORDER(2), UL)))
        level = 2;
    else
        level = 3;

    return level;
}

#define TTBL_4K_NR_CONTIG   16

static unsigned int ttbl_check_contig(unsigned long vfn,
                                      hfn_t hfn,
                                      unsigned int level,
                                      unsigned long left,
                                      unsigned int flags)
{
    unsigned long nr_contig;

    if (!(flags & _PAGE_BLOCK))
        return 1;

    if (hfn_eq(hfn, INVALID_PFN))
        return 1;

    nr_contig = BIT(PGTBL_LEVEL_ORDER(level), UL) * TTBL_4K_NR_CONTIG;

    if ((left < nr_contig) || ((hfn_get(hfn) | vfn) & (nr_contig - 1)))
        return 1;

    return TTBL_4K_NR_CONTIG;
}

static DEFINE_SPINLOCK(ttbl_lock);

static int ttbl_update(unsigned long va,
                       hfn_t hfn,
                       const unsigned long nr_hfns,
                       unsigned int flags)
{
    int ret = 0;
    unsigned long vfn = va >> PAGE_SHIFT;
    unsigned long left = nr_hfns;
    unsigned int order, level, nr_contig, new_flags;
    const hfn_t root = pa_to_hfn(READ_SYSREG(TTBR0_EL2));

    if ((flags & _PAGE_PRESENT) && !PAGE_IS_RO(flags) &&
        !PAGE_XN_MASK(flags)) {
        MSGH("Mappings can't be both Writeable and Executable.\n");
        return -EINVAL;
    }

    if (flags & _PAGE_CONTIG) {
        MSGH("_PAGE_CONTIG is an internal only flag.\n");
        return -EINVAL;
    }

    if (!IS_ALIGNED(va, PAGE_SIZE)) {
        MSGH("The virtual address is not page-size aligned.\n");
        return -EINVAL;
    }

    spin_lock(&ttbl_lock);

    while (left) {
        level = ttbl_mapping_level(vfn, hfn, left, flags);
        order = PGTBL_LEVEL_ORDER(level);

        ASSERT(left >= BIT(order, UL));

        nr_contig = ttbl_check_contig(vfn, hfn, level, left, flags);
        new_flags = flags | ((nr_contig > 1) ? _PAGE_CONTIG : 0);

        for (; nr_contig > 0; nr_contig--) {
            ret = ttbl_update_entry(root, vfn << PAGE_SHIFT,
                                    hfn, level, new_flags);
            if (ret)
                break;

            vfn += 1U << order;

            if (!hfn_eq(hfn, INVALID_PFN))
                hfn = hfn_add(hfn, 1U << order);

            left -= (1U << order);
        }

        if (ret)
            break;
    }

    if (!((flags & _PAGE_PRESENT) && !hfn_eq(hfn, INVALID_PFN)))
        flush_tlb_range_va(va, PAGE_SIZE * nr_hfns);
    else
        isb();

    spin_unlock(&ttbl_lock);

    return ret;
}

int map_pages(unsigned long va,
              hfn_t hfn,
              unsigned long nr_hfns,
              unsigned int flags)
{
    return ttbl_update(va, hfn, nr_hfns, flags);
}

int remove_maps(unsigned long start,
                unsigned long end)
{
    return ttbl_update(start, INVALID_PFN,
            (end - start) >> PAGE_SHIFT, 0);
}

int __bootfunc populate_ttbl_range(unsigned long va,
                                   unsigned long nr_hfns)
{
    return ttbl_update(va, INVALID_PFN, nr_hfns,
            _PAGE_POPULATE);
}

// --------------------------------------------------------------

/**
 * Usage: Map a 4Kb page in a fixmap entry
 */
void set_fixmap(unsigned int map, hfn_t hfn,
                unsigned int flags)
{
    int ret;

    ret = map_pages(HYPOS_FIXMAP_ADDR(map), hfn, 1, flags);

    BUG_ON(ret != 0);
}

void clear_fixmap(unsigned int map)
{
    int ret;

    ret = remove_maps(HYPOS_FIXMAP_ADDR(map),
                      HYPOS_FIXMAP_ADDR(map) + PAGE_SIZE);

    BUG_ON(ret != 0);
}


/**
 * XXX: ioremap implementation
 * Usage: To remap device address ranges.
 */
void *ioremap_attr(hpa_t start, size_t len,
                   unsigned int attributes)
{
    hfn_t hfn = hfn_set(PFN_DOWN(start));
    unsigned int offs = start & (PAGE_SIZE - 1);
    unsigned int nr = PFN_UP(offs + len);
    void *ptr = __vmap(&hfn, nr, 1, 1, attributes, VMAP_DEFAULT);

    if (!ptr)
        return NULL;

    return ptr + offs;
}

void *ioremap(hpa_t pa, size_t len)
{
    return ioremap_attr(pa, len, PAGE_HYPOS_NOCACHE);
}
// --------------------------------------------------------------
