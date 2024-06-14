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

/* Note
 * The MMU uses the most significant bits of the Virtual Address
 * to index entries in a translation table and establish which
 * block is being accessed. The MMU translates the Virtual
 * Addresses of code and data to the Physical Addresses in the
 * actual system. The translation is carried out automatically in
 * hardware and is transparent to the application. In addition to
 * address translation, the MMU controls memory access permissions,
 * memory ordering, and cache policies for each region of memory.
 *
 * +-------------------+           +-------------------+
 * | Application (EL0) | TTBRn_EL1 |  Physical Memory  |
 * +-------------------+ ---------▶| map seen by Guest |
 * |   Guest OS (EL1)  |           |       (IPA)       |
 * +-------------------+           +-------------------+
 *                                           | VTTBR0_EL2
 *                                           ▼
 * +-------------------+ TTBR0_EL2 +-------------------+
 * |  Hypervisor (EL2) | ---------▶|   Real Physical   |
 * +-------------------+           |  Memory Map (PA)  |
 *                                 +-------------------+
 */

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

/* Note ubfx takes (PGTBL_TTBL_SHIFT) bit field from bit postion
 * [ PGTBL_LEVEL_SHIFT(lvl) ] in the [virt] and place it in
 * [slot]
 *
 * When use a 4kB granule size, the hardware can use a 4-level
 * look up process. The 48-bit address has nine address bits per
 * level translated, that is 512 entries each, with the final 12
 * bits selecting a byte within the 4kB coming directly from the
 * original address.
 *
 * +--------------+--------------+--------------+--------------+
 * | TBL0 [47:39] | TBL1 [38:30] | TBL2 [29:21] | TBL3 [20:12] | ...
 * +--------------+--------------+--------------+--------------+
 *        |                                             |
 *        ▼                                             ▼
 * Table Descriptors &                          Page Descriptors
 * Block Descriptors
 *
 * Maximum entries in table = 512
 *
 * For instance,
 * Page table level 0 index shift: (3 - 0) * (12 - 3) + 12 = 39
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
 * XXX: Basic Logic
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
    mov   \tmp2, #TTBL_ENTRY_MEM
    orr   \tmp2, \tmp2, \tbl         /* set up lower attributes */
    adr_l \tbl,  \ptbl               /* tbl => pc-relative address of ptbl */
    str   \tmp2, [\tbl, \tmp1, lsl #3]
.endm

/* Note create a page table entry in \ptbl to \tbl
 */
.macro create_ttbl_entry, ptbl, tbl, virt, lvl, tmp1, tmp2, tmp3
    load_paddr \tmp1, \tbl
    create_ttbl_entry_from_paddr \ptbl, \tmp1, \virt, \lvl, \tmp2, \tmp3
.endm

/* 0xFFFFFFFFFFFFF000 - PGTBL_LEVEL_MASK(3)
 */
.macro create_map_entry, ptbl, virt, phys, tmp1, tmp2, tmp3, type=TTBL_ENTRY_MEM
    and   \tmp3, \phys, #PGTBL_LEVEL_MASK(3) /* tmp3 = PAGE_ALIGNED(phys) */
    get_ttbl_slot \tmp1, \virt, 3
    mov   \tmp2, #\type
    orr   \tmp2, \tmp2, \tmp3
    adr_l \tmp3, \ptbl
    str   \tmp2, [\tmp3, \tmp1, lsl #3]
.endm

// --------------------------------------------------------------
    __TTBL

/* Note MMU setup for boot CPU
 */
FUNC(bootcpu_mmu_setup)
    mov  x17, lr
    bl   page_table_setup

    load_paddr x0, boot_pgtbl0
    ldr  x1, =SCTLR_EL2_SET
    bl   mmu_enable

    /* Switch to runtime mapping */
    ldr  x0, =1f
    br   x0
1:  mov  lr, x17

    b    remove_idmap
END(bootcpu_mmu_setup)

/* Note Page Table Setup
 * DB  -  Block Descriptors
 * DT  -  Table Descriptors
 *                                 Level-(2)
 *                                 :      :
 *                                 +------+
 *                                 |  DB  | - Memory Region
 *             Level-(1)           +------+
 *             :      :            :      :
 *             +------+            +------+
 *             |  DB  |            |  DT  | ----- ~ --▶
 *             +------+            +------+
 *             :      :            :      :
 *             +------+            |      |
 *             |  DT  | ----- ~ --▶+------+
 *             +------+   |
 *             :      : Index
 * +------+    |      |   |
 * | TTBR |---▶+------+ -----
 * +------+
 *
 * XXX: Indexed by bits from the input address. Each lookup level
 *      resolves additional bits.
 */
FUNC_LOCAL(page_table_setup)
    ldr  x0, =HYPOS_VIRT_START

    create_ttbl_entry boot_pgtbl0, boot_pgtbl1, x0, 0, x1, x2, x3
    create_ttbl_entry boot_pgtbl1, boot_pgtbl2, x0, 1, x1, x2, x3

    load_paddr x4, boot_pgtbl3
    ldr  x1, =HYPOS_VIRT_START

.rept DATA_NR_ENTRIES(2)
    mov  x0, x4
    create_ttbl_entry_from_paddr boot_pgtbl2, x0, x1, 2, x2, x3
    add  x1, x1, #PGTBL_LEVEL_SIZE(2)
    add  x4, x4, #PAGE_SIZE
.endr

#if 1
    DBG("- HYPOS [0x")
    ldr  x5, =__hypos_start
    dump_reg x5
    DBG(" ~ 0x")
    ldr  x6, =__hypos_end
    dump_reg x6
    DBG("] -\r\n")
#endif

    /* Map hypos code */
    ldr  x0, =__hypos_start
    ldr  x1, =__hypos_end
    sub  x0, x1, x0

    /* Number of pages * PTE size, PTE size = 8 bytes (64 bits)
     */
    lsr  x0, x0, #PAGE_SHIFT
    lsl  x0, x0, #3

    /* x19                    - paddr start
     * PGTBL_LEVEL_SHIFT(3)   - 12
     */
    adr_l x4, boot_pgtbl3
    lsr  x2, x19, #PGTBL_LEVEL_SHIFT(3)
    lsl  x2, x2,  #PGTBL_LEVEL_SHIFT(3)
    mov  x3, #TTBL_ENTRY_MEM
    orr  x2, x2, x3

    mov  x1, xzr
1:  str  x2, [x4, x1]
    add  x2, x2, #PAGE_SIZE /* next page */
    add  x1, x1, #8         /* next slot */
    cmp  x1, x0
    blt  1b

    /* Note Identity Mapping
     *
     * Identity mapping is needed when hypos is loaded exactly at
     * HYPOS_VIRT_START.
     *
     *     :                    XXX: while enabling MMU, things
     *     |inst                might get weird due to CPU
     *     ▼                    intruction pipeline, etc. In
     * --------- enabling MMU   this case, ID mapping is needed
     *     |inst                to ensure correct memory accesses.
     *     :
     *     ▼
     *
     * XXX: Out-of-order Execution
     * Many processors support out-of-order execution of non-
     * dependent instructions. Whenever an instruction is stalled
     * while it waits for the result of a preceding instruction,
     * the processor can execute subsequent instructions that do
     * not have a dependency.
     * ----------------------------------------------------------
     * VA                        IDMAP
     *
     * ----------------------------------------------------------
     */
    ldr  x0, =HYPOS_VIRT_START
    cmp  x19, x0
    bne  1f

    DBG("- Set Boot Page Table without ID Mapping -\r\n")

    ret

1:  get_ttbl_slot x0, x19, 0
    cmp  x0, #PGTBL0_OFFSET(HYPOS_VIRT_START)
    beq  1f
    create_ttbl_entry boot_pgtbl0, boot_idmap1, x19, 0, x0, x1, x2
    b    from_boot_idmap1

1:  get_ttbl_slot x0, x19, 1
    cmp  x0, #PGTBL1_OFFSET(HYPOS_VIRT_START)
    beq  1f
    create_ttbl_entry boot_pgtbl1, boot_idmap2, x19, 1, x0, x1, x2
    b    from_boot_idmap2

1:  get_ttbl_slot x0, x19, 2
    cmp  x0, #PGTBL2_OFFSET(HYPOS_VIRT_START)
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
    create_map_entry hypos_fixmap, x0, x23, x1, x2, x3, type=TTBL_ENTRY_DEV
    ldr  x0, =FIXMAP_ADDR(0)
    create_ttbl_entry boot_pgtbl2, hypos_fixmap, x0, 2, x1, x2, x3

    DBG("- Set Boot Page Table with ID Mapping -\r\n")

    ret

pgtbl_fatal:

    DBG("- Set Boot Page Table failed -\r\n")

    b    boot_failed
END(page_table_setup)

/* Note Turn on the D-cache, I-cache and the MMU.
 */
FUNC_LOCAL(mmu_enable)
    mov  x4, x0
    mov  x5, x1
    DBG("- Turn on MMU -\r\n")

    flush_local_tlb

    /* Set up Translation Table Base Register 0 (EL2)
     */
    msr  TTBR0_EL2, x4
    isb

    mrs  x0, SCTLR_EL2
    orr  x0, x0, #SCTLR_Axx_ELx_M  /* Enable MMU */
    orr  x0, x0, #SCTLR_Axx_ELx_C  /* Enable D-cache */
    orr  x0, x0, #SCTLR_Axx_ELx_I  /* Enable I-cache */
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
 * x19               =  paddr(_head)
 */
FUNC_LOCAL(remove_idmap)
    DBG("- Remove ID Mapping -\r\n")

    get_ttbl_slot x1, x19, 0
    cmp   x1, #PGTBL0_OFFSET(HYPOS_VIRT_START) /* 0x14 */
    beq   1f

    ldr   x0, =boot_pgtbl0
    str   xzr, [x0, x1, lsl #3]
    b     idmap_removed

1:  get_ttbl_slot x1, x19, 1
    cmp   x1, #PGTBL1_OFFSET(HYPOS_VIRT_START)
    beq   1f

    ldr   x0, =boot_pgtbl1
    str   xzr, [x0, x1, lsl #3]
    b     idmap_removed

1:  get_ttbl_slot x1, x19, 2
    cmp   x1, #PGTBL2_OFFSET(HYPOS_VIRT_START)
    beq   idmap_removed

    ldr   x0, =boot_pgtbl2
    str   xzr, [x0, x1, lsl #3]

idmap_removed:
    flush_local_tlb

    DBG("- ID Mapping Removed -\r\n")

    ret
END(remove_idmap)

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

/* Note When stage 1 MMU disabled:
 *
 * (a) all data access are Device-nGnRnE.
 * (b) all instruction fetches are treated as cacheable.
 * (c) all address have read/write access and executable.
 */

// --------------------------------------------------------------
