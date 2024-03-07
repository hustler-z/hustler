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
 * FilePath: cmd_fsema.c
 * Date: 2023-05-25 16:03:21
 * LastEditTime: 2023-05-31 15:57:42
 * Description:  This file is for semaphore example cmd catalogue.
 * 
 * Modify History:
 *  Ver    Who         Date            Changes
 * -----  ------      --------    --------------------------------------
 *  1.0  liuzhihong   2023/5/26      first release
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "fsema_lock_example.h"

/* usage info function for xxx example */
static void FSemaExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("sema lock \r\n");
    printf("-- run semaphore overall test \r\n");
}

/* entry function for ipc example */
static int FSemaExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FSemaExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "lock"))
    {
        ret = FSemaLockExample();
    }

    return ret;
}

/* register command for sema example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), sema, FSemaExampleEntry, test semaphore driver);
#endif