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
 * FilePath: cmd_serial.c
 * Date: 2023-04-12 14:53:42
 * LastEditTime: 2023-06-21 17:57:24
 * Description:  This file is for serial example cmd catalogue.
 *
 * Modify History:
 *  Ver     Who          Date             Changes
 * -----   ------      --------    --------------------------------------
 *  1.0   liuzhihong   2023/4/12        first release
 *  1.1   liqiaozhong  2023/6/21        add flow, mio, ddma related examples
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"
#include "serial_poll_example.h"
#include "serial_intr_example.h"
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD) || defined(CONFIG_FIREFLY_DEMO_BOARD)
#include "serial_mio_example.h"
#include "serial_ddma_example.h"
#include "serial_ddma_mio_example.h"
#endif
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD)
#include "serial_flow_example.h"
#include "serial_ddma_flow_example.h"
#endif
/* usage info function for serial example */
static void FSerialExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("uart poll_example\r\n");
    printf("-- use poll mode to communicate.\r\n");
    printf("uart intr_example\r\n");
    printf("-- use intr mode to communicate.\r\n");
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD) || defined(CONFIG_FIREFLY_DEMO_BOARD)
    printf("uart mio_example\r\n");
    printf("-- use mio to communicate.\r\n");
    printf("uart ddma_example\r\n");
    printf("-- use ddma to communicate.\r\n");
    printf("uart ddma_mio_example\r\n");
    printf("-- use mio and ddma to communicate.\r\n");
#endif
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD)
    printf("uart flow_example\r\n");
    printf("-- use flow mode to communicate.\r\n");
    printf("uart ddma_flow_example\r\n");
    printf("-- use ddma in flow mode to communicate.\r\n");
#endif
}

/* entry function for serial example */
static int FSerialExampleEntry(int argc, char *argv[])
{
    int ret = 0;
    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FSerialExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "poll_example"))
    {
        ret = FSerialPollExample();
    }
    else if (!strcmp(argv[1], "intr_example"))
    {
        ret = FSerialIntrExample();  
    }
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD) || defined(CONFIG_FIREFLY_DEMO_BOARD)
    else if (!strcmp(argv[1], "mio_example"))
    {
        ret = FSerialMioExample();
    }
    else if (!strcmp(argv[1], "ddma_example"))
    {
        ret = FSerialDdmaExample();
    }
    else if (!strcmp(argv[1], "ddma_mio_example"))
    {
        ret = FSerialDdmaMioExample();
    }
#endif
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD)
    else if (!strcmp(argv[1], "flow_example"))
    {
        ret = FSerialFlowExample();
    }
    else if (!strcmp(argv[1], "ddma_flow_example"))
    {
        ret = FSerialDdmaFlowExample();
    }
#endif
    return ret;
}

/* register command for serial example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), uart, FSerialExampleEntry, serial example);
#endif