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
 * FilePath: spim_intr_loopback_mode_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for spim intr loopback mode example function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/4/12   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "ftypes.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "spim_common.h"
#include "spim_intr_loopback_mode_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
/*spim test id*/
static u32 spi_id = FSPIM_TEST_ID;
/*spim test bytes length*/
static u32 bytes = FSPIM_TEST_BYTES_LENGTH;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
/* function of spim intr loopback mode example*/
int FSpimTxRxInterrupt(FSpim *spim, u8 *tx_buf, fsize_t tx_len, u8 *rx_buf, fsize_t rx_len)
{
    FASSERT_MSG((tx_len == rx_len), "tx_len != rx_len");
    int ret = FSPIM_OPS_OK;
    FSpim *spim_p = spim;
    FError err = FSPIM_SUCCESS;

    FSpimCsOnOff(FSPIM_CS_ON);

    err = FSpimTransferByInterrupt(spim_p, tx_buf, rx_buf, tx_len);
    if (FSPIM_SUCCESS != err)
    {
        ret = FSPIM_OPS_TRANS_FAILED;
    }

    if (FSPIM_OPS_OK != FSpimWaitRxDone(FSPIM_DELAY_MS))
    {
        FSPIM_DEBUG("Wait for rx timeout !!!");
        ret = FSPIM_OPS_TRANS_TIMEOUT;
    }

    FSpimCsOnOff(FSPIM_CS_OFF);
    return ret;
}

int FSpimIntrLoopbackExample()
{
    FError ret = FT_SUCCESS;

    /*spim init and configure mode*/
    ret = FSpimOpsInit(spi_id, SPIM_TEST_MODE_ENABLE, FSPIM_DMA_DISABLE, FSPIM_BIT_16_ENABLE);
    if (ret != FT_SUCCESS)
    {
        FSPIM_ERROR("Spim init failed!!!\r\n");
        return FSPIM_ERR_NOT_READY;
    }

    /*spim interrupt init and loopback mode test*/
    ret = FSpimLoopBack(bytes, FSPIM_OPS_LOOPBACK_INTERRUPT);
    if (ret != FT_SUCCESS)
    {
        FSPIM_ERROR(" Spim intr loopback test failed!!!\r\n");
        return FSPIM_ERR_TRANS_FAIL;
    }

    /*spim deinit*/
    FSpimOpsDeInit();

    if (ret == FT_SUCCESS)
    {
        printf("%s@%d: Spim intr loopback mode example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: Spim intr loopback mode example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
        return FSPIM_ERR_TRANS_FAIL;
    }

    return 0;
}