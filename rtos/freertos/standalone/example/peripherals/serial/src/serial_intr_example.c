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
 * FilePath: serial_intr_example.c
 * Date: 2023-04-12 14:53:42
 * LastEditTime: 2023-04-17 17:46:03
 * Description:  This file is for serial interrupt example function implementation.
 *
 * Modify History:
 *  Ver     Who          Date             Changes
 * -----   ------      --------    --------------------------------------
 *  1.0   liuzhihong   2023/4/17        first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "fsleep.h"
#include "ftypes.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_TARGET_E2000
#include "fio_mux.h"
#endif

#include "serial_intr_example.h"
/************************** Constant Definitions *****************************/
FPl011Format format_default_intr =
{
    .baudrate = FPL011_BAUDRATE,
    .data_bits = FPL011_FORMAT_WORDLENGTH_8BIT,
    .parity = FPL011_FORMAT_NO_PARITY,
    .stopbits = FPL011_FORMAT_1_STOP_BIT
};
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define INTR_TEST_BUFFER_SIZE   10000
/************************** Constant Definitions *****************************/
static u8 send_buffer_i[INTR_TEST_BUFFER_SIZE]; /*intr Buffer for Transmitting Data */
static u8 recv_buffer_i[INTR_TEST_BUFFER_SIZE]; /*intr Buffer for Receiving Data */
static volatile int total_received_count;
static volatile int total_sent_count;
static volatile int total_error_count;
/************************** Function Prototypes ******************************/

/******************************* Functions ***********************************/
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

    /*
     * Data was received with an overrun error, keep the data but determine
     * what kind of errors occurred. Specific to Zynq Ultrascale+ MP.
     */
    if (event == FPL011_EVENT_RECV_ORERR)
    {
        total_received_count = event_data;
        total_error_count++;
    }
}

static FError FSerialIntr(FPl011 *uart_ptr)
{
    u32 intr_mask;
    int i;
    int bad_byte_count = 0;
    total_received_count = 0;

    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(uart_ptr->config.irq_num, cpu_id);

    InterruptInstall(uart_ptr->config.irq_num, (IrqHandler)FPl011InterruptHandler, uart_ptr, "uart");
    InterruptUmask(uart_ptr->config.irq_num);
    FPl011SetHandler(uart_ptr, IntrTestHandler, NULL);
    FPl011SetRxFifoThreadhold(uart_ptr, FPL011IFLS_RXIFLSEL_1_4);
    FPl011SetTxFifoThreadHold(uart_ptr, FPL011IFLS_TXIFLSEL_1_2);

    intr_mask = (FPL011IMSC_RXIM |
                 FPL011IMSC_TXIM |
                 FPL011IMSC_RTIM |
                 FPL011IMSC_FEIM |
                 FPL011IMSC_PEIM |
                 FPL011IMSC_BEIM |
                 FPL011IMSC_OEIM);

    FPl011SetInterruptMask(uart_ptr, intr_mask);

    /* Use local loopback mode. */
    FPl011SetOperMode(uart_ptr, FPL011_OPER_MODE_LOCAL_LOOP);

    for (i = 0; i < INTR_TEST_BUFFER_SIZE; i++)
    {
        send_buffer_i[i] = (i % 26) + 'A';
        recv_buffer_i[i] = 0;
    }

    FPl011Receive(uart_ptr, recv_buffer_i, INTR_TEST_BUFFER_SIZE);
    total_error_count = 0;
    total_sent_count = FPl011Send(uart_ptr, send_buffer_i, INTR_TEST_BUFFER_SIZE);

    while ((total_error_count == 0) && ((total_sent_count != INTR_TEST_BUFFER_SIZE) || (total_received_count != INTR_TEST_BUFFER_SIZE)))
        ;

    for (i = 0; i < INTR_TEST_BUFFER_SIZE; i++)
    {
        if (recv_buffer_i[i] != send_buffer_i[i])
        {
            bad_byte_count++;
        }
    }

    FPl011SetOperMode(uart_ptr, FPL011_OPER_MODE_NORMAL);
    intr_mask ^= FPl011GetInterruptMask(uart_ptr);
    FPl011SetInterruptMask(uart_ptr, intr_mask);
    FPl011InterruptClearAll(uart_ptr);

    if (bad_byte_count != 0)
    {
        printf("FSerialIntr is wrong, the bad_byte_count is %d.\r\n", bad_byte_count);
        return ERR_GENERAL;
    }

    printf("FSerialIntr is ok.\n");

    return FT_SUCCESS;
}

