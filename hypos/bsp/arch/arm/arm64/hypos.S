/**
 * Hustler's Project
 *
 * File:  hypervisor.S
 * Date:  2024/06/03
 * Usage:
 */

#include <asm/define.h>
#include <asm/offset.h>

// --------------------------------------------------------------

/* --------------------------------------------------------------
 * XXX: Context Switching
 *
 * When context switching between vPEs, the hypervisor software
 * saves the state of one vPE and loads the context of another.
 * --------------------------------------------------------------
 * struct vcpu *__context_switch(struct vcpu *prev,
 *                               struct vcpu *next)
 */
FUNC(__context_switch)
    add x8, x0, #VCPU_CONTEXT
    mov  x9, sp
    stp  x19, x20, [x8], #16  /* store callee-saved registers */
    stp  x21, x22, [x8], #16
    stp  x23, x24, [x8], #16
    stp  x25, x26, [x8], #16
    stp  x27, x28, [x8], #16
    stp  x29, x9,  [x8], #16
    str  lr, [x8]

    add  x8, x1, #VCPU_CONTEXT
    ldp  x19, x20, [x8], #16  /* restore callee-saved registers */
    ldp  x21, x22, [x8], #16
    ldp  x23, x24, [x8], #16
    ldp  x25, x26, [x8], #16
    ldp  x27, x28, [x8], #16
    ldp  x29, x9,  [x8], #16
    ldr  lr, [x8]
    mov  sp, x9
    ret
END(__context_switch)

// --------------------------------------------------------------
