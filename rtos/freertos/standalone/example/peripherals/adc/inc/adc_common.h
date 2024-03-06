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
 * FilePath: adc_common.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for adc common definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/2/17   first release
 */
#ifndef  ADC_COMMON_H
#define  ADC_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "fdebug.h"

#define FADC_TEST_DEBUG_TAG "FADC_TEST"
#define FADC_TEST_DEBUG(format, ...) FT_DEBUG_PRINT_D(FADC_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FADC_TEST_INFO(format, ...) FT_DEBUG_PRINT_I(FADC_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FADC_TEST_WARRN(format, ...) FT_DEBUG_PRINT_W(FADC_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FADC_TEST_ERROR(format, ...) FT_DEBUG_PRINT_E(FADC_TEST_DEBUG_TAG, format, ##__VA_ARGS__)

#define REF_VOL 1.25             /* E2000 TESTC board, ADC_VREF = 1.25V */
#define DEFAULT_RESOLUTION  10   /*default resolution 10 bit*/

#ifdef __cplusplus
}
#endif

#endif