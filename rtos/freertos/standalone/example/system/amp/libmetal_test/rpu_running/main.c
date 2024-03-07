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
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:56:39
 * Description:  This file is for rpu running demo
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 
 */


#include "sys_init.h"
#include "ftypes.h"
#include "../common/sys_init.h"
#include "./inc/rpu_running.h"
#include "fsleep.h"

#include "fdebug.h"
#define LIBMETAL_MAIN_DEBUG_TAG "LIBMETAL_MAIN"
#define LIBMETAL_MAIN_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(LIBMETAL_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_MAIN_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(LIBMETAL_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_MAIN_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(LIBMETAL_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)

int main(void)
{
    struct metal_device *device;
	struct metal_io_region *io;
    int ret = 0;
    ret = FLibmetalSysInit();
    if (ret)
    {
        LIBMETAL_MAIN_DEBUG_E("Failed to initialize system.\n");
        return ret;
    }

    ret = FRegisterMetalDevice();
    if (ret)
    {
        LIBMETAL_MAIN_DEBUG_E("Failed to register metal device. \n");
        return ret;
    }
    
    while (1)
    {
        LIBMETAL_MAIN_DEBUG_I("****************************************************\r\n");
        LIBMETAL_MAIN_DEBUG_I("****RPU waiting APU running waking_up_demo......****\r\n");
        LIBMETAL_MAIN_DEBUG_I("****************************************************\r\n");

        ret = ipi_waking_up_demod();/* Used to synchronize APU and RPU */
        if (ret < 0)
        {
            LIBMETAL_MAIN_DEBUG_E("ipi_waking_up_demod() failed.\n");
            goto out;
        }

        LIBMETAL_MAIN_DEBUG_I("\r\n******shmem_demod() running...\r\n");
        ret = shmem_demod();
        if (ret < 0)
        {
            LIBMETAL_MAIN_DEBUG_E("Shared memory demo failed.\n");
            goto out;
        }
        LIBMETAL_MAIN_DEBUG_I("\r\n      Shmem_demod() over.******\r\n");
        printf("\r\n\r\n");

        ret = ipi_waking_up_demod();/* Used to synchronize APU and RPU */
        if (ret < 0)
        {
            LIBMETAL_MAIN_DEBUG_E("ipi_waking_up_demod() failed.\n");
            goto out;
        }

        LIBMETAL_MAIN_DEBUG_I("\r\n******atomic_shmem_demod() running...\r\n");
        ret = atomic_shmem_demod();
        if (ret < 0)
        {
            LIBMETAL_MAIN_DEBUG_E("Shared memory atomic demo failed.\n");
            goto out;
        }
        LIBMETAL_MAIN_DEBUG_I("\r\n      atomic_shmem_demod() over.******\r\n");
        printf("\r\n\r\n");

        ret = ipi_waking_up_demod();/* Used to synchronize APU and RPU */
        if (ret < 0)
        {
            LIBMETAL_MAIN_DEBUG_E("ipi_waking_up_demod() failed.\n");
            goto out;
        }

        LIBMETAL_MAIN_DEBUG_I("\r\n******ipi_shmem_demod() running...\r\n");
        ret = ipi_shmem_demod();
        if (ret){
            LIBMETAL_MAIN_DEBUG_E("Shared memory ipi atomic demo failed.\n");
            goto out;
        }
        LIBMETAL_MAIN_DEBUG_I("\r\n      ipi_shmem_demod() over.******\r\n");
        printf("\r\n\r\n");

        ret = ipi_waking_up_demod();/* Used to synchronize APU and RPU */
        if (ret < 0)
        {
            LIBMETAL_MAIN_DEBUG_E("ipi_waking_up_demod() failed.\n");
            goto out;
        }

        LIBMETAL_MAIN_DEBUG_I("\r\n******ipi_latency_demod() running...\r\n");
        ret = ipi_latency_demod();
        if (ret){
            LIBMETAL_MAIN_DEBUG_E("IPI latency demo failed.\n");
            goto out;
        }
        LIBMETAL_MAIN_DEBUG_I("\r\n      ipi_latency_demod() over.******\r\n");
        printf("\r\n\r\n");

        ret = ipi_waking_up_demod();/* Used to synchronize APU and RPU */
        if (ret < 0)
        {
            LIBMETAL_MAIN_DEBUG_E("ipi_waking_up_demod() failed.\n");
            goto out;
        }

        LIBMETAL_MAIN_DEBUG_I("\r\n******shmem_latency_demod() running...\r\n");
        ret = shmem_latency_demod();
        if (ret){
            LIBMETAL_MAIN_DEBUG_E("Shared memory latency demo failed ret:%d.\n",ret);
            goto out;
        }
        LIBMETAL_MAIN_DEBUG_I("\r\n      shmem_latency_demod() over.******\r\n");
        printf("\r\n\r\n");

        ret = ipi_waking_up_demod();/* Used to synchronize APU and RPU */
        if (ret < 0)
        {
            LIBMETAL_MAIN_DEBUG_E("ipi_waking_up_demod() failed.\n");
            goto out;
        }
        
        LIBMETAL_MAIN_DEBUG_I("\r\n******shmem_throughput_demod() running...\r\n");
        ret = shmem_throughput_demod();
        if (ret){
            LIBMETAL_MAIN_DEBUG_E("Shared memory thoughput demo failed.\n");
            goto out;
        }
        fsleep_millisec(50);/*To printf() over*/
        LIBMETAL_MAIN_DEBUG_I("\r\n      shmem_throughput_demod() over...******\r\n");
        LIBMETAL_MAIN_DEBUG_I("\r\n");
    }
out:
    FLibmetalSysCleanup();
    return 0;
}