/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: i2s_dp_rx_example.c
 * Created Date: 2023-07-10 10:25:00
 * Last Modified: 2023-12-21 16:21:30
 * Description:  This file is for the I2S DP RX example function implmentation.
 *
 * Modify History:
 *  Ver       Who             Date         Changes
 * -----   ----------       --------     ---------------------------------
 *  1.0   wangzongqiang    2023/07/23    init
 *  1.1   liqiaozhong      2023/12/19    solve bdl miss intr issue
 */

#include <string.h>
#include <stdio.h>
#include "strto.h"

#include "fparameters.h"
#include "fparameters_comm.h"

#include "fassert.h"
#include "finterrupt.h"
#include "fcache.h"
#include "fmemory_pool.h"
#include "fcpu_info.h"
#include "fio_mux.h"

#include "fddma.h"
#include "fddma_hw.h"
#include "fddma_bdl.h"
#include "fes8336.h"
#include "fi2s.h"
#include "fi2s_hw.h"

#include "fdcdp.h"
#include "fdc.h"
#include "fdp_hw.h"
#include "fdc_hw.h"
#include "fdc_common_hw.h"

#include "i2s_dp_rx_example.h"
/***************** Macros (Inline Functions) Definitions *********************/
static FI2s i2s_ctrl;
static FI2sConfig i2s_config;
static FDdma ddmac;
static FDdmaConfig ddmac_config;
static FDdmaBdlDesc *bdl_desc_list_g;
static FMemp memp;
static u8 memp_buf[SZ_4M] = {0};

/*音频文件存放地址*/
static u32 rx_buf = 0xa0000000;
/***************** functions*************************************************/
static void FI2sReceiveFlowCallback(void *param)
{
    FI2s *instance_p = (FI2s *)param;
    return;
}

static void FI2sReceiveIdleCallback(void *param)
{
    FI2s *instance_p = (FI2s *)param;
    return;
}

static void FI2sIrqSet(FI2s *instance_p)
{
    FASSERT(instance_p != NULL);

    u32 cpu_id;
    u32 index;

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(instance_p->config.irq_num, cpu_id);

    FI2sRegisterInterruptHandler(instance_p, FI2S_INTR_RECEIVE_FO, FI2sReceiveFlowCallback, (void *)instance_p);
    FI2sRegisterInterruptHandler(instance_p, FI2S_INTR_RECEIVE_FE, FI2sReceiveIdleCallback, (void *)instance_p);

    InterruptSetPriority(instance_p->config.irq_num, 0);
    InterruptInstall(instance_p->config.irq_num, FI2sIntrHandler, instance_p, "I2S");
    InterruptUmask(instance_p->config.irq_num);

    return;
}

static void FI2sIrqAllEnable(FI2s *instance_p)
{
    int channel = 0;
    uintptr address = instance_p->config.base_addr;
    u32 id = instance_p->config.instance_id;
    FI2SIntrEventType event_type = FI2S_INTR_TRANS_FO;

    for (event_type = FI2S_INTR_RECEIVE_FO; event_type <= FI2S_INTR_RECEIVE_FE; event_type++)
    {
        FI2sEnableIrq(instance_p, event_type);
    }

    return;
}

static void FDdmaRxDMADone()
{
    /* RX done */
    return;
}

static void FDdmaSetupInterrupt(FDdma *const instance)
{
    FASSERT(instance);

    FDdmaConfig *config = &instance->config;
    uintptr base_addr = config->base_addr;
    u32 cpu_id = 0;

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(config->irq_num, cpu_id);

    InterruptSetPriority(config->irq_num, config->irq_prority);
    /* register intr callback */
    InterruptInstall(config->irq_num,
                     FDdmaIrqHandler,
                     instance,
                     NULL);

    /* enable ddma0 irq */
    InterruptUmask(config->irq_num);

    return;
}