static FError FSerialIntrInit(u32 id, FPl011Format *format, FPl011 *uart_p)
{
    FPl011Config config_value;
    const FPl011Config *config_p;
    FError ret;
    config_p = FPl011LookupConfig(id);
    if (NULL == config_p)
    {
        printf("Lookup ID is error!\n");
        return ERR_GENERAL;
    }
    memcpy(&config_value, config_p, sizeof(FPl011Config)) ;
#ifdef CONFIG_TARGET_E2000
    /*init iomux*/
    FIOMuxInit();
    FIOPadSetUartMux(id);
#endif
    ret = FPl011CfgInitialize(uart_p, &config_value);
    if (ret != FT_SUCCESS)
    {
        printf("Uart initialize is error!\n");
        return ERR_GENERAL;
    }

    ret = FPl011SetDataFormat(uart_p, format);
    if (ret != FT_SUCCESS)
    {
        printf("Pl011SetDataFormat is error!\n");
        return ERR_GENERAL;
    }

    /* set UART */
    FPl011SetOptions(uart_p, FPL011_OPTION_UARTEN | FPL011_OPTION_FIFOEN | FPL011_OPTION_RXEN | FPL011_OPTION_TXEN | FPL011_OPTION_DTR | FPL011_OPTION_RTS);
    printf("FSerialIntrInit uart_id:%d,baudrate:%d,data_bits:%d,parity:%d,stopbits:%d is ok.\n",id,format->baudrate, format->data_bits + 5, format->parity, format->stopbits + 1);
    return FT_SUCCESS;
}

static FError FSerialIntrDeinit(FPl011 *uart_p)
{
    FError ret;
    FASSERT(uart_p != NULL);

    /* load default farmat */
    ret = FPl011SetDataFormat(uart_p, &format_default_intr);
    if (ret != FT_SUCCESS)
    {
        printf("Pl011ResetDataFormat is error!\n");
        return ERR_GENERAL;
    }

    /* stop UART */
    u32 reg = 0U;
    reg &= ~(FPL011_OPTION_UARTEN | FPL011_OPTION_FIFOEN | FPL011_OPTION_RXEN | FPL011_OPTION_TXEN | FPL011_OPTION_DTR | FPL011_OPTION_RTS);
    FPl011SetOptions(uart_p, reg);

    /* set is_ready to no_ready */
    uart_p->is_ready = 0U;
    printf("FSerialIntrDeinit is ok.\r\n");
    return FT_SUCCESS;
}  

/* function of serial intr loopback mode example */
int FSerialIntrExample()
{   
    int ret = 0;
    FPl011 uart_p;
    FPl011Format format =
    {
        .baudrate = FPL011_BAUDRATE,
        .data_bits = FPL011_FORMAT_WORDLENGTH_8BIT,
        .parity = FPL011_FORMAT_NO_PARITY,
        .stopbits = FPL011_FORMAT_1_STOP_BIT
    };
    memset(&uart_p, 0, sizeof(FPl011));

    for (fsize_t id = 0; id < FUART_NUM ; id++)
    {
        if (id == FUART1_ID) /* FUART1 is used in SHELL */
        {
            continue; 
        }
        /* init process */
        ret = FSerialIntrInit(id, &format, &uart_p);
        if (ret != FT_SUCCESS)
        {
            printf("FSerialIntrInit uart_id :%d is error!\n",id);
            break;
        }
        /* intr mode enable */
        ret = FSerialIntr(&uart_p);
        if (ret != FT_SUCCESS)
        {
            printf("FSerialIntr uart_id :%d is error!\n",id);
            break;
        }
        /* deinit process */
        ret = FSerialIntrDeinit(&uart_p);
        memset(&uart_p, 0, sizeof(FPl011));
        if (ret != FT_SUCCESS)
        {
            printf("FSerialIntrDeinit uart_id :%d is error!\n",id);
            break;
        }

    }
    /*deinit iopad*/
#ifdef CONFIG_TARGET_E2000
    FIOMuxDeInit();
#endif
    if (0 == ret)
    {
        printf("%s@%d: Serial intr example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: Serial intr example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
    }
    return 0;
}

