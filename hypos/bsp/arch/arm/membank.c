/**
 * Hustler's Project
 *
 * File:  bootmem.c
 * Date:  2024/06/19
 * Usage: boot-time memory for transition period
 */

#include <asm/map.h>
#include <asm/bitops.h>
#include <bsp/memz.h>
#include <bsp/debug.h>
#include <bsp/board.h>
#include <org/membank.h>
#include <org/section.h>
#include <org/bitops.h>
#include <org/globl.h>
#include <lib/strops.h>
#include <lib/convert.h>

// --------------------------------------------------------------

/* XXX: Hustler - 2024/07/09 TUE
 *
 * Boot Time Allocators and Page Frame Table Setup
 *
 * Question is that How we map page frame table to the virtual
 * memory region [start, end], since we can access the 'page'
 * structures a lot faster.
 *
 * XXX: Boot-time Memory Management
 *
 *
 *
 *
 *
 *
 *
 *
 */

// --------------------------------------------------------------
unsigned long __read_mostly page_frame_table_base_idx;
unsigned long __read_mostly page_frame_table_va_end;
unsigned long __read_mostly page_frame_table_total_pages;
unsigned long __read_mostly page_frame_table_max_pages;

unsigned long __read_mostly directmap_va_start;
unsigned long __read_mostly directmap_va_end;
hfn_t __read_mostly directmap_hfn_start = INVALID_PFN_INIT;
hfn_t __read_mostly directmap_hfn_end = INVALID_PFN_INIT;
unsigned long __read_mostly directmap_base_idx;

hfn_t first_valid_hfn = INVALID_PFN_INIT;
unsigned int __initdata nr_memchunks;

/* memchunk - this is for page frame table setup. for this purpose,
 * It needs contiguous memory, for each struct page to store.
 */
struct memchunk __initdata memchunks_list[NR_BOOT_MEMBANKS];

static __initdata struct memblock memblock = {
    .boot_mbs.stats.max_banks = NR_BOOT_MEMBANKS,
};

static inline struct membanks *get_boot_membanks(void)
{
    return &memblock.boot_mbs;
}
// --------------------------------------------------------------

/* Use memchunks to track bootmem (Physical Memory)
 *
 * ----------------------------- ~
 * 0            ...            255
 * ----------------------------- ~
 */
static void __bootfunc memchunks_add(unsigned long start,
                                     unsigned long end) {
    unsigned int i;

    if (start >= end)
        return;

    /* Locate at proper index */
    for (i = 0; i < nr_memchunks; i++)
        if (start < memchunks_list[i].end)
            break;

    BUG_ON((i < nr_memchunks) && (end > memchunks_list[i].start));
    BUG_ON(nr_memchunks == (PAGE_SIZE / sizeof(struct memchunk)));

    memmove(&memchunks_list[i + 1], &memchunks_list[i],
            (nr_memchunks - i) * sizeof(*memchunks_list));

    memchunks_list[i] = (struct memchunk){ start, end };
    nr_memchunks++;
}

static void __bootfunc memchunks_zap(unsigned long start,
                                     unsigned long end) {
    unsigned int i;

    for (i = 0; i < nr_memchunks; i++) {
        struct memchunk *mc = &memchunks_list[i];
        if (end <= mc->start)
            break;
        if (start >= mc->end)
            continue;
        if (start <= mc->start)
            mc->start = min(end, mc->end);
        else if (end >= mc->end)
            mc->end = start;
        else {
            unsigned long _end = mc->end;
            mc->end = start;
            memchunks_add(end, _end);
        }
    }
}

