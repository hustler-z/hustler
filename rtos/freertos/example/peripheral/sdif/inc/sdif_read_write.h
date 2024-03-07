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
 * FilePath: sd_read_write.h
 * Date: 2022-07-18 16:43:35
 * LastEditTime: 2022-07-18 16:43:35
 * Description:  This file is for providing some sd apis.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    first commit
 */
#ifndef  SD_READ_WRITE_H
#define  SD_READ_WRITE_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#include "fkernel.h"
/************************** Constant Definitions *****************************/
#define SD_WORK_IRQ            TRUE /* 0 for POLL mode, 1 for IRQ mode */
#define SD_WORK_DMA            TRUE /* 0 for PIO mode, 1 for DMA mode */
#ifdef CONFIG_TARGET_PHYTIUMPI
#define SD_CONTROLLER_ID       FSDIO0_ID
#else
#define SD_CONTROLLER_ID       FSDIO1_ID
#endif

#if SD_WORK_DMA
#define SD_MAX_RW_BLK          10U
#else
#define SD_MAX_RW_BLK          4U
#endif

#define SD_BLOCK_SIZE          512UL
#define SD_START_BLOCK         0 
#define SD_USE_BLOCK           3
/************************** Variable Definitions *****************************/
extern u8 s_dma_buffer[SZ_1M * 4];
extern u8 rw_buf[SD_MAX_RW_BLK * SD_BLOCK_SIZE];
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
BaseType_t FFreeRTOSTfWriteRead(void);
BaseType_t FFreeRTOSeMMCWriteRead(void);

#ifdef __cplusplus
}
#endif

#endif