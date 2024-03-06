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
 * FilePath: fddma_hw.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:47
 * Description:  This files is for ddma register rw operations
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
#define FDDMA_DEBUG_TAG "DDMA-HW"
#define FDDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_WARN(format, ...)   FT_DEBUG_PRINT_W(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/

/****************************************************************************/
/**
 * @name: FDdmaDisable
 * @msg: 去使能DDMA控制器
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 */
void FDdmaDisable(uintptr base_addr)
{
    u32 reg_val = FDdmaReadReg(base_addr, FDDMA_CTL_OFFSET);
    reg_val &= ~FDDMA_CTL_ENABLE;
    FDdmaWriteReg(base_addr, FDDMA_CTL_OFFSET, reg_val);

    FDDMA_DEBUG("DDMA@%p disabled: 0x%x", base_addr, FDdmaReadReg(base_addr, FDDMA_CTL_OFFSET));

    return;
}

/**
 * @name: FDdmaEnable
 * @msg: 使能DDMA控制器
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 */
void FDdmaEnable(uintptr base_addr)
{
    u32 reg_val = FDdmaReadReg(base_addr, FDDMA_CTL_OFFSET);
    reg_val |= FDDMA_CTL_ENABLE;
    FDdmaWriteReg(base_addr, FDDMA_CTL_OFFSET, reg_val);
    FDDMA_DEBUG("DDMA@%p enabled: 0x%x", base_addr, FDdmaReadReg(base_addr, FDDMA_CTL_OFFSET));
    return;
}

/**
 * @name: FDdmaSoftReset
 * @msg: 复位DDMA控制器
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 */
void FDdmaSoftReset(uintptr base_addr)
{
    int delay = 10000;
    u32 reg_val = FDdmaReadReg(base_addr, FDDMA_CTL_OFFSET);
    reg_val |= FDDMA_CTL_SRST;
    FDdmaWriteReg(base_addr, FDDMA_CTL_OFFSET, reg_val);
    FDDMA_DEBUG("DDMA@%p soft reset start: 0x%x", base_addr, FDdmaReadReg(base_addr, FDDMA_CTL_OFFSET));

    while (--delay > 0) /* wait a while to do reset */
        ;

    reg_val &= ~FDDMA_CTL_SRST;
    FDdmaWriteReg(base_addr, FDDMA_CTL_OFFSET, reg_val); /* exit from software reset */
    FDDMA_DEBUG("DDMA@%p soft reset end: 0x%x", base_addr, FDdmaReadReg(base_addr, FDDMA_CTL_OFFSET));
    return;
}

/**
 * @name: FDdmaDisableGlobalIrq
 * @msg: 关闭DDMA全局中断
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 * @param {u32} caps, DDMA中断控制特性
 */
void FDdmaDisableGlobalIrq(uintptr base_addr, u32 caps)
{
    if (caps & FDDMA_CAP_W1_ENABLE_IRQ)
    {
        u32 reg_val = FDdmaReadReg(base_addr, FDDMA_MASK_INTR_OFFSET);
        reg_val |= FDDMA_MASK_EN_GLOBAL_INTR; 
        FDdmaWriteReg(base_addr, FDDMA_MASK_INTR_OFFSET, reg_val);
    }
    else
    {
        u32 reg_val = FDdmaReadReg(base_addr, FDDMA_MASK_INTR_OFFSET);
        reg_val &= ~ FDDMA_MASK_EN_GLOBAL_INTR; 
        FDdmaWriteReg(base_addr, FDDMA_MASK_INTR_OFFSET, reg_val);
    }
    return;
}

/**
 * @name: FDdmaEnableGlobalIrq
 * @msg: 打开DDMA全局中断
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 * @param {u32} caps, DDMA中断控制特性
 */
void FDdmaEnableGlobalIrq(uintptr base_addr, u32 caps)
{
    if (caps & FDDMA_CAP_W1_ENABLE_IRQ)
    {
        u32 reg_val = FDdmaReadReg(base_addr, FDDMA_MASK_INTR_OFFSET);
        reg_val &= ~ FDDMA_MASK_EN_GLOBAL_INTR; 
        FDdmaWriteReg(base_addr, FDDMA_MASK_INTR_OFFSET, reg_val);
    }
    else
    {
        u32 reg_val = FDdmaReadReg(base_addr, FDDMA_MASK_INTR_OFFSET);
        reg_val |= FDDMA_MASK_EN_GLOBAL_INTR; 
        FDdmaWriteReg(base_addr, FDDMA_MASK_INTR_OFFSET, reg_val);
    }

    return;
}

