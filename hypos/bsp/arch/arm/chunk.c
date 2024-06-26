/**
 * Hustler's Project
 *
 * File:  mem.c
 * Date:  2024/06/20
 * Usage:
 */

#include <asm/page.h>
#include <asm/map.h>
#include <asm/bitops.h>
#include <asm-generic/section.h>
#include <asm-generic/chunk.h>
#include <common/type.h>
#include <bsp/alloc.h>
#include <bsp/page.h>
#include <lib/bitops.h>
#include <lib/strops.h>
#include <lib/math.h>

static struct hchunks __initdata bootchunks = {
    .nchunks.nr_chunks = NR_MEM_CHUNKS,
    .rchunks.nr_chunks = NR_MEM_CHUNKS,
};

#define NR_VALID_PAGE_INDEX \
    BITS_TO_LONGS((HYPOS_MEMCHUNK_NR + PAGE_INDEX_COUNT - 1) / PAGE_INDEX_COUNT)

unsigned long __read_mostly
    page_index_valid[NR_VALID_PAGE_INDEX] = { [0] = 1 };

// --------------------------------------------------------------

/* XXX: Page Index Implementation
 *
 * Diagram to make sense of the following variables. The masks
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

    idx = pfn_to_idx(spfn) / PAGE_INDEX_COUNT;
    eidx = (pfn_to_idx(epfn - 1) + PAGE_INDEX_COUNT) / PAGE_INDEX_COUNT;

    for (; idx < eidx; idx++)
        set_bit(idx, page_index_valid);
}

void __bootfunc page_index_setup(void)
{
    const struct memchunks *nmc = &bootchunks.nchunks;
    paddr_t chunk_start = 0x0, chunk_end = 0x0;
    size_t chunk_size = 0;

    u64 mask = page_index_mask(0x0);
    int chunk;

    for (chunk = 0 ; chunk < nmc->nr_chunks; chunk++) {
        chunk_start = nmc->chunk[chunk].start;
        chunk_size = nmc->chunk[chunk].size;

        mask |= chunk_start |
            page_index_range_mask(chunk_start, chunk_size);
    }

    for (chunk = 0 ; chunk < nmc->nr_chunks; chunk++) {
        chunk_start = nmc->chunk[chunk].start;
        chunk_size = nmc->chunk[chunk].size;

        if (~mask & page_index_range_mask(chunk_start, chunk_size))
            mask = 0;
    }

    pfn_idx_hole_setup(mask >> PAGE_SHIFT);

    for (chunk = 0 ; chunk < nmc->nr_chunks; chunk++) {
        chunk_start = nmc->chunk[chunk].start;
        chunk_size = nmc->chunk[chunk].size;
        chunk_end = chunk_start + chunk_size;

        set_page_index_range(__pa_to_pfn(chunk_start),
                             __pa_to_pfn(chunk_end));
    }
}

void __bootfunc populate_boot_allocator(void)
{
    unsigned int chunk;
    const struct memchunks *nmc = &bootchunks.nchunks;
    const struct memchunks *rmc = &bootchunks.rchunks;
    paddr_t start, end;

    for (chunk = 0; chunk < rmc->nr_chunks; chunk++) {
        if (rmc->chunk[chunk].type != MEMCHUNK_STATIC_HEAP)
            continue;

        start = rmc->chunk[chunk].start;
        end = start + rmc->chunk[chunk].size;

        boot_page_setup(start, end);
    }
}

void __bootfunc memchunk_mapping(paddr_t ps, paddr_t pe)
{
    unsigned long nr_idx = __pfn_to_idx(pfn_add(pa_to_pfn(pe), -1)) -
                           __pfn_to_idx(pa_to_pfn(ps)) + 1;
    unsigned long chunk_size = nr_idx * sizeof(struct page);
    pfn_t base_pfn;
    const unsigned long mapping_size = chunk_size < MB(32) ?
        MB(2) : MB(32);
    int ret;

    if (chunk_size > HYPOS_MEMCHUNK_SIZE)
        panic("The memchunk can't cover [0x%016lx - 0x%016lx]",
                ps, pe);

    page_head_idx = __pfn_to_idx(pa_to_pfn(ps));
    chunk_size = ROUNDUP(chunk_size, mapping_size);
    base_pfn = alloc_boot_page(chunk_size >> PAGE_SHIFT,
            32 << (20 - 12));

    ret = map_pages(HYPOS_MEMCHUNK_START, base_pfn,
            chunk_size >> PAGE_SHIFT, PAGE_HYPOS_RW | _PAGE_BLOCK);

    if (ret)
        panic("Uable to setup the memchunk mapping.");

    memset(&page_head[0], 0, nr_idx * sizeof(struct page));
    memset(&page_head[nr_idx], -1,
            chunk_size - (nr_idx * sizeof(struct page)));

    page_virt_end = HYPOS_MEMCHUNK_START +
        (nr_idx * sizeof(struct page));
}

// --------------------------------------------------------------
int __bootfunc hchunks_setup(void)
{
    const struct memchunks *nmc = &bootchunks.nchunks;
    const struct memchunk *mchunk;
    paddr_t ram_start = 0x0;
    paddr_t ram_end = 0x0, mchunk_end = 0x0;
    size_t ram_size = 0;
    unsigned int chunk;

    page_index_setup();

    populate_boot_allocator();

    for (chunk = 0; chunk < nmc->nr_chunks; chunk++) {
        mchunk = &nmc->chunk[chunk];
        mchunk_end = mchunk->start + mchunk->size;

        ram_size += mchunk->size;
        ram_start = min(ram_start, mchunk->start);
        ram_end = max(ram_end, mchunk_end);
    }

    total_pages = ram_size >> PAGE_SHIFT;
    max_pages = PFN_DOWN(ram_end);

    memchunk_mapping(ram_start, ram_end);

    return 0;
}
// --------------------------------------------------------------
