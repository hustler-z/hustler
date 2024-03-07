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
 * FilePath: ddma_spi_loopback.c
 * Date: 2022-07-20 09:24:39
 * LastEditTime: 2022-07-20 09:24:39
 * Description:  This file is for DDMA task implementations.
 *
 * Modify History:
 *  Ver    Who          Date         Changes
 * -----  ------       --------     --------------------------------------
 *  1.0   zhugengyu    2022/7/27    init commit
 *  1.1   liqiaozhong  2023/11/10   synchronous update with standalone sdk
 */
/***************************** Include Files *********************************/
#include <string.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "fcache.h"
#include "fassert.h"
#include "fdebug.h"
#include "fio_mux.h"

#include "fspim_os.h"
#include "fddma_os.h"
#include "fspim_hw.h"
/************************** Constant Definitions *****************************/
#define TX_RX_BUF_LEN           128
#define CHAN_REQ_DONE(chan)    (0x1 << chan) /* if signal, chan req finished */
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FFreeRTOSSpim *spim = NULL;
static FFreeRTOSSpimConifg spim_config =
{
    .spi_mode = FFREERTOS_SPIM_MODE_0,
    .en_dma = TRUE,
    .inner_loopback = TRUE
};
static FFreeRTOSSpiMessage spi_msg =
{
    .tx_buf = NULL,
    .tx_len = 0U,
    .rx_buf = NULL,
    .rx_len = 0U
};
static FFreeRTOSDdma *ddma = NULL;
static FFreeRTOSDdmaConfig ddma_config;
static QueueHandle_t sync = NULL;
static u32 spi_instance_id = 0U;
static u32 dma_trans_bytes = 32U;
static FDdmaChanIndex rx_chan_id = FDDMA_CHAN_0;
static FDdmaChanIndex tx_chan_id = FDDMA_CHAN_1;
static FFreeRTOSRequest rx_request = {0};
static FFreeRTOSRequest tx_request = {0};
static EventGroupHandle_t chan_evt = NULL;
static u8 rx_buf[TX_RX_BUF_LEN] __attribute__((aligned(FDDMA_DDR_ADDR_ALIGMENT))) = {0};
static u8 tx_buf[TX_RX_BUF_LEN] __attribute__((aligned(FDDMA_DDR_ADDR_ALIGMENT))) = {0};
static u32 trans_len = 32U;
static TaskHandle_t send_task = NULL;
static TaskHandle_t recv_task = NULL;
static TimerHandle_t exit_timer = NULL;
static u32 loopback_times = 3U;
static boolean is_running = FALSE;
static const u32 spim_rx_slave_id[FSPI_NUM] =
{
    [FSPI0_ID] = FDDMA0_SPIM0_RX_SLAVE_ID,
    [FSPI1_ID] = FDDMA0_SPIM1_RX_SLAVE_ID,
    [FSPI2_ID] = FDDMA0_SPIM2_RX_SLAVE_ID,
    [FSPI3_ID] = FDDMA0_SPIM3_RX_SLAVE_ID
};
static const u32 spim_tx_slave_id[FSPI_NUM] =
{
    [FSPI0_ID] = FDDMA0_SPIM0_TX_SLAVE_ID,
    [FSPI1_ID] = FDDMA0_SPIM1_TX_SLAVE_ID,
    [FSPI2_ID] = FDDMA0_SPIM2_TX_SLAVE_ID,
    [FSPI3_ID] = FDDMA0_SPIM3_TX_SLAVE_ID
};
/***************** Macros (Inline Functions) Definitions *********************/
#define FDDMA_DEBUG_TAG "DDMA-LP"
#define FDDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_WARN(format, ...)    FT_DEBUG_PRINT_W(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/
static void DdmaSpiLoopbackExitCallback(TimerHandle_t timer)
{
    FError err = FT_SUCCESS;
    printf("exiting.....\r\n");

    if (send_task) /* stop and delete send task */
    {
        vTaskDelete(send_task);
        send_task = NULL;
    }

    if (recv_task) /* stop and delete recv task */
    {
        vTaskDelete(recv_task);
        recv_task = NULL;
    }

    if (spim)
    {
        err = FFreeRTOSSpimDeInit(spim);
        spim = NULL;
    }

    if (ddma)
    {
        if (FT_SUCCESS != FFreeRTOSDdmaRevokeChannel(ddma, rx_chan_id))
        {
            FDDMA_ERROR("Delete RX channel failed.");
        }

        if (FT_SUCCESS != FFreeRTOSDdmaRevokeChannel(ddma, tx_chan_id))
        {
            FDDMA_ERROR("Delete TX channel failed.");
        }

        err = FFreeRTOSDdmaDeinit(ddma);
        ddma = NULL;
    }

    if (chan_evt)
    {
        vEventGroupDelete(chan_evt);
        chan_evt = NULL;
    }

    if (sync)
    {
        vQueueDelete(sync);
        sync = NULL;
    }

    if (pdPASS != xTimerDelete(timer, 0)) /* delete timer itself */
    {
        FDDMA_ERROR("Delete exit timer failed.");
        exit_timer = NULL;
    }

    is_running = FALSE;
}

