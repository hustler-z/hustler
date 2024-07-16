/**
 * Hustler's Project
 *
 * File:  bootmem.h
 * Date:  2024/07/01
 * Usage:
 */

#ifndef _ORG_BOOTMEM_H
#define _ORG_BOOTMEM_H
// --------------------------------------------------------------
#include <asm/at.h>
#include <bsp/type.h>

/* Boot-time Allocaters for Page Frame Table and other Purposes */
#define NR_BOOT_MEMBANKS    (8)      /* Total of 8 Boot Pages */
#define INVALID_PADDR       (~0UL)

struct membank {
    unsigned long start;
    unsigned long size;
    unsigned int type;
    unsigned int nr_pfns;
};

enum membank_type {
    PAGE_MEMBANK_TYPE,
    HEAP_MEMBANK_TYPE,
    INVALID_MEMBANK_TYPE,
};

struct membanks {
    struct {
        unsigned int nr_banks;
        unsigned int max_banks;
    } stats;

    struct {
        paddr_t start;
        paddr_t end;
    } range;

    struct membank banks[NR_BOOT_MEMBANKS];
};

struct memblock {
    struct membanks boot_mbs;
    /* TODO */
};

struct memchunk {
    unsigned long start, end;
};

pfn_t get_memchunks(unsigned long nr_pfns,
                    unsigned long pfn_align);
void memchunks_setup(paddr_t ps, paddr_t pe);

int bootmem_setup(void);

// --------------------------------------------------------------
#endif /* _ORG_BOOTMEM_H */
