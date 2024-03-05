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
 * FilePath: qspi_flash_indirect_example.c
 * Date: 2023-11-13 14:53:42
 * LastEditTime: 2023-11-13 17:46:03
 * Description:  This file is an example function implementation for the indirect mode of qspi flash

 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2023/11/13   first release
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

#include "qspi_flash_indirect_example.h"

/************************** Constant Definitions *****************************/
const u8 Text_Buffer[]={"qspi flash indirect example test!!!"};
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FQspiCtrl qspi_ctrl;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
/* function of qspi flash indirect example */
int FQspiFlashIndirectExample()
{
    u32 qspi_id = FQSPI0_ID;
    u32 ret = FQSPI_SUCCESS;
    u32 rw_start_addr;
    u32 write_addr;
    u8 read_buf[DAT_LENGTH];
    u32 array_index = 0;
    int i = 0;

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
    /* set to channel CSN0 */
    FQspiChannelSet(&qspi_ctrl, FQSPI_CS_0);
    /*set the flash rw addr*/
    rw_start_addr = qspi_ctrl.flash_size - FLASH_WR_OFFSET;
    write_addr = rw_start_addr;
    /* erase norflash */
    ret = FQspiFlashErase(&qspi_ctrl, FQSPI_FLASH_CMD_SE, rw_start_addr);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Failed to erase mem, test result 0x%x.\r\n", ret);
        return FQSPI_NOT_READY;
    }

    /* Write norflash data */
    while (array_index < sizeof(Text_Buffer))
    {
        u8 data_to_write[4] = {0};
        for (i = 0; i < 4; i++)
        {
            if (array_index < sizeof(Text_Buffer))
            {
                data_to_write[i] = Text_Buffer[array_index];
                array_index++;
            }
            else
            {
                break;
            }
        }
        ret = FQspiFlashPortWriteData(&qspi_ctrl, FQSPI_FLASH_CMD_PP, write_addr, (u8 *)(data_to_write), 4);
        write_addr += 4;
    }
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Failed to write mem, test result 0x%x.\r\n", ret);
        return FQSPI_NOT_READY;
    }
    else
    {
        printf("FQspiFlashPortWriteData success!\r\n");
    }

    /* Read norflash through 1-1-1 normal reading data */
    ret = FQspiFlashPortReadData(&qspi_ctrl, FQSPI_FLASH_CMD_READ, rw_start_addr, read_buf, DAT_LENGTH);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Failed to read mem, read len = %d.\r\n", DAT_LENGTH);
        return FQSPI_NOT_READY;
    }
    else
    {
        printf("Read norflash through 0x03 instructs (normal read) successfully!\r\n");
    }
    FtDumpHexByte(read_buf, DAT_LENGTH);
    for (u8 i = 0; i < sizeof(Text_Buffer); i++)
    {
        if (read_buf[i] != Text_Buffer[i])
        {
            FQSPI_ERROR("The read and write data is inconsistent.\r\n");
            return FQSPI_NOT_SUPPORT;
        }
    }

    /*qspi deinit */
    FQspiDeInitialize(&qspi_ctrl);
#ifdef CONFIG_TARGET_E2000
    FIOMuxDeInit(); /*deinit iomux */
#endif

    /* print message on example run result */
    if (ret == FT_SUCCESS)
    {
        printf("%s@%d: qspi flash indirect example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: qspi flash indirect example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
        return FQSPI_NOT_SUPPORT;
    }
    
    return ret;
}