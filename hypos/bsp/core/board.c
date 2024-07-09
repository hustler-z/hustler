/**
 * Hustler's Project
 *
 * File:  board.c
 * Date:  2024/07/08
 * Usage: board related implementation
 */

#include <asm-generic/section.h>
#include <asm-generic/globl.h>
#include <bsp/board.h>
#include <bsp/debug.h>

// --------------------------------------------------------------
extern struct hypos_board radax_zero3w;

static __initdata char *board_name_opt[] = {
    "Radax Zero 3W",
    "Radax Zero 2Pro",
};

static struct hypos_board *__bootfunc board_get(void)
{
    struct hypos_board *this;

    switch (get_globl()->board_type) {
    case RADXA_ZERO3:
        this = &radax_zero3w;
        break;
    case RADXA_ZERO2PRO:
        break;
    default:
        break;
    }

    return this;
}

int __bootfunc board_setup(void)
{
    struct hypos_board *this_board;

    get_globl()->board_type = RADXA_ZERO3;
    this_board = board_get();
    this_board->name =
            board_name_opt[get_globl()->board_type];

    MSGH("This board model is %s\n", this_board->name);

    return 0;
}

struct hypos_ram_region *__bootfunc hypos_ram_get(void)
{
    struct hypos_board *this = board_get();

    return this->mem->dram;
}

struct hypos_mem_region *__bootfunc
                    hypos_mem_get(unsigned int region)
{
    struct hypos_board *this = board_get();

    switch (region) {
    case DATA_REGION:
        return this->mem->data;
    case FIXMAP_REGION:
        return this->mem->fixmap;
    case VMAP_REGION:
        return this->mem->vmap;
    case BOOTMEM_REGION:
        return this->mem->bootmem;
    case DIRECTMAP_REGION:
        return this->mem->directmap;
    default:
        return NULL;
    }
}
// --------------------------------------------------------------
