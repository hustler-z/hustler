/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for canfd interrupt loopback mode example function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/2/24   first release
 * 1.1   huangjin   2023/11/01  improve functions
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "fcan.h"
#include "fcan_hw.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fparameters.h"
#include "fsleep.h"
#include "fassert.h"
#include "ftypes.h"
#include "canfd_common.h"
#include "canfd_intr_loopback_mode_example.h"
#include "fio_mux.h"
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
/* variables used in example */
static FCanCtrl can[FCAN_NUM];
static FCanFrame send_frame;
static u16 can0_recv_irq_flag = 0;
static u16 can1_recv_irq_flag = 0;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/

/*Configuring interrupt service functions*/
static void FCanTxIrqCallback(void *args)
{
    FCanCtrl *instance_p = (FCanCtrl *)args;
    FCAN_TEST_DEBUG("Can%d irq send frame is ok.", instance_p->config.instance_id);
}

static void FCanRxIrqCallback(void *args)
{
    FCanCtrl *instance_p = (FCanCtrl *)args;
    FCAN_TEST_DEBUG("Can%d irq recv frame callback", instance_p->config.instance_id);
    FCanFrame recv_frame;
    u8 i;
    u32 status = 0;
    static u16 can0_recv_times = 0;
    static u16 can1_recv_times = 0;

    if (instance_p->config.instance_id == FCAN0_ID)
    {
        memset(&recv_frame, 0, sizeof(FCanFrame));
        status = FCanRecv(instance_p, &recv_frame);
        if ((status == FT_SUCCESS) && (send_frame.candlc == recv_frame.candlc))
        {
            FCAN_TEST_DEBUG("can 0 recv id is %#x\r\n", recv_frame.canid);
            FCAN_TEST_DEBUG("can 0 recv dlc is %d\r\n", recv_frame.candlc);
            FCAN_TEST_DEBUG("can 0 recv data is ");
            for (i = 0; i < recv_frame.candlc; i++)
            {
                FCAN_TEST_DEBUG("%#x ", recv_frame.data[i]);
                if (recv_frame.data[i] != send_frame.data[i])
                {
                    FCAN_TEST_ERROR("\nCan 0 recv is not equal to can 1 send,can0_recv_times= %d\r\n", can0_recv_times);
                    return;
                }
            }
        }
        else
        {
            FCAN_TEST_ERROR("Can 0 recv failed\n");
            return;
        }
        can0_recv_times++;
        if (can0_recv_times == CAN_LOOPBACK_TEST_TIMES)
        {
            if((recv_frame.canid & CAN_EFF_FLAG) == 0)
                printf("Can 0 recv is equal to can 1 send, use standard frame, can0_recv_times=%d.\n", can0_recv_times);
            else
                printf("Can 0 recv is equal to can 1 send, use extern frame, can0_recv_times=%d.\n", can0_recv_times);
            can0_recv_irq_flag = 1;
            can0_recv_times = 0;
        }

        }
        else if (instance_p->config.instance_id == FCAN1_ID)
        {
            memset(&recv_frame, 0, sizeof(FCanFrame));
            status = FCanRecv(instance_p, &recv_frame);
            if ((status == FT_SUCCESS) && (send_frame.candlc == recv_frame.candlc))
            {
                FCAN_TEST_DEBUG("Can 1 recv id is %#x\r\n", recv_frame.canid);
                FCAN_TEST_DEBUG("Can 1 recv dlc is %d\r\n", recv_frame.candlc);
                FCAN_TEST_DEBUG("Can 1 recv data is ");
                for (i = 0; i < recv_frame.candlc; i++)
                {
                    FCAN_TEST_DEBUG("%#x ", recv_frame.data[i]);
                    if (recv_frame.data[i] != send_frame.data[i])
                    {
                        FCAN_TEST_ERROR("Can 1 recv is not equal to can 0 send.\r\n");
                        return;
                    }
                }
            }
            else
            {
                FCAN_TEST_ERROR("Can 1 recv failed\n");
                return;
            }
            can1_recv_times++;
            if (can1_recv_times == CAN_LOOPBACK_TEST_TIMES)
            {
                if((recv_frame.canid & CAN_EFF_FLAG) == 0)
                    printf("Can 1 recv is equal to can 0 send, use standard frame, can1_recv_times=%d.\n", can1_recv_times);
                else
                    printf("Can 1 recv is equal to can 0 send, use extern frame, can1_recv_times=%d.\n", can1_recv_times);
                can1_recv_irq_flag = 1;
                can1_recv_times = 0;
            }
        }
}

