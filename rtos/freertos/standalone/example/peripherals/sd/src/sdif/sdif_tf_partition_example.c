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
 * FilePath: sdif_tf_partition_example.c
 * Date: 2023-04-05 15:22:56
 * LastEditTime: 2023-04-07 13:49:25
 * Description:  This file is for sdio TF card detect partition example function implmentation.
 *
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------    --------------------------------------
 *  1.0   zhugengyu   2023/10/10    first commit
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "ftypes.h"
#include "fdebug.h"
#include "fassert.h"
#include "fio.h"
#include "fkernel.h"
#include "fparameters.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "fsdif_timing.h"
#include "fsl_sdmmc.h"

#include "fsl_partition.h"
#include "sdif_common.h"
#include "sdif_tf_detect_example.h"
/************************** Constant Definitions *****************************/
enum
{
    FSD_EXAMPLE_OK = 0,
    FSD_EXAMPLE_NOT_YET_INIT,
    FSD_EXAMPLE_INIT_FAILED,
    FSD_EXAMPLE_INVALID_PARAM,
    FSD_EXAMPLE_READ_WRITE_FAILED,
};       
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static sdmmchost_config_t s_inst_config;
static sdmmc_sd_t s_inst;
static sdmmc_partition_info s_part;
/***************** Macros (Inline Functions) Definitions *********************/
#define FSD_EXAMPLE_TAG "FSD_EXAMPLE"
#define FSD_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_WARN(format, ...)    FT_DEBUG_PRINT_W(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_INFO(format, ...)    FT_DEBUG_PRINT_I(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
/* function of TF card detect example */
int FSdifTFCardPartitionExample(boolean is_uhs_card, uint32_t card_clock)
{
    int ret = 0;
    int part_type = 0;
    status_t err = 0;

    FSdifTimingInit();
    SDMMC_OSAInit();

    memset(&s_inst_config, 0, sizeof(s_inst_config));
    memset(&s_inst, 0, sizeof(s_inst));
    memset(&s_part, 0, sizeof(s_part));

    s_inst_config.hostId = FSDIF1_ID;
    s_inst_config.hostType = kSDMMCHOST_TYPE_FSDIF;
    s_inst_config.cardType = kSDMMCHOST_CARD_TYPE_MICRO_SD;
    s_inst_config.enableDMA = SD_WORK_DMA;
    s_inst_config.enableIrq = SD_WORK_IRQ;
    s_inst_config.timeTuner = FSdifGetTimingSetting;
    s_inst_config.endianMode = kSDMMCHOST_EndianModeLittle;
    s_inst_config.maxTransSize = SD_MAX_RW_BLK * SD_BLOCK_SIZE;
    s_inst_config.defBlockSize = SD_BLOCK_SIZE;
    s_inst_config.cardClock = card_clock;
    s_inst_config.isUHSCard = is_uhs_card;

    err = SD_CfgInitialize(&s_inst, &s_inst_config);
    if (kStatus_Success != err)
    {
        FSD_ERROR("Init TF card failed, err = %d !!!", err);
        goto err_exit;
    }

    err = SDMMC_LookupPartition(&s_inst.host, &s_part);
    if (kStatus_Success != err)
    {
        FSD_ERROR("Get partition info failed, err = %d !!!", err);
        goto err_exit;        
    }

    for (u32 i = 0; i < FSL_PARTITION_MAX_NUM; i++)
    {
        printf("partition-%d block-%d +%d \r\n", i, s_part.parts[i].blk_offset, s_part.parts[i].blk_count);  
    }

err_exit:
    if (0 == err)
    {
        printf("%s@%d: TF card partiton detect example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: TF card partiton detect example failed !!!, err = %d \r\n", __func__, __LINE__, err);
    }

    SD_Deinit(&s_inst.card);
    SDMMC_OSADeInit();
    FSdifTimingDeinit();

    return err;
}

int FSdifSDHCCardDetectPartitionExample(void)
{
    return FSdifTFCardPartitionExample(FALSE, SD_CLOCK_50MHZ);
}

int FSdifUHSICardDetectPartitionExample(void)
{
#if defined(CONFIG_E2000Q_DEMO_BOARD) || defined(CONFIG_E2000D_DEMO_BOARD)
    return -1; /* uhs mode is not supported in e2000 demo board */
#else
    return FSdifTFCardPartitionExample(TRUE, SD_CLOCK_100MHZ);
#endif
}