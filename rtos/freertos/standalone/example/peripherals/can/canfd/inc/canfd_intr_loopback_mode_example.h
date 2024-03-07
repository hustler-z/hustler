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
 * FilePath: canfd_intr_loopback_mode_example.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for canfd interrupt loopback mode example function definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/2/24   first release
 */

#ifndef  CANFD_INTR_LOOPBACK_MODE_EXAMPLE_H
#define  CANFD_INTR_LOOPBACK_MODE_EXAMPLE_H

/***************************** Include Files *********************************/
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* function for canfd interrupt loopback mode example */
int FCanfdIntrLoopbackExample();

#ifdef __cplusplus
}
#endif

#endif