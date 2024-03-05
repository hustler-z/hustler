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
 * FilePath: serial_ddma_example.c
 * Date: 2023-04-12 14:53:42
 * LastEditTime: 2023-06-21 17:57:24
 * Description:  This file is for serial transfer under control of DDMA example function implementation.
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

#include "serial_ddma_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
/* user-defined parameters */
#define FDDMA_TX_RX_BUF_LEN 1024 /* change timeout when face large data */
#define UART_CONTROLLER_ID  FUART2_ID
#define DDMA_CONTROLLER_ID  FDDMA0_ID
#define TX_CHAN_ID          0
#define RX_CHAN_ID          1
#define TX_SLAVE_ID         FDDMA0_UART2_TX_SLAVE_ID
#define RX_SLAVE_ID         FDDMA0_UART2_RX_SLAVE_ID

/* DDMA trans data */
static const u8 tx_send_data[] = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
static u8 rx_recv_data[FDDMA_TX_RX_BUF_LEN] = {0};
static const fsize_t trans_size = sizeof(tx_send_data) - 1; /* ignore string end identifier */

/* FT-defined print form */
#define FSERIAL_DEBUG_TAG "FSERIAL_EXAMPLE"
#define FSERIAL_ERROR(format, ...) FT_DEBUG_PRINT_E(FSERIAL_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSERIAL_WARRN(format, ...) FT_DEBUG_PRINT_W(FSERIAL_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSERIAL_INFO(format, ...) FT_DEBUG_PRINT_I(FSERIAL_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSERIAL_DEBUG(format, ...) FT_DEBUG_PRINT_D(FSERIAL_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Constant Definitions *****************************/
static u32 tx_buf[FDDMA_TX_RX_BUF_LEN] __attribute__((aligned(FDDMA_DDR_ADDR_ALIGMENT))) = {0};
static u32 rx_buf[FDDMA_TX_RX_BUF_LEN] __attribute__((aligned(FDDMA_DDR_ADDR_ALIGMENT))) = {0};
static FPl011Config uart_config;
static FPl011 uart_instance;
static FDdmaConfig ddma_config;
static FDdma ddma_instance;
static FDdmaChanConfig tx_chan_config;
static FDdmaChanConfig rx_chan_config;
static FPl011Format uart_format =
{
    .baudrate = FPL011_BAUDRATE,
    .data_bits = FPL011_FORMAT_WORDLENGTH_8BIT,
    .parity = FPL011_FORMAT_NO_PARITY,
    .stopbits = FPL011_FORMAT_1_STOP_BIT
};

/* the following counters are used to determine if the entire buffer has been sent and received */
static volatile int total_received_count;
static volatile int total_send_count;
static volatile int total_error_count;
static volatile boolean rx_dma_done = FALSE;
static volatile boolean tx_dma_done = FALSE;
/************************** Function Prototypes ******************************/

/******************************* Functions ***********************************/
static void FDdmaPl011TxDMADone()
{
    printf("******tx done****** \r\n");
    tx_dma_done = TRUE;

    return;
}

static void FDdmaPl011RxDMADone()
{
    printf("******rx done****** \r\n");
    rx_dma_done = TRUE;

    return;
}

static void FU8buff2U32buff(u32 *buff1, const u8 *buff2, u32 size)
{
    FASSERT(buff1 && buff2);
    FASSERT(size < (FDDMA_TX_RX_BUF_LEN + 1));

    for (size_t i = 0; i < size; i++)
    {
        buff1[i] = buff2[i];
    }

    return;
}

static void FU32buff2U8buff(u8 *buff1, u32 *buff2, u32 size)
{
    FASSERT(buff1 && buff2);
    FASSERT(size < (FDDMA_TX_RX_BUF_LEN + 1));

    for (size_t i = 0; i < size; i++)
    {
        buff1[i] = buff2[i];
    }

    return;
}

static FError FSerialDdmaTest()
{
    FError ret = FT_SUCCESS;
    int timeout = 1000;

    if (FDdmaIsChanRunning(ddma_instance.config.base_addr, TX_CHAN_ID) ||
        FDdmaIsChanRunning(ddma_instance.config.base_addr, RX_CHAN_ID))
    {
        FSERIAL_ERROR("RX or TX chan is already running!");
        return FDDMA_ERR_IS_USED;
    }
    
    /* enable tx channel and ready for use */
    FDdmaChanActive(&ddma_instance, TX_CHAN_ID);

    /* enable rx channel and ready for use */
    FDdmaChanActive(&ddma_instance, RX_CHAN_ID);

    FDdmaStart(&ddma_instance);

    /* wait in interrupt mode until tx and rx all finished or timeout */
    while (FALSE == tx_dma_done)
    {
        if (--timeout <= 0)
        {
            break;
        }

        fsleep_millisec(1);
    }
    if (0 >= timeout)
    {
        FSERIAL_ERROR("Wait DDMA TX end timeout %d!", timeout);
        FDdmaDumpRegisters(ddma_instance.config.base_addr);
        FDdmaDumpChanRegisters(ddma_instance.config.base_addr, TX_CHAN_ID);
        FDdmaDumpChanRegisters(ddma_instance.config.base_addr, RX_CHAN_ID);
        return FDDMA_ERR_WAIT_TIMEOUT;
    }

    printf("Wait for data comming in, put at least %d bytes data to trigger DDMA RX end intr... \r\n", trans_size);
    while (FALSE == rx_dma_done)
    {
        fsleep_millisec(1);
    }

    return FT_SUCCESS;
}

