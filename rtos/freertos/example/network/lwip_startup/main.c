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
 * FilePath: main.c
 * Created Date: 2023-11-20 18:26:15
 * Last Modified: 2023-12-28 15:13:08
 * Description:  This file is for lwip startup example main functions.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong   2023/12/26          first release
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "shell_port.h"
#include "lwip/tcpip.h"
int main()
{
    BaseType_t ret = pdPASS;

    ret = LSUserShellTask();
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }

    tcpip_init(NULL, NULL);
    vTaskStartScheduler(); /* 启动任务，开启调度 */
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("Failed 0x%x.\r\n", ret);

    return 0;
}