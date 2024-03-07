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
 * FilePath: timer_common.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2024-02-19 14:51:22
 * Description:  This file is for timer example common functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/3/31   first release
 * 1.1   liusm      2024/2/19   fix bug of bit64
 */

/***************************** Include Files *********************************/

#include <string.h>
#include "fkernel.h"
#include "ftimer_tacho.h"
#include "timer_common.h"
#include "fparameters.h"
#include "fsleep.h"
#include "finterrupt.h"
#include "fassert.h"
#include "fcpu_info.h"
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/
u64 CntTick = 0;
static FTimerTachoCtrl timer;
static TimerTestConfigs timercfg;
volatile int timerflag;

/************************** Function Prototypes ******************************/
static void CycCmpIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    timerflag++;
    printf("The cyc intr,id: %d,times_in: %d\n\r\n", instance_p->config.id, timerflag);
}

static void OnceCmpIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;

    FTimerSetInterruptMask(instance_p, FTIMER_EVENT_ONCE_CMP, FALSE);
    FTimerStop(instance_p);
    printf("The once cmp intr, timer id: %d\r\n", instance_p->config.id);
    timerflag = 1;
}

/*此中断已经在驱动层进行了屏蔽，由于我们设置的cmp值已经是翻转值，所以等同于中断计数中断，此处可作为用法的拓展*/
static void RolloverIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    if (timerflag == 1 && instance_p->config.cmp_type == FTIMER_ONCE_CMP)
    {
        timerflag == 0;
    }
    else
    {
        timerflag = 1;/* Anything else that you can do.*/
    }
    printf("The roll over cmp intr, timer id: %d\r\n", instance_p->config.id);
}

/**
 * @name: TimerDisableIntr
 * @msg: 失能中断
 * @return {void}
 * @param {FTimerTachoCtrl} *instance_p 驱动控制数据结构
 */
void TimerDisableIntr(FTimerTachoCtrl *instance_p)
{
    u32 irqID = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);

    InterruptMask(irqID);
}

/**
 * @name: TimerEnableIntr
 * @msg: 设置并且使能中断
 * @return {void}
 * @param  {FTimerTachoCtrl} *instance_p 驱动控制数据结构
 */
void TimerEnableIntr(FTimerTachoCtrl *instance_p)
{
    FASSERT(instance_p);

    u32 irqID = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);

    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(irqID, cpu_id);
    /* disable timer irq */
    InterruptMask(irqID);

    /* umask timer irq */
    InterruptSetPriority(irqID, instance_p->config.irq_priority);
    InterruptInstall(irqID, FTimerTachoIntrHandler, instance_p, instance_p->config.name);

    FTimerTachoSetIntr(instance_p);
    /* enable irq */
    InterruptUmask(irqID);

    return ;
}

/**
 * @name: FTimerCfgInit
 * @msg: 加载转换后配置项，并完成初始化操作，定时器处于就绪状态
 * @return {FError} 驱动初始化的错误码信息，FTIMER_TACHO_SUCCESS 表示初始化成功，其它返回值表示初始化失败
 * @param {TimerTestConfigs} *timercfg_p 可操作的配置参数结构体
 */
static FError FTimerCfgInit(const TimerTestConfigs *timercfg_p)
{
    u32 ret = FTIMER_TACHO_SUCCESS;

    FTimerTachoConfig *pconfig = &timer.config;

    memset(&timer, 0, sizeof(timer));
    FTimerGetDefConfig(timercfg_p->id, pconfig);

    if (timercfg_p->restartmode)
    {
        pconfig->timer_mode = FTIMER_RESTART;
    }
    else
    {
        pconfig->timer_mode = FTIMER_FREE_RUN;
    }

    if (timercfg_p->bits32)
    {
        pconfig->timer_bits = FTIMER_32_BITS;
    }
    else
    {
        pconfig->timer_bits = FTIMER_64_BITS;
    }

    if (timercfg_p->cyc_cmp)
    {
        pconfig->cmp_type = FTIMER_CYC_CMP;
        FTimerRegisterEvtCallback(&timer, FTIMER_EVENT_CYC_CMP, CycCmpIntrHandler);
    }
    else
    {
        pconfig->cmp_type = FTIMER_ONCE_CMP;
        FTimerRegisterEvtCallback(&timer, FTIMER_EVENT_ONCE_CMP, OnceCmpIntrHandler);
    }

    FTimerRegisterEvtCallback(&timer, FTIMER_EVENT_ROLL_OVER, RolloverIntrHandler);

    ret = FTimerInit(&timer, pconfig);
    if (FTIMER_TACHO_SUCCESS != ret)
    {
        return ret;
    }

    /*将时间参数us装换成计时器的ticks，我们设置StartTick，将CmpTick设置为最大*/
    FTimerSetStartVal(&timer, timercfg.startcnt);

    if (timercfg_p->bits32)
    {
        ret |= FTimerSetPeriod32(&timer, timercfg.cmptick32);
    }
    else
    {
        ret |= FTimerSetPeriod64(&timer, timercfg.cmptick64);
    }

    printf("Timer Init finished\r\n");
    return ret;
}

