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
 * FilePath: iopad_set_config_example.c
 * Date: 2022-03-01 12:46:47
 * LastEditTime: 2022-03-05 14:22:18
 * Description:  This file is for iopad config setting example function implmentation.
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
#include "fsleep.h"
#include "fiopad.h"

#include "iopad_common.h"
#include "iopad_set_config_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

static FIOPadCtrl iopad_ctrl;

static FIOPadFunc  prev_func = FIOPAD_FUNC0;
static FIOPadPull  prev_pull = FIOPAD_PULL_NONE; 
static FIOPadDrive prev_drive = FIOPAD_DRV0;
   
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

static void FPinSetIoPadSetting(u32 pin_reg_off)
{

    FIOPadGetConfig(&iopad_ctrl, pin_reg_off, &prev_func, &prev_pull, &prev_drive);
    FIOPadSetConfig(&iopad_ctrl, pin_reg_off, FIOPAD_FUNC1, FIOPAD_PULL_UP, FIOPAD_DRV1);
    return;
}

static void FPinRecoverIoPadSetting(u32 pin_reg_off)
{
    FIOPadSetConfig(&iopad_ctrl, pin_reg_off, prev_func, prev_pull, prev_drive);
    return;
}

/* function of iopad set config example */
int FIopadModifySettingExample(void)
{
    uintptr iopad_off;

    FIOPadCfgInitialize(&iopad_ctrl, FIOPadLookupConfig(FIOPAD0_ID));
    printf("[uart-1 rx pin] \r\n");
    FPinGetIoPadSetting(FIOPAD_AW47_REG0_OFFSET);

    printf("[uart-1 rx pin] modified \r\n");
    FPinSetIoPadSetting(FIOPAD_AW47_REG0_OFFSET);
    FPinGetIoPadSetting(FIOPAD_AW47_REG0_OFFSET);

    FPinRecoverIoPadSetting(FIOPAD_AW47_REG0_OFFSET);
    printf("[uart-1 rx pin] recovered \r\n");
    FPinGetIoPadSetting(FIOPAD_AW47_REG0_OFFSET);

    FIOPadDeInitialize(&iopad_ctrl);
    printf("%s@%d: set pin setting example success !!! \r\n", __func__, __LINE__);
    printf("Failed if you cannot input char by uart-1 \r\n");
    return 0;
}