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
 * FilePath: wlan_common.h
 * Date: 2022-07-18 16:43:35
 * LastEditTime: 2022-07-18 16:43:35
 * Description:  This file is for providing some sfud apis.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2023/10/19    first commit
 */
#ifndef  WLAN_AP_COMMON_H
#define  WLAN_AP_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fsl_wifi_config.h"

/************************** Constant Definitions *****************************/
#define FWLAN_WIFI_SSID_LENGTH      32U
#define FWLAN_WIFI_PASSWORD_MIN_LEN 8
#define FWLAN_WIFI_PASSWORD_LENGTH  63
#define FWLAN_WIFI_SECURITY_LENGTH  63U

/* IP Address of Wi-Fi interface in AP (Access Point) mode */
#ifndef FWLAN_WIFI_AP_IP_ADDR
#define FWLAN_WIFI_AP_IP_ADDR "192.168.1.1"
#endif /* FWLAN_WIFI_AP_IP_ADDR */

/* Common Wi-Fi parameters */
#ifndef FWLAN_WIFI_SSID
#define FWLAN_WIFI_SSID "phytium_wifi"
#endif

#ifndef FWLAN_WIFI_PASSWORD
#define FWLAN_WIFI_PASSWORD "12345678"
#endif

#ifndef FWLAN_WIFI_SECURITY
#define FWLAN_WIFI_SECURITY "WPA2"
#endif

#define FWLAN_WIFI_NETWORK_LABEL "phytium_wifi"

typedef void (*FWlanLinkLostCb)(bool linkState);

typedef enum
{
    FWLAN_RET_SUCCESS,
    FWLAN_RET_FAIL,
    FWLAN_NOT_FOUND,
    FWLAN_AUTH_FAILED,
    FWLAN_ADDR_FAILED,
    FWLAN_NOT_CONNECTED,
    FWLAN_NOT_READY,
    FWLAN_TIMEOUT,
    FWLAN_BAD_PARAM,
} FWlanRetStatus;

typedef enum
{
    /* Used when the user only knows SSID and password. This option should be used
     * for WPA2 security and lower. */
    FWLAN_SECURITY_WILDCARD,
    /* Use WPA3 SAE security */
    FWLAN_SECURITY_WPA3_SAE,
} FWlanSecurityType;

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/**
 * @brief  Initialize Wi-Fi driver and WPL layer.
 *         Create an internal Event structure used for WPL layer (blocking API).
 *         Set WPL layer state to WLAN_STATE_INITIALIZED.
 *         Should be the first function called from the WPL layer API.
 *
 * @return FWLAN_RET_SUCCESS Wi-Fi driver and WPL layer initialized successfully.
 */
FWlanRetStatus FWlanInit(void);

/**
 * @brief  Start Wi-Fi driver and register an application link state callback.
 *         Set WPL layer state to WLAN_STATE_STARTED.
 *         FWlanStart should be called only after FWlanInit was successfully performed.
 *
 * @param  callbackFunction Function which will be called from WPL layer in order to
 *                          notify upper application that Wi-Fi link is lost or re/established.
 *
 * @return FWLAN_RET_SUCCESS Wi-Fi driver started successfully.
 */
FWlanRetStatus FWlanStart(FWlanLinkLostCb callbackFunction);

/**
 * @brief  Stop Wi-Fi driver.
 *         Set WPL layer state to WLAN_STATE_NOT_INITIALIZED.
 *         FWlanStop should be called only after FWlanStart was successfully performed.
 *
 * @return FWLAN_RET_SUCCESS Wi-Fi driver is stopped.
 */
FWlanRetStatus FWlanStop(void);

/**
 * @brief  Create an AP (Access Point) network profile and start Wi-Fi AP interface based on this profile.
 *         If AP mode is started successfully, start a DHCP server.
 *         If everything goes well, other devices can connect to this network.
 *         If anything goes wrong, provided network profile is deleted, AP and DHCP are stopped.
 *         FWlanStartAP fails if AP interface is already up.
 *         FWlanStartAP should be called only after FWlanStart was successfully performed.
 *
 * @param  ssid Name of the AP network to be created.
 * @param  password Password of the AP network to be created.
 * @param  chan Channel of the AP network to be created.
 *
 * @return FWLAN_RET_SUCCESS Network profile created, Wi-Fi AP interface up, DHCP server running.
 */
