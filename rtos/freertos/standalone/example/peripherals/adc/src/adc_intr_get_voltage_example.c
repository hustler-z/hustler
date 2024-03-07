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
 * FilePath: adc_intr_get_voltage_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for test adc interrupt mode get voltage example functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/2/17   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "fgeneric_timer.h"
#include "ferror_code.h"
#include "finterrupt.h"
#include "fparameters.h"
#include "fsleep.h"
#include "fadc.h"
#include "fadc_hw.h"
#include "fcpu_info.h"
#include "ftypes.h"
#include "strto.h"
#include "adc_common.h"
#include "adc_intr_get_voltage_example.h"
#include "fio_mux.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/* variables used in example */
static FAdcCtrl adc_ctrl;
static FAdcConfig adc_config;

/************************** Function *****************************************/

static void FAdcCovfinIrqCallback(void *param)
{
    FAdcCtrl *pctrl = (FAdcCtrl *)param;
    for (int channel_num = 0; channel_num < FADC_CHANNEL_NUM; channel_num++)
    {
        if(pctrl->convert_complete[channel_num] == 1)
        {
            FAdcInterruptDisable(pctrl, channel_num, FADC_INTR_EVENT_COVFIN);
        }
    }
}

static void FAdcIrqSet(FAdcCtrl *instance_p)
{
    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(instance_p->config.irq_num, cpu_id);

    FAdcRegisterInterruptHandler(instance_p, FADC_INTR_EVENT_COVFIN, FAdcCovfinIrqCallback, (void *)instance_p);

    InterruptSetPriority(instance_p->config.irq_num, 0);
    InterruptInstall(instance_p->config.irq_num, FAdcIntrHandler, instance_p, "adc");
    InterruptUmask(instance_p->config.irq_num);

    return;
}

/* enable adc convert finish intr*/
static void FAdcIrqAllEnable(FAdcCtrl *instance_p)
{
    int channel = 0;
    FAdcIntrEventType event_type = FADC_INTR_EVENT_COVFIN;

    for (channel = 0; channel < FADC_CHANNEL_NUM; channel++)
    {
        FAdcInterruptEnable(instance_p, channel, event_type);
    }
}

/* disable all adc convert finish intr */
static void FAdcIrqAllDisable(FAdcCtrl *instance_p)
{
    int channel = 0;
    FAdcIntrEventType event_type = FADC_INTR_EVENT_COVFIN;

    for (channel = 0; channel < FADC_CHANNEL_NUM; channel++)
    {
        FAdcInterruptDisable(instance_p, channel, event_type);
    }
}

/* function of adc intr mode get vol example */
int FAdcIntrReadExample(u32 adc_id)
{
    /* Exit with message if adc_id is un-usable in target board */
    if (! adc_id < FADC_NUM)
    {
        FADC_TEST_ERROR("The adc_id can not be used.\n");
        return FADC_ERR_CMD_FAILED;
    }
    
    FError ret = FADC_SUCCESS;
    /* init FAdc instance */
    memset(&adc_ctrl, 0, sizeof(adc_ctrl));
    memset(&adc_config, 0, sizeof(adc_config));
    adc_config = *FAdcLookupConfig(adc_id);
    ret = FAdcCfgInitialize(&adc_ctrl, &adc_config);
    if (ret != FADC_SUCCESS)
    {
        FADC_TEST_ERROR("FAdcCfgInitialize failed.\n");
        return FADC_ERR_CMD_FAILED;
    }

    /* adc convert config */
    FAdcConvertConfig convert_config; 
    memset(&convert_config, 0, sizeof(convert_config));
    convert_config.convert_mode = FADC_CONTINUOUS_CONVERT;
    convert_config.channel_mode = FADC_MULTI_CHANNEL;
    convert_config.convert_interval = 0x10000;
    convert_config.clk_div = 8;
    ret = FAdcVariableConfig(&adc_ctrl, &convert_config);
    if (ret != FADC_SUCCESS)
    {
        FADC_TEST_ERROR("FAdcVariableConfig failed.\n");
        return FADC_ERR_CMD_FAILED;
    }

    /* adc channel threshold config */
    FAdcThresholdConfig threshold_config;
    memset(&threshold_config, 0, sizeof(threshold_config));
    threshold_config.high_threshold = 1000;
    threshold_config.low_threshold = 0;

    /*init iomux*/
    FIOMuxInit();
    FIOPadSetAdcMux(FADC0_ID, FADC_CHANNEL_0);
    FIOPadSetAdcMux(FADC0_ID, FADC_CHANNEL_1);
    FIOPadSetAdcMux(FADC0_ID, FADC_CHANNEL_2);
    FIOPadSetAdcMux(FADC0_ID, FADC_CHANNEL_3);
    FIOPadSetAdcMux(FADC0_ID, FADC_CHANNEL_4);
    FIOPadSetAdcMux(FADC0_ID, FADC_CHANNEL_5);
    FIOPadSetAdcMux(FADC0_ID, FADC_CHANNEL_6);
    FIOPadSetAdcMux(FADC0_ID, FADC_CHANNEL_7);

    for (int channel_cfg = 0; channel_cfg < FADC_CHANNEL_NUM  ; channel_cfg ++)
    {
        ret = FAdcChannelThresholdSet(&adc_ctrl, channel_cfg, &threshold_config);
        if (ret != FADC_SUCCESS)
        {
            FADC_TEST_ERROR("FAdcChannelThresholdSet failed.\n");
            return FADC_ERR_CMD_FAILED;
        }

        /*adc channel enable*/
        FAdcChannelEnable(&adc_ctrl, channel_cfg, TRUE);
    }

    /* set adc irq handler */
    FAdcIrqSet(&adc_ctrl);
    /* enable adc irq */
    FAdcIrqAllEnable(&adc_ctrl);
    /* start adc convert */
    FAdcConvertStart(&adc_ctrl);

    float val = 0.0;
    u16 adc_val = 0;

    for (int channel_rd = 0; channel_rd < FADC_CHANNEL_NUM  ; channel_rd ++)
    {
        ret = FAdcReadConvertResult(&adc_ctrl, channel_rd, &adc_val);
        if (ret == FADC_SUCCESS)
        {
            val = (float)adc_val;
            val = val * REF_VOL / (1 << DEFAULT_RESOLUTION); /* 2^10 */
            printf("Read success, channel=%d, reg_value=%d, value=%f\n", channel_rd , adc_val, val);
        }
        else
        {
            printf("Read failed.\n");
        }
        fsleep_millisec(100);
    }
    /* stop adc convert */
    FAdcConvertStop(&adc_ctrl);
    /* disable adc irq */
    FAdcIrqAllDisable(&adc_ctrl);
    /* deinit adc */
    FAdcDeInitialize(&adc_ctrl);

    FIOMuxDeInit();/*init iomux */

    if (ret == FT_SUCCESS)
    {
        printf("%s@%d: adc intr mode get vol example test success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: adc intr mode get vol example test failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
        return FADC_ERR_CMD_FAILED;
    }

    return ret;
}