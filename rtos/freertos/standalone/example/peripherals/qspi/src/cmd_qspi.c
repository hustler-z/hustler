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
 * FilePath: cmd_qspi.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for qspi cmd catalogue
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/3/8   first release
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
#include "qspi_flash_connection_check_example.h"
#include "qspi_flash_polled_example.h"
#include "qspi_dual_flash_stack_example.h"
#include "qspi_flash_indirect_example.h"

/* usage info function for qspi example */
static void FQspiExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("qspi check \r\n");
    printf("-- run qspi flash connection check example at controller \r\n");
    printf("qspi polled\r\n");
    printf("-- run qspi flash polled example at controller\r\n");
    printf("qspi dual_flash_test \r\n");
    printf("-- run qspi dual flash stack example at controller\r\n");
    printf("qspi indirect \r\n");
    printf("-- run qspi flash indirect example at controller\r\n");
}

/* entry function for qspi example */
static int FQspiExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FQspiExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "check"))
    {
        ret = FQspiFlashConnectCheckExample();
    }

    if (!strcmp(argv[1], "polled"))
    {
        ret = FQspiFlashPolledExample();
    }

    if (!strcmp(argv[1], "dual_flash_test"))
    {
        ret = FQspiDualFlashStackExample();
    }

    if (!strcmp(argv[1], "indirect"))
    {
        ret = FQspiFlashIndirectExample();
    }
    
    return ret;
}

/* register command for qspi example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), qspi, FQspiExampleEntry, qspi example);
#endif