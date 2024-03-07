/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_SDMMC_CONFIG_H
#define _FSL_SDMMC_CONFIG_H

#include "fsl_sdmmc_host.h"
#include "fsl_sd.h"
#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
#include "fsl_mmc.h"
#endif
#if defined(CONFIG_FSL_SDMMC_ENABLE_SDIO)
#include "fsl_sdio.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!@brief sdmmc (SD) instance  */
typedef struct _sdmmc_sd_
{
    sdmmchost_t host;
    sd_card_t card;
    sd_detect_card_t cardDetect;
    sd_io_voltage_t ioVoltage;
} sdmmc_sd_t;

#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
/*!@brief sdmmc (MMC) instance  */
typedef struct _sdmmc_mmc_
{
    sdmmchost_t host;
    mmc_card_t card;
} sdmmc_mmc_t;
#endif

#if defined(CONFIG_FSL_SDMMC_ENABLE_SDIO)
/*!@brief sdmmc (SDIO) instance  */
typedef struct _sdmmc_sdio_
{
    sdmmchost_t host;
    sdio_card_t card;
    sd_detect_card_t cardDetect;
    sd_io_voltage_t ioVoltage;
    sdio_card_int_t sdioInt;
} sdmmc_sdio_t;
#endif

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Configure and initialize host controller for SD card
 *
 * Thread safe function, please note that the function will create the mutex lock dynamically by default,
 * so to avoid the mutex create redundantly, application must follow bellow sequence for card re-initialization
 *
 * @param host host handler
 */
status_t SD_CfgInitialize(sdmmc_sd_t *sdmmc, sdmmchost_config_t *config);

#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
/*!
 * @brief Configure and initialize host controller for MMC card
 *
 * Thread safe function, please note that the function will create the mutex lock dynamically by default,
 * so to avoid the mutex create redundantly, application must follow bellow sequence for card re-initialization
 *
 * @param host host handler
 */
status_t MMC_CfgInitialize(sdmmc_mmc_t *sdmmc, sdmmchost_config_t *config);
#endif

#if defined(CONFIG_FSL_SDMMC_ENABLE_SDIO)
/*!
 * @brief Configure and initialize host controller for SDIO card
 *
 * Thread safe function, please note that the function will create the mutex lock dynamically by default,
 * so to avoid the mutex create redundantly, application must follow bellow sequence for card re-initialization
 *
 * @param host host handler
 */
status_t SDIO_CfgInitialize(sdmmc_sdio_t *sdmmc, sdmmchost_config_t *config);
#endif

#if defined(__cplusplus)
}
#endif
/* @} */
#endif /* _FSL_SDMMC_H */
