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
 * FilePath: psci_test.c
 * Created Date: 2023-06-25 09:27:28
 * Last Modified: 2023-12-08 18:33:32
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe   2023-06-27         first release
 */


#include "fcpu_info.h"
#include "fpsci.h"
#include "stdio.h"
#include "fcpu_info.h"
#include "../psci_slave/sdkconfig.h"


void FPsciFeatureTest(void)
{
    int psci_version = 0;
    u32 cpu_id;
    unsigned long cpu_affinity ;
    int affinity_power_state = 0;
    GetCpuId(&cpu_id);
    GetCpuAffinity(cpu_id,(u64 *)&cpu_affinity) ;
    /* psci version */
    psci_version = FPsciVersion() ;
    printf("major is 0x%x,minor is 0x%x \r\n", FPSCI_MAJOR_VERSION(psci_version),FPSCI_MINOR_VERSION(psci_version)) ;
    /* psci affinity info */
    affinity_power_state = FPsciAffinityInfo(cpu_affinity,0) ;
    printf("current core power state is 0x%x \r\n", affinity_power_state) ;
}


void FPsciHotplugTest(void)
{
    FPsciCpuMaskOn(1<<CONFIG_IMAGE_CORE,CONFIG_IMAGE_LOAD_ADDRESS) ;
}

