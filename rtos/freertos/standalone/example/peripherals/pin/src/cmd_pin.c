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
 * FilePath: cmd_pin.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for the pin cmd catalogue.
 *
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----   -------    --------     --------------------------------------
 *  1.0  liqiaozhong  2023/03/05   first commit
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "ftypes.h"

#include "pin_common.h"
#include "pin_gpio_intr_example.h"
#include "pin_gpio_low_level_example.h"
#include "pin_gpio_pwm_example.h"

/* usage info function for pin example */
static void FPinExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("pin gpio_intr_example\r\n");
    printf("-- run pin gpio interrupt trigger example on defult controller\r\n");
    printf("pin gpio_low_level_example\r\n");
    printf("-- run pin gpio register operation example on defult controller\r\n");
    printf("pin gpio_pwm_example\r\n");
    printf("-- run pin gpio pwm output example on defult controller\r\n");
}

/* entry function for pin example */
static int FPinExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FPinExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "gpio_intr_example"))
    {
        ret = FPinGpioIntrExample();
    }
    else if (!strcmp(argv[1], "gpio_low_level_example"))
    {
        ret = FPinGpioLowLevelExample();    
    }
    else if (!strcmp(argv[1], "gpio_pwm_example"))
    {
        ret = FPinGpioPwmExample();    
    }

    return ret;
}

/* register command for pin example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), pin, FPinExampleEntry, pin example);
#endif