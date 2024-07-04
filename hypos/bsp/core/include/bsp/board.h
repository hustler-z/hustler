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

struct hypos_board {
    struct {
        paddr_t start;
        paddr_t end;
        size_t  size;
    } dram;

    struct {
        paddr_t start;
        size_t  size;
    } flush;

    /* TODO */

    /* Board Name */
    char name[32];
};

struct hypos_board *board_setup(void);
// --------------------------------------------------------------
#endif /* _BSP_BOARD_H */
