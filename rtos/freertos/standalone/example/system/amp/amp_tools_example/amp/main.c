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
 * FilePath: main.c
 * Date: 2023-09-25 14:53:41
 * LastEditTime: 2023-09-27 17:36:39
 * Description:  This file is for the template example functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   Huanghe    2023/12/29  first release
 */

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif


#include <stdio.h>
#include "fcpu_info.h"
#include "fsleep.h"


int main()
{
    u32 cpu_id ;
    GetCpuId(&cpu_id);
    
    fsleep_seconds(1);

    /* if shell command is not enabled, run example one by one */
    printf("\n\n*********************************************************\n");
    printf("core[%d] Hello 0, Phytium Standalone SDk!\r\n",cpu_id);
    printf("*********************************************************\n");
    return 0;
}