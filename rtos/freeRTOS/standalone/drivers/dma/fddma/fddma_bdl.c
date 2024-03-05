/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: fddma_bdl.c
 * Created Date: 2023-08-08 15:46:34
 * Last Modified: 2023-12-21 11:12:11
 * Description:  This file is for DDMA BDL transfer interface implementation which is only used in I2S examples.
 *
 * Modify History:
 *  Ver       Who             Date         Changes
 * -----   ----------       --------     ---------------------------------
 *  1.0  wangzongqiang     2023/07/23    init
 *  1.1  liqiaozhong       2023/12/12    an overhaul
 */

#include <string.h>
#include "fparameters.h"
#include "fassert.h"
#include "fdrivers_port.h"

#include "fddma_hw.h"
#include "fddma.h"
#include "fddma_bdl.h"
/***************** Macros (Inline Functions) Definitions *********************/
#define FDDMA_DEBUG_TAG "DDMA"
#define FDDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_WARN(format, ...)   FT_DEBUG_PRINT_W(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
/*****************************************************************************/
/**
 * @name FDdmaBDLSetDesc
 * @msg: 设置一个BDL描述符，仅在支持BDL模式的DDMA控制器有效，目前仅在I2S用例中使用
 * @return {FError} FGDMA_SUCCESS 表示设置成功
 * @param FGdmaBdlDesc *const first_desc_addr_p BDL描述符列表首地址
 * @param FGdmaBdlDescConfig const *bdl_desc_config_p BDL描述符配置
 * @note 在BDL模式通道配置前应该先完成BDL描述符配置
 */
FError FDdmaBDLSetDesc(FDdmaBdlDesc *const first_desc_addr_p, FDdmaBdlDescConfig const *bdl_desc_config_p)
{
	FASSERT(first_desc_addr_p);
	FASSERT(bdl_desc_config_p);

	FDdmaBdlDesc *bdl_desc_p = &first_desc_addr_p[bdl_desc_config_p->current_desc_num];
	uintptr src_addr = bdl_desc_config_p->src_addr;

#ifdef __aarch64__
	bdl_desc_p->src_addr_l = LOWER_32_BITS(src_addr);
	bdl_desc_p->src_addr_h = UPPER_32_BITS(src_addr);
#else
	bdl_desc_p->src_addr_h = 0U;
	bdl_desc_p->src_addr_l = src_addr;
#endif
	bdl_desc_p->total_bytes = bdl_desc_config_p->trans_length;
	bdl_desc_p->ioc = bdl_desc_config_p->ioc;

	return FDDMA_SUCCESS;
}

/**
 * @name: FDdmaBdlSetChan
 * @msg:  检查并设置用于DDMA BDL模式的通道，仅在支持BDL模式的DDMA控制器有效，目前仅在I2S用例中使用
 * @param {FDdma} *instance_p, DDMA控制器实例
 * @param {FDdmaChan} *dma_chan, DDMA通道实例
 * @param {FDdmaChanConfig} *channel_config_p, DDMA通道配置
 * @return {FError} FDDMA_SUCCESS表示重置成功，其它返回值表示失败
 */
