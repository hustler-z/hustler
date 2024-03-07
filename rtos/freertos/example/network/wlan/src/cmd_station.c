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
 * FilePath: cmd_station.c
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
#include "wlan_station_scan.h"
#include "wlan_station_connect.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
static void WlanCmdUsage()
{
    printf("Usage:\r\n");
    printf("    wlan_station scan\r\n");
    printf("        -- Scan nearby wlan station\r\n");
    printf("    wlan_station join [ssid] [password] [remote-ip]\r\n");
    printf("        -- Join a wlan station in scanned station list\r\n");
    printf("    wlan_station ping [remote-ip]\r\n");
    printf("        -- Ping remote-ip after join a wlan station\r\n");
}

static int WlanCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        WlanCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "scan"))
    {
        if (pdPASS != FFreeRTOSWlanStationScanInit())
        {
            return -2;
        }
    }
    else if (!strcmp(argv[1], "join"))
    {
        if (argc < 4)
        {
            return -3;
        }

        if (pdPASS != FFreeRTOSWlanStationConnectInit(argv[2], argv[3]))
        {
            return -2;
        }
    }
    else if (!strcmp(argv[1], "ping"))
    {
        if (argc < 3)
        {
            return -3;
        }
    
        if (pdPASS != FFreeRTOSWlanStationPing(argv[2]))
        {
            return -2;
        }
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), wlan_station, WlanCmdEntry, wlan functions);