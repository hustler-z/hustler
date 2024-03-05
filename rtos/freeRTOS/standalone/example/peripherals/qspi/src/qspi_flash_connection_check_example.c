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
 * FilePath: qspi_flash_connection_check_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for qspi flash connection check example function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/3/8   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "fkernel.h"
#include "ftypes.h"
#include "fparameters.h"

#include "fqspi_flash.h"
#include "fqspi.h"
#include "qspi_common.h"
#include "sdkconfig.h"
#include "fio_mux.h"

#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "qspi_flash_connection_check_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FQspiCtrl qspi_ctrl;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
/* function of qspi flash connection check example*/
int FQspiFlashConnectCheckExample()
{
    u32 qspi_id = FQSPI0_ID;
    u32 ret = FQSPI_SUCCESS;

#ifdef CONFIG_TARGET_E2000
    /*init iomux*/
    FIOMuxInit();
    FIOPadSetQspiMux(FQSPI0_ID, FQSPI_CS_0);
    FIOPadSetQspiMux(FQSPI0_ID, FQSPI_CS_1);
#endif

#if defined(TARDIGRADE)
 
#endif

    FQspiConfig pconfig = *FQspiLookupConfig(qspi_id);
    /* norflash init, include reset and read flash_size */
    ret = FQspiCfgInitialize(&qspi_ctrl, &pconfig);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Qspi init failed !\r\n");
        return FQSPI_NOT_READY;
    }
    else
    {
        FQSPI_DEBUG("Qspi init successed !\r\n");
    }

    /* detect connected flash infomation */
    ret = FQspiFlashDetect(&qspi_ctrl);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Qspi flash detect failed !\r\n");
        return FQSPI_NOT_READY;
    }
    else
    {
        FQSPI_DEBUG("Qspi flash detect successed !\r\n");
    }
    
    /*qspi deinit */
    FQspiDeInitialize(&qspi_ctrl);
#ifdef CONFIG_TARGET_E2000
    FIOMuxDeInit(); /*init iomux */
#endif

    /* print message on example run result */
    if (ret == FT_SUCCESS)
    {
        printf("%s@%d: qspi flash connection check example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: qspi flash connection check example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
        return FQSPI_NOT_SUPPORT;
    }

    return ret;
}