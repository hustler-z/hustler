/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: mpy_uart.c
 * Created Date: 2023-12-01 15:22:57
 * Last Modified: 2024-01-18 19:50:33
 * Description:  This file is for the uart of micropython
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2023/12/07   Modify the format and establish the version
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "fpl011.h"
#include "fassert.h"
#include "fparameters.h"
#include "mpconfig.h"

#define MPY_SHELL_UART_ID    FUART1_ID
static FPl011 serial;

void MpySerialConfig(void)
{
    s32 ret = FT_SUCCESS;

    const FPl011Config *config_p;
    FPl011Config config_value;
    memset(&serial, 0, sizeof(serial));
    config_p = FPl011LookupConfig(MPY_SHELL_UART_ID) ;
    memcpy(&config_value, config_p, sizeof(FPl011Config));
    /* 初始化PL011 */
    ret = FPl011CfgInitialize(&serial, &config_value);
    FASSERT(FT_SUCCESS == ret);
    FPl011SetOptions(&serial, FPL011_OPTION_UARTEN | FPL011_OPTION_RXEN | FPL011_OPTION_TXEN | FPL011_OPTION_FIFOEN);
    return;
}

// Receive single character
int mp_hal_stdin_rx_chr(void)
{
    char data = 0;
    int length = FPl011Receive(&serial, &data, 1);
    (void)length;
    return data;
}

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len)
{
    FPl011BlockSend(&serial, str, len);
}


