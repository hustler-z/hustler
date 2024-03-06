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
 * FilePath: cmd_adc.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for adc example function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/2/17   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "adc_threshold_exceeds_warning_example.h"
#include "adc_intr_get_voltage_example.h"

#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"


/* usage info function for adc example */
static void FAdcExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("adc intr_read [adc_id]\r\n");
    printf("-- run adc intr mode get vol example at controller [id]\r\n");
    printf("adc th_test [adc_id]\r\n");
    printf("-- run adc threshold exceeds warning example controller [id]\r\n");
}

/* entry function for adc example */
static int FAdcExampleEntry(int argc, char *argv[])
{
    int ret = 0;
    u32 id; 
    /* check input args of example, exit if invaild */
    if (argc < 3)
    {
        FAdcExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "intr_read"))
    {
        id = (u32)simple_strtoul(argv[2], NULL, 10);
        /*run the adc intr mode get vol example*/
        ret = FAdcIntrReadExample(id);
    }
    else if (!strcmp(argv[1], "th_test"))
    {
        id = (u32)simple_strtoul(argv[2], NULL, 10);
        /*run the adc threshold exceeds warning example*/
        ret = FAdcThExceedsWarnExample(id);    
    }

    return ret;
}

/* register command for adc example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), adc, FAdcExampleEntry, adc example);
#endif