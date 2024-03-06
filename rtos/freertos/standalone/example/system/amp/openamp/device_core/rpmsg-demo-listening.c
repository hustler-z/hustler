/*
 * @Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * @FilePath: rpmsg-demo-listening.c
 * @Date: 2023-04-23 16:42:27
 * @LastEditTime: 2023-04-23 16:42:27
 * @Description:  This file is for wait core0 msg
 * 
 * @Modify History: 
 *  Ver   Who           Date        Changes
 * ----- ------         --------    --------------------------------------
 * 1.0  liushengming    2023/04/23  first release
 */

/***************************** Include Files *********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rpmsg_service.h"
#include "fcache.h"
#include "fdebug.h"

/************************** Constant Definitions *****************************/
/***************** Macros (Inline Functions) Definitions *********************/

#define SHUTDOWN_MSG                0xEF56A55A
#define RECV_MSG                    0xE5E5E5E5

#define     DEMO_LIST_SLAVE_DEBUG_TAG "    DEMO_LIST_SLAVE"
#define     DEMO_LIST_SLAVE_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( DEMO_LIST_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     DEMO_LIST_SLAVE_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( DEMO_LIST_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     DEMO_LIST_SLAVE_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( DEMO_LIST_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)

#define DEMO_LISTENING_ADDR   RPMSG_RESERVED_ADDRESSES - 2

#define RPMSG_PING_DEMO     0x1
#define RPMSG_SAM_PING_DEMO 0X2
#define RPMSG_MAT_MULT_DEMO 0X3
#define RPMSG_NO_COPY_DEMO  0X4

static struct rpmsg_endpoint lept;
static volatile u32 flag_req = 0;
static volatile int demo_flag = 0;
/************************** Function Prototypes ******************************/
extern int rpmsg_ping(struct rpmsg_device *rdev, void *priv) ;
extern int rpmsg_sample_ping(struct rpmsg_device *rdev, void *priv) ;
extern int matrix_multiply(struct rpmsg_device *rdev, void *priv) ;
extern int rpmsg_nocopy_ping(struct rpmsg_device *rdev, void *priv) ;
/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
{
    (void)priv;
    DEMO_LIST_SLAVE_DEBUG_E("src:0x%x",src);
    ept->dest_addr = src;
    /* On reception of a shutdown we signal the application to terminate */
    if ((*(unsigned int *)data) == SHUTDOWN_MSG)
    {
        DEMO_LIST_SLAVE_DEBUG_I("Shutdown message is received.\r\n");
        flag_req = SHUTDOWN_MSG;
        return RPMSG_SUCCESS;
    }

    demo_flag = (*(unsigned int *)data);
    /* Send data back to master */
    if (rpmsg_send(ept, data, len) < 0)
    {
        DEMO_LIST_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
    }
    flag_req = RECV_MSG;
    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    rpmsg_destroy_ept(&lept);
    DEMO_LIST_SLAVE_DEBUG_I("Unexpected remote endpoint destroy.\r\n");
    flag_req = SHUTDOWN_MSG;
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
static int app(struct rpmsg_device *rdev, void *priv)
{
    int ret;
    /* Initialize RPMSG framework */
    ret = rpmsg_create_ept(&lept, rdev, DEMO_SERVICE_NAME, DEMO_LISTENING_ADDR, RPMSG_ADDR_ANY, rpmsg_endpoint_cb, rpmsg_service_unbind);
    if (ret)
    {
        DEMO_LIST_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    DEMO_LIST_SLAVE_DEBUG_I("Successfully created rpmsg endpoint.\r\n");
    demo_flag = 0;
    flag_req = 0;
    while (1)
    {
        platform_poll(priv);
        /* we got a shutdown request, exit */
        if (demo_flag != 0 && flag_req == RECV_MSG)
        {
            if (demo_flag == RPMSG_PING_DEMO)
            {
                printf("\r\n\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("***********Demo rpmsg_ping running***********......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                ret = rpmsg_ping(rdev, priv);
                if (ret != 0)
                {
                    DEMO_LIST_SLAVE_DEBUG_E("rpmsg_echo running error,ecode:%d.", ret);
                    return 0;
                }
            }

            if (demo_flag == RPMSG_SAM_PING_DEMO)
            {
                printf("\r\n\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*******Demo rpmsg_sample_ping running********......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                ret = rpmsg_sample_ping(rdev, priv);
                if (ret != 0)
                {
                    DEMO_LIST_SLAVE_DEBUG_E("rpmsg_sample_ping running error,ecode:%d.", ret);
                    return 0;
                }
            }

            if (demo_flag == RPMSG_MAT_MULT_DEMO)
            {
                printf("\r\n\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("********Demo matrix_multiply running*********......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                ret = matrix_multiply(rdev, priv);
                if (ret != 0)
                {
                    DEMO_LIST_SLAVE_DEBUG_E("matrix_multiply running error,ecode:%d.", ret);
                    return 0;
                }
            }

            if (demo_flag == RPMSG_NO_COPY_DEMO)
            {
                printf("\r\n\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*******Demo rpmsg_nocopy_ping running********......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                ret = rpmsg_nocopy_ping(rdev, priv);
                if (ret != 0)
                {
                    DEMO_LIST_SLAVE_DEBUG_E("rpmsg_nocopy_ping running error,ecode:%d.", ret);
                    return 0;
                }
            }
            flag_req = 0;
            demo_flag = 0;
            DEMO_LIST_SLAVE_DEBUG_I(" Demo running over...");
        }
        if (flag_req == SHUTDOWN_MSG)
        {
            break;
        }
    }
    DEMO_LIST_SLAVE_DEBUG_I("Listening demo over.");
    return ret;
}

int rpmsg_demo_listening(int argc, char *argv[])
{
    void *platform;
    struct rpmsg_device *rpdev;
    int ret;

    DEMO_LIST_SLAVE_DEBUG_I("Starting application...\r\n");
    /* Initialize platform */
    ret = platform_init(argc, argv, &platform);
    if (ret)
    {
        DEMO_LIST_SLAVE_DEBUG_E("Failed to initialize platform.\r\n");
        ret = -1;
    }
    else
    {
        rpdev = platform_create_rpmsg_vdev(platform, 0, VIRTIO_DEV_SLAVE, NULL,NULL);
        if (!rpdev)
        {
            DEMO_LIST_SLAVE_DEBUG_E("Failed to create rpmsg virtio device.\r\n");
            ret = -1;
        }
        else
        {
            ret = app(rpdev, platform);
            if (ret != 0)
            {
                DEMO_LIST_SLAVE_DEBUG_E("App listening running error.");
            }
            platform_release_rpmsg_vdev(rpdev, platform);
        }
    }

    DEMO_LIST_SLAVE_DEBUG_I("Stopping application...\r\n");
    platform_cleanup(platform);

    return ret;
}

