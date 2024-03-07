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
 * FilePath: cmd_iopad.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for the iopad cmd catalogue.
 *
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----   -------    --------     --------------------------------------
 *  1.0  liqiaozhong  2023/03/05   first commit
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifndef CONFIG_TARGET_E2000
    #error "This example support only E2000 D/Q/S !!!"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "ftypes.h"

#include "iopad_common.h"
#include "iopad_get_config_example.h"
#include "iopad_set_config_example.h"

/* usage info function for iopad example */
static void FIopadExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("iopad get_iopad_example\r\n");
    printf("-- run iopad config getting example on defult controller\r\n");
    printf("iopad set_iopad_example\r\n");
    printf("-- run iopad config setting example on defult controller\r\n");
}

/* entry function for iopad example */
static int FIopadExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FIopadExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "get_iopad_example"))
    {
        ret = FIopadGetCurrentSettingExample();
    }
    else if (!strcmp(argv[1], "set_iopad_example"))
    {
        ret = FIopadModifySettingExample();    
    }

    return ret;
}

/* register command for iopad example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), iopad, FIopadExampleEntry, iopad example);
#endif