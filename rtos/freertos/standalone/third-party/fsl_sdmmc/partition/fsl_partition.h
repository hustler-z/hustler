/*
 * Copyright (c) 2023 Phytium Information Technology, Inc.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef _FSL_PARTITION_H
#define _FSL_PARTITION_H


#include "fsl_sdmmc.h"
#include "fsl_partition_gpt.h"
#include "fsl_partition_mbr.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    uintptr_t  blk_offset;  /* partition start offset in block */
    size_t     blk_count;   /* partition size in block */
} sdmmc_part;

typedef struct
{
    uint8_t        type;
#define FSL_MBR_BLOCK_INDEX             0U
#define FSL_GPT_HEADER_BLOCK_INDEX      1U
#define FSL_PARTITION_MAX_NUM           4U
    sdmmc_part     parts[FSL_PARTITION_MAX_NUM];
} sdmmc_partition_info;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

status_t SDMMC_LookupPartition(sdmmchost_t *host, sdmmc_partition_info *partInfo);

#if defined(__cplusplus)
}
#endif
/* @} */
#endif /* _FSL_PARTITION_H */