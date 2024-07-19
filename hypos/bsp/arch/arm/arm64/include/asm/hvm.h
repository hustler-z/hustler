/**
 * Hustler's Project
 *
 * File:  hvm.h
 * Date:  2024/07/18
 * Usage: Host Virtual Memory Layout Settings
 *        XXX: this header file can be used by both C and
 *             assembly executing environment.
 */

#ifndef _ASM_HVM_H
#define _ASM_HVM_H
// --------------------------------------------------------------
#include <asm/ttbl.h>
#include <bsp/size.h>

/* Memory slot of 8G */
#define MEM_SLOT0(slot)             (_AT(hva_t, slot) << 39)

/* - Aarch64 Memory Layout -
 *
 * For embedded device of maximum 32G of RAM
 * [0000000800000000 - 0000001000000000)
 * --------------------------------------------------------------
 * 8M                  DATA
 *                     FIXMAP
 *                     VMAP
 *                     PAGEFRAME
 *                     DIRECTMAP
 * --------------------------------------------------------------
 */
#define HYPOS_DATA_VIRT_START      MEM_SLOT0(1)
#define HYPOS_DATA_VIRT_SIZE       MB(8)
#define HYPOS_DATA_NR_ENTRIES(lvl) \
    (HYPOS_DATA_VIRT_SIZE / PGTBL_LEVEL_SIZE(lvl))
// --------------------------------------------------------------

/* HYPOS FIXMAP
 * ------------------------- CONSOLE      (0)
 *  |
 * ------------------------- PERIPHERAL   (1)
 *  |
 * ------------------------- PFNMAP       (2)
 *  |
 *  ▼
 */
#define HYPOS_FIXMAP_VIRT_START    PAGE_ALIGN(HYPOS_DATA_VIRT_START \
                                            + HYPOS_DATA_VIRT_SIZE)
#define HYPOS_FIXMAP_VIRT_SIZE     MB(2)
// --------------------------------------------------------------
#define HYPOS_FIXMAP_ADDR(n)       (HYPOS_FIXMAP_VIRT_START + (n) * PAGE_SIZE)
#define NUM_FIX_PFNMAP             8
#define FIX_CONSOLE                0
#define FIX_PERIPHERAL             1
#define FIX_PFNMAP_START           2
#define FIX_PFNMAP_END             (FIX_PFNMAP_START + NUM_FIX_PFNMAP - 1)
#define FIXADDR_START              HYPOS_FIXMAP_ADDR(0)
#define FIXADDR_END                HYPOS_FIXMAP_ADDR(FIX_PFNMAP_END)
#ifndef __ASSEMBLY__
extern unsigned long directmap_va_start;
#define HYPOS_HEAP_VIRT_START      directmap_va_start
#endif
#define HYPOS_VIRT_START           HYPOS_DATA_VIRT_START
#define SYMBOLS_ORIGIN             HYPOS_VIRT_START

/* XXX: Be aware that the regions below should set base on specific
 *      board settings.
 */
#define HVM_VMAP_START             (FIXADDR_END + PAGE_SIZE)
#define HVM_VMAP_SIZE              MB(256)
#define HVM_BTMB_START             (FIXADDR_END + (PAGE_SIZE * 2) + MB(256))
#define HVM_BTMB_SIZE              MB(128)
#define HVM_DMAP_START             (FIXADDR_END + (PAGE_SIZE * 3) + MB(256 + 128))
#define HVM_DMAP_SIZE              MB(512)

// --------------------------------------------------------------
#endif /* _ASM_HVM_H */
