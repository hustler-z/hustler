/**
 * Hustler's Project
 *
 * File:  global.c
 * Date:  2024/05/22
 * Usage:
 */


#include <asm-generic/globl.h>
#include <asm-generic/section.h>
#include <lib/list.h>
#include <bsp/debug.h>

// --------------------------------------------------------------

/* The one and only hypos global data tracker: globl
 */
struct hypos_globl __initdata boot_globl = {
    .console_enable = true,
    .keyboard_enable = true,
#ifdef __RK3568__
    .baudrate = 1500000,
#endif
    .smode = GLB_EARLY_SERIAL,
    .flags = GLB_INITIALIZED,
    .boot_status = EARLY_BOOT_STAGE,
};

struct hypos_globl *get_globl(void)
{
    struct hypos_globl *globl;

    return (globl = &boot_globl);
}
// --------------------------------------------------------------
