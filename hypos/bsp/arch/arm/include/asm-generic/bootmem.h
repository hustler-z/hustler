/**
 * Hustler's Project
 *
 * File:  bootmem.h
 * Date:  2024/07/01
 * Usage:
 */

#ifndef _ASM_GENERIC_BOOTMEM_H
#define _ASM_GENERIC_BOOTMEM_H
// --------------------------------------------------------------
#include <asm/at.h>
#include <common/type.h>

/* Boot-time Allocaters */
#define NR_MEMBANKS         (256) /* Total of 256 Boot Pages */
#define INVALID_PADDR       (~0UL)

struct membank {
    unsigned long start;
    unsigned long size;
    unsigned int type;
    unsigned int nr_pfns;
};

enum membank_type {
    STATIC_MEMBANK_TYPE,
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

    struct membank banks[NR_MEMBANKS];
};

struct bootmem {
    struct membanks boot_mem;
    struct membanks heap_mem;
};

struct memchunk {
    unsigned long start, end;
};

pfn_t get_memchunks(unsigned long nr_pfns,
                    unsigned long pfn_align);
void memchunks_setup(paddr_t ps, paddr_t pe);

int bootmem_setup(void);

// --------------------------------------------------------------
#endif /* _ASM_GENERIC_BOOTMEM_H */
