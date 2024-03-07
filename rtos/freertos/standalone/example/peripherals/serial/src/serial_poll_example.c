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
 * FilePath: serial_poll_example.c
 * Date: 2023-04-12 14:53:42
 * LastEditTime: 2023-04-14 17:46:03
 * Description:  This file is for serial poll example function implementation.
 *
 * Modify History:
 *  Ver     Who          Date             Changes
 * -----   ------      --------    --------------------------------------
 *  1.0   liuzhihong   2023/4/12        first release
 */

/***************************** Include Files *********************************/

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_TARGET_E2000
#include "fio_mux.h"
#endif

#include "serial_poll_example.h"
/************************** Constant Definitions *****************************/
FPl011Format format_default_poll =
{
    .baudrate = FPL011_BAUDRATE,
    .data_bits = FPL011_FORMAT_WORDLENGTH_8BIT,
    .parity = FPL011_FORMAT_NO_PARITY,
    .stopbits = FPL011_FORMAT_1_STOP_BIT
};
/**************************** Type Definitions *******************************/
/* data structure used in example */

/************************** Variable Definitions *****************************/
/* variables used in example */

/***************** Macros (Inline Functions) Definitions *********************/
#define POLL_TEST_BUFFER_SIZE   26
/************************** Constant Definitions *****************************/
static u8 send_buffer_p[POLL_TEST_BUFFER_SIZE]; /*poll Buffer for Transmitting Data */
static u8 recv_buffer_p[POLL_TEST_BUFFER_SIZE]; /*poll Buffer for Receiving Data */
/************************** Function Prototypes ******************************/

/******************************* Functions ***********************************/
static FError FSerialPolled(FPl011 *uart_ptr)
{
    int i = 0;
    u32 poll_total_send_count, recv_count, poll_bad_byte_count = 0;

    /* Use local loopback mode. */
    FPl011SetOperMode(uart_ptr, FPL011_OPER_MODE_LOCAL_LOOP);
    for (i = 0; i < POLL_TEST_BUFFER_SIZE; i++)
    {
        send_buffer_p[i] = (i % 26) + 'A';
        recv_buffer_p[i] = 0;
    }

    poll_total_send_count = FPl011Send(uart_ptr, send_buffer_p, POLL_TEST_BUFFER_SIZE);
    while (FUART_ISTRANSMITBUSY(uart_ptr->config.base_address))
        ;
    if (poll_total_send_count != POLL_TEST_BUFFER_SIZE)
    {
        printf("Polled send is error %d!\n", poll_total_send_count);
        return ERR_GENERAL;
    }

    recv_count = 0;
    while (recv_count < POLL_TEST_BUFFER_SIZE)
    {
        recv_count += FPl011Receive(uart_ptr, &recv_buffer_p[recv_count], (POLL_TEST_BUFFER_SIZE - recv_count));
    }

    /* Verify the entire receive buffer was successfully received */
    for (i = 0; i < POLL_TEST_BUFFER_SIZE; i++)
    {
        if (recv_buffer_p[i] != send_buffer_p[i])
        {
            poll_bad_byte_count++;
        }
    }

    /* Set the UART in Normal Mode */
    FPl011SetOperMode(uart_ptr, FPL011_OPER_MODE_NORMAL);
    if (poll_bad_byte_count != 0)
    {
        printf("FSerialPoll is wrong!\n");
        return ERR_GENERAL;
    }
    printf("FSerialPoll is ok.\n");

    return ERR_SUCCESS;
}

static FError FSerialPollInit(u32 id, FPl011Format *format, FPl011 *uart_p)
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
    memcpy(&config_value, config_p, sizeof(FPl011Config));

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
    printf("FSerialPollInit baudrate:%d,data_bits:%d,parity:%d,stopbits:%d is ok.\n", format->baudrate, format->data_bits + 5, format->parity, format->stopbits + 1);
    return FT_SUCCESS;
}
static FError FSerialPollDeinit(FPl011 *uart_p)
{
    FError ret;
    FASSERT(uart_p != NULL);

    /* load default farmat */
    ret = FPl011SetDataFormat(uart_p, &format_default_poll);
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
    printf("FSerialPollDeinit is ok.\r\n");
    return FT_SUCCESS;
}

/* function of serial polled loopback mode example */
int FSerialPollExample()
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
        /* init instance */
        ret = FSerialPollInit(id, &format, &uart_p);
        if (ret != FT_SUCCESS)
        {
            printf("FSerialPollInit uart_id :%d is error!\n",id);
            break;
        }
           
        /* poll mode enable */
        ret = FSerialPolled(&uart_p);
        if (ret != FT_SUCCESS)
        {
            printf("FSerialPolled uart_id :%d is error!\n",id);
            break;
        }

        /* deinit instance */
        ret = FSerialPollDeinit(&uart_p);
        memset(&uart_p, 0, sizeof(FPl011));
        if (ret != FT_SUCCESS)
        {
            printf("FSerialPollDeinit uart_id :%d is error!\n",id);
            break;
        }
    }
#ifdef CONFIG_TARGET_E2000
    FIOMuxDeInit();
#endif
    /* print message on example run result */
    if (0 == ret)
    {
        printf("%s@%d: Serial poll example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: Serial poll example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
    }

    return 0;
}