static void DdmaSpiLoopbackAckDMADone(FDdmaChanIrq *const chan_irq_info_p, void *arg)
{
    FASSERT(chan_irq_info_p);

    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;

    FDDMA_INFO("Ack chan-%d %s done for DDMA.", chan_irq_info_p->channel_id,
               (chan_irq_info_p->channel_id == rx_chan_id) ? "RX" : "TX");
    FASSERT_MSG(chan_evt, "RX event group not exists.");

    x_result = xEventGroupSetBitsFromISR(chan_evt,
                                         CHAN_REQ_DONE(chan_irq_info_p->channel_id),
                                         &xhigher_priority_task_woken);


    if (x_result == pdFALSE)
    {
        FDDMA_ERROR("xEventGroupSetBitsFromISR() fail.");
    }

    portYIELD_FROM_ISR(xhigher_priority_task_woken);

    return;
}

static boolean DdmaSpiLoopbackWaitDmaEnd(void)
{
    const TickType_t wait_delay = pdMS_TO_TICKS(10000UL); /* wait for 5 seconds */
    boolean ok = TRUE;
    EventBits_t ev;
    u32 wait_bits = CHAN_REQ_DONE(rx_chan_id) | CHAN_REQ_DONE(tx_chan_id);

    ev = xEventGroupWaitBits(chan_evt, wait_bits, pdTRUE, pdTRUE, wait_delay);
    if ((ev & wait_bits) == wait_bits)
    {
        FDDMA_INFO("DDMA transfer success.");
    }
    else
    {
        if ((ev & CHAN_REQ_DONE(tx_chan_id)) == 0U)
        {
            FDDMA_ERROR("TX timeout.");
        }

        if ((ev & CHAN_REQ_DONE(rx_chan_id)) == 0U)
        {
            FDDMA_ERROR("RX timeout.");
        }

        if (ev & wait_bits == 0U)
        {
            FDDMA_ERROR("Both TX and RX timeout.");
        }

        ok = FALSE;
    }

    return ok;
}

static inline boolean DdmaSpiLoopbackGiveSync()
{
    boolean data = TRUE;
    FASSERT_MSG((NULL != sync), "Sync not exists.");
    if (pdFALSE == xQueueSend(sync, &data, portMAX_DELAY))
    {
        FDDMA_ERROR("Failed to give locker.");
        return FALSE;
    }

    return TRUE;
}

static inline void DdmaSpiLoopbackTakeSync()
{
    boolean data = FALSE;
    FASSERT_MSG((NULL != sync), "Sync not exists.");
    if (pdFALSE == xQueueReceive(sync, &data, portMAX_DELAY))
    {
        FDDMA_ERROR("Failed to give locker.");
    }

    return;
}

