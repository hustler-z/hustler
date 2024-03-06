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
 * FilePath: sata_pcie_fatfs_example.c
 * Date: 2023-08-07 14:53:42
 * LastEditTime: 2023-08-07 17:46:03
 * Description:  This file is for sata pcie fatfs example function implmentation
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

#include "sata_pcie_fatfs_example.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
/* variables used in example */
static ff_fatfs sata_pcie_fatfs;
static TCHAR path_buff[256];
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/

/* function of sata pcie fatfs example*/
int FSataPcieFatfsExample()
{
    int ret = 0;

    MKFS_PARM opt = {0};
    u32 sectors = 30000;
    /*cyc test time*/
    u32 cyc = 3;
    /*set the mount point*/
    const char *mount_point = FF_SATA_PCIE_DISK_MOUNT_POINT;
    /*set the mount point*/
    const char *file_path = "logfile.txt";
    /*format option (FM_FAT, FM_FAT32, FM_EXFAT and FM_SFD)*/
    opt.fmt = FM_EXFAT;

    sprintf(path_buff, "%s", mount_point);
    /*format and set up fatfs*/
    ret = ff_setup(&sata_pcie_fatfs, path_buff, &opt, TRUE);
    if (ret != 0)
    {
        printf("ff_setup failed.ret = %d\n", ret);
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

    /* print message on sata pcie fatfs example run result */
    if (0 == ret)
    {
        printf("%s@%d: sata pcie fatfs example [success].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: sata pcie fatfs example [failure].\r\n", __func__, __LINE__);
    }

    return 0;
}