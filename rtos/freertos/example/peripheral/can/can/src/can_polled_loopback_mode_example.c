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
 * FilePath: can_polled_loopback_mode_example.c
 * Date: 2023-10-11 11:13:48
 * LastEditTime: 2023-10-20 11:13:48
 * Description:  This file is for CAN task implementations 
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2023/10/7   first commit
 */
#include <string.h>
#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "fcan.h"
#include "fcan_os.h"
#include "fcpu_info.h"
#include "fio_mux.h"
#include "fassert.h"
#include "fdebug.h"

#define FCAN_TEST_DEBUG_TAG "FCAN_FREERTOS_POLLED_TEST"
#define FCAN_TEST_DEBUG(format, ...) FT_DEBUG_PRINT_D(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_INFO(format, ...) FT_DEBUG_PRINT_I(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_WARN(format, ...) FT_DEBUG_PRINT_W(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_ERROR(format, ...) FT_DEBUG_PRINT_E(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)

/* can frame config */
#define FCAN_SEND_STID 0x000007FDU
#define FCAN_SEND_EXID 0x1FFFFFFDU
#define FCAN_SEND_LENGTH 8
#define FCAN_SEND_STID_MAX 0x000007FFU
#define FCAN_SEND_EXID_MAX 0x1FFFFFFFU

/* can send period */
#define CAN_SEND_PERIOD             ( pdMS_TO_TICKS( 100UL ))

/* can baudrate */
#define ARB_BAUD_RATE  1000000
#define DATA_BAUD_RATE 1000000

/* Declare a variable of type QueueHandle_t.  This is used to store the queue that is accessed by all three tasks. */
static QueueHandle_t xQueue;

static xSemaphoreHandle test_semaphore;

static xTaskHandle send_handle;
static xTaskHandle recv_handle;

static FFreeRTOSCan *os_can_ctrl_p[FCAN_NUM];

static FCanFrame send_frame[FCAN_NUM];
static FCanFrame recv_frame[FCAN_NUM];

static void FFreeRTOSCanSendTask(void *pvParameters);
static void FFreeRTOSCanRecvTask(void *pvParameters);
static void FFreeRTOSCanDelete(void);


static FError FFreeRTOSCanBaudrateSet(FFreeRTOSCan *os_can_p)
{
    FError ret = FCAN_SUCCESS;

    FCanBaudrateConfig arb_segment_config;
    FCanBaudrateConfig data_segment_config;
    memset(&arb_segment_config, 0, sizeof(arb_segment_config));
    memset(&data_segment_config, 0, sizeof(data_segment_config));
    arb_segment_config.baudrate = ARB_BAUD_RATE;
    arb_segment_config.auto_calc = TRUE;
    arb_segment_config.segment = FCAN_ARB_SEGMENT;

    data_segment_config.baudrate = DATA_BAUD_RATE;
    data_segment_config.auto_calc = TRUE;
    data_segment_config.segment = FCAN_DATA_SEGMENT;

    ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_BAUDRATE_SET, &arb_segment_config);
    if (FCAN_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("FFreeRTOSCanControl arb_segment_config failed.");
        return ret;
    }

    ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_BAUDRATE_SET, &data_segment_config);
    if (FCAN_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("FFreeRTOSCanControl data_segment_config failed.");
        return ret;
    }
    return ret;
}


static FError FFreeRTOSCanIdMaskSet(FFreeRTOSCan *os_can_p, int frame_type)
{
    FError ret = FCAN_SUCCESS;
    FCanIdMaskConfig id_mask;
    memset(&id_mask, 0, sizeof(id_mask));
    for (int i = 0; i < FCAN_ACC_ID_REG_NUM; i++)
    {
        id_mask.filter_index = i;
        id_mask.id = 0;
        id_mask.mask = FCAN_ACC_IDN_MASK;
        if ( frame_type == 1 )
        {
            id_mask.type = EXTEND_FRAME;
        }

        ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_ID_MASK_SET, &id_mask);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ID_MASK_SET %d failed.", i);
            return ret;
        }
    }

    return ret;
}

