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
 * FilePath: cmd_apu_running.c
 * Date: 2023-03-06 14:40:46
 * LastEditTime: 2023-03-06 14:40:46
 * Description:  This file is for APU command of libmetal
 * 
 * Modify History: 
 *  Ver   Who           Date         Changes
 * ----- ------         --------    --------------------------------------
 * 1.0  liushengming    2023/03/06  init
 */

#include "sys_init.h"
#include "strto.h"
#include "ftypes.h"
#include "fpsci.h"
#include "shell.h"
#include "fdebug.h"
#include "fsleep.h"
#include "fprintk.h"
#include "../sdkconfig.h"
#include "../common/sys_init.h"
#include "../inc/apu_running.h"

#define LIBMETAL_MAIN_DEBUG_TAG "LIBMETAL_MAIN"
#define LIBMETAL_MAIN_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(LIBMETAL_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_MAIN_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(LIBMETAL_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_MAIN_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(LIBMETAL_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)

static int flag = 0;
static u32 core_data = 0x0;
static u32 core_mask = 0x0;

static void FLibmetalCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("    libmetalapu [core_mask]\r\n");
    printf("        -- Running libmetal test.\r\n");
    printf("        -- [core_mask],optional parameters,default by Kconfig.\r\n");
    printf("                       Please read the CPU manual carefully before configuration.\r\n");
    printf("        -- [core0] = 0x0001.\r\n");
    printf("        -- [core1] = 0x0002.\r\n");
    printf("        -- [core2] = 0x0004.\r\n");
    printf("        -- [core3] = 0x0008.\r\n");
    printf("        -- [core4] = 0x0010.\r\n");
    printf("        -- [core5] = 0x0020.\r\n");
    printf("        -- [core6] = 0x0040.\r\n");
    printf("        -- [core7] = 0x0080.\r\n");
    printf("        A single running supports starting one core,and one of them is the main core.\r\n");
}

int FLibmetalExample(void)
{
    int ret ;

    ret = FLibmetalSysInit() ;
    if (ret)
    {
        LIBMETAL_MAIN_DEBUG_E("Failed to initialize system. \n");
        return ret;
    }

    ret = FRegisterMetalDevice();
    if (ret)
    {
        LIBMETAL_MAIN_DEBUG_E("Failed to register metal device. \n");
        return ret;
    }

    core_mask = 1 << CONFIG_TARGET_CPU_ID;

    ret = ipi_waking_up_demo(core_mask);/* Used to synchronize APU and RPU */
    if (ret < 0)
    {
        LIBMETAL_MAIN_DEBUG_E("ipi_waking_up_demod() failed.\n");
        goto out;
    }

    ret = shmem_demo();
    if (ret < 0)
    {
        LIBMETAL_MAIN_DEBUG_E("shmem_demo() failed.\n");
        goto out;
    }

    ret = ipi_waking_up_demo(core_mask);/* Used to synchronize APU and RPU */
    if (ret < 0)
    {
        LIBMETAL_MAIN_DEBUG_E("ipi_waking_up_demod() failed.\n");
        goto out;
    }

    ret = atomic_shmem_demo(core_mask);
    if (ret < 0)
    {
        LIBMETAL_MAIN_DEBUG_E("atomic_shmem_demo() failed.\n");
        goto out;
    }

    ret = ipi_waking_up_demo(core_mask);/* Used to synchronize APU and RPU */
    if (ret < 0)
    {
        LIBMETAL_MAIN_DEBUG_E("ipi_waking_up_demod() failed.\n");
        goto out;
    }

    ret = ipi_shmem_demo(core_mask);
    if (ret < 0)
    {
        LIBMETAL_MAIN_DEBUG_E("ipi_shmem_demo() failed.\n");
        goto out;
    }

    ret = ipi_waking_up_demo(core_mask);/* Used to synchronize APU and RPU */
    if (ret < 0)
    {
        LIBMETAL_MAIN_DEBUG_E("ipi_waking_up_demod() failed.\n");
        goto out;
    }

    ret = ipi_latency_demo(core_mask);
    if (ret < 0)
    {
        LIBMETAL_MAIN_DEBUG_E("ipi_latency_demo() failed.\n");
        goto out;
    }

    ret = ipi_waking_up_demo(core_mask);/* Used to synchronize APU and RPU */
    if (ret < 0)
    {
        LIBMETAL_MAIN_DEBUG_E("ipi_waking_up_demod() failed.\n");
        goto out;
    }

    ret = shmem_latency_demo(core_mask);
    if (ret < 0)
    {
        LIBMETAL_MAIN_DEBUG_E("shmem_latency_demo() failed.\n");
        goto out;
    }

    ret = ipi_waking_up_demo(core_mask);/* Used to synchronize APU and RPU */
    if (ret < 0)
    {
        LIBMETAL_MAIN_DEBUG_E("ipi_waking_up_demod() failed.\n");
        goto out;
    }

    ret = shmem_throughput_demo(core_mask);
    if (ret < 0)
    {
        LIBMETAL_MAIN_DEBUG_E("shmem_throughput_demo() failed.\n");
        goto out;
    }
out:
    FLibmetalSysCleanup();
    return ret;
}

static int FLibmetalApu(int argc, char *argv[])
{
    int ret ;

    if (argc > 1)
    {
        core_data = (u32)simple_strtoul(argv[1], NULL, 16);
    }

    if (argc > 2)
    {
        FLibmetalCmdUsage();
        return -1;
    }

    ret = FLibmetalExample();
    
    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), libmetalapu, FLibmetalApu,test libmetalapu);
