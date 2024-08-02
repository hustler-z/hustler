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
#include <asm/xaddr.h>
#include <bsp/type.h>

/* Boot-time Allocaters for Page Frame Table and other Purposes */
#define NR_BOOT_MEMBANKS    (8)      /* Total of 8 Boot Pages */
#define INVALID_PADDR       (~0UL)

struct membank {
    hpa_t        start;
    size_t       size;
    unsigned int type; /* Assoicated with hpm_type (bsp/board.h) */
    unsigned int nr_hfns;
};

struct membanks {
    struct {
        unsigned int nr_banks;
        unsigned int max_banks;
    } stats;

    struct membank banks[NR_BOOT_MEMBANKS];
};

struct memblock {
    struct membanks boot_mbs;
    /* TODO */
};

struct memchunk {
    unsigned long start, end;
};

hfn_t get_memchunks(unsigned long nr_hfns,
                    unsigned long hfn_align);
void memchunks_setup(hpa_t ps, hpa_t pe);

int bootmem_setup(void);

// --------------------------------------------------------------
#endif /* _ORG_BOOTMEM_H */
