/*
 * @Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * @FilePath: cmd_i2c.c
 * @Date: 2023-07-12 16:37:01
 * @LastEditTime: 2023-07-12 16:37:01
 * @Description:  This file is for i2c example cmd catalogue
 * 
 * @Modify History: 
 *  Ver     Who     Date        Changes
 * -----    ------  --------    --------------------------------------
 * 1.0      liusm   2023/7/12   first release
 */
#include "sdkconfig.h"
#include "ferror_code.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"
#include <string.h>
#include "i2c_ds1339_rtc_example.h"
#include "i2c_master_slave_example.h"

/* usage info function for i2c example */
static void FI2cExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("i2c rtc_example\r\n");
    printf("-- use rtc example.\r\n");
#if defined(CONFIG_FIREFLY_DEMO_BOARD)
    printf("i2c ms_example\r\n");
    printf("-- use master and slave communicate example.\r\n");
#endif
}

/* entry function for i2c example */
static int FI2cCmdEntry(int argc, char *argv[])
{
    FError ret = 0;
    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FI2cExampleUsage();
        return -1;
    }
    /* parser example input args and run example */
    else if (!strcmp(argv[1], "rtc_example"))
    {
        ret = FI2cRtcExample();
        if (ret != FT_SUCCESS)
        {
            printf("FSerialPollInit error :0x%x!\n",ret);
            return ret;
        }
    }
#if defined(CONFIG_FIREFLY_DEMO_BOARD)
    else if (!strcmp(argv[1], "ms_example"))
    {
        ret = FI2cMasterSlaveExample();
        if (ret != FT_SUCCESS)
        {
            printf("FI2cMasterSlaveExample error :0x%x!\n",ret);
            return ret;
        }
    }
#endif
    return ret;
}

/* register command for i2c example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), i2c, FI2cCmdEntry, i2c example);
#endif

