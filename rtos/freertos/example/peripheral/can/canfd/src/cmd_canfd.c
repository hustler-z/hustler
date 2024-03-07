/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: cmd_canfd.c
 * Date: 2023-10-20 10:41:45
 * LastEditTime: 2023-10-20 12:41:45
 * Description:  This file is for canfd example command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2023/10/20   first commit
 */
#include "shell.h"
#include "canfd_intr_loopback_mode_example.h"
#include "canfd_polled_loopback_mode_example copy.h"
#include "canfd_id_filter_example.h"
#include <string.h>
#include <stdio.h>

static void CreateTasksCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" canfd intr \r\n");
    printf("    -- Create canfd interrupt test example now. \r\n");
    printf(" canfd polled \r\n");
    printf("    -- Create canfd polled test example now. \r\n");
    printf(" canfd filter \r\n");
    printf("    -- Create canfd filter test example now. \r\n");
}

int CreateTasksCmd(int argc, char *argv[])
{
    if (argc < 2)
    {
        CreateTasksCmdUsage();
        return -1;
    }
    else if (!strcmp(argv[1], "intr"))
    {
        FFreeRTOSCreateCanfdIntrTestTask();
    }
    else if (!strcmp(argv[1], "polled"))
    {
        FFreeRTOSCreateCanfdPolledTestTask();
    }
    else if (!strcmp(argv[1], "filter"))
    {
        FFreeRTOSCanfdCreateFilterTestTask();
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreateTasksCmdUsage();
    }
    
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), canfd, CreateTasksCmd, canfd creating test);


