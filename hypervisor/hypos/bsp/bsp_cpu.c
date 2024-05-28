/**
 * Hustler's Project
 *
 * File:  bsp_cpu.c
 * Date:  2024/05/20
 * Usage: general CPU initialization (core components)
 */

#include <bsp_cpu.h>
#include <bsp_print.h>
#include <board_clk.h>
#include <board_timer.h>
#include <board_uart.h>
#include <board_led.h>
#include <cpu_gic.h>
#include <cpu_mmu.h>

static void bsp_hypos_tags(void)
{
    bsp_print("                  _____ _____      _____ ____  \n");
    bsp_print(" /A__/A  /A  /A  / ___//_  _//A   / ___// __ A \n");
    bsp_print(" V  __ A V A_V A V___A  / / / /_ / ___A A __ / \n");
    bsp_print("  V_A V_A V____//____/  V/  V___A____A  V/  V  HYPOS\n");
    bsp_print("\n");
}

void __init bsp_cpu_setup(void)
{
    cpu_mmu_setup();

    cpu_gic_setup();

    board_serial_setup();

    board_timer_setup();

    board_clk_setup();

    bsp_hypos_tags();
}
