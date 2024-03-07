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
 * FilePath: qspi_flash_indirect_example.c
 * Date: 2023-11-20 11:32:48
 * LastEditTime: 2023-11-20 11:32:48
 * Description:  This file is an example function implementation for the indirect mode of qspi flash
 *
 * Modify History:
 *  Ver   Who           Date           Changes
 * ----- ------       --------      --------------------------------------
 * 1.0  huangjin      2023/11/16    first release
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

static FFreeRTOSQspi *os_qspi_ctrl_p = NULL;

static FFreeRTOSQspiMessage message = {0};

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
    flash_wr_start = os_qspi_ctrl_p->qspi_ctrl.flash_size - FLASH_WR_OFFSET;
    if (os_qspi_ctrl_p == NULL)
    {
        printf("FFreeRTOSWdtInit failed.\n");
        goto qspi_init_exit;
    }

qspi_init_exit:
    vTaskDelete(NULL);
}

static void QspiReadTask(void *pvParameters)
{
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    FError ret = FQSPI_SUCCESS;
    int i = 0;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        if (uxSemaphoreGetCount( xCountingSemaphore ) == READ_WRITE_TASK_NUM)
        {
            FFreeRTOSQspiDelete();
            printf("Delete QspiReadTask successfully.\r\n");
            vTaskDelete(NULL);
        }
        /* Print out the name of this task. */
        printf("QspiReadTask is running. count = %d\r\n", uxSemaphoreGetCount( xCountingSemaphore )+1);

        /* Read norflash data */
        ret = FQspiFlashPortReadData(&os_qspi_ctrl_p->qspi_ctrl, FQSPI_FLASH_CMD_READ, flash_wr_start, rd_buf, DAT_LENGTH);
        if (FQSPI_SUCCESS != ret)
        {
            printf("QspiReadTask FFreeRTOSQspiTransfer failed, return value: 0x%x\r\n", ret);
        }
        taskENTER_CRITICAL(); //进入临界区
        FtDumpHexByte(rd_buf, DAT_LENGTH);
        taskEXIT_CRITICAL(); //退出临界区

        /* 判断读写内容是否一致 */
        for (i = 0; i < DAT_LENGTH; i++)
        {
            if (rd_buf[i] != wr_buf[i])
            {
                printf("The read and write data is inconsistent.\r\n");
                break;
            }
        }
        xSemaphoreGive(xCountingSemaphore);

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
    const char *pcTaskName = "QspiWriteTask is running\r\n";
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    int i = 0;
    FError ret = FQSPI_SUCCESS;
    u32 array_index = 0;
    u32 write_addr = 0;
    
    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        if (uxSemaphoreGetCount( xCountingSemaphore ) == READ_WRITE_TASK_NUM)
        {
            printf("Delete QspiWriteTask successfully.\r\n");
            vTaskDelete(NULL);
        }
        /* Print out the name of this task. */
        printf(pcTaskName);
        for (i = 0; i < DAT_LENGTH; i++)
        {
            wr_buf[i] = uxSemaphoreGetCount( xCountingSemaphore ) + i;
        }

        message.addr = flash_wr_start;
        message.cmd = FQSPI_FLASH_CMD_SE;
        message.cs = QSPI_CS_CHANNEL;
        ret = FFreeRTOSQspiTransfer(os_qspi_ctrl_p, &message);
        if (FQSPI_SUCCESS != ret)
        {
            printf("Failed to erase sectors. return value: 0x%x\r\n", ret);
        }

        write_addr = os_qspi_ctrl_p->qspi_ctrl.flash_size - FLASH_WR_OFFSET;
        /* Write norflash data */
        while (array_index < sizeof(wr_buf))
        {
            u8 data_to_write[4] = {0};
            for (i = 0; i < 4; i++)
            {
                if (array_index < sizeof(wr_buf))
                {
                    data_to_write[i] = wr_buf[array_index];
                    array_index++;
                }
                else
                {
                    break;
                }
            }
            ret = FQspiFlashPortWriteData(&os_qspi_ctrl_p->qspi_ctrl, FQSPI_FLASH_CMD_PP, write_addr, (u8 *)(data_to_write), 4);
            write_addr += 4;
        }
        array_index = 0;

        /* Delay for a period.  This time a call to vTaskDelay() is used which
        places the task into the Blocked state until the delay period has
        expired.  The parameter takes a time specified in 'ticks', and the
        pdMS_TO_TICKS() macro is used (where the xDelay constant is
        declared) to convert TASK_DELAY_MS milliseconds into an equivalent time in
        ticks. */
        vTaskDelay(xDelay);
    }
}

BaseType_t FFreeRTOSQspiIndirectTaskCreate(u32 id)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    memset(&message, 0, sizeof(message));

    xCountingSemaphore = xSemaphoreCreateCounting(READ_WRITE_TASK_NUM, 0);
    if (xCountingSemaphore == NULL)
    {
        printf("FFreeRTOSWdtCreate xCountingSemaphore create failed.\r\n");
        return pdFAIL;
    }

    taskENTER_CRITICAL(); /*进入临界区*/

    xReturn = xTaskCreate((TaskFunction_t)QspiInitTask,  /* 任务入口函数 */
                          (const char *)"QspiInitTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)(uintptr)id,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1,  /* 任务的优先级 */
                          NULL);

    xReturn = xTaskCreate((TaskFunction_t)QspiWriteTask,  /* 任务入口函数 */
                        (const char *)"QspiWriteTask",/* 任务名字 */
                        (uint16_t)1024,  /* 任务栈大小 */
                        NULL,/* 任务入口函数参数 */
                        (UBaseType_t)configMAX_PRIORITIES - 2, /* 任务的优先级 */
                        (TaskHandle_t *)&write_handle); /* 任务控制 */

    xReturn = xTaskCreate((TaskFunction_t)QspiReadTask,  /* 任务入口函数 */
                          (const char *)"QspiReadTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 3, /* 任务的优先级 */
                          (TaskHandle_t *)&read_handle); /* 任务控制 */

    taskEXIT_CRITICAL(); /*退出临界区*/

    return xReturn;
}

static void FFreeRTOSQspiDelete(void)
{
    FIOMuxDeInit();

    FFreeRTOSQspiDeinit(os_qspi_ctrl_p);

    /* delete count sem */
    vSemaphoreDelete(xCountingSemaphore);
}



