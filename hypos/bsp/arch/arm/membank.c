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
#include <asm-generic/membank.h>
#include <asm-generic/section.h>
#include <asm-generic/bitops.h>
#include <asm-generic/globl.h>
#include <lib/strops.h>
#include <lib/convert.h>

// --------------------------------------------------------------
unsigned long __read_mostly page_frame_table_base_idx;
unsigned long __read_mostly page_frame_table_va_end;
unsigned long __read_mostly page_frame_table_total_pages;
unsigned long __read_mostly page_frame_table_max_pages;

unsigned long __read_mostly directmap_va_start;
unsigned long __read_mostly directmap_va_end;
pfn_t __read_mostly directmap_pfn_start = INVALID_PFN_INIT;
pfn_t __read_mostly directmap_pfn_end = INVALID_PFN_INIT;
unsigned long __read_mostly directmap_base_idx;

pfn_t first_valid_pfn = INVALID_PFN_INIT;
unsigned int __initdata nr_memchunks;

static char __initdata bad_memchunk[100]="";

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

pfn_t __bootfunc get_memchunks(unsigned long nr_pfns,
                               unsigned long pfn_align)
{
    unsigned long pg, _end;
    unsigned int  i = 0;

    BUG_ON(!nr_memchunks);
    MSGQ(false, "Tryna allocate %4lu PFNs out from %3u memchunks @_@\n",
            nr_pfns, nr_memchunks);

    while (i++ < nr_memchunks) {
        struct memchunk *mc = &memchunks_list[i];

        pg = (mc->end - nr_pfns) & ~(pfn_align - 1);

        if (pg >= mc->end || pg < mc->start)
            continue;

        _end = mc->end;
        mc->end = pg;
        memchunks_add(pg + nr_pfns, _end);

        MSGQ(false, "Note [%08lx - %08lx] (%04lu PFNs) been allocated out from "
                   "membank<%04u> [%08lx - %08lx]\n",
              pg + nr_pfns, _end, nr_pfns, i, mc->start, mc->end);

        return pfn_set(pg);
    }

    /* XXX: Check the usage when memchunk allocator crashed here */
    MSGE("Ain't no membanks [%3u/%3u] when tryna get %4lu PFNs\n",
            i > nr_memchunks ? 257 : i, nr_memchunks, nr_pfns);

    BUG();

    /* Ain't gonna reach here */
    return INVALID_PFN;
}

void __bootfunc memchunks_setup(paddr_t ps, paddr_t pe)
{
    ps = ROUND_PGUP(ps);
    pe = ROUND_PGDOWN(pe);

    if (pe <= ps)
        return;

    first_valid_pfn = pfn_min(pa_to_pfn(ps), first_valid_pfn);
    memchunks_add(ps >> PAGE_SHIFT, pe >> PAGE_SHIFT);
}
// --------------------------------------------------------------
static void __bootfunc directmap_setup(unsigned long base_pfn,
                            unsigned long nr_pfns)
{
    int rc;
    const struct hypos_board *this_board = board_get();

    /* First call sets the directmap physical and virtual offset. */
    if (pfn_eq(directmap_pfn_start, INVALID_PFN)) {
        unsigned long pfn_gb = base_pfn &
            ~((PGTBL_LEVEL_SIZE(1) >> PAGE_SHIFT) - 1);

        directmap_pfn_start  = pfn_set(base_pfn);
        directmap_base_idx   = __pfn_to_idx(base_pfn);
        directmap_va_start   = this_board->vmem.directmap.start
            + (base_pfn - pfn_gb) * PAGE_SIZE;
    }

    if (base_pfn < pfn_get(directmap_pfn_start))
        exec_panic("Cannot add direct mapping at %lx below heap start %lx\n",
              base_pfn, pfn_get(directmap_pfn_start));

    rc = map_pages((vaddr_t)__pfn_to_va(base_pfn), pfn_set(base_pfn),
            nr_pfns, PAGE_HYPOS_RW | _PAGE_BLOCK);
    if (rc)
        exec_panic("Unable to setup the direct mappings.\n");
}

