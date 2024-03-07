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
 * FilePath: wlan_station_connect.c
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

#include "lwip/tcpip.h"
#include "ping.h"

#include "wlan_common.h"
#include "wlan_station_connect.h"
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/
static char ssid[FWLAN_WIFI_SSID_LENGTH];
static char password[FWLAN_WIFI_PASSWORD_LENGTH];
static const char *remote_ip_str = '\0';
static ip_addr_t remote_ip;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/

static FWlanRetStatus promptJoinNetwork(void)
{
    FWlanRetStatus result;
    int i = 0;
    char ch;

    /* SSID prompt */
    printf("\r\nSSID: %s \r\n", ssid);
    /* Password prompt */
    printf("\r\nPassword: ***** \r\n");

    /* Add Wifi network as known network */
    result = FWlanAddNetwork(ssid, password, ssid);
    if (result != FWLAN_RET_SUCCESS)
    {
        printf("[!] FWlanAddNetwork: Failed to add network, error:  %d\r\n", (uint32_t)result);
        return FWLAN_RET_FAIL;
    }
    printf("[i] FWlanAddNetwork: Success\r\n");

    /* Join the network using label */
    printf("[i] Trying to join the network...\r\n");
    result = FWlanJoin(ssid);
    if (result != FWLAN_RET_SUCCESS)
    {
        printf("[!] FWlanJoin: Failed to join network, error: %d\r\n", (uint32_t)result);
        if (FWlanRemoveNetwork(ssid) != FWLAN_RET_SUCCESS)
            return FWLAN_RET_FAIL;
    }
    printf("[i] FWlanJoin: Success\r\n");

    /* SSID and password was OK, exit the prompt */

    return FWLAN_RET_SUCCESS;
}

static void WlanStationConnect(void *param)
{
    /* prompt user to input a station to join */
    if (FWLAN_RET_SUCCESS != promptJoinNetwork())
    {
        printf("[i] WlanStationConnect: Join network failed\r\n");
        goto task_exit;
    }

task_exit:
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSWlanStationConnectInit(const char *usr_ssid, const char *usr_password)
{
    if ((strlen(usr_ssid) >= FWLAN_WIFI_SSID_LENGTH) ||
        (strlen(usr_password) >= FWLAN_WIFI_PASSWORD_LENGTH))
    {
        printf("[!] Invalid input parameters\r\n");
        return pdFAIL;
    }

    memcpy(ssid, usr_ssid, strlen(usr_ssid));
    ssid[strlen(usr_ssid)] = '\0';
    memcpy(password, usr_password, strlen(usr_password));
    password[strlen(usr_password)] = '\0';

    /* Create the main Task */
    if (xTaskCreate(WlanStationConnect, 
                    "WlanStationConnect", 
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

static void WlanStationPing(void *param)
{
    /* Ping IP address prompt */
    printf("\r\nIP address: %s\r\n", remote_ip_str);
    ping_init(&remote_ip, 10U);

    while (TRUE)
    {
        vTaskDelay(1000);
    }

task_exit:
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSWlanStationPing(const char *usr_remote_ip)
{
    remote_ip_str = usr_remote_ip;
    if (ipaddr_aton(usr_remote_ip, &remote_ip) == 0)
    {
        return pdFAIL;
    }

    /* Create the main Task */
    if (xTaskCreate(WlanStationPing, 
                    "WlanStationPing", 
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