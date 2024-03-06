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
 * Created Date: 2023-10-16 15:16:18
 * Last Modified: 2023-10-24 14:05:38
 * Description:  This file is for lwip iperf example main functions.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong   2023/10/16          first release
 */



#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "lwip_iperf_client_example.h"
#include "lwip_iperf_server_example.h"
#include "lwip_timer.h"

int main()
{

#ifdef CONFIG_USE_LETTER_SHELL
    /* if shell command is enabled, register example entry as shell command */
   TimerLoop();    
#else
    TimerStaticInit();
    
    LwipIperfClientCreate();
    TimerStaticLoop(15);
    LwipIperfClientDeinit();
    
    LwipIperfServerCreate();
    TimerStaticLoop(15);
    LwipIperfServerDeinit();
#endif
    return 0;
}

