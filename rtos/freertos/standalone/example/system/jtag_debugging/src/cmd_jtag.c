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
 * FilePath: cmd_jtag.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for jtag example cmd
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/10/30   first release
 */

/***************************** Include Files *********************************/
#include<stdio.h>
#include <string.h>

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

#include "jtag_walk_through_c_example.h"
#include "jtag_walk_through_cxx_example.h"

/* usage info function for spim example */
static void FJtagExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("jtag debug_c\r\n");
    printf("-- walk through c source code\r\n");
#ifdef CONFIG_ENABLE_CXX
    printf("jtag debug_cxx\r\n");
    printf("-- walk through cxx source code\r\n");
#endif
    printf("jtag brk\r\n");
    printf("-- manually insert a unhandled breakpoint cause sync error\r\n");
}

/* entry function for spim example */
static int FJtagExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FJtagExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "debug_c"))
    {
        ret = JtagWalkThroughCExample();
    }
#ifdef CONFIG_ENABLE_CXX
    else if (!strcmp(argv[1], "debug_cxx"))
    {
        ret = JtagWalkThroughCXXExample();
    }
#endif
    else if (!strcmp(argv[1], "brk"))
    {
        ret = JtagUnhandledSoftBrk();
    }

    return ret;
}

/* register command for spim example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), jtag, FJtagExampleEntry, jtag debugging example);
#endif