static void DdmaInitTask(void *args)
{
    const u32 ddma_id = FDDMA0_ID; /* spi use DDMA-0 only */
    FError err = FT_SUCCESS;
    uintptr spi_base;

    trans_len = dma_trans_bytes;

    spim = FFreeRTOSSpimInit(spi_instance_id, &spim_config); /* init SPIM */
    FASSERT_MSG(spim, "Init SPIM failed.");

    ddma = FFreeRTOSDdmaInit(ddma_id, &ddma_config); /* deinit DDMA */
    FASSERT_MSG(ddma, "Init DDMA failed.");

    spi_base = spim->ctrl.config.base_addr;

    rx_request.slave_id = spim_rx_slave_id[spi_instance_id];
    rx_request.mem_addr = (uintptr)(void *)rx_buf;
    rx_request.dev_addr = spi_base + FSPIM_DR_OFFSET;
    rx_request.trans_len = trans_len;
    rx_request.is_rx = TRUE;
    rx_request.req_done_handler = DdmaSpiLoopbackAckDMADone;
    rx_request.req_done_args = NULL;
    err = FFreeRTOSDdmaSetupChannel(ddma, rx_chan_id, &rx_request);
    FASSERT_MSG(FT_SUCCESS == err, "Init RX channel failed.");

    tx_request.slave_id = spim_tx_slave_id[spi_instance_id];
    tx_request.mem_addr = (uintptr)(void *)tx_buf;
    tx_request.dev_addr = spi_base + FSPIM_DR_OFFSET;
    tx_request.trans_len = trans_len;
    tx_request.is_rx = FALSE;
    tx_request.req_done_handler = DdmaSpiLoopbackAckDMADone;
    tx_request.req_done_args = NULL;
    err = FFreeRTOSDdmaSetupChannel(ddma, tx_chan_id, &tx_request);
    FASSERT_MSG(FT_SUCCESS == err, "Init TX channel failed.");

    DdmaSpiLoopbackGiveSync(); /* give sync and allow sending */

    vTaskDelete(NULL);
}

static void DdmaSpiLoopbackSendTask(void *args)
{
    u32 loop;
    u32 times = 0U;
    char ch = 'A';
    const TickType_t wait_delay = pdMS_TO_TICKS(2000UL); /* wait for 2 seconds */

    for (;;)
    {
        FDDMA_INFO("Waiting send data...");

        DdmaSpiLoopbackTakeSync(); /* is sending, take send sync and cannot send again */

        memset(tx_buf, 0, trans_len);
        memset(rx_buf, 0, trans_len);

        for (loop = 0; loop < trans_len; loop += 4)
        {
            tx_buf[loop] = (u8)ch + (times % 10);
        }

        FCacheDCacheInvalidateRange((uintptr)(void *)tx_buf, trans_len);

        printf("Before loopback ..... \r\n");
        printf("TX buf ===> \r\n");
        FtDumpHexByte((u8 *)tx_buf, trans_len);
        printf("RX buf <=== \r\n");
        FtDumpHexByte((u8 *)rx_buf, trans_len);

        spi_msg.rx_buf = rx_buf;
        spi_msg.rx_len = trans_len;
        spi_msg.tx_buf = tx_buf;
        spi_msg.tx_len = trans_len;

        if ((FFREERTOS_DDMA_OK != FFreeRTOSDdmaStartChannel(ddma, rx_chan_id)) ||
            (FFREERTOS_DDMA_OK != FFreeRTOSDdmaStartChannel(ddma, tx_chan_id)))
        {
            FDDMA_ERROR("Start DDMA channel failed.");
            break;
        }

        vTaskDelay(200);

        /* setup spi transfer only for the first time */
        if ((0 == times) && (FFREERTOS_DDMA_OK != FFreeRTOSSpimTransfer(spim, &spi_msg)))
        {
            FDDMA_ERROR("Start SPI transfer failed.");
            break;;
        }

        if (times++ > loopback_times)
        {
            break;
        }

        vTaskDelay(wait_delay);
    }

task_err:
    printf("Send task finished.\r\n");
    FFreeRTOSDdmaStop(ddma);
    vTaskSuspend(NULL);
}

