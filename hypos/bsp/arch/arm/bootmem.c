/**
 * Hustler's Project
 *
 * File:  bootmem.c
 * Date:  2024/06/19
 * Usage: boot-time memory for transition period
 */

#include <asm/map.h>
#include <asm/bitops.h>
#include <bsp/hypmem.h>
#include <bsp/debug.h>
#include <bsp/board.h>
#include <asm-generic/bootmem.h>
#include <asm-generic/section.h>
#include <asm-generic/globl.h>
#include <lib/strops.h>
#include <lib/convert.h>

// --------------------------------------------------------------
unsigned long __read_mostly pageframe_base_idx;
unsigned long __read_mostly pageframe_va_end;
unsigned long __read_mostly pageframe_total_pages;

unsigned long __read_mostly directmap_va_start;
unsigned long __read_mostly directmap_va_end;
pfn_t __read_mostly directmap_pfn_start = INVALID_PFN_INIT;
pfn_t __read_mostly directmap_pfn_end = INVALID_PFN_INIT;
unsigned long __read_mostly directmap_base_idx;

pfn_t first_valid_pfn = INVALID_PFN_INIT;
// --------------------------------------------------------------

unsigned int __initdata nr_bootpages;

struct bootpage __initdata
    bootpages_list[PAGE_SIZE / sizeof(struct bootpage)];

static __initdata struct bootmem bootmem = {
    .resv_mem.stats.max_banks = NR_MEMBANKS,
    .heap_mem.stats.max_banks = NR_MEMBANKS,
};

static inline struct membanks *get_resv_membanks(void)
{
    return &bootmem.resv_mem;
}

static inline struct membanks *get_heap_membanks(void)
{
    return &bootmem.heap_mem;
}
// --------------------------------------------------------------
static void __bootfunc bootpages_add(unsigned long start,
                                     unsigned long end) {
    unsigned int i;

    if (start >= end)
        return;

    /* Locate at proper index */
    for (i = 0; i < nr_bootpages; i++)
        if (start < bootpages_list[i].end)
            break;

    BUG_ON((i < nr_bootpages) && (end > bootpages_list[i].start));
    BUG_ON(nr_bootpages == (PAGE_SIZE / sizeof(struct bootpage)));

    /* +---+---+ ~ ~ ~
     * |   |   |
     * +---+---+ ~ ~ ~
     *   |   ^
     *   +---+   Set next bootpage ready.
     */
    memmove(&bootpages_list[i + 1], &bootpages_list[i],
            (nr_bootpages - i) * sizeof(*bootpages_list));

    bootpages_list[i] = (struct bootpage){ start, end };
    nr_bootpages++;
}

static void __bootfunc bootpages_zap(unsigned long start,
                                     unsigned long end) {
    unsigned int i;

    for (i = 0; i < nr_bootpages; i++) {
        struct bootpage *bp = &bootpages_list[i];
        if (end <= bp->start)
            break;
        if (start >= bp->end)
            continue;
        if (start <= bp->start)
            bp->start = min(end, bp->end);
        else if (end >= bp->end)
            bp->end = start;
        else {
            unsigned long _end = bp->end;
            bp->end = start;
            bootpages_add(end, _end);
        }
    }
}

pfn_t __bootfunc get_bootpages(unsigned long nr_pfns,
                               unsigned long pfn_align)
{
    unsigned long pg, _end;
    unsigned int i = nr_bootpages;

    BUG_ON(!nr_bootpages);

    while (i--) {
        struct bootpage *bp = &bootpages_list[i];

        pg = (bp->end - nr_pfns) & ~(pfn_align - 1);
        if (pg >= bp->end || pg < bp->start)
            continue;

        _end = bp->end;
        bp->end = pg;
        bootpages_add(pg + nr_pfns, _end);

        return pfn_set(pg);
    }

    BUG();

    /* Ain't gonna reach here */
    return INVALID_PFN;
}

void __bootfunc bootpages_setup(paddr_t ps, paddr_t pe)
{
    ps = ROUND_PGUP(ps);
    pe = ROUND_PGDOWN(pe);

    if (pe <= ps)
        return;

    first_valid_pfn = pfn_min(pa_to_pfn(ps), first_valid_pfn);
    bootpages_add(ps >> PAGE_SHIFT, pe >> PAGE_SHIFT);
}
// --------------------------------------------------------------

static void __bootfunc __bootmem_setup(void)
{
    struct membanks *mbs = get_resv_membanks();
    unsigned int bank;

    /* Set up boot pages allocators.
     */
    for (bank = 0; bank < mbs->stats.nr_banks; bank++) {
        bootpages_setup(mbs->banks[bank].start,
                        mbs->banks[bank].start + mbs->banks[bank].size);
    }
}