static void FCanErrorCallback(void *args)
{
    FCanCtrl *instance_p = (FCanCtrl *)args;
    uintptr base_addr = instance_p->config.base_address;

    FCAN_TEST_DEBUG("Can %d is under error", instance_p->config.instance_id);
    FCAN_TEST_DEBUG("error_status is %x", FCAN_READ_REG32(base_addr, FCAN_INTR_OFFSET));
    FCAN_TEST_DEBUG("rxerr_cnt is %x", FCAN_ERR_CNT_RFN_GET(FCAN_READ_REG32(base_addr, FCAN_ERR_CNT_OFFSET)));
    FCAN_TEST_DEBUG("txerr_cnt is %x", FCAN_ERR_CNT_TFN_GET(FCAN_READ_REG32(base_addr, FCAN_ERR_CNT_OFFSET)));
}

static void FCanBusoffCallback(void *args)
{
    FCanCtrl *instance_p = (FCanCtrl *)args;
    FCAN_TEST_DEBUG("Can %d bus off", instance_p->config.instance_id);
}

static void FCanPerrorCallback(void *args)
{
    FCanCtrl *instance_p = (FCanCtrl *)args;
    FCAN_TEST_DEBUG("Can %d passion error callback", instance_p->config.instance_id);
}

static void FCanPwarnCallback(void *args)
{
    FCanCtrl *instance_p = (FCanCtrl *)args;
    FCAN_TEST_DEBUG("Can %d passion warn callback", instance_p->config.instance_id);
}

static void FCanRxFifoFullCallback(void *args)
{
    FCanCtrl *instance_p = (FCanCtrl *)args;
    FCAN_TEST_DEBUG("Can %d rx_fifo full callback", instance_p->config.instance_id);
}

static void FCanTxFifoEmptyCallback(void *args)
{
    FCanCtrl *instance_p = (FCanCtrl *)args;
    FCAN_TEST_DEBUG("Can %d tx_fifo empty callback", instance_p->config.instance_id);
}

