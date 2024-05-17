/**
 * Copyright (c) 2024 Hustler
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * @file cpu_debug.c
 * @author Hustler Rock (roxhustlin@gmail.com)
 * @brief Early debug uart implementation
 */
#include <vmm_types.h>

#ifdef CONFIG_EARLY_DEBUG_UART
// -------------------------------------------------------------------
#define MAX_NBYTES                (8 + 3)
#define UART_8250_BASE            (0xfe660000UL)
/**
 * rk3566 specified
 * baud rate = (serial clock freq) / (16 * divisor)
 */
#define UART_8250_DLL              (0x01)
#define UART_8250_DLH              (0x00)
#define UART_LCR_STOP              (0x04)
#define UART_LCR_DLAB              (0x80)
#define UART_LCR_WLEN8             (0x03)
#define UART_LCR_PARITY            (0x08)
#define UART_MCR_LOOP              (0x10)
#define UART_FCR_FIFO_ENABLE       (0x01)
#define UART_FCR_R_TRIG_10         (0x80)
#define UART_FCR_T_TRIG_10         (0x20)
#define UART_USR_TX_FIFO_NOT_FULL  (0x02)

/* IO definitions (access restrictions to peripheral registers) */
#ifdef __cplusplus
#define __I     volatile           /*!< brief Defines 'read only' permissions */
#else
#define __I     volatile const     /*!< brief Defines 'read only' permissions */
#endif
#define __O     volatile           /*!< brief Defines 'write only' permissions */
#define __IO    volatile           /*!< brief Defines 'read / write' permissions */

struct rk3566_uart {
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

#define RK3566_DBG_UART          ((struct rk3566_uart *)(UART_8250_BASE))
#define RK3566_BAUD_RATE         (1500000)
#define RK3566_SERIAL_CLK_RATE   (24000000)

void __attribute__ ((section(".entry"))) _debug_serial_init(void)
{
    u32 divisor_latch = RK3566_SERIAL_CLK_RATE / (16 * RK3566_BAUD_RATE);
    /* enable fifo */
    RK3566_DBG_UART->fcr = UART_FCR_FIFO_ENABLE | UART_FCR_R_TRIG_10 |
        UART_FCR_T_TRIG_10;
    /* set parity and stop bit */
    RK3566_DBG_UART->lcr &= ~(UART_LCR_PARITY);
    RK3566_DBG_UART->lcr &= ~(UART_LCR_STOP);
    RK3566_DBG_UART->lcr |= UART_LCR_WLEN8;
    /* set baud rate */
    RK3566_DBG_UART->mcr |= UART_MCR_LOOP;
    RK3566_DBG_UART->lcr |= UART_LCR_DLAB;
    RK3566_DBG_UART->dll = divisor_latch && 0xff;
    RK3566_DBG_UART->dlh = (divisor_latch >> 8) && 0xff;
    /* disable DLAB */
    RK3566_DBG_UART->lcr &= ~(UART_LCR_DLAB);
    RK3566_DBG_UART->mcr &= ~(UART_MCR_LOOP);
}

void __attribute__ ((section(".entry"))) _debug_serial_puts(char *str)
{
    while (*str != '\0') {
        while (!(RK3566_DBG_UART->usr & UART_USR_TX_FIFO_NOT_FULL));
        RK3566_DBG_UART->thr = *str++;
    }
}

static char hex_lut[16] =
    {'0', '1', '2', '3', '4', '5', '6', '7',
     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void __attribute__ ((section(".entry"))) _debug_serial_hexdump(char *prefix, u32 num)
{
    int base = 16, cnt = 8;
    char buf[MAX_NBYTES] = {'0'}, *hexstr;

    _debug_serial_puts(prefix);

    hexstr = &buf[MAX_NBYTES - 1];
    *hexstr   = '\0';
    *hexstr-- = '\r';
    *hexstr-- = '\n';

    do {
        *hexstr-- = hex_lut[num % base];
    } while ((num /= base) || (cnt-- > 0));

    _debug_serial_puts((char *)buf);
}

/**
 * currentEL - Holds the current exception level
 * [3:2] - EL
 * [1:0] - reserved
 * 0b0000 - EL0
 * 0b0100 - EL1
 * 0b1000 - EL2
 * 0b1100 - EL3
 */
char *el0t_info = "\rCurrent Exception Level:\tEL0t\n\r";
char *el1t_info = "\rCurrent Exception Level:\tEL1t\n\r";
char *el1h_info = "\rCurrent Exception Level:\tEL1h\n\r";
char *el2t_info = "\rCurrent Exception Level:\tEL2t\n\r";
char *el2h_info = "\rCurrent Exception Level:\tEL2h\n\r";
char *el3t_info = "\rCurrent Exception Level:\tEL3t\n\r";
char *el3h_info = "\rCurrent Exception Level:\tEL3h\n\r";

void __attribute__ ((section(".entry"))) early_debug_el(void)
{
    int el;
    char *el_info;

    asm volatile ("mrs %0, currentEL" : "=r" (el));
    switch (el) {
    case 0x0:
        el_info = el0t_info;
        break;
    case 0x4:
        el_info = el1t_info;
        break;
    case 0x5:
        el_info = el1h_info;
        break;
    case 0x8:
        el_info = el2t_info;
        break;
    case 0x9:
        el_info = el2h_info;
        break;
    case 0xc:
        el_info = el3t_info;
        break;
    case 0xd:
        el_info = el3h_info;
        break;
    default:
        _debug_serial_puts("\rain't no way!!\n\r");
        return;
    }

    _debug_serial_puts(el_info);
}

void __attribute__ ((section(".entry"))) early_daif_disable(void)
{
    asm volatile ("msr daifset, #0x0f");
}
// -------------------------------------------------------------------
#endif
