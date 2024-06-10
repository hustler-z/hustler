/**
 * Hustler's Project
 *
 * File:  start.S
 * Date:  2024/05/20
 * Usage: armv8 boot process
 */

// --------------------------------------------------------------

/* Note that registers below:
 * --------------------------------------------------------------
 * x19        - paddr start
 * x20        - phys offset
 * x21        - bootloader argument
 * x23        - debug uart address
 * --------------------------------------------------------------
 */

#include <asm/define.h>
#include <asm/debug.h>
#include <asm/sysregs.h>

// --------------------------------------------------------------

    .section .boot.entry, "ax", %progbits

/* Note kickoff - hypos Code Entry Point
 */
FUNC(kickoff)
    /* Disable all exception */
    msr  DAIFSet, #0xF

    /* (a) save bootloader x0 (boot_parm)
     * (c) compute paddr offset and save it
     */
    mov  x21, x0

    ldr  x0,  =kickoff
    adr  x19, kickoff
    sub  x20, x19, x0

    /* Early Debug UART Initialization */
    bl   early_uart_init

    /* Hang if not on boot cpu (i.e., cpu0) */
    mrs  x1, MPIDR_EL1
    ubfx x2, x1, #32, #MPIDR_LEVEL_BITS
    bfi  w1, w2, #24, #MPIDR_LEVEL_BITS
    cbz  w1, bootcpu
    b    boot_failed

bootcpu:
    DBG("- (CPU) 0x")
    print_reg x1
    DBG(" -\r\n")

    b    el2_entry
END(kickoff)

// --------------------------------------------------------------

/* Note Boot CPU Initialization
 * --------------------------------------------------------------
 * (a) Set up page table
 * (b) Enable MMU
 * (c) Set boot CPU stack
 * (d) Enter C
 * --------------------------------------------------------------
 */
FUNC_LOCAL(bootcpu_entry)
    /* Ensure we use SP_EL2 to handle any
     * exceptions occured at EL2.
     */
    msr  SPSel, #1

    ldr  lr, =bootcpu_switched

    /* MMU early setup */
    b    bootcpu_mmu_setup

bootcpu_switched:
    /* Enable all exception */
    msr  DAIFClr, #0xF

    /* Zero out bss section */
    bl   zero_bss

    /* setup hyp stack and ready for C
     */
    ldr  x3, =__hyp_stack_end
    mov  sp, x3

    DBG("- BSP Setup -\r\n")

    mov  x0, x20 /* phys_offset */
    mov  x1, x21
    b    bsp_setup
END(bootcpu_entry)

/* TODO
 * SMP boot entry: after bootcpu's up, time to kick other cpus up
 */
FUNC(bootsmp_entry)
    nop
END(bootsmp_entry)

/* Note System Registers at EL2
 * --------------------------------------------------------------
 * HCR_EL2
 * MAIR_EL2
 * TCR_EL2
 * VBAR_EL2
 * VTCR_EL2
 * --------------------------------------------------------------
 */
FUNC_LOCAL(el2_entry)
    DBG("- (EL)  0x")
    mrs  x7, CurrentEL
    print_reg x7
    DBG(" -\r\n")

    /* Hang if not in EL2 */
    cmp  x7, #ARM64_EL2T
    beq  el2_setup

    /* shall never reach here */
    DBG("- hypos must be booted in EL2 -\r\n")
    b    boot_failed

el2_setup:
    /* Set up Hypervisor Configuration Register (EL2)
     * defines various ops trapped to EL2
     */
    DBG("- Set HCR_EL2 -\r\n")
    ldr  x0, =HCR_EL2_SET
    msr  HCR_EL2, x0

    /* Set up Memory Attribute Indirection Register (EL2)
     */
    DBG("- Set MAIR_EL2 -\r\n")
    ldr  x0, =MAIR_EL2_SET
    msr  MAIR_EL2, x0

    /* Set up Translation Control Register (EL2)
     * controls stage 1 of EL2 & EL0 translation regime
     */
    DBG("- Set TCR_EL2 -\r\n")
    ldr  x0, =TCR_EL2_SET
    mrs  x1, ID_AA64MMFR0_EL1
    ubfm x1, x1, #0, #3
    mov  x2, #5
    cmp  x1, x2
    csel x1, x1, x2, lt
    bfi  x0, x1, #16, #3
    msr  TCR_EL2, x0

    /* Set up Virtualization Translation Control Register (EL2)
     * controls stage 2 of EL1 & EL0 translation regime
     */
    DBG("- Set VTCR_EL2 -\r\n")
    ldr  x0, =VTCR_EL2_SET
    msr  VTCR_EL2, x0

    /* Set up Vector Base Address Register (EL2)
     * (self-explanatory)
     */
    DBG("- Set VBAR_EL2 -\r\n")
    ldr  x0, =hyp_vectors
    msr  VBAR_EL2, x0

    dsb  sy
    isb

    b    bootcpu_entry
END(el2_entry)

/* Note BSS - Better Save Space
 * zero-initialized global and static variables are stored,
 * BSS needs to be zeroed out before C sequence.
 */
FUNC_LOCAL(zero_bss)
    DBG("- BSS Section -\r\n")

    ldr  x0, =__bss_start
    ldr  x1, =__bss_end

1:  str  xzr, [x0], #8
    cmp  x0, x1
    blt  1b

    DBG("- Zero out BSS -\r\n")

    ret
END(zero_bss)

/* Note hypos boot failed
 */
FUNC(boot_failed)
    DBG("- Boot failed -\r\n")

1:  wfi
    b    1b
END(boot_failed)
// --------------------------------------------------------------

/* Note with this, [31:0] => AFF3, AFF2, AFF1, AFF0
 */
FUNC(get_cpu_affinity)
    mrs  x0, MPIDR_EL1
    ubfx x1, x0, #32, #MPIDR_LEVEL_BITS
    bfi  w0, w1, #24, #MPIDR_LEVEL_BITS
    ret
END(get_cpu_affinity)

// --------------------------------------------------------------
