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
 * FilePath: gdma_memcpy.c
 * Date: 2022-07-20 11:07:42
 * LastEditTime: 2022-07-20 11:16:57
 * Description:  This files is for GDMA task implementations 
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0    zhugengyu     2022/7/27     init commit
 *  2.0    liqiaozhong   2023/11/10    synchronous update with standalone sdk
 */
/***************************** Include Files *********************************/
#include <string.h>
#include <stdbool.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "fkernel.h"
#include "fcache.h"
#include "fassert.h"
#include "fdebug.h"
#include "fio_mux.h"

#include "fgdma_os.h"
/************************** Constant Definitions *****************************/
#define FGDMA_CONTROLLER_ID  FGDMA0_ID
#define GDMA_CHAN_TRANS_END(chan)           (0x1U << (chan))
#define GDMA_TASKA_CHANNEL_ID               0U
#define GDMA_TASKB_CHANNEL_ID               1U
#define GDMA_TASKA_TRANS_LEN                256U
#define GDMA_TASKB_TRANS_LEN                1024U
#define GDMA_WORK_TASK_NUM                  2U
#define GDMA_TRANS_TIMES                    3U
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FFreeRTOSGdma *gdma_instance_p = NULL;
static FFreeRTOSGdmaChanCfg os_channel_config_taska;
static FFreeRTOSGdmaChanCfg os_channel_config_taskb;
static TaskHandle_t task_a = NULL;
static TaskHandle_t task_b = NULL;
static TimerHandle_t exit_timer = NULL;
static EventGroupHandle_t chan_evt = NULL;
static xSemaphoreHandle gdma_task_counter = NULL;
static uint8_t src_a[GDMA_TASKA_TRANS_LEN] __attribute__((aligned(16))) = {0U}; /* should be aligned with both read and write burst size, defalut: 16-byte */
static uint8_t dst_a[GDMA_TASKA_TRANS_LEN] __attribute__((aligned(16))) = {0U};
static uint8_t src_b[GDMA_TASKB_TRANS_LEN] __attribute__((aligned(16))) = {0U};
static uint8_t dst_b[GDMA_TASKB_TRANS_LEN] __attribute__((aligned(16))) = {0U};
static uint32_t chan_evt_bits = 0U; /* bits that indicate GDMA_CHAN_TRANS_END */
static bool is_running = FALSE;
/***************** Macros (Inline Functions) Definitions *********************/
#define FGDMA_DEBUG_TAG "GDMA-MEM"
#define FGDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_WARN(format, ...)    FT_DEBUG_PRINT_W(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/

/*****************************************************************************/
static void GdmaMemcpyExitCallback(TimerHandle_t timer)
{
    printf("exiting...\r\n");

    if (task_a) /* stop and delete send task */
    {
        vTaskDelete(task_a);
        task_a = NULL;
    }

    if (task_b) /* stop and delete recv task */
    {
        vTaskDelete(task_b);
        task_a = NULL;
    }

    if (chan_evt)
    {
        vEventGroupDelete(chan_evt);
        chan_evt = NULL;
        chan_evt_bits = 0U;
    }

    if (gdma_task_counter)
    {
        vSemaphoreDelete(gdma_task_counter);
        gdma_task_counter = NULL;
    }

    if (gdma_instance_p)
    {
        if (FFREERTOS_GDMA_OK != FFreeRTOSGdmaDeInit(gdma_instance_p))
        {
            FGDMA_ERROR("Deinit GDMA instance failed.");
        }
        gdma_instance_p = NULL;
    }

    if (exit_timer)
    {
        if (pdPASS != xTimerDelete(exit_timer, 0))
        {
            FGDMA_ERROR("Delete exit timer failed.");
        }
        exit_timer = NULL;
    }

    is_running = FALSE;
}

static void GdmaMemcpyAckChanXEnd(uint32_t channel_id, void *args)
{
    FASSERT(channel_id < FFREERTOS_GDMA_NUM_OF_CHAN);

    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;

    FGDMA_INFO("FreeRTOS ack: GDMA chan-%d transfer end.", channel_id);
    x_result = xEventGroupSetBitsFromISR(chan_evt, GDMA_CHAN_TRANS_END(channel_id), &xhigher_priority_task_woken);
    
    if (x_result == pdFALSE)
    {
        FGDMA_ERROR("xEventGroupSetBitsFromISR() fail.");
    }

    portYIELD_FROM_ISR(xhigher_priority_task_woken);
    
    return;
}

