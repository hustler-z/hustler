/**
 * Hustler's Project
 *
 * File:  bsp_setup.c
 * Date:  2024/05/20
 * Usage: general initialization
 */

#include <bsp_task.h>
#include <bsp_mem.h>
#include <bsp_cpu.h>
#include <bsp_print.h>
#include <bsp_console.h>
#include <cpu_init.h>

/* ------------------------------------------------------------------------
 * Entry C function from assembly code
 *
 * (1) CPU final setup
 *     (include mmu, gic, timer, clock, etc., core components)
 * (2) Memory setup
 * (2) Task setup
 * (3) Console setup (can kick guests up through commands)
 * ------------------------------------------------------------------------
 */
void __init bsp_setup(void)
{
    bsp_cpu_setup();

    bsp_mem_setup();

    bsp_task_setup();

    bsp_console_setup();
}
