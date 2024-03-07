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
 * FilePath: pmu_cmd.c
 * Created Date: 2023-11-01 19:48:07
 * Last Modified: 2023-11-15 09:12:24
 * Description:  This file is for command of pmu
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0    huanghe     2023-11-10        first version
 */
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

extern void FPmuEventIdExample(void)  ;
extern void FPmuCycleCntExample(void)  ;

/* usage info function for pmu example */
static void FPmuExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("pmu event\r\n");
    printf("-- run pmu event example \r\n");
    printf("pmu cycle\r\n");
    printf("-- run pmu cycle example \r\n");
    
}


/* entry function for example */
static int FPmuExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        FPmuExampleUsage();
        return -1;
    }
    
    if (!strcmp(argv[1], "event"))
    {
        FPmuEventIdExample();
    }
    else if(!strcmp(argv[1], "cycle"))
    {
        FPmuCycleCntExample() ;
    }

    return ret;
}

/* register command for pmu example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), pmu, FPmuExampleEntry, pmu example);
#endif