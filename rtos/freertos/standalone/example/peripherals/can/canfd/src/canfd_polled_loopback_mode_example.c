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
 * FilePath: canfd_polled_loopback_mode_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for canfd polled loopback mode example function implmentation
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
#include "fio_mux.h"
#include "fcan_hw.h"
#include "fparameters.h"
#include "fsleep.h"
#include "fassert.h"
#include "ftypes.h"
#include "canfd_common.h"
#include "canfd_polled_loopback_mode_example.h"
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
static FCanFrame recv_frame;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
int FCanfdPolledLoopbackExample()
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
            FCAN_TEST_ERROR("Canfd%d set arb segment baudrate error.", can_id);
            return FCAN_FAILURE;
        }
        ret = FCanBaudrateSet(&can[can_id], &data_segment_config);
        if (ret != FCAN_SUCCESS)
        {
            FCAN_TEST_ERROR("Canfd%d set data segment baudrate error.", can_id);
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
            /*can enable*/
            FCanEnable(&can[can_id], TRUE);
        }

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
            instance_p = &can[FCAN1_ID];
            ret = FCanRecv(instance_p, &recv_frame);
            if ((ret == FT_SUCCESS) && (send_frame.candlc == recv_frame.candlc))
            {
                for (i = 0; i < send_frame.candlc; i++)
                {
                    if (recv_frame.data[i] != send_frame.data[i])
                    {
                        FCAN_TEST_ERROR("\n Can 1 recv is not equal to can 0 send,times= %d.\r\n", send_times);
                        return FCAN_FAILURE;
                    }
                }  
            }
            else
            {
                FCAN_TEST_ERROR("Recv frame failed.\n");
                return FCAN_FAILURE;
            }
        }
        if(ide_flag == 0)
            printf("Can 1 recv is equal to can 0 send, use standard frame.\n");
        else
            printf("Can 1 recv is equal to can 0 send, use extern frame.\n");    
        /* can 1 send, can 0 receive */
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
            fsleep_millisec(CAN_LOOPBACK_TEST_TIMES);
            instance_p = &can[FCAN0_ID];
            ret = FCanRecv(instance_p, &recv_frame);
            if ((ret == FT_SUCCESS) && (send_frame.candlc == recv_frame.candlc))
            {
                for (i = 0; i < recv_frame.candlc; i++)
                {
                    if (recv_frame.data[i] != send_frame.data[i])
                    {
                        FCAN_TEST_ERROR("\nCan 0 recv is not equal to can 1 send,times= %d\r\n", send_times);
                        return FCAN_FAILURE;
                    }
                }  
            }
            else
            {
                FCAN_TEST_ERROR("Recv frame failed.\n");
                return FCAN_FAILURE;
            }
        }
        if(ide_flag == 0)
            printf("Can 0 recv is equal to can 1 send, use standard frame.\n");    
        else
            printf("Can 0 recv is equal to can 1 send, use extern frame.\n");  
    }
    
    for (can_id = FCAN0_ID; can_id < FCAN_NUM; can_id++)
    {
        /*can deinit */
        FCanDeInitialize(&can[can_id]);
    }

    FIOMuxDeInit();/*init iomux */
    
    /* print message on example run result */
    if (ret == FT_SUCCESS)
    {
        printf("%s@%d: can polled loopback mode example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: can polled loopback mode example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
        return FCAN_FAILURE;
    }

    return ret;
}
