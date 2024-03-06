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
 * FilePath: cmd_wdt.c
 * Date: 2023-05-28 14:53:42
 * LastEditTime: 2023-06-4 17:46:03
 * Description:  This file is for wdt cmd function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/5/22   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

#include "wdt_polled_example.h"
#include "wdt_intr_example.h"

/* usage info function for wdt example */
static void FWdtExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("wdt polled\r\n");
    printf("-- run wdt polled example at controller\r\n");
    printf("wdt intr  \r\n");
    printf("-- run wdt intr example at controller \r\n");
}

/* entry function for wdt example */
static int FWdtExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FWdtExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "polled"))
    {
        ret = FWdtPolledExample();
    }
    else if (!strcmp(argv[1], "intr"))
    {
        ret = FWdtIntrExample();    
    }

    return ret;
}

/* register command for wdt example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), wdt, FWdtExampleEntry, wdt example);
#endif