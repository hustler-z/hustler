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
 * FilePath: qspi_flash_polled_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for qspi flash polled example function implmentation
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

#include "qspi_flash_polled_example.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FQspiCtrl qspi_ctrl;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
/* function of qspi flash polled example */
int FQspiFlashPolledExample()
{
    u32 qspi_id = FQSPI0_ID;
    u32 ret = FQSPI_SUCCESS;
    u32 rw_start_addr;
    u8 read_buf[DAT_LENGTH];
    u8 write_buf[DAT_LENGTH] = {0};
    size_t read_len;

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

    /*set the write buffer content*/
    char *write_content = "qspi flash polled example test";
    u8 len = strlen(write_content) + 1;
    memcpy(&write_buf, write_content, len);

    /*set the flash rw addr*/
    rw_start_addr = qspi_ctrl.flash_size - FLASH_WR_OFFSET;
    /* erase norflash */
    ret = FQspiFlashErase(&qspi_ctrl, FQSPI_FLASH_CMD_SE, rw_start_addr);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Failed to erase mem, test result 0x%x.\r\n", ret);
        return FQSPI_NOT_READY;
    }

    /* write norflash data */
    ret = FQspiFlashWriteData(&qspi_ctrl, FQSPI_FLASH_CMD_QPP, rw_start_addr, write_buf, sizeof(write_buf));
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Failed to write mem, test result 0x%x.\r\n", ret);
        return FQSPI_NOT_READY;
    }
    else
    {
        printf("Write success!\r\n");
    }

    /*Read norflash through 1-1-1 normal reading data*/
    ret = FQspiFlashReadDataConfig(&qspi_ctrl, FQSPI_FLASH_CMD_READ);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Failed to config read, test result 0x%x.\r\n", ret);
        return FQSPI_NOT_READY;
    }
    read_len = FQspiFlashReadData(&qspi_ctrl,rw_start_addr, read_buf, DAT_LENGTH);
    if (read_len != sizeof(read_buf))
    {
        FQSPI_ERROR("Failed to read mem, read len = %d.\r\n", read_len);
        return FQSPI_NOT_READY;
    }
    else
    {
        printf("Read norflash through 0x03 instructs (normal read) successfully!\r\n");
    }
    for (u8 i = 0; i < sizeof(read_buf); i++)
    {
        if (read_buf[i] != write_buf[i])
        {
            FQSPI_ERROR("The read and write data is inconsistent.\r\n");
            return FQSPI_NOT_SUPPORT;
        }
    }

    /*Read norflash through 1-1-2 dual output fast read reading data*/
    ret = FQspiFlashReadDataConfig(&qspi_ctrl, FQSPI_FLASH_CMD_DOR);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Failed to config read, test result 0x%x.\r\n", ret);
        return FQSPI_NOT_READY;
    }
    read_len = FQspiFlashReadData(&qspi_ctrl,rw_start_addr, read_buf, DAT_LENGTH);
    if (read_len != sizeof(read_buf))
    {
        FQSPI_ERROR("Failed to read mem, read len = %d.\r\n", read_len);
        return FQSPI_NOT_READY;
    }
    else
    {
        printf("Read norflash through 0x3B instructs (dual output fast read) successfully!\r\n");
    }
    for (u8 i = 0; i < sizeof(read_buf); i++)
    {
        if (read_buf[i] != write_buf[i])
        {
            FQSPI_ERROR("The read and write data is inconsistent.\r\n");
            return FQSPI_NOT_SUPPORT;
        }
    }

    /*Read norflash through 1-1-4 quad output fast read reading data*/
    ret = FQspiFlashReadDataConfig(&qspi_ctrl, FQSPI_FLASH_CMD_QOR);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Failed to config read, test result 0x%x.\r\n", ret);
        return FQSPI_NOT_READY;
    }
    read_len = FQspiFlashReadData(&qspi_ctrl,rw_start_addr, read_buf, DAT_LENGTH);
    if (read_len != sizeof(read_buf))
    {
        FQSPI_ERROR("Failed to read mem, read len = %d.\r\n", read_len);
        return FQSPI_NOT_READY;
    }
    else
    {
        printf("Read norflash through 0x6B instructs (quad output fast read) successfully!\r\n");
    }
    for (u8 i = 0; i < sizeof(read_buf); i++)
    {
        if (read_buf[i] != write_buf[i])
        {
            FQSPI_ERROR("The read and write data is inconsistent.\r\n");
            return FQSPI_NOT_SUPPORT;
        }
    }

    /*print the read content*/
    FtDumpHexByte(read_buf, DAT_LENGTH);
    /*qspi deinit */
    FQspiDeInitialize(&qspi_ctrl);
#ifdef CONFIG_TARGET_E2000
    FIOMuxDeInit(); /*init iomux */
#endif

    /* print message on example run result */
    if (ret == FT_SUCCESS)
    {
        printf("%s@%d: qspi flash polled example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: qspi flash polled example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
        return FQSPI_NOT_SUPPORT;
    }
    
    return ret;
}