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
 * Created Date: 2023-09-20 11:29:05
 * Last Modified: 2023-10-11 15:09:18
 * Description:  This file is for lwip startup example main functions.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong   2023/10/8          first release
 */



#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "lwip_dhcp_example.h"
#include "lwip_ipv4_example.h"
#include "lwip_ipv6_example.h"
#include "lwip_timer.h"
int main()
{

#ifdef CONFIG_USE_LETTER_SHELL
    /* if shell command is enabled, register example entry as shell command */
   TimerLoop();    
#else
    TimerStaticInit();
    
    LwipIpv4TestCreate();
    TimerStaticLoop(10);
    LwipIpv4TestDeinit();

    LwipIpv6TestCreate();
    TimerStaticLoop(10);
    LwipIpv6TestDeinit();

    LwipDhcpTestCreate();
    TimerStaticLoop(10);
    LwipDhcpTestDeinit();

#endif
    return 0;
}

