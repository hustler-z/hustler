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
 * FilePath: cmd_sata_controller.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for sata controller cmd catalogue
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0   zhangyan   2023/3/31   first release
 */

/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

#include "sata_controller_pio_example.h"
#include "sata_controller_fpdma_example.h"
#include "sata_controller_pio_intr_example.h"

/* usage info function for sata controller example */
static void FSataControllerExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("fsata pio \r\n");
    printf("-- run sata controller pio example\r\n");
    printf("fsata fpdma \r\n");
    printf("-- run sata controller fpdma example\r\n");
    printf("fsata pio_intr \r\n");
    printf("-- run sata controller pio intr example\r\n");
}

/* entry function for sata controller example */
static int FSataControllerExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FSataControllerExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "pio"))
    {
        ret = FSataControllerPioExample();
    }
    else if (!strcmp(argv[1], "fpdma"))
    {
        ret = FSataControllerFpdmaExample();    
    }
    else if (!strcmp(argv[1], "pio_intr"))
    {
        ret = FSataControllerPioIntrExample();    
    }

    return ret;
}

/* register command for sata controller example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), fsata, FSataControllerExampleEntry, sata controller example);
#endif