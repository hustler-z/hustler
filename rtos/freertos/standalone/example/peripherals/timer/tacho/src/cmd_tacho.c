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
 * FilePath: cmd_tacho.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for tacho cmd function implmentation
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

#include "tacho_get_fun_rpm_example.h"
#include "tacho_pulse_capture_count_example.h"

/* usage info function for tacho example */
static void FTachoExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("tacho getfunrpm \r\n");
    printf("-- run tacho get fun rpm example at controller \r\n");
    printf("tacho capture\r\n");
    printf("-- run tacho pulse capture count example at controller \r\n");
}

/* entry function for tacho example */
static int FTachoExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FTachoExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "getfunrpm"))
    {
        ret = FTachoGetFunRpmExample();
    }
    else if (!strcmp(argv[1], "capture"))
    {
        ret = FTachoPulseCaptureCountExample();    
    }

    return ret;
}

/* register command for tacho example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), tacho, FTachoExampleEntry, tacho example);
#endif