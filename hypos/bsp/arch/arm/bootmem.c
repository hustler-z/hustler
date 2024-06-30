/**
 * Hustler's Project
 *
 * File:  bootmem.c
 * Date:  2024/06/20
 * Usage:
 */

#include <asm/map.h>

#include <asm/bitops.h>
#include <asm-generic/section.h>
#include <asm-generic/bootmem.h>
#include <common/type.h>
#include <bsp/alloc.h>
#include <bsp/page.h>
#include <lib/bitops.h>
#include <lib/strops.h>
#include <lib/math.h>
#include <lib/convert.h>

// --------------------------------------------------------------
static struct bootmem __initdata bootmem = {
    .hypos_bootpages.max_bootpages = NR_BOOTPAGES,
};

#define NR_VALID_PAGE_INDEX \
    BITS_TO_LONGS((HYPOS_BOOTMEM_NR + PAGE_INDEX_COUNT - 1) \
            / PAGE_INDEX_COUNT)

unsigned long __read_mostly
    page_index_valid[NR_VALID_PAGE_INDEX] = { [0] = 1 };
// --------------------------------------------------------------

/* XXX: Page Index Implementation stolen from XEN
 *
 * Diagbtmc to make sense of the following variables. The masks
 * and shifts are done on pfn values in order to convert to/from
 * idx:
 *
 *                      pfn_hole_mask
 *                      pfn_idx_hole_shift (mask bitsize)
 *                      |
 *                 |---------|
 *                 |         |
 *                 V         V
 *         --------------------------
 *         |HHHHHHH|000000000|LLLLLL| <--- pfn
 *         --------------------------
 *         ^       ^         ^      ^
 *         |       |         |------|
 *         |       |             |
 *         |       |             pfn_idx_bottom_mask
 *         |       |
 *         |-------|
 *             |
 *             pfn_top_mask
 *
 * pa_{top,va_bottom}_mask is simply a shifted pfn_{top,idx_bottom}_mask,
 * where pa_top_mask has zeroes shifted in while pa_va_bottom_mask has
 * ones.
 */
unsigned long __ro_after_init max_pages;
unsigned long __ro_after_init total_pages;
unsigned long __ro_after_init page_head_idx;
unsigned long __ro_after_init page_virt_end;
unsigned long __ro_after_init pfn_idx_bottom_mask;
unsigned long __ro_after_init pfn_top_mask;
unsigned long __ro_after_init pfn_hole_mask;
unsigned long __ro_after_init pfn_idx_hole_shift;
unsigned long __ro_after_init pa_va_bottom_mask;
unsigned long __ro_after_init pa_top_mask;

// --------------------------------------------------------------

static u64 fill_mask(u64 mask)
{
    while (mask & (mask + 1))
        mask |= mask + 1;

    return mask;
}

u64 __bootfunc page_index_mask(u64 base)
{
    return fill_mask(max(base,
                (u64)1 << (MAX_ORDER + PAGE_SHIFT)) - 1);
}

u64 page_index_range_mask(u64 base, u64 len)
{
    return fill_mask(base ^ (base + len - 1));
}

void __bootfunc pfn_idx_hole_setup(unsigned long mask)
{
    unsigned int i, j, bottom_shift = 0, hole_shift = 0;

    for (j = MAX_ORDER - 1; ; ) {
        i = find_next_zero_bit(&mask, BITS_PER_LONG, j + 1);
        if (i >= BITS_PER_LONG)
            break;
        j = find_next_bit(&mask, BITS_PER_LONG, i + 1);
        if (j >= BITS_PER_LONG)
            break;
        if (j - i > hole_shift) {
            hole_shift = j - i;
            bottom_shift = i;
        }
    }

    if (!hole_shift)
        return;

    pfn_idx_hole_shift  = hole_shift;
    pfn_idx_bottom_mask = (1UL << bottom_shift) - 1;
    pa_va_bottom_mask   = (PAGE_SIZE << bottom_shift) - 1;
    pfn_hole_mask       = ((1UL << hole_shift) - 1) << bottom_shift;
    pfn_top_mask        = ~(pfn_idx_bottom_mask | pfn_hole_mask);
    pa_top_mask         = pfn_top_mask << PAGE_SHIFT;
}

void set_page_index_range(unsigned long spfn, unsigned long epfn)
{
    unsigned long idx, eidx;

    idx = __pfn_to_idx(spfn) / PAGE_INDEX_COUNT;
    eidx = (__pfn_to_idx(epfn - 1) + PAGE_INDEX_COUNT) / PAGE_INDEX_COUNT;

    for (; idx < eidx; idx++)
        set_bit(idx, page_index_valid);
}


/* XXX: bootpages_setup()
 *
 * Set up NR_BOOTPAGES (256) bootpages.
 */
