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
 * FilePath: fddma.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:47
 * Description:  This file is for ddma interface implementation.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/5/13    init commit
 * 1.1  liqiaozhong 2023/12/12   an overhaul
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
#define FDDMA_DEBUG_TAG "FDDMA-C"
#define FDDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_WARN(format, ...)   FT_DEBUG_PRINT_W(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/
void FDdmaStop(FDdma *const instance_p);
FError FDdmaChanDeconfigure(FDdma *const instance_p, FDdmaChanIndex channel_id);
/****************************************************************************/
/**
 * @name: FDdmaReset
 * @msg: 重置DDMA控制器
 * @return {FError} FDDMA_SUCCESS表示重置成功，其它返回值表示失败
 * @param {FDdma} *instance_p, DDMA控制器实例
 */
static FError FDdmaReset(FDdma *const instance_p)
{
    FASSERT(instance_p);
    uintptr base_addr = instance_p->config.base_addr;
    FError ret = FDDMA_SUCCESS;

    FDdmaDisable(base_addr); /* disable DDMA  */
    FDdmaDisableGlobalIrq(base_addr, instance_p->config.caps);

    /* disable channel and its irq */
	for (fsize_t chan_id = FDDMA_CHAN_0; chan_id < FDDMA_NUM_OF_CHAN; chan_id++)
	{
		instance_p->chan_irq_info[chan_id].channel_id = chan_id;
		instance_p->chan_irq_info[chan_id].ddma_instance_p = instance_p;

		/* reset channel */
		FDdmaChanDeconfigure(instance_p, chan_id);

		if (instance_p->bind_status & BIT(chan_id))
		{
			instance_p->bind_status |~ BIT(chan_id);
		}
	}
	FDDMA_INFO("Finish to reset all DDMA channels.");

    FDdmaSoftReset(base_addr);
    FDDMA_INFO("Fininsh to reset DDMA controller.");

    FDdmaDumpRegisters(base_addr);

    return ret;
}

/**
 * @name: FDdmaCfgInitialize
 * @msg: 初始化DDMA控制器
 * @return {FError} FDDMA_SUCCESS表示初始化成功，其它返回值表示初始化失败
 * @param {FDdma} *instance_p, DDMA控制器实例
 * @param {FDdmaConfig} *controller_config_p, DDMA控制器配置
 */
FError FDdmaCfgInitialize(FDdma *const instance_p, const FDdmaConfig *controller_config_p)
{
    FASSERT(instance_p);
    FASSERT(controller_config_p);
    uintptr base_addr = controller_config_p->base_addr;
    FError ret = FDDMA_SUCCESS;

    if (FT_COMPONENT_IS_READY == instance_p->is_ready)
    {
    	FDdmaDeInitialize(instance_p);
        FDDMA_WARN("DDMA controller has been initialized already.");
    }

    instance_p->config = *controller_config_p;

    ret = FDdmaReset(instance_p);
    if (FDDMA_SUCCESS == ret)
    {
        instance_p->is_ready = FT_COMPONENT_IS_READY;
        FDDMA_INFO("ddma@0x%x initialized successfully.", base_addr);
    }

    return ret;
}

/**
 * @name: FDdmaDeInitialize
 * @msg: 去初始化DDMA控制器
 * @return {无}
 * @param {FDdma} *instance_p, DDMA控制器实例
 */
void FDdmaDeInitialize(FDdma *const instance_p)
{
    FASSERT(instance_p);
    FASSERT(instance_p->config.base_addr);

	if (FDdmaReset(instance_p) != FDDMA_SUCCESS)
	{
		FDDMA_ERROR("DDMA@0x%x reset fail.", instance_p->config.base_addr);
	}
    memset(instance_p, 0, sizeof(*instance_p));

    return;
}

/**
 * @name: FDdmaChanConfigure
 * @msg: DDMA通道配置与绑定操作
 * @return {FError} FDDMA_SUCCESS表示配置成功，其它返回值表示配置失败
 * @param {FDdma} *instance_p, DDMA控制器实例
 * @param {FDdmaChanIndex} channel_id, 通道号
 * @param {FDdmaChanConfig} *channel_config, DDMA通道配置
 */
