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
 * FilePath: pin_gpio_intr_example.c
 * Date: 2022-03-01 12:54:42
 * LastEditTime: 2022-03-05 17:28:07
 * Description:  This file is for pin gpio interrupt tigger example function implmentation.
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
#include "fcpu_info.h"
#include "fio_mux.h"

#include "fgpio.h"


#include "pin_common.h"
#include "pin_gpio_intr_example.h"
/************************** Constant Definitions *****************************/
#if defined(CONFIG_TARGET_E2000)
#if defined(CONFIG_TARGET_PHYTIUMPI)
static const u32 ctrl_id = FGPIO3_ID;
static const FGpioPinId input_pin_index = {FGPIO3_ID, FGPIO_PORT_A, FGPIO_PIN_1};
static const FGpioPinId output_pin_index = {FGPIO3_ID, FGPIO_PORT_A, FGPIO_PIN_2};
#else
static const u32 ctrl_id = FGPIO4_ID;
static const FGpioPinId input_pin_index = {FGPIO4_ID, FGPIO_PORT_A, FGPIO_PIN_11};
static const FGpioPinId output_pin_index = {FGPIO4_ID, FGPIO_PORT_A, FGPIO_PIN_12};
#endif
#elif defined(CONFIG_TARGET_D2000) || defined(CONFIG_TARGET_FT2004)
static const u32 ctrl_id = FGPIO1_ID;
static const FGpioPinId input_pin_index = {FGPIO1_ID, FGPIO_PORT_A, FGPIO_PIN_3};
static const FGpioPinId output_pin_index = {FGPIO1_ID, FGPIO_PORT_A, FGPIO_PIN_4};
#endif
static FGpioIrqType irq_type = FGPIO_IRQ_TYPE_LEVEL_HIGH;
static const char *irq_type_names[] = 
{
    [FGPIO_IRQ_TYPE_EDGE_FALLING] = "falling edge",
    [FGPIO_IRQ_TYPE_EDGE_RISING] = "rising edge",
    [FGPIO_IRQ_TYPE_LEVEL_LOW] = "level low",
    [FGPIO_IRQ_TYPE_LEVEL_HIGH] = "level high"
};
/**************************** Type Definitions *******************************/
static int intr_flag = 0;
/************************** Variable Definitions *****************************/
static FGpio ctrl_instance; 
static FGpioPin input_pin_instance;
static FGpioPin output_pin_instance;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
static void FPinAckIrq(s32 vector, void *param)
{
    printf("Assert %s for gpio %d-%c-%d !!!\n", 
          irq_type_names[irq_type], 
          input_pin_index.ctrl,
          (input_pin_index.port == FGPIO_PORT_A)?'a':'b',
          input_pin_index.pin);

    intr_flag = 1;

    /* for level-triggered interrupt, reset out-pin level so that interrupt 
       occurr once only */
    if ((FGPIO_IRQ_TYPE_EDGE_FALLING == irq_type) || (FGPIO_IRQ_TYPE_LEVEL_LOW == irq_type))
    {
        FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_HIGH);
    }
    else if ((FGPIO_IRQ_TYPE_EDGE_RISING == irq_type) || (FGPIO_IRQ_TYPE_LEVEL_HIGH == irq_type))
    {
        FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_LOW);
    }
}

static void FPinTriggerFallingEdgeIrq(void)
{
    intr_flag = 0;
    FGpioSetInterruptType(&input_pin_instance, FGPIO_IRQ_TYPE_EDGE_FALLING);
    FGpioSetInterruptMask(&input_pin_instance, TRUE);

    FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_LOW); /* init output pin with low level*/
    fsleep_millisec(100);

    /* falling edge */
    FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_HIGH);
    fsleep_millisec(100);
    FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_LOW);
}

static void FPinTriggerRisingEdgeIrq(void)
{
    intr_flag = 0;
    FGpioSetInterruptType(&input_pin_instance, FGPIO_IRQ_TYPE_EDGE_RISING);
    FGpioSetInterruptMask(&input_pin_instance, TRUE);

    FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_LOW); /* init output pin with low level*/
    fsleep_millisec(100);

    /* rising edge */
    FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_LOW);
    fsleep_millisec(100);
    FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_HIGH);
}

static void FPinTriggerLevelLowIrq(void)
{
    intr_flag = 0;
    FGpioSetInterruptType(&input_pin_instance, FGPIO_IRQ_TYPE_LEVEL_LOW);
    FGpioSetInterruptMask(&input_pin_instance, TRUE);

    FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_HIGH);
    fsleep_millisec(100);

    /* level low */
    FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_LOW);
}

static void FPinTriggerLevelHighIrq(void)
{
    intr_flag = 0;
    FGpioSetInterruptType(&input_pin_instance, FGPIO_IRQ_TYPE_LEVEL_HIGH);
    FGpioSetInterruptMask(&input_pin_instance, TRUE);

    FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_LOW);
    fsleep_millisec(100);

    /* level high */
    FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_HIGH);
}

