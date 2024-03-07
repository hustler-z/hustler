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
 * FilePath: cmd_gic.c
 * Date: 2023-04-26 08:37:22
 * LastEditTime: 2022-04-27 11:00:53
 * Description:  This file is for the gic example cmd catalogue.
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------     --------------------------------------
 * 1.0   liqiaozhong   2023/4/27    first commit and add spi, sgi, spi functions
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"
#include <stdio.h>
#include <string.h>

#include "ppi_example.h"
#include "sgi_example.h"
#include "spi_example.h"
/* usage info function for gic example */
static void FGicExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("gic ppi_example\r\n");
    printf("-- run ppi test example.\r\n");
#ifdef CONFIG_SLAVE_CORE_ID
    printf("gic sgi_example\r\n");
    printf("-- run sgi test example.\r\n");
    printf("gic spi_example\r\n");
    printf("-- run spi test example.\r\n");
#endif
}

/* entry function for gic example */
static int FGicExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FGicExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "ppi_example"))
    {
        ret = FPpiExample();
    }
#ifdef CONFIG_SLAVE_CORE_ID
    else if (!strcmp(argv[1], "sgi_example"))
    {
        ret = FSgiExample();    
    }
    else if (!strcmp(argv[1], "spi_example"))
    {
        ret = FSpiExample();    
    }
#else
    printf("Not set slave core, so cannot use sgi and spi example. \r\n");
#endif

    return ret;
}

/* register command for gic example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), gic, FGicExampleEntry, gic example);
#endif