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
 * FilePath: sfud_probe_example.c
 * Date: 2023-07-31 11:23:42
 * LastEditTime: 2023-08-1 18:47:54
 * Description:  This file is for sfud probe example function implementation.
 *
 * Modify History:
 *  Ver       Who         Date           Changes
 * -----     ------     --------     --------------------------------------
 *  1.0    liqiaozhong  2023/8/1     add sfud probe example
 *  1.1    zhangyan     2023/8/8     modify
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

#include "sfud_qspi_probe_example.h"
/***************** Macros (Inline Functions) Definitions *********************/
#define SFUD_CONTROLLER_ID  SFUD_FQSPI0_INDEX /* Default Use QSPI id */

#define FSFUD_DEBUG_TAG "FSFUD_EXAMPLE"
#define FSFUD_ERROR(format, ...) FT_DEBUG_PRINT_E(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSFUD_WARRN(format, ...) FT_DEBUG_PRINT_W(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSFUD_INFO(format, ...) FT_DEBUG_PRINT_I(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSFUD_DEBUG(format, ...) FT_DEBUG_PRINT_D(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static const sfud_flash *flash = NULL;
/************************** Function Prototypes ******************************/

/******************************* Function ************************************/
static int SfudInfo(void)
{
    flash = sfud_get_device(SFUD_CONTROLLER_ID);
    if (flash == NULL || !flash->init_ok)
    {
        FSFUD_WARRN("Flash on qspi is not found!");
        return 1;
    }
        
    printf("Flash info on qspi:\r\n");
    printf("manufacturer id: 0x%x \r\n", flash->chip.mf_id);
    printf("memory-type id: 0x%x \r\n", flash->chip.type_id);
    printf("capacity id: 0x%x \r\n", flash->chip.capacity_id);
    if (flash->chip.capacity < SZ_1M)
    {
        printf("capacity: %d KB \r\n", flash->chip.capacity / SZ_1K);
    }
    else
    {
        printf("capacity: %d MB\r\n", flash->chip.capacity / SZ_1M);
    }
    printf("Erase granularity: %d Bytes\r\n", flash->chip.erase_gran);

    return 0;
}

/* function of sfud probe example */
int FSfudQspiProbeExample(void)
{
    FError ret = FT_SUCCESS;
    
    /* this function will probe all spi and qspi controllers on board */
    if (sfud_init())
    {
        printf("Sfud init fail!");
        ret = 1;
    }

    ret = SfudInfo();

    if (ret == 0)
    {
        printf("%s@%d: Sfud probe example [success].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: Sfud probe example [failure].\r\n", __func__, __LINE__);
    }

    return ret;
}