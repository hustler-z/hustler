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

#ifdef __aarch64__

const struct ArmMmuRegion mmu_regions[] =
{
    MMU_REGION_FLAT_ENTRY("DEVICE_REGION",
                          0x00, 0x40000000,
                          MT_DEVICE_NGNRE | MT_RW | MT_NS),

    MMU_REGION_FLAT_ENTRY("PCIE_CONFIG_REGION",
                          0x40000000, 0x10000000,
                          MT_DEVICE_NGNRNE | MT_RW | MT_NS),

    MMU_REGION_FLAT_ENTRY("PCIE_IO_REGION",
                          0x50000000, 0x08000000,
                          MT_DEVICE_NGNRE | MT_RW | MT_NS),


    MMU_REGION_FLAT_ENTRY("PCIE_REGION",
                          0x58000000, 0x28000000,
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

const struct ArmMmuConfig mmu_config =
{
    .num_regions = ARRAY_SIZE(mmu_regions),
    .mmu_regions = mmu_regions,
};

#else

#define DDR_MEM NORMAL_MEM

struct mem_desc platform_mem_desc[] =
{
    {
        0x00U,
        0x00U + 0x40000000U,
        0x00U,
        DEVICE_MEM
    },
    {
        0x40000000U,
        0x40000000U + 0x10000000U,
        0x40000000U,
        DEVICE_MEM
    },
    {
        0x50000000U,
        0x50000000U + 0x08000000,
        0x50000000U,
        DEVICE_MEM
    },

    {
        0x58000000U,
        0x58000000U + 0x28000000,
        0x58000000U,
        DEVICE_MEM
    },
    {
        0x80000000U,
        0xffffffffU,
        0x80000000U,
        DDR_MEM
    },
};

const u32 platform_mem_desc_size = sizeof(platform_mem_desc) / sizeof(platform_mem_desc[0]);

#endif