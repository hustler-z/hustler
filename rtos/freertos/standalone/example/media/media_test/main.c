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
 * Date: 2023-08-10 14:53:41
 * LastEditTime: 2023-08-10 17:36:17
 * Description:  This file is for  example test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangzongqiang   2023/08/10   first release
 */
/***************************** Include Files *********************************/

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell_port.h"
#endif

#include "shell_port.h"
#include "media_test_example.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
extern void MediaTimerLoop(void);

int main()
{
#ifdef CONFIG_USE_LETTER_SHELL
    /* if shell command is enabled, register example entry as shell command */
   MediaTimerLoop();  
#else
     FMediaExample();
#endif
    return 0;
}
