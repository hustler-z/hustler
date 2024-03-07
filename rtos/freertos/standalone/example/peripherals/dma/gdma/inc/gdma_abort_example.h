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
 * FilePath: gdma_abort_example.h
 * Created Date: 2023-10-26 15:51:02
 * Last Modified: 2023-10-27 15:23:29
 * Description:  This file is for the gdma abort example function definition.
 *
 * Modify History:
 *  Ver      Who         Date         Changes
 * -----   ------      --------     --------------------------------------
 *  1.0  liqiaozhong   2023/3/28    first commit
 *  2.0  liqiaozhong   2023/10/20   adapt to modified driver
 */

#ifndef GDMA_ABORT_EXAMPLE_H
#define GDMA_ABORT_EXAMPLE_H
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
/* entry function */
int FGdmaAbortExample(void);

#ifdef __cplusplus
}
#endif

#endif