static unsigned long GdmaMemcpyWaitChanXEnd(uint32_t channel_id)
{
    FASSERT(channel_id < FFREERTOS_GDMA_NUM_OF_CHAN);

    EventBits_t evt_result;

    /* block task to wait memcpy finish signal */
    evt_result = xEventGroupWaitBits(chan_evt, GDMA_CHAN_TRANS_END(channel_id),
                                     pdTRUE, pdTRUE, pdMS_TO_TICKS(5000UL)); /* wait for channel end event bit(5s) */
    if ((evt_result & GDMA_CHAN_TRANS_END(channel_id))) /* wait until channel finished memcpy */
    {
        FGDMA_INFO("GDMA memcpy finished. Channel bits: 0x%x.", chan_evt_bits);
        return TRUE;
    }
    else
    {
        FGDMA_ERROR("Wait GDMA memcpy timeout. Channel bits: 0x%x, correct value: 0x%x.", evt_result, chan_evt_bits);
        return FALSE;
    }

    return TRUE;
}

static void GdmaInitTask(void *args)
{
    gdma_instance_p = FFreeRTOSGdmaInit(FGDMA_CONTROLLER_ID);
    FASSERT_MSG(gdma_instance_p, "Init gdma controller failed.");

    FASSERT_MSG(gdma_task_counter, "GDMA task counter does not exist.");
    for (size_t loop = 0; loop < GDMA_WORK_TASK_NUM; loop++)
    {
        xSemaphoreGive(gdma_task_counter);
    }

    vTaskDelete(NULL);
}

static void GdmaMemcpyTaskA(void *args)
{
    FASSERT_MSG(gdma_task_counter, "GDMA task counter does not exist.");
    
    xSemaphoreTake(gdma_task_counter, portMAX_DELAY);

    char ch = 'A';
    uint8_t times = 0U;
    FError err = FFREERTOS_GDMA_OK;

    for (;;)
    {
        /* os channel config set */
        os_channel_config_taska.trans_mode = FFREERTOS_GDMA_OPER_DIRECT;
        os_channel_config_taska.src_addr = (uintptr_t)src_a;
        os_channel_config_taska.dst_addr = (uintptr_t)dst_a;
        os_channel_config_taska.trans_length = GDMA_TASKA_TRANS_LEN;

        err = FFreeRTOSGdmaChanConfigure(gdma_instance_p, GDMA_TASKA_CHANNEL_ID, &os_channel_config_taska);
        if (FFREERTOS_GDMA_OK != err)
        {
            FGDMA_ERROR("FFreeRTOSGdmaChanConfigure in channel-%d failed.", GDMA_TASKA_CHANNEL_ID);
            goto task_err;
        }

        FFreeRTOSGdmaChanRegisterEvtHandler(gdma_instance_p, 
                                            GDMA_TASKA_CHANNEL_ID, 
                                            FFREERTOS_GDMA_CHAN_EVT_TRANS_END,
                                            GdmaMemcpyAckChanXEnd,
                                            NULL);

        ch = (char)('A' + (times) % 10); /* send different content each time */

        memset((void *)src_a, ch, GDMA_TASKA_TRANS_LEN);
        memset((void *)dst_a, 0, GDMA_TASKA_TRANS_LEN);

        /* Memory barrier operation */
        FCacheDCacheInvalidateRange((uintptr_t)src_a, GDMA_TASKA_TRANS_LEN); 
        FCacheDCacheInvalidateRange((uintptr_t)dst_a, GDMA_TASKA_TRANS_LEN);

        FGDMA_INFO("[Task-A]start GDMA memcpy data ...");
        FFreeRTOSGdmaChanStart(gdma_instance_p, GDMA_TASKA_CHANNEL_ID);

        /* recv task has high priority, send task will not run before recv task blocked */
        if (!GdmaMemcpyWaitChanXEnd(GDMA_TASKA_CHANNEL_ID))
        {
            goto task_err;
        }

        FCacheDCacheInvalidateRange((uintptr)src_a, GDMA_TASKA_TRANS_LEN);
        FCacheDCacheInvalidateRange((uintptr)dst_a, GDMA_TASKA_TRANS_LEN);

        /* compare if memcpy success */
        if (0 == memcmp(src_a, dst_a, GDMA_TASKA_TRANS_LEN))
        {
            taskENTER_CRITICAL();
            printf("\r\n[Task-A]GDMA memcpy success.\r\n");
            printf("[Task-A]src buf...\r\n");
            FtDumpHexByte((const uint8_t *)src_a, min((size_t)GDMA_TASKA_TRANS_LEN, (size_t)64U));
            printf("[Task-A]dst buf...\r\n");
            FtDumpHexByte((const uint8_t *)dst_a, min((size_t)GDMA_TASKA_TRANS_LEN, (size_t)64U));
            taskEXIT_CRITICAL();
        }
        else
        {
            FGDMA_ERROR("[Task-A]src != dst, GDMA memcpy failed.");
            goto task_err;
        }

        err = FFreeRTOSGdmaChanStop(gdma_instance_p, GDMA_TASKA_CHANNEL_ID);
        if (FFREERTOS_GDMA_OK != err)
        {
            FGDMA_ERROR("FFreeRTOSGdmaChanStop in channel-%d failed.", GDMA_TASKA_CHANNEL_ID);
            goto task_err;
        }
        
        err = FFreeRTOSGdmaChanDeconfigure(gdma_instance_p, GDMA_TASKA_CHANNEL_ID);
        if (FFREERTOS_GDMA_OK != err)
        {
            FGDMA_ERROR("FFreeRTOSGdmaChanDeconfigure in channel-%d failed.", GDMA_TASKA_CHANNEL_ID);
            goto task_err;
        }

        if (times++ > GDMA_TRANS_TIMES)
        {
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(2000U)); /* wait for 2 seconds */
    }

task_err:
    (void)FFreeRTOSGdmaChanDeconfigure(gdma_instance_p, GDMA_TASKA_CHANNEL_ID);
    printf("[Task-A]exit from GDMA memcpy task.\r\n");
    vTaskSuspend(NULL); /* failed, not able to run, suspend task itself */
}

