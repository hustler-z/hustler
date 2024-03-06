/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: fsleep.c
 * Date: 2021-07-01 18:40:52
 * LastEditTime: 2022-02-17 18:02:45
 * Description:  This file is for creating custom sleep interface for standlone sdk.
 *
 * Modify History:
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huanghe      2022/7/23            first release
 */


#include "fsleep.h"
#include "fgeneric_timer.h"
#include "fkernel.h"
#include "fparameters.h"
#include "ftypes.h"

#define TIMER_COUNT_MAX  -1ULL

static u32 fsleep_general(u32 delay_time, u32 resolution)
{
    volatile u64 cur_tick;
    volatile u64 old_tick;
    volatile u64 pass_tick = 0;
    volatile u64 need_tick;

    GenericTimerStart(GENERIC_TIMER_ID0);
    need_tick = ((u64)delay_time * GenericTimerFrequecy() / resolution);
    old_tick = GenericTimerRead(GENERIC_TIMER_ID0);

    while (pass_tick < need_tick)
    {
        cur_tick = GenericTimerRead(GENERIC_TIMER_ID0);

        if (cur_tick == old_tick)
            continue;
        else if (cur_tick > old_tick)
            pass_tick += cur_tick - old_tick;
        else
            pass_tick += cur_tick + TIMER_COUNT_MAX - old_tick;

        old_tick = cur_tick;
    }

    return 0;
}

u32 fsleep_seconds(u32 seconds)
{
    return fsleep_general(seconds, 1);
}

u32 fsleep_millisec(u32 mseconds)
{
    return fsleep_general(mseconds, NANO_TO_MICRO);
}

u32 fsleep_microsec(u32 mseconds)
{
    return fsleep_general(mseconds, NANO_TO_KILO);
}