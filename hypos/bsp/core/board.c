/**
 * Hustler's Project
 *
 * File:  board.c
 * Date:  2024/07/08
 * Usage: board related implementation
 */

#include <bsp/board.h>
#include <asm-generic/section.h>
#include <asm-generic/globl.h>

// --------------------------------------------------------------
extern struct hypos_board radax_zero3w;

struct hypos_board *__bootfunc board_get(void)
{
    struct hypos_board *this;

    switch (get_globl()->board_type) {
    case RADXA_ZERO3:
        this = &radax_zero3w;
        break;
    default:
        break;
    }

    return this;
}

struct hypos_vmem *__bootfunc vmem_get(void)
{
    struct hypos_vmem *vmem;

    switch (get_globl()->board_type) {
    case RADXA_ZERO3:
        vmem = &radax_zero3w.vmem;
        break;
    default:
        break;
    }

    return vmem;
}

struct hypos_pmem *__bootfunc pmem_get(void)
{
    struct hypos_pmem *pmem;

    switch (get_globl()->board_type) {
    case RADXA_ZERO3:
        pmem = &radax_zero3w.pmem;
        break;
    default:
        break;
    }

    return pmem;
}

int __bootfunc board_setup(void)
{
    get_globl()->board_type = RADXA_ZERO3;

    return 0;
}
// --------------------------------------------------------------
