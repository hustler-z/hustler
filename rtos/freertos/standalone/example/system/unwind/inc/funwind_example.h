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
 * FilePath: example.h
 * Created Date: 2023-12-04 16:42:39
 * Last Modified: 2023-12-04 17:36:38
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

#ifndef FUNWIND_EXAMPLE_H
#define FUNWIND_EXAMPLE_H


#ifdef __cplusplus
extern "C"
{
#endif

void FUnwindtestA(void) ;
void FUnwindUndefTest(void) ;
void FUnwindIrqTest(void) ;

#ifdef __cplusplus
}
#endif

#endif // !