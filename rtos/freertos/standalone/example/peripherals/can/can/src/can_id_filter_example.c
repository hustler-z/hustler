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
 * FilePath: can_id_filter_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for can id filter example function implmentation
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
#include "fparameters.h"
#include "fsleep.h"
#include "fassert.h"
#include "ftypes.h"
#include "can_common.h"
#include "can_id_filter_example.h"
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
static FCanFrame recv_frame;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
/* function of can id filter example */
int FCanIdFilterExample()
{
    FError ret = FCAN_SUCCESS;
    u32 can_id = FCAN0_ID;
    FCanCtrl *instance_p;
    int filter_flag = 0;
    int send_times = 0;
    int i = 0;
    printf("Use can2.0 protocol!\n");
    
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
        /*disable canfd, configured as can mode*/
        FCanFdEnable(&can[can_id], FALSE);
        /*set to normal mode*/
        FCanSetMode(&can[can_id], FCAN_PROBE_NORMAL_MODE);
    }
    /*Baud rate is configured as 1M*/
    FCanBaudrateConfig arb_segment_config;
    FCanBaudrateConfig data_segment_config;
    memset(&arb_segment_config, 0, sizeof(arb_segment_config));
    memset(&data_segment_config, 0, sizeof(data_segment_config));

    arb_segment_config.baudrate = CAN_TEST_ARB_BAUD_RATE;
    arb_segment_config.auto_calc = TRUE;
    arb_segment_config.segment = FCAN_ARB_SEGMENT;

    data_segment_config.baudrate = CAN_TEST_DATA_BAUD_RATE;
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
            /* set filter,and configured can to receive only ID 0x02 frame*/
            FCanIdMaskConfig id_mask;
            FCanEnable(&can[can_id], FALSE);
            memset(&id_mask, 0, sizeof(id_mask));
            for (int i = 0; i < FCAN_ACC_ID_REG_NUM; i++)
            {
                id_mask.filter_index = i;
                /* 只接收发送id=0x02的帧 */
                id_mask.id = 0x02;
                id_mask.mask = 0;
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

        for (send_times = 0; send_times < CAN_ID_FILTER_TEST_TIMES; send_times++)
        {
            memset(&send_frame, 0, sizeof(FCanFrame));
            /*set the canid = 0x02 first time ,and set the canid = 0x01 second time */
            send_frame.canid = (send_times == 0)? 0x02:0x01;
            send_frame.candlc = CAN_TEST_SEND_STID_LENGTH;

            if (ide_flag == 0)
            {
                send_frame.canid &= CAN_SFF_MASK;
            }
            else
            {
                send_frame.canid |= CAN_EFF_FLAG;
            }

            for (i = 0; i < send_frame.candlc; i++)
            {
                send_frame.data[i] = i + 0x01 ;
            }
            /* can 0 send, can 1 receive */
            instance_p = &can[FCAN0_ID];
            ret = FCanSend(instance_p, &send_frame);
            if (ret != FT_SUCCESS)
            {
                printf("Can%d send %d times error.", FCAN0_ID, send_times);
                return FCAN_FAILURE;
            }
            fsleep_millisec(CAN_LOOPBACK_TEST_PERIAD_MS);
            memset(&recv_frame, 0, sizeof(FCanFrame));
            instance_p = &can[FCAN1_ID];
            ret = FCanRecv(instance_p, &recv_frame);
            if (ret == FT_SUCCESS)
            {
                for (i = 0; i < send_frame.candlc; i++)
                {
                    if (recv_frame.data[i] != send_frame.data[i])
                    {
                        filter_flag = 1;
                    }
                    else
                    {
                        filter_flag = 0;
                    }
                }  
            }
            else
            {
                FCAN_TEST_ERROR("Recv frame failed.\n");
                return FCAN_FAILURE;
            }
            
            if (filter_flag == 1)
            {
                if (ide_flag == 0)
                {
                    if (send_frame.canid == 0x01)
                    {
                        printf("The standard frame id 0x%02x was filtered successfully.\n", send_frame.canid);
                    }
                    else
                    {
                        FCAN_TEST_ERROR("Test failed, the standard frame id 0x%02x should not be filtered!\n",send_frame.canid);
                        ret = FCAN_FAILURE;
                        break;
                    }
                }
                else
                {
                    if (send_frame.canid == 0x80000001)
                    {
                        printf("The extern frame id 0x%02x was filtered successfully.\n", send_frame.canid);
                    }
                    else
                    {
                        FCAN_TEST_ERROR("Test failed, the extern frame id 0x%02x should not be filtered!\n",send_frame.canid);
                        ret = FCAN_FAILURE;
                        break;
                    }
                }
            }
            else
            {
                if (ide_flag == 0)
                {
                    printf("The standard frame id 0x%02x was receved successfully.\n",  send_frame.canid);
                }
                else
                {
                    printf("The extern frame id 0x%02x was receved successfully.\n",  send_frame.canid);
                }
            }
        }
    }

    for (can_id = FCAN0_ID; can_id < FCAN_NUM; can_id++)
    {
        /* can deinit */
        FCanDeInitialize(&can[can_id]);
    }

    FIOMuxDeInit();/*init iomux */

    /* print message on example run result */
    if (ret == FT_SUCCESS)
    {
        printf("%s@%d: can id filter example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: can id filter example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
        return FCAN_FAILURE;
    }

    return ret;
}
