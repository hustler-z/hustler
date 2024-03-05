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
 * FilePath: tcp_server.c
 * Date: 2022-10-27 18:16:52
 * LastEditTime: 2022-12-05 18:12:14
 * Description:  This file is creating a tcp server.By Tcp Connection,The server can return received data to the client which send it!
 *
 * Modify History:
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0  liuzhihong     2022/11/25            first release
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
#include "shell.h"
static err_t tcpecho_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{

    if (p != NULL)
    {
        printf("We get data: ");
        printf("%s", (char *)p->payload);
        /* update receive buffer */
        tcp_recved(tpcb, p->tot_len);
        /* Returning received data to the client */
        tcp_write(tpcb, p->payload, p->tot_len, 1);
        memset(p->payload, 0, p->tot_len);
        pbuf_free(p);
    }
    else if (err == ERR_OK)
    {
        printf("Connection closed! Waiting client to connect.\n");
        return tcp_close(tpcb);
    }
    return ERR_OK;
}
static err_t tcpecho_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    printf("Connection Established!\r\n");
    tcp_recv(newpcb, tcpecho_recv);
    return ERR_OK;
}

void TcpServerInit(void)
{
    ip_addr_t local_ip;
    struct tcp_pcb *pcb = NULL;
    err_t ret = ERR_OK;
    /* create a tcp_pcb*/
    pcb = tcp_new();
    if (pcb == NULL)
    {
        printf("Tcp Server Open Failed!\r\n");
        printf("Unable to create a new tcp_pcb\r\n");
        return ;
    }
    /* bind the local ip and local port */
    IP_ADDR4(&local_ip, 192, 168, 4, 10);
    ret = tcp_bind(pcb, &local_ip, 5003);
    if (ret == ERR_USE)
    {
        printf("The port and ip  are already binded!\n");
        tcp_close(pcb);
        return;
    }
    /* listening */
    pcb = tcp_listen(pcb);
    /*register a connection-established function*/
    tcp_accept(pcb, tcpecho_accept);
    printf("Tcp Server Open Success!\r\n");
}


SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), tcpserver, TcpServerInit, Start Tcp server test);