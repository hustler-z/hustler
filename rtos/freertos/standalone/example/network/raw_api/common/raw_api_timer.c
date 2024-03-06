/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: raw_api_timer.c
 * Date: 2022-11-01 15:52:09
 * LastEditTime: 2022-11-01 15:52:10
 * Description:  This file is for create a timer.The timer can keep the netif receiveing message periodically.
 *
 * Modify History:
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0  liuzhihong     2022/11/15            first release
 *  1.1  liuzhihong     2022/11/23           add func sys_now()
 *  1.2  liuzhihong     2022/1/9               u64 -> u32 
 */

#include "fgeneric_timer.h"
#include "fparameters.h"
#include "finterrupt.h"
#include "fassert.h"
#include "shell_port.h"
#include "timeouts.h"

#define TIMER_BASE_RATE_HZ 1000
#define TIMER_INTERRUPT_PRO IRQ_PRIORITY_VALUE_3

static u32 timer_base_cnt = 0;
extern void LwipTestLoop(void);
extern void LwipTestDhcpLoop(u32 msec);

static void GenericTimerIntrHandler(s32 vector, void *param)
{
    (void)vector;
    FASSERT(param != 0);
    u32 timer_base_freq = (u32)(uintptr)param;
    timer_base_cnt++;
    GenericTimerSetTimerValue(GENERIC_TIMER_ID0, GenericTimerFrequecy() / timer_base_freq);/* clear tick interrupt */
}

void TimerLoopInit(void)
{
    u32 cnt_frq;
    timer_base_cnt = 0;
    /* disable timer and get system frequency */
    GenericTimerStop(GENERIC_TIMER_ID0);
    cnt_frq = GenericTimerFrequecy();

    /* set tick rate */
    GenericTimerSetTimerValue(GENERIC_TIMER_ID0, cnt_frq / TIMER_BASE_RATE_HZ);
    GenericTimerInterruptEnable(GENERIC_TIMER_ID0);

    /* set generic timer interrupt */
    InterruptSetPriority(GENERIC_TIMER_NS_IRQ_NUM, TIMER_INTERRUPT_PRO);

    /* install tick handler */
    InterruptInstall(GENERIC_TIMER_NS_IRQ_NUM, GenericTimerIntrHandler,
                     (void *)TIMER_BASE_RATE_HZ, "GenericTimerTick");

    /* enable interrupt */
    InterruptUmask(GENERIC_TIMER_NS_IRQ_NUM);
    GenericTimerStart(GENERIC_TIMER_ID0);
}


void TimerLoop(void)
{
    static u32 _5ms_appear = 0;
    TimerLoopInit();
    LSUserShellInit();

    while (1)
    {

        LSuserShellNoWaitLoop();
        LwipTestLoop();

        if (((timer_base_cnt % 5) == 0) && (_5ms_appear == 0)) /*5ms task */
        {
            _5ms_appear = 1;
            LwipTestDhcpLoop(5);
        }
        else if ((timer_base_cnt % 5) != 0)
        {
            _5ms_appear = 0;
        }
        sys_check_timeouts();
    }
}
u32_t sys_now(void)
{
    return timer_base_cnt;
}