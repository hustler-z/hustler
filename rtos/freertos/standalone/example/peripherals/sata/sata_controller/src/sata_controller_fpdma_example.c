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
 * FilePath: sata_controller_fpdma_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for sata controller fpdma example functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/3/31   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fparameters.h"
#include "fsata.h"
#include "fsata_hw.h"
#include "ftypes.h"
#include "finterrupt.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "sata_controller_common.h"
#include "sata_controller_fpdma_example.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
/*test default data*/
static u8 ahci_host = FSATA_CONTROLLER_TEST_AHCI_HOST;
static u32 start_blk = FSATA_CONTROLLER_TEST_BLOCK;
static u16 blk_num = FSATA_CONTROLLER_TEST_BLOCK_NUM;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
/* function of sata controller fpdma example */

int FSataControllerFpdmaExample()
{   
    FError ret = FT_SUCCESS;

    /*init sata controller */
    ret = FSataControllerInit(ahci_host);
    if (ret != FT_SUCCESS)
    {
        FSATA_ERROR("Sata init failed, please check if the sata is successfully connected\r\n");
        return FSATA_UNKNOWN_DEVICE;
    }

    printf("Test sata read and write in host %d block %d\n", ahci_host, start_blk);

    /*write data*/
    char write_buf[FSATA_BLOCK_SIZE] = "Sata Controller Fpdma Write Successfully!!!";
    ret = FSataControllerReadWrite(ahci_host, start_blk, blk_num, write_buf, SATA_FPDMA_WRITE_MODE);
    if (ret != FT_SUCCESS)
    {
        FSATA_ERROR("Sata conrtoller fpdma write failed !!! \r\n");
        return FSATA_ERR_OPERATION;
    }

    /*read data*/
    char read_buf[FSATA_BLOCK_SIZE];
    ret = FSataControllerReadWrite(ahci_host, start_blk, blk_num, read_buf, SATA_FPDMA_READ_MODE);
    if (ret != FT_SUCCESS)
    {
        FSATA_ERROR("Sata conrtoller fpdma read failed !!! \r\n");
        return FSATA_ERR_OPERATION;
    }
    /*printf the read_buf*/
    FtDumpHexByte(read_buf, FSATA_BLOCK_SIZE * blk_num);
    /*deinit sata controller*/
    FSataControllerDeinit(ahci_host);

    /* print message on example run result */
    if (ret == FT_SUCCESS)
    {
        printf("%s@%d: Sata controller fpdma example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: Sata controller fpdma example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
        return FSATA_ERR_OPERATION;
    }
    
    return ret;
}