/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: eth_board.h
 * Created Date: 2023-09-27 10:00:53
 * Last Modified: 2023-10-11 14:57:36
 * Description:  This file is for recording the mac config on the e2000q-demo board.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong   2023/10/8         first release
 */
#ifndef  ETH_BOARD_H
#define  ETH_BOARD_H

#include "fparameters.h"
#include "lwip_port.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAC_NUM 1 

/* 网卡接口与实际控制器的对应关系 */
#define MAC_NUM0 0
#define MAC_NUM0_CONTROLLER FXMAC0_ID
#define MAC_NUM0_LWIP_PORT_TYPE LWIP_PORT_TYPE_XMAC
#define MAC_NUM0_MII_INTERFACE LWIP_PORT_INTERFACE_SGMII

#ifdef __cplusplus
}
#endif

#endif  
