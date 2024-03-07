/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: cmd_ddma.c
 * Date: 2024-01-25 14:53:41
 * LastEditTime: 2024-02-01 17:36:17
 * Description:  This file is for ddma example cmd catalogue
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2024/01/25  first release
 */

#include<stdio.h>
#include <string.h>

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

#include <ddma_spim_lookback_example.h>


/* usage info function for ddma_spim example */
static void FDdmaSpimExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("ddma spim_lookback example\r\n");
    printf("-- run ddma spim loopback mode example at controller\r\n");

}

/* entry function for spim example */
static int FDdmaSpimExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FDdmaSpimExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "spim_lookback"))
    {
        ret = FDdmaSpimExample();
    }


    return ret;
}

/* register command for ddma spim example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), ddma, FDdmaSpimExampleEntry, ddma example);
#endif