static FError FSerialDdmaInit()
{
    FError ret;

    /* UART init */
    uart_config = *FPl011LookupConfig(UART_CONTROLLER_ID);

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
    FPl011SetInterruptMask(&uart_instance, ~FPL011IMSC_ALLM); /*close all intr*/
    FPl011SetOptions(&uart_instance, 
                     FPL011_OPTION_UARTEN | FPL011_OPTION_RXEN | 
                     FPL011_OPTION_TXEN | FPL011_OPTION_TXDMAEN | 
                     FPL011_OPTION_RXDMAEN);
    FPl011SetOperMode(&uart_instance, FPL011_OPER_MODE_NORMAL); /* use normal mode */

    printf("UART init is ok ==> baudrate:%d, data_bits:%d, parity:%d, stopbits:%d \r\n", 
           uart_format.baudrate, uart_format.data_bits + 5, 
           uart_format.parity, uart_format.stopbits + 1);

    /* DDMA init */
    ddma_config = *FDdmaLookupConfig(DDMA_CONTROLLER_ID);
    ddma_config.irq_prority = IRQ_PRIORITY_VALUE_0;
    ret = FDdmaCfgInitialize(&ddma_instance, &ddma_config);
    if (FDDMA_SUCCESS != ret)
    {
        FSERIAL_ERROR("Init ddma-%d failed, err: 0x%x", DDMA_CONTROLLER_ID, ret);
        return ERR_GENERAL;
    }
    
    /* DDMA chan allocate */
    /* TX */
    tx_chan_config.slave_id = TX_SLAVE_ID;
    tx_chan_config.ddr_addr = (uintptr)tx_buf;
    tx_chan_config.dev_addr = uart_instance.config.base_address + FPL011DR_OFFSET; /* pl011 fifo */
    tx_chan_config.req_mode = FDDMA_CHAN_REQ_TX;
    tx_chan_config.timeout = 0xffff;
    tx_chan_config.trans_len = trans_size * 4; /* u8 data, but u32 register */

    printf("tx channel: %d, slave id: %d, ddr: 0x%x, dev: 0x%x, req mode: %s, trans len: %d \r\n",
           TX_CHAN_ID,
           tx_chan_config.slave_id,
           tx_chan_config.ddr_addr,
           tx_chan_config.dev_addr,
           (FDDMA_CHAN_REQ_TX == tx_chan_config.req_mode) ? "mem => dev" : "dev => mem",
           tx_chan_config.trans_len);
    
    ret = FDdmaChanConfigure(&ddma_instance, TX_CHAN_ID, &tx_chan_config);
    if (FDDMA_SUCCESS != ret)
    {
        FSERIAL_ERROR("TX channel allocate failed: 0x%x.", ret);
        return ret;
    }

    /* RX */
    rx_chan_config.slave_id = RX_SLAVE_ID;
    rx_chan_config.ddr_addr = (uintptr)rx_buf;
    rx_chan_config.dev_addr = uart_instance.config.base_address + FPL011DR_OFFSET; /* pl011 fifo */
    rx_chan_config.req_mode = FDDMA_CHAN_REQ_RX;
    rx_chan_config.timeout = 0xffff;
    rx_chan_config.trans_len = trans_size * 4;

    printf("RX channel: %d, slave id: %d, ddr: 0x%x, dev: 0x%x, req mode: %s, trans len: %d \r\n",
           RX_CHAN_ID,
           rx_chan_config.slave_id,
           rx_chan_config.ddr_addr,
           rx_chan_config.dev_addr,
           (FDDMA_CHAN_REQ_TX == rx_chan_config.req_mode) ? "mem => dev" : "dev => mem",
           rx_chan_config.trans_len);
    
    ret = FDdmaChanConfigure(&ddma_instance, RX_CHAN_ID, &rx_chan_config);
    if (FDDMA_SUCCESS != ret)
    {
        FSERIAL_ERROR("RX channel allocate failed: 0x%x.", ret);
        return ret;
    }

    tx_dma_done = FALSE;
    rx_dma_done = FALSE;

    /* DDMA interrupt set */
    FDdmaRegisterChanEvtHandler(&ddma_instance, TX_CHAN_ID, FDDMA_CHAN_EVT_REQ_DONE, FDdmaPl011TxDMADone, NULL);
    FDdmaRegisterChanEvtHandler(&ddma_instance, RX_CHAN_ID, FDDMA_CHAN_EVT_REQ_DONE, FDdmaPl011RxDMADone, NULL);

    u32 cpu_id = 0;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(ddma_instance.config.irq_num, cpu_id);
    InterruptSetPriority(ddma_instance.config.irq_num, ddma_instance.config.irq_prority);
    InterruptInstall(ddma_instance.config.irq_num, FDdmaIrqHandler, &ddma_instance, NULL); /* register intr callback */
    InterruptUmask(ddma_instance.config.irq_num); /* enable ddma0 irq */

    printf("DDMA interrupt setup done! \r\n");

    return FT_SUCCESS;
}

