/**
 * Hustler's Project
 *
 * File:  board.c
 * Date:  2024/07/08
 * Usage: board related implementation
 */

#include <org/section.h>
#include <org/globl.h>
#include <bsp/board.h>
#include <bsp/debug.h>
#include <bsp/panic.h>

// --------------------------------------------------------------
extern struct hypos_board radax_zero3w;

static __initdata char *board_name_opt[] = {
    "Radax Zero 3W",
    "Radax Zero 2Pro",
};

static __initdata char *board_cpu_vendor[] = {
    "Rockchip",
    "Amlogic",
};

static struct hypos_board *__bootfunc board_get(void)
{
    struct hypos_board *this;

    switch (hypos_get(board_type)) {
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

struct hpm_blk *hypos_hpm_get(unsigned int type)
{
    struct hypos_board *this = board_get();
    struct hpm_blk *hpm = NULL;
    unsigned int blk;

    for (blk = 0; blk < HPM_BLK_NR; blk++) {
        if (type == this->mem->hpm[blk].type) {
            hpm = &this->mem->hpm[blk];
            break;
        }
    }

    BUG_ON(hpm == NULL);

    return hpm;
}

struct hvm_blk *hypos_hvm_get(unsigned int type)
{
    struct hypos_board *this = board_get();
    struct hvm_blk *hvm = NULL;
    unsigned int blk;

    for (blk = 0; blk < HVM_BLK_NR; blk++) {
        if (type == this->mem->hvm[blk].type) {
            hvm = &this->mem->hvm[blk];
            break;
        }
    }

    BUG_ON(hvm == NULL);

    return hvm;
}

struct hypos_cpu *hypos_cpu_get(void)
{
    return board_get()->cpu;
}
// --------------------------------------------------------------
static void hypos_label(void)
{
    MSGI("              ___  __  ____ \n");
    MSGI("    /\\_/\\/\\/\\/ _ \\/  \\/ __/ \n");
    MSGI("    \\  _ \\  / ___/ / /\\__ \\ \n");
    MSGI("     \\/ \\/_/\\/   \\__/ /___/ Hustler Edu\n");
    MSGI("\n");
}

int __bootfunc board_setup(void)
{
    struct hypos_board *this_board;

    hypos_label();

    ASSERT(HVM_BLK_NR <= HVM_BLK_MAX);
    ASSERT(HPM_BLK_NR <= HPM_BLK_MAX);

    hypos_set(RADXA_ZERO3, board_type);
    this_board = board_get();
    this_board->name =
            board_name_opt[hypos_get(board_type)];

    MSGH("This board model is %s %s\n", this_board->name,
            (char *)this_board->cpu->priv);

    return 0;
}
// --------------------------------------------------------------
