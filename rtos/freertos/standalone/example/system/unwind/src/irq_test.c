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
 * FilePath: physical_counter.c
 * Date: 2023-05-56 08:37:22
 * LastEditTime: 2023-05-27 11:00:53
 * Description:  This file is for the physical timer counter test example functions.
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------     --------------------------------------
 * 1.0   wangxiaodong   2023/5/27    first commit 
 */

#include <stdio.h>
#include "fparameters.h"
#include "finterrupt.h"
#include "fprintk.h"
#include "ftypes.h"
#include "fgeneric_timer.h"
#include "fsleep.h"
#include "funwind.h"

/* timer beats per second */
#define BEATS_PER_SEC	1

/*timer interrupt compare type */
#define SET_TIMER_VALUE 1

static u64 ticks_per_beat;
static volatile u64 expected_ticks;
static u32 count = 0;


static volatile int flg  = 0;



static void FGenericPhysicalTimerHandler(s32 vector, void *param)
{
	
	static u64 delta_old;
	u64 delta;
	
	if (vector != GENERIC_TIMER_NS_IRQ_NUM)
		return;

	count++;
	
	delta = GenericTimerRead(GENERIC_TIMER_ID0);

	expected_ticks = delta + ticks_per_beat;

#if SET_TIMER_VALUE
	GenericTimerSetTimerValue(GENERIC_TIMER_ID0, ticks_per_beat);
#else	
	GenericTimerSetTimerCompareValue(GENERIC_TIMER_ID0, expected_ticks);
#endif

	
	FUnwindBacktrace(NULL) ;

	GenericTimerStop(GENERIC_TIMER_ID0);
	InterruptMask(GENERIC_TIMER_NS_IRQ_NUM);

	flg ++ ;
	
	printf("Physical Timer count: %ld, interval ticks: %lld\r\n",
			 count,
			 delta - delta_old);
next:
	
	delta_old = delta;
}

void FUnwindIrqTest(void)
{
	count = 0;
	/* stop timer */
	GenericTimerStop(GENERIC_TIMER_ID0);
    
    /* setup and enable interrupt */
    InterruptSetPriority(GENERIC_TIMER_NS_IRQ_NUM, 2);
    InterruptInstall(GENERIC_TIMER_NS_IRQ_NUM, FGenericPhysicalTimerHandler, NULL, NULL);
    InterruptUmask(GENERIC_TIMER_NS_IRQ_NUM);

	printf("Initializing physical timer, freq = %ld...\n", GenericTimerFrequecy());

	ticks_per_beat = GenericTimerFrequecy() / BEATS_PER_SEC;

	expected_ticks = GenericTimerRead(GENERIC_TIMER_ID0) + ticks_per_beat;

	GenericTimerSetTimerValue(GENERIC_TIMER_ID0, ticks_per_beat);
	GenericTimerInterruptEnable(GENERIC_TIMER_ID0);
	GenericTimerStart(GENERIC_TIMER_ID0);

	while (flg == 0)
	{
		/* code */
	}
	
	flg = 0 ;
}

void FGenericPhysicalTimerStop(void)
{
	GenericTimerStop(GENERIC_TIMER_ID0);
}