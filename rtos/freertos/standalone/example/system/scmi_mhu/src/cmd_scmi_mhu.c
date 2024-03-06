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
 * FilePath: cmd_scmi_mhu.c
 * Date: 2023-07-27 14:53:42
 * LastEditTime: 2023-07-27 18:55:74
 * Description:  This file is for scmi mhu search example function implementation.
 *
 * Modify History:
 *  Ver     Who          Date             Changes
 * -----   ------      --------    --------------------------------------
 *  1.0   liqiaozhong  2023/7/27   add scmi mhu search example
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include <stdio.h>
#include <string.h>

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"

#include "scmi_mhu_search_example.h"
/************************** Function Prototypes ******************************/
#define FCHAN_ID (1U << 0)

static void FScmiCmdUsage()
{
    printf("Usage:\r\n");
    printf("scmi mhu_auto\r\n");
    printf("-- Run AP search SE by SCMI-MHU example.\r\n");
}

static int FScmiMhuExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        FScmiCmdUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "mhu_auto"))
    {
        ret = FScmiMhuSearchExample();
    }

    return ret;
}

/* register command for serial example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), scmi, FScmiMhuExampleEntry, scmi example);
#endif //CONFIG_USE_LETTER_SHELL