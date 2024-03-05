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
 * FilePath: scmi_mhu_search_example.c
 * Date: 2023-07-27 14:53:42
 * LastEditTime: 2023-07-27 18:55:74
 * Description:  This file is for scmi mhu search example function implementation.
 *
 * Modify History:
 *  Ver     Who          Date             Changes
 * -----   ------      --------    --------------------------------------
 *  1.0   liqiaozhong  2023/7/27   add scmi mhu search example
 *  1.1   liusm        2023/12/01  Change the performance domain
 */

/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "fsleep.h"
#include "fdebug.h"
#include "fmmu.h"
#include "fprintk.h"
#include "fgeneric_timer.h"

#include "fmhu.h"
#include "fscmi.h"
#include "fparameters.h"
#include "sdkconfig.h"
#include "ferror_code.h"
#include "fscmi_base.h"
#include "fscmi_sensors.h"
#include "fscmi_perf.h"

#include "scmi_mhu_search_example.h"

#define SCMI_MHU_FCHAN_ID   (1U << 0)
#define SCMI_MHU_POLL_MODE  TRUE

static FScmi scmi;
static struct FScmiConfig scmi_config;
/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
/* function of AP search SE by SCMI-MHU example */
int FScmiMhuSearchExample(void)
{
    FError ret;
    s64 temp;
    u32 i;
    u32 max_perf = 0;
    u32 min_perf = 0;
    u32 opp_num = 0; /* opp_num 0-3 ,You can get frequency from scmi.perf->dom_info->opp when you init first. Open menuconfig debug opp_num on INFO */
    u64 freq = 0; /* Save CPU frequency to printf */
    struct FScmiPerfInfo *pinfo;
    pinfo = &scmi.perf;
    u32 cpu_id = 0;

    GetCpuId(&cpu_id);
    printf("cpu_id:%d.\r\n",cpu_id);
    memset(&scmi, 0, sizeof(FScmi));
    scmi_config.mbox_type = FSCMI_MBOX_MHU_TYPE;
    scmi_config.share_mem = FSCMI_SHR_MEM_ADDR;

    FMhuGetDefConfig(&scmi.scmi_mhu.mhu.config, SCMI_MHU_FCHAN_ID);
    
    ret = FScmiCfgInitialize(&scmi, &scmi_config);
    if (ret != FT_SUCCESS)
    {
        return -2;
    }
    
    printf("SCMI Protocol v%d.%d '%s:%s' Firmware version 0x%x\n", scmi.revision.major_ver,
                                                                    scmi.revision.minor_ver,
                                                                    scmi.revision.vendor_id,
                                                                    scmi.revision.sub_vendor_id,
                                                                    scmi.revision.impl_ver);
    printf("Sensor Version %d.%d\n",scmi.sensors.major_ver, scmi.sensors.minor_ver);
    printf("Found sensors num:%d.\r\n", scmi.sensors.num_sensors);
    for ( i = 0; i < scmi.sensors.num_sensors; i++)
    {
        printf("FScmiSensorGet:id:%d,type:%d,name:%s.\r\n", scmi.sensors.sensor_info[i].id, scmi.sensors.sensor_info[i].type, scmi.sensors.sensor_info[i].name);
        ret = FScmiSensorGetTemp(&scmi, i, &temp);
        f_printk("Get temperature value:%lld `C.\r\n",temp);
        if (ret != FT_SUCCESS)
        {
            return -2;
        }
    }

    printf("Perf Version %d.%d\n",scmi.perf.major_ver, scmi.perf.minor_ver);
    printf("Found perf num_domains:%d,power_scale_mw:%d.\r\n",scmi.perf.num_domains,scmi.perf.power_scale_mw);
    for (i = 0; i < scmi.perf.num_domains; i++)
    {
        struct FPerfDomInfo *dom = pinfo->dom_info + i;
        printf("SCMI Perf opp_count:%d,sustained_freq_khz:%d KHz,sustained_perf_level:%d KHz,mult_factor:%d Hz,name:%s.\n", dom->opp_count,
                                                                                                                            dom->sustained_freq_khz,
                                                                                                                            dom->sustained_perf_level,
                                                                                                                            dom->mult_factor,
                                                                                                                            dom->name);
        for (opp_num = 0; opp_num < dom->opp_count ; opp_num++)
        {
            FScmiDvfsFreqGet(&scmi, i, &freq, TRUE);
            printf("FreqNow domains_id:%d,frep:%dMHz.\r\n",i,freq/1000000);
            FScmiDvfsFreqSet(&scmi, i, FScmiPerfGetOppFreq(&scmi,i,opp_num), TRUE);
            if (ret != FT_SUCCESS)
            {
                return -2;
            }
            else
            {
                fsleep_millisec(2);/*等待频率设置生效,延时量参考opp[opp_num].trans_latency_us*/
                printf("FreqSet domains_id:%d,frep:%dMHz.\r\n",i,FScmiPerfGetOppFreq(&scmi,i,opp_num)/1000000);
                FScmiDvfsFreqGet(&scmi, i, &freq, TRUE);
                if (ret != FT_SUCCESS)
                {
                    return -2;
                }
                else/* 运行性能检测程序 */
                {
                    if (i == 2)/* cpuid == 0 为小核心 domains_id 为 2，两个小核心属于CLUSTER2 */
                    {
                        /*code*/
                    }
                    printf("FreqGet domains_id:%d,frep:%dMHz.\r\n", i , freq/1000000);
                }
            }
            printf("\r\n");
        }
    }
    return ret;
}