hfn_t __bootfunc get_memchunks(unsigned long nr_hfns,
                               unsigned long hfn_align)
{
    unsigned long pg, _end;
    unsigned int  i = 0;

    BUG_ON(!nr_memchunks);
    MSGQ(false, "Tryna allocate %4lu PFNs out from %3u memchunks @_@\n",
            nr_hfns, nr_memchunks);

    while (i++ < nr_memchunks) {
        struct memchunk *mc = &memchunks_list[i];

        pg = (mc->end - nr_hfns) & ~(hfn_align - 1);

        if (pg >= mc->end || pg < mc->start)
            continue;

        _end = mc->end;
        mc->end = pg;
        memchunks_add(pg + nr_hfns, _end);

        MSGQ(false, "Note [%08lx - %08lx] (%04lu PFNs) been allocated out from "
                   "membank<%04u> [%08lx - %08lx]\n",
              pg + nr_hfns, _end, nr_hfns, i, mc->start, mc->end);

        return hfn_set(pg);
    }

    /* XXX: Check the usage when memchunk allocator crashed here */
    MSGE("Allocate %lu PFNs from memchunk[%u] "
         "(maximum of %u memchunks) failed\n",
         nr_hfns, i, nr_memchunks);

    BUG();

    /* Ain't gonna reach here */
    return INVALID_PFN;
}

void __bootfunc memchunks_setup(hpa_t ps, hpa_t pe)
{
    ps = ROUND_PGUP(ps);
    pe = ROUND_PGDOWN(pe);

    if (pe <= ps)
        return;

    first_valid_hfn = hfn_min(pa_to_hfn(ps), first_valid_hfn);
    memchunks_add(ps >> PAGE_SHIFT, pe >> PAGE_SHIFT);
}
// --------------------------------------------------------------
static void __bootfunc directmap_setup(unsigned long base_hfn,
                            unsigned long nr_hfns)
{
    int rc;
    struct hvm_blk *dmap = hypos_hvm_get(DMAP_BLK);

    /* First call sets the directmap physical and virtual offset. */
    if (hfn_eq(directmap_hfn_start, INVALID_PFN)) {
        unsigned long hfn_gb = base_hfn &
            ~((PGTBL_LEVEL_SIZE(1) >> PAGE_SHIFT) - 1);

        directmap_hfn_start  = hfn_set(base_hfn);
        directmap_base_idx   = __hfn_to_idx(base_hfn);
        directmap_va_start   = dmap->start
            + (base_hfn - hfn_gb) * PAGE_SIZE;

        MSGI("[globl] directmap_va_start             %016lx\n"
             BLANK_ALIGN"directmap_hfn_start            %016lx\n"
             BLANK_ALIGN"directmap_base_idx             %016lx\n",
             directmap_va_start, hfn_get(directmap_hfn_start),
             directmap_base_idx);
    }

    if (base_hfn < hfn_get(directmap_hfn_start))
        exec_panic("Cannot add direct mapping at %lx below heap start %lx\n",
              base_hfn, hfn_get(directmap_hfn_start));

    rc = map_pages((hva_t)__hfn_to_va(base_hfn), hfn_set(base_hfn),
            nr_hfns, PAGE_HYPOS_RW | _PAGE_BLOCK);
    if (rc)
        exec_panic("Unable to setup the direct mappings.\n");
}

