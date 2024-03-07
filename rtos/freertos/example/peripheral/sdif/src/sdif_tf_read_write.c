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
 * FilePath: sd_read_write.c
 * Date: 2022-07-25 15:58:24
 * LastEditTime: 2022-07-25 15:58:25
 * Description:   This file is for providing functions used in cmd_sd.c file.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    first commit
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "fassert.h"
#include "fdebug.h"
#include "fsleep.h"
#include "fkernel.h"
#include "fcache.h"
#include "fio.h"

#include "fsdif_timing.h"
#include "fsl_sdmmc.h"
#include "sdif_read_write.h"
/************************** Constant Definitions *****************************/
#define SD_EVT_INIT_DONE    (0x1 << 0)
#define SD_EVT_WRITE_DONE   (0x1 << 1)
#define SD_EVT_READ_DONE    (0x1 << 2)

enum
{
    FSD_EXAMPLE_OK = 0,
    FSD_EXAMPLE_NOT_YET_INIT,
    FSD_EXAMPLE_INIT_FAILED,
    FSD_EXAMPLE_INVALID_PARAM,
    FSD_EXAMPLE_READ_WRITE_FAILED,
};
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static sdmmchost_config_t s_inst_config;
static sdmmc_sd_t s_inst;  

static EventGroupHandle_t sync = NULL;
static TaskHandle_t write_task = NULL;
static TimerHandle_t exit_timer = NULL;
static boolean is_running = FALSE;
static u32 run_times = 1U;

uint8_t s_dma_buffer[SZ_1M * 4] = {0};
u8 rw_buf[SD_MAX_RW_BLK * SD_BLOCK_SIZE] __attribute((aligned(SD_BLOCK_SIZE))) = {0};
/***************** Macros (Inline Functions) Definitions *********************/
#define FSD_EXAMPLE_TAG "FSD_EXAMPLE"
#define FSD_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_WARN(format, ...)    FT_DEBUG_PRINT_W(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_INFO(format, ...)    FT_DEBUG_PRINT_I(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/

/*****************************************************************************/
static void TfExitCallback(TimerHandle_t timer)
{
    FError err = FT_SUCCESS;
    printf("Exiting.....\r\n");

    if (write_task)
    {
        vTaskDelete(write_task);
        write_task = NULL;
    }

    if (sync)
    {
        vEventGroupDelete(sync);
        sync = NULL;
    }

    if (pdPASS != xTimerDelete(timer, 0)) /* delete timer ifself */
    {
        FSD_ERROR("Delete exit timer failed.");
        exit_timer = NULL;
    }

    SD_Deinit(&s_inst.card);
    SDMMC_OSADeInit();
    FSdifTimingDeinit();

    is_running = FALSE;
}

static void TfSendEvent(u32 evt_bits)
{
    FASSERT(sync);
    BaseType_t x_result = pdFALSE;

    FSD_DEBUG("Ack evt 0x%x", evt_bits);
    x_result = xEventGroupSetBits(sync, evt_bits);
}

static boolean SDTfWaitEvent(u32 evt_bits, TickType_t wait_delay)
{
    FASSERT(sync);
    EventBits_t ev;
    ev = xEventGroupWaitBits(sync, evt_bits,
                             pdTRUE, pdFALSE, wait_delay); /* wait for cmd/data done */
    if (ev & evt_bits)
    {
        return TRUE;
    }

    return FALSE;
}

static void TfInitTask(void *args)
{
    status_t err = 0;

    err = SD_CfgInitialize(&s_inst, &s_inst_config);
    if (kStatus_Success != err)
    {
        FSD_ERROR("Init SD failed, err = %d !!!", err);
        goto task_exit;
    }

    TfSendEvent(SD_EVT_INIT_DONE);

task_exit:
    vTaskDelete(NULL); /* delete task itself */
}

