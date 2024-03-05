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
 * FilePath: serial_ddma_example.h
 * Date: 2023-04-12 14:53:42
 * LastEditTime: 2023-06-21 17:57:24
 * Description:  This file is for serial transfer under control of DDMA example function definition.
 *
 * Modify History:
 *  Ver     Who          Date             Changes
 * -----   ------      --------    --------------------------------------
 *  1.0   liuzhihong   2023/4/12        first release
 *  1.1   liqiaozhong  2023/6/21        add flow, mio, ddma related examples
 */

#ifndef  SERIAL_DDMA_EXAMPLE_H
#define  SERIAL_DDMA_EXAMPLE_H
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "ftypes.h"
#include "fdebug.h"
#include "fsleep.h"
#include "fcpu_info.h"
#include "finterrupt.h"
#include "fparameters.h"

#include "fpl011.h"
#include "fddma.h"
#include "fddma_hw.h"

#ifdef __cplusplus
extern "C" 
{
#endif
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
/* entry function for serial transfer under control of DDMA example */
int FSerialDdmaExample(void);

#ifdef __cplusplus
}
#endif

#endif