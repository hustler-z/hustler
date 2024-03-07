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
 * FilePath: fddma_os.h
 * Date: 2022-07-20 09:15:37
 * LastEditTime: 2022-07-20 09:15:38
 * Description:  This files is for providing function related definitions of DDMA driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver    Who          Date         Changes
 * -----  ------       --------     --------------------------------------
 *  1.0   zhugengyu    2022/7/27    init commit
 *  1.1   liqiaozhong  2023/11/10   synchronous update with standalone sdk
 */

#ifndef  FDDMA_OS_H
#define  FDDMA_OS_H
/***************************** Include Files *********************************/
#include <FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>

#include "fparameters.h"
#include "fddma.h"
#include "fddma_hw.h"
/************************** Constant Definitions *****************************/
#ifdef __cplusplus
extern "C"
{
#endif

#define FFREERTOS_DDMA_OK                   FT_SUCCESS
#define FFREERTOS_DDMA_NOT_INIT             FT_CODE_ERR(ErrModPort, ErrDdma, 0)
#define FFREERTOS_DDMA_SEMA_ERR             FT_CODE_ERR(ErrModPort, ErrDdma, 1)
#define FFREERTOS_DDMA_ALREADY_INIT         FT_CODE_ERR(ErrModPort, ErrDdma, 2)
#define FFREERTOS_DDMA_EVT_TIMEOUT          FT_CODE_ERR(ErrModPort, ErrDdma, 3)


#define FFREERTOS_DDMA_IRQ_PRIORITY         IRQ_PRIORITY_VALUE_12
/**************************** Type Definitions *******************************/
typedef struct
{

} FFreeRTOSDdmaConfig; /* freertos DDMA config, reserved for future use */

typedef struct
{
    FDdma ctrl;
    FFreeRTOSDdmaConfig config;
    SemaphoreHandle_t locker;
} FFreeRTOSDdma; /* freertos DDMA instance */

typedef struct
{
    u32 slave_id; /* slave id of periperal */
    uintptr mem_addr; /* memory address of transfer */
    uintptr dev_addr; /* periperal device address of transfer */
    uintptr trans_len; /* total bytes of transfer */
    boolean is_rx; /* TRUE: dev ==> mem, FALSE: mem ==> dev */
    FDdmaChanEvtHandler req_done_handler; /* callback when request done */
    void *req_done_args;
} FFreeRTOSRequest; /* freertos DDMA transfer request */
/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* init and get DDMA instance */
FFreeRTOSDdma *FFreeRTOSDdmaInit(u32 instance_id, const FFreeRTOSDdmaConfig *config);

/* deinit DDMA instance */
FError FFreeRTOSDdmaDeinit(FFreeRTOSDdma *const instance);

/* setup DDMA channel before transfer */
FError FFreeRTOSDdmaSetupChannel(FFreeRTOSDdma *const instance, u32 chan_id, const FFreeRTOSRequest *request);

/* revoke channel setup */
FError FFreeRTOSDdmaRevokeChannel(FFreeRTOSDdma *const instance, u32 chan_id);

/* start DDMA transfer of channel  */
FError FFreeRTOSDdmaStartChannel(FFreeRTOSDdma *const instance, u32 chan_id);

/* stop DDMA transfer of channel */
FError FFreeRTOSDdmaStopChannel(FFreeRTOSDdma *const instance, u32 chan_id);

/* stop all DDMA channel */
FError FFreeRTOSDdmaStop(FFreeRTOSDdma *const instance);

#ifdef __cplusplus
}
#endif

#endif