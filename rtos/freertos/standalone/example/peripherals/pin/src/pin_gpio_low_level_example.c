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
 * FilePath: pin_gpio_low_level_example.c
 * Date: 2022-03-01 14:56:42
 * LastEditTime: 2022-03-05 18:36:06
 * Description:  This file is for pin gpio register operation example function implmentation.
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
#include "fio_mux.h"

#include "fgpio_hw.h"

#include "pin_common.h"
#include "pin_gpio_low_level_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#if defined(CONFIG_TARGET_E2000)
#if defined(CONFIG_TARGET_PHYTIUMPI)
static uintptr gpio_base = FGPIO3_BASE_ADDR;
static const u32 ctrl_id = FGPIO3_ID;
static u32 input_pin = (u32)FGPIO_PIN_1;
static u32 output_pin = (u32)FGPIO_PIN_2;
#else
static uintptr gpio_base = FGPIO4_BASE_ADDR;
static const u32 ctrl_id = FGPIO4_ID;
static u32 input_pin = (u32)FGPIO_PIN_11;
static u32 output_pin = (u32)FGPIO_PIN_12;
#endif
#elif defined(CONFIG_TARGET_D2000) || defined(CONFIG_TARGET_FT2004)
static uintptr gpio_base = FGPIO1_BASE_ADDR;
static const u32 ctrl_id = FGPIO1_ID;
static u32 input_pin = (u32)FGPIO_PIN_3;
static u32 output_pin = (u32)FGPIO_PIN_4;
#endif

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
/* function of gpio low level example */
int FPinGpioLowLevelExample(void)
{
    int ret = 0;
    int flag = 1;
    u32 reg_val;
    u32 set_level = FGPIO_PIN_HIGH;

    /* init pin */
    FIOMuxInit();
    FIOPadSetGpioMux(ctrl_id, input_pin);
    FIOPadSetGpioMux(ctrl_id, output_pin);

    reg_val = FGpioReadReg32(gpio_base, FGPIO_SWPORTA_DDR_OFFSET); /* set direction */
    reg_val &= ~BIT(input_pin); /* 0-input */
    reg_val |= BIT(output_pin); /* 1-output */
    FGpioWriteReg32(gpio_base, FGPIO_SWPORTA_DDR_OFFSET, reg_val);

    /* operations */
    reg_val = FGpioReadReg32(gpio_base, FGPIO_SWPORTA_DR_OFFSET); /* set output pin to low-level */
    reg_val &= ~BIT(output_pin);
    FGpioWriteReg32(gpio_base, FGPIO_SWPORTA_DR_OFFSET, reg_val);

    reg_val = FGpioReadReg32(gpio_base, FGPIO_SWPORTA_DR_OFFSET); /* set output pin to high-level */
    reg_val |= BIT(output_pin); 
    FGpioWriteReg32(gpio_base, FGPIO_SWPORTA_DR_OFFSET, reg_val);
    reg_val = FGpioReadReg32(gpio_base, FGPIO_EXT_PORTA_OFFSET); /* get input pin level */
    if (((BIT(input_pin) & reg_val) ? FGPIO_PIN_HIGH : FGPIO_PIN_LOW) == FGPIO_PIN_HIGH)
    {
        printf("Low level operation works for the first time.\n");
    }
    else
    {
        printf("Low level operation does not work for the first time.\n");
        flag = 0;
    }

    fsleep_millisec(10); /* delay 10ms */

    reg_val = FGpioReadReg32(gpio_base, FGPIO_SWPORTA_DR_OFFSET); /* set output pin to low-level */
    reg_val &= ~BIT(output_pin);
    FGpioWriteReg32(gpio_base, FGPIO_SWPORTA_DR_OFFSET, reg_val);
    reg_val = FGpioReadReg32(gpio_base, FGPIO_EXT_PORTA_OFFSET); /* get input pin level */
    if (((BIT(input_pin) & reg_val) ? FGPIO_PIN_HIGH : FGPIO_PIN_LOW) == FGPIO_PIN_LOW)
    {
        printf("Low level operation works for the second time.\n");
    }
    else
    {
        printf("Low level operation does not work for the second time.\n");
        flag = 0;
    }

    /* print message on example run result */
    if (1 == flag)
    {
        printf("%s@%d: pin GPIO low level example [success].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: pin GPIO low level example [failure].\r\n", __func__, __LINE__);
    }

    return 0;
}
