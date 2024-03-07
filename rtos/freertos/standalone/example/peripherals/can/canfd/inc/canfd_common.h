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
 * FilePath: canfd_common.h
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for canfd common definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/2/17   first release
 * 1.1   huangjin   2023/11/01  add macro definition
 */
#ifndef  CAN_COMMON_H
#define  CAN_COMMON_H

#include "fdebug.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FCAN_TEST_DEBUG_TAG "FCAN_TEST"
#define FCAN_TEST_DEBUG(format, ...) FT_DEBUG_PRINT_D(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_INFO(format, ...) FT_DEBUG_PRINT_I(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_WARN(format, ...) FT_DEBUG_PRINT_W(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_ERROR(format, ...) FT_DEBUG_PRINT_E(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)

#define CAN_TEST_SEND_ID 0x23
#define CAN_TEST_SEND_LENGTH 8
#define CAN_TEST_SEND_STID_LENGTH 8
#define CAN_TEST_SEND_EXID_LENGTH 16
#define CAN_LOOPBACK_TEST_TIMES 30
#define CAN_LOOPBACK_TEST_PERIAD_MS 100
#define CAN_ID_FILTER_TEST_TIMES 2

#define CANFD_TEST_ARB_BAUD_RATE 500000
#define CANFD_TEST_DATA_BAUD_RATE 500000

#ifdef __cplusplus
}
#endif

#endif