/**
 * @name: FDdmaDisableChanIrq
 * @msg: 关闭DDMA通道中断
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 * @param {u32} channel_id, DDMA通道号
 * @param {u32} caps, DDMA中断控制特性
 */
void FDdmaDisableChanIrq(uintptr base_addr, u32 channel_id, u32 caps)
{
    FASSERT_MSG((FDDMA_NUM_OF_CHAN > channel_id), "Channel %d is not supported.", channel_id);

    if (caps & FDDMA_CAP_W1_ENABLE_IRQ)
    {
        u32 reg_val = FDdmaReadReg(base_addr, FDDMA_MASK_INTR_OFFSET);
        reg_val |= FDDMA_MASK_EN_CHAN_INTR(channel_id);
        FDdmaWriteReg(base_addr, FDDMA_MASK_INTR_OFFSET, reg_val);
    }
    else
    {
        u32 reg_val = FDdmaReadReg(base_addr, FDDMA_MASK_INTR_OFFSET);
        reg_val &= ~FDDMA_MASK_EN_CHAN_INTR(channel_id);
        FDdmaWriteReg(base_addr, FDDMA_MASK_INTR_OFFSET, reg_val);
    }

    return;
}

/**
 * @name: FDdmaEnableChanIrq
 * @msg: 打开DDMA通道中断
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 * @param {u32} channel_id, DDMA通道号
 * @param {u32} caps, DDMA中断控制特性
 */
void FDdmaEnableChanIrq(uintptr base_addr, u32 channel_id, u32 caps)
{
    FASSERT_MSG((FDDMA_NUM_OF_CHAN > channel_id), "Channel %d is not supported.", channel_id);

    if (caps & FDDMA_CAP_W1_ENABLE_IRQ)
    {
        u32 reg_val = FDdmaReadReg(base_addr, FDDMA_MASK_INTR_OFFSET);
        reg_val &= ~ FDDMA_MASK_EN_CHAN_INTR(channel_id);
        FDdmaWriteReg(base_addr, FDDMA_MASK_INTR_OFFSET, reg_val);
    }
    else
    {
        u32 reg_val = FDdmaReadReg(base_addr, FDDMA_MASK_INTR_OFFSET);
        reg_val |= FDDMA_MASK_EN_CHAN_INTR(channel_id);
        FDdmaWriteReg(base_addr, FDDMA_MASK_INTR_OFFSET, reg_val);
    }
    return;
}

/**
 * @name: FDdmaDisableChan
 * @msg: 去使能(暂停)DDMA通道
 * @return {FError} FDDMA_SUCCESS 表示去使能成功
 * @param {uintptr} base_addr, DDMA控制器基地址
 * @param {u32} channel_id, DDMA通道号
 */
FError FDdmaDisableChan(uintptr base_addr, u32 channel_id)
{
    FASSERT_MSG((channel_id < FDDMA_NUM_OF_CHAN), "Channel %d is not supported.", channel_id);

    int delay = 1000;
    u32 reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id));

    /* 先写dma_chal_ctl[0]为 1’b0，等待一段时间，读取到该位是 1’b0后，才能进行后续复位等操作 */
    reg_val &= ~FDDMA_CHAN_CTL_EN;
    FDdmaWriteReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id), reg_val);

    do
    {
        reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id));
        if (delay-- <= 0)
        {
            break;
        }
    }
    while (reg_val & FDDMA_CHAN_CTL_EN);

    FDDMA_DEBUG("DDMA@%p channel_idnel %d is disabled: 0x%x", base_addr, channel_id, FDdmaReadReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id)));

    return (delay > 0) ? FDDMA_SUCCESS : FDDMA_ERR_WAIT_TIMEOUT;
}

/**
 * @name: FDdmaEnableChan
 * @msg: 使能DDMA通道
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 * @param {u32} channel_id, DDMA通道号
 */
