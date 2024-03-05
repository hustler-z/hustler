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
 * FilePath: spim_common.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for spim example common functions 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  * 1.0   zhangyan   2023/4/12   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "fdebug.h"
#include "fsleep.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fparameters.h"
#include "sdkconfig.h"

#include "spim_common.h"
#include "fio_mux.h"

#include "fspim_hw.h" /* include low-level header file for internal probe */

#if defined(FSPIM_VERSION_1)
#include "fgpio.h"
#endif
/************************** Variable Definitions *****************************/
static u8 tx_data[FSPIM_TX_RX_LENGTH] = {0}; /* SPI tx buffer */
static u8 rx_data[FSPIM_TX_RX_LENGTH] = {0}; /* SPI rx buffer */
static FSpimConfig spim_config;
static FSpim spim;
static boolean rx_done = FALSE;
static boolean is_ready = FALSE;

#if defined(FSPIM_VERSION_1)
/* D2000/FT2000-4 使用GPIO引脚控制片选信号 */
static FGpioPinId cs_pin_id =
{
    .ctrl = FGPIO1_ID,
    .port = FGPIO_PORT_A,
    .pin = FGPIO_PIN_5
};

static FGpio gpio;
static FGpioPin cs_pin;

static void FSpimSetupCs(void)
{
    FGpioConfig input_cfg = *FGpioLookupConfig(cs_pin_id.ctrl);
    /*init iomux*/
    FIOMuxInit();
    FIOPadSetSpimMux(FSPIM_TEST_ID);
    (void)FGpioCfgInitialize(&gpio, &input_cfg);
    (void)FGpioPinInitialize(&gpio, &cs_pin, cs_pin_id);
    FGpioSetDirection(&cs_pin, FGPIO_DIR_OUTPUT);

    return;
}

void FSpimCsOnOff(boolean on)
{
    if (on)
    {
        FGpioSetOutputValue(&cs_pin, FGPIO_PIN_LOW);
    }
    else
    {
        FGpioSetOutputValue(&cs_pin, FGPIO_PIN_HIGH);
    }
}

#elif defined(FSPIM_VERSION_2) || defined(TARDIGRADE)
/* E2000 使用FSpimSetChipSelection控制片选信号 */
static void FSpimSetupCs(void)
{
    return;
}

void FSpimCsOnOff(boolean on)
{
    FSpimSetChipSelection(&spim, on);
}
#endif

int FSpimWaitRxDone(int timeout)
{
    while (TRUE != rx_done)
    {
        fsleep_microsec(2);

        if (0 >= --timeout)
        {
            break;
        }
    }

    if (0 >= timeout)
    {
        FSPIM_ERROR("Wait for rx timeout");
        return FSPIM_OPS_TRANS_TIMEOUT;
    }

    return FSPIM_OPS_OK;
}

static void FSpimSendRxDoneEvent(void *instance_p, void *param)
{
    FASSERT(instance_p && param);
    FSpim *spim_p = (FSpim *)instance_p;
    boolean *done_flag = (boolean *)param;

    FSPIM_DEBUG("Spim send Rx done !!!\n");

    *done_flag = TRUE;
    
    return;
}

static void FSpimTxFifoOverflowCallback(void *instance_p, void *param)
{
    FASSERT(instance_p);
    FSpim *spim_p = (FSpim *)instance_p;
    FSPIM_WARN("Spim tx fifo overflow!!!\n");

}

static void FSpimRxFifoUnderflowCallback(void *instance_p, void *param)
{
    FASSERT(instance_p);
    FSpim *spim_p = (FSpim *)instance_p;
    FSPIM_WARN("Spim rx fifo underflow!!!\n");
}

static void FSpimRxFifoOverflowCallback(void *instance_p, void *param)
{
    FASSERT(instance_p);
    FSpim *spim_p = (FSpim *)instance_p;
    FSPIM_WARN("Spim rx fifo overflow!!!\n");
}

static FError FSpimSetupInterrupt(FSpim *instance_p)
{
    FASSERT(instance_p);
    FSpimConfig *config_p = &instance_p->config;
    uintptr base_addr = config_p->base_addr;
    u32 evt;
    u32 mask;
    u32 cpu_id = 0;

    GetCpuId(&cpu_id);
  
    InterruptSetTargetCpus(config_p->irq_num, cpu_id);

    rx_done = FALSE;
    FSpimRegisterIntrruptHandler(instance_p, FSPIM_INTR_EVT_RX_DONE, FSpimSendRxDoneEvent, &rx_done);
    FSpimRegisterIntrruptHandler(instance_p, FSPIM_INTR_EVT_TX_OVERFLOW, FSpimTxFifoOverflowCallback, NULL);
    FSpimRegisterIntrruptHandler(instance_p, FSPIM_INTR_EVT_RX_UNDERFLOW, FSpimRxFifoUnderflowCallback, NULL);
    FSpimRegisterIntrruptHandler(instance_p, FSPIM_INTR_EVT_RX_OVERFLOW, FSpimRxFifoOverflowCallback, NULL);

    InterruptSetPriority(config_p->irq_num, config_p->irq_prority);
    /* register intr callback */
    InterruptInstall(config_p->irq_num, FSpimInterruptHandler, instance_p, NULL);

    /* enable tx fifo overflow / rx overflow / rx full */
    FSpimMaskIrq(base_addr, FSPIM_IMR_ALL_BITS);

    /* enable irq */
    InterruptUmask(config_p->irq_num);

    return FSPIM_SUCCESS;
}

