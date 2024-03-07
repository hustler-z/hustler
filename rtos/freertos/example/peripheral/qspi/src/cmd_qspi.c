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
 * FilePath: cmd_qspi.c
 * Date: 2023-11-16 10:41:45
 * LastEditTime: 2023-11-16 10:41:45
 * Description:  This file is for qspi example command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2023/11/16  first commit
 */
#include "shell.h"
#include "qspi_example.h"
#include <string.h>
#include <stdio.h>
#include "projdefs.h"

static void CreateTasksCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("qspi check \r\n");
    printf("-- run qspi flash connection check example at controller \r\n");
    printf("qspi polled\r\n");
    printf("-- run qspi flash polled example at controller\r\n");
    printf("qspi indirect \r\n");
    printf("-- run qspi flash indirect example at controller\r\n");
    printf("qspi dual_flash_test \r\n");
    printf("-- run qspi dual flash stack example at controller\r\n");
}

int CreateTasksCmd(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        CreateTasksCmdUsage();
        return -1;
    }
    else if (!strcmp(argv[1], "check"))
    {
        ret = FFreeRTOSQspiCheckTaskCreate(0);
    }
    else if (!strcmp(argv[1], "polled"))
    {
        ret = FFreeRTOSQspiPolledTaskCreate(0);
    }
    else if (!strcmp(argv[1], "indirect"))
    {
        ret = FFreeRTOSQspiIndirectTaskCreate(0);
    }
    else if (!strcmp(argv[1], "dual_flash_test"))
    {
        ret = FFreeRTOSQspiDualFlashTaskCreate();
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreateTasksCmdUsage();
    }
    
    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), qspi, CreateTasksCmd, qspi creating test);