void __bootfunc bootpages_setup(void)
{
    struct bootpages *bootpages = &bootmem.hypos_bootpages;
    int num;

    /* Set up all membootpages
     */
    for (num = 0; num < bootpages->max_bootpages; num++) {
        bootpages->pages[num].size  = PAGE_SIZE;
        bootpages->pages[num].start = HYPOS_BOOTMEM_START
            + (num * PAGE_SIZE);
        bootpages->pages[num].type  = BOOTMEM_STATIC_HEAP;
        bootpages->nr_bootpages++;
    }

    DEBUG("<%s> Finished\n", __func__);
}

void __bootfunc pageindex_setup(void)
{
    const struct bootpages *bootpages = &bootmem.hypos_bootpages;
    paddr_t bootpage_start = 0x0, bootpage_end = 0x0;
    size_t  bootpage_size  = 0;
    u64 mask = page_index_mask(0x0);
    int num;

    for (num = 0; num < bootpages->nr_bootpages; num++) {
        bootpage_start = bootpages->pages[num].start;
        bootpage_size  = bootpages->pages[num].size;

        mask |= bootpage_start |
            page_index_range_mask(bootpage_start, bootpage_size);
    }

    for (num = 0; num < bootpages->nr_bootpages; num++) {
        bootpage_start = bootpages->pages[num].start;
        bootpage_size  = bootpages->pages[num].size;

        if (~mask & page_index_range_mask(bootpage_start, bootpage_size))
            mask = 0;
    }

    pfn_idx_hole_setup(mask >> PAGE_SHIFT);

    for (num = 0; num < bootpages->nr_bootpages; num++) {
        bootpage_start = bootpages->pages[num].start;
        bootpage_size  = bootpages->pages[num].size;
        bootpage_end   = bootpage_start + bootpage_size;

        set_page_index_range(__pa_to_pfn(bootpage_start),
                             __pa_to_pfn(bootpage_end));
    }

    DEBUG("<%s> Finished\n", __func__);
}

void __bootfunc bootmem_alloctor_setup(void)
{
    int num;
    const struct bootpages *bootpages = &bootmem.hypos_bootpages;
    paddr_t start, end;

    for (num = 0; num < bootpages->nr_bootpages; num++) {
        if (bootpages->pages[num].type != BOOTMEM_STATIC_HEAP)
            continue;

        start = bootpages->pages[num].start;
        end = start + bootpages->pages[num].size;

        /* Set up page-sized membootpage individually.
         */
        if (boot_page_setup(start, end))
            MSGE("Set up Boot Page Allocter Failed >_<\n");
    }

    DEBUG("<%s> Finished\n", __func__);
}

void __bootfunc bootmem_mapping(paddr_t ps, paddr_t pe)
{
    unsigned long nr_idx = pfn_to_idx(pfn_add(pa_to_pfn(pe), -1)) -
                           pfn_to_idx(pa_to_pfn(ps)) + 1;
    unsigned long bootpage_size = nr_idx * sizeof(struct page);
    pfn_t base_pfn;
    const unsigned long mapping_size = bootpage_size < MB(32) ?
        MB(2) : MB(32);
    int ret;

    if (bootpage_size > HYPOS_BOOTMEM_SIZE)
        panic("The membootpage can't cover [0x%016lx - 0x%016lx]",
                ps, pe);

    page_head_idx = pfn_to_idx(pa_to_pfn(ps));
    bootpage_size = ROUNDUP(bootpage_size, mapping_size);

    DEBUG("[BTPG] membootpage size %lu KB, page head index %lu, number of page index %lu\n",
            bootpage_size / KB(1), page_head_idx, nr_idx);

    base_pfn = alloc_boot_page(bootpage_size >> PAGE_SHIFT,
            32 << (20 - 12));

    ret = map_pages(HYPOS_BOOTMEM_START, base_pfn,
            bootpage_size >> PAGE_SHIFT, PAGE_HYPOS_RW | _PAGE_BLOCK);

    if (ret)
        panic("Uable to setup the membootpage mapping.");

    memset(&page_head[0], 0, nr_idx * sizeof(struct page));
    memset(&page_head[nr_idx], -1,
            bootpage_size - (nr_idx * sizeof(struct page)));

    page_virt_end = HYPOS_BOOTMEM_START +
        (nr_idx * sizeof(struct page));
}

