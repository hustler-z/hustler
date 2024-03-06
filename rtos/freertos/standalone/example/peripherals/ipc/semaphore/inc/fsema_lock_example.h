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
 * FilePath: fsema_lock_example.h
 * Date: 2023-05-25 16:03:21
 * LastEditTime: 2023-05-31 15:58:02
 * Description:  This file is for semaphore lock example function definition. 
 * 
 * Modify History:
 *  Ver    Who         Date            Changes
 * -----  ------      --------    --------------------------------------
 *  1.0  liuzhihong   2023/5/26      first release
 */


#ifndef  FSEMA_LOCK_EXAMPLE_H
#define  FSEMA_LOCK_EXAMPLE_H

/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "strto.h"
#include "fkernel.h"

#include "sdkconfig.h"
#include "ftypes.h"
#include "fdebug.h"
#include "fassert.h"
#include "fparameters.h"

#include "fsleep.h"
#include "fsemaphore.h"
#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* entry function for fsema example */
int FSemaLockExample();

#ifdef __cplusplus
}
#endif

#endif