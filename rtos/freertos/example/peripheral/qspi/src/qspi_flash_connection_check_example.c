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
 * FilePath: qspi_flash_connection_check_example.c
 * Date: 2023-11-16 11:32:48
 * LastEditTime: 2023-11-16 11:32:48
 * Description:  This file is for qspi flash connection check example function implmentation
 *
 * Modify History:
 *  Ver   Who           Date           Changes
 * ----- ------       --------      --------------------------------------
 * 1.0   huangjin     2023/11/16    first release
 */
#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fqspi.h"
#include "fqspi_flash.h"
#include "fqspi_os.h"
#include "timers.h"
#include "qspi_example.h"
#include "sdkconfig.h"
#include "fio_mux.h"

static FFreeRTOSQspi *os_qspi_ctrl_p = NULL;

static void FFreeRTOSQspiDelete(void);

static void QspiInitTask(void *pvParameters)
{
    /* The qspi_id to use is passed in via the parameter.
    Cast this to a qspi_id pointer. */
    u32 qspi_id = (u32)(uintptr)pvParameters;

#if defined(CONFIG_TARGET_E2000)
    /*init iomux*/
    FIOMuxInit();
    FIOPadSetQspiMux(qspi_id, FQSPI_CS_0);
    FIOPadSetQspiMux(qspi_id, FQSPI_CS_1);
#endif

    /* init qspi controller */
    os_qspi_ctrl_p = FFreeRTOSQspiInit(qspi_id);
    if (os_qspi_ctrl_p == NULL)
    {
        printf("FFreeRTOSWdtInit failed.\n");
        goto qspi_init_exit;
    }

    FFreeRTOSQspiDelete();

qspi_init_exit:
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSQspiCheckTaskCreate(u32 id)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    taskENTER_CRITICAL(); /*进入临界区*/

    xReturn = xTaskCreate((TaskFunction_t)QspiInitTask,  /* 任务入口函数 */
                          (const char *)"QspiInitTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)(uintptr)id,/* 任务入口函数参数 */
                          (UBaseType_t)2,  /* 任务的优先级 */
                          NULL);

    taskEXIT_CRITICAL(); /*退出临界区*/

    return xReturn;
}

static void FFreeRTOSQspiDelete(void)
{
    BaseType_t xReturn = pdPASS;

    FIOMuxDeInit();
    
    FFreeRTOSQspiDeinit(os_qspi_ctrl_p);
}



