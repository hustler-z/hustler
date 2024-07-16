/**
 * hustler's project
 *
 * file:  smp.s
 * date:  2024/05/31
 * usage: multi-processor implementation
 */

#include <asm/define.h>
#include <asm/debug.h>
#include <asm/sysregs.h>
#include <asm/offset.h>

/* --------------------------------------------------------------
 *  x0  -
 *  x1  -
 *  x2  -
 *  x3  -
 *  x4  -
 *  x5  -
 *  x6  -
 *  x7  -
 *  x8  -
 *  x9  -
 *  x10 -
 *  x11 -
 *  x12 -
 *  x13 -
 *  x14 -
 *  x15 -
 *  x16 -
 *  x17 -
 *  x18 -
 *  x19 -
 *  x20 -
 *  x21 -
 *  x22 -
 *  x23 - UART address
 *  x24 -
 *  x25 -
 *  x26 -
 *  x27 -
 *  x28 -
 *  x29 -
 *  x30 - lr
 * --------------------------------------------------------------
 */

// --------------------------------------------------------------

    __ENTRY

FUNC(smpboot_setup)
    msr  DAIFSet, #0xF

    ldr  x0, =_head
    adr  x19, _head
    sub  x20, x19, x0

    mrs  x0, MPIDR_EL1
    ldr  x13, =(~MPIDR_HWID_MASK)
    bic  x24, x0, x13

    adr_l x0, smpboot_cpu
    dsb  sy
2:  ldr  x1, [x0]
    cmp  x1, x24
    beq  1f
    wfe
    b    2b

1:  bl   get_cpu_affinity
#if ASM_DBG
    DBG("- (CPU) ")
    dump_reg x4
    DBG(" -\r\n")
#endif

    bl  el2_entry
    ldr lr, =smpboot_switched
    b   smpcpu_mmu_setup

smpboot_switched:
    bl   traps_setup

    ldr  x3, =boot_setup
    add  x3, x3, #HCPU_STACK
    ldr  x3, [x3]
    add  x3, x3, #STACK_SIZE
    sub  x3, x3, #HCPU_SIZE
    mov  sp, x3

    b   __smp_setup
END(smpboot_setup)

/* Note with this, [31:0] => AFF3, AFF2, AFF1, AFF0
 */
FUNC(get_cpu_affinity)
    mrs  x4, MPIDR_EL1
    ubfx x5, x4, #32, #MPIDR_LEVEL_BITS
    bfi  w4, w5, #24, #MPIDR_LEVEL_BITS
    ret
END(get_cpu_affinity)

// --------------------------------------------------------------
