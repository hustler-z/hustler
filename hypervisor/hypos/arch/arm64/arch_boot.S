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

.macro boot_args_store, xn, sym, tmp
    adr \tmp, \sym
    str \xn, [\tmp]
.endm

    .section .entry,"ax",%progbits

GLOBAL(boot)
    /* Hang if not on boot cpu (i.e., cpu0) */
    mrs x1, mpidr_el1
    and x1, x1, #0xFF
    cbz x1, save_boot_args
    b   hang

save_boot_args:
    /* (a) save bootloader x0 (boot_parm)
     * (c) compute paddr offset and save it
     */
    boot_args_store x0, boot_parm, x1

    ldr x0, =boot
    adr x19, boot
    sub x20, x19, x0

boot_cpu:
    /* Disable all exception */
    msr daifset, #0xF

    /* Early Debug UART Initialization */
    bl  early_uart_init

    /* Zero out bss section */
    bl  zero_bss

    b   el2_entry

bsp_entry:
    /* MMU early setup */
    bl  arch_bootcpu_mmu_setup

    msr daifclr, #0xF

    /* use SP_ELx and setup hyp stack
     */
    msr spsel, #1
    ldr x0, _hyp_stack_end
    mov sp, x0

    TAGS("- BSP Setup -\r\n")
    /* C Entry point for later booting process
     */
    b   bsp_setup

// ------------------------------------------------------------------------

ENTRY(el2_entry)
    /* Hang if not in EL2 */
    TAGS("- Current EL ")
    mrs  x7, CurrentEL
    print_reg x7
    TAGS(" -\r\n")
    cmp  x7, #ARCH_ARM64_EL2T
    b.eq el2_setup

    /* shall never reach here */
    TAGS("- hypos must be booted in EL2 -\r\n")

    b   hang

el2_setup:
    TAGS("- Set HCR -\r\n")

    /* Set up Hypervisor Configuration Register (EL2)
     * defines various ops trapped to EL2
     */
    ldr x0, hcr_set
    msr hcr_el2, x0

    TAGS("- Set VBAR -\r\n")
    /* Set up Vector Base Address Register (EL2)
     * (self-explanatory)
     */
    ldr x0, =hyp_vectors
    msr vbar_el2, x0

    dsb sy
    isb

    b   bsp_entry
END(el2_entry)

ENTRY(zero_bss)
    TAGS("- Zero out BSS -\r\n")

    ldr  x0, _bss_start
    ldr  x1, _bss_end
zero:
    subs x2, x1, x0
    b.le done
    str  xzr, [x0]
    add  x0, x0, 8
    b    zero
done:
    ret
END(zero_bss)

hang:
    TAGS("- CPU Hang -\r\n")
1:
    wfi
    b   1b
// ------------------------------------------------------------------------
#if 0
/* Under Development
 */
ENTRY(__context_switch)
    add     x8, x0, #vcpu_saved_context
    mov     x9, sp
    stp     x19, x20, [x8], #16
    stp     x21, x22, [x8], #16
    stp     x23, x24, [x8], #16
    stp     x25, x26, [x8], #16
    stp     x27, x28, [x8], #16
    stp     x29, x9, [x8], #16
    str     lr, [x8]

    add     x8, x1, #vcpu_saved_context
    ldp     x19, x20, [x8], #16
    ldp     x21, x22, [x8], #16
    ldp     x23, x24, [x8], #16
    ldp     x25, x26, [x8], #16
    ldp     x27, x28, [x8], #16
    ldp     x29, x9, [x8], #16
    ldr     lr, [x8]
    mov     sp, x9
    ret
END(__context_switch)
#endif
// ------------------------------------------------------------------------
GLOBAL(hcr_set)
    .dword ARCH_HCR_EL2_SET
GLOBAL(boot_parm)
    .dword 0x0
// ------------------------------------------------------------------------
