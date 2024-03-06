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
 * FilePath: serial_mio_example.c
 * Date: 2023-04-12 14:53:42
 * LastEditTime: 2023-06-21 17:57:24
 * Description:  This file is for serial MIO example function implementation.
 *
 * Modify History:
 *  Ver     Who          Date             Changes
 * -----   ------      --------    --------------------------------------
 *  1.0   liuzhihong   2023/4/12        first release
 *  1.1   liqiaozhong  2023/6/21        add flow, mio, ddma related examples
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "fio_mux.h"

#include "serial_mio_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
/* user-defined parameters */
#define TEST_BUFFER_SIZE    10
#define UART_CONTROLLER_ID  FUART1_ID /* as a reference for MIO config */

#ifdef CONFIG_TARGET_PHYTIUMPI
#define MIO_CONTROLLER_ID       FMIO2_ID
#else
#define MIO_CONTROLLER_ID       FMIO15_ID
#endif

/* FT-defined print form */
#define FSERIAL_DEBUG_TAG "FSERIAL_EXAMPLE"
#define FSERIAL_ERROR(format, ...) FT_DEBUG_PRINT_E(FSERIAL_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSERIAL_WARRN(format, ...) FT_DEBUG_PRINT_W(FSERIAL_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSERIAL_INFO(format, ...) FT_DEBUG_PRINT_I(FSERIAL_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSERIAL_DEBUG(format, ...) FT_DEBUG_PRINT_D(FSERIAL_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Constant Definitions *****************************/
static u8 send_buffer[TEST_BUFFER_SIZE]; /* poll buffer for transmitting data */
static u8 recv_buffer[TEST_BUFFER_SIZE]; /* poll buffer for receiving data */
static FPl011Config uart_config;
static FPl011 uart_instance;
static FMioConfig mio_config;
static FMioCtrl mio_ctrl;
static FPl011Format uart_format =
{
    .baudrate = FPL011_BAUDRATE,
    .data_bits = FPL011_FORMAT_WORDLENGTH_8BIT,
    .parity = FPL011_FORMAT_NO_PARITY,
    .stopbits = FPL011_FORMAT_1_STOP_BIT
};

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
static volatile int total_received_count;
static volatile int total_send_count;
static volatile int total_error_count;
/************************** Function Prototypes ******************************/

