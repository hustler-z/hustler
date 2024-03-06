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
 * FilePath: sdio_tf_bench_example.c
 * Date: 2023-04-05 15:22:56
 * LastEditTime: 2023-04-07 13:49:25
 * Description:  This file is for sdio TF card bench example function implmentation.
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

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "fsdif_timing.h"
#include "fsl_sdmmc.h"
#include "sdif_common.h"
#include "sdif_tf_bench_example.h"
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

/************************** Function *****************************************/
/* function of TF card speed bench example */
int FSdifTfCardBenchExample(boolean is_uhs_card, uint32_t card_clock)
{
    int ret = 0;
    status_t err = 0;
    s64 bench_rw = 0;
    u64 bench_blk = SD_START_BLOCK;
    u32 start_time;
    u32 time_cast;
    double trans_time, trans_speed;

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
        FSD_ERROR("Init Tf card failed, err = %d !!!", err);
        goto err_exit;
    }
    
    printf("TF card init success.\n");

    FSdifSetupSystick();

    /* do write bench */
    {
        for (u32 i = 0; i < SD_MAX_RW_BLK; i++) /* copy string to write buffer as each block */
        {
            memset(rw_buf + i * SD_BLOCK_SIZE, ((SD_START_BLOCK + i) & 0xff), SD_BLOCK_SIZE);
        }

        printf("Writing the %s %ld MB data (%ld Blocks), waiting...\r\n", 
                s_inst.card.cid.productName, SD_BENCH_SIZE / SZ_1M, SD_TOTAL_BLOCKS);
        start_time = FSdifGetTick();
        
        while (bench_blk < (SD_START_BLOCK + SD_TOTAL_BLOCKS))
        {
            /* how many blocks to be wrote this time */
            bench_rw = (bench_blk + SD_MAX_RW_BLK) <= (SD_START_BLOCK + SD_TOTAL_BLOCKS) ?
                        SD_MAX_RW_BLK : (SD_START_BLOCK + SD_TOTAL_BLOCKS - bench_blk);

            err = SD_WriteBlocks(&s_inst.card, rw_buf, bench_blk, bench_rw);
            if (kStatus_Success != err)
            {
                FSD_ERROR("TF card write fail.");
                ret = FSD_EXAMPLE_READ_WRITE_FAILED;
                goto err_exit;
            }

            bench_blk += bench_rw;
        }

        time_cast = FSdifGetTick() - start_time;
        trans_time = (double)time_cast / GenericTimerFrequecy();
        trans_speed = (double)SD_BENCH_SIZE / SZ_1M / trans_time;
        printf("Write benchmark success, total time: %d.%03dSec, total size: %ld MB, speed: %f MB/sec\r\n",
                FSdifTickCastSeconds(time_cast),
                FSdifTickCastMilliSec(time_cast),
                SD_BENCH_SIZE / SZ_1M,
                trans_speed);
    }
    
    /* do read bench */
    {
        printf("Reading the %s %ld MB data (%ld Blocks), waiting...\r\n", 
                s_inst.card.cid.productName, SD_BENCH_SIZE / SZ_1M, SD_TOTAL_BLOCKS);
        start_time = FSdifGetTick();

        bench_blk = SD_START_BLOCK;

        while (bench_blk < (SD_START_BLOCK + SD_TOTAL_BLOCKS))
        {
            /* how many blocks to be wrote this time */
            bench_rw = (bench_blk + SD_MAX_RW_BLK) <= (SD_START_BLOCK + SD_TOTAL_BLOCKS) ?
                        SD_MAX_RW_BLK : (SD_START_BLOCK + SD_TOTAL_BLOCKS - bench_blk);

            err = SD_ReadBlocks(&s_inst.card, rw_buf, bench_blk, bench_rw);
            if ((kStatus_Success != err) /*|| 
                (rw_buf[0] != (bench_blk & 0xff)) || 
                (rw_buf[bench_rw * SD_BLOCK_SIZE - 1] != ((bench_blk + bench_rw - 1) & 0xff))*/)
            {
                FSD_ERROR("TF card read fail. 0x%x != 0x%x, 0x%x != 0x%x",
                        rw_buf[0], (bench_blk & 0xff),
                        rw_buf[bench_rw * SD_BLOCK_SIZE - 1], ((bench_blk + bench_rw) & 0xff));
                ret = FSD_EXAMPLE_READ_WRITE_FAILED;
                goto err_exit;
            }

            bench_blk += bench_rw;
        }

        time_cast = FSdifGetTick() - start_time;
        trans_time = (double)time_cast / GenericTimerFrequecy();
        trans_speed = (double)SD_BENCH_SIZE / SZ_1M / trans_time;
        printf("Read benchmark success, total time: %d.%03dSec, total size: %ld MB, speed: %f MB/sec\r\n",
                FSdifTickCastSeconds(time_cast),
                FSdifTickCastMilliSec(time_cast),
                SD_BENCH_SIZE / SZ_1M,
                trans_speed);
    }

err_exit:
    if (kStatus_Success != err)
    {
        FSD_ERROR("TF card read/write failed.");
    }

    SD_Deinit(&s_inst.card);
    FSdifDeinitSystick();
    SDMMC_OSADeInit();
    FSdifTimingDeinit();

    return err;
}

int FSdifSDHCCardBenchExample(void)
{
    return FSdifTfCardBenchExample(FALSE, SD_CLOCK_50MHZ);
}

int FSdifUHSICardBenchExample(void)
{
#if defined(CONFIG_E2000Q_DEMO_BOARD) || defined(CONFIG_E2000D_DEMO_BOARD)
    return -1; /* uhs mode is not supported in e2000 demo board */
#else
    return FSdifTfCardBenchExample(TRUE, SD_CLOCK_100MHZ);
#endif
}