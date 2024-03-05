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
 * FilePath: nested_interrupt_sgi_example.c
 * Date: 2023-07-31 11:23:42
 * LastEditTime: 2023-08-1 18:47:54
 * Description:  This file is for nested interrupt sgi test example function implementation.
 *
 * Modify History:
 *  Ver       Who         Date           Changes
 * -----     ------     --------     --------------------------------------
 *  1.0    liqiaozhong  2023/8/1     add nested interrupt sgi test example
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "fexception.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fsleep.h"
#include "ftypes.h"
#include "fdebug.h"

#include "nested_interrupt_sgi_example.h"

/***************** Macros (Inline Functions) Definitions *********************/
/* The interrupt number to use for the software interrupt generation. This
could be any unused number. In this case the first chip level (non system)
interrupt is used */
#define INTERRUPT_LOW_ID        0 /* SGI */
#define INTERRUPT_HIGH_ID       1

/* The priority of the software interrupt. */
#define INTERRUPT_LOW_PRIORITY   IRQ_PRIORITY_VALUE_6
#define INTERRUPT_HIGH_PRIORITY  IRQ_PRIORITY_VALUE_3

#define FNESTINTR_DEBUG_TAG "FNSETEDINTERRUPT_EXAMPLE"
#define FNESTINTR_ERROR(format, ...) FT_DEBUG_PRINT_E(FNESTINTR_DEBUG_TAG, format, ##__VA_ARGS__)
#define FNESTINTR_WARRN(format, ...) FT_DEBUG_PRINT_W(FNESTINTR_DEBUG_TAG, format, ##__VA_ARGS__)
#define FNESTINTR_INFO(format, ...) FT_DEBUG_PRINT_I(FNESTINTR_DEBUG_TAG, format, ##__VA_ARGS__)
#define FNESTINTR_DEBUG(format, ...) FT_DEBUG_PRINT_D(FNESTINTR_DEBUG_TAG, format, ##__VA_ARGS__)
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static u32 cpu_id = 0;

static volatile u8 low_priority_intr_flag = 0;	/* Flag to update low priority interrupt counter */
static volatile u8 high_priority_intr_flag = 0;	/* Flag to update high priority interrupt counter */
/************************** Constant Definitions *****************************/
float val_low = 0.3;
float val_high = 0.1;

/************************** Function Prototypes ******************************/

/******************************* Function ************************************/

static void FLowPriorityHandlerFunc(void)
{
    val_low = val_low * 2.0;

	/* Update the flag to indicate the interrupt */
	low_priority_intr_flag++;

    /* Activate high-priority intr */
    InterruptCoreInterSend(INTERRUPT_HIGH_ID, (1 << cpu_id));

	/* Wait till interrupts from counter configured with high priority interrupt */
	while(high_priority_intr_flag == 0);

    printf("low_priority_intr_flag is %d, val_low=%f\r\n", low_priority_intr_flag, val_low);
}


static void FLowPriorityHandler(s32 vector, void *param)
{
    static fsize_t value[3] = {0};

	/* Enable the nested interrupts to allow preemption */     
    FInterruptNestedEnable(value);

    /* A function operation must be used between interrupt nesting enable and disable */
    FLowPriorityHandlerFunc();

    /* Disable the nested interrupt before exiting IRQ mode */
    FInterruptNestedDisable(value);
}

static void FHighPriorityHandlerFunc(void)
{
    high_priority_intr_flag++;

    val_high = val_high * 2.0;

    printf("high_priority_intr_flag is %d, val_high=%f\r\n", high_priority_intr_flag, val_high);
}


static void FHighPriorityHandler(s32 vector, void *param)
{
    static fsize_t value[3] = {0};

    /* Enable the nested interrupts to allow preemption */     
    FInterruptNestedEnable(value);

    /* A function operation must be used between interrupt nesting enable and disable */
    FHighPriorityHandlerFunc();

    /* Disable the nested interrupt before exiting IRQ mode */
    FInterruptNestedDisable(value);
}

static void FNestedInterruptSetup(void)
{
    GetCpuId(&cpu_id);

    InterruptSetPriority(INTERRUPT_LOW_ID, INTERRUPT_LOW_PRIORITY);

    InterruptInstall(INTERRUPT_LOW_ID, FLowPriorityHandler, NULL, NULL);

    /* Enable the interrupt. */
    InterruptUmask(INTERRUPT_LOW_ID);

    InterruptSetPriority(INTERRUPT_HIGH_ID, INTERRUPT_HIGH_PRIORITY);

    InterruptInstall(INTERRUPT_HIGH_ID, FHighPriorityHandler, NULL, NULL);

    /* Enable the interrupt. */
    InterruptUmask(INTERRUPT_HIGH_ID);
}

/* Macro to force an interrupt. */
static void FNestedInterruptTrigger(int count)
{
    low_priority_intr_flag = 0;

    high_priority_intr_flag = 0;

    printf("Nested Interrupt Test %d.\r\n", count);
    
    InterruptCoreInterSend(INTERRUPT_LOW_ID, (1 << cpu_id));
}

/* function of nested interrupt sgi test example */
int FNestedIntrSgiExample(void)
{
    val_low = 0.3;
    val_high = 0.1;

    FNestedInterruptSetup();

    for (int i = 0; i < 10; i++)
    {
        FNestedInterruptTrigger(i);
        fsleep_millisec(200);
    }

    printf("%s@%d: Nested interrupt sgi test done.\r\n", __func__, __LINE__);

    return 0;
}