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
 * FilePath: gdma_direct_transfer_example.c
 * Date: 2023-03-27 14:54:41
 * Last Modified: 2024-02-19 10:33:58
 * Description:  This file is for gdma BDL mode transfer example function implmentation.
 *
 * Modify History:
 *  Ver      Who         Date         Changes
 * -----   ------      --------     --------------------------------------
 *  1.0  liqiaozhong   2023/3/28    first commit
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
#include "gdma_direct_transfer_example.h"
/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

#define FGDMA_DEBUG_TAG "GDMA-EXAMPLE"
#define FGDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_WARN(format, ...)    FT_DEBUG_PRINT_W(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)

/* user-define */
#define FGDMA_CONTROLLER_ID  FGDMA0_ID
#define FGDMA_WAIT_MODE      FGDMA_WAIT_INTR
#define FGDMA_CHAN_ID        0
#define FGDMA_MAX_DESC_NUM   4 /* max number of BDL descriptors in one list */
#define FGDMA_VALID_DESC_NUM 4 /* actual number of BDL descriptors used */
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
/* system-define */
static FGdma gdma_instance;
static FGdmaConfig gdma_config;
static FGdmaChanConfig chan_config;
static FGdmaBdlDesc bdl_desc_list[FGDMA_MAX_DESC_NUM] __attribute__((aligned(FGDMA_BDL_DESC_ALIGMENT))) = {0};
static FGdmaBdlDescConfig bdl_desc_config[FGDMA_MAX_DESC_NUM];
static u8 ref_buf[1024 * 1024]; /* to completely insure if transfer result is correct or not */
static volatile u32 trans_end_flag = 0U;

/* user-define */
/* desc config data */
static uintptr desc_src_addr[FGDMA_MAX_DESC_NUM] = {0Xa0100000, 0Xa0300000, 0Xa0500000, 0Xa0700000};
static uintptr desc_dst_addr[FGDMA_MAX_DESC_NUM] = {0Xb0100000, 0Xb0300000, 0Xb0500000, 0Xb0700000};
static fsize_t desc_trans_length[FGDMA_MAX_DESC_NUM] = {1024 * 1024, 512 * 1024, 256 * 1024, 128 * 1024};
static FGdmaBurstSize desc_rd_size[FGDMA_MAX_DESC_NUM] = {FGDMA_BURST_SIZE_8_BYTE, FGDMA_BURST_SIZE_4_BYTE, FGDMA_BURST_SIZE_2_BYTE, FGDMA_BURST_SIZE_1_BYTE};
static FGdmaBurstSize desc_wr_size[FGDMA_MAX_DESC_NUM] = {FGDMA_BURST_SIZE_1_BYTE, FGDMA_BURST_SIZE_2_BYTE, FGDMA_BURST_SIZE_4_BYTE, FGDMA_BURST_SIZE_8_BYTE};
static FGdmaBurstLength desc_rd_length[FGDMA_MAX_DESC_NUM] = {FGDMA_BURST_LENGTH_2, FGDMA_BURST_LENGTH_4, FGDMA_BURST_LENGTH_6, FGDMA_BURST_LENGTH_8};
static FGdmaBurstLength desc_wr_length[FGDMA_MAX_DESC_NUM] = {FGDMA_BURST_LENGTH_1, FGDMA_BURST_LENGTH_3, FGDMA_BURST_LENGTH_5, FGDMA_BURST_LENGTH_7};
/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
static void FGdmaChannelTransferEnd(FGdmaChanIrq *const chan_irq_info_p, void *args)
{   
    FGdmaChanIndex channel_id = chan_irq_info_p->chan_id;
    FGDMA_INFO("Channel-%d Transfer completed.", channel_id);
    trans_end_flag |= BIT(channel_id);
}

