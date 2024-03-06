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
 * FilePath: sdif_tf_read_write_example.c
 * Date: 2023-04-05 15:22:56
 * LastEditTime: 2023-04-07 13:49:25
 * Description:  This file is for sdio TF card read and write example function implmentation.
 *
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------    --------------------------------------
 *  1.0   liqiaozhong  2023/4/7    first commit with init and RW examples
 *  1.1   liqiaozhong  2023/8/11   update according to phytiumpi
 *  2.0   zhugengyu    2023/10/10  replace sdmmc component as fsl_sdmmc
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "ftypes.h"
#include "fdebug.h"
#include "fassert.h"
#include "fio.h"
#include "fparameters.h"
#include "fkernel.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "fsdif_timing.h"
#include "fsl_sdmmc.h"
#include "sdif_common.h"
#include "sdif_tf_read_write_example.h"
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
/***************** Macros (Inline Functions) Definitions *********************/
#define FSD_EXAMPLE_TAG "FSD_EXAMPLE"
#define FSD_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_WARN(format, ...)    FT_DEBUG_PRINT_W(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_INFO(format, ...)    FT_DEBUG_PRINT_I(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/
#ifdef CONFIG_TARGET_PHYTIUMPI
#error "Do not use this example in PhytiumPI, firmware in SD card might be damaged !!!"
#endif
/************************** Function *****************************************/
/* function of TF card read and write example */
int FSdifTfCardReadWriteExample(boolean is_uhs_card, uint32_t card_clock)
{
    int ret = 0;
    status_t err = 0;

    FSdifTimingInit();
    SDMMC_OSAInit();

    memset(&s_inst_config, 0, sizeof(s_inst_config));
    memset(&s_inst, 0, sizeof(s_inst));

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
        FSD_ERROR("Init SD failed, err = %d !!!", err);
        goto err_exit;
    }
    
    /* do read and write operation */
    {
        printf("TF card init success.\n");

        for (u32 i = 0; i < SD_USE_BLOCK; i++) /* copy string to write buffer as each block */
        {
            memset(rw_buf + i * SD_BLOCK_SIZE, (SD_START_BLOCK + i + 1), SD_BLOCK_SIZE);
        }

        err = SD_WriteBlocks(&s_inst.card, rw_buf, SD_START_BLOCK, SD_USE_BLOCK);
        if (kStatus_Success != err)
        {
            FSD_ERROR("TF card write fail.");
            ret = FSD_EXAMPLE_READ_WRITE_FAILED;
            goto err_exit;
        }

        memset(rw_buf, 0, sizeof(rw_buf));

        err = SD_ReadBlocks(&s_inst.card, rw_buf, SD_START_BLOCK, SD_USE_BLOCK);
        if ((kStatus_Success == err))
        {
            FtDumpHexByte(rw_buf, SD_BLOCK_SIZE * min((u32)2, (u32)SD_USE_BLOCK));
            printf("%s@%d: TF card read and write example success !!! \r\n", __func__, __LINE__);
        }
        else
        {
            FSD_ERROR("TF card read fail.");
            ret = FSD_EXAMPLE_READ_WRITE_FAILED;
            goto err_exit;
        }
    }

err_exit:
    if (kStatus_Success != err)
    {
        FSD_ERROR("TF card read/write failed.");
        printf("%s@%d: TF card read and write example failed !!!, err = %d \r\n", __func__, __LINE__, err);
    }

    SD_Deinit(&s_inst.card);
    SDMMC_OSADeInit();
    FSdifTimingDeinit();

    return err;
}

int FSdifSDHCCardReadWriteExample(void)
{
    return FSdifTfCardReadWriteExample(FALSE, SD_CLOCK_50MHZ);
}

int FSdifUHSICardReadWriteExample(void)
{
#if defined(CONFIG_E2000Q_DEMO_BOARD) || defined(CONFIG_E2000D_DEMO_BOARD)
    return -1; /* uhs mode is not supported in e2000 demo board */
#else
    return FSdifTfCardReadWriteExample(TRUE, SD_CLOCK_100MHZ);
#endif
}