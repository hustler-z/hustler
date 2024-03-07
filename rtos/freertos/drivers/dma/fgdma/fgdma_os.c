/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: fgdma_os.c
 * Date: 2022-07-20 10:54:31
 * LastEditTime: 2022-07-20 10:54:31
 * Description:  This file is for required function implementations of gdma driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0    zhugengyu     2022/7/27     init commit
 *  2.0    liqiaozhong   2023/11/10    synchronous update with standalone sdk
 */
/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "finterrupt.h"
#include "fparameters.h"
#include "fdebug.h"
#include "fsleep.h"
#include "fcpu_info.h"

#include "fgdma.h"
#include "fgdma_hw.h"

#include "fgdma_os.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FFreeRTOSGdma gdma_instance[FGDMA_INSTANCE_NUM];
static FGdmaBdlDesc *channel_first_desc_addr[FFREERTOS_GDMA_NUM_OF_CHAN];
/***************** Macros (Inline Functions) Definitions *********************/
#define FGDMA_DEBUG_TAG "GDMA-OS"
#define FGDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_WARN(format, ...)    FT_DEBUG_PRINT_W(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/

/*****************************************************************************/
/* GDMA transfer end irq response function */
static void GdmaChannelTransferEnd(FGdmaChanIrq *const chan_irq_info_p, void *args)
{   
    FASSERT(chan_irq_info_p);

    FGdmaChanIndex channel_id = chan_irq_info_p->chan_id;
    uint32_t ctrl_id = chan_irq_info_p->gdma_instance->config.instance_id;
    FreeRTOSGdmaChanEvtHandler usr_evt_handler = gdma_instance[ctrl_id].os_evt_handler[channel_id][FFREERTOS_GDMA_CHAN_EVT_TRANS_END];
    void *usr_evt_handler_arg = gdma_instance[ctrl_id].os_evt_handler_arg[channel_id][FFREERTOS_GDMA_CHAN_EVT_TRANS_END];

    if (usr_evt_handler != NULL)
    {
        usr_evt_handler(channel_id, usr_evt_handler_arg);
    }
    FGDMA_INFO("Channel: %d Transfer completed.\n", channel_id);

    return;
}

static inline FError FGdmaOsTakeSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "Locker not exists.");

    if (pdFALSE == xSemaphoreTake(locker, portMAX_DELAY))
    {
        FGDMA_ERROR("Failed to take locker!!!");
        return FFREERTOS_GDMA_SEMA_ERR;
    }

    return FFREERTOS_GDMA_OK;
}

static inline void FGdmaOsGiveSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "Locker not exists.");
    if (pdFALSE == xSemaphoreGive(locker))
    {
        FGDMA_ERROR("Failed to give locker!!!");
    }

    return;
}

/* for GIC register */
/* if GDMA chennels share the same intr num of the controller */
static void FGdmaSetupShareInterrupt(FGdma *const gdma_ctrl_p)
{
    FASSERT(gdma_ctrl_p);
    FASSERT(gdma_ctrl_p->gdma_ready);

    uint32_t cpu_id = 0;
    GetCpuId(&cpu_id);
    FGDMA_INFO("cpu id is %d", cpu_id);

    FGdmaConfig *gdma_config_p = &gdma_ctrl_p->config;

    InterruptSetTargetCpus(gdma_config_p->irq_num[0], cpu_id);
    InterruptSetPriority(gdma_config_p->irq_num[0], gdma_config_p->irq_prority);

    /* register intr callback */
    InterruptInstall(gdma_config_p->irq_num[0],
                     FGdmaIrqHandler,
                     gdma_ctrl_p,
                     NULL);

    /* enable gdma irq */
    InterruptUmask(gdma_config_p->irq_num[0]);
    FGDMA_INFO("GDMA GIC interrupt register FINISH.");

    return;
}

