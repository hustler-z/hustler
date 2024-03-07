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
 * FilePath: qspi_dual_flash_stack_example.c
 * Date: 2023-11-20 11:32:48
 * LastEditTime: 2023-11-20 11:32:48
 * Description:  This file is for qspi dual flash stack example function implmentation
 *
 * Modify History:
 *  Ver   Who           Date           Changes
 * ----- ------       --------      --------------------------------------
 * 1.0   huangjin     2023/11/20    first release
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

/* write and read task delay in milliseconds */
#define TASK_DELAY_MS   1000UL

static xTaskHandle read_handle;
static xTaskHandle write_handle;

/* Offset 1M from flash maximum capacity*/
#define FLASH_WR_OFFSET SZ_1M 
/* write and read start address */
static u32 flash_wr_start = 0 ; 

/* write and read cs channel */
#define QSPI_CS_CHANNEL 0

#define DAT_LENGTH  64
static u8 rd_buf[DAT_LENGTH] = {0};
static u8 wr_buf[DAT_LENGTH] = {0};

/* test task number */
#define READ_WRITE_TASK_NUM 3
static xSemaphoreHandle xCountingSemaphore;
static xSemaphoreHandle xCtrlSemaphore;

static FFreeRTOSQspi *os_qspi_ctrl_p = NULL;

static FFreeRTOSQspiMessage message = {0};

static void FFreeRTOSQspiDelete(void *pvParameters);
static void QspiReadTask(void *pvParameters);
static void QspiWriteTask(void *pvParameters);

static void QspiInitTask(void *pvParameters)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    for (int i = 0; i < READ_WRITE_TASK_NUM; i++)
    {
        xSemaphoreGive(xCountingSemaphore);
    }

    xReturn = xTaskCreate((TaskFunction_t)QspiWriteTask,  /* 任务入口函数 */
                        (const char *)"QspiWriteTask",/* 任务名字 */
                        (uint16_t)1024,  /* 任务栈大小 */
                        (void *)pvParameters,/* 任务入口函数参数 */
                        (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                        (TaskHandle_t *)&write_handle); /* 任务控制 */
    
    xReturn = xTaskCreate((TaskFunction_t)QspiReadTask,  /* 任务入口函数 */
                          (const char *)"QspiReadTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)pvParameters,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                          (TaskHandle_t *)&read_handle); /* 任务控制 */

    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSQspiDelete,  /* 任务入口函数 */
                          (const char *)"QspiDeleteTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)pvParameters,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                          NULL); /* 任务控制 */
    if (xReturn != pdPASS)
    {
        goto qspi_init_exit;
    }

qspi_init_exit:
    vTaskDelete(NULL);
}

static void QspiReadTask(void *pvParameters)
{
    int channel = (int)(uintptr)pvParameters;
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    FError ret = FQSPI_SUCCESS;

    xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* Print out the name of this task. */
        printf("CSN%d QspiReadTask is running\r\n", channel);

        message.read_buf = rd_buf;
        message.length = DAT_LENGTH;
        message.addr = flash_wr_start;
        message.cmd = FQSPI_FLASH_CMD_READ;
        message.cs = channel;
        ret = FFreeRTOSQspiTransfer(os_qspi_ctrl_p, &message);
        if (FQSPI_SUCCESS != ret)
        {
            printf("QspiReadTask FFreeRTOSQspiTransfer failed, return value: 0x%x\r\n", ret);
        }
        taskENTER_CRITICAL(); //进入临界区
        FtDumpHexByte(rd_buf, DAT_LENGTH);
        taskEXIT_CRITICAL(); //退出临界区

        /* Delay for a period.  This time a call to vTaskDelay() is used which
        places the task into the Blocked state until the delay period has
        expired.  The parameter takes a time specified in 'ticks', and the
        pdMS_TO_TICKS() macro is used (where the xDelay constant is
        declared) to convert TASK_DELAY_MS milliseconds into an equivalent time in
        ticks. */
        vTaskDelay(xDelay);
    }
}