FError FI2sRxEs8336Init(u32 work_mode, u32 word_length)
{
    FError ret = FES8336_SUCCESS;

    FIOMuxInit();
    FIOPadSetI2sMux();

    ret =  FEs8336Init();/* es8336初始化，I2C slave设置*/
    if (FES8336_SUCCESS != ret)
    {
        printf("Es8336 init failed\r\n");
        return ret;
    }

    FEs8336RegsProbe(); /* 寄存器默认值 */
    ret =  FEs8336Startup(work_mode);
    if (FES8336_SUCCESS != ret)
    {
        printf("Set the es8336 work mode failed.\r\n");
        return ret;
    }

    ret = FEs8336SetFormat(word_length, work_mode); /* 设置ES8336工作模式 */
    if (FES8336_SUCCESS != ret)
    {
        printf("Set the es8336 word length failed.\r\n");

    }

    return ret ;
}

FError FI2sRxInit(u32 work_mode, u32 word_length)
{
    FError ret = FI2S_SUCCESS;
    FI2s *instance = &i2s_ctrl;
    u32 i2s_id = FI2S0_ID; /* 默认使用I2S-0 */

    if (work_mode == AUDIO_PCM_STREAM_CAPTURE)
    {
        i2s_id = FI2S0_ID;
    }
    else
    {
        i2s_id = FI2S2_ID;
    }

    memset(instance, 0, sizeof(*instance));
    memset(instance, 0, sizeof(i2s_config));
    instance->data_config.work_mode = work_mode;
    instance->data_config.word_length = word_length;

    i2s_config = *FI2sLookupConfig(i2s_id);
    ret = FI2sCfgInitialize(instance, &i2s_config);
    if (FI2S_SUCCESS != ret)
    {
        printf("Init the i2s failed.\r\n");
        return ret;
    }

    FI2sClkOutDiv(instance); /* 默认16-bits采集 */
    FI2sIrqSet(instance);
    FI2sIrqAllEnable(instance);
    FI2sTxRxEnable(instance, TRUE); /* 模块使能 */

    return ret;
}

FError FI2sRxDdmaInit(u32 id)
{
    FError ret = FI2S_SUCCESS;

    ddmac_config = *FDdmaLookupConfig(id);
    ret = FDdmaCfgInitialize(&ddmac, &ddmac_config);
    if (FI2S_SUCCESS != ret)
    {
        printf("DDMA config initialize failed.\r\n");
        return ret;
    }
    ret = FMempInit(&memp, memp_buf, memp_buf + sizeof(memp_buf)); /* 申请内存 */

    if (FI2S_SUCCESS != ret)
    {
        printf("I2S memp failed.\r\n");
    }

    return ret;
}

