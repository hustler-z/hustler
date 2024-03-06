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
 * FilePath: gdma_multi_channel_example.c
 * Date: 2023-03-27 14:54:41
 * Last Modified: 2024-02-19 10:34:12
 * Description:  This file is for gdma multi-channel example function implmentation.
 *
 * Modify History:
 *  Ver      Who         Date         Changes
 * -----   ------      --------     --------------------------------------
 *  2.0  liqiaozhong   2023/10/20   adapt to modified driver
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "fio.h"
#include "finterrupt.h"
#include "fsleep.h"
#include "fdebug.h"
#include "fkernel.h"
#include "fcpu_info.h"
#include "fparameters.h"
#include "faarch.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "fgdma.h"
#include "fgdma_hw.h"
#include "gdma_multi_channel_example.h"
/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

#define FGDMA_DEBUG_TAG "GDMA-EXAMPLE"
#define FGDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_WARN(format, ...)    FT_DEBUG_PRINT_W(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)

/* user-define */
#define FGDMA_CONTROLLER_ID    FGDMA0_ID
#define FGDMA_WAIT_MODE        FGDMA_WAIT_INTR
#define FGDMA_CHAN_ID_0        0
#define FGDMA_CHAN_SRC_ADDR_0  0Xa0100000 /* should be aligned with both read and write burst size, defalut: 16-byte */
#define FGDMA_CHAN_DST_ADDR_0  0Xa0200000 /* should be aligned with both read and write burst size */
#define FGDMA_CHAN_TRANS_LEN_0 1024       /* should be aligned with both read and write burst size */
#define FGDMA_CHAN_ID_1        1
#define FGDMA_CHAN_SRC_ADDR_1  0Xa0300000
#define FGDMA_CHAN_DST_ADDR_1  0Xa0400000
#define FGDMA_CHAN_TRANS_LEN_1 1024
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
/* system-define */
static FGdma gdma_instance;
static FGdmaConfig gdma_config;
static FGdmaChanConfig chan_config[FGDMA_NUM_OF_CHAN];
static u32 trans_end_flag = 0U;
/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
static void FGdmaChannelTransferEnd(FGdmaChanIrq *const chan_irq_info_p, void *args)
{   
    FGdmaChanIndex channel_id = chan_irq_info_p->chan_id;
    FGDMA_INFO("Channel: %d Transfer completed.\n", channel_id);
    trans_end_flag |= BIT(channel_id);
}

/* for GIC register */
/* if GDMA chennels share the same intr num of the controller */
static void FGdmaSetupShareInterrupt(FGdma *const gdma_instance_p)
{
    FASSERT(gdma_instance_p);
    FASSERT(gdma_instance_p->gdma_ready);

    u32 cpu_id = 0;
    GetCpuId(&cpu_id);
    FGDMA_INFO("cpu id is %d", cpu_id);

    FGdmaConfig gdma_config_p = gdma_instance_p->config;

    InterruptSetTargetCpus(gdma_config_p.irq_num[0], cpu_id);
    InterruptSetPriority(gdma_config_p.irq_num[0], gdma_config_p.irq_prority);

    /* register intr callback */
    InterruptInstall(gdma_config_p.irq_num[0],
                     FGdmaIrqHandler,
                     gdma_instance_p,
                     NULL);

    /* enable gdma irq */
    InterruptUmask(gdma_config_p.irq_num[0]);
    FGDMA_INFO("GDMA GIC interrupt register FINISH.");

    return;
}

/* if GDMA chennels have their own intr nums */
static void FGdmaSetupPrivateInterrupt(FGdma *const gdma_instance_p, FGdmaChanIndex channel_id)
{
    FASSERT(gdma_instance_p);
    FASSERT(gdma_instance_p->gdma_ready);
    FASSERT(channel_id < FGDMA_NUM_OF_CHAN);

    u32 cpu_id = 0;
    FGdmaConfig gdma_config_p = gdma_instance_p->config;

    InterruptSetTargetCpus(gdma_config_p.irq_num[channel_id], cpu_id);
    InterruptSetPriority(gdma_config_p.irq_num[channel_id], gdma_config_p.irq_prority);

    /* register intr callback */
    InterruptInstall(gdma_config_p.irq_num[channel_id],
                     FGdmaIrqHandlerPrivateChannel,
                     &gdma_instance_p->chan_irq_info[channel_id],
                     NULL);

    /* enable gdma irq */
    InterruptUmask(gdma_config_p.irq_num[channel_id]);
    FGDMA_INFO("GDMA GIC interrupt register FINISH.");

    return;
}

