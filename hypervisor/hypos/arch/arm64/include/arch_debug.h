/**
 * Hustler's Project
 *
 * File:  arch_debug.h
 * Date:  2024/05/20
 * Usage: architectural debug implementation header
 */

#ifndef _ARCH_DEBUG_H
#define _ARCH_DEBUG_H
// ------------------------------------------------------------------------
#include <arch_pgtbl.h>

#define ARCH_EARLY_UART_PA  (0xFE660000U)
#define ARCH_EARLY_UART_VA  \
    (FIXMAP_ADDR(0) + (ARCH_EARLY_UART_PA & ~PAGE_MASK))

// ------------------------------------------------------------------------
#define RK3568_DEBUG_8250_UART
#ifdef  RK3568_DEBUG_8250_UART
#include <arch_bitops.h>

/* Register offsets */
#define UART_RBR          0x00    /* receive buffer       */
#define UART_DLL          0x00    /* divisor latch (ls) (DLAB=1) */
#define UART_THR          0x00    /* transmit holding     */
#define UART_DLM          0x04    /* divisor latch (ms) (DLAB=1) */
#define UART_IER          0x04    /* interrupt enable     */
#define UART_FCR          0x08    /* FIFO control         */
#define UART_IIR          0x08    /* interrupt identity   */
#define UART_LCR          0x0C    /* line control         */
#define UART_MCR          0x10    /* Modem control        */
#define UART_LSR          0x14    /* line status          */
#define UART_MSR          0x18    /* Modem status         */
#define UART_USR          0x7c    /* Status register (DW) */

/* Line Status Register */
#define UART_LSR_DR       BIT(0, U)    /* Data ready           */
#define UART_LSR_OE       BIT(1, U)    /* Overrun              */
#define UART_LSR_PE       BIT(2, U)    /* Parity error         */
#define UART_LSR_FE       BIT(3, U)    /* Framing error        */
#define UART_LSR_BI       BIT(4, U)    /* Break                */
#define UART_LSR_THRE     BIT(5, U)    /* Xmit hold reg empty  */
#define UART_LSR_TEMT     BIT(6, U)    /* Xmitter empty        */
#define UART_LSR_ERR      BIT(7, U)    /* Receiver FIFO error  */

#endif
// ------------------------------------------------------------------------

#ifdef __ASSEMBLY__

#define RODATA_SECT(section, label, msg)    \
.pushsection section, "aMS", %progbits, 1;  \
label:  .asciz msg;                         \
.popsection

#define PR_SECT(section, string)            \
    mov   x3, lr;                           \
    adr_l x0, 98f;                          \
    bl    asm_puts;                         \
    mov   lr, x3;                           \
    RODATA_SECT(section, 98, string)

#define INFO(string) PR_SECT(.rodata.str, string)

#define TAGS(string) PR_SECT(.rodata.idmap, string)

.macro print_reg xb
    mov   x0, \xb
    mov   x4, lr
    bl    asm_putn
    mov   lr, x4
.endm

/* Early stamp for walking thru the assembly code
 */
.macro stamp  ch
    ldr   x2, =ARCH_EARLY_UART_PA
    mov   w3, \ch
    strb  w3, [x2]
.endm

#endif /* __ASSEMBLY__ */

// ------------------------------------------------------------------------
#endif /* _ARCH_DEBUG_H */
