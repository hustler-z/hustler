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
 * FilePath: pin_gpio_pwm_example.c
 * Date: 2022-03-01 15:36:33
 * LastEditTime: 2022-03-05 12:29:08
 * Description:  This file is for pin gpio pwm output example function implmentation.
 *
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----   -------    --------     --------------------------------------
 *  1.0  liqiaozhong  2023/03/05   first commit
 *  1.1  liqiaozhong  2023/8/11    adapt to new iomux
 */


/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include <string.h>
#include <stdio.h>
#include "strto.h"

#include "ftypes.h"
#include "fdebug.h"
#include "fassert.h"
#include "fsleep.h"
#include "finterrupt.h"
#include "fgeneric_timer.h"
#include "fio_mux.h"

#include "fgpio.h"

#include "pin_common.h"
#include "pin_gpio_pwm_example.h"
/************************** Constant Definitions *****************************/
#define SYS_TICK_DELAY   10000U
#define SYS_TICKINTR_PRIORITY  IRQ_PRIORITY_VALUE_11

#if defined(CONFIG_TARGET_E2000)
#if defined(CONFIG_TARGET_PHYTIUMPI)
static const u32 pwm_ctrl_id = FGPIO3_ID;
static const FGpioPinId output_pin_index_1 = {FGPIO3_ID, FGPIO_PORT_A, FGPIO_PIN_1};
static const FGpioPinId output_pin_index_2 = {FGPIO3_ID, FGPIO_PORT_A, FGPIO_PIN_2};
#else
static const u32 pwm_ctrl_id = FGPIO4_ID;
static const FGpioPinId output_pin_index_1 = {FGPIO4_ID, FGPIO_PORT_A, FGPIO_PIN_11};
static const FGpioPinId output_pin_index_2 = {FGPIO4_ID, FGPIO_PORT_A, FGPIO_PIN_12};
#endif
#elif defined(CONFIG_TARGET_D2000) || defined(CONFIG_TARGET_FT2004)
static const u32 pwm_ctrl_id = FGPIO1_ID;
static const FGpioPinId output_pin_index_1 = {FGPIO1_ID, FGPIO_PORT_A, FGPIO_PIN_3};
static const FGpioPinId output_pin_index_2 = {FGPIO1_ID, FGPIO_PORT_A, FGPIO_PIN_4};
#endif
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FGpio pwm_ctrl_instance; 
static FGpioPin output_pin_instance_1;
static FGpioPin output_pin_instance_2;
static FSoftPwm pwm_output_1;
static FSoftPwm pwm_output_2;
/* pwm related parameters */
static u32 tick_delay = SYS_TICK_DELAY;
static u32 tick_level = 0;
static u32 pwm_clk_hz = 10000; /* pwm system tick rate */
static u32 pwm_total_ms; /* represents the time pwm will lasting */
static u32 pwm_range; /* freq_hz = pwm_clk_hz / pwm_range ==> pwm_range = pwm_clk_hz / freq_hz */ 
               /* pwm_range indicates how many system ticks each time a waveform is output  */
static double pwm_duty = 0.8; /* duty cycle of pwm */
static u32 freq_hz = 400; /* waveform output rate */
static u32 pluse_num = 10; /* waveform number to output */
/***************** Macros (Inline Functions) Definitions *********************/
static inline void SetupSysTick(IrqHandler tick_cb)
{
    /* stop timer */
    GenericTimerStop(GENERIC_TIMER_ID0);
    
    /* setup and enable interrupt */
    InterruptSetPriority(GENERIC_TIMER_NS_IRQ_NUM, SYS_TICKINTR_PRIORITY);
    InterruptInstall(GENERIC_TIMER_NS_IRQ_NUM, tick_cb, NULL, NULL);
    InterruptUmask(GENERIC_TIMER_NS_IRQ_NUM);

	GenericTimerSetTimerValue(GENERIC_TIMER_ID0, GenericTimerFrequecy() / pwm_clk_hz);
    GenericTimerInterruptEnable(GENERIC_TIMER_ID0);
	GenericTimerStart(GENERIC_TIMER_ID0);

}

static inline void RevokeSysTick()
{
    GenericTimerInterruptDisable(GENERIC_TIMER_ID0);
    FPIN_TEST_DEBUG("Tick is revoked !!! \r\n");
}
/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
static void PwmStart(FSoftPwm *const pwm)
{
    FASSERT(pwm);
    FASSERT(FALSE == pwm->is_running);

    pwm->level = FGPIO_PIN_LOW; /* start with low level */
    FGpioSetOutputValue(pwm->pin_instance, pwm->level);

    pwm->duty = 0;
    pwm->count = 1;

    pwm->is_running = TRUE;
    FPIN_TEST_DEBUG("Start PWM with duty:%d count:%d!!!\r\n",
                       pwm->duty,
                       pwm->count);

}

static int FSoftPwmSetup(FSoftPwm *const pwm, FGpioPin *pin_instance, u32 range)
{
    FASSERT(pwm);

    if (range == 0)
    {
        return FGPIO_SW_PWM_INVALID_INPUT;
    }

    pwm->pin_instance = pin_instance;
    pwm->range = range;
    pwm->is_running = FALSE;

    PwmStart(pwm);/* start pwm output, include pin set */

    return FGPIO_SW_PWM_OK;
}

