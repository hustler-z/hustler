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
 * FilePath: sdmmc_tf_read_write_example.c
 * Date: 2023-04-05 15:22:56
 * LastEditTime: 2023-04-07 13:49:25
 * Description:  This file is for sdmmc TF card read and write example function implmentation.
 *
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------    --------------------------------------
 *  1.0   liqiaozhong  2023/4/7    first commit with init and RW examples
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

#include "fsl_sdmmc.h"
#include "sdmmc_tf_read_write_example.h"
/************************** Constant Definitions *****************************/
enum
{
    FSD_EXAMPLE_OK = 0,
    FSD_EXAMPLE_NOT_YET_INIT,
    FSD_EXAMPLE_INIT_FAILED,
    FSD_EXAMPLE_INVALID_PARAM,
    FSD_EXAMPLE_READ_WRITE_FAILED,
};

#define SD_MAX_RW_BLK    1024U
#define SD_BLOCK_SIZE    512U

#define SD_START_BLOCK   0 
#define SD_USE_BLOCK     3
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static sdmmchost_config_t s_inst_config;
static sdmmc_sd_t s_inst;
static u8 rw_buf[SD_MAX_RW_BLK * SD_BLOCK_SIZE] __attribute((aligned(SD_BLOCK_SIZE))) = {0};
/***************** Macros (Inline Functions) Definitions *********************/
#define FSD_EXAMPLE_TAG "FSD_EXAMPLE"
#define FSD_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_WARN(format, ...)    FT_DEBUG_PRINT_W(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_INFO(format, ...)    FT_DEBUG_PRINT_I(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
/* function of TF card read and write example */
int FSdmmcTfCardReadWriteExample(void)
{
    int ret = 0;
    int err = 0;

    SDMMC_OSAInit();

    memset(&s_inst_config, 0, sizeof(s_inst_config));
    memset(&s_inst, 0, sizeof(s_inst));

    s_inst_config.hostId = FSDMMC0_ID;
    s_inst_config.hostType = kSDMMCHOST_TYPE_FSDMMC;
    s_inst_config.cardType = kSDMMCHOST_CARD_TYPE_MICRO_SD;
    s_inst_config.enableDMA = TRUE;
    s_inst_config.enableIrq = TRUE;
    s_inst_config.timeTuner = NULL;
    s_inst_config.endianMode = kSDMMCHOST_EndianModeLittle;
    s_inst_config.maxTransSize = SD_MAX_RW_BLK * SD_BLOCK_SIZE;
    s_inst_config.defBlockSize = SD_BLOCK_SIZE;
    s_inst_config.cardClock = SD_CLOCK_50MHZ;
    s_inst_config.isUHSCard = FALSE;

    err = SD_CfgInitialize(&s_inst, &s_inst_config);
    if (kStatus_Success != err)
    {
        FSD_ERROR("Config SD failed, err = %d !!!", err);
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
    if (0 == err)
    {
        printf("%s@%d: TF card read write example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: TF card read write example failed !!!, err = %d \r\n", __func__, __LINE__, err);
    }

    SD_Deinit(&s_inst.card);
    SDMMC_OSADeInit();

    return err;
}