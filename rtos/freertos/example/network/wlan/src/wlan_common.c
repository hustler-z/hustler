/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "wlan_bt_fw.h"
#include "wlan.h"
#include "wifi.h"
#include "wm_net.h"
#include <wm_os.h>
#include "dhcp-server.h"
#include <stdio.h>
#include "event_groups.h"
#include "wlan_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define MAX_JSON_NETWORK_RECORD_LENGTH 185

#define WLAN_SYNC_TIMEOUT_MS portMAX_DELAY

#define WLAN_UAP_NETWORK_NAME "uap-network"

#define WLAN_EVENT_BIT(event) (1 << event)

#define WLAN_SYNC_INIT_GROUP WLAN_EVENT_BIT(WLAN_REASON_INITIALIZED) | WLAN_EVENT_BIT(WLAN_REASON_INITIALIZATION_FAILED)

#define WLAN_SYNC_CONNECT_GROUP                                                                  \
        WLAN_EVENT_BIT(WLAN_REASON_SUCCESS) | WLAN_EVENT_BIT(WLAN_REASON_CONNECT_FAILED) |                    \
        WLAN_EVENT_BIT(WLAN_REASON_NETWORK_NOT_FOUND) | WLAN_EVENT_BIT(WLAN_REASON_NETWORK_AUTH_FAILED) | \
        WLAN_EVENT_BIT(WLAN_REASON_ADDRESS_FAILED)

#define WLAN_SYNC_DISCONNECT_GROUP WLAN_EVENT_BIT(WLAN_REASON_USER_DISCONNECT)

#define WLAN_SYNC_UAP_START_GROUP WLAN_EVENT_BIT(WLAN_REASON_UAP_SUCCESS) | WLAN_EVENT_BIT(WLAN_REASON_UAP_START_FAILED)

#define WLAN_SYNC_UAP_STOP_GROUP WLAN_EVENT_BIT(WLAN_REASON_UAP_STOPPED) | WLAN_EVENT_BIT(WLAN_REASON_UAP_STOP_FAILED)

#define WLAN_EVENT_SCAN_DONE     23
#define WLAN_SYNC_SCAN_GROUP    WLAN_EVENT_BIT(WLAN_EVENT_SCAN_DONE)

typedef enum
{
    WLAN_STATE_NOT_INITIALIZED,
    WLAN_STATE_INITIALIZED,
    WLAN_STATE_STARTED,
} WlanState;

/*******************************************************************************
 * Variables
 ******************************************************************************/
static WlanState s_wlan_state                 = WLAN_STATE_NOT_INITIALIZED;
static bool s_wlan_sta_connected              = false;
static bool s_wlan_uap_activated              = false;
static EventGroupHandle_t s_wlan_sync_event   = NULL;
static FWlanLinkLostCb s_link_lost_cb         = NULL;
static char *ssids_json                       = NULL;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
int WlanEventCallback(enum wlan_event_reason reason, void *data);
static int WLanProcessResults(unsigned int count);

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Callback Function passed to WLAN Connection Manager. The callback function
 * gets called when there are WLAN Events that need to be handled by the
 * application.
 */
