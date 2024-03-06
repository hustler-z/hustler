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
 * FilePath: tacho_pulse_capture_count_example.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for tacho pulse capture count example definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/5/22   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "ftypes.h"
#include "ferror_code.h"
#include "ftimer_tacho.h"
#include "fsleep.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "tacho_common.h"
#include "tacho_pulse_capture_count_example.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
/* function of tacho pulse capture count example */

int FTachoPulseCaptureCountExample()
{
    FError ret = FT_SUCCESS;

    /*init capture mode*/
    ret = FTachoFunctionInit(TIMER_TACHO_ID, 0);
    if (FTIMER_TACHO_SUCCESS != ret)
    {
        return ret;
    }
    for (size_t i = 0; i < 5; i++)
    {
        FTachoPrintCaptureCnt();
        fsleep_millisec(20 * (i+1));
    }
    fsleep_millisec(800);

    /*interrupt deinit and tacho control deinit*/
    FTachoDeinitTest();

    if (FT_SUCCESS == ret)
    {
        printf("%s@%d: tacho pulse capture count example test successfully !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: tacho pulse capture count example test failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
    }
    return 0;
}