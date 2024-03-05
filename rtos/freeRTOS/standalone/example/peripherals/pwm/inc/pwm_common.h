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
 * FilePath: pwm_common.h
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for pwm common definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/3/14   first release
 */
#ifndef  PWM_COMMON_H
#define  PWM_COMMON_H

#include "fdebug.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FPWM_DEBUG_TAG "PWM_TEST"
#define FPWM_ERROR(format, ...)     FT_DEBUG_PRINT_E(FPWM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPWM_WARN(format, ...)     FT_DEBUG_PRINT_W(FPWM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPWM_INFO(format, ...)      FT_DEBUG_PRINT_I(FPWM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPWM_DEBUG(format, ...)     FT_DEBUG_PRINT_D(FPWM_DEBUG_TAG, format, ##__VA_ARGS__)

/* pwm pulse change times */
#define PWM_PULSE_CHANGE_TIME   5

/* pwm pulse amplitude of periodic variation */
#define PWM_PULSE_CHANGE    1000

#ifdef  CONFIG_TARGET_PHYTIUMPI
#define PWM_TEST_ID FPWM2_ID
#else
#define PWM_TEST_ID FPWM6_ID
#endif

#ifdef __cplusplus
}
#endif

#endif