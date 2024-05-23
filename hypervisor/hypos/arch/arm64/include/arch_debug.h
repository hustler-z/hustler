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

#define ARCH_EARLY_UART_PA  (0xFE660000U)
#define ARCH_EARLY_UART_VA  (0x0)

// ------------------------------------------------------------------------
/* Register offsets */
#define UART_RBR          0x00    /* receive buffer       */
#define UART_THR          0x00    /* transmit holding     */
#define UART_IER          0x01    /* interrupt enable     */
#define UART_IIR          0x02    /* interrupt identity   */
#define UART_FCR          0x02    /* FIFO control         */
#define UART_LCR          0x03    /* line control         */
#define UART_MCR          0x04    /* Modem control        */
#define UART_LSR          0x05    /* line status          */
#define UART_MSR          0x06    /* Modem status         */
#define UART_USR          0x1F    /* Status register (DW) */
#define UART_DLL          0x00    /* divisor latch (ls) (DLAB=1) */
#define UART_DLM          0x01    /* divisor latch (ms) (DLAB=1) */
#define UART_XR_EFR       0x09    /* Enhanced function register (Exar) */

/* Interrupt Enable Register */
#define UART_IER_ERDAI    0x01    /* rx data recv'd       */
#define UART_IER_ETHREI   0x02    /* tx reg. empty        */
#define UART_IER_ELSI     0x04    /* rx line status       */
#define UART_IER_EMSI     0x08    /* MODEM status         */

/* Interrupt Identification Register */
#define UART_IIR_NOINT    0x01    /* no interrupt pending */
#define UART_IIR_IMA      0x06    /* interrupt identity:  */
#define UART_IIR_LSI      0x06    /*  - rx line status    */
#define UART_IIR_RDA      0x04    /*  - rx data recv'd    */
#define UART_IIR_THR      0x02    /*  - tx reg. empty     */
#define UART_IIR_MSI      0x00    /*  - MODEM status      */
#define UART_IIR_BSY      0x07    /*  - busy detect (DW) */

/* FIFO Control Register */
#define UART_FCR_ENABLE   0x01    /* enable FIFO          */
#define UART_FCR_CLRX     0x02    /* clear Rx FIFO        */
#define UART_FCR_CLTX     0x04    /* clear Tx FIFO        */
#define UART_FCR_DMA      0x10    /* enter DMA mode       */
#define UART_FCR_TRG1     0x00    /* Rx FIFO trig lev 1   */
#define UART_FCR_TRG4     0x40    /* Rx FIFO trig lev 4   */
#define UART_FCR_TRG8     0x80    /* Rx FIFO trig lev 8   */
#define UART_FCR_TRG14    0xC0    /* Rx FIFO trig lev 14  */

#define UART_FCR_R_TRIG_00 0x00
#define UART_FCR_R_TRIG_01 0x40
#define UART_FCR_R_TRIG_10 0x80
#define UART_FCR_R_TRIG_11 0xC0
#define UART_FCR_T_TRIG_00 0x00
#define UART_FCR_T_TRIG_01 0x10
#define UART_FCR_T_TRIG_10 0x20
#define UART_FCR_T_TRIG_11 0x30

/* Line Control Register */
#define UART_LCR_DLAB     0x80    /* Divisor Latch Access */

/* Modem Control Register */
#define UART_MCR_DTR      0x01    /* Data Terminal Ready  */
#define UART_MCR_RTS      0x02    /* Request to Send      */
#define UART_MCR_OUT2     0x08    /* OUT2: interrupt mask */
#define UART_MCR_LOOP     0x10    /* Enable loopback test mode */
#define UART_MCR_TCRTLR   0x40    /* Access TCR/TLR (TI16C752, EFR[4]=1) */

/* Line Status Register */
#define UART_LSR_DR       0x01    /* Data ready           */
#define UART_LSR_OE       0x02    /* Overrun              */
#define UART_LSR_PE       0x04    /* Parity error         */
#define UART_LSR_FE       0x08    /* Framing error        */
#define UART_LSR_BI       0x10    /* Break                */
#define UART_LSR_THRE     0x20    /* Xmit hold reg empty  */
#define UART_LSR_TEMT     0x40    /* Xmitter empty        */
#define UART_LSR_ERR      0x80    /* Error                */

/* These parity settings can be ORed directly into the LCR. */
#define UART_PARITY_NONE  (0<<3)
#define UART_PARITY_ODD   (1<<3)
#define UART_PARITY_EVEN  (3<<3)
#define UART_PARITY_MARK  (5<<3)
#define UART_PARITY_SPACE (7<<3)

/* Bits in Exar specific UART_XR_EFR register */
#define UART_EFR_ECB      0x10

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

#endif /* __ASSEMBLY__ */

// ------------------------------------------------------------------------
#endif /* _ARCH_DEBUG_H */
