/**
 * Hustler's Project
 *
 * File:  cpu.c
 * Date:  2024/05/20
 * Usage: general CPU initialization (core components)
 */

#include <bsp/cpu.h>
#include <bsp/stdio.h>
#include <bsp/percpu.h>
#include <generic/gicv3.h>
#include <generic/memory.h>

// --------------------------------------------------------------
unsigned long __percpu_offset[NR_CPU];

static void bsp_hypos_tags(void)
{
    pr("                  _____ _____      _____ ____  \n");
    pr(" /A__/A  /A  /A  / ___//_  _//A   / ___// __ A \n");
    pr(" V  __ A V A_V A V___A  / / / /_ / ___A A __ / \n");
    pr("  V_A V_A V____//____/  V/  V___A____A  V/  V  HYPOS\n");
    pr("\n");
}

void bsp_cpu_setup(void)
{
    set_processor_id(0);

    cpu_mem_setup();

    cpu_gicv3_setup();

    bsp_hypos_tags();
}
// --------------------------------------------------------------
