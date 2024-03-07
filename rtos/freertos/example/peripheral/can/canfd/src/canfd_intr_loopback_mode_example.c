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
 * FilePath: canfd_intr_loopback_mode_example.c
 * Date: 2023-10-20 11:32:48
 * LastEditTime: 2023-10-20 11:32:48
 * Description:  This file is for CAN task implementations 
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 huangjin     2023/10/7   first commit
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

#define FCAN_TEST_DEBUG_TAG "FCANFD_FREERTOS_INTR_TEST"
#define FCAN_TEST_DEBUG(format, ...) FT_DEBUG_PRINT_D(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_INFO(format, ...) FT_DEBUG_PRINT_I(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_WARN(format, ...) FT_DEBUG_PRINT_W(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_ERROR(format, ...) FT_DEBUG_PRINT_E(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)

/* canfd frame config */
#define FCAN_SEND_STID 0x000007FDU
#define FCAN_SEND_EXID 0x1FFFFFFDU
#define FCAN_SEND_LENGTH 16
#define FCAN_SEND_STID_MAX 0x000007FFU
#define FCAN_SEND_EXID_MAX 0x1FFFFFFFU

/* canfd send period */
#define CAN_SEND_PERIOD             ( pdMS_TO_TICKS( 100UL ))

/* canfd baudrate */
#define ARB_BAUD_RATE  1000000
#define DATA_BAUD_RATE 2000000

typedef struct
{
    u32 count;
    FFreeRTOSCan *os_can_p;
} FCanQueueData;

/* Declare a variable of type QueueHandle_t.  This is used to store the queue that is accessed by all three tasks. */
static QueueHandle_t xQueue;

static xSemaphoreHandle test_semaphore;

static xTaskHandle send_handle;
static xTaskHandle recv_handle;

static FFreeRTOSCan *os_can_ctrl_p[FCAN_NUM];

static FCanFrame send_frame[FCAN_NUM];
static FCanFrame recv_frame[FCAN_NUM];

static void FFreeRTOSCanfdSendTask(void *pvParameters);
static void FFreeRTOSCanfdRecvTask(void *pvParameters);
static void FFreeRTOSCanDelete(void);

static void FCanTxIrqCallback(void *args)
{
    FFreeRTOSCan *os_can_p = (FFreeRTOSCan *)args;
    FCAN_TEST_DEBUG("Can%d irq send frame is ok.", os_can_p->can_ctrl.config.instance_id);
}

static void FCanRxIrqCallback(void *args)
{
    FFreeRTOSCan *os_can_p = (FFreeRTOSCan *)args;
    FCAN_TEST_DEBUG("Can%d irq recv frame callback.", os_can_p->can_ctrl.config.instance_id);

    static FCanQueueData xSendStructure;
    xSendStructure.os_can_p = os_can_p;

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    xQueueSendToBackFromISR(xQueue, &xSendStructure, &xHigherPriorityTaskWoken);

    /* never call taskYIELD() form ISR! */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static FError FFreeRTOSCanIntrSet(FFreeRTOSCan *os_can_p)
{
    FError ret = FCAN_SUCCESS;

    FCanIntrEventConfig intr_event;
    memset(&intr_event, 0, sizeof(intr_event));

    intr_event.type = FCAN_INTR_EVENT_SEND;
    intr_event.handler = FCanTxIrqCallback;
    intr_event.param = (void *)os_can_p;
    ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_INTR_SET, &intr_event);
    if (FCAN_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("FFreeRTOSCanControl FCAN_INTR_EVENT_SEND failed.");
        return ret;
    }

    intr_event.type = FCAN_INTR_EVENT_RECV;
    intr_event.handler = FCanRxIrqCallback;
    intr_event.param = (void *)os_can_p;
    ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_INTR_SET, &intr_event);
    if (FCAN_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("FFreeRTOSCanControl FCAN_INTR_EVENT_RECV failed.");
        return ret;
    }

    u32 cpu_id;
    GetCpuId(&cpu_id);
    FCanCtrl *instance_p = &os_can_p->can_ctrl;
    InterruptSetTargetCpus(instance_p->config.irq_num, cpu_id);
    InterruptSetPriority(instance_p->config.irq_num, instance_p->config.irq_prority);
    InterruptInstall(instance_p->config.irq_num, FCanIntrHandler, instance_p, "can");
    InterruptUmask(instance_p->config.irq_num);

    return ret;
}


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

    /* The queue is created to hold a maximum of 32 structures of type xData. */
    xQueue = xQueueCreate(32, sizeof(FCanQueueData));
    if (xQueue == NULL)
    {
        printf("FFreeRTOSCreateCanfdIntrTestTask FCanQueueData create failed.\r\n");
    }

    /*init iomux*/
    FIOMuxInit();

    for (instance_id = FCAN0_ID; instance_id < FCAN_NUM; instance_id++)
    {
        FIOPadSetCanMux(instance_id);
        
        /* init canfd controller */
        os_can_ctrl_p[instance_id] = FFreeRTOSCanInit(instance_id);
        if (os_can_ctrl_p[instance_id] == NULL)
        {
            printf("FFreeRTOSCanInit %d failed!!!\r\n", instance_id);
            goto canfd_init_exit;
        }

        /* enable canfd */
        ret = FFreeRTOSCanControl(os_can_ctrl_p[instance_id], FREERTOS_CAN_CTRL_FD_ENABLE, (void *)1);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_FD_ENABLE failed.");
            goto canfd_init_exit;
        }

        /* set canfd baudrate */
        ret = FFreeRTOSCanBaudrateSet(os_can_ctrl_p[instance_id]);
        if (FCAN_SUCCESS != ret)
        {
            printf("FFreeRTOSCanInit FFreeRTOSCanBaudrateSet failed!!!\r\n");
            goto canfd_init_exit;
        }

        /* set canfd id mask */
        ret = FFreeRTOSCanIdMaskSet(os_can_ctrl_p[instance_id], ((int)(uintptr)pvParameters));
        if (FCAN_SUCCESS != ret)
        {
            printf("FFreeRTOSCanInit FFreeRTOSCanIdMaskSet failed!!!\r\n");
            goto canfd_init_exit;
        }

        /* Identifier mask enable */
        ret = FFreeRTOSCanControl(os_can_ctrl_p[instance_id], FREERTOS_CAN_CTRL_ID_MASK_ENABLE, NULL);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ID_MASK_ENABLE failed.");
            goto canfd_init_exit;
        }

        /* init canfd interrupt handler */
        ret = FFreeRTOSCanIntrSet(os_can_ctrl_p[instance_id]);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanInit FFreeRTOSCanIntrSet failed!!!");
            goto canfd_init_exit;
        }

        /* set canfd transfer mode */
        ret = FFreeRTOSCanControl(os_can_ctrl_p[instance_id], FREERTOS_CAN_CTRL_MODE_SET, &tran_mode);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_MODE_SET failed.");
            goto canfd_init_exit;
        }

        /* enable canfd transfer */
        ret = FFreeRTOSCanControl(os_can_ctrl_p[instance_id], FREERTOS_CAN_CTRL_ENABLE, NULL);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ENABLE failed.");
            goto canfd_init_exit;
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

    /* canfd send task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanfdSendTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanfdSendTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          pvParameters,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 5, /* 任务的优先级 */
                          (TaskHandle_t *)&send_handle); /* 任务控制 */
    if (xReturn != pdPASS)
    {
        printf("Create FFreeRTOSCanfdSendTask failed.\r\n");
        goto canfd_init_exit;
    }

    /* canfd recv task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanfdRecvTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanfdRecvTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 5, /* 任务的优先级 */
                          (TaskHandle_t *)&recv_handle); /* 任务控制 */
    if (xReturn != pdPASS)
    {
        printf("Create FFreeRTOSCanfdRecvTask failed.\r\n");
        goto canfd_init_exit;
    }