/**
 * @name: FTimerFunctionInit
 * @msg:  timer init.
 * @param {u8}id :use 0~37 timer
 * @param {boolean}timer_mode:单次定时还是循环定时
 * @param {u64}times:定时时间，单位us
 * @return {FError}
 */
FError FTimerFunctionInit(u8 id, boolean timer_mode, u64 times)
{
    timercfg.id = id;
    timercfg.cyc_cmp = timer_mode;
    CntTick = US2TICKS(times);
    printf("\n***CntTick: %llu\r\n", CntTick);
    if (CntTick > 0xffffffff)
    {
        timercfg.bits32 = FALSE;
        timercfg.startcnt = 0;
        timercfg.cmptick64 = CntTick;
    }
    else
    {
        timercfg.bits32 = TRUE;
        timercfg.startcnt = MAX_32_VAL - CntTick;
        timercfg.cmptick32 = MAX_32_VAL;/* Set CmpTick max value ,that we can easy to trigger RolloverIntr. */
    }

    return FTimerCfgInit(&timercfg);
}

/**
 * @name: FTimerStartTest
 * @msg:  start timer.
 * @param {u64 times,boolean forceLoad}
 * @return {FError}
 */
FError FTimerStartTest(u64 times, boolean forceLoad)
{
    u32 ret = FTIMER_TACHO_SUCCESS;

    /* 如果使用单次模式，可以传递参数，进行二次或者多次定时 */
    if (0 != times && (timer.config.cmp_type == FTIMER_ONCE_CMP))
    {
        timer.config.force_load = forceLoad;
        /*32bit 64bit,可能发生动态切换，定时超过 85.899346 s = 85899.346 ms = 85899346 us*/
        CntTick = US2TICKS(times);
        if (CntTick > 0xffffffff)
        {
            timer.config.timer_bits = FTIMER_64_BITS;
            timercfg.startcnt = 0;
        }
        else
        {
            timer.config.timer_bits = FTIMER_32_BITS;
            timercfg.startcnt = MAX_32_VAL - CntTick;
        }

        ret = FTimerSetStartVal(&timer, timercfg.startcnt);

        if (FTIMER_TACHO_SUCCESS != ret)
        {
            return ret;
        }
    }

    /*unmask intr*/
    TimerEnableIntr(&timer);

    ret = FTimerStart(&timer);
    if (FTIMER_TACHO_SUCCESS != ret)
    {
        return ret;
    }

    timerflag = 0;      /*changed in interrupt handle function*/
    if (timer.config.timer_bits == FTIMER_32_BITS)
    {
        for (size_t i = 0; i < 5; i++)
        {
            /* code */
            printf("The cur 32cnt: %x\n", FTimerGetCurCnt32(&timer));
            fsleep_millisec(500);
        }
    }
    else
    {
        for (size_t i = 0; i < 5; i++)
        {
            /* code */
            printf("The cur 64cnt: %llx\n", FTimerGetCurCnt64(&timer));
            fsleep_millisec(500);
        }
    }

    return ret;
}

/**
 * @name: FTimerStopTest
 * @msg:  stop timer.
 * @param {void}
 * @return {void}
 */
void FTimerStopTest(void)
{
    FTimerStop(&timer);
    return;
}

/**
 * @name: FTimerDeInitTest
 * @msg:  deinit timer.
 * @param {void}
 * @return {void}
 */
void FTimerDeInitTest(void)
{
    /* disable timer irq */
    TimerDisableIntr(&timer);
    FTimerDeInit(&timer);
    return;
}

/**
 * @name: FTimerDebug
 * @msg:  printf some timer registers value.
 * @param {void}
 * @return {void}
 */
void FTimerDebug(void)
{
    FTimeSettingDump(&timer);
    return;
}