/* if GDMA chennels have their own intr nums */
static void FGdmaSetupPrivateInterrupt(FGdma *const gdma_ctrl_p, FGdmaChanIndex channel_id)
{
    FASSERT(gdma_ctrl_p);
    FASSERT(gdma_ctrl_p->gdma_ready);
    FASSERT(channel_id < FFREERTOS_GDMA_NUM_OF_CHAN);

    uint32_t cpu_id = 0;
    FGdmaConfig *gdma_config_p = &gdma_ctrl_p->config;

    InterruptSetTargetCpus(gdma_config_p->irq_num[channel_id], cpu_id);
    InterruptSetPriority(gdma_config_p->irq_num[channel_id], gdma_config_p->irq_prority);

    /* register intr callback */
    InterruptInstall(gdma_config_p->irq_num[channel_id],
                     FGdmaIrqHandlerPrivateChannel,
                     &gdma_ctrl_p->chan_irq_info[channel_id],
                     NULL);

    /* enable gdma irq */
    InterruptUmask(gdma_config_p->irq_num[channel_id]);
    FGDMA_INFO("GDMA GIC interrupt register FINISH.");

    return;
}

void FFreeRTOSGdmaChanRegisterEvtHandler(FFreeRTOSGdma *const instance_p,
                                         uint32_t channel_id,
                                         FFreeRTOSGdmaChanEvtType evt,
                                         FreeRTOSGdmaChanEvtHandler os_evt_handler, 
                                         void *os_evt_handler_arg)
{
    FASSERT(instance_p);
    FASSERT(channel_id < FFREERTOS_GDMA_NUM_OF_CHAN);
    FASSERT(evt < FFREERTOS_GDMA_CHAN_NUM_OF_EVT);
    FASSERT(os_evt_handler);

    instance_p->os_evt_handler[channel_id][evt] = os_evt_handler;
    instance_p->os_evt_handler_arg[channel_id][evt] =os_evt_handler_arg;

    return;
}

/* GDMA ctrl(controller) init function */
FFreeRTOSGdma *FFreeRTOSGdmaInit(uint32_t ctrl_id)
{
    FASSERT_MSG(ctrl_id < FGDMA_INSTANCE_NUM, "Invalid GDMA controller id.");

    FFreeRTOSGdma *instance_p = &gdma_instance[ctrl_id];
    FGdma *ctrl_p = &instance_p->ctrl;
    FGdmaConfig ctrl_config;
    FMemp *memp = &instance_p->memp;
    void *memp_buf_beg = (void *)instance_p->memp_buf;
    void *memp_buf_end = (void *)instance_p->memp_buf + sizeof(instance_p->memp_buf);
    FError err = FFREERTOS_GDMA_OK;

    taskENTER_CRITICAL(); /* forbidden scheduler during init */

    ctrl_config = *FGdmaLookupConfig(ctrl_id);
    ctrl_config.irq_prority = FFREERTOS_GDMA_IRQ_PRIORITY;
    err = FGdmaCfgInitialize(ctrl_p, &ctrl_config);
    if (FGDMA_SUCCESS != err)
    {
        FGDMA_ERROR("Init gdma-%d failed, err = 0x%x", ctrl_id, err);
        goto err_exit;
    }

    if (ctrl_p->config.caps & FGDMA_IRQ1_MASK)
    {
        FGdmaSetupShareInterrupt(ctrl_p);
    }

    /* init memory pool */
    err = FMempInit(memp, memp_buf_beg, memp_buf_end);
    if (FMEMP_SUCCESS != err)
    {
        FGDMA_ERROR("Init memp failed, 0x%x", err);
        goto err_exit;
    }

    FASSERT_MSG(NULL == instance_p->locker, "Mutex instance_p->locker aready exists.");
    FASSERT_MSG((instance_p->locker = xSemaphoreCreateMutex()) != NULL, "Create mutex instance_p->locker failed.");

err_exit:
    taskEXIT_CRITICAL(); /* allow schedule after init */
    return (FT_SUCCESS == err) ? instance_p : NULL; /* exit with NULL if failed */
}