static void __bootfunc directmap_setup(unsigned long base_pfn,
                            unsigned long nr_pfns)
{
    int rc;

    /* First call sets the directmap physical and virtual offset. */
    if (pfn_eq(directmap_pfn_start, INVALID_PFN)) {
        unsigned long pfn_gb = base_pfn &
            ~((PGTBL_LEVEL_SIZE(1) >> PAGE_SHIFT) - 1);

        directmap_pfn_start  = pfn_set(base_pfn);
        directmap_base_idx   = __pfn_to_idx(base_pfn);
        directmap_va_start   = HYPOS_DIRECTMAP_START
            + (base_pfn - pfn_gb) * PAGE_SIZE;

        MSGI("[globl] directmap_base_idx    %016lx\n"
             "        pfn_gb (Local)        %016lx\n"
             "        directmap_va_start    %016lx\n",
                directmap_base_idx, pfn_gb, directmap_va_start);
    }

    if (base_pfn < pfn_get(directmap_pfn_start))
        exec_panic("Cannot Add Direct Mapping at %lx Below Heap Start %lx\n",
              base_pfn, pfn_get(directmap_pfn_start));

    rc = map_pages((vaddr_t)__pfn_to_va(base_pfn), pfn_set(base_pfn),
            nr_pfns, PAGE_HYPOS_RW | _PAGE_BLOCK);
    if (rc)
        exec_panic("Unable to Setup the Direct Mappings.\n");

    MSGO("<%s> Finished\n", __func__);
}

static void __bootfunc pageframe_setup(paddr_t ps, paddr_t pe)
{
    unsigned long nr_idx = pfn_to_idx(pfn_add(pa_to_pfn(pe), -1))
            - pfn_to_idx(pa_to_pfn(ps)) + 1;
    unsigned long pageframe_size = nr_idx * sizeof(struct page);
    pfn_t base_pfn;
    const unsigned long map_size = pageframe_size < MB(32) ? MB(2) : MB(32);
    int rc;

    pageframe_base_idx = pfn_to_idx(pa_to_pfn(ps));
    pageframe_size     = ROUNDUP(pageframe_size, map_size);
    base_pfn           = get_bootpages(pageframe_size >> PAGE_SHIFT,
                                      32 << (20 - 12));
    rc = map_pages(HYPOS_BOOTMEM_START, base_pfn,
                   pageframe_size >> PAGE_SHIFT,
                   PAGE_HYPOS_RW | _PAGE_BLOCK);
    if (rc)
        exec_panic("Unable to setup pageframe mapping");

    memset(&pageframe[0], 0, nr_idx * sizeof(struct page));
    memset(&pageframe[nr_idx], -1, pageframe_size - (nr_idx * sizeof(struct page)));

    pageframe_va_end = HYPOS_BOOTMEM_START + (nr_idx * sizeof(struct page));
}

// --------------------------------------------------------------
#define IDX_GROUP_SHIFT PGTBL_LEVEL_SHIFT(2)

#define IDX_GROUP_COUNT ((1 << IDX_GROUP_SHIFT) / \
                         (ISOLATE_LSB(sizeof(*pageframe))))

unsigned long __read_mostly idx_group_valid[BITS_TO_LONGS(
    (NR_PAGEFRAME + IDX_GROUP_COUNT - 1) / IDX_GROUP_COUNT)] = { [0] = 1 };

void set_idx_range(unsigned long smfn, unsigned long emfn)
{
    unsigned long idx, eidx;

    idx = __pfn_to_idx(smfn) / IDX_GROUP_COUNT;
    eidx = (__pfn_to_idx(emfn - 1) + IDX_GROUP_COUNT) / IDX_GROUP_COUNT;

    for ( ; idx < eidx; ++idx)
        set_bit(idx, idx_group_valid);
}

static void __bootfunc idx_setup(void)
{
    paddr_t bank_start, bank_size, bank_end;
    struct membanks *mbs = get_resv_membanks();
    int bank;

    for (bank = 0 ; bank < mbs->stats.nr_banks; bank++) {
        bank_start = mbs->banks[bank].start;
        bank_size = mbs->banks[bank].size;
        bank_end = bank_start + bank_size;

        set_idx_range(__pa_to_pfn(bank_start),
                      __pa_to_pfn(bank_end));
    }
}

