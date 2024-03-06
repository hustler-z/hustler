/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: lwip_timer.c
 * Created Date: 2023-09-20 11:29:05
 * Last Modified: 2023-11-17 17:02:20
 * Description:  This file is for lwip timer function implementation.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong   2023/10/8          first release
 */

#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "fgeneric_timer.h"
#include "ftypes.h"
#include "fparameters.h"
#include "finterrupt.h"
#include "fassert.h"
#include "timeouts.h"


#ifdef CONFIG_USE_LETTER_SHELL
#include "shell_port.h"
#endif

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #error "Please include sdkconfig.h first"
#endif

#include "lwip_port.h"
#include "lwip/ip4_addr.h"
#include "lwip/init.h"
#include "netif/ethernet.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"
#include "lwip/dhcp.h"
#include "ifconfig.h"

#define TIMER_BASE_RATE_HZ 1000 /* 为timer_base_cnt 提供time base */
#define TIMER_INTERRUPT_PRO IRQ_PRIORITY_VALUE_3

#ifdef CONFIG_ARCH_ARMV8_AARCH64
#define MAX_TIMER_CNT 0xffffffffffffffff
#else
#define MAX_TIMER_CNT 0xffffffff
#endif

static u32 timer_base_cnt = 0;


void LwipTestLoop(void)
{
    struct netif *netif;

    netif = netif_list;

    while (netif != NULL)
    {
        if (netif->state)
        {
#ifdef CONFIG_LWIP_RX_POLL
            LwipEthProcessLoop(netif);
#endif
            LwipPortInput(netif);
            LinkDetectLoop(netif);
        }
        netif = netif->next;
    }
}

void LwipTestDhcpLoop(u32 msec)
{
    LwipPortDhcpLoop(msec);
}

static void GenericTimerIntrHandler(s32 vector, void *param)
{
    (void)vector;
    FASSERT(param != 0);
    u32 timer_base_freq = (u32)(uintptr)param;
    timer_base_cnt++;
    /* clear tick interrupt */
    GenericTimerSetTimerValue(GENERIC_TIMER_ID0, GenericTimerFrequecy() / timer_base_freq);
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

#ifdef CONFIG_USE_LETTER_SHELL
void TimerLoop(void)
{
    u32 _5ms_appear = 0;
    u32 base_cnt_back = 0;
    TimerLoopInit();
    LSUserShellInit();

    lwip_init(); /* lwip only init once */
    
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
        if(timer_base_cnt != base_cnt_back)
        {
            sys_check_timeouts();
            base_cnt_back = timer_base_cnt;
        }
    }
}
#else
void TimerStaticInit(void)
{
    TimerLoopInit();
    lwip_init(); /* lwip only init once */
}

/**
 * @name: TimerStaticLoop
 * @msg: 在指定的time时间内进行网卡数据收发
 * @return void
 * @note: 
 * @param {u32} time
 */
void TimerStaticLoop(u32 time)
{
    u32 _5ms_appear = 0;
    u32 base_cnt_back = 0;
    u32 timer_start=0;
    u32 time_pass_cnt=0;
    u32 time_cnt;
    u32 old = 0;

    time_cnt = time * GenericTimerFrequecy();
    timer_start = GenericTimerRead(GENERIC_TIMER_ID0);
    while (time_pass_cnt<time_cnt)
    {
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
        if(timer_base_cnt != base_cnt_back)
        {
            sys_check_timeouts();
            base_cnt_back = timer_base_cnt;
        }

       
        if(GenericTimerRead(GENERIC_TIMER_ID0)>timer_start)
            time_pass_cnt = GenericTimerRead(GENERIC_TIMER_ID0)-timer_start;
        else
            time_pass_cnt = GenericTimerRead(GENERIC_TIMER_ID0)+(MAX_TIMER_CNT-timer_start);
        
        if(old != time-time_pass_cnt/GenericTimerFrequecy())
        {
           printf("Only left %d seconds\r\n", time-time_pass_cnt/GenericTimerFrequecy());
           old = time-time_pass_cnt/GenericTimerFrequecy();
        }
    }

     ListIf();

}
#endif

u32_t sys_now(void)
{
    return timer_base_cnt;
}