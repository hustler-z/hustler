/**
 * Hustler's Project
 *
 * File:  vmap.c
 * Date:  2024/06/25
 * Usage:
 */

#include <asm-generic/spinlock.h>
#include <asm-generic/section.h>
#include <asm-generic/bootmem.h>
#include <asm-generic/globl.h>
#include <asm/at.h>
#include <asm/bitops.h>
#include <asm/map.h>
#include <lib/bitops.h>
#include <lib/bitmap.h>
#include <bsp/hackmem.h>
#include <bsp/check.h>
#include <bsp/vmap.h>

static DEFINE_SPINLOCK(vm_lock);

static void *__read_mostly vm_base[VMAP_REGION_NR];

#define vm_bitmap(x) ((unsigned long *)vm_base[x])

/* highest allocated bit in the bitmap */
static unsigned int __read_mostly vm_top[VMAP_REGION_NR];

/* total number of bits in the bitmap */
static unsigned int __read_mostly vm_end[VMAP_REGION_NR];

/* lowest known clear bit in the bitmap */
static unsigned int vm_low[VMAP_REGION_NR];

// --------------------------------------------------------------
void __bootfunc vm_init_type(enum vmap_region type, void *start, void *end)
{
    unsigned int i, nr;
    unsigned long va;

    ASSERT(!vm_base[type]);

    vm_base[type] = start;
    vm_end[type] = PFN_DOWN(end - start);
    vm_low[type]= PFN_UP((vm_end[type] + 7) / 8);
    nr = PFN_UP((vm_low[type] + 7) / 8);
    vm_top[type] = nr * PAGE_SIZE * 8;

    for (i = 0, va = (unsigned long)vm_bitmap(type);
            i < nr; ++i, va += PAGE_SIZE) {
        pfn_t pfn;
        int rc;

        if (get_globl()->boot_status == EARLY_BOOT_STAGE)
            pfn = get_bootpages(1, 1);
        else {
            struct page *pg = halloc_page(0);

            BUG_ON(!pg);
            pfn = page_to_pfn(pg);
        }
        rc = map_pages(va, pfn, 1, PAGE_HYPOS);
        BUG_ON(rc);

        zero_page((void *)va);
    }

    bitmap_fill(vm_bitmap(type), vm_low[type]);

    populate_ttbl_range(va, vm_low[type] - nr);
}

int __bootfunc vmap_setup(void)
{
    vm_init_type(VMAP_DEFAULT, (void *)HYPOS_VMAP_VIRT_START,
        (void *)(HYPOS_VMAP_VIRT_START + HYPOS_VMAP_VIRT_SIZE));

    return 0;
}
// --------------------------------------------------------------

static void *vm_alloc(unsigned int nr, unsigned int align,
                      enum vmap_region t)
{
    unsigned int start, bit;

    if (!align)
        align = 1;
    else if (align & (align - 1))
        align = ISOLATE_LSB(align);

    ASSERT((t >= VMAP_DEFAULT) && (t < VMAP_REGION_NR));
    if (!vm_base[t])
        return NULL;

    spinlock(&vm_lock);
    for (; ;) {
        pfn_t pfn;

        ASSERT(vm_low[t] == vm_top[t] || !test_bit(vm_low[t], vm_bitmap(t)));
        for (start = vm_low[t]; start < vm_top[t]; ) {
            bit = find_next_bit(vm_bitmap(t), vm_top[t], start + 1);
            if (bit > vm_top[t])
                bit = vm_top[t];

            start = (start + align) & ~(align - 1);
            if (bit < vm_top[t]) {
                if (start + nr < bit)
                    break;
                start = find_next_zero_bit(vm_bitmap(t), vm_top[t], bit + 1);
            } else {
                if (start + nr <= bit)
                    break;
                start = bit;
            }
        }

        if (start < vm_top[t])
            break;

        spinunlock(&vm_lock);

        if (vm_top[t] >= vm_end[t])
            return NULL;

        if (get_globl()->boot_status == EARLY_BOOT_STAGE)
            pfn = get_bootpages(1, 1);
        else {
            struct page *pg = halloc_page(0);

            if (!pg)
                return NULL;
            pfn = page_to_pfn(pg);
        }

        spinlock(&vm_lock);

        if (start >= vm_top[t]) {
            unsigned long va = (unsigned long)vm_bitmap(t) + vm_top[t] / 8;

            if (!map_pages(va, pfn, 1, PAGE_HYPOS)) {
                zero_page((void *)va);
                vm_top[t] += PAGE_SIZE * 8;
                if (vm_top[t] > vm_end[t])
                    vm_top[t] = vm_end[t];
                continue;
            }
        }

        if (get_globl()->boot_status == EARLY_BOOT_STAGE)
            bootpages_setup(pfn_to_pa(pfn), pfn_to_pa(pfn) + PAGE_SIZE);
        else
            hfree_page(pfn_to_page(pfn));

        if (start >= vm_top[t]) {
            spinunlock(&vm_lock);
            return NULL;
        }
    }

    for (bit = start; bit < start + nr; ++bit)
        set_bit(bit, vm_bitmap(t));
    if (bit < vm_top[t])
        ASSERT(!test_bit(bit, vm_bitmap(t)));
    else
        ASSERT(bit == vm_top[t]);
    if (start <= vm_low[t] + 2)
        vm_low[t] = bit;
    spinunlock(&vm_lock);

