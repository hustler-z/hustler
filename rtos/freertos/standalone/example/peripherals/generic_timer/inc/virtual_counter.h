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
 * FilePath: virtual_counter.h
 * Date: 2023-05-25 14:53:41
 * LastEditTime: 2023-05-27 17:36:39
 * Description:  This file is for the generic timer virtual counter test example functions definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2023/5/27  first release
 */


#ifndef  TIMER_TEST_EXAMPLE_H
#define  TIMER_TEST_EXAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif


void FGenericPhysicalTimerStart(void);
void FGenericPhysicalTimerStop(void);

void FGenericVirtualTimerStart(void);
void FGenericVirtualTimerStop(void);

#ifdef __cplusplus
}
#endif

#endif