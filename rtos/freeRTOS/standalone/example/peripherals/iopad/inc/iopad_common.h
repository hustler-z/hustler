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
 * FilePath: iopad_common.h
 * Date: 2023-02-28 14:53:42
 * LastEditTime: 2023-03-05 17:46:03
 * Description:  This file is for iopad common definition.
 *
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----   -------    --------     --------------------------------------
 *  1.0  liqiaozhong  2023/03/05   first commit
 */

#ifndef  IOPAD_COMMON_H
#define  IOPAD_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "fdebug.h"
#include "ftypes.h"
#include "fkernel.h"

#define FIOPAD_TEST_DEBUG_TAG "FIOPAD_TEST"
#define FIOPAD_TEST_DEBUG(format, ...) FT_DEBUG_PRINT_D(FIOPAD_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FIOPAD_TEST_INFO(format, ...) FT_DEBUG_PRINT_I(FIOPAD_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FIOPAD_TEST_WARRN(format, ...) FT_DEBUG_PRINT_W(FIOPAD_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FIOPAD_TEST_ERROR(format, ...) FT_DEBUG_PRINT_E(FIOPAD_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
#ifdef __cplusplus
}
#endif

#endif