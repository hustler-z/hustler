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
 * FilePath: wlan_ap_config.c
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
#include <assert.h>

#include "FreeRTOS.h"
#include "task.h"

#include "fdebug.h"
#include "fsleep.h"
#include "fkernel.h"

#include "wlan_common.h"
#include "wlan_ap_config.h"
/************************** Constant Definitions *****************************/
/* Parameters that apply to AP mode only */
#ifndef WIFI_AP_CHANNEL
#define WIFI_AP_CHANNEL 1
#endif

#define MAX_RETRY_TICKS 50

typedef enum
{
    WIFI_STATE_CLIENT,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CLIENT_SCAN,
    WIFI_STATE_AP,
    WIFI_STATE_AP_SCAN,
} WifiBoardStates;

struct WifiBoardStatesVariables
{
    WifiBoardStates wifiState;
    char ssid[FWLAN_WIFI_SSID_LENGTH];
    char password[FWLAN_WIFI_PASSWORD_LENGTH];
    char security[FWLAN_WIFI_SECURITY_LENGTH];
    bool connected;
    TaskHandle_t mainTask;
};
/************************** Variable Definitions *****************************/
static char ssid[FWLAN_WIFI_SSID_LENGTH] = FWLAN_WIFI_SSID;
static char password[FWLAN_WIFI_PASSWORD_LENGTH] = FWLAN_WIFI_PASSWORD;
static char security[FWLAN_WIFI_SECURITY_LENGTH] = FWLAN_WIFI_SECURITY;
struct WifiBoardStatesVariables g_board_state;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/* Link lost callback */
static void LinkStatusChangeCallback(bool linkState)
{
    if (linkState == false)
    {
        /* -------- LINK LOST -------- */
        /* DO SOMETHING */
        printf("-------- LINK LOST --------\r\n");
    }
    else
    {
        /* -------- LINK REESTABLISHED -------- */
        /* DO SOMETHING */
        printf("-------- LINK REESTABLISHED --------\r\n");
    }
}

/* Initialize and start local AP */
static uint32_t SetBoardToAP()
{
    uint32_t result;

    /* Set the global ssid and password to the default AP ssid and password */
    strcpy(g_board_state.ssid, FWLAN_WIFI_SSID);
    strcpy(g_board_state.password, FWLAN_WIFI_PASSWORD);

    /* Start the access point */
    printf("Starting Access Point: SSID: %s, Chnl: %d\r\n", g_board_state.ssid, WIFI_AP_CHANNEL);
    result = FWlanStartAP(g_board_state.ssid, g_board_state.password, WIFI_AP_CHANNEL);

    if (result != FWLAN_RET_SUCCESS)
    {
        printf("[!] Failed to start access point, err = %d\r\n", result);
        assert(0);
    }
    g_board_state.connected = true;

    char ip[16];
    FWlanGetIP(ip, 0);
    printf(" Now join that network on your device and connect to this IP: %s\r\n", ip);

    return 0;
}

/* Connect to the external AP in g_board_state.ssid */
static uint32_t SetBoardToClient()
{
    int32_t result;
    /* If we are already connected, skip the initialization */
    if (!g_board_state.connected)
    {
        /* Add Wi-Fi network */
        if (strstr(g_board_state.security, "WPA3_SAE"))
        {
            result = FWlanAddNetworkWithSecurity(g_board_state.ssid, g_board_state.password, FWLAN_WIFI_NETWORK_LABEL, FWLAN_SECURITY_WPA3_SAE);
        }
        else
        {
            result = FWlanAddNetworkWithSecurity(g_board_state.ssid, g_board_state.password, FWLAN_WIFI_NETWORK_LABEL, FWLAN_SECURITY_WILDCARD);
        }
        if (result == FWLAN_RET_SUCCESS)
        {
            printf("Connecting as client to ssid: %s with password %s\r\n", g_board_state.ssid, g_board_state.password);
            result = FWlanJoin(FWLAN_WIFI_NETWORK_LABEL);
        }

        if (result != FWLAN_RET_SUCCESS)
        {
            printf("[!] Cannot connect to Wi-Fi\r\n[!]ssid: %s\r\n[!]passphrase: %s\r\n", g_board_state.ssid,
                   g_board_state.password);
            char c;
            do
            {
                printf("[i] To reset the board to AP mode, press 'r'.\r\n");
                printf("[i] In order to try connecting again press 'a'.\r\n");

                do
                {
                    c = GETCHAR();
                    // Skip over \n and \r and don't print the prompt again, just get next char
                } while (c == '\n' || c == '\r');

                switch (c)
                {
                    case 'r':
                    case 'R':
                        {
                            // Reset back to AP mode
                            g_board_state.wifiState = WIFI_STATE_AP;
                            return 0;
                        }
                        break;
                    case 'a':
                    case 'A':
                        // Try connecting again...
                        return 0;
                    default:
                        printf("Unknown command %c, please try again.\r\n", c);
                }

            } while (1);
        }
        else
        {
            printf("[i] Connected to Wi-Fi\r\nssid: %s\r\n[!]passphrase: %s\r\n", g_board_state.ssid,
                   g_board_state.password);
            g_board_state.connected = true;
            char ip[16];
            FWlanGetIP(ip, 1);
            printf(" Now join that network on your device and connect to this IP: %s\r\n", ip);
        }
    }
    return 0;
}

