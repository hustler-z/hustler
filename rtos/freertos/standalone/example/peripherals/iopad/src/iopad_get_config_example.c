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
 * FilePath: iopad_get_config_example.c
 * Date: 2022-03-01 15:56:42
 * LastEditTime: 2022-03-05 18:26:08
 * Description:  This file is for iopad config getting example function implmentation.
 *
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----   -------    --------     --------------------------------------
 *  1.0   zhugengyu  2023/03/05    first commit
 */


/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifndef CONFIG_TARGET_E2000
    #error "This example support only E2000 D/Q/S !!!"
#endif

#include <stdio.h>
#include <string.h>
#include "ftypes.h"
#include "fiopad.h"


#include "iopad_get_config_example.h"
/************************** Constant Definitions *****************************/
#define FPIN_IOPAD_NUM      147U

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FIOPadCtrl iopad_ctrl;
static uintptr beg_off = FIOPAD_REG0_BEG_OFFSET;
static uintptr end_off = FIOPAD_REG0_END_OFFSET;
static const char *pull_state[FIOPAD_NUM_OF_PULL] = {"none", "down", "up"}; 
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
static void FPinGetIoPadSetting(u32 pin_reg_off)
{   
    FIOPadFunc func = FIOPAD_FUNC0;
    FIOPadPull pull = FIOPAD_PULL_NONE; 
    FIOPadDrive drive = FIOPAD_DRV0;

    FIOPadGetConfig(&iopad_ctrl ,pin_reg_off, &func, &pull, &drive);

    printf("[@0x%x] func: %d, ds: %d, pull: %s\r\n",
            pin_reg_off,
            func,
            drive,
            pull_state[pull]);

    return;    
}

static void FPinGetIoPadDelaySetting(u32 pin_reg_off)
{

    FIOPadDelay in_roungh_delay = FIOPadGetDelay(&iopad_ctrl, pin_reg_off, FIOPAD_INPUT_DELAY, FIOPAD_DELAY_COARSE_TUNING);
    FIOPadDelay in_delicate_delay = FIOPadGetDelay(&iopad_ctrl, pin_reg_off, FIOPAD_INPUT_DELAY, FIOPAD_DELAY_FINE_TUNING);
    FIOPadDelay out_roungh_delay = FIOPadGetDelay(&iopad_ctrl, pin_reg_off, FIOPAD_OUTPUT_DELAY, FIOPAD_DELAY_COARSE_TUNING);
    FIOPadDelay out_delicate_delay = FIOPadGetDelay(&iopad_ctrl, pin_reg_off, FIOPAD_OUTPUT_DELAY, FIOPAD_DELAY_FINE_TUNING);    

    printf("[@0x%x] delay(roungh-delicate): in %d-%d, out %d-%d\r\n",
            pin_reg_off,
            in_roungh_delay,
            in_delicate_delay,
            out_roungh_delay, 
            out_delicate_delay);    
    
    return;
}

int FIopadGetCurrentSettingExample(void)
{
    int ret = 0;
    uintptr iopad_off;

    FIOPadCfgInitialize(&iopad_ctrl, FIOPadLookupConfig(FIOPAD0_ID));
    /* iopad is in continuous array */
    for (iopad_off = beg_off; iopad_off <= end_off; iopad_off += sizeof(u32))
    {
        FPinGetIoPadSetting(iopad_off);
    }

    /* iopad is not in continuous array */
    FPinGetIoPadDelaySetting(FIOPAD_AE55_REG1_OFFSET);

    FIOPadDeInitialize(&iopad_ctrl);

    if (0 == ret)
    {
        printf("%s@%d: get pin setting example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: get pin setting example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
    }

    return 0;
}