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
 * FilePath: cmd_fatfs.c
 * Date: 2023-08-07 14:53:42
 * LastEditTime: 2023-08-07 17:46:03
 * Description:  This file is for fatfs cmd catalogue
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/8/3   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

#if defined(CONFIG_FATFS_SDMMC_FSDIF_EMMC)
#include "emmc_fatfs_example.h"
#endif
#if defined(CONFIG_FATFS_RAM_DISK)
#include "ram_fatfs_example.h"
#endif
#if defined(CONFIG_FATFS_FSATA)
#include "sata_fatfs_example.h"
#endif
#if defined(CONFIG_FATFS_FSATA_PCIE)
#include "sata_pcie_fatfs_example.h"
#endif
#if defined(CONFIG_FATFS_SDMMC_FSDMMC_TF) || defined(CONFIG_FATFS_SDMMC_FSDIF_TF)
#include "tf_fatfs_example.h"
#endif

/* usage info function for fatfs example */
static void FFatfsExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("fatfs <storage>\r\n");
    printf("-- run fatfs example at controller\r\n");
    printf("-- support ram, tf, tf-ro, emmc, fsata, psata\r\n");
}

/* entry function for fatfs example */
static int FFatfsExampleEntry(int argc, char *argv[])
{
    int ret = 0;
    u32 id = 0U;
    
    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FFatfsExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
#if defined(CONFIG_FATFS_RAM_DISK)
    if (!strcmp(argv[1], "ram"))
    {
        ret = FRamFatfsExample();
    }
#endif
    
#if defined(CONFIG_FATFS_SDMMC_FSDMMC_TF) || defined(CONFIG_FATFS_SDMMC_FSDIF_TF)
    if (!strcmp(argv[1], "tf"))
    {
        ret = FTfFatfsExample();  
    }
    
    if (!strcmp(argv[1], "tf-ro")) /* no touch or format filesystem, just read out */
    {
        ret = FTfFatfsReadOnlyExample();  
    }
#endif
    
#if defined(CONFIG_FATFS_SDMMC_FSDIF_EMMC)
    if (!strcmp(argv[1], "emmc"))
    {
        ret = FEmmcFatfsExample();    
    }
#endif
    
#if defined(CONFIG_FATFS_FSATA_PCIE)
    if (!strcmp(argv[1], "psata"))
    {
        ret = FSataPcieFatfsExample();    
    }
#endif
    
#if defined(CONFIG_FATFS_FSATA)
    if (!strcmp(argv[1], "fsata"))
    {
        ret = FSataFatfsExample();    
    }
#endif

    return ret;
}

/* register command for xxx example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), fatfs, FFatfsExampleEntry, fatfs example);
#endif