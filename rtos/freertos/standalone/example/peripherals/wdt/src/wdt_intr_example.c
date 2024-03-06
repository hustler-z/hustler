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
 * FilePath: wdt_intr_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for wdt interrupt example function implmentation
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

#include "wdt_intr_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
/*wdt control handle*/
static FWdtCtrl wdt_ctrl;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
static void FWdtHandler(s32 vector, void *param)
{
    FASSERT(param != NULL);
    FWdtRefresh((FWdtCtrl *)param);
    u32 seconds = GenericTimerRead(GENERIC_TIMER_ID0) / GenericTimerFrequecy();
    printf("Wdt interrupt time seconds: %d\r\n", seconds);
}

static void FWdtIntrEnable()
{
    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(wdt_ctrl.config.irq_num, cpu_id);

    /* interrupt init */
    FWdtConfig *pconfig = &(wdt_ctrl.config);
    InterruptSetPriority(pconfig->irq_num, pconfig->irq_prority);
    InterruptInstall(pconfig->irq_num, FWdtHandler, (void *)&wdt_ctrl, pconfig->instance_name);
    InterruptUmask(pconfig->irq_num);

    #if defined(TARDIGRADE)
    InterruptSetTargetCpus(wdt_ctrl.config.irq_twice_num, cpu_id);

    /* twice interrupt init */
    InterruptSetPriority(pconfig->irq_num, pconfig->irq_twice_prority);
    InterruptInstall(pconfig->irq_twice_num, FWdtHandler, (void *)&wdt_ctrl, pconfig->instance_name);
    InterruptUmask(pconfig->irq_twice_num);
    #endif
}

int FWdtIntrExample()
{
    FError ret = 0;
    FWdtIdentifier wdt_identify;

    memset(&wdt_ctrl, 0, sizeof(wdt_ctrl));
    FWdtConfig pconfig = *FWdtLookupConfig(WDT_INTR_TEST_ID);
    /* wdt init, include reset and read */
    ret = FWdtCfgInitialize(&wdt_ctrl, &pconfig);
    if (FWDT_SUCCESS != ret)
    {
        printf("The wdt init failed\n");
        return FWDT_NOT_READY;
    }

    memset(&wdt_identify, 0, sizeof(wdt_identify));
    /*read wdt version and code*/
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

    /*interrupt init*/
    FWdtIntrEnable();

    /*set the timeout*/
    ret = FWdtSetTimeout(&wdt_ctrl, WDT_INTR_TEST_TIMEOUT);
    if (FWDT_SUCCESS != ret)
    {
        printf("set timeout failed\n");
        return FWDT_ERR_INVAL_PARM;
    }
    /*watchdog start timing*/
    FWdtStart(&wdt_ctrl);
    /*open the watchdog for 8 seconds*/
    fsleep_seconds(8);
    /*watchdog stop timing*/
    FWdtStop(&wdt_ctrl);
    /* interrupt deinit */
    InterruptMask(wdt_ctrl.config.irq_num);
    /* watchdog deinit */
    FWdtDeInitialize(&wdt_ctrl);

    if (FT_SUCCESS == ret)
    {
        printf("%s@%d: wdt interrupt example test [success].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: wdt interrupt example test [failure].\r\n", __func__, __LINE__);
    }

    return ret;
}