static void GdmaMemcpyTaskB(void *args)
{
    FASSERT_MSG(gdma_task_counter, "GDMA task counter does not exist.");
    
    xSemaphoreTake(gdma_task_counter, portMAX_DELAY);

    char ch = '0';
    uint8_t times = 0U;
    FError err = FFREERTOS_GDMA_OK;

    for (;;)
    {
        /* os channel config set */
        os_channel_config_taskb.trans_mode = FFREERTOS_GDMA_OPER_BDL;
        os_channel_config_taskb.src_addr = (uintptr_t)src_b;
        os_channel_config_taskb.dst_addr = (uintptr_t)dst_b;
        os_channel_config_taskb.trans_length = GDMA_TASKB_TRANS_LEN;

        err = FFreeRTOSGdmaChanConfigure(gdma_instance_p, GDMA_TASKB_CHANNEL_ID, &os_channel_config_taskb);
        if (FFREERTOS_GDMA_OK != err)
        {
            FGDMA_ERROR("FFreeRTOSGdmaChanConfigure in channel-%d failed.", GDMA_TASKB_CHANNEL_ID);
            goto task_err;
        }

        FFreeRTOSGdmaChanRegisterEvtHandler(gdma_instance_p, 
                                            GDMA_TASKB_CHANNEL_ID, 
                                            FFREERTOS_GDMA_CHAN_EVT_TRANS_END,
                                            GdmaMemcpyAckChanXEnd,
                                            NULL);

        ch = (char)('0' + (times) % 10); /* send different content each time */

        memset((void *)src_b, ch, GDMA_TASKB_TRANS_LEN);
        memset((void *)dst_b, 0, GDMA_TASKB_TRANS_LEN);

        /* Memory barrier operation */
        FCacheDCacheInvalidateRange((uintptr_t)src_b, GDMA_TASKB_TRANS_LEN); 
        FCacheDCacheInvalidateRange((uintptr_t)dst_b, GDMA_TASKB_TRANS_LEN);

        FGDMA_INFO("[Task-B]start GDMA memcpy data ...");
        FFreeRTOSGdmaChanStart(gdma_instance_p, GDMA_TASKB_CHANNEL_ID);

        /* recv task has high priority, send task will not run before recv task blocked */
        if (!GdmaMemcpyWaitChanXEnd(GDMA_TASKB_CHANNEL_ID))
        {
            goto task_err;
        }

        FCacheDCacheInvalidateRange((uintptr)src_b, GDMA_TASKB_TRANS_LEN);
        FCacheDCacheInvalidateRange((uintptr)dst_b, GDMA_TASKB_TRANS_LEN);

        /* compare if memcpy success */
        if (0 == memcmp(src_a, dst_a, GDMA_TASKB_TRANS_LEN))
        {
            taskENTER_CRITICAL();
            printf("\r\n[Task-B]GDMA memcpy success.\r\n");
            printf("[Task-B]src buf...\r\n");
            FtDumpHexByte((const uint8_t *)src_b, min((size_t)GDMA_TASKB_TRANS_LEN, (size_t)64U));
            printf("[Task-B]dst buf...\r\n");
            FtDumpHexByte((const uint8_t *)dst_b, min((size_t)GDMA_TASKB_TRANS_LEN, (size_t)64U));
            taskEXIT_CRITICAL();
        }
        else
        {
            FGDMA_ERROR("[TaskB]src != dst, GDMA memcpy failed.");
            goto task_err;
        }

        err = FFreeRTOSGdmaChanStop(gdma_instance_p, GDMA_TASKB_CHANNEL_ID);
        if (FFREERTOS_GDMA_OK != err)
        {
            FGDMA_ERROR("FFreeRTOSGdmaChanStop in channel-%d failed.", GDMA_TASKB_CHANNEL_ID);
            goto task_err;
        }
        
        err = FFreeRTOSGdmaChanDeconfigure(gdma_instance_p, GDMA_TASKB_CHANNEL_ID);
        if (FFREERTOS_GDMA_OK != err)
        {
            FGDMA_ERROR("FFreeRTOSGdmaChanDeconfigure in channel-%d failed.", GDMA_TASKB_CHANNEL_ID);
            goto task_err;
        }

        if (times++ > GDMA_TRANS_TIMES)
        {
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(2000U)); /* wait for 2 seconds */
    }

task_err:
    (void)FFreeRTOSGdmaChanDeconfigure(gdma_instance_p, GDMA_TASKB_CHANNEL_ID);
    printf("[Task-B]exit from GDMA memcpy task.\r\n");
    vTaskSuspend(NULL); /* failed, not able to run, suspend task itself */
}

