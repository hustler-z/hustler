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
 * LastEditTime: 2022-02-17 17:38:03
 * Description:  This file is for creating a main loop from which the programme will start.
 *
 * Modify History:
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0  liuzhihong    2022/11/16            first release
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
extern void TimerLoop(void);
int main()
{
    TimerLoop();
    return 0;
}