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
 * FilePath: timer.c
 * Date: 2022-08-23 09:17:20
 * LastEditTime: 2022-08-23 09:17:20
 * Description:  This file is for provide the timer for test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2022/12/20  Modify the format and establish the version
 */

/***************************** Include Files *********************************/

#include "fgeneric_timer.h"
#include "finterrupt.h"
#include "ftypes.h"
#include "fparameters.h"
#include "fdebug.h"
#include "fassert.h"
#include "shell_port.h"
#include "lvgl_disp_test.h"
#include "../lvgl.h"
#include "lv_conf.h"

/************************** Variable Definitions *****************************/
#ifdef __aarch64__
    volatile u64 TIMER_BASE_RATE_HZ = 1000;
#else
    volatile u32 TIMER_BASE_RATE_HZ = 1000;
#endif
#define TIMER_INTERRUPT_PRO IRQ_PRIORITY_VALUE_3

static u64 timer_base_cnt = 0;
/************************** Function Prototypes ******************************/

static void GenericTimerIntrHandler(s32 vector, void *param)
{
    (void)vector;
    FASSERT(param != 0);
    u32 timer_base_freq = (u32)(uintptr)param;
    timer_base_cnt++;
    lv_tick_inc(1);
    GenericTimerSetTimerValue(GENERIC_TIMER_ID0, GenericTimerFrequecy() / timer_base_freq);/* clear tick interrupt */
}

void LvglTimerLoopInit(void)
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


void LvglTimerLoop(void)
{
    static u32 _5ms_appear = 0;
    LvglTimerLoopInit();
    LSUserShellInit();

    while (1)
    {
        LSuserShellNoWaitLoop();

        if (((timer_base_cnt % 5) == 0) && (_5ms_appear == 0)) /*5ms task */
        {
            _5ms_appear = 1;
            lv_hpd_detect();
        }
        else if ((timer_base_cnt % 5) != 0)
        {
            _5ms_appear = 0;
        }
    }
}


