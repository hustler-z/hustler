/**
 * Hustler's Project
 *
 * File:  mem.h
 * Date:  2024/06/21
 * Usage:
 */

#ifndef _ASM_GENERIC_BOOTMEM_H
#define _ASM_GENERIC_BOOTMEM_H
// --------------------------------------------------------------
#include <asm/vpa.h>
#include <asm-generic/bitops.h>
#include <common/type.h>

#define NR_BOOTPAGES        (256)
#define INVALID_PADDR       (~0UL)

struct bootpage {
    vaddr_t start;
    size_t size;
    unsigned int type;
};

struct bootpages {
    unsigned int nr_bootpages;
    unsigned int max_bootpages;
    struct bootpage pages[NR_BOOTPAGES];
};

struct bootmem {
    struct bootpages hypos_bootpages;
};

int bootmem_setup(void);

#define PAGE_INDEX_SHIFT       PGTBL_LEVEL_SHIFT(2)
#define PAGE_INDEX_COUNT \
    ((1 << PAGE_INDEX_SHIFT) / (ISOLATE_LSB(sizeof(*page_head))))

enum bootmem_type {
    BOOTMEM_DEFAULT,
    BOOTMEM_STATIC_HEAP,
    BOOTMEM_RESERVED,
};
// --------------------------------------------------------------
int boot_page_setup(paddr_t ps, paddr_t pe);
pfn_t alloc_boot_page(unsigned long nr_pfns,
                      unsigned long pfn_align);
// --------------------------------------------------------------
#endif /* _ASM_GENERIC_BOOTMEM_H */
