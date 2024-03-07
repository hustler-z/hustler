/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This is a sample demonstration application that showcases usage of rpmsg
 * This application is meant to run on the remote CPU running baremetal code.
 * This application allows to check the compatibility with running on
 * the master CPU. For this it echo MSG_LIMIT time message sent by the rpmsg
 * sample client available in distribution.
 */

#include <stdio.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rpmsg_service.h"
#include "fdebug.h"

#define     VIRTIO_DEV_SLAVE_DEBUG_TAG "    SLAVE_02"
#define     VIRTIO_DEV_SLAVE_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( VIRTIO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     VIRTIO_DEV_SLAVE_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( VIRTIO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     VIRTIO_DEV_SLAVE_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( VIRTIO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)

#define MSG_LIMIT	100

static struct rpmsg_endpoint lept;
int shutdown_req = 0;
uint32_t count = 0;
/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	(void)priv;
	(void)src;
	char payload[RPMSG_BUFFER_SIZE];

	/* Send data back MSG_LIMIT time to master */
	memset(payload, 0, RPMSG_BUFFER_SIZE);
	memcpy(payload, data, len);
	if (++count <= MSG_LIMIT) 
	{
		VIRTIO_DEV_SLAVE_DEBUG_I("echo message number %u: %s\r\n",(unsigned int)count, payload);
		if (rpmsg_send(ept, (char *)data, len) < 0) 
		{
			VIRTIO_DEV_SLAVE_DEBUG_E("rpmsg_send failed\r\n");
			shutdown_req = 1;
		}
	}
	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	VIRTIO_DEV_SLAVE_DEBUG_I("unexpected Remote endpoint destroy\r\n");
	shutdown_req = 1;
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
static int app(struct rpmsg_device *rdev, void *priv)
{
	int ret;
	shutdown_req = 0;
	count = 0;
	/* Initialize RPMSG framework */
	VIRTIO_DEV_SLAVE_DEBUG_I("Try to create rpmsg endpoint.\r\n");

	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
			       RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			       rpmsg_endpoint_cb, rpmsg_service_unbind);
	if (ret) {
		VIRTIO_DEV_SLAVE_DEBUG_E("Failed to create endpoint.\r\n");
		return -1;
	}

	VIRTIO_DEV_SLAVE_DEBUG_I("Successfully created rpmsg endpoint.\r\n");
	while (1) {
		platform_poll(priv);

		/* we got a shutdown request, exit */
		if (shutdown_req) 
		{
			break;
		}
	}
	rpmsg_destroy_ept(&lept);

	return 0;
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int rpmsg_sample_echo(struct rpmsg_device *rdev, void *priv)
{
	metal_assert(rdev);
    metal_assert(priv);
	int ret;
	
	VIRTIO_DEV_SLAVE_DEBUG_I("Starting rpmsg_sample_echo application...\r\n");

	ret = app(rdev, priv);
	if (ret != 0)
    {
        VIRTIO_DEV_SLAVE_DEBUG_E("Rpmsg_sample_echo application error,code:0x%x",ret);
        return ret;
    }

	VIRTIO_DEV_SLAVE_DEBUG_I("Stopping rpmsg_sample_echo application...\r\n");

	return ret;
}