    return vm_base[t] + start * PAGE_SIZE;
}

static unsigned int vm_index(const void *va, enum vmap_region type)
{
    unsigned long addr = (unsigned long)va & ~(PAGE_SIZE - 1);
    unsigned int idx;
    unsigned long start = (unsigned long)vm_base[type];

    if (!start)
        return 0;

    if (addr < start + (vm_end[type] / 8) ||
         addr >= start + vm_top[type] * PAGE_SIZE)
        return 0;

    idx = PFN_DOWN(va - vm_base[type]);
    return !test_bit(idx - 1, vm_bitmap(type)) &&
           test_bit(idx, vm_bitmap(type)) ? idx : 0;
}

static unsigned int vm_size(const void *va, enum vmap_region type)
{
    unsigned int start = vm_index(va, type), end;

    if (!start)
        return 0;

    end = find_next_zero_bit(vm_bitmap(type), vm_top[type], start + 1);

    return min(end, vm_top[type]) - start;
}

static void vm_free(const void *va)
{
    enum vmap_region type = VMAP_DEFAULT;
    unsigned int bit = vm_index(va, type);

    if (!bit) {
        type = VMAP_HYPOS;
        bit = vm_index(va, type);
    }

    if (!bit) {
        WARN_ON(va != NULL);
        return;
    }

    spinlock(&vm_lock);
    if (bit < vm_low[type]) {
        vm_low[type] = bit - 1;
        while (!test_bit(vm_low[type] - 1, vm_bitmap(type)))
            --vm_low[type];
    }
    while (__test_and_clear_bit(bit, vm_bitmap(type)))
        if (++bit == vm_top[type])
            break;
    spinunlock(&vm_lock);
}

void *__vmap(const pfn_t *pfn, unsigned int granularity,
             unsigned int nr, unsigned int align, unsigned int flags,
             enum vmap_region type)
{
    void *va = vm_alloc(nr * granularity, align, type);
    unsigned long cur = (unsigned long)va;

    for (; va && nr--; ++pfn, cur += PAGE_SIZE * granularity) {
        if (map_pages(cur, *pfn, granularity, flags)) {
            vunmap(va);
            va = NULL;
        }
    }

    return va;
}

void *vmap(const pfn_t *pfn, unsigned int nr)
{
    return __vmap(pfn, 1, nr, 1, PAGE_HYPOS, VMAP_DEFAULT);
}

void *vmap_contig(pfn_t pfn, unsigned int nr)
{
    return __vmap(&pfn, nr, 1, 1, PAGE_HYPOS, VMAP_DEFAULT);
}

unsigned int vmap_size(const void *va)
{
    unsigned int pages = vm_size(va, VMAP_DEFAULT);

    if (!pages)
        pages = vm_size(va, VMAP_HYPOS);

    return pages;
}

void vunmap(const void *va)
{
    unsigned long addr = (unsigned long)va;
    unsigned pages = vmap_size(va);

#ifndef _PAGE_NONE
    remove_maps(addr, addr + PAGE_SIZE * pages);
#else
    map_pages(addr, INVALID_MFN, pages, _PAGE_NONE);
#endif
    vm_free(va);
}

static void *vmalloc_type(size_t size, enum vmap_region type)
{
    pfn_t *pfn;
    unsigned int i, pages = PFN_UP(size);
    struct page *pg;
    void *va;

    ASSERT(size);

    if (PFN_DOWN(size) > pages)
        return NULL;

    pfn = hmalloc_array(pfn_t, pages);
    if ( pfn == NULL )
        return NULL;

    for (i = 0; i < pages; i++) {
        pg = halloc_page(0);
        if ( pg == NULL )
            goto error;
        pfn[i] = page_to_pfn(pg);
    }

    va = __vmap(pfn, 1, pages, 1, PAGE_HYPOS, type);
    if ( va == NULL )
        goto error;

    hmfree(pfn);
    return va;

 error:
    while (i--)
        hfree_page(pfn_to_page(pfn[i]));
    hmfree(pfn);
    return NULL;
}

void *vmalloc(size_t size)
{
    return vmalloc_type(size, VMAP_DEFAULT);
}

void *hvmalloc(size_t size)
{
    return vmalloc_type(size, VMAP_HYPOS);
}

void *vzalloc(size_t size)
{
    void *p = vmalloc_type(size, VMAP_DEFAULT);
    int i;

    if (p == NULL)
        return NULL;

    for (i = 0; i < size; i += PAGE_SIZE)
        zero_page(p + i);

    return p;
}

void vfree(void *va)
{
    unsigned int i, pages;
    struct page *pg;
    PGLIST_HEAD(pglist);

    if (!va)
        return;

    pages = vmap_size(va);
    ASSERT(pages);

    for (i = 0; i < pages; i++) {
        struct page *page = vmap_to_page(va + i * PAGE_SIZE);

        ASSERT(page);
        pglist_add(page, &pglist);
    }

    vunmap(va);

    while ((pg = pglist_remove_head(&pglist)) != NULL)
        hfree_page(pg);
}
// --------------------------------------------------------------