static void QspiWriteTask(void *pvParameters)
{
    char *pcTaskName;
    int channel = (int)(uintptr)pvParameters;
    if (channel == 0)
    {
        pcTaskName = "CSN0 write content successfully";
    }
    else if (channel == 1)
    {
        pcTaskName = "CSN1 write content successfully";
    }

    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    int i = 0;
    FError ret = FQSPI_SUCCESS;

    xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* Print out the name of this task. */
        printf("CSN%d QspiWriteTask is running\r\n", channel);
        
        /*set the write buffer content*/
        u8 len = strlen(pcTaskName) + 1;
        memcpy(&wr_buf, pcTaskName, len);

        message.addr = flash_wr_start;
        message.cmd = FQSPI_FLASH_CMD_SE;
        message.cs = channel;
        ret = FFreeRTOSQspiTransfer(os_qspi_ctrl_p, &message);
        if (FQSPI_SUCCESS != ret)
        {
            printf("QspiWriteTask FFreeRTOSQspiTransfer failed, return value: 0x%x\r\n", ret);
        }

        message.write_buf = wr_buf;
        message.length = DAT_LENGTH;
        message.addr = flash_wr_start;
        message.cmd = FQSPI_FLASH_CMD_PP;
        message.cs = channel;
        ret = FFreeRTOSQspiTransfer(os_qspi_ctrl_p, &message);
        if (FQSPI_SUCCESS != ret)
        {
            printf("QspiWriteTask FFreeRTOSQspiTransfer failed, return value: 0x%x\r\n", ret);
        }

        /* Delay for a period.  This time a call to vTaskDelay() is used which
        places the task into the Blocked state until the delay period has
        expired.  The parameter takes a time specified in 'ticks', and the
        pdMS_TO_TICKS() macro is used (where the xDelay constant is
        declared) to convert TASK_DELAY_MS milliseconds into an equivalent time in
        ticks. */
        vTaskDelay(xDelay);
    }
}

BaseType_t FFreeRTOSQspiDualFlashTaskCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    memset(&message, 0, sizeof(message));

    xCountingSemaphore = xSemaphoreCreateCounting(READ_WRITE_TASK_NUM, 0);
    if (xCountingSemaphore == NULL)
    {
        printf("FFreeRTOSWdtCreate xCountingSemaphore create failed.\r\n");
        return pdFAIL;
    }

    xCtrlSemaphore = xSemaphoreCreateBinary();
    if (xCtrlSemaphore != NULL)
    {
        xSemaphoreGive(xCtrlSemaphore);
    }

#if defined(CONFIG_TARGET_E2000)
    /*init iomux*/
    FIOMuxInit();
    FIOPadSetQspiMux(FQSPI0_ID, FQSPI_CS_0);
    FIOPadSetQspiMux(FQSPI0_ID, FQSPI_CS_1);
#endif

    /* init qspi controller */
    os_qspi_ctrl_p = FFreeRTOSQspiInit(FQSPI0_ID);
    flash_wr_start = os_qspi_ctrl_p->qspi_ctrl.flash_size - FLASH_WR_OFFSET;
    if (os_qspi_ctrl_p == NULL)
    {
        printf("FFreeRTOSWdtInit failed.\n");
    }

    taskENTER_CRITICAL(); /*进入临界区*/
    /* qspi 通道0 */
    xSemaphoreTake(xCtrlSemaphore, portMAX_DELAY);
    xReturn = xTaskCreate((TaskFunction_t)QspiInitTask,  /* 任务入口函数 */
                          (const char *)"QspiInitTask0",  /* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)0,  /* 任务入口函数参数 */
                          (UBaseType_t)2,  /* 任务的优先级 */
                          NULL);
    /* qspi 通道1 */
    xSemaphoreTake(xCtrlSemaphore, portMAX_DELAY);
    xReturn = xTaskCreate((TaskFunction_t)QspiInitTask,  /* 任务入口函数 */
                          (const char *)"QspiInitTask1",  /* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)1,  /* 任务入口函数参数 */
                          (UBaseType_t)2,  /* 任务的优先级 */
                          NULL);
    taskEXIT_CRITICAL(); /*退出临界区*/

    return xReturn;
}

static void FFreeRTOSQspiDelete(void *pvParameters)
{
    xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);

    BaseType_t xReturn = pdPASS;

    if (read_handle)
    {
        vTaskDelete(read_handle);
        printf("Delete QspiReadTask successfully.\r\n");
    }

    if (write_handle)
    {
        vTaskDelete(write_handle);
        printf("Delete QspiWriteTask successfully.\r\n");
    }

    xSemaphoreGive(xCtrlSemaphore);

    if ((int)(uintptr)pvParameters == 1)
    {
        /* 删除信号量 */
        vSemaphoreDelete(xCountingSemaphore);
        vSemaphoreDelete(xCtrlSemaphore);
        /* 去初始化 */
        FIOMuxDeInit();
        FFreeRTOSQspiDeinit(os_qspi_ctrl_p);
    }
    
    printf("Delete QspiDeleteTask successfully.\r\n");
    vTaskDelete(NULL);
}