static void FCanIrqSet(FCanCtrl *instance_p)
{
    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(instance_p->config.irq_num, cpu_id);

    /* Enable interrupts, tx over, rx over, error, bus off, passive error, passive warning, rx fifo full, tx fifo empty*/
    FCanIntrEventConfig intr_event;
    memset(&intr_event, 0, sizeof(intr_event));

    intr_event.type = FCAN_INTR_EVENT_SEND;
    intr_event.handler = FCanTxIrqCallback;
    intr_event.param = (void *)instance_p;
    FCanRegisterInterruptHandler(instance_p, &intr_event);
    FCanInterruptEnable(instance_p, intr_event.type);

    intr_event.type = FCAN_INTR_EVENT_RECV;
    intr_event.handler = FCanRxIrqCallback;
    intr_event.param = (void *)instance_p;
    FCanRegisterInterruptHandler(instance_p, &intr_event);
    FCanInterruptEnable(instance_p, intr_event.type);

    intr_event.type = FCAN_INTR_EVENT_ERROR;
    intr_event.handler = FCanErrorCallback;
    intr_event.param = (void *)instance_p;
    FCanRegisterInterruptHandler(instance_p, &intr_event);
    FCanInterruptEnable(instance_p, intr_event.type);

    intr_event.type = FCAN_INTR_EVENT_BUSOFF;
    intr_event.handler = FCanBusoffCallback;
    intr_event.param = (void *)instance_p;
    FCanRegisterInterruptHandler(instance_p, &intr_event);
    FCanInterruptEnable(instance_p, intr_event.type);

    intr_event.type = FCAN_INTR_EVENT_PERROE;
    intr_event.handler = FCanPerrorCallback;
    intr_event.param = (void *)instance_p;
    FCanRegisterInterruptHandler(instance_p, &intr_event);
    FCanInterruptEnable(instance_p, intr_event.type);

    intr_event.type = FCAN_INTR_EVENT_PWARN;
    intr_event.handler = FCanPwarnCallback;
    intr_event.param = (void *)instance_p;
    FCanRegisterInterruptHandler(instance_p, &intr_event);
    FCanInterruptEnable(instance_p, intr_event.type);

    intr_event.type = FCAN_INTR_EVENT_FIFOFULL;
    intr_event.handler = FCanRxFifoFullCallback;
    intr_event.param = (void *)instance_p;
    FCanRegisterInterruptHandler(instance_p, &intr_event);
    FCanInterruptEnable(instance_p, intr_event.type);

    intr_event.type = FCAN_INTR_EVENT_FIFOEMPTY;
    intr_event.handler = FCanTxFifoEmptyCallback;
    intr_event.param = (void *)instance_p;
    FCanRegisterInterruptHandler(instance_p, &intr_event);
    FCanInterruptEnable(instance_p, intr_event.type);

    InterruptSetPriority(instance_p->config.irq_num, 0);
    InterruptInstall(instance_p->config.irq_num, FCanIntrHandler, instance_p, "canfd");
    InterruptUmask(instance_p->config.irq_num);
}

