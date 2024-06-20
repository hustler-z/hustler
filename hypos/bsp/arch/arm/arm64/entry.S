/**
 * Hustler's Project
 *
 * File:  entry.S
 * Date:  2024/05/23
 * Usage:
 */

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
 *
 * TODO:
 * (1) when exception happened, dump useful infos for later analysis
 * (2) save and restore execution environment when possible
 */

#include <asm/debug.h>
#include <asm/define.h>
#include <asm/offset.h>

// --------------------------------------------------------------
.macro trap_entry, hyp
    dmb  sy

    sub  sp, sp, #(HCPU_SPSR_EL1 - HCPU_LR)
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

    add  x21, sp, #HCPU_SIZE

    stp  lr, x21, [sp, #HCPU_LR]
    mrs  x21, elr_el2
    str  x21, [sp, #HCPU_PC]

    add  x21, sp, #HCPU_CPSR
    mrs  x22, SPSR_EL2
    mrs  x23, ESR_EL2
    stp  x22, x23, [x21]
.endm

.macro trap_exit, hyp
    ldr  x21, [sp, #HCPU_PC]
    ldr  x22, [sp, #HCPU_CPSR]

    pop  x0,  x1
    pop  x2,  x3
    pop  x4,  x5
    pop  x6,  x7
    pop  x8,  x9

    msr  ELR_EL2, x21
    msr  SPSR_EL2, x22

    pop  x10, x11
    pop  x12, x13
    pop  x14, x15
    pop  x16, x17
    pop  x18, x19
    pop  x20, x21
    pop  x22, x23
    pop  x24, x25
    pop  x26, x27
    pop  x28, x29

    ldr  lr, [sp], #(HCPU_SPSR_EL1 - HCPU_LR)

    dmb  sy

    eret
.endm

.macro trap_ventry label, hyp
    .align 7                    /* 256 (2 << 7) byte */
\label:
    trap_entry hyp=\hyp
    mov  x0, sp
    b  \label\()_handler
    trap_exit  hyp=\hyp
.endm

// --------------------------------------------------------------

/* EL2 Hypervisor Exception Vectors
 */
    .align 11                   /* 4096 (2 << 11) byte */
GLOBAL(hyp_vectors)
    trap_ventry hyp_syn_el2t, 1
    trap_ventry hyp_irq_el2t, 1
    trap_ventry hyp_fiq_el2t, 1
    trap_ventry hyp_serror_el2t, 1

    trap_ventry hyp_syn_el2h, 1
    trap_ventry hyp_irq_el2h, 1
    trap_ventry hyp_fiq_el2h, 1
    trap_ventry hyp_serror_el2h, 1

    trap_ventry guest_syn, 0
    trap_ventry guest_irq, 0
    trap_ventry guest_fiq, 0
    trap_ventry guest_serror, 0

    trap_ventry guest_syn_compact, 0
    trap_ventry guest_irq_compact, 0
    trap_ventry guest_fiq_compact, 0
    trap_ventry guest_serror_compact, 0
END(hyp_vectors)

// --------------------------------------------------------------

/* EL3 SMC Dummy Exception Vectors
 */
.macro smc_ventry label
    .align 7
\label:
    DBG("- SMC Exception -\r\n")
    b  .
.endm

    .align 11
GLOBAL(smc_vectors)
    smc_ventry smc_sync_sp0
    smc_ventry smc_irq_sp0
    smc_ventry smc_fiq_sp0
    smc_ventry smc_serror_sp0

    smc_ventry smc_sync_spx
    smc_ventry smc_irq_spx
    smc_ventry smc_fiq_spx
    smc_ventry smc_serror_spx

    smc_ventry hyp_syn
    smc_ventry hyp_irq
    smc_ventry hyp_fiq
    smc_ventry hyp_serror

    smc_ventry hyp_syn_compact
    smc_ventry hyp_irq_compact
    smc_ventry hyp_fiq_compact
    smc_ventry hyp_serror_compact
END(smc_vectors)
// --------------------------------------------------------------
