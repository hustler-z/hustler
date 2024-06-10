/**
 * Hustler's Project
 *
 * File:  cpu.h
 * Date:  2024/05/21
 * Usage:
 */

#ifndef _BSP_CPU_H
#define _BSP_CPU_H
// --------------------------------------------------------------

#ifdef __RK3568__
#include <rk3568/board.h>
#endif

#define NR_CPU     NR_BOARD_CPU

void bsp_cpu_setup(void);

// --------------------------------------------------------------
#endif /* _BSP_CPU_H */
