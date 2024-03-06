/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: i2c_ds1339_rtc_example.h
 * Date: 2022-06-23 17:15:16
 * LastEditTime: 2022-06-23 17:15:16
 * Description:  This file is for ds1339 defintion
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming    2022/06/23     init
 */

#ifndef I2C_DS1339_RTC_EXAMPLE_H
#define I2C_DS1339_RTC_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* includes */

#include "ftypes.h"
#include "fi2c.h"
#include "fmio.h"
#include "fmio_hw.h"
#include "sdkconfig.h"
#include "fparameters.h"
/* This structure holds the Real-Time Clock configuration values */

typedef struct
{
    u16 year;               /* year */
    u8 month;           /* month */
    u8 day_of_month;    /* day of month */
    u8 day_of_week;     /* day of week */
    u8 hour;                /* hour */
    u8 minute;          /* minute */
    u8 second;          /* second */
} FRtcDateTimer;

/* defines */
#define FDSRTC_SUCCESS FT_SUCCESS
/*
 * RTC register addresses
 */
#define DS1339_SEC_REG      0x0
#define DS1339_MIN_REG      0x1
#define DS1339_HOUR_REG     0x2
#define DS1339_DAY_REG      0x3
#define DS1339_DATE_REG     0x4
#define DS1339_MONTH_REG    0x5
#define DS1339_YEAR_REG     0x6


/*
 * the following macros convert from BCD to binary and back.
 * Be careful that the arguments are chars, only char width returned.
 */

#define BCD_TO_BIN(bcd) (( ((((bcd)&0xf0)>>4)*10) + ((bcd)&0xf) ) & 0xff)
#define BIN_TO_BCD(bin) (( (((bin)/10)<<4) + ((bin)%10) ) & 0xff)


/* global functions */

FError FI2cRtcExample(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* INCds1339Rtc */
