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
 * FilePath: tacho_common.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for tacho common definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/5/22   first release
 */

#include "fkernel.h"
#include "ferror_code.h"
#include "ftypes.h"
#include "fparameters.h"
#ifndef  TACHO_COMMON_H
#define  TACHO_COMMON_H


#if defined(CONFIG_TARGET_PHYTIUMPI)
#define TIMER_TACHO_ID FTACHO1_ID
#else
#define TIMER_TACHO_ID FTACHO12_ID
#endif

#ifdef __cplusplus
extern "C"
{
#endif
typedef struct
{
    u32     id;             /* id of tacho */
    boolean work_mode;      /*tacho or capture*/
    boolean bits32;         /*otherwise 64 bit*/
    boolean restart_mode;    /*otherwise free-run*/
    u32     plus_num;  /* plus_num of period to calculate rpm */
    u8      edge_mode;      /* rising falling or double edge*/
    u8      jitter_level;       /* anti_jitter_number 0~3 */
    u8      captue_cnt;     /* capture mode,set the number of pulses to trigger interrupt */
} TachoTestConfigs;

#define MAX_32_VAL GENMASK(31, 0)
#define MAX_64_VAL GENMASK_ULL(63, 0)

#define TACHO_MAX   10000
#define TACHO_MIN   10
#define TACHO_PERIOD 1000000 /* 1000000/50000000 = 0.02s  20ms ticks period at 50Mhz pclk*/
#define CAPTURE_MAX 0x7f

/*init tacho */
FError FTachoFunctionInit(u8 id, boolean tacho_mode);
/*get the rpm*/
FError FTachoGetRPM(void);
/*deinit tacho*/
void FTachoDeinitTest(void);
/*get the capture count*/
void FTachoPrintCaptureCnt(void);

#ifdef __cplusplus
}
#endif

#endif