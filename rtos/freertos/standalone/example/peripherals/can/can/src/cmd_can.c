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
 * FilePath: cmd_can.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for can cmd catalogue
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/2/24   first release
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

#include "can_intr_loopback_mode_example.h"
#include "can_polled_loopback_mode_example.h"
#include "can_id_filter_example.h"

/* usage info function for can example */
static void FCanExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("can intr \r\n");
    printf("-- run can interrupt loopback mode example at controller \r\n");
    printf("can polled \r\n");
    printf("-- run can polled loopback mode example at controller\r\n");
#if defined(CONFIG_TARGET_E2000)
    printf("can filter \r\n");
    printf("-- run can id filter example at controller\r\n");
#endif
}

/* entry function for can example */
static int FCanExampleEntry(int argc, char *argv[])
{
    int ret = 0;
    u32 id = 0U;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FCanExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "intr"))
    {
        ret = FCanIntrLoopbackExample();
    }

    if (!strcmp(argv[1], "polled"))
    {
        ret = FCanPolledLoopbackExample();
    }

#if defined(CONFIG_TARGET_E2000)
    if (!strcmp(argv[1], "filter"))
    {
        ret = FCanIdFilterExample();
    }
#endif

    return ret;
}

/* register command for can example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), can, FCanExampleEntry, can example);
#endif