static FError FSerialDdmaDeinit()
{
    FError ret;

    /* load default format */
    ret = FPl011SetDataFormat(&uart_instance, &uart_format);
    if (ret != FT_SUCCESS)
    {
        FSERIAL_ERROR("Pl011ResetDataFormat() error!");
        return ERR_GENERAL;
    }

    /* stop UART */
    FPl011SetInterruptMask(&uart_instance, ~FPL011IMSC_ALLM); /*close all intr*/
    
    u32 reg = 0U;
    reg &= ~(FPL011_OPTION_UARTEN | FPL011_OPTION_RXEN | 
             FPL011_OPTION_TXEN | FPL011_OPTION_TXDMAEN | 
             FPL011_OPTION_RXDMAEN);
    FPl011SetOptions(&uart_instance, reg);

    uart_instance.is_ready = 0U; /*set is_ready to no_ready*/
    printf("UART deinit is ok. \r\n");

    /* DDMA deinit */
    InterruptMask(ddma_instance.config.irq_num);

    FDdmaStop(&ddma_instance);

    ret = FDdmaChanDeactive(&ddma_instance, TX_CHAN_ID);
    ret = FDdmaChanDeconfigure(&ddma_instance, TX_CHAN_ID);
    if (FDDMA_SUCCESS != ret)
    {
        FSERIAL_ERROR("TX channel revoke failed: 0x%x", ret);
        return FDDMA_ERR_IS_USED;
    }

    ret = FDdmaChanDeactive(&ddma_instance, RX_CHAN_ID);
    ret = FDdmaChanDeconfigure(&ddma_instance, RX_CHAN_ID);
    if (FDDMA_SUCCESS != ret)
    {
        FSERIAL_ERROR("RX channel revoke failed: 0x%x", ret);
        return FDDMA_ERR_IS_USED;
    }

    FDdmaDeInitialize(&ddma_instance);
    
    return FT_SUCCESS;
}

/* function of serial under control of DDMA example */
int FSerialDdmaExample()
{
    int ret = 0;

    /* hardware init */
    ret = FSerialDdmaInit();
    if (ret != FT_SUCCESS)
    {
        FSERIAL_ERROR("FSerialFlowInit() error ==> UART id: %d", UART_CONTROLLER_ID);
    }

    memset(rx_buf, 0, sizeof(rx_buf));
    FU8buff2U32buff(tx_buf, tx_send_data, trans_size); /* DDMA only support 32-bit data, but UART tx and rx registers are 8-bit */

    printf("Uart DDMA test \r\n");
    printf("before transfer....... \r\n");
    printf("tx buf ===> \r\n");
    FtDumpHexByte((u8 *)tx_send_data, trans_size);
    printf("rx buf <=== \r\n");
    FtDumpHexByte((u8 *)rx_recv_data, trans_size);

    /* DDMA enable */
    ret = FSerialDdmaTest();
    if (ret != FT_SUCCESS)
    {
        FSERIAL_ERROR("FSerialFlowTest() error ==> UART id: %d", UART_CONTROLLER_ID);
    }
    
    FU32buff2U8buff(rx_recv_data, rx_buf, trans_size);
    printf("After test ..... \r\n");
    printf("tx buf ===> \r\n");
    printf("tx buf ===> \r\n");
    FtDumpHexByte((u8 *)tx_send_data, trans_size);
    printf("rx buf <=== \r\n");
    FtDumpHexByte((u8 *)rx_recv_data, trans_size);


    /* deinit MIO and UART instance */
    ret = FSerialDdmaDeinit();
    if (ret != FT_SUCCESS)
    {
        FSERIAL_ERROR("FSerialDdmaDeinit() erroe ==> UART id: %d", UART_CONTROLLER_ID);
    }

    /* print message on example run result */
    if (0 == ret)
    {
        printf("%s@%d: Serial ddma example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: Serial ddma example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
    }

    return 0;
}
