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
 * FilePath: fgdma_os.h
 * Date: 2022-07-20 10:54:37
 * LastEditTime: 2022-07-20 10:54:37
 * Description:  This file is for providing function related definitions of gdma driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0    zhugengyu     2022/7/27     init commit
 *  2.0    liqiaozhong   2023/11/10    synchronous update with standalone sdk
 */
#ifndef  FGDMA_OS_H
#define  FGDMA_OS_H
/***************************** Include Files *********************************/
#include <FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>

#include "fparameters.h"
#include "fgdma.h"
#include "fmemory_pool.h"
/************************** Constant Definitions *****************************/
#ifdef __cplusplus
extern "C"
{
#endif

#define FFREERTOS_GDMA_OK                   FT_SUCCESS
#define FFREERTOS_GDMA_COMMON_ERR           FT_CODE_ERR(ErrModPort, ErrGdma, 0)
#define FFREERTOS_GDMA_NOT_INIT             FT_CODE_ERR(ErrModPort, ErrGdma, 1)
#define FFREERTOS_GDMA_SEMA_ERR             FT_CODE_ERR(ErrModPort, ErrGdma, 2)
#define FFREERTOS_GDMA_ALREADY_INIT         FT_CODE_ERR(ErrModPort, ErrGdma, 3)
#define FFREERTOS_GDMA_ALLOCATE_FAIL        FT_CODE_ERR(ErrModPort, ErrGdma, 4)
#define FFREERTOS_GDMA_MEMCPY_FAIL          FT_CODE_ERR(ErrModPort, ErrGdma, 5)

#define FFREERTOS_GDMA_IRQ_PRIORITY         IRQ_PRIORITY_VALUE_12
#define FFREERTOS_GDMA_NUM_OF_CHAN          FGDMA_NUM_OF_CHAN
/**************************** Type Definitions *******************************/
typedef enum
{
    FFREERTOS_GDMA_CHAN_EVT_BDL_END,   /* one BDL descriptor transfer end */
    FFREERTOS_GDMA_CHAN_EVT_TRANS_END, /* channel transfer end */
    FFREERTOS_GDMA_CHAN_EVT_BUSY,      /* channel is still transferring */

    FFREERTOS_GDMA_CHAN_NUM_OF_EVT
} FFreeRTOSGdmaChanEvtType; /* FreeRTOS GDMA channel event */

typedef enum
{
    FFREERTOS_GDMA_OPER_DIRECT = 0, /* direct mode */
    FFREERTOS_GDMA_OPER_BDL         /* BDL mode */
} FFreeRTOSGdmaChanMode; /* FreeRTOS GDMA channel mode */

typedef void (*FreeRTOSGdmaChanEvtHandler)(uint32_t channel_id, void *arg);

typedef struct
{
    FGdma                      ctrl;
    SemaphoreHandle_t          locker;
    FMemp                      memp;                                                                   /* instance of memory pool */
    uint8_t                    memp_buf[SZ_16K];                                                       /* buffer used to support dynamic memory */
    FreeRTOSGdmaChanEvtHandler os_evt_handler[FGDMA_NUM_OF_CHAN][FFREERTOS_GDMA_CHAN_NUM_OF_EVT];
    void                       *os_evt_handler_arg[FGDMA_NUM_OF_CHAN][FFREERTOS_GDMA_CHAN_NUM_OF_EVT];
} FFreeRTOSGdma; /* GDMA instance in FreeRTOS */

typedef struct
{
    FFreeRTOSGdmaChanMode trans_mode;  /* GDMA channel mode: direct or BDL */
    uintptr_t               src_addr;  /* GDMA transfer source address - physical address */
    uintptr_t               dst_addr;  /* GDMA transfer destination address - physical address */
    size_t               trans_length; /* GDMA transfer data length */
} FFreeRTOSGdmaChanCfg; /* GDMA channel config in FreeRTOS */
/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/* GDMA ctrl(controller) init function */
FFreeRTOSGdma *FFreeRTOSGdmaInit(uint32_t instance_id);

/* GDMA instance deinit function */
FError FFreeRTOSGdmaDeInit(FFreeRTOSGdma *const instance_p);

/* GDMA channenl configure function */
FError FFreeRTOSGdmaChanConfigure(FFreeRTOSGdma *const instance_p, 
                                  uint32_t channel_id, 
                                  FFreeRTOSGdmaChanCfg const *os_channel_config_p);

/* GDMA channel transfer start function */
void FFreeRTOSGdmaChanStart(FFreeRTOSGdma *const instance_p, uint32_t channel_id);

/* GDMA channel stop function */
FError FFreeRTOSGdmaChanStop(FFreeRTOSGdma *const instance_p, uint32_t channel_id);

/* GDMA channel deconfigure function */
FError FFreeRTOSGdmaChanDeconfigure(FFreeRTOSGdma *const instance_p, uint32_t channel_id);

void FFreeRTOSGdmaChanRegisterEvtHandler(FFreeRTOSGdma *const instance_p,
                                         uint32_t channel_id,
                                         FFreeRTOSGdmaChanEvtType evt,
                                         FreeRTOSGdmaChanEvtHandler os_evt_handler, 
                                         void *os_evt_handler_arg);

#ifdef __cplusplus
}
#endif

#endif