FError FDdmaChanConfigure(FDdma *const instance_p, FDdmaChanIndex channel_id , const FDdmaChanConfig *channel_config)
{
    FASSERT(instance_p);
    FASSERT(channel_id < FDDMA_NUM_OF_CHAN);
    FASSERT(channel_config);

    FError ret = FDDMA_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;
    u32 reg_val;

    if (FDdmaIsChanRunning(base_addr, channel_id))
	{
		FDDMA_ERROR("Channel-%d is still running.", channel_id);
		return FDDMA_ERR_IS_USED;
	}

    if (instance_p->bind_status & BIT(channel_id))
    {
        FDDMA_WARN("Channel-%d has already been used.", channel_id);
        return FDDMA_ERR_IS_USED;
    }

    if (channel_config->ddr_addr % FDDMA_DDR_ADDR_ALIGMENT)
    {
        FDDMA_ERROR("DDR addr 0x%x must be aligned with %d bytes.", channel_config->ddr_addr, FDDMA_DDR_ADDR_ALIGMENT);
        return FDDMA_ERR_INVALID_INPUT;
    }

    if ((FDDMA_MIN_TRANSFER_LEN > channel_config->trans_len) ||
        (0 != channel_config->trans_len % FDDMA_MIN_TRANSFER_LEN))
    {
        FDDMA_ERROR("Invalid transfer size %d bytes, it should be an integer multiple of 4 bytes.", channel_config->trans_len);
        return FDDMA_ERR_INVALID_INPUT;
    }

    /* disable DDMA controller */
    FDdmaStop(instance_p);

    /* step1: pause and reset channel */
    FDdmaChanDeconfigure(instance_p, channel_id);

    /* step2: select and bind channel */
    FDdmaSetChanSelect(base_addr, channel_id, channel_config->slave_id); /* select channel */
    FDdmaSetChanBind(base_addr, channel_id, TRUE); /* bind channel */

    /* step3: setup transfer src and dst */
    /* dma_tx_req: ddr --> dev 从内存中读取数据，写入外设 */
    /* dma_rx_req: dev --> ddr 从外设读取数据到内存 */
#ifdef __aarch64__
    FDdmaWriteReg(base_addr, FDDMA_CHAN_DDR_LOW_ADDR_OFFSET(channel_id), LOWER_32_BITS(channel_config->ddr_addr));
    FDdmaWriteReg(base_addr, FDDMA_CHAN_DDR_UP_ADDR_OFFSET(channel_id), UPPER_32_BITS(channel_config->ddr_addr));
#else
    FDdmaWriteReg(base_addr, FDDMA_CHAN_DDR_LOW_ADDR_OFFSET(channel_id), (u32)(channel_config->ddr_addr));
    FDdmaWriteReg(base_addr, FDDMA_CHAN_DDR_UP_ADDR_OFFSET(channel_id), 0);
#endif

    /* step4: 写dma_chal_dev_addr寄存器，配置每个通道访问外设寄存器地址 */
    FDdmaWriteReg(base_addr, FDDMA_CHAN_DEV_ADDR_OFFSET(channel_id), channel_config->dev_addr);

    /* step5: 写dma_chal_ts寄存器，配置每个通道传输数据的总数量 */
    FDdmaWriteReg(base_addr, FDDMA_CHAN_TS_OFFSET(channel_id), channel_config->trans_len);

    /* step6: set channel request direction */
    FDdmaSetChanDirection(base_addr, channel_id, (FDDMA_CHAN_REQ_RX == channel_config->req_mode) ? TRUE : FALSE);

    FDDMA_INFO("Channel-%d AND ddr@0x%x", channel_id, FDDMA_CHAN_DDR_LOW_ADDR_OFFSET(channel_id));
    FDDMA_INFO("ddr_addr: 0x%x", FDdmaReadReg(base_addr, FDDMA_CHAN_DDR_LOW_ADDR_OFFSET(channel_id)));
    FDDMA_INFO("dev_addr: 0x%x", FDdmaReadReg(base_addr, FDDMA_CHAN_DEV_ADDR_OFFSET(channel_id)));
    FDDMA_INFO("Transfer length: %d", FDdmaReadReg(base_addr, FDDMA_CHAN_TS_OFFSET(channel_id)));

    if (ret == FDDMA_SUCCESS)
    {
        instance_p->bind_status |= BIT(channel_id);
        FDDMA_INFO("Channel-%d has been successfully configured.", channel_id);
    }

    return ret;
}

