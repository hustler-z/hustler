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
 * FilePath: cmd_timer.c
 * Date: 2023-05-22 14:53:42
 * LastEditTime: 2023-05-26 17:46:03
 * Description:  This file is for timer cmd function implmentation
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

#include "timer_once_timing_example.h"
#include "timer_cycle_timing_example.h"

/* usage info function for timer example */
static void FTimerExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("timer once_test \r\n");
    printf("-- run timer once timing example at controller \r\n");
    printf("timer cyc_test \r\n");
    printf("-- run timer cycle timing example at controller \r\n");
}

/* entry function for timer example */
static int FTimerExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FTimerExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "once_test"))
    {
        ret = FTimerOnceTimingExample();
    }
    else if (!strcmp(argv[1], "cyc_test"))
    {

        ret = FTimerCycleTimingExample();    
    }

    return ret;
}

/* register command for timer example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), timer, FTimerExampleEntry, timer example);
#endif