/**
 * Hustler's Project
 *
 * File:  arch_boot.S
 * Date:  2024/05/20
 * Usage: armv8 boot process
 */

/* Initializing Exceptions
 *
 * (1) Setting up the vector table
 * (2) Asynchronous exceptions routing and masking seturations
 *
 * Reset vector
 *
 * In AArch64, the processor starts execution from an IMPLEMENTAION
 * -DEFINED address, which is defined by the hardware input pins
 * RVBARADDR and can be read by the RVBAR_EL3 register. must place
 * boot code at this address.
 *
 * The vector table in AArch64 mode contains 16 entries. Each entry
 * is 128B in size and contains at most 32 instructions. Vector
 * tables must be placed at a 2KB-aligned address. The addresses
 * are specified by initializing VBAR_ELn registers.
 */

#include <arch_def.h>
#include <arch_debug.h>
#include <arch_sysreg.h>

    .section .entry,"ax",%progbits
    .globl   boot
boot:
    /* Save bootloader arguments in x0 */
    mov  x21, x0

    /* Disable all exception */
    msr daifset, 0xF

    /* Early Debug UART Initialization */
    bl  early_uart_init

    /* Hang if not in EL2 */
    TAGS("- Current EL ")
    mrs x2, CurrentEL
    print_reg x2
    TAGS(" -\r\n")
    cmp x2, #ARCH_ARM64_EL2T

    beq hyp_entry

    /* shall never reach here */
    TAGS("- hypos must be booted in EL2 -\r\n")

hang:
    wfi
    b   hang

hyp_entry:
    /* Hang if _code_start is not 4k aligned */
    ldr  x1, _code_start
page_size_align:
    movn x2, #0xFFF
    ands x3, x2, x1
    cmp  x3, x2
    blt  hang

    TAGS("- SysReg Setup -\r\n")

    /* Set up Memory Attribute Indirection Register (EL2)
     *
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

    /* Set up Hypervisor Configuration Register (EL2)
     * defines various ops trapped to EL2
     */
    ldr x0, hcr_set
    msr hcr_el2, x0

    /* Set up Vector Base Address Register (EL2)
     * (self-explanatory)
     */
    ldr x0, =hyp_vectors
    msr vbar_el2, x0

    dsb sy
    isb

    TAGS("- Vectors Setup -\r\n")

    /* Zero out bss section */
    ldr  x0, _bss_start
    ldr  x1, _bss_end
zero:
    subs x2, x1, x0
    ble  done
    str  xzr, [x0]
    add  x0, x0, 8
    b    zero
done:

    /* use SP_ELx and setup hyp stack
     */
    msr SPSel, #1
    ldr x0, _hyp_stack_end
    mov sp, x0

    TAGS("- BSP Setup -\r\n")
    /* C Entry point for later booting process
     */
    b   bsp_setup

// ------------------------------------------------------------------------
GLOBAL(mair_set)
    .dword ARCH_MAIR_EL2_SET

GLOBAL(tcr_set)
    .dword ARCH_TCR_EL2_SET

GLOBAL(vtcr_set)
    .dword ARCH_VTCR_EL2_SET

GLOBAL(hcr_set)
    .dword ARCH_HCR_EL2_SET
// ------------------------------------------------------------------------