static void __bootfunc page_frame_table_setup(paddr_t ps, paddr_t pe)
{
    unsigned long nr_idx = pfn_to_idx(pfn_add(pa_to_pfn(pe), -1))
                           - pfn_to_idx(pa_to_pfn(ps)) + 1;
    unsigned long page_frame_table_size = nr_idx * sizeof(struct page);
    const struct hypos_vmem *vmem = vmem_get();
    unsigned long nr_pfns = page_frame_table_size >> PAGE_SHIFT;
    pfn_t base_pfn;
    int rc;

    if (page_frame_table_size > vmem->boot.size)
        exec_panic("The frame can't cover the PA [%016lx - %016lx]\n",
                ps, pe);

    page_frame_table_base_idx = pfn_to_idx(pa_to_pfn(ps));
    base_pfn = get_memchunks(nr_pfns, 1);

    MSGQ(true, "Page frame table base PFN %08lx [size of %8lu bytes]\n",
            pfn_get(base_pfn), page_frame_table_size);

    rc = map_pages(vmem->boot.start, base_pfn,
                   page_frame_table_size >> PAGE_SHIFT,
                   PAGE_HYPOS_RW | _PAGE_BLOCK);
    if (rc)
        exec_panic("Unable to setup page frame table mapping");

    MSGQ(true, "Zero out page frame table\n");
    memset(&page_frame_table[0], 0, nr_idx * sizeof(struct page));

    page_frame_table_va_end = vmem->boot.start
            + (nr_idx * sizeof(struct page));
}
// --------------------------------------------------------------
static void __bootfunc dump_va(void)
{
    const struct hypos_board *this_board = board_get();

    MSGI("[globl] Virtual Memory Layout                                  @_@\n"
         "        ----------------------------------------------------------\n"
         "        Region           Size                                Range\n"
         "        ----------------------------------------------------------\n"
         "        DATA     %8lu MB [%016lx - %016lx]\n"
         "        FIXMAP   %8lu KB [%016lx - %016lx]\n"
         "        BOOT     %8lu MB [%016lx - %016lx]\n"
         "        VMAP     %8lu MB [%016lx - %016lx]\n"
         "        DMAP     %8lu MB [%016lx - %016lx]\n"
         "        ----------------------------------------------------------\n",
         this_board->vmem.data.size / MB(1),
         this_board->vmem.data.start,
         this_board->vmem.data.end,
         this_board->vmem.fixmap.size / KB(1),
         this_board->vmem.fixmap.start,
         this_board->vmem.fixmap.end,
         this_board->vmem.boot.size / MB(1),
         this_board->vmem.boot.start,
         this_board->vmem.boot.end,
         this_board->vmem.vmap.size / MB(1),
         this_board->vmem.vmap.start,
         this_board->vmem.vmap.end,
         this_board->vmem.directmap.size / MB(1),
         this_board->vmem.directmap.start,
         this_board->vmem.directmap.end);
}

static void __bootfunc mbb_setup(paddr_t ps, paddr_t pe)
{
    struct membanks *mbb = get_boot_membanks();
    const struct hypos_vmem *vmem = vmem_get();
    size_t bank_size = vmem->boot.size / mbb->stats.max_banks;
    unsigned int bank;
    unsigned long mb_start, mb_end;

    /* Set up boot membanks */
    for (bank = 0; bank < mbb->stats.max_banks; bank++) {
        mbb->banks[bank].start = ps + (bank * bank_size);
        mbb->banks[bank].size  = bank_size;
        mbb->banks[bank].type  = PAGE_MEMBANK_TYPE;
        mbb->stats.nr_banks++;
    }

    mbb->range.start = ps;
    mbb->range.end = mbb->banks[mbb->stats.nr_banks - 1].start + bank_size;

    ASSERT(mbb->range.end == pe);

    /* Set up memchunk allocators. */
    for (bank = 0; bank < mbb->stats.nr_banks; bank++) {
        const struct membank *mb = &mbb->banks[bank];
        memchunks_setup(mb->start, mb->start + mb->size);
    }

    /* Set up direct mapping for page frames */
    for (bank = 0; bank < mbb->stats.nr_banks; bank++) {
        const struct membank *mb = &mbb->banks[bank];
        mb_start = min(mb->start, mb_start);
        mb_end   = max(mb->start + mb->size, mb_end);

        directmap_setup(PFN_DOWN(mb->start),
                        PFN_DOWN(mb->size));
    }

    page_frame_table_setup(mbb->range.start, mbb->range.end);
}

