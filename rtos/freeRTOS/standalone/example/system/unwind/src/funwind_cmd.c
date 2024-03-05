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
#include "funwind_example.h"
#include "sdkconfig.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"

static void FUnwindCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("    unwind irq \r\n");
    printf("        -- Use irq exceptions to display unwind properties  \r\n");
    printf("    unwind loop \r\n");
    printf("        -- Use loop functions to display unwind properties \r\n");
    printf("    unwind undef \r\n");
    printf("        -- Use undef instruct to display unwind properties \r\n");
}

static int FUnwindCmdEntry(int argc, char *argv[])
{

    u32 times = 0;
    if (argc < 2)
    {
        FUnwindCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "irq"))
    {
        FUnwindIrqTest() ;
    }
    else if (!strcmp(argv[1], "loop"))
    {
        FUnwindtestA() ;
    }
    else if (!strcmp(argv[1], "undef"))
    {
        FUnwindUndefTest() ;
    }
    else
    {
        FUnwindCmdUsage();
        return -1;
    }
  
    return 0;
}


SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), unwind, FUnwindCmdEntry, test unwind example);

#endif