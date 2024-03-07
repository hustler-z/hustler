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
 * FilePath: fmedia_os.c
 * Date: 2022-09-15 14:20:19
 * LastEditTime: 2022-09-21 16:59:51
 * Description:  This file is for providing the media driver
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  Wangzq     2022/12/20  Modify the format and establish the version
 */
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "ftypes.h"
#include "fdebug.h"
#include "fparameters_comm.h"
#include "fmedia_os.h"
#include "fdcdp.h"
#include "fdp_hw.h"
#include "fdp.h"
#include "fdc_common_hw.h"

/***************** Macros (Inline Functions) Definitions *********************/

#define FMEDIA_DEBUG_TAG "FFreeRTOSMEDIA"
#define FMEDIA_ERROR(format, ...) FT_DEBUG_PRINT_E(FMEDIA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEDIA_INFO(format, ...) FT_DEBUG_PRINT_I(FMEDIA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEDIA_DEBUG(format, ...) FT_DEBUG_PRINT_D(FMEDIA_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/**
 * @name: FFreeRTOSMediaHwInit
 * @msg:  init the media,dc and dp
 * @param  {u32} channel is the dc channel
 * @param  {u32} width is the width
 * @param  {u32} height is the height
 * @param  {u32} multi_mode is multi display mode,0:clone,1:hor,2:ver
 * @param  {u32} color_depth is the color depth
 * @param  {u32} refresh_rate is the refresh rate of screen
 * @return err code information, 0 indicates success，others indicates failed
 */
FFreeRTOSMedia *FFreeRTOSMediaHwInit(u32 channel,FFreeRTOSMedia *instance)
{
    FError ret = FT_SUCCESS;
    u32 index;
    FDcDpCfgInitialize(&instance->dcdp_ctrl);

    for (index = 0; index < FDCDP_INSTANCE_NUM; index ++)
    {
        instance->dcdp_ctrl.dc_instance_p[index].config = *FDcLookupConfig(index);
        instance->dcdp_ctrl.dp_instance_p[index].config = *FDpLookupConfig(index);
    }
    ret = FDcDpInitialize(&instance->dcdp_ctrl, channel);
    if (ret != FMEDIA_DP_SUCCESS)
    {
        FMEDIA_ERROR("DcDp initial failed");
        goto err_exit;
    }

err_exit:
    return (FT_SUCCESS == ret) ? instance : NULL; /* exit with NULL if failed */
}

