/**
 * Hustler's Project
 *
 * File:  board.h
 * Date:  2024/07/03
 * Usage:
 */

#ifndef _BSP_BOARD_H
#define _BSP_BOARD_H
// --------------------------------------------------------------
#include <bsp/type.h>

enum board_option {
    RADXA_ZERO3 = 0,
    RADXA_ZERO2PRO,
    /* TODO */
};

enum cpu_vendor {
    VENDOR_ROCKCHIP = 0,
    VENDOR_AMLOGIC,
};

struct hypos_cpu {
    unsigned int  core_nr;
    unsigned int  vendor;

    char priv[64];
};
// --------------------------------------------------------------
enum hpm_type {
    TXT_BLK,
    RAM_BLK,
    RSV_BLK,
    HPM_BLK_NR
};

enum hvm_type {
    DATA_BLK = 0,
    FMAP_BLK, /* Fix Memory Map */
    VMAP_BLK, /* Virtual Memory Map */
    BTMB_BLK, /* Boot-time Memory Bank */
    DMAP_BLK, /* Direct Memory Map */
    HVM_BLK_NR
};

struct hpm_blk {
    const hpa_t start;
    const hpa_t end;
    const unsigned int type;
};

struct hvm_blk {
    const hva_t  start;
    const size_t size;
    const unsigned int type;
};

#define HPM_BLK_MAX               (4)
#define HVM_BLK_MAX               (8)

struct hypos_mem {
    struct hpm_blk hpm[HPM_BLK_MAX];
    struct hvm_blk hvm[HVM_BLK_MAX];
};

struct hpm_blk *hypos_hpm_get(unsigned int type);
struct hvm_blk *hypos_hvm_get(unsigned int type);
// --------------------------------------------------------------
struct hypos_board {
    struct hypos_mem *mem;
    struct hypos_cpu *cpu;
    char *name; /* Board Name */
};

int board_setup(void);
// --------------------------------------------------------------
#endif /* _BSP_BOARD_H */
