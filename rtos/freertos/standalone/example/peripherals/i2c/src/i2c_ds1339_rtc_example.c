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
 * FilePath: i2c_ds1339_rtc_example.c
 * Date: 2022-06-23 17:14:36
 * LastEditTime: 2022-06-23 17:14:36
 * Description:  This file is for providing functions to file cmd_rtc_ds1339.c 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming    2022/06/23     init
 */

#include <string.h>
#include <stdio.h>
#include "ftypes.h"
#include "fdebug.h"
#include "fsleep.h"
#include "i2c_ds1339_rtc_example.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fparameters.h"
#include "fi2c.h"
#include "fi2c_hw.h"
#include "fmio_hw.h"
#include "fmio.h"
#include "ferror_code.h"
#include "fio_mux.h"

#ifndef CONFIG_TARGET_E2000
    #error "This example support only E2000 D/Q/S !!!"
#endif

#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD) || defined(CONFIG_FIREFLY_DEMO_BOARD)
#define DS_1339_MIO FMIO9_ID
#else
#define DS_1339_MIO FMIO0_ID
#endif

#define DS_1339_ADDR 0x68
#define FDSRTC_SUCCESS FT_SUCCESS
#define RTC_TEST_DEBUG_TAG "RTC_TEST"

#define RTC_TEST_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(RTC_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define RTC_TEST_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(RTC_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define RTC_TEST_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(RTC_TEST_DEBUG_TAG, format, ##__VA_ARGS__)

static FI2c master_device;
static FMioCtrl rtc_ds1339;

u8(*bcd_date)[7] = {0};

static FError DsRtcDateTest(FRtcDateTimer *rtc_time)
{
    FASSERT(rtc_time != NULL);

    /* Check validity of seconds value */
    if (rtc_time->second > 59)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of minutes value */
    if (rtc_time->minute > 59)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of day of week value */
    if (rtc_time->day_of_week < 1 || rtc_time->day_of_week > 7)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of hours value */
    if (rtc_time->hour > 23)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of day of month value */
    if (rtc_time->day_of_month < 1 || rtc_time->day_of_month > 31)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of month value */
    if (rtc_time->month < 1 || rtc_time->month > 12)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of year value */
    if (rtc_time->year > 2099)
    {
        return FI2C_ERR_INVAL_PARM;
    }
    return FDSRTC_SUCCESS;
}

FError FDs1339RtcInit(void)
{
    FError ret = FDSRTC_SUCCESS;
    FMioCtrl *pctrl = &rtc_ds1339;
    const FMioConfig *mioconfig_p ;
    FI2c *instance_p = &master_device;
    const FI2cConfig *i2cconfig_p;
    FI2cConfig i2cconfig;

    /* init mio fuction */
    FIOMuxInit();
    FIOPadSetMioMux(DS_1339_MIO);
    mioconfig_p = FMioLookupConfig(DS_1339_MIO);
    if (NULL == mioconfig_p)
    {
        RTC_TEST_DEBUG_E("Mio error inval parameters.\r\n");
        return FMIO_ERR_INVAL_PARM;
    }

    pctrl->config = *mioconfig_p;
    ret = FMioFuncInit(pctrl, FMIO_FUNC_SET_I2C);
    if (ret != FDSRTC_SUCCESS)
    {
        RTC_TEST_DEBUG_E("DS1339 MioInit error.\r\n");
        return ret;
    }

    /* get standard config of i2c */
    i2cconfig_p = FI2cLookupConfig(FI2C0_ID);
    /* Modify configuration */
    i2cconfig = *i2cconfig_p;
    i2cconfig.base_addr = FMioFuncGetAddress(pctrl, FMIO_FUNC_SET_I2C);
    i2cconfig.irq_num = FMioFuncGetIrqNum(pctrl, FMIO_FUNC_SET_I2C);
    i2cconfig.slave_addr = DS_1339_ADDR;

    FI2cDeInitialize(instance_p);
    /* Initialization */
    ret = FI2cCfgInitialize(instance_p, &i2cconfig);
    if (ret != FDSRTC_SUCCESS)
    {
        return ret;
    }
    /*  callback function for FI2C_MASTER_INTR_EVT interrupt  */
    instance_p->master_evt_handlers[FI2C_EVT_MASTER_TRANS_ABORTED] = NULL;
    instance_p->master_evt_handlers[FI2C_EVT_MASTER_READ_DONE] = NULL;
    instance_p->master_evt_handlers[FI2C_EVT_MASTER_WRITE_DONE] = NULL;

    RTC_TEST_DEBUG_I("Set target slave_addr: 0x%x with mio-%d.\r\n", instance_p->config.slave_addr, DS_1339_MIO);
    RTC_TEST_DEBUG_I("Base_addr:0x%x,IRQ_num:%d.\r\n", instance_p->config.base_addr, instance_p->config.irq_num);
    return ret;
}

/**
 * @name:FDs1339RtcSet
 * @msg:Set RTC data from DS1339
 * @return {*}
 * @param {FRtcDateTimer*} rtc_time
 */
