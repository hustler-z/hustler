/**
 * Hustler's Project
 *
 * File:  debug.S
 * Date:  2024/05/20
 * Usage: Early assembly debug print
 */

#include <asm/define.h>
#include <asm/debug.h>

// --------------------------------------------------------------
.macro early_uart_ready xb c
1:
    ldrb w\c, [\xb, #UART_LSR]
    and  w\c, w\c, #UART_LSR_THRE
    cmp  w\c, #UART_LSR_THRE
    bne  1b
.endm

.macro early_uart_transmit xb wt
    strb \wt, [\xb, #UART_THR]
.endm
// --------------------------------------------------------------

/* Note Early Uart Initialization
 *
 * 8250-uart since uboot initialized
 */
FUNC(early_uart_init)
    ldr  x23, =DEBUG_UART_PA

    DBG("- UART Enabled -\r\n")

    ret
END(early_uart_init)

/* Note Print String
 *
 * void asm_puts(char *str);
 */
FUNC(asm_puts)
    early_uart_ready x23, 1
    ldrb w1, [x0], #1
    cbz  w1, 1f
    early_uart_transmit x23, w1
    b    asm_puts
1:
    ret
END(asm_puts)

/* Note Print Number
 *
 * void asm_putn(unsigned long num);
 */
FUNC(asm_putn)
    adr_l x1, hex
    mov   x3, #16
1:
    early_uart_ready x23, 2
    and   x2, x0, #(0xf<<60)
    lsr   x2, x2, #60
    ldrb  w2, [x1, x2]
    early_uart_transmit x23, w2
    lsl   x0, x0, #4
    subs  x3, x3, #1
    bne   1b
    ret
END(asm_putn)

RODATA_SECT(.rodata.debug, hex, "0123456789abcdef")
// --------------------------------------------------------------