FWlanRetStatus FWlanStartAP(char *ssid, char *password, int chan);

/**
 * @brief  Stop DHCP server, stop Wi-Fi AP (Access Point) interface and delete AP network profile.
 *         FWlanStopAP should be called only after FWlanStartAP was successfully performed.
 *
 * @return FWLAN_RET_SUCCESS DHCP server stopped, AP interface down, AP network profile deleted.
 */
FWlanRetStatus FWlanStopAP(void);

/**
 * @brief  Scan for nearby Wi-Fi networks.
           Print available networks information and store them in an internal buffer in JSON fomrat.
           The returned buffer is dynamically allocated, caller is responsible for deallocation.
 *         FWlanScan should be called only after FWlanStart was successfully performed.
 *
 * @return Pointer to buffer with scan results.
 */
char *FWlanScan(void);

/**
 * @brief  Create and save a new STA (Station) network profile.
 *         This STA network profile can be used in future (FWlanRemoveNetwork / FWlanJoin) calls based on its label.
 *         FWlanAddNetwork should be called only after FWlanStart was successfully performed.
 *
 * @param  ssid Name of the STA network to be created.
 * @param  password Password of the STA network to be created.
 * @param  label Alias for the network to be added. A network may be referred by its label.
 * @param  security Prefered security type. Refer to FWlanSecurityType for list of options.
 *
 * @return FWLAN_RET_SUCCESS New STA network profile was successfully saved.
 */
FWlanRetStatus FWlanAddNetworkWithSecurity(char *ssid, char *password, char *label, FWlanSecurityType security);

/**
 * @brief  Create and save a new STA (Station) network profile.
 *         This STA network profile can be used in future (FWlanRemoveNetwork / FWlanJoin) calls based on its label.
 *         FWlanAddNetwork should be called only after FWlanStart was successfully performed.
 *
 * @param  ssid Name of the STA network to be created.
 * @param  password Password of the STA network to be created.
 * @param  label Alias for the network to be added. A network may be referred by its label.
 *
 * @return FWLAN_RET_SUCCESS New STA network profile was successfully saved.
 */
FWlanRetStatus FWlanAddNetwork(char *ssid, char *password, char *label);

/**
 * @brief  Delete a previously added STA (Station) network profile.
 *         The profile to be deleted is referred by its label and should have been previously added using
 * FWlanAddNetwork. FWlanRemoveNetwork should be called only after FWlanAddNetwork was successfully performed (for this
 * network).
 *
 * @param  label Alias for the network to be deleted. Label was set by FWlanAddNetwork.
 *
 * @return FWLAN_RET_SUCCESS The profile network is deleted.
 */
FWlanRetStatus FWlanRemoveNetwork(char *label);

/**
 * @brief  Connect to a Wi-Fi network.
 *         Wi-Fi network is chosen by a STA network label.
 *         The Wi-Fi network to connect to is referred by its label and should have been previously added using
 * FWlanAddNetwork. FWlanJoin should be called only after FWlanAddNetwork was successfully performed (for this network).
 *
 * @param  label Alias for the network to connect to. Label was set by FWlanAddNetwork.
 *
 * @return FWLAN_RET_SUCCESS Device joined the Wi-Fi network using its STA interface.
 */
FWlanRetStatus FWlanJoin(char *label);

/**
 * @brief  Disconnect from currently connected Wi-Fi network.
 *         FWlanLeave should be called only after FWlanJoin was successfully performed.
 *
 * @return FWLAN_RET_SUCCESS Device left the Wi-Fi network it was connected to.
 */
FWlanRetStatus FWlanLeave(void);

/**
 * @brief  Get IP for AP interface or STA interface.
 *
 * @param  ip Pointer where IP should be stored.
 * @param  client 0 for AP, 1 for STA.
 *
 * @return FWLAN_RET_SUCCESS if the IP was successfully retrieved.
 */
FWlanRetStatus FWlanGetIP(char *ip, int client);
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif