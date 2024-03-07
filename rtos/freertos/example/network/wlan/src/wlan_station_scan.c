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
 * FilePath: wlan_station_scan.c
 * Date: 2022-07-12 09:53:00
 * LastEditTime: 2022-07-12 09:53:02
 * Description:  This file is for providing functions used in cmd_sf.c file.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2023/10/19    first commit
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "fdebug.h"
#include "fsleep.h"
#include "fkernel.h"

#include "wlan_common.h"
#include "wlan_station_scan.h"
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/* Link lost callback */
static void LinkStatusChangeCallback(bool linkState)
{
    if (linkState == false)
    {
        printf("-------- LINK LOST --------\r\n");
    }
    else
    {
        printf("-------- LINK REESTABLISHED --------\r\n");
    }
}

void WlanStationScanTask(void *param)
{
    FWlanRetStatus result;
    char *scan_result;

    result = FWlanInit();
    if (result != FWLAN_RET_SUCCESS)
    {
        printf("[!] FWlanInit: Failed, error: %d\r\n", (uint32_t)result);
        goto task_exit;
    }

    result = FWlanStart(LinkStatusChangeCallback); 
    if (result != FWLAN_RET_SUCCESS)
    {
        printf("[!] FWlanStart: Failed, error: %d\r\n", (uint32_t)result);
        goto task_exit;
    }

    scan_result = FWlanScan();
    if (scan_result == NULL)
    {
        printf("[!] FWlanScan: Failed to scan\r\n");
        goto task_exit;
    }

    vPortFree(scan_result);

task_exit:
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSWlanStationScanInit(void)
{
    /* Create the main Task */
    if (xTaskCreate(WlanStationScanTask, 
                    "WlanStationScanTask", 
                    4096, 
                    NULL, 
                    (UBaseType_t)configMAX_PRIORITIES - 1, 
                    NULL) != pdPASS)
    {
        printf("[!] MAIN Task creation failed!\r\n");
        return pdFAIL;
    }

    return pdPASS;
}