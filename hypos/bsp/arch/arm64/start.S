/**
 * Hustler's Project
 *
 * File:  start.S
 * Date:  2024/05/20
 * Usage: armv8 boot process
 */

/* Note
 * --------------------------------------------------------------
 * The requirements are:
 * MMU = off, D-cache = off, I-cache = on or off,
 * x0 = physical address to the FDT blob.
 *
 * x19        - paddr start
 * x20        - phys offset
 * x21        - bootloader argument
 * x23        - debug uart address
 *
 * XXX: Keep in mind that the very start address is linked to
 *      HYPOS_VIRT_START, and loaded at any 4k-aligned address.
 * --------------------------------------------------------------
 */

#include <asm/define.h>
#include <asm/debug.h>
#include <asm/sysregs.h>
#include <asm/gicv3.h>

// --------------------------------------------------------------

    __HEAD

/* Note hypos Code Entry Point
 */
GLOBAL(_head)
    b    _entry
    .long   0
    .quad   __hypos_end - _head /* code size */
    .quad   0
    .quad   0
    .quad   0
    .quad   0
    .ascii  "ARM\x64"
    .long   0

    __ENTRY

_entry:

    /* Disable all exception */
    msr  DAIFSet, #0xF

    /* (a) save bootloader x0 (boot_parm)
     * (c) compute phys offset and save it
     */
    mov  x21, x0

    ldr  x0,  =_head
    adr  x19, _head
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
    dump_reg x1
    DBG(" -\r\n")

    bl   el2_entry

    ldr  lr, =runtime_switched

    /* MMU early setup */
    b    bootcpu_mmu_setup

runtime_switched:
    /* Zero out bss section */
    bl   zero_bss

    /* Set up traps */
    bl   setup_traps

    /* Enable all exception */
    msr  DAIFClr, #0xF

    DBG("- BSP Setup -\r\n")

    /* setup hyp stack and ready for C
     */
    ldr  x3, =__hyp_stack_end
    mov  sp, x3

    mov  x0, x20 /* phys_offset */
    mov  x1, x21
    b    bsp_setup

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
    dump_reg x7
    DBG(" -\r\n")

    /* Hang if not in EL2 */
    cmp  x7, #ARM64_EL2T
    beq  el2_setup

    /* shall never reach here */
    DBG("- hypos must be booted in EL2 -\r\n")
    b    boot_failed

el2_setup:

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

    isb

    /* Ensure we use SP_EL2 to handle any
     * exceptions occured at EL2.
     */
    msr  SPSel, #1

    ret
END(el2_entry)

/* Note BSS - Better Save Space
 * zero-initialized global and static variables are stored,
 * BSS needs to be zeroed out before C sequence.
 */
FUNC_LOCAL(zero_bss)
    DBG("- Time to clear out BSS -\r\n")

#if 1
    adr_l x5, __bss_start
    DBG("- BSS   [0x")
    dump_reg x5
    DBG(" ~ 0x")
    adr_l x6, __bss_end
    dump_reg x6
    DBG("] -\r\n")
#endif

    adr_l x0, __bss_start
    adr_l x1, __bss_end

1:  str  xzr, [x0], #8
    cmp  x0, x1
    blt  1b

    DBG("- Zeroed out BSS -\r\n")

    ret
END(zero_bss)

FUNC_LOCAL(setup_traps)
    /* Set up Vector Base Address Register (EL2)
     * (self-explanatory)
     */
    DBG("- Set VBAR_EL2 -\r\n")
    ldr  x0, =hyp_vectors
    msr  VBAR_EL2, x0

    /* Monitor Debug Configuration Register (EL2)
     * Provides EL2 configuration options for self-hosted
     * debug and the Performance Monitors Extension.
     */
    DBG("- Set MDCR_EL2 -\r\n")
    ldr  x0, =MDCR_EL2_SET
    msr  MDCR_EL2, x0

    /* Achitectural Feature Trap Register (EL2)
     * Controls trapping to EL2 of accesses to CPACR,
     * CPACR_EL1, trace, Activity Monitor, SVE, and
     * Advanced SIMD and floating-point functionality.
     */
    DBG("- Set CPTR_EL2 -\r\n")
    ldr  x0, =CPTR_EL2_SET
    msr  CPTR_EL2, x0

    /* Set up Hypervisor Configuration Register (EL2)
     * To route exception to EL2, set HCR_EL2.{AMO, FMO, IMO}
     */
    DBG("- Set HCR_EL2 -\r\n")
    ldr  x0, =HCR_EL2_SET
    msr  HCR_EL2, x0

    isb

    ret
END(setup_traps)

/* Note hypos boot failed
 */
FUNC(boot_failed)
    DBG("- Boot failed -\r\n")

1:  wfi
    b    1b
END(boot_failed)

/* Note with this, [31:0] => AFF3, AFF2, AFF1, AFF0
 */
FUNC(get_cpu_affinity)
    mrs  x0, MPIDR_EL1
    ubfx x1, x0, #32, #MPIDR_LEVEL_BITS
    bfi  w0, w1, #24, #MPIDR_LEVEL_BITS
    ret
END(get_cpu_affinity)

// --------------------------------------------------------------
