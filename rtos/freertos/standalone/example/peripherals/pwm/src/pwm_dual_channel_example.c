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
 * FilePath: pwm_dual_channel_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for pwm dual channel example function implmentation
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
#include "fassert.h"
#include "finterrupt.h"
#include "fparameters.h"
#include "fpwm.h"
#include "fpwm_hw.h"
#include "fcpu_info.h"
#include "fsleep.h"
#include "fio_mux.h"

#if defined(TARDIGRADE)
#include "fmio_hw.h"
#include "fmio.h"
#endif

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "pwm_common.h"
#include "pwm_dual_channel_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FPwmCtrl pwm_ctrl;
static FPwmConfig pwm_config;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/

#if defined(TARDIGRADE)
static FMioCtrl mio_ctrl;

static FError FMioPwmInit(void)
{
    FError ret = FT_SUCCESS;
    FPwmCtrl *pwm_instance_p = &pwm_ctrl;
    FMioCtrl *mio_instance_p = &mio_ctrl;

    const FMioConfig *mio_config_p ;

    FPwmConfig pwm_config;

    FIOPadSetMioMux(FMIO1_ID);

    mio_config_p = FMioLookupConfig(PWM_MIO);
    if (NULL == mio_config_p)
    {
        printf("Mio error inval parameters.\r\n");
        return FMIO_ERR_INVAL_PARM;
    }

    mio_instance_p->config = *mio_config_p;

    printf("mio_base_addr=%p.\r\n", mio_instance_p->config.mio_base_addr);
    ret = FMioFuncInit(mio_instance_p, FMIO_FUNC_SET_PWM);
    if (ret != FT_SUCCESS)
    {
        printf("Pwm mio init error.\r\n");
        return ret;
    }
    else
    {
        printf("Pwm mio function init ok.\r\n");
    }


    /* get standard config of pwm */
    pwm_config = *FPwmLookupConfig(FPWM0_ID);

    /* Modify configuration */
    pwm_config.pwm_base_addr = FMioFuncGetAddress(mio_instance_p, FMIO_FUNC_SET_PWM);
    pwm_config.irq_num[FPWM_CHANNEL_0] = FMioFuncGetIrqNum(mio_instance_p, FMIO_FUNC_SET_PWM);

    FPwmDeInitialize(pwm_instance_p);
    /* Initialization */
    ret = FPwmCfgInitialize(pwm_instance_p, &pwm_config);
    if(ret != FT_SUCCESS)
    {
        return ret;
    }

    /*  callback function for interrupt  */
 
    printf("Pwm channel base addr:0x%x.\r\n",
            pwm_instance_p->config.pwm_base_addr);
    
    return ret;
}
#endif

static void FPwmCounterIrqCallback(void *args)
{
    FPwmCtrl *instance_p = (FPwmCtrl *)args;
    FPWM_INFO("Pwm%d irq counter callback.", instance_p->config.instance_id);
}

static void FPwmFifoEmptyIrqCallback(void *args)
{
    FPwmCtrl *instance_p = (FPwmCtrl *)args;
    FPWM_INFO("Pwm%d irq fifo empty callback", instance_p->config.instance_id);
}

static void FPwmIrqSet(FPwmCtrl *instance_p, u32 channel)
{
    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(instance_p->config.irq_num[channel], cpu_id);
    FPwmRegisterInterruptHandler(instance_p, FPWM_INTR_EVENT_COUNTER,FPwmCounterIrqCallback, (void *)instance_p);
    FPwmRegisterInterruptHandler(instance_p, FPWM_INTR_EVENT_FIFO_EMPTY,FPwmFifoEmptyIrqCallback, (void *)instance_p);

    InterruptSetPriority(instance_p->config.irq_num[channel], instance_p->config.irq_prority[channel]);
    InterruptInstall(instance_p->config.irq_num[channel], FPwmIntrHandler, (void *)instance_p, "pwm");
    InterruptUmask(instance_p->config.irq_num[channel]);
}

