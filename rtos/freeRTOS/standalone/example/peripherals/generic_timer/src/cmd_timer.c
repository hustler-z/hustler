/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: cmd_timer.c
 * Date: 2022-04-11 16:05:49
 * LastEditTime: 2022-04-11 16:05:49
 * Description:  This file is for the generic timer test command functions
 *
 * Modify History:
 *  Ver   Who        Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2023/5/26  first release
 */

#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "ftypes.h"
#include "fgeneric_timer.h"
#include "fdebug.h"
#include "physical_counter.h"
#include "virtual_counter.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"

static void FGenericCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("    gtimer start p \r\n");
    printf("        -- start generic timer physical counter \r\n");
    printf("    gtimer stop p \r\n");
    printf("        -- stop generic timer physical counter \r\n");
    printf("    gtimer start v \r\n");
    printf("        -- start generic timer virtual counter \r\n");
    printf("    gtimer stop v \r\n");
    printf("        -- stop generic timer virtual counter \r\n");

}

static int FGenericCmdEntry(int argc, char *argv[])
{

    u32 times = 0;
    if (argc < 3)
    {
        FGenericCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "start"))
    {
        if (!strcmp(argv[2], "p"))
        {
            FGenericPhysicalTimerStart();
        }
        else if (!strcmp(argv[2], "v"))
        {
            FGenericVirtualTimerStart();
        }    
       
    }
    else if (!strcmp(argv[1], "stop"))
    {
        if (!strcmp(argv[2], "p"))
        {
            FGenericPhysicalTimerStop();
        }
        else if (!strcmp(argv[2], "v"))
        {
            FGenericVirtualTimerStop();
        }    
    }
  
    return 0;
}


SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), gtimer, FGenericCmdEntry, test generic timer);

#endif