/* function of gpio interrupt trigger example */
int FPinGpioIntrExample(void)
{
    int ret = 0;
    input_pin_instance.index = input_pin_index;
    output_pin_instance.index = output_pin_index;

    /* init ctrl */
    FGpioConfig input_cfg = *FGpioLookupConfig(ctrl_id);
    ret = FGpioCfgInitialize(&ctrl_instance, &input_cfg);

    FIOMuxInit();
    FIOPadSetGpioMux(input_pin_index.ctrl, (u32)input_pin_index.pin);
    FIOPadSetGpioMux(output_pin_index.ctrl, (u32)output_pin_index.pin);

// #if defined(CONFIG_ENABLE_IOPAD)
//     /* init pins */
//     FIOPadCfgInitialize(&iopad_ctrl, FIOPadLookupConfig(FIOPAD0_ID));
// #if defined(CONFIG_TARGET_E2000D)
//     FIOPadSetFunc(&iopad_ctrl, FIOPAD_AC45_REG0_OFFSET, FIOPAD_FUNC6);
//     FIOPadSetFunc(&iopad_ctrl, FIOPAD_AE43_REG0_OFFSET, FIOPAD_FUNC6);
// #endif
// #endif /* CONFIG_ENABLE_IOPAD */

// #if defined(CONFIG_ENABLE_IOCTRL)
//     FIOCtrlCfgInitialize(&ioctrl, FIOCtrlLookupConfig(FIOCTRL0_ID));
// #if defined(CONFIG_TARGET_D2000)
//     FIOCtrlSetFunc(&ioctrl, FIOCTRL_LPC_LAD0_PAD, FIOCTRL_FUNC1);
//     FIOCtrlSetFunc(&ioctrl, FIOCTRL_LPC_LAD1_PAD, FIOCTRL_FUNC1);
// #endif
// #endif /* CONFIG_ENABLE_IOCTRL */

// #if defined(CONFIG_TARGET_D2000) || defined(CONFIG_TARGET_FT2004)
    
// #endif

    ret = FGpioPinInitialize(&ctrl_instance, &input_pin_instance, input_pin_index);
    ret = FGpioPinInitialize(&ctrl_instance, &output_pin_instance, output_pin_index);

    if (ret != 0)
    {
        printf("Fail to init ctrl or pins.");
        return FGPIO_ERR_NOT_INIT;
    }

    FGpioSetDirection(&input_pin_instance, FGPIO_DIR_INPUT);
    FGpioSetDirection(&output_pin_instance, FGPIO_DIR_OUTPUT);

    /* input pin irq set */
    FGpioSetInterruptMask(&input_pin_instance, FALSE); /* disable pin irq */
    FASSERT_MSG((FGPIO_IRQ_BY_CONTROLLER == FGpioGetPinIrqSourceType(input_pin_instance.index)), 
                 "Irq is not reported by controller.");
    u32 cpu_id;
    u32 irq_num;
#if defined(FGPIO_VERSION_1)
    irq_num = ctrl_instance.config.irq_num;
#elif defined(FGPIO_VERSION_2)
    irq_num = ctrl_instance.config.irq_num[0];
#endif
    GetCpuId(&cpu_id);
    FPIN_TEST_INFO("cpu_id is cpu_id %d, irq_num %d", cpu_id, irq_num);
    InterruptSetTargetCpus(irq_num, cpu_id);
    InterruptSetPriority(irq_num, ctrl_instance.config.irq_priority); /* setup interrupt */
    InterruptInstall(irq_num,
                     FGpioInterruptHandler,
                     &ctrl_instance,
                     NULL); /* register intr handler */
    InterruptUmask(irq_num);

    FGpioRegisterInterruptCB(&input_pin_instance, 
                             FPinAckIrq, 
                             NULL, 
                             FALSE); /* register intr callback */

    /* trigger irq as one of four types */
    switch (irq_type)
    {
        case FGPIO_IRQ_TYPE_EDGE_FALLING:
            FPinTriggerFallingEdgeIrq();
            break;
        case FGPIO_IRQ_TYPE_EDGE_RISING:
            FPinTriggerRisingEdgeIrq();
            break;
        case FGPIO_IRQ_TYPE_LEVEL_LOW:
            FPinTriggerLevelLowIrq();
            break;
        case FGPIO_IRQ_TYPE_LEVEL_HIGH:    
            FPinTriggerLevelHighIrq();
            break;
    }

    /* wait interrupt handle done */
    fsleep_millisec(1000);

    FGpioSetInterruptMask(&input_pin_instance, FALSE);

    /* deinit ctrl and pin instance */  
    FGpioDeInitialize(&ctrl_instance);

    InterruptMask(irq_num);

    /* print message on example run result */
    if (1 == intr_flag)
    {
        printf("%s@%d: pin GPIO intr example [success].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: pin GPIO intr example [failure].\r\n", __func__, __LINE__);
    }

    return 0;
}
