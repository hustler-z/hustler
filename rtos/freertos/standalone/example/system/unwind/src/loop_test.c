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
 * FilePath: loop_test.c
 * Created Date: 2023-12-04 16:44:36
 * Last Modified: 2023-12-04 17:38:51
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */
#include "funwind.h"



static void FUnwindtestB(void);

static void FUnwindtestC(void);


void FUnwindtestA(void) 
{

    FUnwindtestB();

}

static void FUnwindtestB(void) 
{
    FUnwindtestC();
}

static void FUnwindtestC(void) 
{
    FUnwindBacktrace(NULL) ;
}