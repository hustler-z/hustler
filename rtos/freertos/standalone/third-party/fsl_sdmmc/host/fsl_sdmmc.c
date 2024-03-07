/*
 * Copyright (c) 2023 Phytium Information Technology, Inc.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_sdmmc_common.h"
#include "fsl_sdmmc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#ifdef CONFIG_FSL_SDMMC_USE_FSDIF
extern status_t FSDIFHOST_SDConfig(sdmmc_sd_t *sdmmc, sdmmchost_config_t *config);
extern status_t FSDIFHOST_MMCConfig(sdmmc_mmc_t *sdmmc, sdmmchost_config_t *config);
#if defined(CONFIG_FSL_SDMMC_ENABLE_SDIO)
extern status_t FSDIFHOST_SDIOConfig(sdmmc_sdio_t *sdmmc, sdmmchost_config_t *config);
#endif

extern status_t FSDIFHOST_Init(sdmmchost_t *host);
#endif

#ifdef CONFIG_FSL_SDMMC_USE_FSDMMC
extern status_t FSDMMCHOST_SDConfig(sdmmc_sd_t *sdmmc, sdmmchost_config_t *config);
extern status_t FSDMMCHOST_Init(sdmmchost_t *host);
#endif
/*!
/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char* TAG = "SDMMC";

/*******************************************************************************
 * Code
 ******************************************************************************/
status_t SD_CfgInitialize(sdmmc_sd_t *sdmmc, sdmmchost_config_t *config)
{
    assert(sdmmc);
    assert(config);
    status_t status = kStatus_Fail;
    sdmmchost_t *host = &sdmmc->host;
    sd_card_t *card = &sdmmc->card;
    sd_detect_card_t *card_cd = &sdmmc->cardDetect;

    /* link data structures */
    host->config = *config;
    host->cd = card_cd;
    card->host = host;
    host->card = card;
    card->usrParam.cd = card_cd;

    /* allocate aligned memory from innerl buffer */
    card->internalBuffer = SDMMC_OSAMemoryAlignedAllocate(host->config.maxTransSize, 
                                                          host->config.defBlockSize);
    if (NULL == card->internalBuffer)
    {
        return kStatus_OutOfRange;
    }

    memset(card->internalBuffer, 0U, host->config.maxTransSize);
    card->internalBufferSize = host->config.maxTransSize;

    SDMMC_LOG("Internal buffer@0x%x, length = 0x%x",
                card->internalBuffer,
                host->config.maxTransSize);

    host->capability = 0U;

#if defined(CONFIG_FSL_SDMMC_USE_FSDIF)
    if (kSDMMCHOST_TYPE_FSDIF == host->config.hostType)
    {
        status = FSDIFHOST_SDConfig(sdmmc, config);
    }
#endif

#if defined(CONFIG_FSL_SDMMC_USE_FSDMMC)
    if (kSDMMCHOST_TYPE_FSDMMC == host->config.hostType)
    {
        status = FSDMMCHOST_SDConfig(sdmmc, config);
    }
#endif

    if (kStatus_Success == status)
    {
        status = SD_Init(card);
    }

    return status;
}

#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
status_t MMC_CfgInitialize(sdmmc_mmc_t *sdmmc, sdmmchost_config_t *config)
{
    assert(sdmmc);
    assert(config);
    status_t status = kStatus_Fail;
    sdmmchost_t *host = &sdmmc->host;
    mmc_card_t *card = &sdmmc->card;

    /* link data structures */
    host->config = *config;
    card->host = host;
    host->card = card;

    /* allocate aligned memory from innerl buffer */
    card->internalBuffer = SDMMC_OSAMemoryAlignedAllocate(host->config.maxTransSize, 
                                                          host->config.defBlockSize);
    if (NULL == card->internalBuffer)
    {
        return kStatus_OutOfRange;
    }

    memset(card->internalBuffer, 0U, host->config.maxTransSize);
    card->internalBufferSize = host->config.maxTransSize;

    host->capability = 0U;

#if defined(CONFIG_FSL_SDMMC_USE_FSDIF)
    if (kSDMMCHOST_TYPE_FSDIF == host->config.hostType)
    {
        status = FSDIFHOST_MMCConfig(sdmmc, config);
    }
#endif

    if (kStatus_Success == status)
    {
        status = MMC_Init(card);
    }

    return status;    
}
#endif

#if defined(CONFIG_FSL_SDMMC_ENABLE_SDIO)
status_t SDIO_CfgInitialize(sdmmc_sdio_t *sdmmc, sdmmchost_config_t *config)
{
    assert(sdmmc);
    assert(config);
    status_t status = kStatus_Fail;
    sdmmchost_t *host = &sdmmc->host;
    sdio_card_t *card = &sdmmc->card;
    sd_detect_card_t *card_cd = &sdmmc->cardDetect;

    /* link data structures */
    host->config = *config;
    host->cd = card_cd;
    card->host = host;
    host->card = card;
    card->usrParam.cd = card_cd;

    /* allocate aligned memory from innerl buffer */
    card->internalBuffer = SDMMC_OSAMemoryAlignedAllocate(host->config.maxTransSize, 
                                                          host->config.defBlockSize);
    if (NULL == card->internalBuffer)
    {
        return kStatus_OutOfRange;
    }

    memset(card->internalBuffer, 0U, host->config.maxTransSize);
    card->internalBufferSize = host->config.maxTransSize;

    host->capability = 0U;

#if defined(CONFIG_FSL_SDMMC_USE_FSDIF)
    if (kSDMMCHOST_TYPE_FSDIF == host->config.hostType)
    {
        status = FSDIFHOST_SDIOConfig(sdmmc, config);
    }
#endif

    if (kStatus_Success == status)
    {
        status = SDIO_Init(card);
    }

    return status;
}
#endif

status_t SDMMCHOST_Init(sdmmchost_t *host)
{
    assert(host);
    status_t status = kStatus_Fail;

#if defined(CONFIG_FSL_SDMMC_USE_FSDIF)
    if (kSDMMCHOST_TYPE_FSDIF == host->config.hostType)
    {
        status = FSDIFHOST_Init(host);
    }
#endif

#if defined(CONFIG_FSL_SDMMC_USE_FSDMMC)
    if (kSDMMCHOST_TYPE_FSDMMC == host->config.hostType)
    {
        status = FSDMMCHOST_Init(host);
    }
#endif

    return status;
}

