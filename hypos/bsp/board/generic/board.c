/**
 * Hustler's Project
 *
 * File:  board.c
 * Date:  2024/06/07
 * Usage: some basic setups before normal bootflow.
 */

#include <asm/debug.h>
#include <generic/board.h>
#include <asm-generic/section.h>
#include <bsp/debug.h>

// --------------------------------------------------------------

int __bootfunc board_setup(void)
{


#ifdef __RK3568__
#include <rk3568/board.h>
    MSGH("Board <%s> Setup\n", BOARD_NAME);
#endif

    return 0;
}

// --------------------------------------------------------------
