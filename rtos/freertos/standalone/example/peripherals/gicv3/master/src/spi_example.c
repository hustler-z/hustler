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
 * FilePath: spi_example.c
 * Date: 2023-04-26 08:37:22
 * LastEditTime: 2022-04-27 11:00:53
 * Description:  This file is for the spi test example functions.
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------     --------------------------------------
 * 1.0   liqiaozhong   2023/4/27    first commit and add spi, sgi, spi functions
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "ftypes.h"
#include "finterrupt.h"
#include "fcache.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "fpl011.h"
#include "gic_common.h"
#include "spi_example.h"
/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define TEST_BUFFER_SIZE         120
#define SPI_ROUTE_TRANS_LENGTH   5
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static u8 send_buffer[TEST_BUFFER_SIZE];    /* Buffer for Transmitting Data */
static u8 recv_buffer[TEST_BUFFER_SIZE];    /* Buffer for Receiving Data */
static u8 *share_recv_buffer = 0;
static u32 *share_flg_pointor = 0;
FPl011 *uart_p = (FPl011 *)FGIC_SPI_UART_INSTANCE_ADDRESS;
static volatile int total_received_count;
static volatile int total_sent_count;
static int total_error_count;
/* user defined parameters */
static u32 priority = IRQ_PRIORITY_VALUE_0;
/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
static void IntrTestHandler(void *args, u32 event, u32 event_data)
{
    /* All of the data has been sent */
    if (event == FPL011_EVENT_SENT_DATA)
    {
        total_sent_count = event_data;
    }

    /* All of the data has been received */
    if (event == FPL011_EVENT_RECV_DATA)
    {
        total_received_count = event_data;
    }
    /*
     * Data was received, but not the expected number of bytes, a
     * timeout just indicates the data stopped for 8 character times
     */
    if (event == FPL011_EVENT_RECV_TOUT)
    {
        total_received_count = event_data;
    }

    /*
     * Data was received with an error, keep the data but determine
     * what kind of errors occurred
     */
    if (event == FPL011_EVENT_RECV_ERROR)
    {
        total_received_count = event_data;
        total_error_count++;
    }

    /*
     * Data was received with an parity or frame or break error, keep the data
     * but determine what kind of errors occurred. Specific to Zynq Ultrascale+
     * MP.
     */
    if (event == FPL011_EVENT_PARE_FRAME_BRKE)
    {
        total_received_count = event_data;
        total_error_count++;
    }
}

/* early data init */
static void FGicSpiEarlyInit(void)
{
    share_flg_pointor = (u32 *)SHARE_BUFFER_BASE;
    share_recv_buffer = (u8 *)SHARE_BUFFER_BASE + SHARE_BUFFER_DATA_OFFSET;
}

/* init Uart-0 and spi intr */
static int FGicSpiUartInit(void)
{
    FPl011Config config_value;
    FPl011Format format;
    const FPl011Config *config_p;
    FError ret;
    config_p = FPl011LookupConfig(FGIC_SPI_UART_INDEX);
    if (NULL == config_p)
    {
        FGIC_ERROR("Lookup ID is error.");
        return FGIC_EXAMPLE_INIT_FAILED;
    }
    memcpy(&config_value, config_p, sizeof(FPl011Config));

    ret = FPl011CfgInitialize(uart_p, &config_value);
    if (ret != FT_SUCCESS)
    {
        FGIC_ERROR("Uart initialize is error.");
        return FGIC_EXAMPLE_INIT_FAILED;
    }

    format.baudrate = 115200; /* In bps, ie 1200 */
    format.data_bits = FPL011_FORMAT_WORDLENGTH_8BIT;
    format.parity = FPL011_FORMAT_NO_PARITY;
    format.stopbits = FPL011_FORMAT_1_STOP_BIT;

    ret = FPl011SetDataFormat(uart_p, &format);
    if (ret != FT_SUCCESS)
    {
        FGIC_ERROR("Pl011SetDataFormat is error.");
        return FGIC_EXAMPLE_INIT_FAILED;
    }

    /* Start Uart */
    FPl011SetOptions(uart_p, FPL011_OPTION_UARTEN | FPL011_OPTION_RXEN | FPL011_OPTION_TXEN | FPL011_OPTION_DTR | FPL011_OPTION_RTS);

    FGIC_INFO("TestUartInit is ok.");

    /* common uart isr init */
    FPl011SetHandler(uart_p, IntrTestHandler, NULL);
    InterruptSetPriority(uart_p->config.irq_num, priority);
    InterruptInstall(uart_p->config.irq_num, (IrqHandler)FPl011InterruptHandler, uart_p, "uart");
    FPl011SetRxFifoThreadhold(uart_p, FPL011IFLS_RXIFLSEL_1_4);
    FPl011SetTxFifoThreadHold(uart_p, FPL011IFLS_TXIFLSEL_1_4);
    return FGIC_EXAMPLE_OK;
}