/* Now First of All, Save Enough Memory Space for Page Frames:
 *
 * Number of PFNs = ram_size / PAGE_SIZE
 * Reserved Memory = Number of PFNs * sizeof(struct page)
 *
 * To set up the allocator to be able to allocate good amout of
 * contiguous memory for page frame table.
 */
static void __bootfunc mb_setup(void)
{
    struct hypos_pmem *pmem = pmem_get();
    struct hypos_vmem *vmem = vmem_get();
    const paddr_t code_spaddr = get_globl()->boot_spaddr;
    const paddr_t code_epaddr = get_globl()->boot_epaddr;

    if (pmem->dram.start != code_spaddr)
        MSGE("Code ain't start at %016lx but %016lx\n",
                pmem->dram.start, code_spaddr);

    /* To check the physical memory range - RAM Size */
    MSGI("[globl] Physical Memory Available                              @_<\n"
         "        ----------------------------------------------------------\n"
         "        DRAM  [%016lx - %016lx]    %8lu MB\n"
         "        USED  [%016lx - %016lx)    %8lu KB\n"
         "        LEFT  [%016lx - %016lx]    %8lu MB\n"
         "        ----------------------------------------------------------\n",
         pmem->dram.start,
         pmem->dram.end,
         pmem->dram.size / MB(1),
         code_spaddr, code_epaddr,
         (code_epaddr - code_spaddr) / KB(1),
         code_epaddr, pmem->dram.end,
         (pmem->dram.end - code_epaddr) / MB(1));

    pmem->nr_pfns = (pmem->dram.end - code_epaddr) / PAGE_SIZE;

    pmem->avail.start = PAGE_ALIGN(code_epaddr);
    pmem->avail.end   = pmem->dram.end;

    /* Ensure that boot_mbs is good enough for contiguous page frame table */
    ASSERT((pmem->nr_pfns * sizeof(struct page)) <= (vmem->boot.size / NR_BOOT_MEMBANKS));

    MSGH("This board needs %8lu PFNs (size of %4lu MB) to set up page frame table\n",
        pmem->nr_pfns, (pmem->nr_pfns * sizeof(struct page)) / MB(1));

    mbb_setup(pmem->avail.start, pmem->avail.start + vmem->boot.size);
}

int __bootfunc bootmem_setup(void)
{
    const struct membanks *mbb = (const struct membanks *)get_boot_membanks();
    unsigned int bank;
    unsigned long mbh_start = INVALID_PADDR,
                  mbh_end = 0,
                  mbh_size = 0;

    dump_va();

    mb_setup();

    page_frame_table_total_pages = mbh_size >> PAGE_SHIFT;
    directmap_va_end    = HYPOS_HEAP_VIRT_START + mbh_end - mbh_start;
    directmap_pfn_start = pa_to_pfn(mbh_start);
    directmap_pfn_end   = pa_to_pfn(mbh_end);

    MSGI("[globl] directmap_va_end      %016lx\n"
         "        directmap_pfn_start   %016lx\n"
         "        directmap_pfn_end     %016lx\n"
         "        page_frame_table_total_pages   %16lu\n",
        directmap_va_end, pfn_get(directmap_pfn_start),
        pfn_get(directmap_pfn_end), page_frame_table_total_pages);


    page_frame_table_max_pages = PFN_DOWN(mbh_end);

    return 0;
}
// --------------------------------------------------------------
