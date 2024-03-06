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
 * FilePath: cmd_gdma.c
 * Created Date: 2023-08-15 13:58:06
 * Last Modified: 2023-10-27 15:22:59
 * Description:  This file is for the gdma cmd catalogue.
 *
 * Modify History:
 *  Ver      Who         Date         Changes
 * -----   ------      --------     --------------------------------------
 *  1.0  liqiaozhong   2023/3/28    first commit
 *  2.0  liqiaozhong   2023/10/20   adapt to modified driver
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

#include "gdma_direct_transfer_example.h"
#include "gdma_bdl_transfer_example.h"
#include "gdma_performance_example.h"
#include "gdma_abort_example.h"
#include "gdma_multi_channel_example.h"
/* usage info function */
static void FGdmaExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("gdma direct_transfer_example\r\n");
    printf("-- use direct mode to transfer\r\n");
    printf("gdma bdl_transfer_example\r\n");
    printf("-- use bdl mode to transfer\r\n");
    printf("gdma performance_example\r\n");
    printf("-- gdma performance test\r\n");
    printf("gdma abort_example\r\n");
    printf("-- gdma abort operation\r\n");
    printf("gdma multi_channel_example\r\n");
    printf("-- gdma mutli-channel transfer\r\n");
}

/* entry function */
static int FGdmaExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FGdmaExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "direct_transfer_example"))
    {
        ret = FGdmaDirectTransferExample();
    }
    else if (!strcmp(argv[1], "bdl_transfer_example"))
    {
        ret = FGdmaBDLTransferExample(); 
    }
    else if (!strcmp(argv[1], "performance_example"))
    {
        ret = FGdmaPerformanceExample(); 
    }
    else if (!strcmp(argv[1], "abort_example"))
    {
        ret = FGdmaAbortExample(); 
    }
    else if (!strcmp(argv[1], "multi_channel_example"))
    {
        ret = FGdmaMultiChannelExample(); 
    }
    return ret;
}

/* register command */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), gdma, FGdmaExampleEntry, gdma example);
#endif