int FSpimOpsDeInit(void)
{
    FSpim *spim_p = &spim;
    FError ret = FSPIM_SUCCESS;
    /*interrupt deinit*/
    InterruptMask(spim_p->config.irq_num);
    /*spim deinit*/
    FSpimDeInitialize(spim_p);
    /*deinit iopad*/
    FIOMuxDeInit();
#if defined(FSPIM_VERSION_1)
if (FT_COMPONENT_IS_READY == cs_pin.is_ready)
{
    FGpioPinDeInitialize(&cs_pin);
}
#endif
    is_ready = FALSE;
    return FSPIM_OPS_OK;
}

int FSpimOpsInit(u32 spi_id, boolean test_mode, boolean en_dma, boolean bit_16)
{
    FError ret = FSPIM_SUCCESS;
    /*deinit the spi first*/
    FSpimOpsDeInit();
    if (TRUE == is_ready)
    {
        return FSPIM_OPS_ALREADY_INIT;
    }

    if (spi_id >= FSPI_NUM)
    {
        return FSPIM_OPS_INVALID_PARA;
    }
    FSpimSetupCs();
    FSpim *spim_p = &spim;
    spim_config = *FSpimLookupConfig(spi_id);
    spim_config.slave_dev_id = FSPIM_SLAVE_DEV_0;
    spim_config.cpha = FSPIM_CPHA_2_EDGE;
    spim_config.cpol = FSPIM_CPOL_LOW;
    spim_config.n_bytes = bit_16 ? FSPIM_2_BYTE : FSPIM_1_BYTE;
    spim_config.en_test = test_mode;
    spim_config.en_dma = en_dma;

    ret = FSpimCfgInitialize(spim_p, &spim_config);
    if (FSPIM_SUCCESS != ret)
    {
        return FSPIM_OPS_INIT_FAILED;
    }

    FSPIM_INFO("Initialization of spi-%d was successful !!!", spi_id);
    is_ready = TRUE;
    return FSPIM_OPS_OK;
}

int FSpimLoopBack(u32 bytes, FSpimOpsLoopBackType type)
{
    u32 loop;
    static const FSpimOpsTxRx handlers[] =
    {
        [FSPIM_OPS_LOOPBACK_POLL_FIFO] = FSpimTxRxPollFifo,
        [FSPIM_OPS_LOOPBACK_INTERRUPT] = FSpimTxRxInterrupt
    };
    static const char *test_types[] =
    {
        "poll-fifo", "interrupt"
    };
    FSpimOpsTxRx tx_rx_handler = handlers[type];
    FError err = FSPIM_SUCCESS;
    FSpim *spim_p = &spim;

    if (FALSE == is_ready)
    {
        FSPIM_ERROR("Spi is not initialized.");
        return FSPIM_OPS_NOT_YET_INIT;
    }

    if (FSPIM_TX_RX_LENGTH < bytes)
    {
        FSPIM_ERROR("transfer buffer is %d bytes < %d as demand", FSPIM_TX_RX_LENGTH, bytes);
        return FSPIM_OPS_TEST_ABORT;
    }

    if (FSPIM_OPS_LOOPBACK_INTERRUPT == type)
    {
        err = FSpimSetupInterrupt(spim_p);
        if (FSPIM_SUCCESS != err)
        {
            return FSPIM_OPS_INIT_FAILED;
        }

    }

    for (loop = 0; loop < bytes; loop++)
    {
        tx_data[loop] = (u8)loop;
        rx_data[loop] = 0;
    }

    if (FSPIM_OPS_OK != tx_rx_handler(&spim, tx_data, bytes, rx_data, bytes))
    {
        return FSPIM_OPS_TEST_ABORT;
    }

    /* Validate that we got all transmitted bytes back in the exact order */
    for (loop = 0; loop < bytes; loop++)
    {
        if (tx_data[loop] != rx_data[loop])
        {
            FSPIM_ERROR("loopback test failed: %d != %d", tx_data[loop], rx_data[loop]);
            return FSPIM_OPS_TEST_FAILED;
        }
    }

    printf("%s test with %d-round success !!!\r\n", test_types[type], bytes);
    return FSPIM_OPS_OK;
}