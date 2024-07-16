/**
 * Hustler's Project
 *
 * File:  bsp.h
 * Date:  2024/05/21
 * Usage: Global data can be used everywhere
 */

#ifndef _ORG_GLOBAL_H
#define _ORG_GLOBAL_H
// --------------------------------------------------------------

#ifndef __ASSEMBLY__

#include <bsp/device.h>
#include <bsp/jump.h>
#include <lib/list.h>
#include <asm/globl.h>
#include <org/bitops.h>
#include <bsp/type.h>

/* Hypos Global Data Container
 */
struct hypos_globl {
    unsigned long       flags; /* hypos_globl_flags */
    unsigned long       baudrate;
    unsigned int        smode; /* hypos_serial_mode */
    unsigned int        precon_buf_idx;
    bool                console_enable;
    bool                keyboard_enable;
    bool                print_once;
    unsigned int        hypos_status; /* hypos_status */
    unsigned int        board_type;
    unsigned long       phys_offset;
    unsigned long       boot_param;
    paddr_t             boot_spaddr;
    paddr_t             boot_epaddr;
    paddr_t             bootmem_paddr;
    size_t              bootmem_size;
    size_t              dram_size;
    struct hlist_head   pw_list;
    struct hypos_device_table *dev_tbl;
    struct funcjmp      *fjmp;
    struct arch_globl   arch;
};

enum hypos_globl_status {
    HYPOS_EARLY_BOOT_STAGE = 0,
    HYPOS_SMP_BOOT_STAGE,
    HYPOS_FINAL_BOOT_STAGE,
    HYPOS_ACTIVE_STATE,
    HYPOS_SUSPEND_STATE,
    HYPOS_RESUME_STATE,
};

enum hypos_globl_flags {
    GLB_INITIALIZED      = BIT(0, UL),
    GLB_PERIODIC_RUN     = BIT(1, UL),
    GLB_DEVICE_INIT      = BIT(2, UL),
};

enum hypos_serial_mode {
    GLB_EARLY_SERIAL     = 0x00,
    GLB_STDIO_SERIAL     = 0x01,
    GLB_BASIC_SERIAL     = 0x02,
};

/* TODO better algorithm for this global tracker
 *
 * Currently, access glb with external linkage:
 *
 * extern struct hypos_globl *glb;
 *
 * simple implementation allow me to track things going on in
 * hypos.
 *
 * XXX: Method above doesn't seem working properly. Now try
 * access the global structure through this public interface.
 */

struct hypos_globl *get_globl(void);
#define hypos_get(member)        ((get_globl())->member)
#define hypos_set(value, member) ((get_globl())->member = value)

#endif /* !__ASSEMBLY__ */
// --------------------------------------------------------------
#endif /* _ORG_GLOBAL_H */