FError FI2sDdmaDeviceToMemoryCopy(u32 chan_id, u32 work_mode, uintptr src, fsize_t total_bytes, fsize_t per_buff_len)
{
	FError ret = FI2S_SUCCESS;
	fsize_t bdl_num = total_bytes / per_buff_len;
    FDdmaChanConfig ddma_chan_config;

	FCacheDCacheFlushRange((uintptr)src, total_bytes);
	FDdmaSetupInterrupt(&ddmac);
	for (u32 chan = FDDMA_CHAN_0; chan < FDDMA_NUM_OF_CHAN; chan++) /* 清除中断 */
	{

		FDdmaClearChanIrq(ddmac_config.base_addr, chan, ddmac_config.caps);
		u32 status = FDdmaReadReg(ddmac_config.base_addr, FDDMA_STA_OFFSET);
	}

	FDdmaBdlDesc *bdl_desc_list = FMempMallocAlign(&memp, bdl_num * sizeof(FDdmaBdlDesc), FDDMA_BDL_ADDR_ALIGMENT); /* DDMA描述符首地址需128字节对齐 */
	if ((NULL == bdl_desc_list))
	{
		printf("FDdmaBdlDesc allocate failed.\r\n");
		return FDDMA_ERR_IS_USED;
	}
	memset(bdl_desc_list, 0, bdl_num * sizeof(FDdmaBdlDesc));

	FDdmaBdlDescConfig *bdl_desc_config = FMempCalloc(&memp, 1, bdl_num * sizeof(FDdmaBdlDescConfig));
	if ((NULL == bdl_desc_config))
	{
		printf("FDdmaBdlDescConfig allocate failed.\r\n");
		return FDDMA_ERR_IS_USED;
	}

	/* set BDL descriptors */
	for (fsize_t loop = 0; loop < bdl_num; loop++)
	{
		bdl_desc_config[loop].current_desc_num = loop;
		bdl_desc_config[loop].src_addr = (uintptr)(src + per_buff_len * loop);
		bdl_desc_config[loop].trans_length = per_buff_len;
		bdl_desc_config[loop].ioc = TRUE;
	}

	/* set BDL descriptor list with descriptor configs */
	for (fsize_t loop = 0; loop <  bdl_num; loop++)
	{
		FDdmaBDLSetDesc(bdl_desc_list, &bdl_desc_config[loop]);
	}

	ddma_chan_config.slave_id = 0U,
	ddma_chan_config.req_mode = work_mode;
	ddma_chan_config.ddr_addr = (uintptr)src;
	ddma_chan_config.dev_addr = i2s_config.base_addr + FI2S_RXDMA ;
	ddma_chan_config.trans_len = total_bytes;
	ddma_chan_config.timeout = 0xffff,
	ddma_chan_config.first_desc_addr = (uintptr)bdl_desc_list;
	ddma_chan_config.valid_desc_num = bdl_num;

	FDdmaRegisterChanEvtHandler(&ddmac, chan_id, FDDMA_CHAN_EVT_REQ_DONE, FDdmaRxDMADone, NULL);

	ret = FDdmaChanBdlConfigure(&ddmac, chan_id, &ddma_chan_config);
	if (ret !=  FI2S_SUCCESS)
	{
		printf("DDMA BDL configure failed.\r\n");
		return ret;
	}

	FDdmaChanActive(&ddmac, chan_id);
	FDdmaStart(&ddmac);

	FMempFree(&memp, bdl_desc_config);

	return ret;
}

FError FI2sDdmaDpRxExample(void)
{
    FError ret = FES8336_SUCCESS;
    const u32 ddma_id = FDDMA2_I2S_ID; /* I2S所绑定的DDMA默认是DDMA-2 */
    const u32 channel = 1; /* 接收通道为DDMA通道1 */
    u32 total_byte = 4096 * 16384; /* 接收音频的最大长度，超过此长度将覆盖原有音频，此值为自定义 */
    u32 per_buffer_len = 16384; /* 每一个BDL描述符所负责的数据长度，请保持total_byte是per_buffer_len的倍数 */
    u32 work_mode = AUDIO_PCM_STREAM_CAPTURE; /* capture mode */
    u32 word_length = AUDIO_PCM_STREAM_WORD_LENGTH_16; /* 16-bits word length */

    ret = FI2sRxEs8336Init(work_mode, word_length);
    if (FES8336_SUCCESS != ret)
    {
        printf("Init the es8336 failed.\r\n");
        return ret;
    }

    ret = FI2sRxDdmaInit(ddma_id);
    if (FES8336_SUCCESS != ret)
    {
        printf("Init DDMA-2 failed.\r\n");
        return ret;
    }

    ret = FI2sRxInit(work_mode, word_length);
    if (FI2S_SUCCESS != ret)
    {
        printf("Init the I2S failed.\r\n");
        return ret;
    }

    ret = FI2sDdmaDeviceToMemoryCopy(channel, work_mode, (uintptr)rx_buf, total_byte, per_buffer_len);
    if (FI2S_SUCCESS != ret)
    {
        printf("I2S trans failed.\r\n");
        return ret;
    }

    return ret;
}

FError FI2sDdmaDpRxStopWork(void)
{
    FError ret = FES8336_SUCCESS;

    FDdmaStop(&ddmac);
    FDdmaDeInitialize(&ddmac);

    FI2sStopWork(&i2s_ctrl);
    FEs8336Reset();

    FMempFree(&memp, bdl_desc_list_g);

    printf("DDMA and I2S RX stop.\r\n");

    return ret;
}