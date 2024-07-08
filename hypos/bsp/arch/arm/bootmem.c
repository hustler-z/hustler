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
#include <asm-generic/bitops.h>
#include <asm-generic/globl.h>
#include <lib/strops.h>
#include <lib/convert.h>

// --------------------------------------------------------------
unsigned long __read_mostly pgframe_base_idx;
unsigned long __read_mostly pgframe_va_end;
unsigned long __read_mostly pgframe_total_pages;
unsigned long __read_mostly pgframe_max_pages;

unsigned long __read_mostly directmap_va_start;
unsigned long __read_mostly directmap_va_end;
pfn_t __read_mostly directmap_pfn_start = INVALID_PFN_INIT;
pfn_t __read_mostly directmap_pfn_end = INVALID_PFN_INIT;
unsigned long __read_mostly directmap_base_idx;

pfn_t first_valid_pfn = INVALID_PFN_INIT;
unsigned int __initdata nr_memchunks;

static char __initdata bad_memchunk[100]="";

/* Static allocate a page for memchunks_list */
struct memchunk __initdata
    memchunks_list[PAGE_SIZE / sizeof(struct memchunk)];

static __initdata struct bootmem bootmem = {
    .boot_mem.stats.max_banks = NR_MEMBANKS,
    .heap_mem.stats.max_banks = NR_MEMBANKS,
};

static inline struct membanks *get_heap_membanks(void)
{
    return &bootmem.heap_mem;
}

