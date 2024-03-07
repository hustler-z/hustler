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
 * FilePath: timer_common.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for timer common definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/5/22   first release
 */

#include "fkernel.h"
#include "ferror_code.h"
#ifndef  TIMER_COMMON_H
#define  TIMER_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    u32     id;             /* id of timer */
    boolean bits32;         /*otherwise 64 bit*/
    boolean restartmode;    /*otherwise free-run*/
    boolean cyc_cmp;         /*otherwise once cmp*/
    boolean clear_cnt;       /*otherwise not clear*/
    boolean forceload;      /*otherwise not force-load*/
    u32 startcnt;           /*start cnt num*/
    u32 cmptick32;          /*32bit cnt num*/
    u64 cmptick64;          /*64bit cnt num*/
} TimerTestConfigs;

#define MAX_32_VAL GENMASK(31, 0)
#define MAX_64_VAL GENMASK_ULL(63, 0)

#define US2TICKS(us) ((FTIMER_CLK_FREQ_HZ * (us) / 1000000ULL ) + 1ULL)
#define MS2TICKS(ms) (US2TICKS(1000ULL) * (ms))

#define TIMER_EXAMPLE_TEST_ID 0

/*init timer*/
FError FTimerFunctionInit(u8 id, boolean timer_mode, u64 times);
/*start timer */
FError FTimerStartTest(u64 times, boolean forceLoad);
/*deinit timer*/
void FTimerDeInitTest(void);

#ifdef __cplusplus
}
#endif

#endif

