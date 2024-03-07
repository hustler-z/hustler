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
 * FilePath: tcp_client.c
 * Created Date: 2022-11-07 10:16:55
 * Last Modified: 2023-06-07 11:00:17
 * Description:  This file is for creating a tcp client.By Tcp Connection,The client can send a tcp packet to the destination host periodically.
 *
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong  2022/11/23          first release
 *  1.1   liuzhihong  2023/06/06      memory double free solved 
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

#define TCP_CLIENT_PORT 0
#define TCP_SERVER_PORT 5001
void TcpClientInit(void);
static struct tcp_pcb *client_pcb = NULL;
int init_flag=0;
static void client_err(void *arg, err_t err)
{
    /*connect failed, free client_pcb*/
    printf("Connect error! PCB Closed by Core!\n");
    if(client_pcb->state == SYN_SENT)  goto exit;

    tcp_close(client_pcb);

exit:
    init_flag=0;
    printf("Please input command tcp_client to connect to server again!\n");
}
static err_t client_send(void *arg, struct tcp_pcb *tpcb)
{
    uint8_t send_buf[] = "This is a data from TCP Client.\n";
    printf("Writing data to Send buffer \n");
    return  tcp_write(tpcb, send_buf, sizeof(send_buf), 1);;
}
static err_t client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    err_t ret = ERR_OK;
    if (p != NULL)
    {
        /*update the tcp send windows*/
        tcp_recved(tpcb, p->tot_len);
        printf("We get a data from server\n");
        memset(p->payload, 0, p->tot_len);
        pbuf_free(p);
    }
    else if (err == ERR_OK)
    {
        printf("Server has been Disconnected!\n");
    }
    return ret;
}
static err_t client_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    
    printf("Tcp Client Connected To Server Success!\n");
    /*register a timer func  the Cycle time is 10*500ms=5s */
    tcp_poll(pcb, client_send, 10);
    /*register a receive func*/
    tcp_recv(pcb, client_recv);

    init_flag=1;
    return ERR_OK;
}

void TcpClientInit(void)
{

    if(init_flag)
    {
        printf("Tcp client has been opened, Please Close it first!\n");
        return ;
    }
    err_t ret;
    ip_addr_t tmp_ip;
    /* create a tcp_pcb */
    client_pcb = tcp_new();
    IP_ADDR4(&tmp_ip, 192, 168, 4, 10);
    tcp_bind(client_pcb, &tmp_ip, TCP_CLIENT_PORT);
    IP_ADDR4(&tmp_ip, 192, 168, 4, 50);
    printf("**************** Client Start Connectting ****************\n");
    /*return immediately, call pcb->err if connect failed*/
    ret = tcp_connect(client_pcb, &tmp_ip, TCP_SERVER_PORT, client_connected); 
    if (ret != ERR_OK)
    {
        printf("Memory is not enough! Connect Failed!\r\n");
        tcp_close(client_pcb);
        init_flag=0;
        return;
    }
    /*register a Exception Handling Functions*/
    tcp_err(client_pcb, client_err);
}
void TcpClientDeinit(void)
{
    if (init_flag)
    {
        printf("We are Closing Tcp Client!\n");
        tcp_close(client_pcb);
        init_flag=0;
    }
    else
    {
        printf("There is no alive client ,Please Open Client first!\n");
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), tcpclient, TcpClientInit, Start Tcp client test);
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), tcpclientclose, TcpClientDeinit, Close Tcp client);