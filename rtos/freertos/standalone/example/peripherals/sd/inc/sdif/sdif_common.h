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
 * FilePath: sdio_tf_detect_example.h
 * Date: 2023-04-05 15:22:56
 * LastEditTime: 2023-04-07 13:49:25
 * Description:  This file is for sdio TF card detect example function definition.
 *
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------    --------------------------------------
 *  1.0   liqiaozhong  2023/4/7    first commit with init and RW examples
 */

#ifndef  SDIF_TF_DETECT_EXAMPLE_H
#define  SDIF_TF_DETECT_EXAMPLE_H

/***************************** Include Files *********************************/
#include "ftypes.h"
#include "fkernel.h"
#include "sdkconfig.h"
#include "fparameters.h"
#include "fgeneric_timer.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
#define SYS_TICKRATE_HZ        100
#define SD_WORK_IRQ            TRUE /* 0 for POLL mode, 1 for IRQ mode */
#define SD_WORK_DMA            TRUE /* 0 for PIO mode, 1 for DMA mode */
#ifdef CONFIG_TARGET_PHYTIUMPI
#define SD_CONTROLLER_ID       FSDIO0_ID
#else
#define SD_CONTROLLER_ID       FSDIO1_ID
#endif

#if SD_WORK_DMA
#define SD_MAX_RW_BLK          1024U
#else
#define SD_MAX_RW_BLK          4U
#endif

#define SD_BENCH_SIZE          (u64)SZ_1M * 200ULL          
#define SD_BLOCK_SIZE          512UL /* most tested sd memory cards have 512 block size by default */
#define SDIO_BLOCK_SIZE        256UL /* most tested sdio cards have 256 block size by default */
#define SD_START_BLOCK         0 /* assing start block index \
                                    at least start from 500M address to protect firmware */
#define SD_USE_BLOCK           3
#define SD_TOTAL_BLOCKS        (SD_BENCH_SIZE / SD_BLOCK_SIZE)

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
/* share buffer in different examples because they are space-consuming */
extern uint8_t s_dma_buffer[SZ_1M * 4];
extern u8 rw_buf[SD_MAX_RW_BLK * SD_BLOCK_SIZE];
/***************** Macros (Inline Functions) Definitions *********************/
static inline void FSdifSetupSystick(void)
{    
    GenericTimerStop(GENERIC_TIMER_ID0);
	GenericTimerSetTimerValue(GENERIC_TIMER_ID0, GenericTimerFrequecy() / SYS_TICKRATE_HZ);
	GenericTimerStart(GENERIC_TIMER_ID0);
}

static inline void FSdifDeinitSystick(void)
{
    GenericTimerStop(GENERIC_TIMER_ID0);
}

static inline u32 FSdifGetTick(void)
{
    return GenericTimerRead(GENERIC_TIMER_ID0);
}

static inline u32 FSdifTickCastSeconds(u32 tick)
{
    return (u32)(tick / (u32)GenericTimerFrequecy());
}

static inline u32 FSdifTickCastMilliSec(u32 tick)
{
    return (u32)(tick % (u32) GenericTimerFrequecy() / (((u32)GenericTimerFrequecy() * 1 + 999) / 1000));
}

/************************** Function Prototypes ******************************/


#ifdef __cplusplus
}
#endif

#endif