// --------------------------------------------------------------
static void __bootfunc dump_va_layout(void)
{
    MSGI("[globl] Virtual Memory Layout                                  @_@\n"
         "        ----------------------------------------------------------\n"
         "        Region           Size                                Range\n"
         "        ----------------------------------------------------------\n"
         "        DATA     %8lu MB [%016lx - %016lx]\n"
         "        FIXMAP   %8lu KB [%016lx - %016lx]\n"
         "        BOOTMEM  %8lu MB [%016lx - %016lx]\n"
         "        VMAP     %8lu MB [%016lx - %016lx]\n"
         "        DMAP     %8lu MB [%016lx - %016lx]\n"
         "        ----------------------------------------------------------\n",
         HYPOS_DATA_VIRT_SIZE / MB(1),
         HYPOS_DATA_VIRT_START,
         HYPOS_DATA_VIRT_START + HYPOS_DATA_VIRT_SIZE,
         (FIXADDR_END - FIXADDR_START) / KB(1),
         FIXADDR_START, FIXADDR_END,
         HYPOS_BOOTMEM_SIZE / MB(1),
         HYPOS_BOOTMEM_START,
         HYPOS_BOOTMEM_END,
         HYPOS_VMAP_VIRT_SIZE / MB(1),
         HYPOS_VMAP_VIRT_START,
         HYPOS_VMAP_VIRT_END,
         HYPOS_DIRECTMAP_SIZE / MB(1),
         HYPOS_DIRECTMAP_START,
         HYPOS_DIRECTMAP_END);
}

int __bootfunc board_ram_setup(void)
{
    struct membanks *mbs = get_resv_membanks();
    struct hypos_board *this_board = board_setup();
    paddr_t code_spaddr = get_globl()->boot_spaddr;
    paddr_t code_epaddr = get_globl()->boot_epaddr;
    size_t bank_size = HYPOS_BOOTMEM_SIZE / NR_MEMBANKS;
    paddr_t resv_mem_start;
    int bank;

    if (this_board->dram.start != code_spaddr)
        MSGE("Code ain't Start at %016lx but %016lx\n",
                this_board->dram.start, code_spaddr);

    MSGI("[globl] Physical Memory Available                              @_<\n"
         "        ----------------------------------------------------------\n"
         "        DRAM  [%016lx - %016lx] %8lu MB\n"
         "        USED  [%016lx - %016lx] %8lu BYTES\n"
         "        ----------------------------------------------------------\n",
         this_board->dram.start, this_board->dram.end,
         this_board->dram.size / MB(1),
         code_spaddr, code_epaddr,
         (code_epaddr - code_spaddr));

    resv_mem_start = PAGE_ALIGN(code_epaddr);

    for (bank = 0; bank < mbs->stats.max_banks; bank++) {
        mbs->banks[bank].start = resv_mem_start + (bank * PAGE_SIZE);
        mbs->banks[bank].size  = bank_size;
        mbs->stats.nr_banks++;
    }

    DEBUG("Reserved Membanks been Initialized [%u Banks]\n",
            mbs->stats.nr_banks);

    return 0;
}

int __bootfunc bootmem_setup(void)
{
    const struct membanks *mbs = get_resv_membanks();
    unsigned int bank;
    unsigned long mb_start = 0, mb_end = 0, mb_size = 0,
                  mbs_start = 0, mbs_end = 0, mbs_size = 0;

    dump_va_layout();

    idx_setup();

    __bootmem_setup();

    for (bank = 0; bank < mbs->stats.max_banks; bank++) {
        mb_start = mbs->banks[bank].start;
        mb_size  = mbs->banks[bank].size;
        mb_end   = mb_start + mb_size;

        mbs_start = min(mb_start, mbs_start);
        mbs_size += mb_size;
        mbs_end   = max(mb_end, mbs_end);

        directmap_setup(PFN_DOWN(mb_start),
                        PFN_DOWN(mb_size));
    }

    DEBUG("[banks] MEMBANK [%016lx - %016lx] of %8lx MB\n",
            mbs_start, mbs_end, mbs_size / MB(1));

    pageframe_total_pages = mbs_size >> PAGE_SHIFT;
    directmap_va_end      = HYPOS_HEAP_VIRT_START + mbs_end - mbs_start;
    directmap_pfn_start   = pa_to_pfn(mbs_start);
    directmap_pfn_end     = pa_to_pfn(mbs_end);

    MSGI("[globl] directmap_va_end      %016lx\n"
         "        directmap_pfn_start   %016lx\n"
         "        directmap_pfn_end     %016lx\n"
         "        pageframe_total_pages %016lx\n",
        directmap_va_end, pfn_get(directmap_pfn_start),
        pfn_get(directmap_pfn_end), pageframe_total_pages);

    pageframe_setup(mbs_start, mbs_end);

    return 0;
}
// --------------------------------------------------------------
