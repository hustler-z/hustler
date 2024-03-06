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
 * FilePath: lwip_iperf_client_example.c
 * Created Date: 2023-10-18 14:30:04
 * Last Modified: 2023-10-25 17:02:37
 * Description:  This file is for lwip iperf client example function implementation.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong  2023/10/18          first release
 */
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"
#include "ftypes.h"
#include "fassert.h"
#include "fparameters.h"
#include "eth_board.h"
#ifndef SDK_CONFIG_H__
    #error "Please include sdkconfig.h first"
#endif


#include "lwip_port.h"
#include "lwip/ip4_addr.h"
#include "lwip/init.h"
#include "netif/ethernet.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"
#include "lwiperf.h"

#define LWIPERF_TCP_PORT_REMOTE 5001
const ip_addr_t remote= IPADDR4_INIT_BYTES(192,168,4,50);

#define ETH_NAME_PREFIX 'e'

#define CONFIG_DEFAULT_INIT(config,driver_config,instance_id,interface_type)           \
		.config.magic_code = LWIP_PORT_CONFIG_MAGIC_CODE,  \
		.config.driver_type = driver_config,		  \
		.config.mac_instance = instance_id,                          \
		.config.mii_interface = interface_type, \
		.config.autonegotiation = 1,                       \
		.config.phy_speed = LWIP_PORT_SPEED_1000M,         \
		.config.phy_duplex = LWIP_PORT_FULL_DUPLEX, \
		.config.capability = LWIP_PORT_MODE_NAIVE, 



typedef struct
{
    UserConfig lwip_mac_config;
    u32 dhcp_en;
    char*  ipaddr;
    char*  netmask; 
    char*  gw;
    unsigned char mac_address[6];
    struct netif netif;
} BoardMacConfig;

static BoardMacConfig board_mac_config[MAC_NUM] = 
 {
    #if defined(MAC_NUM0)
            {
                CONFIG_DEFAULT_INIT(lwip_mac_config,MAC_NUM0_LWIP_PORT_TYPE,MAC_NUM0_CONTROLLER,MAC_NUM0_MII_INTERFACE)
                .dhcp_en=0,
                .ipaddr="192.168.4.10",
                .gw="192.168.4.1",
                .netmask="255.255.255.0",
                .mac_address={0x98, 0x0e, 0x24, 0x00, 0x11, 0x0},
            },
    #endif
    #if defined(MAC_NUM1) 
            {
                CONFIG_DEFAULT_INIT(lwip_mac_config,MAC_NUM1_LWIP_PORT_TYPE,MAC_NUM1_CONTROLLER,MAC_NUM1_MII_INTERFACE)
                .dhcp_en=0,
                .ipaddr="192.168.4.11",
                .gw="192.168.4.1",
                .netmask="255.255.255.0",
                .mac_address={0x98, 0x0e, 0x24, 0x00, 0x11, 0x1},
            },
    #endif
};



static void SetIP(ip_addr_t* ipaddr,ip_addr_t* gw,ip_addr_t* netmask,u32 mac_id)
{
    
    if(inet_aton(board_mac_config[mac_id].ipaddr,ipaddr)==0)
        printf("The addr of ipaddr is wrong\r\n");      
    if(inet_aton(board_mac_config[mac_id].gw,gw)==0)
        printf("The addr of gw is wrong\r\n");
    if(inet_aton(board_mac_config[mac_id].netmask,netmask)==0)
        printf("The addr of netmask is wrong\r\n");

}

int LwipIperfClientCreate(void)
{
    FError ret = FT_SUCCESS;
    /* mac init */
    for (int i = 0; i < MAC_NUM; i++)
    {
       
        struct netif *netif_p = NULL;
        ip_addr_t ipaddr,netmask, gw;
        board_mac_config[i].lwip_mac_config.name[0] = ETH_NAME_PREFIX;
        itoa(board_mac_config[i].lwip_mac_config.mac_instance, &(board_mac_config[i].lwip_mac_config.name[1]), 10);

         /* mac ip addr set: char* -> ip_addr_t */
        SetIP(&ipaddr,&gw,&netmask,i);
    /* ******************************************************************* */

        netif_p= &board_mac_config[i].netif;
        /* Add network interface to the netif_list, and set it as default */
        if (!LwipPortAdd(netif_p, &ipaddr, &netmask, &gw, board_mac_config[i].mac_address, (UserConfig *)&board_mac_config[i]))
        {
            printf("Error adding N/W interface %d.\n\r",board_mac_config[i].lwip_mac_config.mac_instance);
            return ERR_GENERAL;
        }
        printf("LwipPortAdd mac_instance %d is over.\n\r",board_mac_config[i].lwip_mac_config.mac_instance);

        netif_set_default(netif_p);

        if (netif_is_link_up(netif_p))
        {
            /* 当netif完全配置好时，必须调用该函数 */
            netif_set_up(netif_p);
            if (board_mac_config[i].dhcp_en == 1)
            {
                LwipPortDhcpSet(netif_p, TRUE);
            }
        }
        else
        {
            /* 当netif链接关闭时，必须调用该函数 */
            netif_set_down(netif_p);
        }

       
    }
    printf("Network setup complete.\n");

    printf("Now start iperf client test\r\n");
    if(lwiperf_start_tcp_client(&remote, LWIPERF_TCP_PORT_REMOTE, LWIPERF_CLIENT,
                                  NULL, NULL))
    {
        printf("Start iperf client test success !!! \r\n");
    }
    else
    {   
        printf("Start iperf client test failed !!! \r\n");
        return ERR_GENERAL;
    }     
  

    if (ret == FT_SUCCESS)
    {
        printf("%s@%d: Lwip iperf example success !!! \r\n", __func__, __LINE__);
        printf("[system_example_pass]\r\n");
    }
    else
    {
        printf("%s@%d: Lwip iperf example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
    }
    return 0;
}

void LwipIperfClientDeinit(void)
{    
    printf("Now reset all active iperf session. \r\n");
    lwiperf_reset();
    printf("Reset all active iperf session complete! \r\n");
    for (int i = 0; i < MAC_NUM; i++)
    {
        struct netif *netif_p = NULL;
        netif_p=&board_mac_config[i].netif;
        LwipPortStop(netif_p,board_mac_config[i].dhcp_en); 
    }

}