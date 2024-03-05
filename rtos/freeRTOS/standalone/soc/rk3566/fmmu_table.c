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
 * FilePath: fmmu_table.c
 * Date: 2023-11-6 10:33:28
 * LastEditTime: 2023-11-6 10:33:28
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan    2023/11/6       init commit
 */
#include "fmmu.h"
#include "fparameters.h"

#ifdef __aarch64__

const struct ArmMmuRegion mmu_regions[] =
{
    MMU_REGION_FLAT_ENTRY("DEVICE_REGION",
                          0x00, 0x40000000,
                          MT_DEVICE_NGNRE | MT_RW | MT_NS),

    MMU_REGION_FLAT_ENTRY("PCIE_CONFIG_REGION",
                          0x40000000, 0x10000000,
                          MT_DEVICE_NGNRNE | MT_RW | MT_NS),

    MMU_REGION_FLAT_ENTRY("PCIE_REGION",
                          0x50000000, 0x30000000,
                          MT_DEVICE_NGNRE | MT_RW | MT_NS),

    MMU_REGION_FLAT_ENTRY("DDR_2G_REGION",
                          0x80000000, 0x80000000,
                          MT_NORMAL | MT_RW | MT_NS),

    MMU_REGION_FLAT_ENTRY("PCIE_REGION",
                          0x1000000000, 0x1000000000,
                          MT_DEVICE_NGNRE | MT_RW | MT_NS),

    MMU_REGION_FLAT_ENTRY("DDR_EXTEND_REGION",
                          0x2000000000, 0x2000000000,
                          MT_NORMAL | MT_RW | MT_NS),
};

const uint32_t mmu_regions_size = ARRAY_SIZE(mmu_regions);

const struct ArmMmuConfig mmu_config =
{
    .num_regions = mmu_regions_size,
    .mmu_regions = mmu_regions,
};

#else

#define DDR_MEM NORMAL_MEM

struct mem_desc platform_mem_desc[] =
{
    {
        0x80000000,
        0xFFFFFFFF,
        0x80000000,
        DDR_MEM
    },
    {
        0, //< QSPI
        0x1FFFFFFF,
        0,
        DEVICE_MEM
    },
    {
        0x20000000, //<! LPC
        0x27FFFFFF,
        0x20000000,
        DEVICE_MEM
    },
    {
        FDEV_BASE_ADDR, //<! Device register
        FDEV_END_ADDR,
        FDEV_BASE_ADDR,
        DEVICE_MEM
    },
    {
        0x30000000, //<! debug
        0x39FFFFFF,
        0x30000000,
        DEVICE_MEM
    },
    {
        0x3A000000, //<! Internal register space in the on-chip network
        0x3AFFFFFF,
        0x3A000000,
        DEVICE_MEM
    },
    {
        0x40000000,
        0x7FFFFFFF,
        0x40000000,
        DEVICE_MEM
    },
};

const u32 platform_mem_desc_size = sizeof(platform_mem_desc) / sizeof(platform_mem_desc[0]);

#endif
