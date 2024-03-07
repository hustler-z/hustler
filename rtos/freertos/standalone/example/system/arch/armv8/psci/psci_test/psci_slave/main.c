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
 * FilePath: main.c
 * Created Date: 2023-06-16 13:26:03
 * Last Modified: 2023-06-26 18:18:24
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe     2023-06-16          init code
 */



#include <stdio.h>
#include "sdkconfig.h"
#include "ftypes.h"
#include "shell_port.h"
#include "fpsci.h"
#include "fsleep.h"


void SlaveTestCode(void)
{
    int cnt = 5;
    printf("stat to slave test\n");
    do
    {
        /* code */
        fsleep_millisec(1000) ;
        printf("slave: count down %d \n",cnt);
        cnt -- ;
    } while (cnt > 0);
    FPsciCpuOff() ;
    printf("code must die \n");
}


int main(void)
{
    // LSUserShellLoop();

    SlaveTestCode() ;
}



