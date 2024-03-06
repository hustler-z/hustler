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
 * FilePath: nested_interrupt_timer_example.c
 * Date: 2023-05-56 08:37:22
 * LastEditTime: 2023-05-27 11:00:53
 * Description:  This file is for nested interrupt generic timer test example functions.
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------     --------------------------------------
 * 1.0   wangxiaodong   2023/9/21    first commit 
 */

#include <stdio.h>
#include "fparameters.h"
#include "finterrupt.h"
#include "fexception.h"
#include "fprintk.h"
#include "ftypes.h"
#include "fgeneric_timer.h"

/* timer beats per second */
#define PBEATS_PER_SEC	10

/* timer beats per second */
#define VBEATS_PER_SEC	10

#define TEST_TIMES 10

static u32 p_ticks_per_beat;
static u32 p_count = 0;

static u32 v_ticks_per_beat;
static u32 v_count = 0;

static unsigned long FEmulDivision(u64 val, u64 div)
{
	unsigned long cnt = 0;

	while (val > div) 
	{
		val -= div;
		cnt++;
	}
	return cnt;
}

static u64 FTicksToNs(u64 ticks)
{
	return FEmulDivision(ticks * 1000,
			     GenericTimerFrequecy() / 1000 / 1000);
}


static void FGenericPhysicalTimerFunc(void)
{

	f_printk("Physical Timer in\r\n");

	GenericTimerStart(GENERIC_TIMER_ID1);
	p_count++;

	GenericTimerSetTimerValue(GENERIC_TIMER_ID0, p_ticks_per_beat);

	if(p_count == TEST_TIMES)
	{
		GenericTimerStop(GENERIC_TIMER_ID0);
		InterruptMask(GENERIC_TIMER_NS_IRQ_NUM);
	}

	f_printk("Physical Timer out\r\n");

}


static void FGenericPhysicalTimerHandler(s32 vector, void *param)
{
	
	static fsize_t value[3] = {0};

	if (vector != GENERIC_TIMER_NS_IRQ_NUM)
		return;

	FInterruptNestedEnable(value);

	/* A function operation must be used between interrupt nesting enable and disable */
	FGenericPhysicalTimerFunc();

	FInterruptNestedDisable(value);
}

static void FGenericVirtualTimerHandlerFunc(void)
{
	static u64 delta_old;
	u64 delta;

	f_printk("Virtual Timer in\r\n");

	v_count++;

	GenericTimerSetTimerValue(GENERIC_TIMER_ID1, v_ticks_per_beat);
	
	if(v_count == TEST_TIMES)
	{
		GenericTimerStop(GENERIC_TIMER_ID1);
		InterruptMask(GENERIC_VTIMER_IRQ_NUM);
	}

	GenericTimerStop(GENERIC_TIMER_ID1);	
	f_printk("Virtual Timer out\r\n");

}


static void FGenericVirtualTimerHandler(s32 vector, void *param)
{
	static fsize_t value[3] = {0};

	if (vector != GENERIC_VTIMER_IRQ_NUM)
		return;

	FInterruptNestedEnable(value);

	/* A function operation must be used between interrupt nesting enable and disable */
	FGenericVirtualTimerHandlerFunc();

	FInterruptNestedDisable(value);
}

void FNestedIntrTimerExample(void)
{
	p_count = 0;
	v_count = 0;

	/* stop timer */
	GenericTimerStop(GENERIC_TIMER_ID0);
	GenericTimerStop(GENERIC_TIMER_ID1);

    /* setup and enable interrupt */
    InterruptSetPriority(GENERIC_TIMER_NS_IRQ_NUM, 0x6);
	InterruptSetPriority(GENERIC_VTIMER_IRQ_NUM, 0x3);

    InterruptInstall(GENERIC_TIMER_NS_IRQ_NUM, FGenericPhysicalTimerHandler, NULL, NULL);
	InterruptInstall(GENERIC_VTIMER_IRQ_NUM, FGenericVirtualTimerHandler, NULL, NULL);
	
    InterruptUmask(GENERIC_TIMER_NS_IRQ_NUM);
	InterruptUmask(GENERIC_VTIMER_IRQ_NUM);

	f_printk("Initializing physical timer, freq = %ld...\r\n", GenericTimerFrequecy());

	p_ticks_per_beat = GenericTimerFrequecy() / PBEATS_PER_SEC;

	f_printk("Initializing virtual timer, freq = %ld...\r\n", GenericTimerFrequecy());

	v_ticks_per_beat = 200;

	GenericTimerSetTimerValue(GENERIC_TIMER_ID0, p_ticks_per_beat);
	GenericTimerInterruptEnable(GENERIC_TIMER_ID0);
	GenericTimerStart(GENERIC_TIMER_ID0);

	GenericTimerSetTimerValue(GENERIC_TIMER_ID1, v_ticks_per_beat);
	GenericTimerInterruptEnable(GENERIC_TIMER_ID1);
	
	
}
