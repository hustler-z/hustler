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
 * FilePath: qspi_dual_flash_stack_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for qspi dual flash stack example function implmentation
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

#include "qspi_dual_flash_stack_example.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FQspiCtrl qspi_ctrl;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
static FError FQspiFlashWriteThenRead(FQspiCtrl *pctrl, u8 channel, u32 chip_addr, u8 write_command, u8 read_command, u8 *write_buf, u8 len)
{
    u32 ret = FQSPI_SUCCESS;
    u8 read_buf[DAT_LENGTH];
    size_t read_len;

    FQspiChannelSet(pctrl, channel);
    /* erase norflash */
    ret = FQspiFlashErase(pctrl, FQSPI_FLASH_CMD_SE, chip_addr);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Failed to erase CSN%d mem, test result 0x%x.\r\n", channel, ret);
        return FQSPI_NOT_READY;
    }

    /* write norflash data */
    ret = FQspiFlashWriteData(pctrl, write_command, chip_addr, write_buf, len);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Failed to write CSN%d mem, test result 0x%x.\r\n", channel, ret);
        return FQSPI_NOT_READY;
    }
    else
    {
        printf("Write success!\r\n");
    }
    /*Read norflash data*/
    ret = FQspiFlashReadDataConfig(pctrl, read_command);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Failed to config read CSN%d, test result 0x%x.\r\n", channel, ret);
        return FQSPI_NOT_READY;
    }
    read_len = FQspiFlashReadData(pctrl,chip_addr, read_buf, DAT_LENGTH);
    if (read_len != sizeof(read_buf))
    {
        FQSPI_ERROR("Failed to read CSN%d mem, read len = %d.\r\n", channel, read_len);
        return FQSPI_NOT_READY;
    }
    else
    {
        printf("Read CSN%d norflash successfully!\r\n", channel);
    }
    /*print the read content*/
    FtDumpHexByte(read_buf, DAT_LENGTH);

    return ret;
}

/* function of qspi flash qspi dual flash stack example */
int FQspiDualFlashStackExample()
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

    char *cs0_write_content= "CSN0 write content successfully";
    char *cs1_write_content= "CSN1 write content successfully";
   
    /*set the flash rw addr*/
    rw_start_addr = qspi_ctrl.flash_size - FLASH_WR_OFFSET;

    /*set the CSN0 write buffer content*/
    u8 len = strlen(cs0_write_content) + 1;
    memcpy(&write_buf, cs0_write_content, len);
    /*Use CSN0 for reading and writing*/
    FQspiFlashWriteThenRead(&qspi_ctrl, FQSPI_CS_0, rw_start_addr,FQSPI_FLASH_CMD_QPP, FQSPI_FLASH_CMD_QIOR, write_buf, len);
    
    /*set the CSN1 write buffer content*/
    len = strlen(cs1_write_content) + 1;
    memcpy(&write_buf, cs1_write_content, len);
    /*Use CSN1 for reading and writing*/
    FQspiFlashWriteThenRead(&qspi_ctrl, FQSPI_CS_1, rw_start_addr,FQSPI_FLASH_CMD_QPP, FQSPI_FLASH_CMD_QIOR, write_buf, len);

    /*qspi deinit */
    FQspiDeInitialize(&qspi_ctrl);
#ifdef CONFIG_TARGET_E2000
    FIOMuxDeInit(); /*init iomux */
#endif
    /* print message on example run result */
    if (ret == FT_SUCCESS)
    {
        printf("%s@%d: qspi dual flash stack example success !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: qspi dual flash stack example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
        return FQSPI_NOT_SUPPORT;
    }
    
    return ret;
}