void FDdmaEnableChan(uintptr base_addr, u32 channel_id)
{
    FASSERT_MSG((FDDMA_NUM_OF_CHAN > channel_id), "Channel %d is not supported.", channel_id);
    u32 reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id));
    reg_val |= FDDMA_CHAN_CTL_EN;
    FDdmaWriteReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id), reg_val);
    FDDMA_DEBUG("DDMA@%p channel_id %d enabled: 0x%x", base_addr, channel_id, FDdmaReadReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id)));
    return;
}

/**
 * @name: FDdmaClearChanIrq
 * @msg: 清除DDMA通道中断状态
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 * @param {u32} channel_id, DDMA通道号
 * @param {u32} caps, DDMA中断控制特性
 */
void FDdmaClearChanIrq(uintptr base_addr, u32 channel_id, u32 caps)
{
    FASSERT_MSG((FDDMA_NUM_OF_CHAN > channel_id), "Channel %d is not supported.", channel_id);
    /* write 1 to clear irq status of channel_idnel */
      if (caps & FDDMA_CAP_W1_ENABLE_IRQ)
      {
        FDdmaWriteReg(base_addr, FDDMA_STA_OFFSET, FDDMA_STA_CHAN_REQ_DONE(channel_id));
      }
      else
      {
       FDdmaWriteReg(base_addr, FDDMA_STA_OFFSET, FDDMA_BDL_STA_CHAN_REQ_DONE(channel_id));
      }
}

/**
 * @name: FDdmaChanSoftReset
 * @msg: DDMA通道软复位，通道相关寄存器(但不包括0x4，0x20，以及中断相关的寄存器)与FIFO都会复位
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 * @param {u32} channel_id, DDMA通道号
 */
void FDdmaChanSoftReset(uintptr base_addr, u32 channel_id)
{
    FASSERT_MSG((FDDMA_NUM_OF_CHAN > channel_id), "Channel-%d is not supported.", channel_id);
    int delay = 1000;
    u32 reg_val;

    if (FDdmaIsChanRunning(base_addr, channel_id))
    {
    	FDDMA_ERROR("DDMA channel-%d is working, please pause it before soft reset.", channel_id);
    }

    reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id));
    reg_val |= FDDMA_CHAN_CTL_SRST;
    FDdmaWriteReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id), reg_val);

    while (--delay > 0) /* wait a while to do soft reset */
        ;

    reg_val &= ~FDDMA_CHAN_CTL_SRST;
    FDdmaWriteReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id), reg_val);
    FDDMA_DEBUG("Channel reset done, ctrl: 0x%x", FDdmaReadReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id)));

    return;
}

/**
 * @name: FDdmaIsChanRunning
 * @msg: 检查通道是否在工作中
 * @return {boolean} TRUE: 在工作中
 * @param {uintptr} base_addr, DDMA控制器基地址
 * @param {u32} channel_id, DDMA通道号
 */
boolean FDdmaIsChanRunning(uintptr base_addr, u32 channel_id)
{
    FASSERT_MSG((FDDMA_NUM_OF_CHAN > channel_id), "Channel %d is not supported.", channel_id);
    u32 reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id));
    return (FDDMA_CHAN_CTL_EN & reg_val) ? TRUE : FALSE;
}

/**
 * @name: FDdmaSetChanSelect
 * @msg: 将DDMA通道与外设绑定
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 * @param {u32} channel_id, DDMA通道号
 * @param {u32} slave_id, 外设对应的slave id
 */
