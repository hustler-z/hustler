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
 * FilePath: cmd_atomic.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for atomic example function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong   2023/2/17   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "atomic_example.h"

#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"


/* usage info function for atomic example */
static void FAtomicExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("atomic start\r\n");
    printf("-- run atomic example\r\n");
}

/* entry function for atomic example */
static int FAtomicExampleEntry(int argc, char *argv[])
{
    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FAtomicExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "start"))
    {
        /*run the atomic example*/
        FAtomicExample();
    }
   
    return 0;
}

/* register command for atomic example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), atomic, FAtomicExampleEntry, atomic example);
#endif