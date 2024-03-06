/*
 * Copyright (c) 2023 Phytium Information Technology, Inc.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_sdmmc_common.h"
#include "fsl_sdmmc.h"
#include "fparameters.h"
#include "fsdmmc.h"
#include "fsdmmc_hw.h"

#include "finterrupt.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
static const char* TAG = "SDMMC:FSdmmc";

typedef struct _fsdmmchost_dev_
{
    FSdmmc hc;
    FSdmmcConfig hc_cfg;
    FSdmmcCmd cmd_pkg;
    FSdmmcData dat_pkg;
    volatile boolean cmd_done;
    volatile boolean data_done;
    volatile boolean cmd_error;
    volatile boolean data_error;
} fsdmmchost_dev_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern status_t FSDMMCHOST_CmdSwitch(sd_card_t *card);
/*!
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static void FSDMMCHOST_Relax(void)
{
    SDMMC_OSADelay(1);
}

static void FSDMMCHOST_RemoveCardCB(void *para)
{
    assert(para);
    fsdmmchost_dev_t *dev = (fsdmmchost_dev_t *)para;

    SDMMC_LOG("Card removed !!!");
}

static void FSDMMCHOST_CommandDoneCB(void *para)
{
    assert(para);
    fsdmmchost_dev_t *dev = (fsdmmchost_dev_t *)para;

    dev->cmd_done = TRUE;
}

static void FSDMMCHOST_CommandErrorCB(void *para)
{
    assert(para);
    fsdmmchost_dev_t *dev = (fsdmmchost_dev_t *)para;

    dev->cmd_done = TRUE;
    dev->cmd_error = TRUE;
}

static void FSDMMCHOST_DataDoneCB(void *para)
{
    assert(para);
    fsdmmchost_dev_t *dev = (fsdmmchost_dev_t *)para;

    dev->data_done = TRUE;
}

static void FSDMMCHOST_DataErrorCB(void *para)
{
    assert(para);
    fsdmmchost_dev_t *dev = (fsdmmchost_dev_t *)para;

    dev->data_done = TRUE;
    dev->data_error = TRUE;
}

static void FSDMMCHOST_SetupIrq(fsdmmchost_dev_t *dev)
{
    FSdmmc *ctrl_p = &(dev->hc);
    FSdmmcConfig *config_p = &ctrl_p->config;
    uintptr base_addr = config_p->base_addr;
    u32 reg_val;

    /* disable all interrupt */
    FSdmmcSetInterruptMask(base_addr, FSDMMC_CMD_INTR, FSDMCC_NORMAL_INT_ALL_BITS, FALSE);
    FSdmmcSetInterruptMask(base_addr, FSDMMC_ERROR_INTR, FSDMMC_ERROR_INT_ALL_BITS, FALSE);
    FSdmmcSetInterruptMask(base_addr, FSDMMC_DMA_BD_INTR, FSDMMC_BD_ISR_ALL_BITS, FALSE);

    /* clear interrupt status */
    FSdmmcClearNormalInterruptStatus(base_addr);
    FSdmmcClearErrorInterruptStatus(base_addr);
    FSdmmcClearBDInterruptStatus(base_addr);

    /* register intr, attach interrupt handler */
    InterruptSetPriority(config_p->irq_num[FSDMMC_CMD_INTR], 0);
    InterruptSetPriority(config_p->irq_num[FSDMMC_ERROR_INTR], 0);
    InterruptSetPriority(config_p->irq_num[FSDMMC_DMA_BD_INTR], 0);

    InterruptInstall(config_p->irq_num[FSDMMC_CMD_INTR], FSdmmcCmdInterrupHandler, ctrl_p, "FSDMMC-CMD");
    InterruptInstall(config_p->irq_num[FSDMMC_ERROR_INTR], FSdmmcErrInterrupHandler, ctrl_p, "FSDMMC-ERR");
    InterruptInstall(config_p->irq_num[FSDMMC_DMA_BD_INTR], FSdmmcDmaInterrupHandler, ctrl_p, "FSDMMC-DMA");

    /* umask and enable fsdio interrupt */
    InterruptUmask(config_p->irq_num[FSDMMC_CMD_INTR]);
    InterruptUmask(config_p->irq_num[FSDMMC_ERROR_INTR]);
    InterruptUmask(config_p->irq_num[FSDMMC_DMA_BD_INTR]);

    /* enable some interrupts */
    FSdmmcSetInterruptMask(base_addr, FSDMMC_CMD_INTR, FSDMCC_NORMAL_INT_ALL_BITS, TRUE);
    FSdmmcSetInterruptMask(base_addr, FSDMMC_ERROR_INTR, FSDMMC_ERROR_INT_ALL_BITS, TRUE);
    FSdmmcSetInterruptMask(base_addr, FSDMMC_DMA_BD_INTR, FSDMMC_BD_ISR_ALL_BITS, TRUE);

    /* register interrupt event handler */
    FSdmmcRegisterInterruptHandler(ctrl_p, FSDMMC_EVT_CARD_REMOVED, FSDMMCHOST_RemoveCardCB, dev);
    FSdmmcRegisterInterruptHandler(ctrl_p, FSDMMC_EVT_CMD_DONE, FSDMMCHOST_CommandDoneCB, dev);
    FSdmmcRegisterInterruptHandler(ctrl_p, FSDMMC_EVT_CMD_ERROR, FSDMMCHOST_CommandErrorCB, dev);
    FSdmmcRegisterInterruptHandler(ctrl_p, FSDMMC_EVT_CMD_RESP_ERROR, FSDMMCHOST_CommandErrorCB, dev);
    FSdmmcRegisterInterruptHandler(ctrl_p, FSDMMC_EVT_DATA_READ_DONE, FSDMMCHOST_DataDoneCB, dev);
    FSdmmcRegisterInterruptHandler(ctrl_p, FSDMMC_EVT_DATA_WRITE_DONE, FSDMMCHOST_DataDoneCB, dev);
    FSdmmcRegisterInterruptHandler(ctrl_p, FSDMMC_EVT_DATA_ERROR, FSDMMCHOST_DataErrorCB, dev);    

    return;
}

