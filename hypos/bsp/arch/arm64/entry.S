/**
 * Hustler's Project
 *
 * File:  entry.S
 * Date:  2024/05/23
 * Usage:
 */

#include <asm/define.h>
#include <asm/hypregs.h>

// --------------------------------------------------------------

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

// --------------------------------------------------------------

/* Stack pushing/popping register pairs
 */
.macro push, xreg1, xreg2
    stp \xreg1, \xreg2, [sp, #-16]!
.endm

.macro pop,  xreg1, xreg2
    ldp \xreg1, \xreg2, [sp], #16
.endm

// --------------------------------------------------------------
.macro hyp_arch_entry
    sub  sp, sp, #(ARCH_REGS_SPSR_EL1 - ARCH_REGS_LR)
    push x28, x29
    push x26, x27
    push x24, x25
    push x22, x23
    push x20, x21
    push x18, x19
    push x16, x17
    push x14, x15
    push x12, x13
    push x10, x11
    push x8,  x9
    push x6,  x7
    push x4,  x5
    push x2,  x3
    push x0,  x1

    stp lr, x21, [sp, #ARCH_REGS_LR]
    mrs x21, elr_el2
    str x21, [sp, #ARCH_REGS_PC]

    add x21, sp, #ARCH_REGS_CPSR
    mrs x22, SPSR_EL2
    mrs x23, ESR_EL2
    stp x22, x23, [x21]
.endm

.macro hyp_arch_exit
    ldr x21, [sp, #ARCH_REGS_PC]
    ldr x22, [sp, #ARCH_REGS_CPSR]

    pop x0,  x1
    pop x2,  x3
    pop x4,  x5
    pop x6,  x7
    pop x8,  x9

    msr ELR_EL2, x21
    msr SPSR_EL2, x22

    pop x10, x11
    pop x12, x13
    pop x14, x15
    pop x16, x17
    pop x18, x19
    pop x20, x21
    pop x22, x23
    pop x24, x25
    pop x26, x27
    pop x28, x29

    ldr lr, [sp], #(ARCH_REGS_SPSR_EL1 - ARCH_REGS_LR)

    dmb sy

    eret
.endm

.macro hyp_ventry label
    .align 7                    /* 256 (2 << 7) byte */
\label:
    hyp_arch_entry
    b  \label\()_handler
    hyp_arch_exit
.endm

// --------------------------------------------------------------
    .align 11                   /* 4096 (2 << 11) byte */
LABEL(hyp_vectors)
    hyp_ventry hyp_syn_el2t
    hyp_ventry hyp_irq_el2t
    hyp_ventry hyp_fiq_el2t
    hyp_ventry hyp_serror_el2t

    hyp_ventry hyp_syn_el2h
    hyp_ventry hyp_irq_el2h
    hyp_ventry hyp_fiq_el2h
    hyp_ventry hyp_serror_el2h

    hyp_ventry guest_syn
    hyp_ventry guest_irq
    hyp_ventry guest_fiq
    hyp_ventry guest_serror

    hyp_ventry guest_syn_compat
    hyp_ventry guest_irq_compat
    hyp_ventry guest_fiq_compat
    hyp_ventry guest_serror_compat
END(hyp_vectors)
// --------------------------------------------------------------
