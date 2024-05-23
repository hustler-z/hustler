/**
 * Hustler's Project
 *
 * File:  bsp_console.c
 * Date:  2024/05/20
 * Usage: console initialization
 */

#include <bsp_console.h>
#include <bsp_cmd.h>
#include <bsp_print.h>

struct bsp_cmd_ops console_cmd_ops;

void __init bsp_console_setup(void)
{
    bsp_cmd_register(&console_cmd_ops);
}