static void FSDMMCHOST_RevokeIrq(fsdmmchost_dev_t *dev)
{
    FSdmmc *ctrl_p = &(dev->hc);

    /* disable sdmmc irq */
    InterruptMask(ctrl_p->config.irq_num[FSDMMC_CMD_INTR]);
    InterruptMask(ctrl_p->config.irq_num[FSDMMC_ERROR_INTR]);
    InterruptMask(ctrl_p->config.irq_num[FSDMMC_DMA_BD_INTR]);
}

static void FSDMMCHOST_Deinit(sdmmchost_t *host)
{
    fsdmmchost_dev_t *dev = (fsdmmchost_dev_t *)host->dev;

    FSDMMCHOST_RevokeIrq(dev);
    FSdmmcDeInitialize(&dev->hc);

    memset(&dev->hc, 0U, sizeof(dev->hc));
    memset(dev, 0U, sizeof(*dev));

    SDMMC_LOG("Sdmmc ctrl deinited !!!");
}

static status_t FSDMMCHOST_Reset(sdmmchost_t *host)
{
    fsdmmchost_dev_t *dev = (fsdmmchost_dev_t *)host->dev;
    uintptr base_addr = dev->hc.config.base_addr;

    FSdmmcSoftwareReset(base_addr, FSDMMC_TIMEOUT);
    return kStatus_Success;
}

static void FSDMMCHOST_SwitchToVoltage(sdmmchost_t *host, uint32_t voltage)
{
    switch (voltage)
    {
        case kSDMMC_OperationVoltage300V:
        case kSDMMC_OperationVoltage330V:
            return; /* 3.3v only */
            break;
    }

    /* do not support voltage */
    return;
}

static status_t FSDMMCHOST_ExecuteTuning(sdmmchost_t *host, uint32_t tuningCmd, uint32_t *revBuf, uint32_t blockSize)
{
    /* do not support tuning */
    return kStatus_Success;
}

static void FSDMMCHOST_EnableDDRMode(sdmmchost_t *host, bool enable, uint32_t nibblePos)
{
    /* do not support DDR mode */
}

static void FSDMMCHOST_EnableHS400Mode(sdmmchost_t *host, bool enable)
{
    SDMMC_LOGE(TAG, "%s not implmented", __func__);
}

static void FSDMMCHOST_EnableStrobeDll(sdmmchost_t *host, bool enable)
{
    SDMMC_LOGE(TAG, "%s not implmented", __func__);
}

