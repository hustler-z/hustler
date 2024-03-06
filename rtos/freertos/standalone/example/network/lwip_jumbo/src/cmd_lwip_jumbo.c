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
 * FilePath: cmd_lwip_jumbo.c
 * Created Date: 2023-11-08 16:05:47
 * Last Modified: 2023-11-08 17:32:36
 * Description:  This file is for lwip jumbo example cmd catalogue.
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong   2023/11/8          first release
 */


/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

#include "lwip_jumbo_example.h"

#define EXAMPLE_IDLE 0
#define JUMBO_EXAMPLE_RUNNING 1

static u32 init_flag_mask=EXAMPLE_IDLE;

static void  LwipJumboExampleCheckState(void)
{
    switch(init_flag_mask)
    {
        case JUMBO_EXAMPLE_RUNNING:
            printf("Lwip jumbo example is running, we need to deinitialize it first! \r\n ");
            LwipJumboDeinit();
            init_flag_mask=EXAMPLE_IDLE;
            break;
        default:
            break;
    }
}
/* usage info function for lwip jumbo example */
static void LwipJumboExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("lwip jumbo\r\n");
    printf("-- run lwip ipv4 mode example to initialize mac controller and enable jumbo capability of mac \r\n");
}

/* entry function for lwip jumbo example */
static int LwipJumboExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        LwipJumboExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "jumbo"))
    {
        LwipJumboExampleCheckState();
        ret = LwipJumboCreate();
        init_flag_mask = JUMBO_EXAMPLE_RUNNING;       
    }

    return ret;
}

/* register command for lwip jumbo example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), lwip, LwipJumboExampleEntry, lwip jumbo example);
#endif