/**
 * Hustler's Project
 *
 * File:  arch_entry.S
 * Date:  2024/05/23
 * Usage:
 */

#include <arch_def.h>
#include <arch_debug.h>
#include <arch_sysreg.h>

// ------------------------------------------------------------------------
.macro load_paddr xb, sym
    ldr \xb, =\sym
    add \xb, \xb, x20
.endm

.macro flush_local_tlb
    dsb  nshst
    tlbi alle2
    dsb  nsh
    isb
.endm

/* Build boot page table's first-level entries
 */
page_table_setup:
    nop
END(page_table_setup)

/* Turn on the Data Cache and the MMU.
 */
mmu_enable:
    mov x4, x0
    mov x5, x1

    TAGS("- Turn on MMU -\r\n")

    flush_local_tlb

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
    nop
END(remove_idmap)

ENTRY(boot_cpu_mem_setup)
    mov x6, lr
    bl page_table_setup
    load_paddr x0, boot_pgtbl

    mov x1, #0
    bl mmu_enable

    mov lr, x6

    b  remove_idmap
END(boot_cpu_mem_setup)

// ------------------------------------------------------------------------