// --------------------------------------------------------------
int __bootfunc bootmem_setup(void)
{
    const struct bootpages *bootpages = &bootmem.hypos_bootpages;
    const struct bootpage *bootpage;
    paddr_t bm_start = INVALID_PADDR;
    paddr_t bm_end = 0x0, bootpage_end = 0x0;
    size_t  bm_size = 0;
    unsigned int num;

    bootpages_setup();

    pageindex_setup();

    bootmem_alloctor_setup();

    for (num = 0; num < bootpages->nr_bootpages; num++) {
        bootpage     = &bootpages->pages[num];
        bootpage_end = bootpage->start + bootpage->size;

        bm_size += bootpage->size;
        bm_start = min(bm_start, bootpage->start);
        bm_end   = max(bm_end, bootpage_end);

        directmap_setup(PFN_DOWN(bootpage->start),
                        PFN_DOWN(bootpage->size));
    }

    total_pages         = bm_size >> PAGE_SHIFT;
    directmap_virt_end  = HYPOS_HEAP_VIRT_START + bm_end - bm_start;
    directmap_pfn_start = pa_to_pfn(bm_start);
    directmap_pfn_end   = pa_to_pfn(bm_end);

    max_pages = PFN_DOWN(bm_end);

    DEBUG("[BTPG] BOOTMEM RANGE [0x%016lx - 0x%016lx] %lu pages\n",
            bm_start, bm_end, total_pages);

    bootmem_mapping(bm_start, bm_end);

    return 0;
}
// --------------------------------------------------------------

/* Boot Time Memory Slots
 */
struct bootmem_slot {
    unsigned long start, end;
};

static char __initdata badpage[100] = "";

pfn_t valid_pfn_1st = INVALID_PFN_INIT;

/* Statically reserve (4 KB) 1 page-sized memory */
static struct bootmem_slot __initdata
    bootmem_slots[PAGE_SIZE / sizeof(struct bootmem_slot)];
static unsigned int __initdata nr_bootmem_slots;

static void __bootfunc bootmem_slot_add(unsigned long start,
                                        unsigned long end)
{
    unsigned int i;

    if (start >= end)
        return;

    for (i = 0; i < nr_bootmem_slots; i++)
        if (start < bootmem_slots[i].end)
            break;

    BUG_ON((i < nr_bootmem_slots) && (end > bootmem_slots[i].start));

    memmove(&bootmem_slots[i + 1], &bootmem_slots[i],
            (nr_bootmem_slots - i) * sizeof(*bootmem_slots));

    bootmem_slots[i] = (struct bootmem_slot){start, end};
    nr_bootmem_slots++;
}

static void __bootfunc bootmem_slot_zap(unsigned long start,
                                        unsigned long end)
{
    unsigned int i;
    unsigned long _end;
    struct bootmem_slot *slot;

    for (i = 0; i < nr_bootmem_slots; i++) {
        slot = &bootmem_slots[i];
        if (end <= slot->start)
            break;
        if (start >= slot->end)
            continue;
        if (start <= slot->start)
            slot->start = min(end, slot->end);
        else if (end >= slot->end)
            slot->end = start;
        else {
            _end = slot->end;
            slot->end = start;
            bootmem_slot_add(end, _end);
        }
    }
}

int __bootfunc boot_page_setup(paddr_t ps, paddr_t pe)
{
    unsigned long bad_spfn, bad_epfn;
    const char *ptr;

    ps = ROUND_PGUP(ps);
    pe = ROUND_PGDOWN(pe);

    if (pe <= ps)
        return -1;

    valid_pfn_1st = pfn_min(pa_to_pfn(ps), valid_pfn_1st);
    bootmem_slot_add(ps >> PAGE_SHIFT, pe >> PAGE_SHIFT);
    ptr = badpage;

    while (*ptr != '\0') {
        bad_spfn = _strtoul(ptr, &ptr, 0);
        bad_epfn = bad_spfn;

        if (*ptr == '-') {
            ptr++;
            bad_epfn = _strtoul(ptr, &ptr, 0);
            if (bad_epfn < bad_spfn)
                bad_epfn = bad_spfn;
        }

        if (*ptr == ',')
            ptr++;
        else if (*ptr != '\0')
            break;

        bootmem_slot_zap(bad_spfn, bad_epfn + 1);
    }

    return 0;
}

pfn_t __bootfunc alloc_boot_page(unsigned long nr_pfns,
                                 unsigned long pfn_align)
{
    unsigned long pg, _end;
    unsigned int i = nr_bootmem_slots;
    struct bootmem_slot *slot = NULL;

    DEBUG("[BTPG] nr_bootmem_slots (%u)\n", nr_bootmem_slots);

    while (i--) {
        slot = &bootmem_slots[i];
        pg = (slot->end - nr_pfns) & ~(pfn_align - 1);

        if (pg >= slot->end || pg < slot->start)
            continue;
        _end = slot->end;
        bootmem_slot_add(pg + nr_pfns, _end);

        return pfn_set(pg);
    }

    BUG();

    return pfn_set(0);
}
// --------------------------------------------------------------