static uint32_t FSDMMCHOST_GetSignalLineStatus(sdmmchost_t *host, uint32_t signalLine)
{
    /* do not support busy state */
    return true;
}

static void FSDMMCHOST_ConvertDataToLittleEndian(sdmmchost_t *host, uint32_t *data, uint32_t wordSize, uint32_t format)
{
    uint32_t temp = 0U;

    if (((uint32_t)host->config.endianMode == (uint32_t)kSDMMCHOST_EndianModeLittle) &&
        (format == kSDMMC_DataPacketFormatMSBFirst))
    {
        for (uint32_t i = 0U; i < wordSize; i++)
        {
            temp    = data[i];
            data[i] = SWAP_WORD_BYTE_SEQUENCE(temp);
        }
    }
    else if ((uint32_t)host->config.endianMode == (uint32_t)kSDMMCHOST_EndianModeHalfWordBig)
    {
        for (uint32_t i = 0U; i < wordSize; i++)
        {
            temp    = data[i];
            data[i] = SWAP_HALF_WROD_BYTE_SEQUENCE(temp);
        }
    }
    else if (((uint32_t)host->config.endianMode == (uint32_t)kSDMMCHOST_EndianModeBig) &&
             (format == kSDMMC_DataPacketFormatLSBFirst))
    {
        for (uint32_t i = 0U; i < wordSize; i++)
        {
            temp    = data[i];
            data[i] = SWAP_WORD_BYTE_SEQUENCE(temp);
        }
    }
    else
    {
        /* nothing to do */
    }
}

static status_t FSDMMCHOST_CardDetectInit(sdmmchost_t *host, void *cd)
{
    return kStatus_Success;
}

static void FSDMMCHOST_SetCardPower(sdmmchost_t *host, bool enable)
{

}

static void FSDMMCCHOST_EnableCardInt(sdmmchost_t *host, bool enable)
{
    SDMMC_LOGE(TAG, "%s not implmented", __func__);
}

static status_t FSDMMCHOST_CardIntInit(sdmmchost_t *host, void *sdioInt)
{
    SDMMC_LOGE(TAG, "%s not implmented", __func__);
    return kStatus_Fail;    
}

static void FSDMMCHOST_SetCardBusWidth(sdmmchost_t *host, uint32_t dataBusWidth)
{
    /* bus width is fixed as 4-bit */
}

static void FSDMMCHOST_SendCardActive(sdmmchost_t *host)
{

}

static status_t FSDMMCHOST_PollingCardDetectStatus(sdmmchost_t *host, uint32_t waitCardStatus, uint32_t timeout)
{
    return kStatus_Success;
}

static uint32_t FSDMMCHOST_CardDetectStatus(sdmmchost_t *host)
{
    fsdmmchost_dev_t *dev = (fsdmmchost_dev_t *)host->dev;
    uintptr base_addr = dev->hc_cfg.base_addr;    

    return FSdmmcCheckIfCardExists(base_addr) ? kSD_Inserted : kSD_Removed;
}

static uint32_t FSDMMCHOST_SetCardClock(sdmmchost_t *host, uint32_t targetClock)
{
    fsdmmchost_dev_t *dev = (fsdmmchost_dev_t *)host->dev;
    uintptr base_addr = dev->hc_cfg.base_addr;

    if (FSDMMC_SUCCESS != FSdmmcSetCardClk(base_addr, targetClock)) 
    {
        SDMMC_LOGE(TAG, "Failed to update clock");
        return 0U;
    }
    else
    {
        SDMMC_LOGD(TAG, "BUS CLOCK: %d", targetClock);
        return targetClock;
    }    
}

static void FSDMMCHOST_ForceClockOn(sdmmchost_t *host, bool enable)
{
    /* no support to on/off clock */
}

static bool FSDMMCHOST_IsCardBusy(sdmmchost_t *host)
{
    return false;
}

static status_t FSDMMCHOST_PreCommand(sdmmchost_t *host, sdmmchost_transfer_t *content)
{
    if ((kSDMMC_ReadSingleBlock == content->command->index) ||
        (kSDMMC_ReadMultipleBlock == content->command->index) ||
        (kSDMMC_WriteSingleBlock == content->command->index) ||
        (kSDMMC_WriteMultipleBlock == content->command->index))
    {
        /* it's a quirk that some host controller need to send CMD23 before mulit-block read/write */
        u32 block_count = content->data->blockCount;
        return SDMMC_SetBlockCount(host, block_count);
    }

    return kStatus_Success;
}