void FDdmaSetChanSelect(uintptr base_addr, u32 channel_id, u32 slave_id)
{
    FASSERT_MSG((FDDMA_NUM_OF_CHAN > channel_id), "Channel %d is not supported.", channel_id);
    FASSERT_MSG((FDDMA_MAX_SLAVE_ID >= slave_id), "Invalid slave id %d", slave_id);

    u32 reg_val;

    if (FDDMA_CHAN_4 > channel_id)
    {
        reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_0_3_CFG_OFFSET);
        reg_val &= ~FDDMA_CHAN_0_3_SEL_MASK(channel_id);
        reg_val |= FDDMA_CHAN_0_3_SEL(channel_id, slave_id);
        reg_val |= FDDMA_CHAN_0_3_SEL_EN(channel_id);
        FDdmaWriteReg(base_addr, FDDMA_CHAN_0_3_CFG_OFFSET, reg_val);
        FDDMA_DEBUG("DDMA@%p sets the slave id of channel_id-%d to %d, 0x%x", base_addr, channel_id, slave_id, FDdmaReadReg(base_addr, FDDMA_CHAN_0_3_CFG_OFFSET));
    }
    else
    {
        reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_4_7_CFG_OFFSET);
        reg_val &= ~FDDMA_CHAN_4_7_SEL_MASK(channel_id);
        reg_val |= FDDMA_CHAN_4_7_SEL(channel_id, slave_id);
        reg_val |= FDDMA_CHAN_4_7_SEL_EN(channel_id);
        FDdmaWriteReg(base_addr, FDDMA_CHAN_4_7_CFG_OFFSET, reg_val);
        FDDMA_DEBUG("DDMA@%p sets the slave id of channel_id-%d to %d, 0x%x", base_addr, channel_id, slave_id, FDdmaReadReg(base_addr, FDDMA_CHAN_4_7_CFG_OFFSET));
    }

    return;
}

/**
 * @name: FDdmaSetChanDeselect
 * @msg: 取消DDMA通道与外设的绑定
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 * @param {u32} channel_id, DDMA通道号
 * @param {u32} slave_id, 外设对应的slave id
 */
void FDdmaSetChanDeselect(uintptr base_addr, u32 channel_id)
{
    FASSERT_MSG((FDDMA_NUM_OF_CHAN > channel_id), "Channel %d is not supported.", channel_id);

    u32 reg_val;

    if (FDDMA_CHAN_4 > channel_id)
    {
        reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_0_3_CFG_OFFSET);
        reg_val &= ~FDDMA_CHAN_0_3_SEL_MASK(channel_id);
        FDdmaWriteReg(base_addr, FDDMA_CHAN_0_3_CFG_OFFSET, reg_val);
    }
    else
    {
        reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_4_7_CFG_OFFSET);
        reg_val &= ~FDDMA_CHAN_4_7_SEL_MASK(channel_id);
        FDdmaWriteReg(base_addr, FDDMA_CHAN_4_7_CFG_OFFSET, reg_val);
    }
    FDDMA_INFO("DDMA channel-%d set deselection done.", channel_id);

    return;
}

/**
 * @name: FDdmaSetChanBind
 * @msg: 修改通道的绑定状态
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 * @param {u32} channel_id, DDMA通道号
 * @param {boolean} bind, TRUE: 绑定，FALSE: 解除绑定
 */
void FDdmaSetChanBind(uintptr base_addr, u32 channel_id, boolean bind)
{
    FASSERT_MSG((FDDMA_NUM_OF_CHAN > channel_id), "Channel %d is not supported.", channel_id);

    u32 reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_BIND_OFFSET);

    if (bind)
    {
        reg_val |= BIT(channel_id);
    }
    else
    {
        reg_val &= ~BIT(channel_id);
    }

    FDDMA_DEBUG("DDMA@%p %s channel_id-%d, 0x%x", base_addr, bind ? "bind" : "unbind", channel_id, FDdmaReadReg(base_addr, FDDMA_CHAN_BIND_OFFSET));
    FDdmaWriteReg(base_addr, FDDMA_CHAN_BIND_OFFSET, reg_val);

    return;
}

/**
 * @name: FDdmaSetChanDirection
 * @msg: 设置通道的方向
 * @return {*}
 * @param {uintptr} base_addr, DDMA控制器基地址
 * @param {u32} channel_id, DDMA通道号
 * @param {boolean} is_rx, TRUE: 接收, FALSE: 发送
 */
void FDdmaSetChanDirection(uintptr base_addr, u32 channel_id, boolean is_rx)
{
    FASSERT_MSG((FDDMA_NUM_OF_CHAN > channel_id), "Channel %d is not supported.", channel_id);

    u32 reg_val = FDdmaReadReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id));
    if (is_rx)
    {
        reg_val |= FDDMA_CHAN_CTL_RX_MODE;    /* device to memory */
    }
    else
    {
        reg_val &= ~FDDMA_CHAN_CTL_RX_MODE;    /* memory to device */
    }
    FDdmaWriteReg(base_addr, FDDMA_CHAN_CTL_OFFSET(channel_id), reg_val);

    return;
}