static void DdmaSpiLoopbackRecvTask(void *args)
{
    u32 times = 0U;

    for (;;)
    {
        FDDMA_INFO("Waiting for recv data...");

        /* block recv task until RX done */
        if (!DdmaSpiLoopbackWaitDmaEnd())
        {
            continue;
        }

        if ((FFREERTOS_DDMA_OK != FFreeRTOSDdmaStopChannel(ddma, tx_chan_id)) ||
            (FFREERTOS_DDMA_OK != FFreeRTOSDdmaStopChannel(ddma, rx_chan_id)))
        {
            FDDMA_ERROR("Stop DDMA transfer failed.");
            continue;
        }

        FCacheDCacheInvalidateRange((uintptr)(void *)rx_buf, trans_len);

        printf("After loopback ..... \r\n");
        printf("TX buf ===> \r\n");
        FtDumpHexByte(tx_buf, trans_len);
        printf("RX buf <=== \r\n");
        FtDumpHexByte(rx_buf, trans_len);

        /* compare if loopback success */
        if (0 == memcmp(rx_buf, tx_buf, trans_len))
        {
            printf("Loopback transfer success.\r\n");
        }
        else
        {
            FDDMA_ERROR("RX data != TX data, loopback transfer failed.");
        }

        if (times++ > loopback_times)
        {
            break;
        }

        DdmaSpiLoopbackGiveSync(); /* recv finish, give send sync and allow sending */
    }

    printf("Receive task finished.\r\n");
    FFreeRTOSDdmaStop(ddma);
    vTaskSuspend(NULL);
}

BaseType_t FFreeRTOSRunDDMASpiLoopback(u32 spi_id, u32 bytes)
{
    BaseType_t ret = pdPASS;
    const TickType_t total_run_time = pdMS_TO_TICKS(30000UL); /* loopback runs for 10 secs deadline */

    if (is_running)
    {
        FDDMA_ERROR("Task is running.");
        return pdPASS;
    }

    is_running = TRUE;
    spi_instance_id = spi_id;
    dma_trans_bytes = bytes;

    FASSERT_MSG(NULL == chan_evt, "Event group exists.");
    FASSERT_MSG((chan_evt = xEventGroupCreate()) != NULL, "Create event group failed.");

    FASSERT_MSG(NULL == sync, "Sync exists.");
    FASSERT_MSG((sync = xQueueCreate(1, sizeof(boolean))) != NULL, "Create sync failed.");

    taskENTER_CRITICAL(); /* no schedule when create task */

    ret = xTaskCreate((TaskFunction_t)DdmaInitTask,  /* task entry */
                      (const char *)"DdmaInitTask",/* task name */
                      (uint16_t)4096,  /* task stack size in words */
                      NULL, /* task params */
                      (UBaseType_t)configMAX_PRIORITIES - 1,  /* task priority */
                      NULL); /* task handler */

    FASSERT_MSG(pdPASS == ret, "Create task failed.");

    ret = xTaskCreate((TaskFunction_t)DdmaSpiLoopbackSendTask,  /* task entry */
                      (const char *)"DdmaSpiLoopbackSendTask",/* task name */
                      (uint16_t)4096,  /* task stack size in words */
                      NULL, /* task params */
                      (UBaseType_t)configMAX_PRIORITIES - 2,  /* task priority */
                      (TaskHandle_t *)&send_task); /* task handler */

    FASSERT_MSG(pdPASS == ret, "Create task failed.");

    ret = xTaskCreate((TaskFunction_t)DdmaSpiLoopbackRecvTask,  /* task entry */
                      (const char *)"DdmaSpiLoopbackRecvTask",/* task name */
                      (uint16_t)4096,  /* task stack size in words */
                      NULL, /* task params */
                      (UBaseType_t)configMAX_PRIORITIES - 1,  /* task priority */
                      (TaskHandle_t *)&recv_task); /* task handler */

    FASSERT_MSG(pdPASS == ret, "Create task failed.");

    exit_timer = xTimerCreate("Exit-Timer",                 /* Text name for the software timer - not used by FreeRTOS. */
                              total_run_time,                 /* The software timer's period in ticks. */
                              pdFALSE,                        /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                              NULL,                           /* use timer id to pass task data for reference. */
                              DdmaSpiLoopbackExitCallback);   /* The callback function to be used by the software timer being created. */

    FASSERT_MSG(NULL != exit_timer, "Create exit timer failed.");

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    ret = xTimerStart(exit_timer, 0); /* start */

    FASSERT_MSG(pdPASS == ret, "Start exit timer failed.");

    return ret;
}