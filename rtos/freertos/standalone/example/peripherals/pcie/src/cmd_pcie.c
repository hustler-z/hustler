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
 * FilePath: cmd_pcie.c
 * Date: 2023-05-09 13:26:58
 * LastEditTime: 2023-05-09 17:23:07
 * Description:  This file is for PCIe example cmd catalogue.
 *
 * Modify History:
 *  Ver       Who        Date         Changes
 * -----    ------     --------    --------------------------------------
 * 1.0     huanghe    2023/08/07    first release
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

/* usage info function for PCIe example */


extern int FPcieConfigReadExample(void) ;

static void FPcieExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("pcie enum_test\r\n");
    printf("-- run PCIe probe and enumerate example.\r\n");
}

/* entry function for PCIe example */
static int FPcieExampleEntry(int argc, char *argv[])
{
    int ret = 0;
    int index = 0 ;
    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FPcieExampleUsage();
        return -1;
    }

    if (!strcmp(argv[1], "enum_test"))
    {
        ret = FPcieConfigReadExample() ;
    }
    else
    {
         FPcieExampleUsage();
    }

    return ret;
}

/* register command for PCIe example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), pcie, FPcieExampleEntry, PCIe example);
#endif