static void __bootfunc page_frame_table_setup(hpa_t ps, hpa_t pe)
{
    const struct hvm_blk *btmb = hypos_hvm_get(BTMB_BLK);
    unsigned long nr_idx = hfn_to_idx(hfn_add(pa_to_hfn(pe), -1))
                           - hfn_to_idx(pa_to_hfn(ps)) + 1;
    unsigned long page_frame_table_size = nr_idx * sizeof(struct page);
    const unsigned long mapping_size =
            page_frame_table_size < MB(32) ? MB(2) : MB(32);
    hfn_t base_hfn;
    int rc;

    if (page_frame_table_size > btmb->size)
        exec_panic("The frame can't cover the PA [%016lx - %016lx]\n",
                ps, pe);

    page_frame_table_base_idx = hfn_to_idx(pa_to_hfn(ps));
    /* page_frame_table_size is either 2M or 32M */
    page_frame_table_size = ROUNDUP(page_frame_table_size, mapping_size);
    base_hfn = get_memchunks(page_frame_table_size >> PAGE_SHIFT, 1);

    MSGI("[globl] Page Frame Table Mapping\n"
         BLANK_ALIGN"----------------------------------------------------------\n"
         BLANK_ALIGN"Base VA             Base PFN                Number of PFNs\n"
         BLANK_ALIGN"----------------------------------------------------------\n"
         BLANK_ALIGN"%016lx    %016lx                   %03lu\n"
         BLANK_ALIGN"----------------------------------------------------------\n",
         btmb->start, hfn_get(base_hfn), page_frame_table_size >> PAGE_SHIFT);

    rc = map_pages(btmb->start, base_hfn,
                   page_frame_table_size >> PAGE_SHIFT,
                   PAGE_HYPOS_RW | _PAGE_BLOCK);
    if (rc)
        exec_panic("Unable to setup page frame table mapping");

    memset(&page_frame_table[0], 0, nr_idx * sizeof(struct page));

    page_frame_table_va_end = btmb->start
            + (nr_idx * sizeof(struct page));
}
// --------------------------------------------------------------
static void __bootfunc dump_hva(void)
{
    const struct hvm_blk *data = hypos_hvm_get(DATA_BLK);
    const struct hvm_blk *fmap = hypos_hvm_get(FMAP_BLK);
    const struct hvm_blk *vmap = hypos_hvm_get(VMAP_BLK);
    const struct hvm_blk *btmb = hypos_hvm_get(BTMB_BLK);
    const struct hvm_blk *dmap = hypos_hvm_get(DMAP_BLK);

    MSGI("[globl] Virtual Memory Layout                                  @_@\n"
         BLANK_ALIGN"----------------------------------------------------------\n"
         BLANK_ALIGN"Region           Size                                Range\n"
         BLANK_ALIGN"----------------------------------------------------------\n"
         BLANK_ALIGN"DATA     %8lu MB [%016lx - %016lx]\n"
         BLANK_ALIGN"FMAP     %8lu KB [%016lx - %016lx]\n"
         BLANK_ALIGN"BOOT     %8lu MB [%016lx - %016lx]\n"
         BLANK_ALIGN"VMAP     %8lu MB [%016lx - %016lx]\n"
         BLANK_ALIGN"DMAP     %8lu MB [%016lx - %016lx]\n"
         BLANK_ALIGN"----------------------------------------------------------\n",
         data->size / MB(1),
         data->start,
         data->start + data->size,
         fmap->size / KB(1),
         fmap->start,
         fmap->start + fmap->size,
         btmb->size / MB(1),
         btmb->start,
         btmb->start + btmb->size,
         vmap->size / MB(1),
         vmap->start,
         vmap->start + vmap->size,
         dmap->size / MB(1),
         dmap->start,
         dmap->start + dmap->size);
}

static void __bootfunc pre_membank_setup(void)
{
    struct membanks *mbs = get_boot_membanks();
    struct hpm_blk *_hpm = hypos_hpm_get(RAM_BLK);
    unsigned int bank;
    size_t bank_size = (_hpm->end - _hpm->start) / NR_BOOT_MEMBANKS;

    /* Set up boot membanks, for simplicity, shall ensure
     * each membank ain't overlap.
     *
     * XXX: boot-time membanks associated with hpm_blks,
     *      nr_banks = HPM_BLK_NR <= max_banks
     */
    ASSERT(HPM_BLK_NR <= mbs->stats.max_banks);

    for (bank = 0; bank < mbs->stats.max_banks &&
            mbs->stats.nr_banks <= mbs->stats.max_banks; bank++) {
        mbs->banks[bank].start = _hpm->start + (bank_size * bank);
        mbs->banks[bank].size  = bank_size;
        mbs->banks[bank].type  = RAM_BLK;
        mbs->stats.nr_banks++;
    }

    MSGQ(true, "%u membanks (%lu MB each) available in RAM\n",
            mbs->stats.nr_banks, bank_size / MB(1));
}