FError FDdmaChanBdlConfigure(FDdma *const instance_p, FDdmaChanIndex channel_id, const FDdmaChanConfig *channel_config_p)
{
    FASSERT(instance_p);
    FASSERT(channel_id < FDDMA_NUM_OF_CHAN);
    FASSERT(channel_config_p);

    FError ret = FDDMA_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;
    FDdmaBdlDesc *first_desc_addr_p = (FDdmaBdlDesc *)channel_config_p->first_desc_addr;
    u32 reg_val;

    if (FDdmaIsChanRunning(base_addr, channel_id))
	{
		FDDMA_WARN("Channel-%d is still running.", channel_id);
		return FDDMA_ERR_IS_USED;
	}

    if (instance_p->bind_status & BIT(channel_id))
	{
		FDDMA_WARN("Channel-%d has already been used.", channel_id);
		return FDDMA_ERR_IS_USED;
	}

    if (channel_config_p->ddr_addr % FDDMA_DDR_ADDR_ALIGMENT)
    {
        FDDMA_ERROR("DDR addr 0x%x must be aligned with %d bytes.", channel_config_p->ddr_addr, FDDMA_DDR_ADDR_ALIGMENT);
        return FDDMA_ERR_INVALID_INPUT;
    }

    if ((FDDMA_MIN_TRANSFER_LEN > channel_config_p->trans_len) ||
        (0 != channel_config_p->trans_len % FDDMA_MIN_TRANSFER_LEN))
    {
        FDDMA_ERROR("Invalid transfer size %d bytes, it should be an integer multiple of 4 bytes.", channel_config_p->trans_len);
        return FDDMA_ERR_INVALID_INPUT;
    }

    /* check BDL desc list */
    for (fsize_t loop = 0; loop < channel_config_p->valid_desc_num; loop++)
    {
        if (first_desc_addr_p[loop].src_addr_l == 0 && first_desc_addr_p[loop].src_addr_h == 0)
        {
            FDDMA_ERROR("BDL descriptor-%d has not been set.", loop + 1);
            return FDDMA_ERR_INVALID_INPUT;
        }
    }
    FDDMA_INFO("BDL descriptor list has been completely set.");

    /* disable DDMA controller */
    FDdmaStop(instance_p);

    /* step1: pause and reset channel */
    FDdmaChanDeconfigure(instance_p, channel_id);

    /* step2: write first address of BDL descrip list to corresponding REG */
#ifdef __aarch64__
    FDdmaWriteReg(base_addr, FDDMA_CHAN_DDR_UP_ADDR_OFFSET(channel_id), UPPER_32_BITS((uintptr)first_desc_addr_p));
    FDdmaWriteReg(base_addr, FDDMA_CHAN_DDR_LOW_ADDR_OFFSET(channel_id), LOWER_32_BITS((uintptr)first_desc_addr_p));
#else
    FDdmaWriteReg(base_addr, FDDMA_CHAN_DDR_UP_ADDR_OFFSET(channel_id), 0x0U);
    FDdmaWriteReg(base_addr, FDDMA_CHAN_DDR_LOW_ADDR_OFFSET(channel_id), (u32)((uintptr)first_desc_addr_p));
#endif

    /* step3: basic transfer REG set */
    FDdmaWriteReg(base_addr, FDDMA_CHAN_DEV_ADDR_OFFSET(channel_id), channel_config_p->dev_addr);
    FDdmaWriteReg(base_addr, FDDMA_CHAN_BUFFER_SIZE(channel_id), channel_config_p->trans_len);
    FDdmaWriteReg(base_addr, FDDMA_CHAN_BDL_VALID_NUM(channel_id), channel_config_p->valid_desc_num - 1); /* 寄存器要求填入的数值为有效BDL描述个数减一 */

    if (channel_config_p->req_mode == FDDMA_CHAN_REQ_TX)
	{
		/*通道信号线源选择，通道0做发送端，通道1做接收端*/
		reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_0_3_CFG_OFFSET);
		reg_val |= FDDMA_CHAN_0_3_SEL_EN(channel_id);
		reg_val &= ~ FDDMA_CHAN_0_3_SEL_BDL(channel_id);
		FDdmaWriteReg(base_addr, FDDMA_CHAN_0_3_CFG_OFFSET, reg_val);

		/*AXI读写每次传输字节长度*/
		reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_DSIZE(channel_id));
		reg_val |= FDDMA_CHAN_AXI_READ_SIZE_SET(FDDMA_BURST_SIZE_4_BYTE);
		reg_val |= FDDMA_CHAN_AXI_WRITE_SIZE_SET(FDDMA_BURST_SIZE_2_BYTE);
		FDdmaWriteReg(base_addr, FDDMA_CHAN_DSIZE(channel_id), reg_val);

		/*AXI读写每次传输长度*/
		reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_DLENTH(channel_id));
		reg_val |= FDDMA_CHAN_AXI_READ_LENGTH_SET(FDDMA_BURST_LENGTH_1);
		reg_val |= FDDMA_CHAN_AXI_WRITE_LENGTH_SET(FDDMA_BURST_LENGTH_1);
		FDdmaWriteReg(base_addr, FDDMA_CHAN_DLENTH(channel_id), reg_val);

		/*配置通道DMA->device传输模式*/
		reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id));
		reg_val &= ~FDDMA_CHAN_CTL_RXTX_MODE;
		FDdmaWriteReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id), reg_val);
	}

	else
	{
		/*通道信号线源选择，通道0做发送端，通道1做接收端*/
		reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_0_3_CFG_OFFSET);
		reg_val |= FDDMA_CHAN_0_3_SEL_EN(channel_id);
		reg_val |= FDDMA_CHAN_0_3_SEL_BDL(channel_id);
		FDdmaWriteReg(base_addr, FDDMA_CHAN_0_3_CFG_OFFSET, reg_val);

		/*AXI读写每次传输字节长度*/
		reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_DSIZE(channel_id));
		reg_val |= FDDMA_CHAN_AXI_READ_SIZE_SET(FDDMA_BURST_SIZE_2_BYTE);
		reg_val |= FDDMA_CHAN_AXI_WRITE_SIZE_SET(FDDMA_BURST_SIZE_4_BYTE);
		FDdmaWriteReg(base_addr, FDDMA_CHAN_DSIZE(channel_id), reg_val);

		/*AXI读写每次传输长度*/
		reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_DLENTH(channel_id));
		reg_val |= FDDMA_CHAN_AXI_READ_LENGTH_SET(FDDMA_BURST_LENGTH_1);
		reg_val |= FDDMA_CHAN_AXI_WRITE_LENGTH_SET(FDDMA_BURST_LENGTH_1);
		FDdmaWriteReg(base_addr, FDDMA_CHAN_DLENTH(channel_id), reg_val);
		/*配置通道device - > DMA传输模式*/
		reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id));
		reg_val |= FDDMA_CHAN_CTL_RXTX_MODE;
		FDdmaWriteReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id), reg_val);
	}

    if (ret == FDDMA_SUCCESS)
	{
		instance_p->bind_status |= BIT(channel_id);
		FDDMA_INFO("Channel-%d has been successfully configured.", channel_id);
	}

    return ret;
}