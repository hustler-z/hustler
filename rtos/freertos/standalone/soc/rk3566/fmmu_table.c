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

/* memory[0]	[0x200000-0x3fffffff], 0x3fe00000 bytes flags: 0
 * reserved.cnt = 0x2 / max = 0x10
 * reserved[0]	[0x3cebc000-0x3fffffff], 0x03144000 bytes flags: 0
 * reserved[1]	[0x3debc070-0x3fffffff], 0x02143f90 bytes flags: 0
 *
 * When modify the mapping range, ensure that mmu region size is
 * page aligned - Hustler did.
 */
const struct ArmMmuRegion mmu_regions[] =
{
    MMU_REGION_FLAT_ENTRY("DDR_2G_REGION",
                          0x00200000, 0x80000000,
                          MT_NORMAL | MT_RW | MT_NS),

    MMU_REGION_FLAT_ENTRY("DEVICE_REGION",
                          0xf0000000, 0x40000000,
                          MT_DEVICE_NGNRE | MT_RW | MT_NS),
};

const struct ArmMmuConfig mmu_config =
{
    .num_regions = ARRAY_SIZE(mmu_regions),
    .mmu_regions = mmu_regions,
};

#else

#define DDR_MEM NORMAL_MEM
/* TO-DO */
#endif
