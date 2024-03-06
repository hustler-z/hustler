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
 * FilePath: fgeneric_timer.h
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:30:13
 * Description:  This file is for generic timer API's 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  huanghe	2021/11/13		initialization
 *  1.1  zhugengyu 2022/06/05     add tick api
 *  1.2  wangxiaodong 2023/05/29  modify api
 */


#ifndef FGENERIC_TIMER_H
#define FGENERIC_TIMER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ftypes.h"

/* Generic timer register defines. The format is: coproc, opc1, CRn, CRm, opc2 */
#define CNTFRQ		15, 0, 14, 0, 0 /*Counter-timer Frequency register*/
#define CNTP_CTL	15, 0, 14, 2, 1 /*Counter-timer Physical Timer Control register*/
#define CNTP_TVAL	15, 0, 14, 2, 0 /*Counter-timer Physical Timer TimerValue register*/
#define CNTV_CTL	15, 0, 14, 3, 1 /*Counter-timer Virtual Timer Control register*/
#define CNTV_TVAL	15, 0, 14, 3, 0 /*Counter-timer Virtual Timer TimerValue register*/

/* Generic timer 64 bit register defines. The format is: coproc, opc1, CRm */
#define CNTP_CVAL_64	15, 2, 14  /*Counter-timer Physical Timer CompareValue register*/
#define CNTPCT_64	    15, 0, 14  /*Counter-timer Physical Count register*/
#define CNTV_CVAL_64	15, 3, 14  /*Counter-timer Virtual Timer CompareValue register*/
#define CNTVCT_64	    15, 1, 14  /*Counter-timer Virtual Count register*/
#define CNTVOFF_64	    15, 4, 14  /*Counter-timer Virtual Offset register*/

/* Set generic timer CompareValue */
void GenericTimerSetTimerCompareValue(u32 id, u64 timeout);

/* Set generic timer TimerValue */
void GenericTimerSetTimerValue(u32 id, u32 timeout);

/* Unmask generic timer interrupt */
void GenericTimerInterruptEnable(u32 id);

/* Mask generic timer interrupt */
void GenericTimerInterruptDisable(u32 id);

/* Enable generic timer */
void GenericTimerStart(u32 id);

/* Get generic timer physical count value */
u64 GenericTimerRead(u32 id);

/* Get generic timer frequency of the system counter */
u32 GenericTimerFrequecy(void);

/* Disable generic timer */
void GenericTimerStop(u32 id);


#ifdef __cplusplus
}
#endif

#endif