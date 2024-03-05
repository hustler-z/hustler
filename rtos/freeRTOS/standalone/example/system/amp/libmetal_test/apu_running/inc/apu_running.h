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
 * FilePath: apu_running.h
 * Date: 2023-03-06 15:04:16
 * LastEditTime: 2023-03-06 15:04:16
 * Description:  This file is for define apu_running function
 * 
 * Modify History: 
 *  Ver   Who           Date         Changes
 * ----- ------         --------    --------------------------------------
 * 1.0  liushengming    2023/03/06  init
 */

#ifndef APU_RUNNING_H
#define APU_RUNNING_H

#ifdef __cplusplus
extern "C"
{
#endif

int shmem_demo() ;
int atomic_shmem_demo(u32 core_mask) ;
int ipi_shmem_demo(u32 core_mask) ;
int ipi_latency_demo(u32 core_mask) ;
int shmem_latency_demo(u32 core_mask) ;
int shmem_throughput_demo(u32 core_mask) ;
int ipi_waking_up_demo(u32 core_mask);

#ifdef __cplusplus
}
#endif

#endif