BaseType_t FFreeRTOSRunGdmaMemcpy(void)
{
    BaseType_t ret = pdPASS;
    const TickType_t total_run_time = pdMS_TO_TICKS(30000UL); /* loopback run for 30 secs deadline */

    if (is_running)
    {
        FGDMA_ERROR("task is running, please wait for the program to complete.");
        return pdPASS;
    }

    is_running = TRUE;

    FASSERT_MSG(NULL == chan_evt, "Event group has been created aready.");
    FASSERT_MSG((chan_evt = xEventGroupCreate()) != NULL, "Create event group failed.");

    FASSERT_MSG(NULL == gdma_task_counter, "GDMA task counter has been created aready.");
    FASSERT_MSG((gdma_task_counter = xSemaphoreCreateCounting(GDMA_WORK_TASK_NUM, 0U)) != NULL, "create event group failed !!!");

    taskENTER_CRITICAL(); /* no schedule when create task */

    ret = xTaskCreate((TaskFunction_t)GdmaInitTask,  /* task entry */
                      (const char *)"GdmaInitTask",/* task name */
                      (uint16_t)1024,  /* task stack size in words */
                      NULL, /* task params */
                      (UBaseType_t)configMAX_PRIORITIES - 1,  /* task priority */
                      NULL); /* task handler */

    FASSERT_MSG(pdPASS == ret, "create task failed");

    ret = xTaskCreate((TaskFunction_t)GdmaMemcpyTaskA,  /* task entry */
                      (const char *)"GdmaMemcpyTaskA",/* task name */
                      (uint16_t)4096,  /* task stack size in words */
                      NULL, /* task params */
                      (UBaseType_t)configMAX_PRIORITIES - 1,  /* task priority */
                      (TaskHandle_t *)&task_a); /* task handler */

    FASSERT_MSG(pdPASS == ret, "create task failed");

    ret = xTaskCreate((TaskFunction_t)GdmaMemcpyTaskB,  /* task entry */
                      (const char *)"GdmaMemcpyTaskB",/* task name */
                      (uint16_t)4096,  /* task stack size in words */
                      NULL, /* task params */
                      (UBaseType_t)configMAX_PRIORITIES - 2,  /* task priority */
                      (TaskHandle_t *)&task_b); /* task handler */

    FASSERT_MSG(pdPASS == ret, "create task failed");

    exit_timer = xTimerCreate("Exit-Timer",                 /* Text name for the software timer - not used by FreeRTOS. */
                              total_run_time,                 /* The software timer's period in ticks. */
                              pdFALSE,                        /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                              NULL,                           /* use timer id to pass task data for reference. */
                              GdmaMemcpyExitCallback);        /* The callback function to be used by the software timer being created. */

    FASSERT_MSG(NULL != exit_timer, "create exit timer failed");

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    ret = xTimerStart(exit_timer, 0); /* start */

    FASSERT_MSG(pdPASS == ret, "start exit timer failed");

    return ret;
}