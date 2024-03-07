/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: cmd_ap.c
 * Date: 2022-07-12 09:33:12
 * LastEditTime: 2022-07-12 09:33:12
 * Description:  This file is for providing user command functions.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    first commit
 */
/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"

#include "FreeRTOS.h"

#include "../src/shell.h"
#include "wlan_ap_config.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
static void APCmdUsage()
{
    printf("Usage:\r\n");
    printf("    wlan_ap start\r\n");
    printf("        -- Start run the wireless access point\r\n");
}

static int APCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        APCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "start"))
    {
        if (pdPASS != FFreeRTOSStartAP())
        {
            return -2;
        }
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), wlan_ap, APCmdEntry, wireless ap functions);