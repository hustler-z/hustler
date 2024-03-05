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
 * FilePath: pin_gpio_pwm_example.h
 * Date: 2022-03-01 13:16:42
 * LastEditTime: 2022-03-05 14:28:08
 * Description:  This file is for gpio pwm output function definition.
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----   -------    --------     --------------------------------------
 *  1.0  liqiaozhong  2023/03/05   first commit
 */

#ifndef  PIN_GPIO_PWM_EXAMPLE_H
#define  PIN_GPIO_PWM_EXAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif
/***************************** Include Files *********************************/
#include "ftypes.h"
#include "fgpio.h"
#include "fkernel.h"
/**************************** Type Definitions *******************************/
typedef struct /* pwm instance */
{
    FGpioPin *pin_instance;
    u32 range;
    volatile boolean is_running;
    u32 duty;
    FGpioPinVal level;
    u32 count;
} FSoftPwm; 

enum
{
    FGPIO_SW_PWM_OK = 0,
    FGPIO_SW_PWM_INVALID_INPUT = 1,
    FGPIO_SW_PWM_INIT_FAILED = 2,
    FGPIO_SW_PWM_PIN_NOT_SUPPORT = 3,
    FGPIO_SW_PWM_NOT_INITED = 4,
    FGPIO_SW_PWM_IRQ_NOT_SETUP = 5,
    FGPIO_SW_PWM_TIMEOUT = 6,
    FGPIO_SW_PWM_INVALID_STATE = 102
};
/************************** Function Prototypes ******************************/
int FPinGpioPwmExample(void);

#ifdef __cplusplus
}
#endif

#endif