int WlanEventCallback(enum wlan_event_reason reason, void *data)
{
#ifdef FWLAN_DEBUG
    printf("-------- WlanEventCallback %d --------\r\n", reason);
#endif
    if (s_wlan_state >= WLAN_STATE_INITIALIZED)
    {
        xEventGroupSetBits(s_wlan_sync_event, WLAN_EVENT_BIT(reason));
    }

    switch (reason)
    {
        case WLAN_REASON_SUCCESS:
            if (s_wlan_sta_connected)
            {
                s_link_lost_cb(true);
            }
            break;

        case WLAN_REASON_AUTH_SUCCESS:
            break;

        case WLAN_REASON_CONNECT_FAILED:
        case WLAN_REASON_NETWORK_NOT_FOUND:
        case WLAN_REASON_NETWORK_AUTH_FAILED:
            if (s_wlan_sta_connected)
            {
                s_link_lost_cb(false);
            }
            break;

        case WLAN_REASON_ADDRESS_SUCCESS:
            break;
        case WLAN_REASON_ADDRESS_FAILED:
            break;
        case WLAN_REASON_LINK_LOST:
            if (s_wlan_sta_connected)
            {
                s_link_lost_cb(false);
            }
            break;

        case WLAN_REASON_CHAN_SWITCH:
            break;
        case WLAN_REASON_WPS_DISCONNECT:
            break;
        case WLAN_REASON_USER_DISCONNECT:
            break;
        case WLAN_REASON_INITIALIZED:
            break;
        case WLAN_REASON_INITIALIZATION_FAILED:
            break;
        case WLAN_REASON_PS_ENTER:
            break;
        case WLAN_REASON_PS_EXIT:
            break;
        case WLAN_REASON_UAP_SUCCESS:
            break;

        case WLAN_REASON_UAP_CLIENT_ASSOC:
#ifdef FWLAN_DEBUG
            printf("Client => ");
            print_mac((const char *)data);
            printf("Associated with Soft AP\r\n");
#endif
            break;
        case WLAN_REASON_UAP_CLIENT_DISSOC:
#ifdef FWLAN_DEBUG
            printf("Client => ");
            print_mac((const char *)data);
            printf("Dis-Associated from Soft AP\r\n");
#endif
            break;

        case WLAN_REASON_UAP_START_FAILED:
            break;
        case WLAN_REASON_UAP_STOP_FAILED:
            break;
        case WLAN_REASON_UAP_STOPPED:
            break;
        default:
#ifdef FWLAN_DEBUG
            printf("Unknown Wifi CB Reason %d\r\n", reason);
#endif
            break;
    }

    return WM_SUCCESS;
}