/* Uart send and receive test through spi intr */
static int FGicSpiTest(FPl011 *uart_p)
{
    u32 intr_mask;
    int i;
    int bad_byte_count = 0;
    InterruptSetTargetCpus(uart_p->config.irq_num, MASTER_CORE_ID);
    InterruptUmask(uart_p->config.irq_num);

    intr_mask = (FPL011IMSC_RXIM |
                 FPL011IMSC_TXIM |
                 FPL011IMSC_RTIM |
                 FPL011IMSC_FEIM |
                 FPL011IMSC_PEIM |
                 FPL011IMSC_BEIM |
                 FPL011IMSC_OEIM);

    FPl011SetInterruptMask(uart_p, intr_mask);

    /* Use local loopback mode. */
    FPl011SetOperMode(uart_p, FPL011_OPER_MODE_LOCAL_LOOP);

    for (i = 0; i < TEST_BUFFER_SIZE; i++)
    {
        send_buffer[i] = (i % 26) + 'A';
        recv_buffer[i] = 0;
    }

    total_received_count = 0;
    total_sent_count = 0;
    total_error_count = 0;
    FPl011Receive(uart_p, recv_buffer, TEST_BUFFER_SIZE);
    total_sent_count  = FPl011Send(uart_p, send_buffer, TEST_BUFFER_SIZE);

    while ((total_error_count == 0) && ((total_sent_count != TEST_BUFFER_SIZE) || (total_received_count != TEST_BUFFER_SIZE)))
        ;

    for (i = 0; i < TEST_BUFFER_SIZE; i++)
    {
        if (recv_buffer[i] != send_buffer[i])
        {
            FGIC_ERROR("Send is %c, recv get is %c", send_buffer[i], recv_buffer[i]);
            bad_byte_count++;
        }
    }

    FPl011SetOperMode(uart_p, FPL011_OPER_MODE_NORMAL);
    FPl011SetInterruptMask(uart_p, ~intr_mask);

    if (bad_byte_count != 0)
    {
        FGIC_ERROR("SpiTest is error %d \r\n", bad_byte_count);
        return FGIC_EXAMPLE_TEST_FAIL;
    }

    FGIC_INFO("SpiTest is ok.");

    return FGIC_EXAMPLE_OK;
}

static int FGicSpiRouteTest(FPl011 *uart_p)
{
    u32 intr_mask;
    int i;
    int ret = 0;
    int bad_byte_count = 0;
    u32 timeout_cnt = 0;
    *share_flg_pointor = 0;

    InterruptSetTargetCpus(uart_p->config.irq_num, SLAVE_CORE_ID);
    InterruptUmask(uart_p->config.irq_num);

    intr_mask = (FPL011IMSC_RXIM |
                 FPL011IMSC_RTIM);

    FPl011SetInterruptMask(uart_p, intr_mask);

    /* Use local loopback mode. */
    FPl011SetOperMode(uart_p, FPL011_OPER_MODE_LOCAL_LOOP);

    for (i = 0; i < SPI_ROUTE_TRANS_LENGTH; i++)
    {
        send_buffer[i] = (i % 26) + 'A';
        share_recv_buffer[i] = 0;
    }
    FCacheDCacheFlushLine((intptr)share_recv_buffer);
    FPl011BlockSend(uart_p, send_buffer, SPI_ROUTE_TRANS_LENGTH);
    while (*share_flg_pointor != SHARE_BUFFER_FLG_FROM_SLAVE)
    {
        if (timeout_cnt ++ >= 0xf00000)
        {
            FGIC_ERROR("master recive time out");
            goto end_label;
        }

        FCacheDCacheInvalidateLine((intptr)share_flg_pointor);
    }
    FCacheDCacheInvalidateLine((intptr)share_recv_buffer);
    for (i = 0; i < SPI_ROUTE_TRANS_LENGTH; i++)
    {
        if (share_recv_buffer[i] != send_buffer[i])
        {
            bad_byte_count++;
            FGIC_ERROR("Spi routing test recv data is error.");
            ret = FGIC_EXAMPLE_TEST_FAIL;
        }
    }

end_label:
    FPl011SetOperMode(uart_p, FPL011_OPER_MODE_NORMAL);
    FPl011SetInterruptMask(uart_p, ~intr_mask);

    if (bad_byte_count != 0)
    {
        FGIC_ERROR("RouteTest is error %d \r\n", bad_byte_count);
        return FGIC_EXAMPLE_TEST_FAIL;
    }

    FGIC_INFO("RouteTest is %s", (ret == FT_SUCCESS) ? "ok" : "fail");

    return ret;
}


/* function of SPI intr example */
int FSpiExample(void)
{
    int ret = FGIC_EXAMPLE_OK;
    int err = FGIC_EXAMPLE_OK;

    FGicSpiEarlyInit();
    ret = FGicSpiUartInit();
    if (ret != FGIC_EXAMPLE_OK)
    {
        err = FGIC_EXAMPLE_INIT_FAILED;
        FGIC_ERROR("Spi-Uart-0 init fail.");
        goto exit;
    }

    ret = FGicSpiTest(uart_p);
    if (ret != FGIC_EXAMPLE_OK)
    {
        err = FGIC_EXAMPLE_TIMEOUT;
        FGIC_ERROR("Spi-Uart test timeout.");
    }

    FCacheDCacheFlushRange((intptr)uart_p, sizeof(FPl011)); /* flush uart instance to slave */
    FGIC_INFO("FGicSpiRouteTest is start.");
    
    ret = FGicSpiRouteTest(uart_p);
    if (ret != FGIC_EXAMPLE_OK)
    {
        err = FGIC_EXAMPLE_TIMEOUT;
        FGIC_ERROR("Spi route test timeout.");
    }

exit:
    /* print message on example run result */
    if (0 == err)
    {
        printf("%s@%d: spi example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: spi example failed !!!, ret = %d \r\n", __func__, __LINE__, err);
    }

    return 0;
}