/* GDMA instance deinit function */
FError FFreeRTOSGdmaDeInit(FFreeRTOSGdma *const instance_p)
{
    FASSERT(instance_p);
    FGdma *ctrl_p = &instance_p->ctrl;
    FGdmaConfig *config = &ctrl_p->config;
    FMemp *memp = &instance_p->memp;
    FError err = FFREERTOS_GDMA_OK;

    taskENTER_CRITICAL();  /* no schedule when deinit */

    err = FGdmaGlobalStop(ctrl_p);
    if (FMEMP_SUCCESS != err)
    {
        FGDMA_ERROR("FGdmaGlobalStop failed, err = 0x%x.", err);
        taskEXIT_CRITICAL();
        return err;
    }
    FMempDeinit(memp);
    FGdmaDeInitialize(ctrl_p);

    vSemaphoreDelete(instance_p->locker);
    instance_p->locker = NULL;

    taskEXIT_CRITICAL(); /* allow schedule after deinit */

    return err;
}

/* GDMA channenl configure function */
FError FFreeRTOSGdmaChanConfigure(FFreeRTOSGdma *const instance_p, 
                                  uint32_t channel_id, 
                                  FFreeRTOSGdmaChanCfg const *os_channel_config_p)
{
    FASSERT(instance_p);
    FASSERT_MSG(channel_id < FFREERTOS_GDMA_NUM_OF_CHAN, "Invalid GDMA channel id.");
    FASSERT(os_channel_config_p);

    FGdma *ctrl_p = &instance_p->ctrl;
    FError err = FFREERTOS_GDMA_OK;

    err = FGdmaOsTakeSema(instance_p->locker);
    if (FFREERTOS_GDMA_OK != err)
    {
        FGDMA_ERROR("Mutex instance_p->locker is occupied.");
        return err;
    }

    /* channel config set */
    FGdmaChanConfig channel_config = FGDMA_DEFAULT_CHAN_CONFIG();

    if (os_channel_config_p->trans_mode == FFREERTOS_GDMA_OPER_DIRECT)
    {
        channel_config.trans_mode = FGDMA_OPER_DIRECT;
        channel_config.src_addr = os_channel_config_p->src_addr;
        channel_config.dst_addr = os_channel_config_p->dst_addr;
        channel_config.trans_length = os_channel_config_p->trans_length;
    }
    else if (os_channel_config_p->trans_mode == FFREERTOS_GDMA_OPER_BDL)
    {
        FMemp *memp = &instance_p->memp;

        /* allocate a piece of memory space from memroy pool for BDL descriptor list */
        FGdmaBdlDesc *first_desc_addr = FMempMallocAlign(memp, 
                                                         sizeof(FGdmaBdlDesc) * channel_config.valid_desc_num, 
                                                         FGDMA_BDL_DESC_ALIGMENT);
        if (first_desc_addr == NULL)
        {
            FGDMA_ERROR("Allocate memory space from memroy pool failed.");
            err = FFREERTOS_GDMA_ALLOCATE_FAIL;
            goto err_exit;
        }
        channel_first_desc_addr[channel_id] = first_desc_addr;

        /* creat BDL descriptor list */
        FGdmaBdlDescConfig bdl_desc_config[channel_config.valid_desc_num];
        s32 pre_desc_trans_len = os_channel_config_p->trans_length / channel_config.valid_desc_num;
        for (s32 loop = 0; loop < channel_config.valid_desc_num; loop++)
        {
            bdl_desc_config[loop] = FGDMA_DEFAULT_DESC_CONFIG();
            bdl_desc_config[loop].current_desc_num = loop;
            bdl_desc_config[loop].src_addr = os_channel_config_p->src_addr + loop * pre_desc_trans_len;
            bdl_desc_config[loop].dst_addr = os_channel_config_p->dst_addr + loop * pre_desc_trans_len;
            bdl_desc_config[loop].trans_length = pre_desc_trans_len;
            FGdmaBDLSetDesc(first_desc_addr, &bdl_desc_config[loop]);
        }

        channel_config.trans_mode = FGDMA_OPER_BDL;
        channel_config.first_desc_addr = first_desc_addr;
    }

    /* use channel config to configure channel */
    err = FGdmaChanConfigure(ctrl_p, channel_id, &channel_config);
    if (FGDMA_SUCCESS != err)
    {
        FGDMA_ERROR("GDMA channel configure failed.");
        goto err_exit;
    }

    /* register GIC interrupt */
    if (ctrl_p->config.caps & FGDMA_IRQ2_MASK)
    {
        FGdmaSetupPrivateInterrupt(ctrl_p, channel_id);
    }

    FGdmaChanRegisterEvtHandler(ctrl_p,
                                channel_id,
                                FGDMA_CHAN_EVT_TRANS_END,
                                GdmaChannelTransferEnd,
                                NULL);

err_exit:
    FGdmaOsGiveSema(instance_p->locker);
    return err;
}