static void FPwmStop(FSoftPwm *const pwm)
{
    FASSERT(pwm);
    FASSERT(TRUE == pwm->is_running);

    pwm->is_running = FALSE; /* endup with low level */
    FGpioSetOutputValue(pwm->pin_instance, FGPIO_PIN_LOW);
    FPIN_TEST_DEBUG("Stop pwm with level:%d!!!\r\n", FGPIO_PIN_LOW);
}

static void FPwmSetDuty(FSoftPwm *const pwm, u32 duty)
{
    FASSERT(pwm);
    FASSERT(duty <= pwm->range);

    pwm->duty = duty;
    FPIN_TEST_DEBUG("Set PWM duty as %f!!! \r\n", duty);
}

/* tick callback function, at the software level to achieve the output target waveform logic */
static void FPwmTickInterruptHandler(FSoftPwm *const pwm)
{
    FASSERT(pwm);
    if (pwm->is_running)
    {
        FGpioPinVal new_level = FGPIO_PIN_LOW;
        if (pwm->duty > 0)
        {
            if (pwm->duty < pwm->range) /*duty means PWM duty ratio*/
            {
                new_level = (pwm->count <= pwm->duty) ? FGPIO_PIN_HIGH : FGPIO_PIN_LOW; /* toggle level if over-pass PWM range */
            }
            else
            {
                new_level = FGPIO_PIN_HIGH;
            }
        }
        if (pwm->level != new_level)
        {
            pwm->level = new_level;
            FGpioSetOutputValue(pwm->pin_instance, pwm->level);
        }
        if (++pwm->count > pwm->range) /*update count if over-pass PWM range*/
        {
            pwm->count = 1;
        }
    }
}

static void PwmClkHandler()
{
    GenericTimerInterruptDisable(GENERIC_TIMER_ID0);
    GenericTimerSetTimerValue(GENERIC_TIMER_ID0, GenericTimerFrequecy() / pwm_clk_hz);
    GenericTimerInterruptEnable(GENERIC_TIMER_ID0);
    FPwmTickInterruptHandler(&pwm_output_1);
    FPwmTickInterruptHandler(&pwm_output_2);
    FPwmTickInterruptHandler(&pwm_output_1);
    FPwmTickInterruptHandler(&pwm_output_2);
}


/* function of gpio pwm example */
int FPinGpioPwmExample(void)
{
    int ret = 0;
    /* init ctrl */
    FGpioConfig input_cfg = *FGpioLookupConfig(pwm_ctrl_id);
    ret = FGpioCfgInitialize(&pwm_ctrl_instance, &input_cfg);
    
    /* init pins */
    FIOMuxInit();
    FIOPadSetGpioMux(output_pin_index_1.ctrl, (u32)output_pin_index_1.pin);
    FIOPadSetGpioMux(output_pin_index_2.ctrl, (u32)output_pin_index_2.pin);

    ret = FGpioPinInitialize(&pwm_ctrl_instance, &output_pin_instance_1, output_pin_index_1);
    ret = FGpioPinInitialize(&pwm_ctrl_instance, &output_pin_instance_2, output_pin_index_2);

    if (ret != 0)
    {
        printf("Fail to init ctrl or pins.");
        return FGPIO_ERR_NOT_INIT;
    }

    FGpioSetDirection(&output_pin_instance_1, FGPIO_DIR_OUTPUT);
    FGpioSetDirection(&output_pin_instance_2, FGPIO_DIR_OUTPUT);

    /* pwm instance init */
    pwm_range = pwm_clk_hz / freq_hz;
    pwm_total_ms = (u32)(((double)(pluse_num) / (double)freq_hz) * 1000);
    
    printf("Pwm task start.\r\n");

    ret = FSoftPwmSetup(&pwm_output_1, &output_pin_instance_1, pwm_range);
    ret = FSoftPwmSetup(&pwm_output_2, &output_pin_instance_2, pwm_range);
    if (FGPIO_SW_PWM_OK != ret)
    {
        printf("Pwm instance init failed.\r\n");
        return ret;
    }

    FPwmSetDuty(&pwm_output_1, (u32)(pwm_range * (1 - pwm_duty)));
    FPwmSetDuty(&pwm_output_2, (u32)(pwm_range * pwm_duty));

    /* operations */
    fsleep_millisec(1000);

    SetupSysTick(PwmClkHandler); /* start generate pwm, continues pwm_total_ms */
    fsleep_millisec(pwm_total_ms);
    printf("Pwm output done.\r\n");

    /* tick revoke and deinit */
    RevokeSysTick();
    FPwmStop(&pwm_output_1);
    FPwmStop(&pwm_output_2);
    FGpioDeInitialize(&pwm_ctrl_instance);

    /* print message on example run result */
    if (0 == ret)
    {
        printf("%s@%d: pin GPIO pwm signal send [success].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: pin GPIO pwm signal send [failure].\r\n", __func__, __LINE__);
    }
    printf("Console part finish, you should check the actual pwm wave on oscilloscope.\r\n");

    return 0;
}
