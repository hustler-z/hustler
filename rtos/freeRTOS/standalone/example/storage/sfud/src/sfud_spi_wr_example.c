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
 * FilePath: sfud_spi_wr_example.c
 * Date: 2023-07-31 11:23:42
 * LastEditTime: 2023-10-08 18:47:54
 * Description:  This file is for sfud spi read write example function implementation.
 *
 * Modify History:
 *  Ver       Who         Date           Changes
 * -----     ------     --------     --------------------------------------
 *  1.0    liqiaozhong  2023/8/1     add sfud read write example
 *  1.1    zhangyan     2023/8/7     modify
 *  1.2    liqiaozhong  2023/10/8    divide example into spi and qspi parts
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include <string.h>
#include <stdio.h>

#include "sfud.h"

#include "ftypes.h"
#include "fdebug.h"
#include "fkernel.h"
#include "fparameters.h"

#include "sfud_spi_wr_example.h"
/***************** Macros (Inline Functions) Definitions *********************/
#if defined CONFIG_TARGET_E2000Q || defined CONFIG_TARGET_E2000D
#define SFUD_CONTROLLER_ID  SFUD_FSPIM2_INDEX /* E2000 Default Use SPI2 id  */
#else
#define SFUD_CONTROLLER_ID  SFUD_FSPIM0_INDEX /* Default Use SPI0 id */
#endif

#define FLASH_WR_OFFSET     0x100 /* Flash write address offset */
#define SFUD_FLASH_LEN      20

#define FSFUD_DEBUG_TAG "FSFUD_EXAMPLE"
#define FSFUD_ERROR(format, ...) FT_DEBUG_PRINT_E(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSFUD_WARRN(format, ...) FT_DEBUG_PRINT_W(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSFUD_INFO(format, ...) FT_DEBUG_PRINT_I(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSFUD_DEBUG(format, ...) FT_DEBUG_PRINT_D(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static u8 write_buf[SFUD_FLASH_LEN + 1] = {0};
static u8 read_buf[SFUD_FLASH_LEN + 1] = {0};
static sfud_flash *flash = NULL;
/************************** Function Prototypes ******************************/

/******************************* Function ************************************/
/* example function used in cmd */
int FSfudSpiWRExample(void)
{
    FError ret = FT_SUCCESS;
    u32 rw_start_addr;

    /* this function will probe all spi and qspi controllers on board */
    if (sfud_init())
    {
        printf("Sfud init fail!");
        return SFUD_ERR_INIT_FAILED;
    }

    flash = sfud_get_device(SFUD_CONTROLLER_ID);
    if (flash == NULL || !flash->init_ok)
    {
        FSFUD_ERROR("Flash on spi controller %d is not found!", SFUD_CONTROLLER_ID);
        return SFUD_ERR_NOT_FOUND;
    }

    rw_start_addr = flash->chip.capacity - FLASH_WR_OFFSET;

    /* erase before read */
    ret = sfud_erase(flash, rw_start_addr, SFUD_FLASH_LEN);
    if (ret)
    {
        FSFUD_ERROR("Sfud erase fail.");
        return SFUD_ERR_WRITE;
    }

    ret = sfud_read(flash, rw_start_addr, SFUD_FLASH_LEN, read_buf);
    if (ret == 0)
    {
        printf("Before sfud write:\r\n");
        FtDumpHexByte(read_buf, SFUD_FLASH_LEN);
    }
    else
    {
        FSFUD_ERROR("Sfud read fail.");
        return SFUD_ERR_READ;
    }

    sprintf(write_buf, "write whatever u like to 0x%x", rw_start_addr);
    printf("%s\r\n", write_buf);
    ret = sfud_erase_write(flash, rw_start_addr, SFUD_FLASH_LEN, write_buf);
    if (ret)
    {
        FSFUD_ERROR("Sfud write fail.");
        return SFUD_ERR_WRITE;
    }

    ret = sfud_read(flash, rw_start_addr, SFUD_FLASH_LEN, read_buf);
    if (ret == 0)
    {
        printf("After sfud write:\r\n");
        FtDumpHexByte(read_buf, SFUD_FLASH_LEN);
    }
    else
    {
        FSFUD_ERROR("Sfud read fail.");
        return SFUD_ERR_READ;
    }
    
    if (ret == 0)
    {
        printf("%s@%d: Sfud spi read write example success!!!\r\n", __func__, __LINE__);
        printf("[system_example_pass]\r\n");
    }
    else
    {
        printf("%s@%d: Sfud spi read write example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
    }

    return ret;
}