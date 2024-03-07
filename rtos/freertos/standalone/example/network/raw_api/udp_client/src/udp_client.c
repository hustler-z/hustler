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
 * FilePath: udp_client.c
 * Date: 2022-10-25 16:44:49
 * LastEditTime: 2022-12-05 18:13:22
 * Description:  This file is for creating a udp client.The client can send a udp packet to the destination host.
 *
 * Modify History:
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0  liuzhihong     2022/11/16            first release
 */
#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/init.h"
#include "netif/etharp.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include <stdio.h>
#include <string.h>
#include "shell_port.h"
#define UDP_ECHO_PORT 5001

void UdpClient(void)
{
    struct udp_pcb *client_pcb;
    struct pbuf *q = NULL;

    /* create a udp_pcb */
    client_pcb = udp_new();


    if (client_pcb == NULL)
    {
        printf("Create new Udb_pcb falied.\n");
        return ;
    }
    /*bind the local port and local ip */
    ip_addr_t ipaddr;

    IP_ADDR4(&ipaddr,  192, 168,   4, 10);
    udp_bind(client_pcb, &ipaddr, UDP_ECHO_PORT);
    IP_ADDR4(&ipaddr,  192, 168,   4, 50);
    udp_connect(client_pcb, &ipaddr, 5002);

    /*applying for memory resources*/
    const char *reply = "This is a data from Udp client!\n";
    q = pbuf_alloc(PBUF_TRANSPORT, strlen(reply) + 1, PBUF_RAM);

    if (!q)
    {
        printf("Out of PBUF_RAM.\n");
        return;
    }

    memset(q->payload, 0, q->len);
    memcpy(q->payload, reply, strlen(reply));

    /*sending data*/
    printf("We sent a data to Udp Server.\n");
    udp_send(client_pcb, q);

    /*Releasing resources*/
    udp_remove(client_pcb);
    pbuf_free(q);
}




SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), udpclient,  UdpClient, Start Udp client);
