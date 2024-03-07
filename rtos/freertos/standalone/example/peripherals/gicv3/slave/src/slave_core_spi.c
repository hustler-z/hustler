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
 * FilePath: slave_core_spi.c
 * Date: 2023-04-26 08:37:22
 * LastEditTime: 2022-04-27 11:00:53
 * Description:  This file is for slave core spi functions.
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------     --------------------------------------
 * 1.0   liqiaozhong   2023/4/27    first commit and add spi, sgi, spi functions
 */

#include <stdio.h>
#include <string.h>

#include "fpl011.h"
#include "finterrupt.h"
#include "fparameters.h"
#include "fcache.h"
#include "fmmu.h"
#include "fcache.h"

#include "gic_common.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static u32 *share_flg_pointor;
static u8 *share_recv_buffer;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
FPl011 *uart_p = (FPl011 *)FGIC_SPI_UART_INSTANCE_ADDRESS;

static void SlaveCoreSpiHandler(s32 vector, void *param)
{
    FPl011 *uart_p = (FPl011 *)param;
    u32 reg_value = 0;
    u32 recv_cnt = 0;
    FASSERT(uart_p != NULL);
    FASSERT(uart_p->is_ready == FT_COMPONENT_IS_READY);
    FCacheDCacheInvalidateRange((intptr)uart_p, sizeof(FPl011));
    reg_value = FUART_READREG32(uart_p->config.base_address, FPL011IMSC_OFFSET);
    reg_value &= FUART_READREG32(uart_p->config.base_address, FPL011MIS_OFFSET);

    if ((reg_value & ((u32)FPL011MIS_RTMIS)) != (u32)0)
    {
        /* Received Timeout interrupt */
        recv_cnt = FPl011Receive(uart_p, share_recv_buffer, SHARE_BUFFER_DATA_OFFSET);
    }

    if ((reg_value & ((u32)FPL011MIS_RXMIS)) != (u32)0)
    {
        /* Received data interrupt */
        recv_cnt += FPl011Receive(uart_p, share_recv_buffer, SHARE_BUFFER_DATA_OFFSET);
    }

    if (recv_cnt != 0)
    {
        for (size_t i = 0; i < recv_cnt; i++)
        {
            printf("Slave get byte %c \r\n", share_recv_buffer[i]);
        }

        *share_flg_pointor   = SHARE_BUFFER_FLG_FROM_SLAVE;
        FCacheDCacheFlushLine((intptr)share_flg_pointor);
    }

    /* Clear the interrupt status. */
    FUART_WRITEREG32(uart_p->config.base_address, FPL011ICR_OFFSET, reg_value);
}

void SlaveCoreSpiEarlyInit(void)
{
    share_flg_pointor = (u32 *)SHARE_BUFFER_BASE;
    share_recv_buffer = (u8 *)(SHARE_BUFFER_BASE + SHARE_BUFFER_DATA_OFFSET);
    FSetTlbAttributes((uintptr)uart_p, sizeof(FPl011), (MT_NORMAL_NC | MT_P_RW_U_RW));
    InterruptInstall(FGIC_SPI_UART_IRQ_VECTOR, SlaveCoreSpiHandler, uart_p, "slave_uart");
}
