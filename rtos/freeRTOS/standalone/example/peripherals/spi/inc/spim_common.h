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
 * FilePath: spim_common.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for spim common definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/4/12   first release
 */
#ifndef  SPIM_COMMON_H
#define  SPIM_COMMON_H

#include "fdebug.h"
#include "fspim.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FSPIM_DEBUG_TAG "SPIM_TEST"
#define FSPIM_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_WARN(format, ...)   FT_DEBUG_PRINT_W(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_INFO(format, ...)    FT_DEBUG_PRINT_I(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)

#define FSPIM_TX_RX_LENGTH          256   /*default spim tx and rx length */
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD)
#define FSPIM_TEST_ID               2     /*default spim test id*/
#else
#define FSPIM_TEST_ID               0     /*default spim test id*/
#endif
#define FSPIM_TEST_BYTES_LENGTH     32    /*default spim test bytes_length*/
#define FSPIM_DELAY_MS              100   /*timer Delay in milli-seconds */

/*spi chip select */
#define FSPIM_CS_ON                 TRUE
#define FSPIM_CS_OFF                FALSE

#define FSPIM_DMA_ENABLE            TRUE
#define FSPIM_DMA_DISABLE           FALSE

/*if bit_16 enable, the transmission data width is 16 bits, otherwise it is 8 bits*/
#define FSPIM_BIT_16_ENABLE         TRUE
#define FSPIM_BIT_16_DISABLE        FALSE

/*spim test mode enable or disable*/
#define SPIM_TEST_MODE_ENABLE       TRUE

typedef int (*FSpimOpsTxRx)(FSpim *spim ,u8 *tx_buf, fsize_t tx_len, u8 *rx_buf, fsize_t rx_len);

enum
{
    FSPIM_OPS_OK = 0,
    FSPIM_OPS_ALREADY_INIT,
    FSPIM_OPS_NOT_YET_INIT,
    FSPIM_OPS_INVALID_PARA,
    FSPIM_OPS_NOT_SUPPORT,
    FSPIM_OPS_INIT_FAILED,
    FSPIM_OPS_TEST_ABORT,
    FSPIM_OPS_TEST_FAILED,
    FSPIM_OPS_TRANS_TIMEOUT,
    FSPIM_OPS_TRANS_FAILED,
};

typedef enum
{
    FSPIM_OPS_LOOPBACK_POLL_FIFO,
    FSPIM_OPS_LOOPBACK_INTERRUPT
} FSpimOpsLoopBackType;

/*init spim function*/
int FSpimOpsInit(u32 spi_id, boolean test_mode, boolean en_dma, boolean bit_16);
/*spim loopback function*/
int FSpimLoopBack(u32 bytes, FSpimOpsLoopBackType type);
/*deinit spim function*/
int FSpimOpsDeInit(void);
/*spim tx and rx in interrupt mode*/
int FSpimTxRxInterrupt(FSpim *spim, u8 *tx_buf, fsize_t tx_len, u8 *rx_buf, fsize_t rx_len);
/*spim tx and rx in polled mode*/
int FSpimTxRxPollFifo(FSpim *spim, u8 *tx_buf, fsize_t tx_len, u8 *rx_buf, fsize_t rx_len);
/*spim cs on and off function*/
void FSpimCsOnOff(boolean on);
/*spim determine if reception is complete in interrupt mode*/
int FSpimWaitRxDone(int timeout);

#ifdef __cplusplus
}
#endif

#endif

