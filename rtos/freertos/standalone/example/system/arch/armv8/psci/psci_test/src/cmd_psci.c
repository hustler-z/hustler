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
 * FilePath: smcc_smc.c
 * Created Date: 2023-06-16 13:35:21
 * Last Modified: 2023-06-28 16:42:24
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0    huanghe     2023-06-28        first version
 */
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif
#include "string.h"
#include "stdio.h"

#ifdef CONFIG_USE_LETTER_SHELL

static void FLibmetalCmdUsage(void)
{
    printf("Usage:\r\n") ;
    printf("psci feature\r\n") ;
    printf("-- display psci some features\r\n") ;
    printf("psci amp\r\n") ;
    printf("-- The cpu hotplug function was tested using psci protocol \r\n") ;
}



#include "shell.h"
#include "psci_test.h"
static int FPsciExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FLibmetalCmdUsage() ;
        return -1;
    }

    if (!strcmp(argv[1], "feature"))
    {
        FPsciFeatureTest() ;
    }

    if (!strcmp(argv[1], "amp"))
    {
        FPsciHotplugTest() ;
    }

    return ret ;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), psci, FPsciExampleEntry, psci example);

#endif
