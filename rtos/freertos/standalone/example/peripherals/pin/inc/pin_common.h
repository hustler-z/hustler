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
 * FilePath: pin_common.h
 * Date: 2023-02-28 14:53:42
 * LastEditTime: 2023-03-05 17:46:03
 * Description:  This file is for pin common definition.
 *
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----   -------    --------     --------------------------------------
 *  1.0  liqiaozhong  2023/03/05   first commit
 */

#ifndef  PIN_COMMON_H
#define  PIN_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "fdebug.h"
#include "ftypes.h"
#include "fkernel.h"

#include "fgpio.h"

#define FPIN_TEST_DEBUG_TAG "FPIN_TEST"
#define FPIN_TEST_DEBUG(format, ...) FT_DEBUG_PRINT_D(FPIN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPIN_TEST_INFO(format, ...) FT_DEBUG_PRINT_I(FPIN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPIN_TEST_WARRN(format, ...) FT_DEBUG_PRINT_W(FPIN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPIN_TEST_ERROR(format, ...) FT_DEBUG_PRINT_E(FPIN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
#ifdef __cplusplus
}
#endif

#endif