/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: lwip_start.c
 * Date: 2022-11-07 10:16:55
 * LastEditTime: 2022-12-06 11:36:50
 * Description:  This file is for initiailizing a netif.The ip and other infomation can be set manually.
 *
 * Modify History:
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0  liuzhihong    2022/11/15            first release
 *  1.1  liuzhihong    2022/1/9          add functiuon calls in LwipTestLoop:LinkDetectLoop(netif)
 */


#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"
#include "ftypes.h"
#include "fassert.h"
#include "fparameters.h"
#ifndef SDK_CONFIG_H__
    #error "Please include sdkconfig.h first"
#endif


#include "lwipopts.h"
#include "lwip_port.h"
#include "lwip/ip4_addr.h"
#include "lwip/init.h"
#include "netif/ethernet.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"
#include "shell_port.h"

#if LWIP_IPV6
    #include "lwip/ip.h"
    #include "lwip/ip6_addr.h"
#else
    #if LWIP_DHCP
        #include "lwip/dhcp.h"
    #endif
#endif

typedef struct
{
    const char *ipaddr;
    const char *gateway;
    const char *netmask;
} InputAddress;

typedef struct
{
    UserConfig lwip_mac_config;
    InputAddress input_address;
    u32 dhcp_en;
} InputConfig;

InputConfig input_config = {0};


static boolean init_flag = FALSE;


void LwipTestCreate(void *args)
{
    FASSERT(args != NULL);
    struct netif *netif_p = NULL;

    InputConfig *input_conf = (InputConfig *)args;
    ip_addr_t ipaddr = {0}, netmask = {0}, gw = {0};

    /* the mac address of the board. this should be unique per board */
    unsigned char mac_address[6] =
    {0x98, 0x0e, 0x24, 0x00, 0x11, 0};

    netif_p = malloc(sizeof(struct netif));
    if (netif_p == NULL)
    {
        printf("Malloc netif is error.\r\n");
        goto exit;
    }
    printf("netif_p is %p \r\n", netif_p);
    mac_address[5] = input_conf->lwip_mac_config.mac_instance;

    /* convert string to a binary address */
    if (input_conf->input_address.ipaddr)
    {
        if (inet_aton(input_conf->input_address.ipaddr, &ipaddr) == 0)
        {
            goto failed;
        }
    }

    if (input_conf->input_address.gateway)
    {
        if (inet_aton(input_conf->input_address.gateway, &gw) == 0)
        {
            goto failed;
        }
    }

    if (input_conf->input_address.netmask)
    {
        if (inet_aton(input_conf->input_address.netmask, &netmask) == 0)
        {
            goto failed;
        }
    }

    /* 初始化LwIP堆 */
    if (init_flag == FALSE)
    {
        lwip_init(); /* lwip only init once */
        init_flag = TRUE;
    }

    /* Add network interface to the netif_list, and set it as default */
    if (!LwipPortAdd(netif_p, &ipaddr, &netmask, &gw, mac_address, (UserConfig *)args))
    {
        printf("Error adding N/W interface\n\r");
        return ;
    }
    printf("LwipPortAdd is over \n\r");

#if LWIP_IPV6
    netif_p->ip6_autoconfig_enabled = 1;
    netif_create_ip6_linklocal_address(netif_p, 1);
    netif_ip6_addr_set_state(netif_p, 0, IP6_ADDR_VALID);
#endif

    netif_set_default(netif_p);

    if (netif_is_link_up(netif_p))
    {
        /* 当netif完全配置好时，必须调用该函数 */
        netif_set_up(netif_p);
        if (input_conf->dhcp_en == 1)
        {
            LwipPortDhcpSet(netif_p, TRUE);
        }
    }
    else
    {
        /* 当netif链接关闭时，必须调用该函数 */
        netif_set_down(netif_p);
    }

    printf("Network setup complete.\n");

    goto exit ;
failed:
    printf("Failed \r\n");
    free(netif_p);
exit:

    return;
}

void LwipTestInint(void *args)
{
    LwipTestCreate(args);
}


void LwipTestLoop(void)
{
    struct netif *netif;
    if (init_flag == FALSE)
    {
        return;
    }

    netif = netif_list;

    while (netif != NULL)
    {
        if (netif->state)
        {
#ifdef CONFIG_LWIP_RX_POLL
            LwipEthProcessLoop(netif);
#endif
            LwipPortInput(netif);
            LinkDetectLoop(netif);
        }
        netif = netif->next;
    }
}