static void __bootfunc post_membank_setup(void)
{
    const struct membanks *mbs = get_boot_membanks();
    hpa_t ram_start = INVALID_PADDR, ram_end = 0;
    size_t ram_size = 0;
    unsigned int bank;

    /* Set up memchunk allocators.
     */
    for (bank = 0; bank < mbs->stats.nr_banks; bank++) {
        const struct membank *mb = &mbs->banks[bank];
        memchunks_setup(mb->start, mb->start + mb->size);
    }

    /* Set up direct mapping for page frames */
    for (bank = 0; bank < mbs->stats.nr_banks; bank++) {
        const struct membank *mb = &mbs->banks[bank];
        ram_start = min(mb->start, ram_start);
        ram_end   = max(mb->start + mb->size, ram_end);

        directmap_setup(PFN_DOWN(mb->start),
                        PFN_DOWN(mb->size));
    }

    page_frame_table_setup(ram_start, ram_end);

    page_frame_table_total_pages = (ram_end - ram_start) >> PAGE_SHIFT;
    directmap_va_end = HYPOS_HEAP_VIRT_START + ram_end - ram_start;
    directmap_hfn_start = pa_to_hfn(ram_start);
    directmap_hfn_end   = pa_to_hfn(ram_end);

    /* -------------------------- directmap_va_start
     *      |
     *      :
     *      |
     *      ▼
     * -------------------------- directmap_va_end
     */
    MSGI("[globl] directmap_va_end               %016lx\n"
         BLANK_ALIGN"directmap_hfn_end              %016lx\n"
         BLANK_ALIGN"page_frame_table_total_pages   %16lu\n",
         directmap_va_end, hfn_get(directmap_hfn_end),
         page_frame_table_total_pages);

    page_frame_table_max_pages = page_frame_table_total_pages;
}

/* XXX: Membanks
 *
 * Now First of All, Save Enough Memory Space for Page Frames:
 *
 * Number of PFNs = ram_size / PAGE_SIZE
 * Reserved Memory = Number of PFNs * sizeof(struct page)
 *
 * To set up the allocator to be able to allocate good amout of
 * contiguous memory for page frame table.
 *
 * membank views the system memory as collections of contiguous
 * regions.
 */
static void __bootfunc membank_setup(void)
{
    const struct hpm_blk *hram = hypos_hpm_get(RAM_BLK);
    const struct hpm_blk *resv = hypos_hpm_get(RSV_BLK);
    const struct hpm_blk *text = hypos_hpm_get(TXT_BLK);
    const hpa_t code_ehpa = hypos_get(boot_epaddr);

    /* To check the physical memory range - RAM Size */
    MSGI("[globl] Physical Memory Available                              @_<\n"
         BLANK_ALIGN"----------------------------------------------------------\n"
         BLANK_ALIGN"CODE  [%016lx - %016lx)    %8lu KB\n"
         BLANK_ALIGN"DRAM  [%016lx - %016lx]    %8lu MB\n"
         BLANK_ALIGN"RESV  [%016lx - %016lx]    %8lu MB\n"
         BLANK_ALIGN"----------------------------------------------------------\n",
         text->start, text->end,
         (text->end - text->start) / KB(1),
         hram->start, hram->end,
         (hram->end - hram->start) / MB(1),
         resv->start, resv->end,
         (resv->end - resv->start) / MB(1));

    /* Ensure that a good amount of memory (data region)
     * to cover the hypos codes in general.
     */
    ASSERT(text->end > code_ehpa);

    pre_membank_setup();

    post_membank_setup();
}

int __bootfunc bootmem_setup(void)
{
    dump_hva();

    membank_setup();

    return 0;
}
// --------------------------------------------------------------
