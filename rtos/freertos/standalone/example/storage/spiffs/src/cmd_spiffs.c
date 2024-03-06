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
 * FilePath: cmd_spiffs.c
 * Date: 2023-07-31 11:23:42
 * LastEditTime: 2023-08-1 18:47:54
 * Description:  This file is for spiffs example cmd catalogue.
 *
 * Modify History:
 *  Ver       Who         Date           Changes
 * -----     ------     --------     --------------------------------------
 *  1.0    liqiaozhong  2023/8/1     first release
 *  1.1    zhangyan     2023/8/8     modify
 *  1.2    liqiaozhong  2023/10/8    divide example into spi and qspi parts
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include <string.h>
#include <stdio.h>
#include "strto.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"

#include "spiffs_spi_mount_example.h"
#include "spiffs_qspi_mount_example.h"
#include "spiffs_spi_basic_example.h"
#include "spiffs_qspi_basic_example.h"
/******************************* Functions ***********************************/
/* usage info function for sfud example */
static void SfudCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("spiffs spi_mount_example \r\n");
    printf("-- Run spiffs mount example at one spi controller.\r\n");
    printf("spiffs qspi_mount_example \r\n");
    printf("-- Run spiffs mount example at one qspi controller.\r\n");
    printf("spiffs spi_basic_example \r\n");
    printf("-- Run spiffs basic example at one spi controller.\r\n");
    printf("spiffs qspi_basic_example \r\n");
    printf("-- Run spiffs basic example at one qspi controller.\r\n");
}

/* entry function for sfud example */
static int SfudCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        SfudCmdUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "spi_mount_example"))
    {
        ret = FSpiffsSpiMountExample();
    }
    else if (!strcmp(argv[1], "qspi_mount_example"))
    {
        ret = FSpiffsQspiMountExample();
    }
    else if (!strcmp(argv[1], "spi_basic_example"))
    {
        ret = FSpiffsSpiBasicExample();
    }
    else if (!strcmp(argv[1], "qspi_basic_example"))
    {
        ret = FSpiffsQspiBasicExample();
    }

    return ret;
}
/* register command for the example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), spiffs, SfudCmdEntry, test spiffs);
#endif //CONFIG_USE_LETTER_SHELL