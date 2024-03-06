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
 * FilePath: pcie_ep_rc_example.h
 * Created Date: 2023-08-03 13:28:35
 * Last Modified: 2023-08-08 09:00:19
 * Description:  This file is for ep and rc test configurations
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0     huanghe    2023/08/07    first release
 */
#ifndef  PCIE_ENUMERATE_EXAMPLE_H
#define  PCIE_ENUMERATE_EXAMPLE_H

#include "fparameters.h"
#include "fcompiler.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct FPcieRegInfo
{
    const char *name;
    u8 offset;
};

struct FPcieCapabilityInfo
{
    u8 cid;
    const char *name;
};


#define FPCIE_TEST_STATE_IDLE    0
#define FPCIE_TEST_STATE_PROCESS 1
#define FPCIE_TEST_STATE_END     2
#define FPCIE_TEST_STATE_SYNC     3

/* Define the offsets for different test functions */
#define FPCIE_TEST_EP_FUN1_OFFSET 0x0
#define FPCIE_TEST_EP_FUN2_OFFSET 0x4
#define FPCIE_TEST_EP_FUN3_OFFSET 0x8
#define FPCIE_TEST_EP_FUN4_OFFSET 0xc
#define FPCIE_TEST_EP_FUN5_OFFSET 0x10
#define FPCIE_TEST_EP_FUN6_OFFSET 0x14
#define FPCIE_TEST_EP_FUN7_OFFSET 0x18
#define FPCIE_TEST_EP_FUN8_OFFSET 0x1c
#define FPCIE_TEST_EP_FUN9_OFFSET 0x20
#define FPCIE_TEST_EP_FUN10_OFFSET 0x24
#define FPCIE_TEST_EP_FUN11_OFFSET 0x28
#define FPCIE_TEST_EP_FUN12_OFFSET 0x2c

/* ep configuration */

#define VENDOR_ID 0x1db7
#define DEVICE_ID 0xdcad
#define SUB_CLASS 0x80
#define BASE_CLASS 0x4

#define BAR0_ADDR 0x80000000U
#define BAR0_SIZE 4096 // 4KB



#if defined(FAARCH64_USE)
#define AARCH64_OUTBOUND_RANGE_START 0x1100000000ULL
#define AARCH64_OUTBOUND_RANGE_END   0x1200000000ULL

#define BAR4_ADDR 0x2000000000ULL
#define BAR4_SIZE 64 * 1024  // 64KB


#else
#define AARCH64_OUTBOUND_RANGE_START 0
#define AARCH64_OUTBOUND_RANGE_END   0


#define BAR4_ADDR 0x91000000U
#define BAR4_SIZE 64 * 1024  /* 64KB */

#endif

#define OUTBOUND_RANGE_START 0x60000000U 
#define OUTBOUND_RANGE_END 0x70000000U

#define MAP_EP_ADDR 0x60000000 /* EP side address for mapping */
#define MAP_RC_ADDR 0xc0000000 /* RC side address for mapping */
#define MAP_SIZE (64 * 1024 *1024) /* 64MB */
#define DATA_BUFFER_SIZE 128

#define FUN_NUM 0

#define PCIE_DMA_ENGINE_INDEX 0
#define PCIEC_EP_INDEX FPCIEC_INSTANCE2_INDEX

/* rc configuration */
#define PCIE_ECAM_INSTANCE_ID FPCIE_ECAM_INSTANCE0
#define PCIEC_RC_INSTANCE_ID  FPCIEC_INSTANCE2_INDEX


#ifdef __cplusplus
}
#endif

#endif