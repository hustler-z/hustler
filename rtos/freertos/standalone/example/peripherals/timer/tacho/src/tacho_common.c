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
 * FilePath: tacho_common.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for tacho common definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/5/22   first release
 */

#include <string.h>
#include "fkernel.h"
#include "ftimer_tacho.h"
#include "fparameters.h"
#include "finterrupt.h"
#include "fsleep.h"
#include "fassert.h"
#include "fcpu_info.h"
#include "tacho_common.h"
#include "fio_mux.h"

static FTimerTachoCtrl tacho;
static TachoTestConfigs tachocfg;\




/************************** Function Prototypes ******************************/
static void TachOverIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);
    InterruptMask(irq_num);
    u32 rpm;
    FTachoGetFanRPM(instance_p, &rpm);
    printf("TachOver intr,tacho id: %d,rpm: %d\r\n", instance_p->config.id, rpm);
    InterruptUmask(irq_num);
}

static void CapIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);
    InterruptMask(irq_num);
    printf("TachCapt intr,tacho id: %d\r\n", instance_p->config.id);
    InterruptUmask(irq_num);
}

static void TachUnderIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);
    InterruptMask(irq_num);
    u32 rpm;
    FTachoGetFanRPM(instance_p, &rpm);
    printf("TachUnder intr,tacho id: %d,rpm: %d\r\n", instance_p->config.id, rpm);
    InterruptUmask(irq_num);
}

void TachoDisableIntr(FTimerTachoCtrl *instance_p)
{
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);

    InterruptMask(irq_num);
}

void TachoEnableIntr(FTimerTachoCtrl *instance_p)
{
    FASSERT(instance_p);

    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);

    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(irq_num, cpu_id);

    /* disable timer irq */
    InterruptMask(irq_num);

    /* umask timer irq */
    InterruptSetPriority(irq_num, instance_p->config.irq_priority);
    InterruptInstall(irq_num, FTimerTachoIntrHandler, instance_p, instance_p->config.name);

    FTimerTachoSetIntr(instance_p);
    /* enable irq */
    InterruptUmask(irq_num);

    return;
}

static FError FTachoCfgInit(const TachoTestConfigs *tachocfg_p)
{
    u32 ret = FTIMER_TACHO_SUCCESS;

    FTimerTachoConfig *pconfig = &tacho.config;
    memset(&tacho, 0, sizeof(tacho));
    /* tacho  */
    FTachoGetDefConfig(tachocfg_p->id, pconfig);

    if (tachocfg_p->work_mode == FTIMER_WORK_MODE_TACHO)
    {
        pconfig->work_mode = FTIMER_WORK_MODE_TACHO;

        if (tachocfg_p->bits32 == FTIMER_32_BITS)
        {
            pconfig->timer_bits = FTIMER_32_BITS;
        }
        else
        {
            pconfig->timer_bits = FTIMER_64_BITS;
        }

        if (tachocfg_p->restart_mode)
        {
            pconfig->timer_mode = FTIMER_RESTART;
        }
        else
        {
            pconfig->timer_mode = FTIMER_FREE_RUN;
        }

        FTimerRegisterEvtCallback(&tacho, FTACHO_EVENT_OVER, TachOverIntrHandler);
        FTimerRegisterEvtCallback(&tacho, FTACHO_EVENT_UNDER, TachUnderIntrHandler);
    }
    else
    {
        pconfig->work_mode = FTIMER_WORK_MODE_CAPTURE;
        pconfig->captue_cnt = tachocfg.captue_cnt;/* 边沿检测计数默认值 */
        FTimerRegisterEvtCallback(&tacho, FTACHO_EVENT_CAPTURE, CapIntrHandler);
    }

    if (tachocfg_p->edge_mode == FTACHO_RISING_EDGE)
    {
        pconfig->edge_mode = FTACHO_RISING_EDGE;
    }
    else if (tachocfg_p->edge_mode == FTACHO_FALLING_EDGE)
    {
        pconfig->edge_mode = FTACHO_FALLING_EDGE;
    }
    else
    {
        pconfig->edge_mode = FTACHO_DOUBLE_EDGE;
    }

    switch (tachocfg_p->jitter_level)
    {
        case FTACHO_JITTER_LEVEL0:
            pconfig->jitter_level = FTACHO_JITTER_LEVEL0;
            break;
        case FTACHO_JITTER_LEVEL1:
            pconfig->jitter_level = FTACHO_JITTER_LEVEL1;
            break;
        case FTACHO_JITTER_LEVEL2:
            pconfig->jitter_level = FTACHO_JITTER_LEVEL2;
            break;
        case FTACHO_JITTER_LEVEL3:
            pconfig->jitter_level = FTACHO_JITTER_LEVEL3;
            break;
        default:
            pconfig->jitter_level = FTACHO_JITTER_LEVEL0;
            break;
    }

    if (tachocfg_p->plus_num != 0)
    {
        pconfig->plus_num = tachocfg_p->plus_num;
    }

    ret = FTachoInit(&tacho, pconfig);
}

static void FTachoSetMaxMin(u32 over_limt, u32 under_limt)
{
    FTachoSetOverLimit(&tacho, over_limt);
    FTachoSetUnderLimit(&tacho, under_limt);
}

FError FTachoFunctionInit(u8 id, boolean tacho_mode)
{
    u32 ret;
    tachocfg.id = id;
    tachocfg.edge_mode = FTACHO_RISING_EDGE;/* Not open operation interface for cmd */
    tachocfg.jitter_level = FTACHO_JITTER_LEVEL0;/* Not open operation interface for cmd */
    tachocfg.bits32 = FTIMER_32_BITS;/* Not open operation interface for cmd.*/
    tachocfg.restart_mode = FTIMER_RESTART;/* Not open operation interface for cmd.*/
    tachocfg.captue_cnt = CAPTURE_MAX;/* Not open operation interface for cmd.*/
    tachocfg.plus_num = TACHO_PERIOD;
    if (tacho_mode)
    {
        tachocfg.work_mode = FTIMER_WORK_MODE_TACHO;
    }
    else
    {
        tachocfg.work_mode = FTIMER_WORK_MODE_CAPTURE;
    }
    /*init iomux*/
    FIOMuxInit();
    FIOPadSetTachoMux(TIMER_TACHO_ID);
    ret = FTachoCfgInit(&tachocfg);
    if (ret != FT_SUCCESS)
    {
        return ret;
    }

    if (tacho_mode)
    {
        FTachoSetMaxMin(TACHO_MAX, TACHO_MIN); /* Not open operation interface for cmd */
    }
    TachoEnableIntr(&tacho);

    FTimerStart(&tacho);
}

FError FTachoGetRPM(void)
{
    u32 TachoRPM;
    u32 ret = FTIMER_TACHO_SUCCESS;

    ret = FTachoGetFanRPM(&tacho, &TachoRPM);
    if (ret != FTIMER_TACHO_SUCCESS)
    {
        printf("Tachometer get error,please check init\r\n");
        return ret;
    }

    printf("Tachometer RPM : %d\r\n", TachoRPM);

    return ret;
}

void FTachoPrintCaptureCnt(void)
{
    u32 cnt = 0;
    cnt = FTachoGetCaptureCnt(&tacho);
    printf("****Get CaptureCnt is : %d\r\n", cnt);
}

void FTachoDeinitTest(void)
{
    /* disable tacho irq */
    TachoDisableIntr(&tacho);
    FTimerStop(&tacho);
    FTachoDeInit(&tacho);
    /*init iomux */
    FIOMuxDeInit();
    return;
}
