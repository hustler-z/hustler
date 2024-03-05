/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: i2s_dp_init.c
 * Created Date: 2023-10-07 16:48:37
 * Last Modified: 2023-12-21 16:21:06
 * Description:  This file is for the I2S DP init example function implmentation.
 *
 * Modify History:
 *  Ver       Who             Date         Changes
 * -----   ----------       --------     ---------------------------------
 *  1.0   wangzongqiang    2023/07/23    init
 *  1.1   liqiaozhong      2023/12/19    solve bdl miss intr issue
 */

#include <string.h>
#include <stdio.h>
#include "strto.h"

#include "fparameters.h"

#include "fparameters_comm.h"

#include "finterrupt.h"
#include "fcache.h"
#include "fmemory_pool.h"
#include "fcpu_info.h"

#include "fdcdp.h"
#include "fdc.h"
#include "fdp_hw.h"
#include "fdc_hw.h"
#include "fdc_common_hw.h"

#include "i2s_dp_init.h"
/***************** Macros (Inline Functions) Definitions *********************/
static FDcDp dcdp_config;
static u8 *static_frame_buffer_address = (u8 *)0xb0000000;
/***************** functions*************************************************/
/**
 * @name: FMediaCtrlProbe
 * @msg:  init the media control
 * @return ret,FMEDIA_DP_SUCCESS means success
 */
FError FMediaCtrlProbe(void)
{
    FError ret;
    u32 index;

    FDcDpCfgInitialize(&dcdp_config);
    for (index = 0; index < FDCDP_INSTANCE_NUM; index ++)
    {
        dcdp_config.dc_instance_p[index].config = *FDcLookupConfig(index);
        dcdp_config.dp_instance_p[index].config = *FDpLookupConfig(index);
    }

    return ret;
}

/**
 * @name: FMediaInit
 * @msg:  init the media control
 * @return ret,FMEDIA_DP_SUCCESS means success
 */
FError FMediaInit(void)
{
    FError ret = FMEDIA_DCDP_SUCCESS;
    u32 index;
    u32 start_index;
    u32 end_index;

    /*设置用户参数*/
    u32 channel = 2;/* 0 or 1 or 2*/
    if (channel == FDCDP_INSTANCE_NUM)
    {
        start_index = 0;
        end_index = FDCDP_INSTANCE_NUM;
    }
    else
    {
        start_index = channel;
        end_index = channel + 1;
    }
    for (index = start_index; index < end_index; index ++)
    {
        dcdp_config.user_config[index].color_depth = 32;
        dcdp_config.user_config[index].width = 640;
        dcdp_config.user_config[index].height = 480;
        dcdp_config.user_config[index].refresh_rate = 60;
        dcdp_config.user_config[index].multi_mode = 0;
        dcdp_config.user_config[index].fb_phy = (uintptr)static_frame_buffer_address;
        dcdp_config.user_config[index].fb_virtual = (uintptr)static_frame_buffer_address;
    }

    ret = FMediaCtrlProbe();
    if (ret != FMEDIA_DCDP_SUCCESS)
    {
        printf("FMediaCtrlProbe probe error.\r\n");
    }

    ret = FDcDpInitialize(&dcdp_config, channel);
    if (ret != FMEDIA_DCDP_SUCCESS)
    {
        printf("FDcDpInitialize probe error.\r\n");
    }

    return ret;
}