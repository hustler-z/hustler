
/**
 * Hustler's Project
 *
 * File:  at.c
 * Date:  2024/07/09
 * Usage: address translation related
 */

#include <asm/at.h>
#include <bsp/panic.h>
#include <bsp/board.h>
#include <bsp/debug.h>
// --------------------------------------------------------------

struct page *page_frame_table;

/* HEAP MEMBANKS (Physical Address to Virtual Address)
 *
 * [VA] 0x00000a0000000000 [PA] 0x0000000000200000        (_head)
 */
void *pa_to_va(paddr_t pa)
{
    struct hypos_mem_region *directmap
                        = hypos_mem_get(DIRECTMAP_REGION);

    ASSERT((pfn_to_idx(pa_to_pfn(pa)) - directmap_base_idx)
           < (directmap->size >> PAGE_SHIFT));

    return (void *)(HYPOS_HEAP_VIRT_START -
                    (directmap_base_idx << PAGE_SHIFT)
                    + pa_to_directmapoff(pa));
}

void page_frame_table_init(void)
{
    struct hypos_mem_region *bootmem
                            = hypos_mem_get(BOOTMEM_REGION);
    MSGH("Note that boot page frame table sets up @ %016lx\n",
            bootmem->va_start);
    page_frame_table = (struct page *)(bootmem->va_start);
}
// --------------------------------------------------------------