/* Clean up the local AP after waiting for all tasks to clean up */
static uint32_t CleanUpAP()
{
    /* Give time for reply message to reach the web interface before destorying the conection */
    vTaskDelay(10000 / portTICK_PERIOD_MS);

    printf("[i] Stopping AP!\r\n");
    if (FWlanStopAP() != FWLAN_RET_SUCCESS)
    {
        printf("Error while stopping ap\r\n");
        assert(0);
    }

    return 0;
}

/* Wait for any transmissions to finish and clean up the Client connection */
static uint32_t CleanUpClient(void)
{
    /* Give time for reply message to reach the web interface before destroying the connection */
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    /* Leave the external AP */
    if (FWlanLeave() != FWLAN_RET_SUCCESS)
    {
        printf("[!] Error Leaving from Client network.\r\n");
        assert(0);
    }

    /* Remove the network profile */
    if (FWlanRemoveNetwork(FWLAN_WIFI_NETWORK_LABEL) != FWLAN_RET_SUCCESS)
    {
        printf("[!] Failed to remove network profile.\r\n");
        assert(0);
    }

    return 0;
}

static void StartAP(void *arg)
{
    uint32_t result = 1;

    /* Credentials from last time have been found. The board will attempt to
        * connect to this network as a client */
    printf("[i] AP SSID: %s, Password: %s, Security: %s\r\n", ssid, password, security);

    memset(g_board_state.ssid, 0U, sizeof(g_board_state.ssid));
    memset(g_board_state.password, 0U, sizeof(g_board_state.password));
    memset(g_board_state.security, 0U, sizeof(g_board_state.security));

    g_board_state.connected = false;
    g_board_state.wifiState = WIFI_STATE_AP;
    strcpy(g_board_state.ssid, ssid);
    strcpy(g_board_state.password, password);
    strcpy(g_board_state.security, security);

    /* Initialize Wi-Fi board */
    printf("[i] Initializing Wi-Fi connection... \r\n");

    result = FWlanInit();
    if (result != FWLAN_RET_SUCCESS)
    {
        printf("[!] WPL Init failed: %d\r\n", (uint32_t)result);
        goto task_exit;
    }

    result = FWlanStart(LinkStatusChangeCallback);
    if (result != FWLAN_RET_SUCCESS)
    {
        printf("[!] WPL Start failed %d\r\n", (uint32_t)result);
        goto task_exit;
    }

    printf("[i] Successfully initialized Wi-Fi module\r\n");

    /* Here other tasks can be created that will run the enduser app.... */

    /* Main Loop */
    while (TRUE)
    {
        /* The SetBoardTo<state> function will configure the board Wifi to that given state.
         * After that, this task will suspend itself. It will remain suspended until it is time
         * to switch the state again. Uppon resuming, it will clean up the current state.
         * Every time the Wi-Fi state changes, this loop will perform an iteration switching back
         * and fourth between the two states as required.
         */
        switch (g_board_state.wifiState)
        {
            case WIFI_STATE_CLIENT:
                SetBoardToClient();
                /* Suspend here until its time to swtich back to AP */
                vTaskSuspend(NULL);
                CleanUpClient();
                break;
            case WIFI_STATE_AP:
            default:
                SetBoardToAP();
                /* Suspend here until its time to stop the AP */
                vTaskSuspend(NULL);
                CleanUpAP();
        }
    }
    
task_exit:
    vTaskDelete(NULL);    
}

BaseType_t FFreeRTOSStartAP(void)
{
    /* Create the main Task */
    if (xTaskCreate(StartAP, 
                    "StartAP", 
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