static void FFreeRTOSCanInitTask(void *pvParameters)
{
    FError ret = FCAN_SUCCESS;
    BaseType_t xReturn = pdPASS;
    u32 instance_id = FCAN0_ID;
    u32 tran_mode = FCAN_PROBE_NORMAL_MODE;

    /* The queue is created to hold a maximum of 32 structures of type u32 instance_id . */
    xQueue = xQueueCreate(32, sizeof(u32));
    if (xQueue == NULL)
    {
        printf("FFreeRTOSCreateCanPolledTestTask FCanQueueData create failed.\r\n");
    }

    /*init iomux*/
    FIOMuxInit();

    for (instance_id = FCAN0_ID; instance_id < FCAN_NUM; instance_id++)
    {
        FIOPadSetCanMux(instance_id);

        /* init can controller */
        os_can_ctrl_p[instance_id] = FFreeRTOSCanInit(instance_id);
        if (os_can_ctrl_p[instance_id] == NULL)
        {
            printf("FFreeRTOSCanInit %d failed!!!\r\n", instance_id);
            goto can_init_exit;
        }

        /* set can baudrate */
        ret = FFreeRTOSCanBaudrateSet(os_can_ctrl_p[instance_id]);
        if (FCAN_SUCCESS != ret)
        {
            printf("FFreeRTOSCanInit FFreeRTOSCanBaudrateSet failed!!!\r\n");
            goto can_init_exit;
        }

        /* set can id mask */
        ret = FFreeRTOSCanIdMaskSet(os_can_ctrl_p[instance_id], ((int)(uintptr)pvParameters));
        if (FCAN_SUCCESS != ret)
        {
            printf("FFreeRTOSCanInit FFreeRTOSCanIdMaskSet failed!!!\r\n");
            goto can_init_exit;
        }

        /* Identifier mask enable */
        ret = FFreeRTOSCanControl(os_can_ctrl_p[instance_id], FREERTOS_CAN_CTRL_ID_MASK_ENABLE, NULL);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ID_MASK_ENABLE failed.");
            goto can_init_exit;
        }

        /* set can transfer mode */
        ret = FFreeRTOSCanControl(os_can_ctrl_p[instance_id], FREERTOS_CAN_CTRL_MODE_SET, &tran_mode);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_MODE_SET failed.");
            goto can_init_exit;
        }

        /* enable can transfer */
        ret = FFreeRTOSCanControl(os_can_ctrl_p[instance_id], FREERTOS_CAN_CTRL_ENABLE, NULL);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ENABLE failed.");
            goto can_init_exit;
        }

    }

    printf("FFreeRTOSCanInitTask execute success !!!\r\n");

    if ( ((int)(uintptr)pvParameters) == 0 )
    {
        printf("Standard frame test example!!!\r\n");
    }
    else if ( ((int)(uintptr)pvParameters) == 1 )
    {
        printf("Extended frame test example!!!\r\n");
    }

    /* can send task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanSendTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanSendTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          pvParameters,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 5, /* 任务的优先级 */
                          (TaskHandle_t *)&send_handle); /* 任务控制 */
    if (xReturn != pdPASS)
    {
        printf("Create FFreeRTOSCanSendTask failed.\r\n");
        goto can_init_exit;
    }

    /* can recv task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanRecvTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanRecvTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 5, /* 任务的优先级 */
                          (TaskHandle_t *)&recv_handle); /* 任务控制 */
    if (xReturn != pdPASS)
    {
        printf("Create FFreeRTOSCanRecvTask failed.\r\n");
        goto can_init_exit;
    }

can_init_exit:
    vTaskDelete(NULL);
}

static void FFreeRTOSCanSendTask(void *pvParameters)
{
    FError ret = FCAN_SUCCESS;
    u32 instance_id = FCAN0_ID;
    u32 count[FCAN_NUM] = {0};
    u32 send_max_id;
    int i = 0;
    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        printf("\r\ncan send task running.\r\n");
        for (instance_id = FCAN0_ID; instance_id <= FCAN1_ID; instance_id++)
        {
            send_frame[instance_id].candlc = FCAN_SEND_LENGTH;                         //DLC
            if (((int)(uintptr)pvParameters) == 1) 
            {
                send_frame[instance_id].canid = FCAN_SEND_EXID + count[instance_id];   //EXID                                    //IDE
                send_frame[instance_id].canid |= CAN_EFF_FLAG;
                send_max_id = CAN_EFF_MASK | CAN_EFF_FLAG;
            }  
            else
            {
                send_frame[instance_id].canid = FCAN_SEND_STID + count[instance_id];   //STID
                send_frame[instance_id].canid &= CAN_SFF_MASK;
                send_max_id = CAN_SFF_MASK;
            }

            for (i = 0; i < send_frame[instance_id].candlc; i++)
            {
                send_frame[instance_id].data[i] = i + (instance_id << 4);
            }
            ret = FFreeRTOSCanSend(os_can_ctrl_p[instance_id], &send_frame[instance_id]);
            if (ret != FCAN_SUCCESS)
            {
                printf("can%d send failed.\n", instance_id);
            }
            count[instance_id]++;
            vTaskDelay(CAN_SEND_PERIOD);
            xQueueSendToBack(xQueue, &instance_id, portMAX_DELAY);
        }
        if ( (send_frame[instance_id - 1].canid == send_max_id) )
        {
            vTaskDelete(NULL);
        }
    }
}