static void FGdmaOneBDLDescTransferEnd(FGdmaChanIrq *const chan_irq_info_p, void *args) /* one single BDL descriptor transfer end */
{   
    FGdmaChanIndex channel_id = chan_irq_info_p->chan_id;
    FGDMA_INFO("One BDL desciptor in Channel-%d transfer end.", channel_id);
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
int FGdmaBDLTransferExample(void)
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

    /* set BDL descriptor list */
    for (fsize_t loop = 0; loop < FGDMA_MAX_DESC_NUM; loop++) /* set descriptor config first */
    {
        bdl_desc_config[loop] = FGDMA_DEFAULT_DESC_CONFIG();
        bdl_desc_config[loop].current_desc_num = loop;
        bdl_desc_config[loop].src_addr = desc_src_addr[loop];
        bdl_desc_config[loop].dst_addr = desc_dst_addr[loop];
        bdl_desc_config[loop].trans_length = desc_trans_length[loop];
        bdl_desc_config[loop].rd_size = desc_rd_size[loop];
        bdl_desc_config[loop].wr_size = desc_wr_size[loop];
        bdl_desc_config[loop].rd_length = desc_rd_length[loop];
        bdl_desc_config[loop].wr_length = desc_wr_length[loop];
    }

    bdl_desc_config[0].ioc = TRUE; /* when this decspritor transfer end, output an interrupt */
    bdl_desc_config[3].ioc = TRUE;

    for (fsize_t loop = 0; loop < FGDMA_MAX_DESC_NUM; loop++) /* set BDL descriptor list with descriptor configs */
    {
        FGdmaBDLSetDesc(bdl_desc_list, &bdl_desc_config[loop]);
    }

    /* setup GDMA chan */
    chan_config = FGDMA_DEFAULT_CHAN_CONFIG(); /* chan_config here cannot be pointer format */
    chan_config.trans_mode = FGDMA_OPER_BDL;
    chan_config.wait_mode = FGDMA_WAIT_MODE;
    chan_config.first_desc_addr = bdl_desc_list;
    chan_config.valid_desc_num = 4;
    for (fsize_t loop = 0; loop < FGDMA_MAX_DESC_NUM; loop++) /* transfer data set */
    {
        memset((void *)desc_src_addr[loop], 0xaa, desc_trans_length[loop]);
        memset((void *)desc_dst_addr[loop], 0xff, desc_trans_length[loop]);
    }
    memset((void *)ref_buf, 0xaa, 1024 * 1024);

    ret = FGdmaChanConfigure(&gdma_instance, FGDMA_CHAN_ID, &chan_config);
    if (ret != FGDMA_SUCCESS)
    {
        FGDMA_ERROR("Failed to configure chan-%d. ret = 0x%x", FGDMA_CHAN_ID, ret);
        goto exit;
    }

    /* register GIC interrupt */
    if (chan_config.wait_mode == FGDMA_WAIT_INTR)
    {
        if (gdma_instance.config.caps & FGDMA_IRQ2_MASK)
        {
            FGdmaSetupPrivateInterrupt(&gdma_instance, FGDMA_CHAN_ID);
        }
        else if (gdma_config.caps & FGDMA_IRQ1_MASK)
        {
            FGdmaSetupShareInterrupt(&gdma_instance);
        }

        FGdmaChanRegisterEvtHandler(&gdma_instance,
                                    FGDMA_CHAN_ID,
                                    FGDMA_CHAN_EVT_TRANS_END,
                                    FGdmaChannelTransferEnd,
                                    NULL);

        FGdmaChanRegisterEvtHandler(&gdma_instance,
                                    FGDMA_CHAN_ID,
                                    FGDMA_CHAN_EVT_BDL_END,
                                    FGdmaOneBDLDescTransferEnd,
                                    NULL);                       
    }

    /* as a contrast to transfer result, take desciptor-3 for example */
    printf("<GDMA transfer in BDL mode>\r\n");
    printf("before transfer ...\r\n");
    printf("src buf...\r\n");
    FtDumpHexByte((const u8 *)(desc_src_addr[3] + desc_trans_length[3] - 64), MIN(64, desc_trans_length[3]));
    printf("dst buf...\r\n");
    FtDumpHexByte((const u8 *)(desc_dst_addr[3] + desc_trans_length[3] - 64), MIN(64, desc_trans_length[3]));

    /* start GDMA chan transfer */
    FGdmaChanStartTrans(&gdma_instance, FGDMA_CHAN_ID);

    WMB(); /* memory barrier operation */

    /* wait transfer end */
    if (chan_config.wait_mode == FGDMA_WAIT_INTR)
        ret = FGdmaChanIntrWaitTransEnd(&gdma_instance, FGDMA_CHAN_ID);
    else if (chan_config.wait_mode == FGDMA_WAIT_POLL)
        ret = FGdmaChanPollWaitTransEnd(&gdma_instance, FGDMA_CHAN_ID);
    if (ret != FGDMA_SUCCESS)
    {
        FGDMA_ERROR("chan-%d wait transfer end TIMEOUT.", FGDMA_CHAN_ID);
        goto exit;
    }

    /* disable channel after transfer end */
    FGdmaChanAbort(&gdma_instance, FGDMA_CHAN_ID);
    
    WMB(); /* memory barrier operation */
    /* show last 128-byte transfer result, take desciptor-3 for example */
    printf("after transfer ...\r\n");
    printf("src buf...\r\n");
    FtDumpHexByte((const u8 *)(desc_src_addr[3] + desc_trans_length[3] - 64), MIN(64, desc_trans_length[3])); 
    printf("dst buf...\r\n");
    FtDumpHexByte((const u8 *)(desc_dst_addr[3] + desc_trans_length[3] - 64), MIN(64, desc_trans_length[3]));

    /* check if the transfer results are right */
    for (fsize_t loop = 0; loop < FGDMA_MAX_DESC_NUM; loop++)
    {
        if ((0 == memcmp((void *)desc_src_addr[loop], (void *)ref_buf, desc_trans_length[loop])) && 
            (0 == memcmp((void *)desc_dst_addr[loop], (void *)ref_buf, desc_trans_length[loop])))
        {
            FGDMA_INFO("Transfer SUCCESS.");
        }
        else
        {
            FGDMA_ERROR("Descriptor-%d transfer result WRONG.", loop);
            ret = FGDMA_ERR_COMMON;
            goto exit;
        }
    }

exit:
    FGdmaGlobalStop(&gdma_instance);
    FGdmaDeInitialize(&gdma_instance);
    
    /* print message on example run result */
    if (ret == FGDMA_SUCCESS)
    {
        printf("%s@%d: GDMA bdl transfer example [success].\r\n", __func__, __LINE__);
        return 0;
    }
    else
    {
        printf("%s@%d: GDMA bdl transfer example [failure].\r\n", __func__, __LINE__);
        return 1;
    }
}