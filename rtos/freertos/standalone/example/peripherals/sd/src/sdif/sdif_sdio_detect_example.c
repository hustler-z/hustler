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
 * FilePath: sdif_sdio_detect_example.c
 * Date: 2023-04-05 15:22:56
 * LastEditTime: 2023-04-07 13:49:25
 * Description:  This file is for sdio sdio card detect example function implmentation.
 *
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------    --------------------------------------
 *  1.0     zhugengyu  2023/10/23    first commit with detect examples
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "ftypes.h"
#include "fdebug.h"
#include "fassert.h"
#include "fio.h"
#include "fkernel.h"
#include "fparameters.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "fio_mux.h"
#include "fgpio.h"

#include "fsdif_timing.h"
#include "fsl_sdmmc.h"
#include "sdif_common.h"
#include "sdif_sdio_detect_example.h"
/************************** Constant Definitions *****************************/
#define MARVELL_VENDOR_ID 	0x02df

/** Device ID for SD8797 */
#define SD_DEVICE_ID_8797   (0x9129)
/** Device ID for SD8782 */
#define SD_DEVICE_ID_8782   (0x9121)
/** Device ID for SD8801 */
#define SD_DEVICE_ID_8801   (0x9139)

#ifndef CONFIG_TARGET_PHYTIUMPI
#define SDIO_HOST_ID FSDIF1_ID;
#else
#define SDIO_HOST_ID FSDIF0_ID;
#endif

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static sdmmchost_config_t s_inst_config;
static sdmmc_sdio_t s_inst;
static FGpio gpio;
static FGpioConfig gpio_config;
static FGpioPinId PDn_index;
static FGpioPin PDn; /* external PDn assertion */
/***************** Macros (Inline Functions) Definitions *********************/
#define FSD_EXAMPLE_TAG "FSD_EXAMPLE"
#define FSD_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_WARN(format, ...)    FT_DEBUG_PRINT_W(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_INFO(format, ...)    FT_DEBUG_PRINT_I(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/

/************************** Function *****************************************/

/* according to mw8801 tech manual, extern PDn assertion is a way to reset the module */
static void FSdifSdioMW8801PowerUp(void)
{
    gpio_config = *FGpioLookupConfig(FGPIO4_ID);
    (void)FGpioCfgInitialize(&gpio, &gpio_config);

    PDn_index.ctrl = FGPIO4_ID;
    PDn_index.port = FGPIO_PORT_A;
    PDn_index.pin  = FGPIO_PIN_11;

    FIOPadSetGpioMux(PDn_index.ctrl, (u32)PDn_index.pin);

    (void)FGpioPinInitialize(&gpio, &PDn, PDn_index);
    (void)FGpioSetDirection(&PDn, FGPIO_DIR_OUTPUT);

    /* transitions from low to high for PDn pin to reset sdio card */
    FGpioSetOutputValue(&PDn, FGPIO_PIN_LOW);
    SDMMC_OSADelay(50);
    FGpioSetOutputValue(&PDn, FGPIO_PIN_HIGH);
    SDMMC_OSADelay(50);

    FGpioDeInitialize(&gpio);
}

void FSdifSdioCardIntCallback(void *data)
{
    printf("SDIO Card interrupt \r\n");
}

int FSdifSdioCardDetectExample(void)
{
    int ret = 0;
    status_t err = 0;
    
    FSdifTimingInit();
#ifndef CONFIG_TARGET_PHYTIUMPI /* phytium pi embedded rtl8821cs sdio instead */
    FSdifSdioMW8801PowerUp();
#endif
    SDMMC_OSAInit();

    memset(&s_inst_config, 0, sizeof(s_inst_config));
    memset(&s_inst, 0, sizeof(s_inst));

    s_inst_config.hostId = SDIO_HOST_ID;
    s_inst_config.hostType = kSDMMCHOST_TYPE_FSDIF;
    s_inst_config.cardType = kSDMMCHOST_CARD_TYPE_SDIO;
    s_inst_config.enableDMA = SD_WORK_DMA;
    s_inst_config.enableIrq = SD_WORK_IRQ;
    s_inst_config.timeTuner = FSdifGetTimingSetting;
    s_inst_config.endianMode = kSDMMCHOST_EndianModeLittle;
    s_inst_config.maxTransSize = SD_MAX_RW_BLK * SDIO_BLOCK_SIZE;
    s_inst_config.defBlockSize = SD_BLOCK_SIZE;
    s_inst_config.cardClock = SD_CLOCK_25MHZ;
    s_inst_config.isUHSCard = FALSE;
    s_inst_config.sdioCardInt = FSdifSdioCardIntCallback;
    s_inst_config.sdioCardIntArg = NULL;
    /* set SDIO interrupt pin mux */
    FIOPadSetSdMux(s_inst_config.hostId);

    err = SDIO_CfgInitialize(&s_inst, &s_inst_config);
    if (kStatus_Success != err)
    {
        FSD_ERROR("Init SDIO failed, err = %d !!!", err);
        goto err_exit;
    }
    
err_exit:
    if (0 == err)
    {
        printf("%s@%d: SDIO card detect example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: SDIO card detect example failed !!!, err = %d \r\n", __func__, __LINE__, err);
    }

    SDIO_Deinit(&s_inst.card);
    SDMMC_OSADeInit();
    FSdifTimingDeinit();

    return err;
}
