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
 * FilePath: cmd_pwm.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for pwm cmd catalogue
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/3/14   first release
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

#include "pwm_dead_band_example.h"
#include "pwm_dual_channel_example.h"

/* usage info function for pwm example */
static void FPwmExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("pwm dead_band\r\n");
    printf("-- run pwm dead band example at controller \r\n");
    printf("pwm dual_channel\r\n");
    printf("-- run pwm dual channel at controller \r\n");
}

/* entry function for pwm example */
static int FPwmExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FPwmExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "dead_band"))
    {
        ret = FPwmDeadBandExample();
    }

    if (!strcmp(argv[1], "dual_channel"))
    {
        ret = FPwmDualChannelExample();
    }

    return ret;
}

/* register command for pwm example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), pwm, FPwmExampleEntry, pwm example);
#endif