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
 * FilePath: ppi_example.c
 * Date: 2023-04-26 08:37:22
 * LastEditTime: 2022-04-27 11:00:53
 * Description:  This file is for the ppi test example functions.
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------     --------------------------------------
 * 1.0   liqiaozhong   2023/4/27    first commit and add spi, sgi, spi functions
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "finterrupt.h"
#include "fgeneric_timer.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "gic_common.h"
#include "ppi_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static volatile u32 timer_cnt = 0;

/* user defined parameters */
static u32 priority = IRQ_PRIORITY_VALUE_0;
static u32 tick_rate_hz = 1000;
/***************** Macros (Inline Functions) Definitions *********************/
#define PPI_TEST_MAX_TIME_CNT 60
/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
static void GenericTimerTickIntrHandler(s32 vector, void *param)
{
    long tick_rate_hz = (long)param;

    if (timer_cnt ++ >= PPI_TEST_MAX_TIME_CNT)
    {
        GenericTimerStop(GENERIC_TIMER_ID0);
    }

    GenericTimerSetTimerValue(GENERIC_TIMER_ID0, GenericTimerFrequecy() / tick_rate_hz); /* clear tick interrupt */
}

/* early data init */
static void FGicPpiEarlyInit(void)
{
    // wait to add
}

/* PPI clock intr register */
static void FGicPpiInit(u32 tick_rate_hz, u32 prority)
{
    u32 cntfrq;
    long rate_hz = 0;
    rate_hz = tick_rate_hz;
    /* disable timer and get system frequency */
    GenericTimerStop(GENERIC_TIMER_ID0);
    cntfrq = GenericTimerFrequecy();

    /* set tick rate */
    GenericTimerSetTimerValue(GENERIC_TIMER_ID0, cntfrq / tick_rate_hz);
    GenericTimerInterruptEnable(GENERIC_TIMER_ID0);

    /* set generic timer interrupt */
    InterruptSetPriority(GENERIC_TIMER_NS_IRQ_NUM, prority);

    InterruptInstall(GENERIC_TIMER_NS_IRQ_NUM, GenericTimerTickIntrHandler,
                     (void *)rate_hz, "GenericTimerTick");

    /* enable interrupt */
    InterruptUmask(GENERIC_TIMER_NS_IRQ_NUM);
}

static int FGicPpiTest(void)
{
    u32 temp_timer_cnt = 0;
    u32 cnt = 0;

    timer_cnt = 0;
    GenericTimerStart(GENERIC_TIMER_ID0);

    while (timer_cnt != PPI_TEST_MAX_TIME_CNT)
    {
        if ((cnt++) >= 0xfffffff)
        {
            FGIC_ERROR("FGicPpiTest timeout.");
            return -1;
        }

        if (temp_timer_cnt != timer_cnt)
        {
            cnt = 0;
            temp_timer_cnt = timer_cnt;
        }
    }
    printf("FGicPpiTest is ok. \r\n");
    return FGIC_EXAMPLE_OK;
}

/* function of PPI intr example */
int FPpiExample(void)
{
    int ret = FGIC_EXAMPLE_OK;
    int err = FGIC_EXAMPLE_OK;

    FGicPpiEarlyInit();
    FGicPpiInit(tick_rate_hz, priority);
    FGicPpiTest();

    /* print message on example run result */
    if (FGIC_EXAMPLE_OK == err)
    {
        printf("%s@%d: ppi example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: ppi example failed !!!, err = %d \r\n", __func__, __LINE__, err);
    }

    return FGIC_EXAMPLE_OK;
}