FWlanRetStatus FWlanInit(void)
{
    FWlanRetStatus status = FWLAN_RET_SUCCESS;
    int ret;

    if (s_wlan_state != WLAN_STATE_NOT_INITIALIZED)
    {
        status = FWLAN_RET_FAIL;
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        if (s_wlan_sync_event == NULL)
        {
            s_wlan_sync_event = xEventGroupCreate();
        }

        if (s_wlan_sync_event == NULL)
        {
            status = FWLAN_RET_FAIL;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        ret = wlan_init(wlan_fw_bin, wlan_fw_bin_len);
        if (ret != WM_SUCCESS)
        {
            status = FWLAN_RET_FAIL;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        s_wlan_state = WLAN_STATE_INITIALIZED;
    }

    return status;
}

FWlanRetStatus FWlanStart(FWlanLinkLostCb callbackFunction)
{
    FWlanRetStatus status = FWLAN_RET_SUCCESS;
    int ret;
    EventBits_t syncBit;

    if (s_wlan_state != WLAN_STATE_INITIALIZED)
    {
        status = FWLAN_NOT_READY;
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        xEventGroupClearBits(s_wlan_sync_event, WLAN_SYNC_INIT_GROUP);

        ret = wlan_start(WlanEventCallback);
        if (ret != WM_SUCCESS)
        {
            status = FWLAN_RET_FAIL;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        syncBit = xEventGroupWaitBits(s_wlan_sync_event, WLAN_SYNC_INIT_GROUP, pdTRUE, pdFALSE, WLAN_SYNC_TIMEOUT_MS);
        if (syncBit & WLAN_EVENT_BIT(WLAN_REASON_INITIALIZED))
        {
            s_link_lost_cb = callbackFunction;
            status       = FWLAN_RET_SUCCESS;
        }
        else if (syncBit & WLAN_EVENT_BIT(WLAN_REASON_INITIALIZATION_FAILED))
        {
            status = FWLAN_RET_FAIL;
        }
        else
        {
            status = FWLAN_TIMEOUT;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        s_wlan_state = WLAN_STATE_STARTED;
    }

    return status;
}

FWlanRetStatus FWlanStop(void)
{
    FWlanRetStatus status = FWLAN_RET_SUCCESS;
    int ret;

    if (s_wlan_state != WLAN_STATE_STARTED)
    {
        status = FWLAN_NOT_READY;
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        ret = wlan_stop();
        if (ret != WM_SUCCESS)
        {
            status = FWLAN_RET_FAIL;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        s_wlan_state = WLAN_STATE_INITIALIZED;
    }

    return status;
}

FWlanRetStatus FWlanStartAP(char *ssid, char *password, int chan)
{
    FWlanRetStatus status = FWLAN_RET_SUCCESS;
    int ret;
    enum wlan_security_type security = WLAN_SECURITY_NONE;
    EventBits_t syncBit;
    struct wlan_network uap_network;

    if ((s_wlan_state != WLAN_STATE_STARTED) || (s_wlan_uap_activated != false))
    {
        status = FWLAN_NOT_READY;
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        if ((strlen(ssid) == 0) || (strlen(ssid) > IEEEtypes_SSID_SIZE))
        {
            printf("SSID: %s length = %d \r\n", ssid, strlen(ssid));
            status = FWLAN_BAD_PARAM;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        if (strlen(password) == 0)
        {
            security = WLAN_SECURITY_NONE;
        }
        else if ((strlen(password) >= FWLAN_WIFI_PASSWORD_MIN_LEN) && (strlen(password) <= FWLAN_WIFI_PASSWORD_LENGTH))
        {
            security = WLAN_SECURITY_WPA2;
        }
        else
        {
            status = FWLAN_BAD_PARAM;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        wlan_initialize_uap_network(&uap_network);

        memcpy(uap_network.ssid, ssid, strlen(ssid));
        uap_network.ip.ipv4.address  = ipaddr_addr(FWLAN_WIFI_AP_IP_ADDR);
        uap_network.ip.ipv4.gw       = ipaddr_addr(FWLAN_WIFI_AP_IP_ADDR);
        uap_network.channel          = chan;
        uap_network.security.type    = security;
        uap_network.security.psk_len = strlen(password);
        strncpy(uap_network.security.psk, password, strlen(password));
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        ret = wlan_add_network(&uap_network);
        if (ret != WM_SUCCESS)
        {
            status = FWLAN_RET_FAIL;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        xEventGroupClearBits(s_wlan_sync_event, WLAN_SYNC_UAP_START_GROUP);

        ret = wlan_start_network(uap_network.name);
        if (ret != WM_SUCCESS)
        {
            status = FWLAN_RET_FAIL;
        }
        else
        {
            syncBit =
                xEventGroupWaitBits(s_wlan_sync_event, WLAN_SYNC_UAP_START_GROUP, pdTRUE, pdFALSE, WLAN_SYNC_TIMEOUT_MS);
            if (syncBit & WLAN_EVENT_BIT(WLAN_REASON_UAP_SUCCESS))
            {
                status = FWLAN_RET_SUCCESS;
            }
            else if (syncBit & WLAN_EVENT_BIT(WLAN_REASON_UAP_START_FAILED))
            {
                status = FWLAN_RET_FAIL;
            }
            else
            {
                status = FWLAN_TIMEOUT;
            }
        }

        if (status != FWLAN_RET_SUCCESS)
        {
            wlan_remove_network(uap_network.name);
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        ret = dhcp_server_start(net_get_uap_handle());
        if (ret != WM_SUCCESS)
        {
            wlan_stop_network(uap_network.name);
            wlan_remove_network(uap_network.name);
            status = FWLAN_RET_FAIL;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        s_wlan_uap_activated = true;
    }

    return status;
}

FWlanRetStatus FWlanStopAP(void)
{
    FWlanRetStatus status = FWLAN_RET_SUCCESS;
    int ret;
    EventBits_t syncBit;

    if ((s_wlan_state != WLAN_STATE_STARTED) || (s_wlan_uap_activated != true))
    {
        status = FWLAN_NOT_READY;
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        dhcp_server_stop();

        xEventGroupClearBits(s_wlan_sync_event, WLAN_SYNC_UAP_START_GROUP);

        ret = wlan_stop_network(WLAN_UAP_NETWORK_NAME);
        if (ret != WM_SUCCESS)
        {
            status = FWLAN_RET_FAIL;
        }
        else
        {
            syncBit =
                xEventGroupWaitBits(s_wlan_sync_event, WLAN_SYNC_UAP_STOP_GROUP, pdTRUE, pdFALSE, WLAN_SYNC_TIMEOUT_MS);
            if (syncBit & WLAN_EVENT_BIT(WLAN_REASON_UAP_STOPPED))
            {
                status = FWLAN_RET_SUCCESS;
            }
            else if (syncBit & WLAN_EVENT_BIT(WLAN_REASON_UAP_STOP_FAILED))
            {
                status = FWLAN_RET_FAIL;
            }
            else
            {
                status = FWLAN_TIMEOUT;
            }
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        ret = wlan_remove_network(WLAN_UAP_NETWORK_NAME);
        if (ret != WM_SUCCESS)
        {
            status = FWLAN_RET_FAIL;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        s_wlan_uap_activated = false;
    }

    return status;
}

static int WLanProcessResults(unsigned int count)
{
    int ret                             = 0;
    struct wlan_scan_result scan_result = {0};
    uint32_t ssids_json_len             = count * MAX_JSON_NETWORK_RECORD_LENGTH;

    /* Add length of "{"networks":[]}" */
    ssids_json_len += 15;

    ssids_json = pvPortMalloc(ssids_json_len);
    if (ssids_json == NULL)
    {
        printf("[!] Memory allocation failed\r\n");
        xEventGroupSetBits(s_wlan_sync_event, WLAN_EVENT_BIT(WLAN_EVENT_SCAN_DONE));
        return WM_FAIL;
    }

    /* Start building JSON */
    strcpy(ssids_json, "{\"networks\":[");
    uint32_t ssids_json_idx = strlen(ssids_json);

    for (int i = 0; i < count; i++)
    {
        wlan_get_scan_result(i, &scan_result);

        printf("%s\r\n", scan_result.ssid);
        printf("     BSSID         : %02X:%02X:%02X:%02X:%02X:%02X\r\n", (unsigned int)scan_result.bssid[0],
               (unsigned int)scan_result.bssid[1], (unsigned int)scan_result.bssid[2],
               (unsigned int)scan_result.bssid[3], (unsigned int)scan_result.bssid[4],
               (unsigned int)scan_result.bssid[5]);
        printf("     RSSI          : %ddBm\r\n", -(int)scan_result.rssi);
        printf("     Channel       : %d\r\n", (int)scan_result.channel);

        char security[40];
        security[0] = '\0';

        if (scan_result.wpa2_entp)
        {
            strcat(security, "WPA2_ENTP ");
        }
        if (scan_result.wep)
        {
            strcat(security, "WEP ");
        }
        if (scan_result.wpa)
        {
            strcat(security, "WPA ");
        }
        if (scan_result.wpa2)
        {
            strcat(security, "WPA2 ");
        }
        if (scan_result.wpa3_sae)
        {
            strcat(security, "WPA3_SAE ");
        }

        if (i != 0)
        {
            /* Add ',' separator before next entry */
            ssids_json[ssids_json_idx++] = ',';
        }

        ret = snprintf(
            ssids_json + ssids_json_idx, ssids_json_len - ssids_json_idx - 1,
            "{\"ssid\":\"%s\",\"bssid\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\"signal\":\"%ddBm\",\"channel\":%d,"
            "\"security\":\"%s\"}",
            scan_result.ssid, (unsigned int)scan_result.bssid[0], (unsigned int)scan_result.bssid[1],
            (unsigned int)scan_result.bssid[2], (unsigned int)scan_result.bssid[3], (unsigned int)scan_result.bssid[4],
            (unsigned int)scan_result.bssid[5], -(int)scan_result.rssi, (int)scan_result.channel, security);
        if (ret > 0)
        {
            ssids_json_idx += ret;
        }
        else
        {
            printf("[!] JSON creation failed\r\n");
            vPortFree(ssids_json);
            ssids_json = NULL;
            xEventGroupSetBits(s_wlan_sync_event, WLAN_EVENT_BIT(WLAN_EVENT_SCAN_DONE));
            return WM_FAIL;
        }
    }

    /* End of JSON "]}" */
    strcpy(ssids_json + ssids_json_idx, "]}");

    xEventGroupSetBits(s_wlan_sync_event, WLAN_EVENT_BIT(WLAN_EVENT_SCAN_DONE));
    return WM_SUCCESS;
}

char *FWlanScan(void)
{
    FWlanRetStatus status = FWLAN_RET_SUCCESS;
    int ret;
    EventBits_t syncBit;

    if (s_wlan_state != WLAN_STATE_STARTED)
    {
        status = FWLAN_NOT_READY;
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        ret = wlan_scan(WLanProcessResults);
        if (ret != WM_SUCCESS)
        {
            status = FWLAN_RET_FAIL;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        syncBit = xEventGroupWaitBits(s_wlan_sync_event, WLAN_SYNC_SCAN_GROUP, pdTRUE, pdFALSE, WLAN_SYNC_TIMEOUT_MS);
        if (syncBit & WLAN_EVENT_BIT(WLAN_EVENT_SCAN_DONE))
        {
            status = FWLAN_RET_SUCCESS;
        }
        else
        {
            status = FWLAN_TIMEOUT;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        return ssids_json;
    }

    return NULL;
}

FWlanRetStatus FWlanAddNetworkWithSecurity(char *ssid, char *password, char *label, FWlanSecurityType security)
{
    FWlanRetStatus status = FWLAN_RET_SUCCESS;
    int ret;
    struct wlan_network sta_network;
    memset(&sta_network, 0, sizeof(struct wlan_network));

    if (s_wlan_state != WLAN_STATE_STARTED)
    {
        status = FWLAN_NOT_READY;
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        if ((strlen(label) == 0) || (strlen(label) > (WLAN_NETWORK_NAME_MAX_LENGTH - 1)))
        {
            status = FWLAN_BAD_PARAM;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        if ((strlen(ssid) == 0) || (strlen(ssid) > IEEEtypes_SSID_SIZE))
        {
            status = FWLAN_BAD_PARAM;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        size_t password_len = strlen(password);

        if (password_len == 0)
        {
            sta_network.security.type = WLAN_SECURITY_NONE;
        }
        else if ((password_len >= FWLAN_WIFI_PASSWORD_MIN_LEN) && (password_len <= FWLAN_WIFI_PASSWORD_LENGTH))
        {
            switch (security)
            {
                case FWLAN_SECURITY_WILDCARD:
                    sta_network.security.type = WLAN_SECURITY_WILDCARD;
                    sta_network.security.mfpc = true;
                    sta_network.security.mfpr = true;
                    sta_network.security.password_len = password_len;
                    strncpy(sta_network.security.password, password, password_len);
                    sta_network.security.psk_len = password_len;
                    strncpy(sta_network.security.psk, password, password_len);
                    break;
                case FWLAN_SECURITY_WPA3_SAE:
                    sta_network.security.type = WLAN_SECURITY_WPA3_SAE;
                    sta_network.security.mfpc = true;
                    sta_network.security.mfpr = true;
                    sta_network.security.password_len = strlen(password);
                    strncpy(sta_network.security.password, password, strlen(password));
                    break;
                default:
                    printf("[!] Unimplemented security type (%d)\r\n", security);
                    status = FWLAN_BAD_PARAM;
                    break;
            }
        }
        else
        {
            status = FWLAN_BAD_PARAM;
        }
    }
    
    strcpy(sta_network.name, label);
    strcpy(sta_network.ssid, ssid);
    sta_network.ip.ipv4.addr_type = ADDR_TYPE_DHCP;
    sta_network.ssid_specific     = 1;

    if (status == FWLAN_RET_SUCCESS)
    {
        ret = wlan_add_network(&sta_network);
        if (ret != WM_SUCCESS)
        {
            status = FWLAN_RET_FAIL;
        }
    }

    return status;
}

FWlanRetStatus FWlanAddNetwork(char *ssid, char *password, char *label)
{
    return FWlanAddNetworkWithSecurity(ssid, password, label, FWLAN_SECURITY_WILDCARD);
}

FWlanRetStatus FWlanRemoveNetwork(char *label)
{
    FWlanRetStatus status = FWLAN_RET_SUCCESS;
    int ret;

    if (s_wlan_state != WLAN_STATE_STARTED)
    {
        status = FWLAN_NOT_READY;
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        if ((strlen(label) == 0) || (strlen(label) > (WLAN_NETWORK_NAME_MAX_LENGTH - 1)))
        {
            status = FWLAN_BAD_PARAM;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        ret = wlan_remove_network(label);
        if (ret != WM_SUCCESS)
        {
            status = FWLAN_RET_FAIL;
        }
    }

    return status;
}

FWlanRetStatus FWlanJoin(char *label)
{
    FWlanRetStatus status = FWLAN_RET_SUCCESS;
    int ret;
    EventBits_t syncBit;

    if ((s_wlan_state != WLAN_STATE_STARTED) || (s_wlan_sta_connected != false))
    {
        status = FWLAN_NOT_READY;
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        if ((strlen(label) == 0) || (strlen(label) > (WLAN_NETWORK_NAME_MAX_LENGTH - 1)))
        {
            status = FWLAN_BAD_PARAM;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        xEventGroupClearBits(s_wlan_sync_event, WLAN_SYNC_CONNECT_GROUP);

        ret = wlan_connect(label);
        if (ret != WM_SUCCESS)
        {
            status = FWLAN_RET_FAIL;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        syncBit = xEventGroupWaitBits(s_wlan_sync_event, WLAN_SYNC_CONNECT_GROUP, pdTRUE, pdFALSE, WLAN_SYNC_TIMEOUT_MS);
        if (syncBit & WLAN_EVENT_BIT(WLAN_REASON_SUCCESS))
        {
            status = FWLAN_RET_SUCCESS;
        }
        else if (syncBit & WLAN_EVENT_BIT(WLAN_REASON_CONNECT_FAILED))
        {
            status = FWLAN_RET_FAIL;
        }
        else if (syncBit & WLAN_EVENT_BIT(WLAN_REASON_NETWORK_NOT_FOUND))
        {
            status = FWLAN_NOT_FOUND;
        }
        else if (syncBit & WLAN_EVENT_BIT(WLAN_REASON_NETWORK_AUTH_FAILED))
        {
            status = FWLAN_AUTH_FAILED;
        }
        else if (syncBit & WLAN_EVENT_BIT(WLAN_REASON_ADDRESS_FAILED))
        {
            status = FWLAN_ADDR_FAILED;
        }
        else
        {
            status = FWLAN_TIMEOUT;
        }

        if (status != FWLAN_RET_SUCCESS)
        {
            /* Abort the next connection attempt */
            FWlanLeave();
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        s_wlan_sta_connected = true;
    }

    return status;
}

FWlanRetStatus FWlanLeave(void)
{
    FWlanRetStatus status = FWLAN_RET_SUCCESS;
    int ret;
    EventBits_t syncBit;

    if (s_wlan_state != WLAN_STATE_STARTED)
    {
        status = FWLAN_NOT_READY;
    }

    enum wlan_connection_state connection_state = WLAN_DISCONNECTED;
    wlan_get_connection_state(&connection_state);
    if (connection_state == WLAN_DISCONNECTED)
    {
        s_wlan_sta_connected = false;
        return FWLAN_RET_SUCCESS;
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        xEventGroupClearBits(s_wlan_sync_event, WLAN_SYNC_DISCONNECT_GROUP);

        ret = wlan_disconnect();
        if (ret != WM_SUCCESS)
        {
            status = FWLAN_RET_FAIL;
        }
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        syncBit = xEventGroupWaitBits(s_wlan_sync_event, WLAN_SYNC_DISCONNECT_GROUP, pdTRUE, pdFALSE, WLAN_SYNC_TIMEOUT_MS);
        if (syncBit & WLAN_EVENT_BIT(WLAN_REASON_USER_DISCONNECT))
        {
            status = FWLAN_RET_SUCCESS;
        }
        else
        {
            status = FWLAN_TIMEOUT;
        }
    }

    s_wlan_sta_connected = false;

    return status;
}

FWlanRetStatus FWlanGetIP(char *ip, int client)
{
    FWlanRetStatus status = FWLAN_RET_SUCCESS;
    int ret;
    struct wlan_ip_config addr;

    if (ip == NULL)
    {
        status = FWLAN_RET_FAIL;
    }

    if (status == FWLAN_RET_SUCCESS)
    {
        if (client)
        {
            ret = wlan_get_address(&addr);
        }
        else
        {
            ret = wlan_get_uap_address(&addr);
        }

        if (ret == WM_SUCCESS)
        {
            net_inet_ntoa(addr.ipv4.address, ip);
        }
        else
        {
            status = FWLAN_RET_FAIL;
        }
    }

    return status;
}
