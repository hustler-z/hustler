/**
 * Hustler's Project
 *
 * File:  mmu.S
 * Date:  2024/05/23
 * Usage:
 */

#include <asm/define.h>
#include <asm/debug.h>
#include <asm/sysregs.h>
#include <asm/ttbl.h>
// --------------------------------------------------------------


/* +-------------+
 * | Reserved    |               TTBR0_EL2
 * +-------------+            +-------------+
 * |             |   FIXMAP   |             |
 * | Peripherals |----------->| Translation |
 * |             |            |    Table    |
 * +-------------+            |             |
 * |             |            +-------------+
 * |             |
 * |             |
 * : Reserved    :
 * |             |
 * |             |
 * |             |
 * +-------------+
 * |             |
 * | ROM         |
 * |             |                   STAGE 2
 * +-------------+                +-----------> VA
 * |             |               /     PA
 * |             |   STAGE 1    /       \
 * | RAM         |-----------> +  @VTTBR_EL2/VSTTBR_EL2
 * |             |     IPA
 * |             |       \
 * +-------------+   @TTBRx_EL1
 * | Reserved    |
 * +-------------+
 */


// --------------------------------------------------------------
.macro flush_local_tlb
    dsb  nshst
    tlbi alle2
    dsb  nsh
    isb
.endm

.macro load_paddr xb, sym
    ldr \xb, =\sym
    add \xb, \xb, x20 /* paddr offset */
.endm

/*  Note ubfx takes (PGTBL_TTBL_SHIFT) bit field from bit postion
 *  [ PGTBL_LEVEL_SHIFT(lvl) ] in the [virt] and place it in
 *  [slot]
 *
 *  +--------------+--------------+--------------+--------------+
 *  | TBL0 [47:39] | TBL1 [38:30] | TBL2 [29:21] | TBL3 [20:12] | ...
 *  +--------------+--------------+--------------+--------------+
 *
 *  For instance,
 *  Page table level 0 index shift: (3 - 0) * (12 - 3) + 12 = 39
 *
 *      47           39
 *       +--------------+ ... (virt)
 *        \              \
 *     ... +--------------+   (slot)
 *          8             0
 *
 * slot   - 9-bit translation table index
 * virt   - virtual address
 * lvl    - page table level (0 ~ 3)
 */
.macro get_ttbl_slot, slot, virt, lvl
    ubfx  \slot, \virt, #PGTBL_LEVEL_SHIFT(\lvl), #PGTBL_TTBL_SHIFT
.endm

/* Note
 *
 * +----------------+----------------------------------+-----------+
 * |   Upper Attr   |        Block Descriptor          | Lower Attr|
 * +----------------+----------------------------------+-----------+
 *  63            48 47                              12 11         0
 *
 * Basic Logic
 * -----------------------------------------------------------------
 *
 *  PGTBL 0
 * +-------+
 * |       |      PGTBL 1
 * |       |     +-------+
 * |       | ◀-- |       |
 * :       :     |       |
 * |       |     |       | ◀-- .. Place next-level page table base
 * +-------+     :       :        address in current-level page
 *               |       |        table based on table index in
 *               +-------+        virtual address.
 *
 */