static status_t FSDMMCHOST_PostCommand(sdmmchost_t *host, sdmmchost_transfer_t *content)
{
    /* FT2004/D2000 need switch to 4-bit bus width before first DMA in CMD51 */
    if ((kSDMMC_SelectCard == content->command->index) && (0U != content->command->argument))
    {
        (void)FSDMMCHOST_CmdSwitch(host->card);
    }

    return kStatus_Success;
}

static void FSDMMCHOST_CovertCommandInfo(sdmmchost_t *host, sdmmchost_transfer_t *in_trans, 
                                         FSdmmcCmd *out_cmd, FSdmmcData *out_data)
{
    uint32_t cmd_ind = in_trans->command->index;

    if (kCARD_ResponseTypeNone != in_trans->command->responseType)
    {
        out_cmd->flag |= FSDMMC_CMD_FLAG_EXP_RESP;


        if (kCARD_ResponseTypeR2 == in_trans->command->responseType)
        {
            /* need 136 bits long response */
            out_cmd->flag |= FSDMMC_CMD_FLAG_EXP_LONG_RESP;
        }
    }

    if (kSDMMC_GoIdleState == cmd_ind)
    {
        out_cmd->flag |= FSDMMC_CMD_FLAG_NEED_INIT;
    }

    out_cmd->cmdidx = in_trans->command->index;
    out_cmd->cmdarg = in_trans->command->argument;
    if (NULL != in_trans->data)
    {
        /* assign read/write flag */
        /* copy addr of in/out data buffer */
        out_cmd->flag |= FSDMMC_CMD_FLAG_EXP_DATA;
        if (in_trans->data->rxData)
        {
            out_cmd->flag |= FSDMMC_CMD_FLAG_READ_DATA;
            out_data->buf = (uint8_t *)in_trans->data->rxData;
        }
        else
        {
            out_cmd->flag |= FSDMMC_CMD_FLAG_WRITE_DATA;
            out_data->buf = (uint8_t *)in_trans->data->txData;
        }

        out_data->blksz = in_trans->data->blockSize;
        out_data->blkcnt = in_trans->data->blockCount;
        out_data->datalen = in_trans->data->blockSize * 
                            in_trans->data->blockCount; 

        SDMMC_LOGD(TAG, "buf = 0x%x, len = 0x%x", 
                    out_data->buf, out_data->datalen);

        /* save data info */
        out_cmd->data_p = out_data;       
    }

    return;
}

static void FSDMMCHOST_GetCommandResp(uintptr_t base_addr, FSdmmcCmd *in_cmd, sdmmchost_transfer_t *out_trans)
{
	if (in_cmd->flag & FSDMMC_CMD_FLAG_EXP_RESP)
	{
		if (in_cmd->flag & FSDMMC_CMD_FLAG_EXP_LONG_RESP)
		{
            /* in FT2004/D2000, byte-order of response no need filp, but array need reverse */
			in_cmd->response[3] = FSDMMC_READ_REG(base_addr, FSDMMC_CMD_RESP_1_REG_OFFSET);
			in_cmd->response[2] = FSDMMC_READ_REG(base_addr, FSDMMC_CMD_RESP_2_REG_OFFSET);
			in_cmd->response[1] = FSDMMC_READ_REG(base_addr, FSDMMC_CMD_RESP_3_REG_OFFSET);
			in_cmd->response[0] = FSDMMC_READ_REG(base_addr, FSDMMC_CMD_RESP_4_REG_OFFSET);
        }
        else
		{
			in_cmd->response[0] = FSDMMC_READ_REG(base_addr, FSDMMC_CMD_RESP_1_REG_OFFSET);
			in_cmd->response[1] = 0;
			in_cmd->response[2] = 0;
			in_cmd->response[3] = 0;			
		}

        memcpy((u32 *)out_trans->command->response, in_cmd->response, sizeof(u32) * 4); /* copy back response buffer */
	}
}