/* GDMA channel transfer start function */
void FFreeRTOSGdmaChanStart(FFreeRTOSGdma *const instance_p, uint32_t channel_id)
{
    FASSERT(instance_p);
    FASSERT_MSG(channel_id < FFREERTOS_GDMA_NUM_OF_CHAN, "Invalid GDMA channel id.");

    FGdma *ctrl_p = &instance_p->ctrl;
    FError err = FFREERTOS_GDMA_OK;

    err = FGdmaOsTakeSema(instance_p->locker);
    if (FFREERTOS_GDMA_OK != err)
    {
        FGDMA_ERROR("Mutex instance_p->locker is occupied.");
        goto err_exit;
    }

    FGdmaChanStartTrans(ctrl_p, channel_id); /* start channel transfer */

err_exit:
    FGdmaOsGiveSema(instance_p->locker);
    return;
}

/* GDMA channel stop function */
FError FFreeRTOSGdmaChanStop(FFreeRTOSGdma *const instance_p, uint32_t channel_id)
{
    FASSERT(instance_p);
    FASSERT_MSG(channel_id < FFREERTOS_GDMA_NUM_OF_CHAN, "Invalid GDMA channel id.");

    FGdma *ctrl_p = &instance_p->ctrl;
    FError err = FFREERTOS_GDMA_OK;

    err = FGdmaOsTakeSema(instance_p->locker);
    if (FFREERTOS_GDMA_OK != err)
    {
        FGDMA_ERROR("Mutex instance_p->locker is occupied.");
        goto err_exit;
    }

    err = FGdmaChanAbort(ctrl_p, channel_id);
    if (err != FFREERTOS_GDMA_OK)
    {
        FGDMA_ERROR("GDMA abort failed.");
        goto err_exit;
    }

err_exit:
    FGdmaOsGiveSema(instance_p->locker);
    return err;
}

FError FFreeRTOSGdmaChanDeconfigure(FFreeRTOSGdma *const instance_p, uint32_t channel_id)
{
    FASSERT(instance_p);
    FASSERT_MSG(channel_id < FFREERTOS_GDMA_NUM_OF_CHAN, "Invalid GDMA channel id.");

    FGdma *ctrl_p = &instance_p->ctrl;
    FError err = FFREERTOS_GDMA_OK;
    FMemp *memp = &instance_p->memp;

    err = FGdmaOsTakeSema(instance_p->locker);
    if (FFREERTOS_GDMA_OK != err)
    {
        FGDMA_ERROR("Mutex instance_p->locker is occupied.");
        goto err_exit;
    }

    /* free dynamic memroy allocated for BDL desciptor list */
    if (channel_first_desc_addr[channel_id])
    {
        FMempFree(memp, channel_first_desc_addr[channel_id]);
        channel_first_desc_addr[channel_id] = NULL;
    }

    /* deconfigure GDMA channel */
    err = FGdmaChanDeconfigure(ctrl_p, channel_id);
    if (FGDMA_SUCCESS != err)
    {
        FGDMA_ERROR("Deconfigure GDMA channel-%d failed.", channel_id);
    }

err_exit:
    FGdmaOsGiveSema(instance_p->locker);
    return err;
}