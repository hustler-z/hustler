/**
 * Hustler's Project
 *
 * File:  smp.S
 * Date:  2024/05/31
 * Usage: Multi-processor Implementation
 */

#include <asm/define.h>
#include <asm/debug.h>
#include <asm/sysregs.h>

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

/* TODO
 * SMP boot entry: after bootcpu's up, time to kick other cpus up
 */
GLOBAL(bootsmp_entry)
    msr  DAIFSet, 0x0F

    ldr  x0, =_head
    adr  x19, _head
    sub  x20, x19, x0

    bl   get_cpu_affinity
    mov  x24, x4
#if ASM_DBG
    DBG("- (CPU) 0x")
    dump_reg x24
    DBG(" -\r\n")
#endif
END(bootsmp_entry)

/* Note with this, [31:0] => AFF3, AFF2, AFF1, AFF0
 */
FUNC(get_cpu_affinity)
    mrs  x4, MPIDR_EL1
    ubfx x5, x4, #32, #MPIDR_LEVEL_BITS
    bfi  w4, w5, #24, #MPIDR_LEVEL_BITS
    ret
END(get_cpu_affinity)

// --------------------------------------------------------------
