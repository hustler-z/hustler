/**
 * Hustler's Project
 *
 * File:  cache.S
 * Date:  2024/05/23
 * Usage:
 */

#include <asm/define.h>
// --------------------------------------------------------------
/* Note
 *
 * dcache_line_size - get the minimum D-cache line size from the CTR register.
 */
.macro  dcache_line_size, reg, tmp
    mrs  \tmp, ctr_el0           // read CTR
    ubfm \tmp, \tmp, #16, #19    // cache line size encoding
    mov  \reg, #4                // bytes per word
    lsl  \reg, \reg, \tmp        // actual cache line size
.endm

/* Note __flush_dcache_area(kaddr, size)
 *
 * Ensure that the data held in the page kaddr is written back to the
 * page in question.
 *
 * - kaddr   - kernel address
 * - size    - size in question
 */
FUNC(__flush_dcache_area)
    dcache_line_size x2, x3
    add  x1, x0, x1
    sub  x3, x2, #1
    bic  x0, x0, x3
1:  dc   civac, x0           // clean & invalidate D line / unified line
    add  x0, x0, x2
    cmp  x0, x1
    blo  1b
    dsb  sy
    ret
END(__flush_dcache_area)
// --------------------------------------------------------------