void LwipTestDhcpLoop(u32 msec)
{
#if (LWIP_DHCP==1)
    LwipPortDhcpLoop(msec);
#endif
}

static int LwipDeviceSet(int argc, char *argv[])
{
    u32 id = 0, interface_type = 0, driver_type = 0;
    memset(&input_config, 0, sizeof(input_config));
    LWIP_PORT_CONFIG_DEFAULT_INIT(input_config.lwip_mac_config);

    if (!strcmp(argv[1], "probe"))
    {
        if (argc < 6)
        {
            printf("Input error: Too few parameters!\n");
            printf("All parameters will be set to 0!\n");
        }
        else
        {
            driver_type = (u32)simple_strtoul(argv[2], NULL, 10);
            id = (u32)simple_strtoul(argv[3], NULL, 10);
            interface_type = (u32)simple_strtoul(argv[4], NULL, 10);
            input_config.dhcp_en = (u32)simple_strtoul(argv[5], NULL, 10);
            if (input_config.dhcp_en == 0)
            {
                if (argc == 9)
                {
                    input_config.input_address.ipaddr  = argv[6];
                    input_config.input_address.gateway = argv[7];
                    input_config.input_address.netmask = argv[8];
                }
                else
                {
                    printf("Input error: Missing parameters!\n");
                    printf("All Ip address will be set to 0!\n");
                }

            }
            else
            {
                 if (argc == 9)
                {
                    input_config.input_address.ipaddr  = argv[6];
                    input_config.input_address.gateway = argv[7];
                    input_config.input_address.netmask = argv[8];
                }
               
                printf("Dhcp Open: All IP addresses will be determined by the dhcp server!\n");
            
                
            }
        }

        input_config.lwip_mac_config.mac_instance = id;
        input_config.lwip_mac_config.name[0] = 'e';
        itoa(id, &input_config.lwip_mac_config.name[1], 10);

        const char *name = input_config.lwip_mac_config.name;
        if (LwipPortGetByName(name))
        {
            printf("Netif alread exist! \r\n");
            return -1;
        }


        if (interface_type == 0)
        {
            input_config.lwip_mac_config.mii_interface = LWIP_PORT_INTERFACE_RGMII;
        }
        else
        {
            input_config.lwip_mac_config.mii_interface = LWIP_PORT_INTERFACE_SGMII;
        }
        if (driver_type == 0)
        {
            input_config.lwip_mac_config.driver_type = LWIP_PORT_TYPE_XMAC;
        }
        else
        {
            input_config.lwip_mac_config.driver_type = LWIP_PORT_TYPE_GMAC;
        }
        
        LwipTestInint(&input_config);
    }
    else if (!strcmp(argv[1], "deinit"))
    {
        if (argc <= 1)
        {
            printf("Please enter lwip deinit <name>\r\n") ;
            printf("        -- use name to deinit neitf object\r\n");
            printf("        -- <name> is netif name\r\n");
            return -1;
        }
        struct netif *netif_p = NULL;
        netif_p = LwipPortGetByName(argv[2]);
        if (netif_p == NULL)
        {
            printf("netif %s is not invalid.\r\n", argv[2]);
            return -1;
        }

        /* close netif */
        LwipPortStop(netif_p,input_config.dhcp_en);
        free(netif_p);
    }
    else
    {
        printf("Please enter lwip probe <dirver id> <device id> <interface id> <dhcp_en> <ipaddr> <gateway> <netmask> \r\n") ;
        printf("        -- driver id is driver type set, 0 is xmac ,1 is gmac \r\n");
        printf("        -- device id is mac instance number \r\n");
        printf("        -- interface id is media independent interface  , 0 is rgmii ,1 is sgmii \r\n");
        printf("        -- dhcp_en is dhcp function set ,1 is enable ,0 is disable .But this depends on whether the protocol stack supports it ");
        printf("        -- <ipaddr> Ip address of netif \r\n");
        printf("        -- <gateway> Gateway of netif \r\n");
        printf("        -- <netmask> Netmask of netif \r\n");
        printf("Please enter lwip deinit <name> \r\n") ;
        printf("        -- use name to deinit neitf object \r\n");
        printf("        -- <name> is netif name \r\n");
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), lwip, LwipDeviceSet, Setup LWIP device test);

