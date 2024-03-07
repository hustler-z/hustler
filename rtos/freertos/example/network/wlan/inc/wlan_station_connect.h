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
 * FilePath: wlan_station_connect.h
 * Date: 2022-07-18 16:43:35
 * LastEditTime: 2022-07-18 16:43:35
 * Description:  This file is for providing some sfud apis.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2023/10/19    first commit
 */
#ifndef  WLAN_STATION_CONNECT_H
#define  WLAN_STATION_CONNECT_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
BaseType_t FFreeRTOSWlanStationConnectInit(const char *usr_ssid, const char *usr_password);
BaseType_t FFreeRTOSWlanStationPing(const char *usr_remote_ip);

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif