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
 * FilePath: qspi_common.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for qspi common definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/3/8   first release
 */
#ifndef  QSPI_COMMON_H
#define  QSPI_COMMON_H

#include "fdebug.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FQSPI_DEBUG_TAG "FQSPI_TEST"
#define FQSPI_ERROR(format, ...)    FT_DEBUG_PRINT_E(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_WARN(format, ...)     FT_DEBUG_PRINT_W(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_INFO(format, ...)     FT_DEBUG_PRINT_I(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_DEBUG(format, ...)    FT_DEBUG_PRINT_D(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)

#define FLASH_WR_OFFSET SZ_1M 

#define DAT_LENGTH 64

#ifdef __cplusplus
}
#endif

#endif

