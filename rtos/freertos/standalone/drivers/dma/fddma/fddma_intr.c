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
 * FilePath: fddma_intr.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:47
 * Description:  This file is for ddma interrupt implementation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/5/13    init commit
 */

/***************************** Include Files *********************************/
#include <string.h>

#include "fparameters.h"
#include "fassert.h"
#include "fdrivers_port.h"

#include "fddma_hw.h"
#include "fddma.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FDDMA_DEBUG_TAG "DDMA-INTR"
#define FDDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_WARN(format, ...)   FT_DEBUG_PRINT_W(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)

#define FDDMA_CALL_EVT_HANDLER(express, chan_irq_info_p, args) \
    do                                                         \
	{                                                          \
		if (express)                                           \
		{                                                      \
			express(chan_irq_info_p, args);                    \
		}                                                      \
	} while (0)                                                \
/************************** Function Prototypes ******************************/

/****************************************************************************/
/**
 * @name: FDdmaChanIrqHandler
 * @msg: DDMA通道中断处理函数
 * @return {*}
 * @param {FDdma} *instance_p, DDMA实例
 * @param {FDdmaChanIndex} chan_idx, DDMA通道号
 */
static void FDdmaChanIrqHandler(FDdmaChanIrq *const chan_irq_info_p)
{
	FASSERT(chan_irq_info_p);

	FDdmaChanIndex channel_id = chan_irq_info_p->channel_id;
	FDdma *instance_p = chan_irq_info_p->ddma_instance_p;
	uintptr base_addr = instance_p->config.base_addr;

    u32 chan_status = FDdmaReadChanStatus(base_addr, channel_id);
    FDDMA_INFO("channel-%d irq status: 0x%x", channel_id, chan_status);

    FDDMA_CALL_EVT_HANDLER(chan_irq_info_p->evt_handlers[FDDMA_CHAN_EVT_REQ_DONE],
						   chan_irq_info_p,
						   chan_irq_info_p->evt_handler_args[FDDMA_CHAN_EVT_REQ_DONE]);

    if (FDDMA_CHAN_STS_FIFO_EMPTY & chan_status)
    {
        FDDMA_CALL_EVT_HANDLER(chan_irq_info_p->evt_handlers[FDDMA_CHAN_EVT_FIFO_EMPTY],
        					   chan_irq_info_p,
        			           chan_irq_info_p->evt_handler_args[FDDMA_CHAN_EVT_FIFO_EMPTY]);
    }

    if (FDDMA_CHAN_STS_FIFO_FULL & chan_status)
    {
        FDDMA_CALL_EVT_HANDLER(chan_irq_info_p->evt_handlers[FDDMA_CHAN_EVT_FIFO_FULL],
        		               chan_irq_info_p,
        		               chan_irq_info_p->evt_handler_args[FDDMA_CHAN_EVT_FIFO_FULL]);
    }

    return;
}

/**
 * @name: FDdmaIrqHandler
 * @msg: DDMA中断处理函数
 * @return {无}
 * @param {s32} vector
 * @param {void} *param, 输入参数
 */
void FDdmaIrqHandler(s32 vector, void *args)
{
    FASSERT(NULL != args);
    FDdma *const instance_p = (FDdma * const)args;
    uintptr base_addr = instance_p->config.base_addr;
    u32 status = FDdmaReadStatus(base_addr);

    FDDMA_INFO("Into DDMA total irq handler, status: 0x%x", status);

    /* DDMA-2与DDMA-0的中断使能寄存器的规则不同 */
    /* poll, clear and process every chanel interrupt status */
    FDdmaDisableGlobalIrq(base_addr, instance_p->config.caps); /* disable interrupt from occur again */
    if (instance_p->config.caps & FDDMA_CAP_W1_ENABLE_IRQ)
    {
        for (fsize_t chan_id = FDDMA_CHAN_0; chan_id < FDDMA_NUM_OF_CHAN; chan_id++)
        {
            if (0 == (FDDMA_STA_CHAN_REQ_DONE(chan_id) & status))
            {
                continue;
            }
            FDDMA_INFO("Into channel-%d irq handler", chan_id);
            FDdmaClearChanIrq(base_addr, chan_id, instance_p->config.caps);
            FDdmaChanIrqHandler(&instance_p->chan_irq_info[chan_id]);
        }
    }
    else
    {
        for (fsize_t chan_id = FDDMA_CHAN_0; chan_id < FDDMA_NUM_OF_CHAN; chan_id++)
        {
            if (0 == (FDDMA_BDL_STA_CHAN_REQ_DONE(chan_id) & status))
            {
                continue;
            }
            FDDMA_INFO("Into channel-%d irq handler", chan_id);
            FDdmaClearChanIrq(base_addr, chan_id, instance_p->config.caps);
            FDdmaChanIrqHandler(&instance_p->chan_irq_info[chan_id]);
        }
    }

    FDdmaEnableGlobalIrq(base_addr, instance_p->config.caps); /* re-enable interrupt */
    return;
}

/**
 * @name: FDdmaRegisterChanEvtHandler
 * @msg: 注册DDMA通道中断响应事件函数
 * @return {无}
 * @param {FDdma} *instance_p, DDMA控制器实例
 * @param {FDdmaChanIndex} channel_id, 通道号
 * @param {FDdmaChanEvt} evt, 中断事件
 * @param {FDdmaChanEvtHandler} handler, 中断响应事件函数
 * @param {void} *handler_arg, 中断响应事件函数输入参数
 */
void FDdmaRegisterChanEvtHandler(FDdma *const instance_p,
		                         FDdmaChanIndex channel_id,
								 FDdmaChanEvt evt,
								 FDdmaChanEvtHandler handler,
								 void *handler_arg)
{
	FASSERT(instance_p);
	FASSERT(channel_id < FDDMA_NUM_OF_CHAN);
	FASSERT(evt < FDDMA_NUM_OF_CHAN_EVT);

    instance_p->chan_irq_info[channel_id].evt_handlers[evt] = handler;
    instance_p->chan_irq_info[channel_id].evt_handler_args[evt] = handler_arg;

    return;
}