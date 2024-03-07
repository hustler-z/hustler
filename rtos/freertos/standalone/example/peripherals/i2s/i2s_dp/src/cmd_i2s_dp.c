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
 * FilePath: cmd_i2s_dp.c
 * Created Date: 2023-08-03 10:51:32
 * Last Modified: 2023-12-21 16:20:23
 * Description:  This file is for the I2S DP example cmd catalogue.
 *
 * Modify History:
 *  Ver       Who             Date         Changes
 * -----   ----------       --------     ---------------------------------
 *  1.0   wangzongqiang    2023/07/23    init
 *  1.1   liqiaozhong      2023/12/19    solve bdl miss intr issue
 */

#include <stdio.h>
#include "shell.h"
#include "strto.h"
#include <string.h>

#include "sdkconfig.h"
#include "ferror_code.h"

#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "i2s_dp_rx_example.h"
#include "i2s_dp_tx_example.h"
#include "i2s_dp_init.h"

/* usage info function for i2s example */
static void FI2sExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("i2s media\r\n");
    printf("-- run I2S DP media.\r\n");
    printf("i2s rx_example\r\n");
    printf("-- run I2S RX example.\r\n");
    printf("i2s tx_example\r\n");
    printf("-- run I2S TX example.\r\n");
    printf("i2s stop\r\n");
    printf("-- stop I2S RX and TX task.\r\n");
}

static int FI2SCmdEntry(int argc, char *argv[])
{
    FError ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FI2sExampleUsage();
        return -1;
    }

    if (!strcmp(argv[1], "media"))
    {
        ret = FMediaInit();
        if (ret != FT_SUCCESS)
        {
            printf("FI2s transmmit error: 0x%x\r\n", ret);
            return ret;
        }
        return ret;
    }

    if (!strcmp(argv[1], "rx_example"))
    {
        ret = FI2sDdmaDpRxExample();
        if (ret != FT_SUCCESS)
        {
            printf("FI2s transmmit error: 0x%x\r\n", ret);
            return ret;
        }
        return ret;
    }

    if (!strcmp(argv[1], "tx_example"))
    {
        ret = FI2sDdmaDpTxExample();
        if (ret != FT_SUCCESS)
        {
            printf("FI2s transmmit error: 0x%x\r\n", ret);
            return ret;
        }
        return ret;
    }

    if (!strcmp(argv[1], "stop"))
    {
        ret = FI2sDdmaDpRxStopWork();
        ret = FI2sDdmaDpTxStopWork();
        if (ret != FT_SUCCESS)
        {
            printf("FI2S stop error :0x%x\r\n", ret);
            return ret;
        }
        return ret;
    }

    return ret;
}

/* register command for i2s example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), i2s, FI2SCmdEntry, I2S DP example);
#endif