/**
 * Hustler's Project
 *
 * File:  alloc.c
 * Date:  2024/05/22
 * Usage: memory management system setup
 *        (Initialization, Allocation, Deallocation)
 */

#include <asm-generic/section.h>
#include <bsp/alloc.h>
#include <bsp/check.h>
#include <bsp/debug.h>
#include <lib/strops.h>
#include <lib/convert.h>

// --------------------------------------------------------------

/* Boot Time Memory Slots
 */
struct bootmem_slot {
    unsigned long start, end;
};

static char __initdata badpage[100] = "";

pfn_t valid_1st_pfn = INVALID_PFN_INIT;

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

    ASSERT((i < nr_bootmem_slots) && (end > bootmem_slots[i].start));
    ASSERT(nr_bootmem_slots == (PAGE_SIZE / sizeof(struct bootmem_slot)));

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

int __bootfunc bootmem_setup(paddr_t ps, paddr_t pe)
{
    unsigned long bad_spfn, bad_epfn;
    const char *ptr;

    DEBUG("%s() been Called.\n", __func__);

    ps = ROUND_PGUP(ps);
    pe = ROUND_PGDOWN(pe);
    if (pe <= ps)
        return -1;

    valid_1st_pfn = pfn_min(pa_to_pfn(ps), valid_1st_pfn);
    bootmem_slot_add(ps >> PAGE_SHIFT, pe >> PAGE_SHIFT);
    ptr = badpage;

    while (*ptr != '\0') {
        bad_spfn = conv_strtoul(ptr, &ptr, 0);
        bad_epfn = bad_spfn;

        if (*ptr == '-') {
            ptr++;
            bad_epfn = conv_strtoul(ptr, &ptr, 0);
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

pfn_t __bootfunc boot_page_alloc(unsigned long nr_pfns,
                                 unsigned long pfn_align)
{
    unsigned long pg, _end;
    unsigned int i = nr_bootmem_slots;
    struct bootmem_slot *slot = NULL;

    DEBUG("%s() been Called.\n", __func__);

    while (i--) {
        slot = &bootmem_slots[i];
        pg = (slot->end - nr_pfns) & ~(pfn_align - 1);

        if (pg >= slot->end || pg < slot->start)
            continue;
        _end = slot->end;
        bootmem_slot_add(pg + nr_pfns, _end);

        return to_pfn_t(pg);
    }

    BUG();

    return to_pfn_t(0);
}
// --------------------------------------------------------------

/* hypervisor: small chunk of memory allocator
 * a.k.a. bite-size allocator
 */
void *balloc(size_t len)
{
    void *bvaddr = NULL;

    /* TODO
     */
    return bvaddr;
}

void *bcalloc(size_t mb_nr, size_t mb_size)
{
    size_t size = mb_nr * mb_size;
    void *bvaddr;

    bvaddr = balloc(size);
    if (!bvaddr)
        return bvaddr;

    memset(bvaddr, '\0', size);

    return bvaddr;
}

void bfree(void *mem)
{

}

// --------------------------------------------------------------
