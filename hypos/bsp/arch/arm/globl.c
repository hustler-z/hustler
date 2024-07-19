/**
 * Hustler's Project
 *
 * File:  global.c
 * Date:  2024/05/22
 * Usage:
 */


#include <org/globl.h>
#include <org/section.h>
#include <lib/list.h>
#include <bsp/panic.h>

// --------------------------------------------------------------

/* The one and only hypos global data tracker: globl
 */
static struct hypos_globl __initdata boot_globl = {
    .console_enable = true,
    .keyboard_enable = true,
#ifdef __RK3568__
    .baudrate = 1500000,
#endif
    .accessible = true,
    .smode = GLB_EARLY_SERIAL,
    .flags = GLB_INITIALIZED,
    .hypos_status = HYPOS_EARLY_BOOT_STAGE,
};

struct hypos_globl *get_globl(void)
{
    struct hypos_globl *globl = &boot_globl;

    ASSERT(globl->accessible);

    return globl;
}

// --------------------------------------------------------------
