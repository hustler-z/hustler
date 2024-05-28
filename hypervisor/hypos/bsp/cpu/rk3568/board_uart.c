/**
 * Hustler's Project
 *
 * File:  board_uart.c
 * Date:  2024/05/20
 * Usage: board debug serial initialization
 */

#include <common_type.h>
#include <board_uart.h>
#include <rk3568.h>

#ifdef __cplusplus
#define __I   volatile
#else
#define __I   volatile const
#endif
#define __O   volatile
#define __IO  volatile

struct board_uart_regs {
    union {
        __I  u32 rbr;              /* Address Offset: 0x0000 */
        __IO u32 dll;              /* Address Offset: 0x0000 */
        __O  u32 thr;              /* Address Offset: 0x0000 */
    };
    union {
        __IO u32 dlh;              /* Address Offset: 0x0004 */
        __IO u32 ier;              /* Address Offset: 0x0004 */
    };
    union {
        __O  u32 fcr;              /* Address Offset: 0x0008 */
        __I  u32 iir;              /* Address Offset: 0x0008 */
    };
    __IO u32 lcr;                  /* Address Offset: 0x000C */
    __IO u32 mcr;                  /* Address Offset: 0x0010 */
    __I  u32 lsr;                  /* Address Offset: 0x0014 */
    __I  u32 msr;                  /* Address Offset: 0x0018 */
    __IO u32 scr;                  /* Address Offset: 0x001C */
         u32 reserved0020[4];      /* Address Offset: 0x0020 */
    union {
        __I  u32 srbr;             /* Address Offset: 0x0030 */
        __O  u32 sthr;             /* Address Offset: 0x0030 */
    };
         u32 reserved0034[15];     /* Address Offset: 0x0034 */
    __IO u32 far;                  /* Address Offset: 0x0070 */
    __I  u32 tfr;                  /* Address Offset: 0x0074 */
    __O  u32 rfw;                  /* Address Offset: 0x0078 */
    __I  u32 usr;                  /* Address Offset: 0x007C */
    __I  u32 tfl;                  /* Address Offset: 0x0080 */
    __I  u32 rfl;                  /* Address Offset: 0x0084 */
    __O  u32 srr;                  /* Address Offset: 0x0088 */
    __IO u32 srts;                 /* Address Offset: 0x008C */
    __IO u32 sbcr;                 /* Address Offset: 0x0090 */
    __IO u32 sdmam;                /* Address Offset: 0x0094 */
    __IO u32 sfe;                  /* Address Offset: 0x0098 */
    __IO u32 srt;                  /* Address Offset: 0x009C */
    __IO u32 stet;                 /* Address Offset: 0x00A0 */
    __IO u32 htx;                  /* Address Offset: 0x00A4 */
    __O  u32 dmasa;                /* Address Offset: 0x00A8 */
         u32 reserved00ac[18];     /* Address Offset: 0x00AC */
    __I  u32 cpr;                  /* Address Offset: 0x00F4 */
    __I  u32 ucv;                  /* Address Offset: 0x00F8 */
    __I  u32 ctr;                  /* Address Offset: 0x00FC */
};

struct board_uart_cfgs {
    unsigned long clk_rate;
    unsigned long baud_rate;
    unsigned long uart_base;
};

struct board_uart_dev {
    struct board_uart_cfgs *uart_cfgs;
    struct board_uart_regs *uart_regs;
};

static void board_uart_irq_setup(void)
{

}

static void board_uart_set_baudrate(struct board_uart_dev *uart_dev)
{

}

void __init board_serial_setup(void)
{

}