int FPwmDualChannelExample()
{
    u32 ret = FPWM_SUCCESS;
    u32 pwm_id = PWM_TEST_ID;
    FPwmVariableConfig pwm_cfg;
    /*init iomux*/
    FIOMuxInit(); 
#if defined(TARDIGRADE)
    FMioPwmInit();
#else   
    /*set channel 0 and 1 iopad*/
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

#endif

    /*set pwm channel_0 configuration*/
    memset(&pwm_cfg, 0, sizeof(pwm_cfg));
    pwm_cfg.tim_ctrl_mode = FPWM_MODULO;
    pwm_cfg.tim_ctrl_div = 500-1;
    pwm_cfg.pwm_period = 10000;
    pwm_cfg.pwm_pulse = 2000;
    pwm_cfg.pwm_mode = FPWM_OUTPUT_COMPARE;
    pwm_cfg.pwm_polarity = FPWM_POLARITY_NORMAL;
    pwm_cfg.pwm_duty_source_mode = FPWM_DUTY_CCR;
    ret = FPwmVariableSet(&pwm_ctrl, FPWM_CHANNEL_0, &pwm_cfg);
    if (ret != FPWM_SUCCESS)
    {
        FPWM_ERROR("FPwmVariableSet failed.");
        return FPWM_ERR_CMD_FAILED;
    }

    /*set pwm channel_1 configuration*/
    memset(&pwm_cfg, 0, sizeof(pwm_cfg));
    pwm_cfg.tim_ctrl_mode = FPWM_MODULO;
    pwm_cfg.tim_ctrl_div = 1000-1;
    pwm_cfg.pwm_period = 20000;
    pwm_cfg.pwm_pulse = 2000;
    pwm_cfg.pwm_mode = FPWM_OUTPUT_COMPARE;
    pwm_cfg.pwm_polarity = FPWM_POLARITY_NORMAL;
    pwm_cfg.pwm_duty_source_mode = FPWM_DUTY_CCR;
    ret = FPwmVariableSet(&pwm_ctrl, FPWM_CHANNEL_1, &pwm_cfg);
    if (ret != FPWM_SUCCESS)
    {
        FPWM_ERROR("FPwmVariableSet failed.");
        return FPWM_ERR_CMD_FAILED;
    }

    /*enable channel_0 interrupt*/
    FPwmIrqSet(&pwm_ctrl, FPWM_CHANNEL_0);
    /*enable channel_1 interrupt*/
    FPwmIrqSet(&pwm_ctrl, FPWM_CHANNEL_1);

    /*enable pwm channel_0*/
    FPwmEnable(&pwm_ctrl, FPWM_CHANNEL_0);
    /*enable pwm channel_1*/
    FPwmEnable(&pwm_ctrl, FPWM_CHANNEL_1);

    for (int i = 0; i < PWM_PULSE_CHANGE_TIME; i++)
    {
        fsleep_seconds(1);
        pwm_cfg.pwm_pulse = (pwm_cfg.pwm_pulse + PWM_PULSE_CHANGE) % pwm_cfg.pwm_period;
        FPwmPulseSet(&pwm_ctrl, FPWM_CHANNEL_0, pwm_cfg.pwm_pulse);
        FPwmPulseSet(&pwm_ctrl, FPWM_CHANNEL_1, pwm_cfg.pwm_pulse);
		#if defined(TARDIGRADE)
        /* toggle gpio output */
        FPwmGpioSet(&pwm_ctrl, FPWM_CHANNEL_0, (i%2));
        FPwmGpioSet(&pwm_ctrl, FPWM_CHANNEL_1, (i%2));
		#endif
        printf("count= %d, pwm_pulse = %d\n", i, pwm_cfg.pwm_pulse);
        fsleep_seconds(1);
    }

    /*disable pwm*/
    FPwmDisable(&pwm_ctrl, FPWM_CHANNEL_0);
    FPwmDisable(&pwm_ctrl, FPWM_CHANNEL_1);

    /*deinit pwm*/
    FPwmDeInitialize(&pwm_ctrl);
    /*deinit iopad*/
    FIOMuxDeInit();
    /* print message on example run result */
    if (ret == FPWM_SUCCESS)
    {
        printf("%s@%d: pwm dual channel example success !!! \r\n", __func__, __LINE__);
        printf("[system_example_pass]\r\n");
    }
    else
    {
        printf("%s@%d: pwm dual channel example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
        return FPWM_ERR_CMD_FAILED;
    }

    return ret;
}