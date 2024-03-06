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
 * FilePath: gic_common.h
 * Date: 2023-04-26 08:37:22
 * LastEditTime: 2022-04-27 11:00:53
 * Description:  This file is for gic example Macros(Inline Functions) Definitions.
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------     --------------------------------------
 * 1.0   liqiaozhong   2023/4/27    first commit and add spi, sgi, spi functions
 */

#ifndef GIC_COMMON_H
#define GIC_COMMON_H

/***************************** Include Files *********************************/
#include "fparameters.h"
#include "sdkconfig.h"
#include "fdebug.h"

#ifdef __cplusplus
extern "C"
{
#endif
/************************** Constant Definitions *****************************/
enum
{
    FGIC_EXAMPLE_OK = 0,
    FGIC_EXAMPLE_TIMEOUT,
    FGIC_EXAMPLE_TEST_FAIL,
    FGIC_EXAMPLE_NOT_YET_INIT, 
    FGIC_EXAMPLE_INIT_FAILED,
    FGIC_EXAMPLE_INVALID_PARAM,
};

/***************** Macros (Inline Functions) Definitions *********************/
#define SHARE_BUFFER_BASE           CONFIG_SHARE_BUFFER_BASE
#define SHARE_BUFFER_FLG_OFFSET     0
#define SHARE_BUFFER_DATA_OFFSET    0x10
#define SHARE_BUFFER_DATA_BYTE_LENGTH 0xff

/* SHARE_BUFFER_FLG_OFFSET */
#define SHARE_BUFFER_FLG_FROM_SLAVE 0x2
#define SHARE_BUFFER_FLG_FROM_MASTER 0x1

#ifdef CONFIG_SLAVE_CORE_ID
    #define SLAVE_CORE_ID   CONFIG_SLAVE_CORE_ID
#endif

#ifdef CONFIG_MASTER_CORE_ID
    #define MASTER_CORE_ID   CONFIG_MASTER_CORE_ID
#endif

#define SLAVE_ELF_ADDRESS CONFIG_SLAVE_ELF_ADDRESS

#define FGIC_SPI_UART_INDEX FUART0_ID
#define FGIC_SPI_UART_BASE FUART0_BASE_ADDR
#define FGIC_SPI_UART_INSTANCE_ADDRESS CONFIG_FGIC_SPI_UART_INSTANCE_ADDRESS
#define FGIC_SPI_UART_IRQ_VECTOR FUART0_IRQ_NUM

/* SGI test */
#define FGIC_OTHER_SGI_VECT 0

/* LOG print */
#define FGIC_DEBUG_TAG "FGIC_EXAMPLE"
#define FGIC_ERROR(format, ...) FT_DEBUG_PRINT_E(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGIC_WARRN(format, ...) FT_DEBUG_PRINT_W(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGIC_INFO(format, ...) FT_DEBUG_PRINT_I(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGIC_DEBUG(format, ...) FT_DEBUG_PRINT_D(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif

