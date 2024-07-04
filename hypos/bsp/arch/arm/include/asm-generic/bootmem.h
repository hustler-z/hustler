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

/* Boot-time Allocaters */
#define NR_MEMBANKS         (256) /* Total of 256 Boot Pages */

struct membank {
    unsigned long start;
    unsigned long size;
    unsigned long type;
};

struct membanks {
    struct {
        unsigned int nr_banks;
        unsigned int max_banks;
    } stats;
    struct membank banks[NR_MEMBANKS];
};

struct bootmem {
    struct membanks resv_mem;
    struct membanks heap_mem;
};

struct bootpage {
    unsigned long start, end;
};

pfn_t get_bootpages(unsigned long nr_pfns,
                    unsigned long pfn_align);
void bootpages_setup(paddr_t ps, paddr_t pe);
int board_ram_setup(void);
int bootmem_setup(void);

enum {
    MEMBANK_NORMAL = 0,
    MEMBANK_FDT,
    MEMBANK_HEAP,
};
// --------------------------------------------------------------
#endif /* _ASM_GENERIC_BOOTMEM_H */