/**
 * @name: FDdmaChanDeconfigure
 * @msg: 释放DDMA通道
 * @return {FError} FDDMA_SUCCESS表示释放成功，其它返回值表示释放失败
 * @param {FDdma} *instance_p, DDMA控制器实例
 * @param {FDdmaChanIndex} channel_id, 通道号
 */
FError FDdmaChanDeconfigure(FDdma *const instance_p, FDdmaChanIndex channel_id)
{
    FASSERT(instance_p);
    FASSERT(channel_id < FDDMA_NUM_OF_CHAN);

    uintptr base_addr = instance_p->config.base_addr;
    FError ret = FDDMA_SUCCESS;

    ret = FDdmaDisableChan(base_addr, channel_id);
    if (FDDMA_SUCCESS != ret)
    {
        FDDMA_ERROR("Failure to disable DDMA@%p channel-%d.", base_addr, channel_id);
        return ret;
    }
    FDdmaDisableChanIrq(base_addr, channel_id, instance_p->config.caps); /* disable channel irq */
    FDdmaClearChanIrq(base_addr, channel_id, instance_p->config.caps);
    FDdmaSetChanBind(base_addr, channel_id, FALSE);
    FDdmaSetChanDeselect(base_addr, channel_id);
    FDdmaChanSoftReset(base_addr, channel_id);

    instance_p->bind_status &= ~BIT(channel_id); /* set bind status */

    FDDMA_INFO("Deconfigure channel-%d", channel_id);

    return ret;
}

/**
 * @name: FDdmaChanActive
 * @msg: 使能指定的DDMA通道
 * @return {FError} 返回FDDMA_SUCCESS表示成功，返回其它表示失败
 * @param {FDdma} *instance_p, DDMA控制器实例
 * @param {FDdmaChanIndex} channel_id, 通道号
 */
void FDdmaChanActive(FDdma *const instance_p, FDdmaChanIndex channel_id)
{
	FASSERT(instance_p);
	FASSERT(channel_id < FDDMA_NUM_OF_CHAN);
	FASSERT(instance_p->bind_status & BIT(channel_id));

    uintptr base_addr = instance_p->config.base_addr;

    FDdmaClearChanIrq(base_addr, channel_id, instance_p->config.caps);
    FDdmaEnableChan(base_addr, channel_id);
    FDdmaEnableChanIrq(base_addr, channel_id, instance_p->config.caps);

    return;
}

/**
 * @name: FDdmaChanDeactive
 * @msg: 去使能指定的DDMA通道
 * @return {FError} 返回FDDMA_SUCCESS表示成功，返回其它表示失败
 * @param {FDdma} *instance_p, DDMA控制器实例
 * @param {FDdmaChanIndex} channel_id, 通道号
 */
FError FDdmaChanDeactive(FDdma *const instance_p, FDdmaChanIndex channel_id)
{
	FASSERT(instance_p);
	FASSERT(channel_id < FDDMA_NUM_OF_CHAN);

    return FDdmaDisableChan(instance_p->config.base_addr, channel_id);
}

/**
 * @name: FDdmaStart
 * @msg: 启动DDMA控制器，开始传输
 * @return {FError} FDDMA_SUCCESS表示启动成功，其它返回值表示启动失败
 * @param {FDdma} *instance_p, DDMA控制器实例
 */
void FDdmaStart(FDdma *const instance_p)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready);

    uintptr base_addr = instance_p->config.base_addr;

    FDdmaEnableGlobalIrq(base_addr, instance_p->config.caps);
    FDdmaEnable(base_addr);

    return;
}

/**
 * @name: FDdmaStop
 * @msg: 停止DDMA控制器
 * @return {FError} FDDMA_SUCCESS表示停止成功，其它返回值表示停止失败
 * @param {FDdma} *instance_p, DDMA控制器实例
 */
void FDdmaStop(FDdma *const instance_p)
{
    FASSERT(instance_p);
    FError ret = FDDMA_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;

    FDdmaDisable(base_addr);
    FDdmaDisableGlobalIrq(base_addr, instance_p->config.caps);

    return;
}