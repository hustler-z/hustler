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
 * FilePath: fddma.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:52
 * Description:  This file is for ddma interface definition.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/5/13    init commit
 */

#ifndef  FDDMA_H
#define  FDDMA_H

#ifdef __cplusplus
extern "C"
{
#endif
/***************************** Include Files *********************************/
#include "ftypes.h"
#include "ferror_code.h"
/************************** Constant Definitions *****************************/
/* ddma capacity mask  */
#define FDDMA_CAP_W0_ENABLE_IRQ  BIT(0) /* 1 in REG means disable, 0 means enable */
#define FDDMA_CAP_W1_ENABLE_IRQ  BIT(1) /* by contrast, 1 in REG means enable, 0 means disable  */

#define FDDMA_SUCCESS                   FT_SUCCESS
#define FDDMA_ERR_NOT_INIT              FT_MAKE_ERRCODE(ErrModBsp, ErrDdma, 0)
#define FDDMA_ERR_IS_USED               FT_MAKE_ERRCODE(ErrModBsp, ErrDdma, 1)
#define FDDMA_ERR_INVALID_INPUT         FT_MAKE_ERRCODE(ErrModBsp, ErrDdma, 2)
#define FDDMA_ERR_WAIT_TIMEOUT          FT_MAKE_ERRCODE(ErrModBsp, ErrDdma, 3)
/**************************** Type Definitions *******************************/
typedef struct _FDdma FDdma;
typedef struct _FDdmaChanIrq FDdmaChanIrq;

typedef enum
{
    FDDMA_CHAN_0 = 0,
    FDDMA_CHAN_1,
    FDDMA_CHAN_2,
    FDDMA_CHAN_3,
    FDDMA_CHAN_4,
    FDDMA_CHAN_5,
    FDDMA_CHAN_6,
    FDDMA_CHAN_7,

    FDDMA_NUM_OF_CHAN
} FDdmaChanIndex; /* DDMA channel index */

typedef enum
{
    FDDMA_CHAN_REQ_RX = 0,
    FDDMA_CHAN_REQ_TX,
} FDdmaChanRequst; /* DDMA channel direction */

typedef enum
{
	FDDMA_CHAN_EVT_REQ_DONE = 0,
    FDDMA_CHAN_EVT_FIFO_EMPTY,
    FDDMA_CHAN_EVT_FIFO_FULL,

    FDDMA_NUM_OF_CHAN_EVT
} FDdmaChanEvt; /* DDMA channel interrupt event */

typedef void (*FDdmaChanEvtHandler)(FDdmaChanIrq *const chan_irq_info_p, void *arg); /* DDMA interrupt event handler */

typedef struct
{
    u32 id;                     /* DDMA ctrl id */
    uintptr base_addr;          /* DDMA ctrl base address */
    u32 irq_num;                /* DDMA ctrl interrupt id */
    u32 irq_prority;            /* DDMA ctrl interrupt priority */
    u32 caps;                   /* DDMAs have a different interrupt control method, such as FDDMA_CAP_W0_ENABLE_IRQ */
} FDdmaConfig; /* DDMA instance configuration */

typedef struct
{
    u32 slave_id;               /* Perpherial slave id for DDMA */
    FDdmaChanRequst req_mode;   /* DDMA transfer direction */
    uintptr ddr_addr;           /* DDMA channel DDR address, could be source or destination, physical address */
    u32 dev_addr;               /* DDMA channel Perpherial base address, could be source or destination */
    u32 trans_len;              /* DDMA channel transfer length */
#define FDDMA_MIN_TRANSFER_LEN      4  /* min bytes in transfer */
    u32 timeout;                /* timeout = 0 means no use DMA timeout */
    /* BDL模式，目前只针对I2S */
    uintptr  first_desc_addr;   /* BDL描述符列表首地址-物理地址 */
    u32      valid_desc_num;    /* 需要使用的BDL描述符个数，从BDL描述符列表第一个描述符开始计数 */
} FDdmaChanConfig;  /* DDMA channel instance */

typedef struct _FDdmaChanIrq
{
	FDdmaChanIndex      channel_id;                               /* 信息所属DDMA通道的ID */
    FDdma               *ddma_instance_p;                         /* 信息所属DDMA通道所属的DDMA控制器实例 */
    void                *evt_handler_args[FDDMA_NUM_OF_CHAN_EVT]; /* DDMA通道事件回调函数输入参数 */
    FDdmaChanEvtHandler evt_handlers[FDDMA_NUM_OF_CHAN_EVT];      /* DDMA通道事件回调函数 */
} FDdmaChanIrq; /* DDMA通道中断回调信息 */

typedef struct _FDdma
{
    FDdmaConfig config;  /* DDMA控制器配置 */
    u32 is_ready;        /* DDMA控制器初始化是否完成 */
    u32 bind_status;     /* DDMA通道绑定标志位，第几位表示第几通道已经被外设绑定并进行过相关配置 */
    FDdmaChanIrq chan_irq_info[FDDMA_NUM_OF_CHAN]; /* DDMA通道事件回调信息集合 */
} FDdma; /* DDMA instance */
/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FDDMA_DDR_ADDR_ALIGMENT            128  /* DMA DDR Buffer need align wiht 128 bytes */
#define FDDMA_GET_BURST_SIZE(brust_align)  (1U << brust_align)
/************************** Function Prototypes ******************************/
/* 获取DDMA实例默认配置 */
const FDdmaConfig *FDdmaLookupConfig(u32 instance_id);

/* 初始化DDMA控制器 */
FError FDdmaCfgInitialize(FDdma *const instance, const FDdmaConfig *controller_config);

/* 去初始化DDMA控制器 */
void FDdmaDeInitialize(FDdma *const instance_p);

/* 配置指定的DDMA通道 */
FError FDdmaChanConfigure(FDdma *const instance_p, FDdmaChanIndex channel_id , const FDdmaChanConfig *channel_config);

/* 释放指定的DDMA通道 */
FError FDdmaChanDeconfigure(FDdma *const instance_p, FDdmaChanIndex channel_id);

/* 使能指定的DDMA通道 */
void FDdmaChanActive(FDdma *const instance_p, FDdmaChanIndex channel_id);

/* 去使能指定的DDMA通道 */
FError FDdmaChanDeactive(FDdma *const instance_p, FDdmaChanIndex channel_id);

/* 启动DDMA控制器并开始传输，请确保FDdmaChanConfigure()与FDdmaChanActive()步骤已完成 */
void FDdmaStart(FDdma *const instance_p);

/* 停止DDMA控制器 */
void FDdmaStop(FDdma *const instance_p);

/* DDMA中断处理函数 */
void FDdmaIrqHandler(s32 vector, void *args);

/* 注册DDMA通道中断响应事件函数 */
void FDdmaRegisterChanEvtHandler(FDdma *const instance_p,
								 FDdmaChanIndex channel_id,
								 FDdmaChanEvt evt,
								 FDdmaChanEvtHandler handler,
								 void *handler_arg);

/* DDMA控制器寄存器自检测试 */
void FDdmaDumpRegisters(uintptr base_addr);

void FDdmaDumpChanRegisters(uintptr base_addr, FDdmaChanIndex chan);

#ifdef __cplusplus
}
#endif

#endif