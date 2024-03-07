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
 * FilePath: wdt_polled_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for wdt polled example function implmentation
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
#include "fgeneric_timer.h"
#include "fassert.h"
#include "fparameters.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fwdt.h"
#include "fwdt_hw.h"
#include "fsleep.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "wdt_polled_example.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
/*wdt control handle*/
static FWdtCtrl wdt_ctrl;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
/* function of wdt polled example*/
int FWdtPolledExample()
{
     FError ret = 0;
    FWdtIdentifier wdt_identify;

    memset(&wdt_ctrl, 0, sizeof(wdt_ctrl));
    FWdtConfig pconfig = *FWdtLookupConfig(WDT_POLLED_TEST_ID);
    /* wdt init, include reset and read */
    ret = FWdtCfgInitialize(&wdt_ctrl, &pconfig);
    if (FWDT_SUCCESS != ret)
    {
        printf("The wdt init failed\n");
        return FWDT_NOT_READY;
    }

    memset(&wdt_identify, 0, sizeof(wdt_identify));
    ret = FWdtReadFWdtReadWIIDR(&wdt_ctrl, &wdt_identify);
    if (FWDT_SUCCESS != ret)
    {
        printf("The wdt init failed!!!\n");
        return FWDT_NOT_READY;
    }
    else
    {
        printf("The wdt version = %#x, continuation_code=%#x, identity_code=%#x\n ",
               wdt_identify.version,
               wdt_identify.continuation_code,
               wdt_identify.identity_code);
    }
    
    /*set the timeout*/
    ret = FWdtSetTimeout(&wdt_ctrl, WDT_POLLED_TEST_TIMEOUT);
    if (FWDT_SUCCESS != ret)
    {
        printf("set timeout failed\n");
        return FWDT_ERR_INVAL_PARM;
    }

    /*watchdog start timing*/
    FWdtStart(&wdt_ctrl);

    for (int i = 0; i < 5; i++)
    {
        fsleep_seconds(2);
        FWdtRefresh(&wdt_ctrl);
        u32 seconds = GenericTimerRead(GENERIC_TIMER_ID0) / GenericTimerFrequecy();
        printf("Wdt time seconds: %d\r\n", seconds);
    }

    /*watchdog stop timing*/
    FWdtStop(&wdt_ctrl);
    /* watchdog deinit */
    FWdtDeInitialize(&wdt_ctrl);

    if (FT_SUCCESS == ret)
    {
        printf("%s@%d: wdt polled example test [success].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: wdt polled example test [failure].\r\n", __func__, __LINE__);
    }

    return ret;
}