static void TfWriteReadTask(void *args)
{
    u32 times = 0U;
    const TickType_t wait_delay = pdMS_TO_TICKS(2000UL); /* wait for 2 seconds */
    FError err;
    const uintptr trans_len = SD_USE_BLOCK * SD_BLOCK_SIZE;

    SDTfWaitEvent(SD_EVT_INIT_DONE, portMAX_DELAY);

    for (;;)
    {
        /* do read and write operation */
        {
            printf("TF card init success.\n");

            for (u32 i = 0; i < SD_USE_BLOCK; i++) /* copy string to write buffer as each block */
            {
                memset(rw_buf + i * SD_BLOCK_SIZE, (SD_START_BLOCK + i + 1), SD_BLOCK_SIZE);
            }

            err = SD_WriteBlocks(&s_inst.card, rw_buf, SD_START_BLOCK, SD_USE_BLOCK);
            if (kStatus_Success != err)
            {
                FSD_ERROR("TF card write fail.");
                goto task_exit;
            }

            memset(rw_buf, 0, sizeof(rw_buf));

            err = SD_ReadBlocks(&s_inst.card, rw_buf, SD_START_BLOCK, SD_USE_BLOCK);
            if ((kStatus_Success == err))
            {
                FtDumpHexByte(rw_buf, SD_BLOCK_SIZE * min((u32)2, (u32)SD_USE_BLOCK));
                printf("%s@%d: TF card read and write example success !!! \r\n", __func__, __LINE__);
            }
            else
            {
                FSD_ERROR("TF card read fail.");
                goto task_exit;
            }
        }

        vTaskDelay(wait_delay);

        if (++times > run_times)
        {
            break;
        }
    }

task_exit:
    printf("Exit from write task.\r\n");
    vTaskSuspend(NULL); /* suspend task */
}

BaseType_t FFreeRTOSTfWriteRead(void)
{
    BaseType_t ret = pdPASS;
    const TickType_t total_run_time = pdMS_TO_TICKS(30000UL); /* run for 10 secs deadline */

    if (is_running)
    {
        FSD_ERROR("Task is running.");
        return pdPASS;
    }

    FASSERT_MSG(NULL == sync, "Event group exists.");
    FASSERT_MSG((sync = xEventGroupCreate()) != NULL, "Create event group failed.");

    is_running = TRUE;

    FSdifTimingInit();
    SDMMC_OSAInit();

    memset(&s_inst_config, 0, sizeof(s_inst_config));
    memset(&s_inst, 0, sizeof(s_inst));

    s_inst_config.hostId = FSDIF1_ID;
    s_inst_config.hostType = kSDMMCHOST_TYPE_FSDIF;
    s_inst_config.cardType = kSDMMCHOST_CARD_TYPE_MICRO_SD;
    s_inst_config.enableDMA = SD_WORK_DMA;
    s_inst_config.enableIrq = SD_WORK_IRQ;
    s_inst_config.timeTuner = FSdifGetTimingSetting;
    s_inst_config.endianMode = kSDMMCHOST_EndianModeLittle;
    s_inst_config.maxTransSize = SD_MAX_RW_BLK * SD_BLOCK_SIZE;
    s_inst_config.defBlockSize = SD_BLOCK_SIZE;
    s_inst_config.cardClock = SD_CLOCK_50MHZ;
    s_inst_config.isUHSCard = FALSE;
    
    ret = xTaskCreate((TaskFunction_t)TfInitTask,
                      (const char *)"TfInitTask",
                      (uint16_t)2048,
                      NULL,
                      (UBaseType_t)configMAX_PRIORITIES - 1,
                      NULL);
    FASSERT_MSG(pdPASS == ret, "Create task failed.");

    ret = xTaskCreate((TaskFunction_t)TfWriteReadTask,
                      (const char *)"TfWriteReadTask",
                      (uint16_t)2048,
                      NULL,
                      (UBaseType_t)configMAX_PRIORITIES - 2,
                      &write_task);

    FASSERT_MSG(pdPASS == ret, "Create task failed.");

    exit_timer = xTimerCreate("Exit-Timer",                 /* Text name for the software timer - not used by FreeRTOS. */
                              total_run_time,                 /* The software timer's period in ticks. */
                              pdFALSE,                        /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                              NULL,                           /* use timer id to pass task data for reference. */
                              TfExitCallback);                /* The callback function to be used by the software timer being created. */

    FASSERT_MSG(NULL != exit_timer, "Create exit timer failed.");

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    ret = xTimerStart(exit_timer, 0); /* start */

    FASSERT_MSG(pdPASS == ret, "Start exit timer failed.");

    return ret;
}