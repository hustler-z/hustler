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
 * FilePath: pwm_dead_band_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for pwm dead band example function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/3/14   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "ftypes.h"
#include "fgeneric_timer.h"
#include "fparameters.h"
#include "fpwm.h"
#include "fpwm_hw.h"
#include "fsleep.h"
#include "fio_mux.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "pwm_common.h"
#include "pwm_dead_band_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FPwmCtrl pwm_ctrl;
static FPwmConfig pwm_config;
/***************** Macros (Inline Functions) Definitions *********************/

/* pwm channel use, 0/1 */
#define PWM_CHANNEL_USE     FPWM_CHANNEL_0

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/

int FPwmDeadBandExample()
{
    u32 ret = FPWM_SUCCESS;
    u32 pwm_id = PWM_TEST_ID;
    FPwmVariableConfig pwm_cfg;
    FPwmDbVariableConfig db_cfg;
    /*init iomux*/
    FIOMuxInit();   
    FIOPadSetPwmMux(pwm_id, 0);
    FIOPadSetPwmMux(pwm_id, 1);
    /*init pwm config*/
    memset(&pwm_ctrl, 0, sizeof(pwm_ctrl));
    memset(&pwm_config, 0, sizeof(pwm_config));
    pwm_config = *FPwmLookupConfig(pwm_id);
    ret = FPwmCfgInitialize(&pwm_ctrl, &pwm_config);
    if (ret != FPWM_SUCCESS)
    {
        FPWM_ERROR("FPwmCfgInitialize failed.");
        return FPWM_ERR_CMD_FAILED;
    }

    /*set pwm db configuration*/
    memset(&db_cfg, 0, sizeof(db_cfg));
    db_cfg.db_rise_cycle = 800;
    db_cfg.db_fall_cycle = 800;
    db_cfg.db_polarity_sel = FPWM_DB_AH;
    db_cfg.db_in_mode = FPWM_DB_IN_MODE_PWM0;
    db_cfg.db_out_mode = FPWM_DB_OUT_MODE_ENABLE_RISE_FALL;
    ret = FPwmDbVariableSet(&pwm_ctrl, &db_cfg);
    if (ret != FPWM_SUCCESS)
    {
        FPWM_ERROR("FPwmDbVariableSet failed.");
        return FPWM_ERR_CMD_FAILED;
    }
 
    /*set pwm channel configuration*/
    memset(&pwm_cfg, 0, sizeof(pwm_cfg));
    pwm_cfg.tim_ctrl_mode = FPWM_MODULO;
    pwm_cfg.tim_ctrl_div = 500-1;
    pwm_cfg.pwm_period = 10000;
    pwm_cfg.pwm_pulse = 2000;
    pwm_cfg.pwm_mode = FPWM_OUTPUT_COMPARE;
    pwm_cfg.pwm_polarity = FPWM_POLARITY_NORMAL;
    pwm_cfg.pwm_duty_source_mode = FPWM_DUTY_CCR;
    ret = FPwmVariableSet(&pwm_ctrl, PWM_CHANNEL_USE, &pwm_cfg);
    if (ret != FPWM_SUCCESS)
    {
        FPWM_ERROR("FPwmVariableSet failed.");
        return FPWM_ERR_CMD_FAILED;
    }

    /*get pwm channel configuration*/
    memset(&pwm_cfg, 0, sizeof(pwm_cfg));
    FPwmVariableGet(&pwm_ctrl, PWM_CHANNEL_USE, &pwm_cfg);
    FPWM_DEBUG("FPwmVariableGet:\n");
    FPWM_DEBUG("pwm_cfg.tim_ctrl_mode = %d\n", pwm_cfg.tim_ctrl_mode);
    FPWM_DEBUG("pwm_cfg.tim_ctrl_div = %d\n", pwm_cfg.tim_ctrl_div);
    FPWM_DEBUG("pwm_cfg.pwm_period = %d\n", pwm_cfg.pwm_period);
    FPWM_DEBUG("pwm_cfg.pwm_pulse = %d\n", pwm_cfg.pwm_pulse);
    FPWM_DEBUG("pwm_cfg.pwm_mode = %d\n", pwm_cfg.pwm_mode);
    FPWM_DEBUG("pwm_cfg.pwm_polarity = %d\n", pwm_cfg.pwm_polarity);
    FPWM_DEBUG("pwm_cfg.pwm_duty_source_mode = %d\n", pwm_cfg.pwm_duty_source_mode);

    /*enable pwm*/
    FPwmEnable(&pwm_ctrl, PWM_CHANNEL_USE);

    for (int i = 0; i < PWM_PULSE_CHANGE_TIME; i++)
    {
        fsleep_seconds(1);
        pwm_cfg.pwm_pulse = (pwm_cfg.pwm_pulse + PWM_PULSE_CHANGE) % pwm_cfg.pwm_period;
        FPwmPulseSet(&pwm_ctrl, PWM_CHANNEL_USE, pwm_cfg.pwm_pulse);
        printf("count= %d, pwm_pulse = %d, pwm_period=%d\n", i, pwm_cfg.pwm_pulse, pwm_cfg.pwm_period);
        fsleep_seconds(1);
    }

    /*disable pwm*/
    FPwmDisable(&pwm_ctrl, PWM_CHANNEL_USE);

    /*deinit pwm*/
    FPwmDeInitialize(&pwm_ctrl);
    /*deinit iopad*/
    FIOMuxDeInit();
    /* print message on example run result */
    if (ret == FPWM_SUCCESS)
    {
        printf("%s@%d: pwm dead band example success !!! \r\n", __func__, __LINE__);
        printf("[system_example_pass]\r\n");
    }
    else
    {
        printf("%s@%d: pwm dead band example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
        return FPWM_ERR_CMD_FAILED;
    }

    return ret;
}