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
 * FilePath: timer_cycle_timing_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for timer cycle timing example function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/5/22   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "ftimer_tacho.h"
#include "ftypes.h"
#include "fsleep.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "timer_common.h"
#include "timer_cycle_timing_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/

int FTimerCycleTimingExample()
{
    FError ret = 0;
    u32 id = TIMER_EXAMPLE_TEST_ID;

    printf("Cyc timer id:%d start... \r\n", id);
    /*1 second cycle timing*/
    ret = FTimerFunctionInit(id, 1, 1000000); 
    if (FTIMER_TACHO_SUCCESS != ret)
    {
        return ret;
    }

    /*Using initialization values, immediately take effect timer*/
    ret = FTimerStartTest(0, TRUE);
    if (FTIMER_TACHO_SUCCESS != ret)
    {
        return ret;
    }
    /*5.1s*/
    fsleep_millisec(5100);

    /*interrupt deinit and timer control deinit*/
    FTimerDeInitTest();

    /*print message on example run result */
    if (FTIMER_TACHO_SUCCESS == ret)
    {
        printf("%s@%d: timer cycle timing example test [success].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: timer cycle timing example test [failure].\r\n", __func__, __LINE__);
    }

    return 0;
}