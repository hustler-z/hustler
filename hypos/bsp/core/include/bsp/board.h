/**
 * Hustler's Project
 *
 * File:  board.h
 * Date:  2024/07/03
 * Usage:
 */

#ifndef _BSP_BOARD_H
#define _BSP_BOARD_H
// --------------------------------------------------------------
#include <common/type.h>

enum board_option {
    RADXA_ZERO3 = 0,
    RADXA_ZERO2PRO,
    /* TODO */
};

enum memory_region_option {
    DATA_REGION = 0,
    FIXMAP_REGION,
    VMAP_REGION,
    BOOTMEM_REGION,
    DIRECTMAP_REGION,
};

struct hypos_mem_region {
    const vaddr_t va_start;
    const size_t  size;
    paddr_t       pa_start;
};

struct hypos_ram_region {
    const paddr_t ram_start;
    const paddr_t ram_end;
    unsigned long nr_pfns;
    paddr_t map_start;
    paddr_t map_end;
};

struct hypos_mem {
    struct hypos_ram_region *dram;
    struct hypos_mem_region *data;
    struct hypos_mem_region *fixmap;
    struct hypos_mem_region *vmap;
    struct hypos_mem_region *bootmem;
    struct hypos_mem_region *directmap;
};

struct hypos_board {
    struct hypos_mem *mem;
    /* TODO */

    char *name; /* Board Name */
};

int board_setup(void);
struct hypos_ram_region *hypos_ram_get(void);
struct hypos_mem_region *hypos_mem_get(unsigned int region);
// --------------------------------------------------------------
#endif /* _BSP_BOARD_H */