FError FDs1339RtcSet(FRtcDateTimer *rtc_time)
{
    FASSERT(rtc_time != NULL);
    FError ret = FDSRTC_SUCCESS;
    FI2c *instance_p = &master_device;
    /*
    data_buf[0] u8 second;
    data_buf[1] u8 minute;
    data_buf[2] u8 hour;
    data_buf[3] u8 monthday;
    data_buf[4] u8 weekday;
    data_buf[5] u8 monCent;
    data_buf[6] u8 year;
    */
    u8 century;
    u8 data_buf[7] = {0};

    ret = DsRtcDateTest(rtc_time);
    if (FDSRTC_SUCCESS != ret)
    {
        return ret;
    }

    data_buf[0] = BIN_TO_BCD(rtc_time->second);

    data_buf[1] = BIN_TO_BCD(rtc_time->minute);

    data_buf[2] = BIN_TO_BCD(rtc_time->hour);

    data_buf[3] = BIN_TO_BCD(rtc_time->day_of_week + 1);

    data_buf[4] = BIN_TO_BCD(rtc_time->day_of_month);

    if (rtc_time->year >= 2000)
    {
        century = 0x80;
    }
    else
    {
        century = 0x0;
    }
    data_buf[5] = (BIN_TO_BCD(rtc_time->month) | century);

    data_buf[6] = BIN_TO_BCD(rtc_time->year % 100);

    /*FI2cMasterWriteIntr*/
    ret = FI2cMasterWritePoll(instance_p, 0, 1, data_buf, sizeof(data_buf));
    if (ret != FDSRTC_SUCCESS)
    {
        RTC_TEST_DEBUG_E("DS1339  FI2cMasterWritePoll error.\r\n");
        return ret;
    }

    return ret;
}

/**
 * @name:FDs1339RtcGet
 * @msg:Get RTC data from DS1339
 * @return {*}
 * @param {FRtcDateTimer*} rtc_time
 */
FError FDs1339RtcGet(FRtcDateTimer *rtc_time)
{
    FASSERT(rtc_time != NULL);
    FError ret = FDSRTC_SUCCESS;
    FI2c *instance_p = &master_device;
    /*
    data_buf[0] u8 second;
    data_buf[1] u8 minute;
    data_buf[2] u8 hour;
    data_buf[3] u8 monthday;
    data_buf[4] u8 weekday;
    data_buf[5] u8 monCent;
    data_buf[6] u8 year;
    */
    u8 century;
    u8 data_buf[7] = {0};

    /*FI2cMasterWriteIntr*/
    ret = FI2cMasterReadPoll(instance_p, 0, 1, data_buf, sizeof(data_buf));
    if (ret != FDSRTC_SUCCESS)
    {
        RTC_TEST_DEBUG_E("DS1339  FI2cMasterReadPoll error.\r\n");
        return ret;
    }

    rtc_time->second = BCD_TO_BIN(data_buf[DS1339_SEC_REG] & 0x7F);

    rtc_time->minute = BCD_TO_BIN(data_buf[DS1339_MIN_REG] & 0x7F);

    rtc_time->hour = BCD_TO_BIN(data_buf[DS1339_HOUR_REG] & 0x3F);

    rtc_time->day_of_week = BCD_TO_BIN((data_buf[DS1339_DAY_REG] - 1) & 0x07);

    rtc_time->day_of_month = BCD_TO_BIN(data_buf[DS1339_DATE_REG] & 0x3F);

    rtc_time->month = BCD_TO_BIN(data_buf[DS1339_MONTH_REG] & 0x1F);

    if (data_buf[DS1339_MONTH_REG] & 0x80)
    {
        rtc_time->year = BCD_TO_BIN(data_buf[DS1339_YEAR_REG]) + 2000;
    }
    else
    {
        rtc_time->year = BCD_TO_BIN(data_buf[DS1339_YEAR_REG]) + 1900;
    }

    return ret;
}

/**
 * @name: FI2cRtcExample
 * @msg: Init ds1339,set default data and read data from chip
 * @return {FError}
 * @note: 
 */
FError FI2cRtcExample(void)
{
    FRtcDateTimer date_time = {2023, 07, 13, 4, 10, 38, 30};
    FError ret = FDSRTC_SUCCESS;
    int exit_time = 5;
    ret = FDs1339RtcInit();
    if (ret != FDSRTC_SUCCESS)
    {
        RTC_TEST_DEBUG_E("FDs1339RtcInit error.");
        goto err;
    }
    printf("Set:year: %d, month: %d, day: %d,week: %d, hour: %d, minute: %d, second: %d\r\n",
            date_time.year,
            date_time.month,
            date_time.day_of_month,
            date_time.day_of_week,
            date_time.hour,
            date_time.minute,
            date_time.second);
    ret = FDs1339RtcSet(&date_time);
    if (ret != FDSRTC_SUCCESS)
    {
        RTC_TEST_DEBUG_E("FDs1339RtcSet error.");
        goto err;
    }
    printf("Set success!!!\r\n");
    while (exit_time-- > 0)
    {
        ret = FDs1339RtcGet(&date_time);
        if (FDSRTC_SUCCESS != ret)
        {
            RTC_TEST_DEBUG_E("Get date time failed 0x%x\r\n", ret);
            break;
        }
        printf("date_time: %d-%d-%d week:%d time:%d:%d:%d\r\n",
                date_time.year,
                date_time.month,
                date_time.day_of_month,
                date_time.day_of_week,
                date_time.hour,
                date_time.minute,
                date_time.second); /* 从FRtcGetDateTime获取当前时间 */
        fsleep_millisec(1000);
    }

    /* print message on example run result */
err:
    if (0 == ret)
    {
        printf("%s@%d: I2C DS1339 RTC example [success]\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: I2C DS1339 RTC example [failure]\r\n", __func__, __LINE__);
    }

    return 0;
}
