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
 * FilePath: cmd_can.c
 * Date: 2023-10-16 10:41:45
 * LastEditTime: 2023-10-16 10:41:45
 * Description:  This file is for can example command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2023/10/16   first commit
 */
#include "shell.h"
#include "can_example.h"
#include <string.h>
#include <stdio.h>

static void CreateTasksCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" can intr \r\n");
    printf("    -- Create can interrupt test example now. \r\n");
    printf(" can polled \r\n");
    printf("    -- Create can polled test example now. \r\n");
#if defined(CONFIG_TARGET_E2000)
    printf(" can filter \r\n");
    printf("    -- Create can filter test example now. \r\n");
#endif
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
        FFreeRTOSCreateCanIntrTestTask();
    }
    else if (!strcmp(argv[1], "polled"))
    {
        FFreeRTOSCreateCanPolledTestTask();
    }
#if defined(CONFIG_TARGET_E2000)
    else if (!strcmp(argv[1], "filter"))
    {
        FFreeRTOSCanCreateFilterTestTask();
    }
#endif
    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreateTasksCmdUsage();
    }
    
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), can, CreateTasksCmd, can creating test);