static inline struct membanks *get_boot_membanks(void)
{
    return &bootmem.boot_mem;
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
    MSGQ(false, "Tryna Allocate %4lu PFNs out from %3u Memchunks @_@\n",
            nr_pfns, nr_memchunks);

    while (i++ < nr_memchunks) {
        struct memchunk *mc = &memchunks_list[i];

        pg = (mc->end - nr_pfns) & ~(pfn_align - 1);

        if (pg >= mc->end || pg < mc->start)
            continue;

        _end = mc->end;
        mc->end = pg;
        memchunks_add(pg + nr_pfns, _end);

        MSGQ(true, "Note [%08lx - %08lx] (%2lu PFNs) been Allocated out from "
                    "Memchunk<%3u> [%08lx - %08lx]\n",
              pg + nr_pfns, _end, nr_pfns, i, mc->start, mc->end);

        return pfn_set(pg);
    }

    /* XXX: Check the usage when memchunk allocator crashed here */
    MSGE("Ain't no Memchunks [%3u/%3u] When Tryna Get %4lu PFNs\n",
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
        exec_panic("Cannot Add Direct Mapping at %lx Below Heap Start %lx\n",
              base_pfn, pfn_get(directmap_pfn_start));

    rc = map_pages((vaddr_t)__pfn_to_va(base_pfn), pfn_set(base_pfn),
            nr_pfns, PAGE_HYPOS_RW | _PAGE_BLOCK);
    if (rc)
        exec_panic("Unable to Setup the Direct Mappings.\n");
}

static void __bootfunc pgframe_setup(paddr_t ps, paddr_t pe)
{
    unsigned long nr_idx = pfn_to_idx(pfn_add(pa_to_pfn(pe), -1))
                           - pfn_to_idx(pa_to_pfn(ps)) + 1;
    unsigned long pgframe_size = nr_idx * sizeof(struct page);
    const struct hypos_vmem *vmem = vmem_get();
    pfn_t base_pfn, tmp_pfn;
    int rc, i = pgframe_size >> PAGE_SHIFT;

    if (pgframe_size > vmem->boot.size)
        exec_panic("The Frame Can't Cover the PA [%016lx - %016lx]\n",
                ps, pe);

    pgframe_base_idx = pfn_to_idx(pa_to_pfn(ps));

    for (; i > 0; i--) {
        tmp_pfn = get_memchunks(1, 1);
        if (i == 1)
            base_pfn = tmp_pfn;
    }

    MSGQ(true, "Page Frame Table Base PFN %08lx Size of %8lu Bytes\n",
            pfn_get(base_pfn), pgframe_size);

    rc = map_pages(vmem->boot.start, base_pfn,
                   pgframe_size >> PAGE_SHIFT,
                   PAGE_HYPOS_RW | _PAGE_BLOCK);
    if (rc)
        exec_panic("Unable to setup pgframe mapping");

    memset(&pgframe[0], 0, nr_idx * sizeof(struct page));
    memset(&pgframe[nr_idx], -1, pgframe_size -
            (nr_idx * sizeof(struct page)));

    pgframe_va_end = vmem->boot.start
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

static void __bootfunc mbh_setup(paddr_t ps, paddr_t pe)
{
    struct membanks *mbh = get_heap_membanks();
    unsigned int bank;
    size_t bank_size = (pe - ps) / mbh->stats.max_banks;

    /* Set up heap membanks */
    for (bank = 0; bank < mbh->stats.max_banks; bank++) {
        mbh->banks[bank].start = ps + (bank * bank_size);
        mbh->banks[bank].size  = bank_size;
        mbh->banks[bank].type  = STATIC_MEMBANK_TYPE;
        mbh->stats.nr_banks++;
    }

    mbh->range.start = ps;
    mbh->range.end = mbh->banks[mbh->stats.nr_banks - 1].start + bank_size;

    ASSERT(mbh->range.end <= pe);
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
        mbb->banks[bank].type  = HEAP_MEMBANK_TYPE;
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
}

/* Now First of All, Save Enough Memory Space for Page Frames:
 *
 * Number of PFNs = ram_size / PAGE_SIZE
 * Reserved Memory = Number of PFNs * sizeof(struct page)
 */
static void __bootfunc mb_setup(void)
{
    struct hypos_pmem *pmem = pmem_get();
    struct hypos_vmem *vmem = vmem_get();
    const paddr_t code_spaddr = get_globl()->boot_spaddr;
    const paddr_t code_epaddr = get_globl()->boot_epaddr;

    if (pmem->dram.start != code_spaddr)
        MSGE("Code ain't Start at %016lx but %016lx\n",
                pmem->dram.start, code_spaddr);

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

    MSGH("This Board Needs to Set up %8lu PFNs (Size of %4lu MB)\n",
        pmem->nr_pfns, (pmem->nr_pfns * sizeof(struct page)) / MB(1));

    mbb_setup(code_epaddr, code_epaddr + vmem->boot.size);

    mbh_setup(code_epaddr + vmem->boot.size, pmem->dram.end);
}

int __bootfunc bootmem_setup(void)
{
    const struct membanks *mbb = (const struct membanks *)get_boot_membanks();
    const struct membanks *mbh = (const struct membanks *)get_heap_membanks();
    unsigned int bank;
    unsigned long mbh_start = INVALID_PADDR,
                  mbh_end = 0,
                  mbh_size = 0;

    dump_va();

    mb_setup();

    pgframe_setup(mbb->range.start, mbb->range.end);

    pgframe_total_pages = mbh_size >> PAGE_SHIFT;
    directmap_va_end    = HYPOS_HEAP_VIRT_START + mbh_end - mbh_start;
    directmap_pfn_start = pa_to_pfn(mbh_start);
    directmap_pfn_end   = pa_to_pfn(mbh_end);

    MSGI("[globl] directmap_va_end      %016lx\n"
         "        directmap_pfn_start   %016lx\n"
         "        directmap_pfn_end     %016lx\n"
         "        pgframe_total_pages   %16lu\n",
        directmap_va_end, pfn_get(directmap_pfn_start),
        pfn_get(directmap_pfn_end), pgframe_total_pages);


    pgframe_max_pages = PFN_DOWN(mbh_end);

    return 0;
}
// --------------------------------------------------------------