static FError FGdmaChanPollWaitTransEnd(FGdma *const gdma_instance_p, FGdmaChanIndex channel_id)
{
    int timeout = 1000000000;
    u32 chan_status;

    while (TRUE) /* wait in poll */
    {
        chan_status = FGdmaReadChanStatus(gdma_instance.config.base_addr, channel_id);
        if (chan_status & FGDMA_CHX_INT_STATE_TRANS_END) /* wait transfer end status */
        {
            break;
        }
        if (--timeout <= 0)
        {
            FGDMA_ERROR("Wait for a GDMA transfer TIMEOUT, DMA_Cx_STATE(0x2C): 0x%x.", chan_status);
            return FGDMA_ERR_COMMON;
        }
        __asm__ __volatile__("NOP");
    }
    FGDMA_INFO("Transfer FINISH.");

    return FGDMA_SUCCESS;
}

static FError FGdmaChanIntrWaitTransEnd(FGdma *const gdma_instance_p, FGdmaChanIndex channel_id)
{
    int timeout = 1000000000;
    u32 chan_status;

    while (TRUE) /* wait trans_end_intr until timeout */
    {
        if (trans_end_flag & BIT(channel_id))
        {
            trans_end_flag &= ~BIT(channel_id);
            break;
        }
        if (--timeout <= 0)
        {
            FGDMA_ERROR("Wait for a GDMA transfer TIMEOUT, DMA_Cx_STATE(0x2C): 0x%x.", chan_status);
            return FGDMA_ERR_COMMON;
        }
        __asm__ __volatile__("NOP");
    }
    FGDMA_INFO("Transfer FINISH.");

    return FGDMA_SUCCESS;
}