int FCanfdIntrLoopbackExample()
{
    FError ret = FCAN_SUCCESS;
    u32 can_id = FCAN0_ID;
    FCanCtrl *instance_p;
    int send_times = 0;
    int i = 0;
    printf("Use canfd protocol!\n");

    /*init iomux*/
    FIOMuxInit();
    for (can_id = FCAN0_ID; can_id < FCAN_NUM ; can_id++)
    {
        FIOPadSetCanMux(can_id);
        ret = FCanCfgInitialize(&can[can_id], FCanLookupConfig(can_id));
        /*Exit with message if can-id is un-usable in target board */
        if (ret != FCAN_SUCCESS)
        {
            FCAN_TEST_DEBUG("Can%d Initialize error.", can_id);
            return FCAN_FAILURE;
        }
        /*enable canfd*/
        FCanFdEnable(&can[can_id], TRUE);
        /*set to normal mode*/
        FCanSetMode(&can[can_id], FCAN_PROBE_NORMAL_MODE);
    }
    /*Baud rate is configured as 1M*/
    FCanBaudrateConfig arb_segment_config;
    FCanBaudrateConfig data_segment_config;
    memset(&arb_segment_config, 0, sizeof(arb_segment_config));
    memset(&data_segment_config, 0, sizeof(data_segment_config));

    arb_segment_config.baudrate = CANFD_TEST_ARB_BAUD_RATE;
    arb_segment_config.auto_calc = TRUE;
    arb_segment_config.segment = FCAN_ARB_SEGMENT;

    data_segment_config.baudrate = CANFD_TEST_DATA_BAUD_RATE;
    data_segment_config.auto_calc = TRUE;
    data_segment_config.segment = FCAN_DATA_SEGMENT;

    for (can_id = FCAN0_ID; can_id < FCAN_NUM; can_id++)
    {
        ret = FCanBaudrateSet(&can[can_id], &arb_segment_config);
        if (ret != FCAN_SUCCESS)
        {
            FCAN_TEST_ERROR("Can%d set arb segment baudrate error.", can_id);
            return FCAN_FAILURE;
        }

        ret = FCanBaudrateSet(&can[can_id], &data_segment_config);
        if (ret != FCAN_SUCCESS)
        {
            FCAN_TEST_ERROR("Can%d set data segment baudrate error.", can_id);
            return FCAN_FAILURE;

        }
    }
    /* 首先测试标准帧，再测试扩展帧 */
    u8 ide_flag = 0;
    for(ide_flag = 0 ; ide_flag < 2; ide_flag++)
    {
        for (can_id = FCAN0_ID; can_id < FCAN_NUM; can_id++)
        {
            /* set filter */
            FCanIdMaskConfig id_mask;
            FCanEnable(&can[can_id], FALSE);
            memset(&id_mask, 0, sizeof(id_mask));
            for (int i = 0; i < FCAN_ACC_ID_REG_NUM; i++)
            {
                id_mask.filter_index = i;
                id_mask.id = 0;
                id_mask.mask = FCAN_ACC_IDN_MASK;
                if ( ide_flag == 1 )
                {
                    id_mask.type = EXTEND_FRAME;
                }
                ret |= FCanIdMaskFilterSet(&can[can_id], &id_mask);
            }

            if (ret != FCAN_SUCCESS)
            {
                FCAN_TEST_ERROR("Can%d set mask filter error", can_id);
                return FCAN_FAILURE;

            }
            /* Identifier mask enable */
            FCanIdMaskFilterEnable(&can[can_id]);
            /* init interrupt */
            FCanIrqSet(&can[can_id]);
            /*can enable*/
            FCanEnable(&can[can_id], TRUE);
        }

        fsleep_millisec(CAN_LOOPBACK_TEST_PERIAD_MS);
        memset(&send_frame, 0, sizeof(FCanFrame));
        send_frame.canid = CAN_TEST_SEND_ID;
        send_frame.candlc = CAN_TEST_SEND_EXID_LENGTH;

        if (ide_flag == 0)
        {
            send_frame.canid &= CAN_SFF_MASK;
        }
        else
        {
            send_frame.canid |= CAN_EFF_FLAG;
        }
        /* can 0 send, can 1 receive */
        for (send_times = 0; send_times < CAN_LOOPBACK_TEST_TIMES; send_times++)
        {
            for (i = 0; i < send_frame.candlc; i++)
            {
                send_frame.data[i] = i + send_times;
            }
            instance_p = &can[FCAN0_ID];
            ret = FCanSend(instance_p, &send_frame);
            if (ret != 0)
            {
                FCAN_TEST_ERROR("Can%d send %d times error.", FCAN0_ID, send_times);
                return FCAN_FAILURE;
            }
            fsleep_millisec(CAN_LOOPBACK_TEST_PERIAD_MS);
        }
        
        memset(&send_frame, 0, sizeof(FCanFrame));
        send_frame.canid = CAN_TEST_SEND_ID;
        send_frame.candlc = CAN_TEST_SEND_EXID_LENGTH;

        if (ide_flag == FALSE)
        {
            send_frame.canid &= CAN_SFF_MASK;
        }
        else
        {
            send_frame.canid |= CAN_EFF_FLAG;
        }
        for (send_times = 0; send_times < CAN_LOOPBACK_TEST_TIMES; send_times++)
        {
            for (i = 0; i < send_frame.candlc; i++)
            {
                send_frame.data[i] = i + send_times;
            }
            instance_p = &can[FCAN1_ID];
            ret = FCanSend(instance_p, &send_frame);
            if (ret != 0)
            {
                FCAN_TEST_ERROR("Can%d send %d times error.", FCAN1_ID, send_times);
                return FCAN_FAILURE;
            }
            fsleep_millisec(CAN_LOOPBACK_TEST_PERIAD_MS);
        }
    }

    for (can_id = FCAN0_ID; can_id < FCAN_NUM; can_id++)
    {
        /* interrupt deinit */
        InterruptMask(can[can_id].config.irq_num);   
        /* can deinit */
        FCanDeInitialize(&can[can_id]);
    }
    
    FIOMuxDeInit();/*init iomux */
    /* print message on example run result */
    if (ret == FT_SUCCESS && can0_recv_irq_flag == 1 && can1_recv_irq_flag == 1)
    {
        printf("%s@%d: can interrupt loopback mode example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: can interrupt loopback mode example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
        return FCAN_FAILURE;
    }

    return ret;
}