canfd_init_exit:
    vTaskDelete(NULL);
}

static void FFreeRTOSCanfdSendTask(void *pvParameters)
{
    FError ret = FCAN_SUCCESS;
    u32 instance_id = FCAN0_ID;
    u32 count[FCAN_NUM] = {0};
    u32 send_max_id;
    int i = 0;
    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        printf("\r\ncanfd send task running.\r\n");
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
                printf("canfd%d send failed.\n", instance_id);
            }
            count[instance_id]++;
            vTaskDelay(CAN_SEND_PERIOD);
        }
        if ( (send_frame[instance_id - 1].canid == send_max_id) )
        {
            vTaskDelete(NULL);
        }
    }
}

static void FFreeRTOSCanfdRecvTask(void *pvParameters)
{
    FError ret = FCAN_SUCCESS;
    u32 count[FCAN_NUM] = {0};
    int i = 0;
    static FCanQueueData xReceiveStructure;
    FFreeRTOSCan *os_can_p;
    u32 instance_id = FCAN0_ID;
    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* wait recv interrupt give semphore */
        xQueueReceive(xQueue, &xReceiveStructure, portMAX_DELAY);
        os_can_p = xReceiveStructure.os_can_p;
        instance_id = os_can_p->can_ctrl.config.instance_id;
        ret = FFreeRTOSCanRecv(os_can_p, &recv_frame[instance_id]);
        if (FCAN_SUCCESS == ret)
        {
            printf("\r\ncanfd%d recv id is 0x%02x.\r\n", instance_id, recv_frame[instance_id].canid);
            printf("canfd%d recv dlc is %d.\r\n", instance_id, recv_frame[instance_id].candlc);
            printf("canfd%d recv data is ", instance_id);
            for (i = 0; i < recv_frame[instance_id].candlc; i++)
            {
                printf("0x%02x ", recv_frame[instance_id].data[i]);
                if (recv_frame[instance_id].data[i] != send_frame[FCAN1_ID - instance_id].data[i])
                {
                    FCAN_TEST_ERROR("\ncount%d = %d: canfd%d recv is not equal to canfd%d send!!!\r\n", instance_id,  count[instance_id], instance_id, FCAN1_ID - instance_id);
                }
            }
            printf("\ncount%d = %d: canfd%d recv is equal to canfd%d send!!!\r\n", instance_id, count[instance_id], instance_id, FCAN1_ID - instance_id);
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

/* create canfd intr test, can0 and can1 loopback */
BaseType_t FFreeRTOSCreateCanfdIntrTestTask(void)
{
    BaseType_t xReturn = pdPASS;
    BaseType_t timer_started = pdPASS;

    test_semaphore = xSemaphoreCreateBinary();
    if (test_semaphore != NULL)
    {
        xSemaphoreGive(test_semaphore);
    }

    /* canfd intr example standard frame task */
    xSemaphoreTake(test_semaphore, portMAX_DELAY);
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanInitTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanInitTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)0,/* 任务入口函数参数 */
                          (UBaseType_t)1,  /* 任务的优先级 */
                          NULL); /* 任务控制 */

    /* canfd intr example extended frame task */
    xSemaphoreTake(test_semaphore, portMAX_DELAY);
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanInitTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanInitTask2",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)1,/* 任务入口函数参数 */
                          (UBaseType_t)1,  /* 任务的优先级 */
                          NULL); /* 任务控制 */

    return xReturn;
}

static void FFreeRTOSCanDelete(void)
{
    /* deinit canfd os instance */
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