/* example function used in cmd */
int FGdmaMultiChannelExample(void)
{
    FError ret = FGDMA_SUCCESS;

    /* setup GDMA controller */
    gdma_config = *FGdmaLookupConfig(FGDMA_CONTROLLER_ID);
    gdma_config.wait_mode = FGDMA_WAIT_MODE;
    ret = FGdmaCfgInitialize(&gdma_instance, &gdma_config);
    if (ret != FGDMA_SUCCESS)
    {
        FGDMA_ERROR("Failed to initialize gdma-%d. ret = 0x%x", FGDMA_CONTROLLER_ID, ret);
        goto exit;
    }

    /* setup GDMA chan-0 */
    chan_config[0] = FGDMA_DEFAULT_CHAN_CONFIG(); /* chan_config[0] here cannot be pointer format */
    chan_config[0].trans_mode = FGDMA_OPER_DIRECT;
    chan_config[0].wait_mode = FGDMA_WAIT_MODE;
    chan_config[0].src_addr = FGDMA_CHAN_SRC_ADDR_0;
    chan_config[0].dst_addr = FGDMA_CHAN_DST_ADDR_0;
    chan_config[0].trans_length = FGDMA_CHAN_TRANS_LEN_0;

    memset((void *)chan_config[0].src_addr, 0xaa, chan_config[0].trans_length); /* transfer data set */
    memset((void *)chan_config[0].dst_addr, 0xff, chan_config[0].trans_length);
    
    ret = FGdmaChanConfigure(&gdma_instance, FGDMA_CHAN_ID_0, &chan_config[0]);
    if (ret != FGDMA_SUCCESS)
    {
        FGDMA_ERROR("FAILURE to configure chan-%d. ret = 0x%x", FGDMA_CHAN_ID_0, ret);
        goto exit;
    }

    /* register GIC interrupt */
    if (chan_config[0].wait_mode == FGDMA_WAIT_INTR)
    {
        if (gdma_instance.config.caps & FGDMA_IRQ2_MASK)
        {
            FGdmaSetupPrivateInterrupt(&gdma_instance, FGDMA_CHAN_ID_0);
        }
        else if (gdma_config.caps & FGDMA_IRQ1_MASK)
        {
            FGdmaSetupShareInterrupt(&gdma_instance);
        }
        FGdmaChanRegisterEvtHandler(&gdma_instance,
                                    FGDMA_CHAN_ID_0,
                                    FGDMA_CHAN_EVT_TRANS_END,
                                    FGdmaChannelTransferEnd,
                                    NULL);
    }

    /* setup GDMA chan-1 */
    chan_config[1] = FGDMA_DEFAULT_CHAN_CONFIG(); /* chan_config[1] here cannot be pointer format */
    chan_config[1].trans_mode = FGDMA_OPER_DIRECT;
    chan_config[1].wait_mode = FGDMA_WAIT_MODE;
    chan_config[1].src_addr = FGDMA_CHAN_SRC_ADDR_1;
    chan_config[1].dst_addr = FGDMA_CHAN_DST_ADDR_1;
    chan_config[1].trans_length = FGDMA_CHAN_TRANS_LEN_1;

    memset((void *)chan_config[1].src_addr, 0xaa, chan_config[1].trans_length); /* transfer data set */
    memset((void *)chan_config[1].dst_addr, 0xff, chan_config[1].trans_length);
    
    ret = FGdmaChanConfigure(&gdma_instance, FGDMA_CHAN_ID_1, &chan_config[1]);
    if (ret != FGDMA_SUCCESS)
    {
        FGDMA_ERROR("FAILURE to configure chan-%d. ret = 0x%x", FGDMA_CHAN_ID_1, ret);
        goto exit;
    }

    /* register GIC interrupt */
    if (chan_config[1].wait_mode == FGDMA_WAIT_INTR)
    {
        if (gdma_instance.config.caps & FGDMA_IRQ2_MASK)
        {
            FGdmaSetupPrivateInterrupt(&gdma_instance, FGDMA_CHAN_ID_1);
        }
        else if (gdma_config.caps & FGDMA_IRQ1_MASK)
        {
            FGdmaSetupShareInterrupt(&gdma_instance);
        }
        FGdmaChanRegisterEvtHandler(&gdma_instance,
                                    FGDMA_CHAN_ID_1,
                                    FGDMA_CHAN_EVT_TRANS_END,
                                    FGdmaChannelTransferEnd,
                                    NULL);
    }

    /* start GDMA transfer, all configured chans will start transfer */
    FGdmaGlobalStartTrans(&gdma_instance);

    for (fsize_t chan_id = 0; chan_id < FGDMA_NUM_OF_CHAN; chan_id++)
    {
        if (!(gdma_instance.chan_ready_flag & BIT(chan_id)))
            break;

        /* wait transfer end */
        if (chan_config[chan_id].wait_mode == FGDMA_WAIT_INTR)
            ret = FGdmaChanIntrWaitTransEnd(&gdma_instance, chan_id);
        else if (chan_config[chan_id].wait_mode == FGDMA_WAIT_POLL)
            ret = FGdmaChanPollWaitTransEnd(&gdma_instance, chan_id);
        if (ret != FGDMA_SUCCESS)
        {
            FGDMA_ERROR("chan-%d wait transfer end TIMEOUT.", chan_id);
            ret = FGDMA_ERR_COMMON;
            break;
        }

        /* disable channel after transfer end */
        FGdmaChanAbort(&gdma_instance, chan_id);

        /* check if the transfer result is right */
        DSB(); /* memory barrier operation */
        if (0 == memcmp((void *)chan_config[chan_id].src_addr, (void *)chan_config[chan_id].dst_addr, chan_config[chan_id].trans_length))
        {
            printf("chan-%d transfer SUCCESS.\r\n", chan_id);
        }
        else
        {
            FGDMA_ERROR("chan-%d transfer result WRONG.", chan_id);
            ret = FGDMA_ERR_COMMON;
            break;
        }
    }

exit:
    FGdmaGlobalStop(&gdma_instance);
    FGdmaDeInitialize(&gdma_instance);
    
    /* print message on example run result */
    if (ret == FGDMA_SUCCESS)
    {
        printf("%s@%d: GDMA muti-channel example [success].\r\n", __func__, __LINE__);
        return 0;
    }
    else
    {
        printf("%s@%d: GDMA muti-channel example [failure].\r\n", __func__, __LINE__);
        return 1;
    }
}