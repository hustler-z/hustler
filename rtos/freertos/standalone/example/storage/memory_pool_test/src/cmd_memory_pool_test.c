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
 * FilePath: cmd_memory_pool_test_example.c
 * Date: 2023-07-31 11:23:42
 * LastEditTime: 2023-08-1 18:47:54
 * Description:  This file is for memory pool basic test example cmd catalogue.
 *
 * Modify History:
 *  Ver       Who         Date           Changes
 * -----     ------     --------     --------------------------------------
 *  1.0    liqiaozhong  2023/8/1     add memory pool basic test example
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "strto.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"

#include "memory_pool_basic_example.h"
/******************************* Functions ***********************************/
/* usage info function for memory pool test example */
static void MemoryPoolCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("memorypool basic_example\r\n");
    printf("-- Run memory pool basic tests.\r\n");
}

/* entry function for memory pool test example */
static int MemoryPoolCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        MemoryPoolCmdUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "basic_example"))
    {
        ret = MemoryPoolBasicExample();
    }

    return ret;
}
/* register command for the example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), memorypool, MemoryPoolCmdEntry, test memory pool);
#endif //CONFIG_USE_LETTER_SHELL