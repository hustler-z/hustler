/**
 * Hustler's Project
 *
 * File:  board.c
 * Date:  2024/06/07
 * Usage: some basic setups before normal bootflow.
 */

#include <generic/board.h>
#include <asm-generic/section.h>

// --------------------------------------------------------------

int __bootfunc board_setup(void)
{
#ifdef __RK3568__
#include <rk3568/board.h>
    board_debug_uart_init();
#endif
    return 0;
}

// --------------------------------------------------------------