.macro create_ttbl_entry_from_paddr, ptbl, tbl, virt, lvl, tmp1, tmp2
    get_ttbl_slot \tmp1, \virt, \lvl /* get table index */
    mov   \tmp2, #TTBL_ENTRY_DEF
    orr   \tmp2, \tmp2, \tbl /* set up lower attributes */
    adr_l \tbl,  \ptbl /* tbl => pc-relative address of ptbl */
    str   \tmp2, [\tbl, \tmp1, lsl #3]
.endm

.macro create_ttbl_entry, ptbl, tbl, virt, lvl, tmp1, tmp2, tmp3
    load_paddr \tmp1, \tbl
    create_ttbl_entry_from_paddr \ptbl, \tmp1, \virt, \lvl, \tmp2, \tmp3
.endm

/* 0xFFFFFFFFFFFFF000 - PGTBL3_MASK
 */
.macro create_map_entry, ptbl, virt, phys, tmp1, tmp2, tmp3, type=TTBL_ENTRY_DEF
    and   \tmp3, \phys, #PGTBL3_MASK /* tmp3 = PAGE_ALIGNED(phys) */
    get_ttbl_slot \tmp1, \virt, 3
    mov   \tmp2, #\type
    orr   \tmp2, \tmp2, \tmp3
    adr_l \tmp3, \ptbl
    str   \tmp2, [\tmp3, \tmp1, lsl #3]
.endm

// --------------------------------------------------------------
    .section .boot.idmap, "ax", %progbits

/* Note MMU setup for boot CPU
 */
FUNC(bootcpu_mmu_setup)
    mov  x6, lr
    bl   page_table_setup

    load_paddr x0, boot_pgtbl0
    mov  x1, #0
    bl   mmu_enable

    /* Switch to runtime mapping */
    ldr  x0, =1f
    br   x0
1:  mov  lr, x6

    b    remove_idmap
END(bootcpu_mmu_setup)
// --------------------------------------------------------------

/* Note Page Table Setup
 *                               L3
 * L0                            +---+
 * +---+     L1                  |   |
 * |   | --▶ +---+     L2        :   : x 4
 * :   :     |   | --▶ +---+     |   |
 * |   |     :   :     |   | --▶ +---+
 * +---+     |   |     :   :
 *           +---+     |   |
 *                     +---+ --▶ +---+
 *                               |   |
 *                               :   : fixmap
 *                               |   |
 *                               +---+
 */

FUNC_LOCAL(page_table_setup)
    ldr  x0, =HYPERVISOR_VIRT_START

    /* boot_pgtbl0 -> boot_pgtbl1 */
    create_ttbl_entry boot_pgtbl0, boot_pgtbl1, x0, 0, x1, x2, x3

    /* boot_pgtbl1 -> boot_pgtbl2 */
    create_ttbl_entry boot_pgtbl1, boot_pgtbl2, x0, 1, x1, x2, x3

    load_paddr x4, boot_pgtbl3
    ldr  x1, =HYPERVISOR_VIRT_START

.rept DATA_NR_ENTRIES(2) /* DATA, BSS, etc., sections */
    mov  x0, x4

    /* boot_pgtbl2 -> boot_pgtbl3 */
    create_ttbl_entry_from_paddr boot_pgtbl2, x0, x1, 2, x2, x3
    add  x1, x1, #PGTBL2_SIZE
    add  x4, x4, #PAGE_SIZE
.endr

    /* Map hypos */
    ldr  x0, =__hypos_start
    ldr  x1, =__hypos_end
    sub  x0, x1, x0

    /* Number of pages * PTE size
     * PTE size = 8 bytes (64 bits)
     */
    lsr  x0, x0, #PAGE_SHIFT
    lsl  x0, x0, #3

    /* x19            - paddr start
     * PGTBL3_SHIFT   - 12
     */
    adr_l x4, boot_pgtbl3
    lsr  x2, x19, #PGTBL3_SHIFT
    lsl  x2, x2,  #PGTBL3_SHIFT
    mov  x3, #TTBL_ENTRY_DEF
    orr  x2, x2, x3

    mov  x1, xzr

1:  str  x2, [x4, x1]
    add  x2, x2, #PAGE_SIZE /* next page */
    add  x1, x1, #8         /* next slot */
    cmp  x1, x0
    blt  1b

    /* Note
     * Identity mapping is needed when hypos is loaded exactly at
     * HYPERVISOR_VIRT_START.
     * ----------------------------------------------------------
     * VA                        IDMAP
     *
     * ----------------------------------------------------------
     */
    ldr  x0, =HYPERVISOR_VIRT_START
    cmp  x19, x0
    bne  1f
    ret

1:  get_ttbl_slot x0, x19, 0
    cmp  x0, #PGTBL0_SLOT
    beq  1f
    create_ttbl_entry boot_pgtbl0, boot_idmap1, x19, 0, x0, x1, x2
    b    from_boot_idmap1

1:  get_ttbl_slot x0, x19, 1
    cmp  x0, #PGTBL1_SLOT
    beq  1f
    create_ttbl_entry boot_pgtbl1, boot_idmap2, x19, 1, x0, x1, x2
    b    from_boot_idmap2

1:  get_ttbl_slot x0, x19, 2
    cmp  x0, #PGTBL2_SLOT
    beq  pgtbl_fatal
    create_ttbl_entry boot_pgtbl2, boot_idmap3, x19, 2, x0, x1, x2
    b    from_boot_idmap3

from_boot_idmap1:
    create_ttbl_entry boot_idmap1, boot_idmap2, x19, 1, x0, x1, x2
from_boot_idmap2:
    create_ttbl_entry boot_idmap2, boot_idmap3, x19, 2, x0, x1, x2
from_boot_idmap3:
    create_map_entry  boot_idmap3, x19, x19, x0, x1, x2

    /* For debug purpose, periperal fixmap is needed.
     * we set debug uart vaddr fixmap.
     */
    ldr  x0, =DEBUG_UART_VA
    create_map_entry hypos_fixmap, x0, x23, x1, x2, x3, type=TTBL_ENTRY_DEM

    ldr  x0, =FIXMAP_ADDR(0)

    /* pgtbl2 -> fixmap */
    create_ttbl_entry boot_pgtbl2, hypos_fixmap, x0, 2, x1, x2, x3

    DBG("- Set boot PGTBL -\r\n")

    ret

pgtbl_fatal:

    DBG("- Set boot PGTBL failed -\r\n")

    b    boot_failed
END(page_table_setup)

/* Note Turn on the dcache and the MMU.
 */
FUNC_LOCAL(mmu_enable)
    mov  x4, x0
    mov  x5, x1

    flush_local_tlb

    /* Set up Translation Table Base Register 0 (EL2)
     */
    msr  TTBR0_EL2, x4
    isb

    mrs  x0, SCTLR_EL2
    orr  x0, x0, #SCTLR_Axx_ELx_M /* Enable MMU */
    orr  x0, x0, #SCTLR_Axx_ELx_C /* Enable D-cache */
    orr  x0, x0, #SCTLR_Axx_ELx_I /* Enable I-cache */
    orr  x0, x0, x5
    dsb  sy
    msr  SCTLR_EL2, x0
    isb

    ldr  x23, =DEBUG_UART_VA

    DBG("- MMU Enabled -\r\n")

    ret
END(mmu_enable)

/* Note
 * Identity Mapping  -  1:1 Mapping
 * x19 = paddr(kickoff)
 */
FUNC_LOCAL(remove_idmap)
    DBG("- Remove Idmap -\r\n")

    get_ttbl_slot x1, x19, 0
    cmp   x1, #PGTBL0_SLOT /* PGTBL0_SLOT: 0x14 */
    beq   1f

    ldr   x0, =boot_pgtbl0
    DBG("- Remove from Boot pgtbl 0 -\r\n")
    str   xzr, [x0, x1, lsl #3]
    DBG("- Done -\r\n")
    b     idmap_removed

1:  get_ttbl_slot x1, x19, 1
    cmp   x1, #PGTBL1_SLOT
    beq   1f

    ldr   x0, =boot_pgtbl1
    str   xzr, [x0, x1, lsl #3]
    b     idmap_removed

1:  get_ttbl_slot x1, x19, 2
    cmp   x1, #PGTBL2_SLOT
    beq   idmap_removed

    ldr   x0, =boot_pgtbl2
    str   xzr, [x0, x1, lsl #3]

idmap_removed:
    flush_local_tlb

    DBG("- Idmap Removed -\r\n")

    ret
END(remove_idmap)

// --------------------------------------------------------------
FUNC(switch_ttbr_id)
    dsb  ish
    isb

    mrs  x1, SCTLR_EL2
    bic  x1, x1, #SCTLR_Axx_ELx_M
    msr  SCTLR_EL2, x1
    isb

    flush_local_tlb

    msr  TTBR0_EL2, x0
    isb

    ic   iallu
    isb

    mrs  x1, SCTLR_EL2
    orr  x1, x1, #SCTLR_Axx_ELx_M
    msr  SCTLR_EL2, x1
    isb

    ret
END(switch_ttbr_id)
// --------------------------------------------------------------

/* Note When stage 1 MMU disabled:
 *
 * (a) all data access are Device-nGnRnE.
 * (b) all instruction fetches are treated as cacheable.
 * (c) all address have read/write access and executable.
 */

// --------------------------------------------------------------
