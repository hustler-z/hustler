/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: main.c
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:41:34
 * Description:  This file is for running shell
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming 2022/02/17    first commit
 */

#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif
#ifndef CONFIG_USE_LETTER_SHELL
    #error "Please include letter shell first!!!"
#endif
#include "shell_port.h"
#include "i2c_ds1339_rtc_example.h"
#include "i2c_master_slave_example.h"

int main()
{
#ifdef CONFIG_USE_LETTER_SHELL
    /* if shell command is enabled, register example entry as shell command */
    LSUserShellLoop();    
#else
    /* if shell command is not enabled, run example one by one */
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD)
    FI2cRtcExample();
#endif
#if defined(CONFIG_FIREFLY_DEMO_BOARD)
    FI2cMasterSlaveExample();
#endif

#endif
    return 0;
}
