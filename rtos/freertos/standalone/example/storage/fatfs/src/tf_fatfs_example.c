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
 * FilePath: tf_fatfs_example.c
 * Date: 2023-08-07 14:53:42
 * LastEditTime: 2023-08-07 17:46:03
 * Description:  This file is for tf fatfs example function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/8/3   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "ftypes.h"
#include "ff_utils.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#if defined(CONFIG_FATFS_SDMMC_FSDIF_TF) || defined(CONFIG_FATFS_SDMMC_FSDIF_EMMC)
#include "fsdif_timing.h"
#endif
#include "fsl_sdmmc.h"
#include "tf_fatfs_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
/* variables used in example */
static ff_fatfs tf_fatfs;
static TCHAR path_buff[256];
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/

/* function of tf fatfs example*/
int FTfFatfsExample()
{
    int ret = 0;

    MKFS_PARM opt = {0};
    u32 sectors = 1000;
    /*cyc test time*/
    u32 cyc = 3;

    /*set the mount point*/
#if defined(CONFIG_FATFS_SDMMC_FSDMMC_TF)
    const char *mount_point = FF_FSDMMC_TF_DISK_MOUNT_POINT;/*support FT2000/4,D2000*/
#elif defined(CONFIG_FATFS_SDMMC_FSDIF_TF)
    const char *mount_point = FF_FSDIF_TF_DISK_MOUNT_POINT;/*support E2000,PhytiumPi*/

    FSdifTimingInit();
#endif
    
    SDMMC_OSAInit();

    const char *file_path = "logfile.txt";
    /*format option (FM_FAT, FM_FAT32, FM_EXFAT and FM_SFD)*/
    opt.fmt = FM_EXFAT;

    sprintf(path_buff, "%s", mount_point);
    /*format and set up fatfs*/
    ret = ff_setup(&tf_fatfs, path_buff, &opt, TRUE);
    if (ret != 0)
    {
        printf("ff_setup failed.\n");
        return ret;
    }
    /*print relevant information after successful set up fatfs*/
    ff_dump_info(path_buff);

    /*speed bench*/
    ret = ff_speed_bench(path_buff, sectors);
    if (ret != 0)
    {
        printf("ff_speed_bench failed.\n");
        return ret;
    }
    /*basic test*/
    ret = ff_basic_test(mount_point, file_path);
    if (ret != 0)
    {
        printf("mount_point failed.\n");
        return ret;
    }
    /*cycle test*/
    ret = ff_cycle_test(path_buff, cyc);
    if (ret != 0)
    {
        printf("ff_cycle_test failed.\n");
        return ret;
    }

    SDMMC_OSADeInit();
#if defined CONFIG_FATFS_SDMMC_FSDIF_TF
    FSdifTimingDeinit();
#endif

    /* print message on tf fatfs example run result */
    if (0 == ret)
    {
        printf("%s@%d: tf fatfs example [success].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: tf fatfs example [failure].\r\n", __func__, __LINE__);
    }

    return 0;
}

/* function of tf fatfs example use one partition of card*/
int FTfFatfsReadOnlyExample()
{
    int ret = 0;
    FRESULT fr = FR_OK;
    static BYTE path[256];

    MKFS_PARM opt = {0};

    /*set the mount point*/
#if defined(CONFIG_FATFS_SDMMC_FSDMMC_TF)
    const char *mount_point = FF_FSDMMC_TF_DISK_MOUNT_POINT;/*support FT2000/4,D2000*/
#elif defined(CONFIG_FATFS_SDMMC_FSDIF_TF)
    const char *mount_point = FF_FSDIF_TF_DISK_MOUNT_POINT;/*support E2000,PhytiumPi*/

    FSdifTimingInit();
#endif
    
    SDMMC_OSAInit();

    /*format option (FM_FAT, FM_FAT32, FM_EXFAT and FM_SFD)*/
#if defined(CONFIG_TARGET_PHYTIUMPI)
    opt.fmt = FM_FAT32; /* use the 3rd partition of tf card, which is fat32 format */
#else
    opt.fmt = FM_EXFAT;
#endif

    sprintf(path_buff, "%s", mount_point);
    /*no format and just read filesystem*/
    ret = ff_setup(&tf_fatfs, path_buff, &opt, FALSE);
    if (ret != 0)
    {
        printf("ff_setup failed.\n");
        return ret;
    }

    /*print relevant information after successful set up fatfs*/
    ff_dump_info(path_buff);

    sprintf(path, "%s", mount_point); /* append dir */
    fr = ff_list_test(path);
    if (fr != FR_OK)
    {
        printf("ff_list_test %s failed, err = %d  !!! \r\n", path, fr);
        return fr;
    }

    SDMMC_OSADeInit();
#if defined CONFIG_FATFS_SDMMC_FSDIF_TF
    FSdifTimingDeinit();
#endif

    /* print message on tf fatfs example run result */
    if (0 == ret)
    {
        printf("%s@%d: tf fatfs example successfully !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: tf fatfs example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
    }

    return 0;
}