static status_t FSDMMCHOST_TransferFunction_Poll(sdmmchost_t *host, sdmmchost_transfer_t *content)
{
    assert(content);
    status_t status = kStatus_Success;
    fsdmmchost_dev_t *dev = (fsdmmchost_dev_t *)host->dev;
    FSdmmcCmd *cmd_data = &(dev->cmd_pkg);
    FSdmmcData *trans_data = &(dev->dat_pkg);
    uint32_t timeout = 5000;

    status = FSDMMCHOST_PreCommand(host, content);
    if (kStatus_Success != status)
    {
        return status; 
    }

    memset(cmd_data, 0U, sizeof(*cmd_data));
    memset(trans_data, 0U, sizeof(*trans_data));

    dev->cmd_done = FALSE;
    dev->data_done = FALSE;
    dev->cmd_error = FALSE;
    dev->data_error = FALSE;

    FSDMMCHOST_CovertCommandInfo(host, content, cmd_data, trans_data);

    if (FSDMMC_SUCCESS == FSdmmcPollTransfer(&(dev->hc), cmd_data))
    {
        FSDMMCHOST_GetCommandResp(dev->hc.config.base_addr, cmd_data, content);
        status = FSDMMCHOST_PostCommand(host, content);      
        SDMMC_LOGI(TAG, "CMD [%d] END: 0x%x.", content->command->index, 0);
    }
    else
    {
        status = kStatus_Fail;
    }

    return status;  
}

static status_t FSDMMCHOST_TransferFunction_Irq(sdmmchost_t *host, sdmmchost_transfer_t *content)
{
    assert(content);
    status_t status = kStatus_Success;
    fsdmmchost_dev_t *dev = (fsdmmchost_dev_t *)host->dev;
    FSdmmcCmd *cmd_data = &(dev->cmd_pkg);
    FSdmmcData *trans_data = &(dev->dat_pkg);
    uint32_t timeout = 1000;
    uint32_t loop;

    status = FSDMMCHOST_PreCommand(host, content);
    if (kStatus_Success != status)
    {
        return status; 
    }

    memset(cmd_data, 0U, sizeof(*cmd_data));
    memset(trans_data, 0U, sizeof(*trans_data));

    dev->cmd_done = FALSE;
    dev->data_done = FALSE;
    dev->cmd_error = FALSE;
    dev->data_error = FALSE;

    FSDMMCHOST_CovertCommandInfo(host, content, cmd_data, trans_data);

    if (FSDMMC_SUCCESS != FSdmmcInterruptTransfer(&(dev->hc), cmd_data))
    {
        status = kStatus_Fail;
        return status;
    }

    /* wait cmd transfer done */
    loop = 0;
    while ((FALSE == dev->cmd_done) && (loop++ < timeout))
    {
        FSDMMCHOST_Relax();
    }

    /* wait cmd timeout */
    if (loop >= timeout)
    {
        SDMMC_LOGE(TAG, "Sdmmc transfer cmd timeout.");
        status = kStatus_Fail;
        return status;
    }

    /* wait data transfer done */
    loop = 0;
    while ((cmd_data->data_p) && (FALSE == dev->data_done) && 
           (loop++ < timeout))
    {
        FSDMMCHOST_Relax();
    }

    /* wait data timeout */
    if (loop >= timeout)
    {
        SDMMC_LOGE(TAG, "Sdmmc transfer data timeout.");
        status = kStatus_Fail;
        return status;
    }

    FSDMMCHOST_GetCommandResp(dev->hc.config.base_addr, cmd_data, content);
    status = FSDMMCHOST_PostCommand(host, content);

    return status;
}

static const sdmmchost_ops_t sdmmc_ops = 
{
    .deinit = FSDMMCHOST_Deinit,
    .reset = FSDMMCHOST_Reset,

    .switchToVoltage = FSDMMCHOST_SwitchToVoltage,
    .executeTuning = FSDMMCHOST_ExecuteTuning,
    .enableDDRMode = FSDMMCHOST_EnableDDRMode,
    .enableHS400Mode = FSDMMCHOST_EnableHS400Mode,
    .enableStrobeDll = FSDMMCHOST_EnableStrobeDll,
    .getSignalLineStatus = FSDMMCHOST_GetSignalLineStatus,
    .convertDataToLittleEndian = FSDMMCHOST_ConvertDataToLittleEndian,

    .cardDetectInit = FSDMMCHOST_CardDetectInit,
    .cardSetPower = FSDMMCHOST_SetCardPower,
    .cardEnableInt = FSDMMCCHOST_EnableCardInt,
    .cardIntInit = FSDMMCHOST_CardIntInit,
    .cardSetBusWidth = FSDMMCHOST_SetCardBusWidth,
    .cardPollingDetectStatus = FSDMMCHOST_PollingCardDetectStatus,
    .cardDetectStatus = FSDMMCHOST_CardDetectStatus,
    .cardSendActive = FSDMMCHOST_SendCardActive,
    .cardSetClock = FSDMMCHOST_SetCardClock,
    .cardForceClockOn = FSDMMCHOST_ForceClockOn,
    .cardIsBusy = FSDMMCHOST_IsCardBusy,

    .transferFunction = NULL,

    .startBoot = NULL,
    .readBootData = NULL,
    .enableBoot = NULL,
};

