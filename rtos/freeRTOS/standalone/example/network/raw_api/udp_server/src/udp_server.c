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
 * FilePath: udp_server.c
 * Date: 2022-10-25 10:53:14
 * LastEditTime: 2022-12-05 18:30:49
 * Description:  This file is for creating a udp server.The server can return received data to the client which send it!
 *
 * Modify History:
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0  liuzhihong    2022/11/17            first release
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
#include"shell_port.h"
#define UDP_ECHO_PORT 5001

static void udp_demo_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    struct pbuf *q = NULL;
    const char *reply = "This is reply!\n";
    if (arg)
    {
        printf("%s", (char *)arg);
        printf("%s", (char *)p->payload);
    }
    pbuf_free(p);
    q = pbuf_alloc(PBUF_TRANSPORT, strlen(reply) + 1, PBUF_RAM);
    if (!q)
    {
        printf("Out of PBUF_RAM\n");
        return;
    }
    memset(q->payload, 0, q->len);
    memcpy(q->payload, reply, strlen(reply));
    udp_sendto(upcb, q, addr, port);
    pbuf_free(q);
}
static char *st_buffer = "We get a data: ";
void UdpServerInit(void)
{
    struct udp_pcb *server_pcb;
    server_pcb = udp_new();
    if (server_pcb == NULL)
    {
        printf("Udp Server open failed!\r\n");
        printf("Unable to create a new udp_pcb.\r\n");
        return ;
    }
    /* 绑定端口号 */
    ip_addr_t ipaddr;
    IP_ADDR4(&ipaddr,  192, 168,   4, 10);
    udp_bind(server_pcb, &ipaddr, UDP_ECHO_PORT);

    /* 注册接收数据回调函数 */
    udp_recv(server_pcb, udp_demo_callback, (void *)st_buffer);
    printf("Udp Server Open Success!\r\n");
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), udpserver, UdpServerInit, Start Udp server);