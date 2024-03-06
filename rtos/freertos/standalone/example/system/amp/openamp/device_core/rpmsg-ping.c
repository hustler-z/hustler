/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* This is a sample demonstration application that showcases usage of rpmsg
This application is meant to run on the remote CPU running baremetal code.
This application echoes back data that was sent to it by the master core. */

/***************************** Include Files *********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rpmsg_service.h"
#include "fdebug.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define     PING_DEV_MASTER_DEBUG_TAG "    MASTER_01"
#define     PING_DEV_MASTER_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( PING_DEV_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
#define     PING_DEV_MASTER_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( PING_DEV_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
#define     PING_DEV_MASTER_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( PING_DEV_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
/**************************** Type Definitions *******************************/

#define PAYLOAD_MIN_SIZE            1
#define DEMO_ADDR                   0x1

/************************** Variable Definitions *****************************/
struct _payload
{
    unsigned long num;
    unsigned long size;
    unsigned char data[];
};

static char flg_cnt;
/* Globals */
static struct rpmsg_endpoint lept;
static struct _payload *i_payload;
static int rnum = 0;
static int err_cnt = 0;
static int ept_deleted = 0;

/************************** Function Prototypes ******************************/

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
                             uint32_t src, void *priv)
{
    int i;
    struct _payload *r_payload = (struct _payload *)data;

    (void)ept;
    (void)src;
    (void)priv;

    if (r_payload->size == 0)
    {
        PING_DEV_MASTER_DEBUG_E(" Invalid size of package is received.\r\n");
        err_cnt++;
        return RPMSG_SUCCESS;
    }
    /* Validate data buffer integrity. */
    for (i = 0; i < (int)r_payload->size; i++)
    {
        if (r_payload->data[i] != flg_cnt)
        {
            PING_DEV_MASTER_DEBUG_I("Data corruption at index %d.\r\n", i);
            PING_DEV_MASTER_DEBUG_I("Want data is %d.\r\n", flg_cnt);
            PING_DEV_MASTER_DEBUG_I("Get data is %d.\r\n", r_payload->data[i]);
            err_cnt++;
            break;
        }
    }
    rnum = r_payload->num + 1;
    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    rpmsg_destroy_ept(&lept);
    PING_DEV_MASTER_DEBUG_I("Echo test: service is destroyed.\r\n");
    ept_deleted = 1;
}

static void rpmsg_name_service_bind_cb(struct rpmsg_device *rdev,
                                       const char *name, uint32_t dest)
{
    PING_DEV_MASTER_DEBUG_I("New endpoint notification is received.\r\n");
    if (strcmp(name, RPMSG_SERVICE_NAME))
    {
        PING_DEV_MASTER_DEBUG_E("Unexpected name service %s.\r\n", name);
    }
    else
        (void)rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
                               RPMSG_ADDR_ANY, dest,
                               rpmsg_endpoint_cb,
                               rpmsg_service_unbind);

}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
static int app(struct rpmsg_device *rdev, void *priv)
{
    int ret;
    int i;
    int size, max_size, num_payloads;
    int expect_rnum = 0;
    rnum = 0;
    err_cnt = 0;
    ept_deleted = 0;

    PING_DEV_MASTER_DEBUG_I("Send data to remote core, retrieve the echo.");
    PING_DEV_MASTER_DEBUG_I(" and validate its integrity ..\r\n");

    max_size = rpmsg_virtio_get_buffer_size(rdev);
    if (max_size < 0)
    {
        PING_DEV_MASTER_DEBUG_E("No avaiable buffer size.\r\n");
        return -1;
    }
    max_size -= sizeof(struct _payload);
    num_payloads = max_size - PAYLOAD_MIN_SIZE + 1;
    i_payload =
        (struct _payload *)metal_allocate_memory(2 * sizeof(unsigned long) +
                max_size);

    if (!i_payload)
    {
        PING_DEV_MASTER_DEBUG_E("Memory allocation failed.\r\n");
        return -1;
    }

    /* Create RPMsg endpoint */
    ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
                           RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                           rpmsg_endpoint_cb, rpmsg_service_unbind);

    if (ret)
    {
        PING_DEV_MASTER_DEBUG_E("Failed to create RPMsg endpoint.\r\n");
        metal_free_memory(i_payload);
        return ret;
    }
    while (!is_rpmsg_ept_ready(&lept))
    {
        PING_DEV_MASTER_DEBUG_I("start to wait platform_poll \r\n");
        platform_poll(priv);
    }

    PING_DEV_MASTER_DEBUG_I("RPMSG endpoint is binded with remote.\r\n");
    for (i = 0, size = PAYLOAD_MIN_SIZE; i < num_payloads; i++, size++)
    {
        i_payload->num  = i;
        i_payload->size = size;
        flg_cnt++;
        /* Mark the data buffer. */
        memset(&(i_payload->data[0]), flg_cnt, size);

        ret = rpmsg_send(&lept, i_payload, (2 * sizeof(unsigned long)) + size);
        if (ret < 0)
        {
            PING_DEV_MASTER_DEBUG_E("Failed to send data...\r\n");
            break;
        }

        expect_rnum++;
        do
        {
            platform_poll(priv);
        }
        while ((rnum < expect_rnum) && !err_cnt && !ept_deleted);

    }

    PING_DEV_MASTER_DEBUG_I("**********************************\r\n");
    PING_DEV_MASTER_DEBUG_I(" Test Results: Error count = %d \r\n", err_cnt);
    PING_DEV_MASTER_DEBUG_I("**********************************\r\n");
    /* Destroy the RPMsg endpoint */
    /* lept.addr = RPMSG_ADDR_ANY 远程endpoint 才能触发 rpmsg_service_unbind 事件回调退出 */
    rpmsg_destroy_ept(&lept);
    PING_DEV_MASTER_DEBUG_I("Quitting application .. Echo test end\r\n");

    metal_free_memory(i_payload);
    return 0;
}

int rpmsg_ping(struct rpmsg_device *rdev, void *priv)
{
    metal_assert(rdev);
    metal_assert(priv);
    int ret;

    PING_DEV_MASTER_DEBUG_I("Starting rpmsg_ping application...\r\n");
   
    ret = app(rdev, priv);
    if (ret != 0)
    {
        PING_DEV_MASTER_DEBUG_E("Rpmsg_ping application error,code:0x%x",ret);
        return ret;
    }

    PING_DEV_MASTER_DEBUG_I("Stopping rpmsg_ping application...\r\n");

    return ret;
}

