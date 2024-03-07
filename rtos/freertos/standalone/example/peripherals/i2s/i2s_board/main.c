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
 * FilePath: main.c
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:36:17
 * Description:  This file is for the I2S board example main function.
 *
 * Modify History:
 *  Ver       Who             Date         Changes
 * -----   ----------       --------     ---------------------------------
 *  1.0   wangzongqiang    2023/07/23    init
 *  1.1   liqiaozhong      2023/12/19    solve bdl miss intr issue
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell_port.h"
#endif

#include "fsleep.h"

#include "i2s_tx_example.h"
#include "i2s_rx_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
int main()
{
#ifdef CONFIG_USE_LETTER_SHELL
    /* if shell command is enabled, register example entry as shell command */
    LSUserShellLoop();    
#else
    FI2sDdmaRxExample();
    fsleep_seconds(5); /* 此处延时为了确保能够接收到有效的音频 */

    FI2sDdmaTxExample();
    fsleep_seconds(5); /* 此处延时为了确保能够输出完整的音频 */

    FI2sDdmaRxStopWork();
    FI2sDdmaTxStopWork();
#endif
    return 0;
}