static status_t FSDMMCHOST_DoInit(sdmmchost_t *host)
{
    status_t ret = kStatus_Success;
    fsdmmchost_dev_t *dev = (fsdmmchost_dev_t *)host->dev;
    dev->hc_cfg = *FSdmmcLookupConfig(host->config.hostId);

    /* init sdmmc ctrl */
    memset(&dev->hc, 0, sizeof(dev->hc));
    if (FSDMMC_SUCCESS != FSdmmcCfgInitialize(&dev->hc, &dev->hc_cfg))
    {
        SDMMC_LOGE(TAG, "Sdmmc ctrl init failed.");
        ret = kStatus_Fail;
        return ret;         
    }

    if (host->config.enableIrq)
    {
        FSDMMCHOST_SetupIrq(dev);
    }

    return ret;
}

status_t FSDMMCHOST_Init(sdmmchost_t *host)
{
    assert(host);

    /* find the space for dev instance */
    fsdmmchost_dev_t *dev = SDMMC_OSAMemoryAllocate(sizeof(fsdmmchost_dev_t));
    if (NULL == dev)
    {
        return kStatus_OutOfRange;
    }

    memset(dev, 0U, sizeof(*dev));

    SDMMC_LOGI(TAG, "Allocate INST@0x%x", &(dev->hc));

    host->ops = sdmmc_ops;
    host->dev = dev;

    if (host->config.enableIrq)
    {
        host->ops.transferFunction = FSDMMCHOST_TransferFunction_Irq;
    }
    else
    {
        host->ops.transferFunction = FSDMMCHOST_TransferFunction_Poll;
    }

    return FSDMMCHOST_DoInit(host);
}

status_t FSDMMCHOST_SDConfig(sdmmc_sd_t *sdmmc, sdmmchost_config_t *config)
{
    assert(sdmmc);
    assert(config);
    sdmmchost_t *host = &sdmmc->host;
    sd_card_t *card = &sdmmc->card;
    sd_detect_card_t *card_cd = &sdmmc->cardDetect;
    sd_io_voltage_t *io_voltage = &sdmmc->ioVoltage;

    card_cd->type = kSD_DetectCardByHostCD;
    card_cd->cdDebounce_ms = 10;

    card->usrParam.pwr = NULL;
    card->usrParam.powerOnDelayMS = 0U;
    card->usrParam.powerOffDelayMS = 0U;
    card->usrParam.ioStrength = NULL;
    card->usrParam.capability = (uint32_t)kSDMMCHOST_Support4BitDataWidth |
                                (uint32_t)kSDMMCHOST_SupportDetectCardByData3 |
                                (uint32_t)kSDMMCHOST_SupportDetectCardByCD;

    card->usrParam.ioVoltage = NULL; /* no need to switch voltage */
    card->noInteralAlign = FALSE;

    host->capability |= (uint32_t)kSDMMCHOST_SupportVoltage3v3;

    if (host->config.cardClock >= SD_CLOCK_50MHZ)
    {
        host->capability |= (uint32_t)kSDMMCHOST_SupportHighSpeed;
    }

    card->usrParam.maxFreq = (uint32_t)host->config.cardClock;
    host->maxBlockCount = host->config.maxTransSize / host->config.defBlockSize;
    host->maxBlockSize  = SDMMCHOST_SUPPORT_MAX_BLOCK_LENGTH;
    host->sourceClock_Hz = FSDMMC_CLK_FREQ_HZ; /* 600MHz */

    return kStatus_Success;
}