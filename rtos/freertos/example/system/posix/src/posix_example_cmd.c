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
 * FilePath: posix_example_cmd.c
 * Date: 2023-10-12 10:41:45
 * LastEditTime: 2023-10-12 10:41:45
 * Description:  This file is for freertos posix command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2023/06/25 first commit
 */

#include <string.h>
#include <stdio.h>
#include "shell.h"
#include "posix_example.h"

static void CreatePosixCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" posix thread \r\n");
    printf("    -- Create posix thread test now.\r\n");
    printf(" posix demo \r\n");
    printf("    -- Create posix test now.\r\n");
}

int CreatePosixCmd(int argc, char *argv[])
{
    static int create_flg = 0; /* 1 is tasks has been created*/

    if (argc < 2)
    {
        CreatePosixCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "demo"))
    { 
        CreatePOSIXDemoTasks(); 
    }

    if (!strcmp(argv[1], "thread"))
    {
        CreateThreadDemoTasks();
    }

    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreatePosixCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), posix, CreatePosixCmd, posix task test);


