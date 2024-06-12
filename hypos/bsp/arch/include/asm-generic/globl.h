/**
 * Hustler's Project
 *
 * File:  bsp.h
 * Date:  2024/05/21
 * Usage: Global data can be used everywhere
 */

#ifndef _ASM_GENERIC_GLOBAL_H
#define _ASM_GENERIC_GLOBAL_H
// --------------------------------------------------------------

#ifndef __ASSEMBLY__

#include <bsp/device.h>
#include <bsp/jump.h>
#include <lib/list.h>
#include <asm/globl.h>
#include <asm-generic/bitops.h>
#include <generic/type.h>

/* Hypos Global Data Container
 */
struct hypos_globl {
    unsigned long       flags;
    unsigned long       baudrate;
    unsigned int        precon_buf_idx;
    unsigned int        console_enable;
    unsigned int        keyboard_enable;
    struct hlist_head   *pw_list;
    struct hypos_device_table *dev_tbl;
    struct funcjmp      *fjmp;
    struct arch_globl   arch;
};

enum hypos_globl_flags {
    GLB_INITIALIZED      = BIT(0, UL),
    GLB_PERIODIC_RUNNING = BIT(1, UL),
    GLB_DEV_INITIALIZED  = BIT(2, UL),
};

/* TODO better algorithm for this global tracker
 *
 * Currently, access glb with external linkage:
 *
 * extern struct hypos_globl *glb;
 *
 * simple implementation allow me to track things going on in
 * hypos.
 */
int glb_setup(void);

bool glb_is_initialized(void);

#endif /* !__ASSEMBLY__ */
// --------------------------------------------------------------
#endif /* _ASM_GENERIC_GLOBAL_H */