/******************************* Functions ***********************************/
static void IntrTestHandler(void *args, u32 event, u32 event_data)
{
    /* All of the data has been sent */
    if (event == FPL011_EVENT_SENT_DATA)
    {
        total_send_count = event_data;
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

static FError FSerialMioTest()
{    
    int i;
    u32 intr_mask;
    u32 cpu_id;

    total_received_count = 0;
    total_error_count = 0;

    /* intr init */
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(uart_instance.config.irq_num, cpu_id);
    InterruptInstall(uart_instance.config.irq_num, (IrqHandler)FPl011InterruptHandler, &uart_instance, "uart");
    InterruptUmask(uart_instance.config.irq_num);
	FPl011SetHandler(&uart_instance, IntrTestHandler, NULL);
	FPl011SetRxFifoThreadhold(&uart_instance, FPL011IFLS_RXIFLSEL_1_4);
    FPl011SetTxFifoThreadHold(&uart_instance, FPL011IFLS_TXIFLSEL_1_2);

    intr_mask = (FPL011IMSC_RXIM | FPL011IMSC_TXIM |
				 FPL011IMSC_RTIM | FPL011IMSC_FEIM |
	             FPL011IMSC_PEIM | FPL011IMSC_BEIM |
				 FPL011IMSC_OEIM);
    FPl011SetInterruptMask(&uart_instance, intr_mask);

    printf("UART intr init ==> base = %p, irq= %d \r\n", uart_instance.config.base_address, uart_instance.config.irq_num);

    /* Use normal mode */
    FPl011SetOperMode(&uart_instance, FPL011_OPER_MODE_NORMAL);

    for (i = 0; i < TEST_BUFFER_SIZE; i++)
    {
        send_buffer[i] = (i % 26) + 'A';
        recv_buffer[i] = 0;
    }
    printf("UART send A~%c characters. \r\n", 'A' + TEST_BUFFER_SIZE - 1);
    printf("UART recv begin, you can put %d characters from uart to test. \r\n", TEST_BUFFER_SIZE);

    FPl011Receive(&uart_instance, recv_buffer, TEST_BUFFER_SIZE);
    total_send_count = FPl011Send(&uart_instance, send_buffer, TEST_BUFFER_SIZE);

    while ((total_error_count == 0) && ((total_send_count != TEST_BUFFER_SIZE) || (total_received_count != TEST_BUFFER_SIZE)))
    {
        ;
    }
    for (i = 0; i < total_received_count; i++)
    {
        printf("Received %d character = %c. \r\n", i, recv_buffer[i]);
    }

    intr_mask ^= FPl011GetInterruptMask(&uart_instance);
    FPl011SetInterruptMask(&uart_instance, intr_mask);
    FPl011InterruptClearAll(&uart_instance);

    return FT_SUCCESS;
}

static FError FSerialMioInit()
{
    FError ret;

    /* MIO init */
    mio_ctrl.config = *FMioLookupConfig(MIO_CONTROLLER_ID);
    
    ret = FMioFuncInit(&mio_ctrl, FMIO_FUNC_SET_UART);
    if (ret != FT_SUCCESS)
    {
        FSERIAL_ERROR("MIO initialize error.");
        return ERR_GENERAL;
    }
    /*init iomux*/
    FIOMuxInit();
    FIOPadSetMioMux(MIO_CONTROLLER_ID);
    /* UART init and connect to MIO */
    uart_config = *FPl011LookupConfig(UART_CONTROLLER_ID);
    uart_config.base_address = FMioFuncGetAddress(&mio_ctrl, FMIO_FUNC_SET_UART);
    uart_config.irq_num = FMioFuncGetIrqNum(&mio_ctrl, FMIO_FUNC_SET_UART);
    uart_config.ref_clock_hz = FMIO_CLK_FREQ_HZ;

    ret = FPl011CfgInitialize(&uart_instance, &uart_config);
    if (ret != FT_SUCCESS)
    {
        FSERIAL_ERROR("UART initialize error!");
        return ERR_GENERAL;
    }

    ret = FPl011SetDataFormat(&uart_instance, &uart_format);
    if (ret != FT_SUCCESS)
    {
        FSERIAL_ERROR("Pl011SetDataFormat() error!");
        return ERR_GENERAL;
    }

    /* set UART */
    FPl011SetOptions(&uart_instance, 
                     FPL011_OPTION_UARTEN | FPL011_OPTION_FIFOEN | 
                     FPL011_OPTION_RXEN | FPL011_OPTION_TXEN | 
                     FPL011_OPTION_DTR | FPL011_OPTION_RTS);
    printf("UART init is ok ==> baudrate:%d, data_bits:%d, parity:%d, stopbits:%d \r\n", 
           uart_format.baudrate, uart_format.data_bits + 5, 
           uart_format.parity, uart_format.stopbits + 1);

    return FT_SUCCESS;
}

static FError FSerialMioDeinit()
{
    FError ret;

    /* deinit mio func */
    ret = FMioFuncDeinit(&mio_ctrl);
    if (ret != FT_SUCCESS)
    {
        FSERIAL_ERROR("MIO deinit error!");
        return ret;
    }

    /* load default format */
    ret = FPl011SetDataFormat(&uart_instance, &uart_format);
    if (ret != FT_SUCCESS)
    {
        FSERIAL_ERROR("Pl011ResetDataFormat() error!");
        return ERR_GENERAL;
    }

    /* stop UART */
    u32 reg = 0U;
    reg &= ~(FPL011_OPTION_UARTEN | FPL011_OPTION_FIFOEN | FPL011_OPTION_RXEN | FPL011_OPTION_TXEN);
    FPl011SetOptions(&uart_instance, reg);

    /* set is_ready to no_ready */
    uart_instance.is_ready = 0U;
    printf("FSerialMioDeinit is ok.\r\n");
    
    return FT_SUCCESS;
}

/* function of serial MIO mode example */
int FSerialMioExample()
{
    int ret = 0;

    /* hardware init */
    ret = FSerialMioInit();
    if (ret != FT_SUCCESS)
    {
        FSERIAL_ERROR("FSerialMioInit() error ==> mio_id: %d", MIO_CONTROLLER_ID);
    }
        
    /* poll mode enable */
    ret = FSerialMioTest();
    if (ret != FT_SUCCESS)
    {
        FSERIAL_ERROR("FSerialNormalTest() error ==> mio_id: %d", MIO_CONTROLLER_ID);
    }

    /* deinit MIO and UART instance */
    ret = FSerialMioDeinit();
    if (ret != FT_SUCCESS)
    {
        FSERIAL_ERROR("FSerialMioDeinit() erroe ==> mio_id: %d", MIO_CONTROLLER_ID);
    }
    /*deinit iopad*/
    FIOMuxDeInit();
    /* print message on example run result */
    if (0 == ret)
    {
        printf("%s@%d: Serial mio example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: Serial mio example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
    }

    return 0;
}