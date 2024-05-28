/**
 * Hustler's Project
 *
 * File:  arch_mmu.S
 * Date:  2024/05/23
 * Usage:
 */

#include <arch_def.h>
#include <arch_debug.h>
#include <arch_sysreg.h>
#include <arch_ttbl.h>

// ------------------------------------------------------------------------
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

/*  ubfx takes (PGTBL_TTBL_SHIFT) bit field from bit postion
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
 */
.macro get_ttbl_slot, slot, virt, lvl
    ubfx  \slot, \virt, #PGTBL_LEVEL_SHIFT(\lvl), #PGTBL_TTBL_SHIFT
.endm

.macro create_ttbl_entry_from_paddr, ptbl, tbl, virt, lvl, tmp1, tmp2
    get_ttbl_slot \tmp1, \virt, \lvl
    mov   \tmp2, #TTBL_ENTRY_DEF
    orr   \tmp2, \tmp2, \tbl
    adr_l \tbl, \ptbl
    str   \tmp2, [\tbl, \tmp1, lsl #3]
.endm

.macro create_ttbl_entry, ptbl, tbl, virt, lvl, tmp1, tmp2, tmp3
    load_paddr \tmp1, \tbl
    create_ttbl_entry_from_paddr \ptbl, \tmp1, \virt, \lvl, \tmp2, \tmp3
.endm

.macro create_map_entry, ptbl, virt, phys, tmp1, tmp2, tmp3, type=TTBL_ENTRY_DEF
    and   \tmp3, \phys, #PGTBL3_MASK
    get_ttbl_slot \tmp1, \virt, 3
    mov   \tmp2, #\type
    orr   \tmp2, \tmp2, \tmp3
    adr_l \tmp3, \ptbl
    str   \tmp2, [\tmp3, \tmp1, lsl #3]
.endm

// ------------------------------------------------------------------------
    .section .text.idmap, "ax", %progbits

page_table_setup:
    TAGS("- Set PageTable -\r\n")

    ldr x0, =HYPERVISOR_VIRT_START
    create_ttbl_entry pgtbl0, pgtbl1, x0, 0, x1, x2, x3
    create_ttbl_entry pgtbl1, pgtbl2, x0, 1, x1, x2, x3

    load_paddr x4, pgtbl3
    ldr x1, =HYPERVISOR_VIRT_START

.rept DATA_PGTBL2_NR_ENTRIES /* DATA_NR_ENTRIES(2) */
    mov x0, x4
    create_ttbl_entry_from_paddr pgtbl2, x0, x1, 2, x2, x3
    add x1, x1, #PGTBL2_SIZE
    add x4, x4, #PAGE_SIZE
.endr

    ldr x0, =_hypos_virt_start
    ldr x1, =_hypos_virt_end
    sub x0, x1, x0
    lsr x0, x0, #PAGE_SHIFT
    lsl x0, x0, #3

    adr_l x4, pgtbl3
    lsr x2, x19, #PGTBL3_SHIFT
    lsl x2, x2, #PGTBL3_SHIFT
    mov x3, #TTBL_ENTRY_DEF
    orr x2, x2, x3

    mov x1, xzr
1:
    str x2, [x4, x1]
    add x2, x2, #PAGE_SIZE
    add x1, x1, #8
    cmp x1, x0
    blt 1b

    ldr x0, =HYPERVISOR_VIRT_START
    cmp x19, x0
    bne 1f
    ret
1:
    get_ttbl_slot x0, x19, 0
    cmp x0, #PGTBL0_SLOT
    beq 1f
    create_ttbl_entry pgtbl0, idmap1, x19, 0, x0, x1, x2
    b   link_from_idmap1
1:
    get_ttbl_slot x0, x19, 1
    cmp x0, #PGTBL1_SLOT
    beq 1f
    create_ttbl_entry pgtbl1, idmap2, x19, 1, x0, x1, x2
    b   link_from_idmap2
1:
    get_ttbl_slot x0, x19, 2
    cmp x0, #PGTBL2_SLOT
    beq pgtbl_fatal
    create_ttbl_entry pgtbl2, idmap3, x19, 2, x0, x1, x2
    b   link_from_idmap3

link_from_idmap1:
    create_ttbl_entry idmap1, idmap2, x19, 1, x0, x1, x2
link_from_idmap2:
    create_ttbl_entry idmap2, idmap3, x19, 2, x0, x1, x2
link_from_idmap3:
    create_map_entry  idmap3, x19, x19, x0, x1, x2

    ldr x0, =ARCH_EARLY_UART_VA
    create_map_entry  fixmap, x0, x23, x1, x2, x3, type=TTBL_ENTRY_DEM

    ldr x0, =FIXMAP_ADDR(0)
    create_ttbl_entry pgtbl2, fixmap, x0, 2, x1, x2, x3
    ret

pgtbl_fatal:
    TAGS("- Unable to build boot page tables -\r\n")
    b   fail
END(page_table_setup)

/* Turn on the Data Cache and the MMU.
 */
mmu_enable:
    mov x4, x0
    mov x5, x1

    TAGS("- Turn on MMU -\r\n")

    flush_local_tlb

    /* Set up Memory Attribute Indirection Register (EL2)
     */
    ldr x0, mair_set
    msr mair_el2, x0

    /* Set up Translation Control Register (EL2)
     * controls stage 1 of EL2 & EL0 translation regime
     */
    ldr x0, tcr_set
    msr tcr_el2, x0

    /* Set up Virtualization Translation Control Register (EL2)
     * controls stage 2 of EL1 & EL0 translation regime
     */
    ldr x0, vtcr_set
    msr vtcr_el2, x0

    /* Set up Translation Table Base Register 0 (EL2)
     */
    msr ttbr0_el2, x4
    isb

    mrs x0, sctlr_el2
    orr x0, x0, #ARCH_SCTLR_MMU_EN
    orr x0, x0, #ARCH_SCTLR_DCACHE_EN
    orr x0, x0, x5
    dsb sy
    msr sctlr_el2, x0
    isb

    ldr x23, =ARCH_EARLY_UART_VA
    TAGS("- MMU turned on -\r\n")

    ret
END(mmu_enable)

/* Remove the 1:1 map from the page tables
 */
remove_idmap:
    TAGS("- Remove idmap -\r\n")

    get_ttbl_slot x1, x19, 0
    cmp x1, #PGTBL0_SLOT
    beq 1f

    ldr x0, =pgtbl0
    str xzr, [x0, x1, lsl #3]
    b   idmap_removed
1:
    get_ttbl_slot x1, x19, 1
    cmp x1, #PGTBL1_SLOT
    beq 1f

    ldr x0, =pgtbl1
    str xzr, [x0, x1, lsl #3]
    b   idmap_removed
1:
    get_ttbl_slot x1, x19, 2
    cmp x1, #PGTBL2_SLOT
    beq idmap_removed

    ldr x0, =pgtbl2
    str xzr, [x0, x1, lsl #3]

idmap_removed:
    flush_local_tlb

    ret
END(remove_idmap)

ENTRY(arch_bootcpu_mmu_setup)
    mov x6, lr

    bl  page_table_setup
    load_paddr x0, pgtbl0

    mov x1, #0  /* No extra SCTLR configuration */
    bl  mmu_enable

    mov lr, x6

    bl  remove_idmap

    ret
END(arch_bootcpu_mmu_setup)

ENTRY(switch_ttbr_id)
    dsb ish
    isb

    mrs x1, sctlr_el2
    bic x1, x1, #ARCH_SCTLR_MMU_EN
    msr sctlr_el2, x1
    isb

    flush_local_tlb

    msr ttbr0_el2, x0
    isb

    ic  iallu
    isb

    mrs x1, sctlr_el2
    orr x1, x1, #ARCH_SCTLR_MMU_EN
    msr sctlr_el2, x1
    isb

    ret
END(switch_ttbr_id)

// ------------------------------------------------------------------------
fail:
    TAGS("- Boot failed -\r\n")
1:
    wfe
    b     1b
// ------------------------------------------------------------------------
GLOBAL(mair_set)
    .dword ARCH_MAIR_EL2_SET

GLOBAL(tcr_set)
    .dword ARCH_TCR_EL2_SET

GLOBAL(vtcr_set)
    .dword ARCH_VTCR_EL2_SET
// ------------------------------------------------------------------------

/* When stage 1 MMU disabled:
 *
 * (a) all data access are Device-nGnRnE.
 * (b) all instruction fetches are treated as cacheable.
 * (c) all address have read/write access and executable.
 */

// ------------------------------------------------------------------------
