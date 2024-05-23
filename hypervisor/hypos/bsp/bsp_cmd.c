/**
 * Hustler's Project
 *
 * File:  bsp_cmd.c
 * Date:  2024/05/20
 * Usage: general commands interact with console
 */

#include <bsp_cmd.h>

/* ------------------------------------------------------------------------
 * meminfo
 * ------------------------------------------------------------------------
 */
static void bsp_meminfo(void)
{

}

/* ------------------------------------------------------------------------
 * cpuinfo
 * ------------------------------------------------------------------------
 */
static void bsp_cpuinfo(void)
{

}

/* ------------------------------------------------------------------------
 * hypinfo
 * ------------------------------------------------------------------------
 */
static void bsp_hypinfo(void)
{

}

/* ------------------------------------------------------------------------
 * guestup [guest ID]
 * ------------------------------------------------------------------------
 */
static void bsp_guestup(int guestid)
{

}

void bsp_cmd_register(struct bsp_cmd_ops *cmd_ops)
{
    cmd_ops->meminfo = bsp_meminfo;
    cmd_ops->cpuinfo = bsp_cpuinfo;
    cmd_ops->hypinfo = bsp_hypinfo;
    cmd_ops->guestup = bsp_guestup;
}
