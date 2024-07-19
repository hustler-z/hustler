/**
 * Hustler's Project
 *
 * File:  debug.h
 * Date:  2024/05/20
 * Usage: architectural debug implementation header
 */

#ifndef _ASM_DEBUG_H
#define _ASM_DEBUG_H
// --------------------------------------------------------------
#include <asm/hvm.h>
#include <org/bitops.h>

#ifdef __RK3568__
#include <rk3568/rk3568.h>
#define DEBUG_UART_PA     UART2_BASE

#include <bsp/ns16550.h>
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
#endif

#define DEBUG_UART_VA  \
    (HYPOS_FIXMAP_ADDR(0) + (DEBUG_UART_PA & ~PAGE_MASK))

// --------------------------------------------------------------

#ifdef __ASSEMBLY__
/* If enable assembly debug info
 */
#define ASM_DBG                             (1)

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

#define DBG(string) PR_SECT(.rodata.debug, string)

.macro dump_reg xb
    mov   x0, \xb
    mov   x4, lr
    bl    asm_putn
    mov   lr, x4
.endm

#else /* __ASSEMBLY__ */

void early_putc(char c);
void early_flush(void);
void early_debug(const char *s);

#endif /* !__ASSEMBLY__ */

// --------------------------------------------------------------
#endif /* _ASM_DEBUG_H */