static void FFreeRTOSCanRecvTask(void *pvParameters)
{
    FError ret = FCAN_SUCCESS;
    u32 count[FCAN_NUM] = {0};
    int i = 0;
    u32 instance_id = FCAN1_ID;
    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* wait can send give semphore */
        xQueueReceive(xQueue, &instance_id, portMAX_DELAY);
        instance_id = FCAN1_ID - instance_id;
        ret = FFreeRTOSCanRecv(os_can_ctrl_p[instance_id], &recv_frame[instance_id]);
        if (FCAN_SUCCESS == ret)
        {
            printf("\r\ncan%d recv id is 0x%02x.\r\n", instance_id, recv_frame[instance_id].canid);
            printf("can%d recv dlc is %d.\r\n", instance_id, recv_frame[instance_id].candlc);
            printf("can%d recv data is ", instance_id);
            for (i = 0; i < recv_frame[instance_id].candlc; i++)
            {
                printf("0x%02x ", recv_frame[instance_id].data[i]);
                if (recv_frame[instance_id].data[i] != send_frame[FCAN1_ID - instance_id].data[i])
                {
                    FCAN_TEST_ERROR("\ncount%d = %d: can%d recv is not equal to can%d send!!!\r\n", instance_id,  count[instance_id], instance_id, FCAN1_ID - instance_id);
                }
            }
            printf("\ncount%d = %d: can%d recv is equal to can%d send!!!\r\n", instance_id, count[instance_id], instance_id, FCAN1_ID - instance_id);
            count[instance_id]++;
        }
        if ((instance_id == 0) 
        && ((recv_frame[instance_id].canid == (FCAN_SEND_EXID_MAX | CAN_EFF_FLAG)) 
        || (recv_frame[instance_id].canid == FCAN_SEND_STID_MAX)))
        {
            FFreeRTOSCanDelete();
        }
    }
}

/* create can polled test, can0 and can1 loopback */
BaseType_t FFreeRTOSCreateCanPolledTestTask(void)
{
    BaseType_t xReturn = pdPASS;
    BaseType_t timer_started = pdPASS;

    test_semaphore = xSemaphoreCreateBinary();
    if (test_semaphore != NULL)
    {
        xSemaphoreGive(test_semaphore);
    }

    /* can polled example standard frame task */
    xSemaphoreTake(test_semaphore, portMAX_DELAY);
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanInitTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanInitTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)0,/* 任务入口函数参数 */
                          (UBaseType_t)1,  /* 任务的优先级 */
                          NULL); /* 任务控制 */

    /* can polled example extended frame task */
    xSemaphoreTake(test_semaphore, portMAX_DELAY);
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanInitTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanInit2Task",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)1,/* 任务入口函数参数 */
                          (UBaseType_t)1,  /* 任务的优先级 */
                          NULL); /* 任务控制 */

    return xReturn;
}

static void FFreeRTOSCanDelete(void)
{
    /* deinit can os instance */
    FFreeRTOSCanDeinit(os_can_ctrl_p[FCAN1_ID]);
    FFreeRTOSCanDeinit(os_can_ctrl_p[FCAN0_ID]);

    /*iopad deinit */
    FIOMuxDeInit();

    /* delete queue */
    vQueueDelete(xQueue);

    xSemaphoreGive(test_semaphore);

    if (recv_handle)
    {
        vPrintf("\r\nDelete FFreeRTOSCanRecvTask success.\r\n");
        vPrintf("\r\nDelete FFreeRTOSCanSendTask success.\r\n");
        vTaskDelete(recv_handle);
    }
}
