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
 * FilePath: sdio_sdio_detect_example.h
 * Date: 2023-04-05 15:22:56
 * LastEditTime: 2023-04-07 13:49:25
 * Description:  This file is for sdio card detect example function definition.
 *
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------    --------------------------------------
 *  1.0     zhugengyu  2023/10/23    first commit with detect examples
 */

#ifndef  SDIF_SDIO_DETECT_EXAMPLE_H
#define  SDIF_SDIO_DETECT_EXAMPLE_H

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
/* entry function for TF card detect example */
int FSdifSdioCardDetectExample(void);

#ifdef __cplusplus
}
#endif

#endif
