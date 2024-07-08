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

struct hypos_vmem_range {
    const vaddr_t start;
    const vaddr_t end;
    const size_t  size;
};

struct hypos_vmem {
    struct hypos_vmem_range data;
    struct hypos_vmem_range fixmap;
    struct hypos_vmem_range boot;
    struct hypos_vmem_range vmap;
    struct hypos_vmem_range directmap;
};

struct hypos_pmem {
    struct {
        const paddr_t start;
        const paddr_t end;
        const size_t  size;
    } dram;

    struct {
        const paddr_t start;
        const paddr_t end;
        const size_t  size;
    } flush;

    /* Available physical memory for hypos
     * memory space mapping.
     */
    struct {
        paddr_t start;
        paddr_t end;
    } avail;

    /* Number of PFNs for the whole RAM */
    size_t nr_pfns;
};

struct hypos_board {
    struct hypos_pmem pmem;
    struct hypos_vmem vmem;

    /* TODO */

    /* Board Name */
    char name[32];
};

struct hypos_board *board_get(void);
struct hypos_vmem  *vmem_get(void);
struct hypos_pmem  *pmem_get(void);
int board_setup(void);
// --------------------------------------------------------------
#endif /* _BSP_BOARD_H */
