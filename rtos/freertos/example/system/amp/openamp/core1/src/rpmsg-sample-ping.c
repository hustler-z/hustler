/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This is a sample demonstration application that showcases usage of rpmsg
 * This application is meant to run on the remote CPU running baremetal code.
 * This application simulate sample rpmsg driver. For this it echo 100
 * time message sent by the rpmsg sample client available in distribution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rpmsg_service.h"
#include "fdebug.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define     VIRTIO_DEV_MASTER_DEBUG_TAG "    MASTER_02"
#define     VIRTIO_DEV_MASTER_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( VIRTIO_DEV_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
#define     VIRTIO_DEV_MASTER_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( VIRTIO_DEV_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
#define     VIRTIO_DEV_MASTER_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( VIRTIO_DEV_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)

#define HELLO_MSG	"hello world!"
#define BYE_MSG		"goodbye!"
#define MSG_LIMIT	100

/* Globals */
static struct rpmsg_endpoint lept;
static int rnum = 0;
static int err_cnt = 0;
static int ept_deleted = 0;

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	char payload[RPMSG_BUFFER_SIZE];
	char seed[20];

	(void)ept;
	(void)src;
	(void)priv;

	memset(payload, 0, RPMSG_BUFFER_SIZE);
	memcpy(payload, data, len);
	VIRTIO_DEV_MASTER_DEBUG_I("received message %d: %s of size %lu \r\n",
		rnum + 1, payload, (unsigned long)len);

	if (rnum == (MSG_LIMIT - 1))
		sprintf (seed, "%s", BYE_MSG);
	else
		sprintf (seed, "%s", HELLO_MSG);

	VIRTIO_DEV_MASTER_DEBUG_I(" seed %s: \r\n", seed);

	if (strncmp(payload, seed, len)) {
		VIRTIO_DEV_MASTER_DEBUG_E(" Invalid message is received.\r\n");
		err_cnt++;
		return RPMSG_SUCCESS;
	}
	rnum++;
	VIRTIO_DEV_MASTER_DEBUG_I(" rnum %d: \r\n", rnum);
	if (rnum == MSG_LIMIT) 
	{
		ept_deleted = 1;
	}
	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	rpmsg_destroy_ept(&lept);
	VIRTIO_DEV_MASTER_DEBUG_I("echo test: service is destroyed\r\n");
	ept_deleted = 1;
}

static void rpmsg_name_service_bind_cb(struct rpmsg_device *rdev,
				       const char *name, uint32_t dest)
{
	VIRTIO_DEV_MASTER_DEBUG_I("new endpoint notification is received.\r\n");
	if (strcmp(name, RPMSG_SERVICE_NAME))
		VIRTIO_DEV_MASTER_DEBUG_E("Unexpected name service %s.\r\n", name);
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
	rnum = 0;
    err_cnt = 0;
    ept_deleted = 0;

	VIRTIO_DEV_MASTER_DEBUG_I(" 1 - Send data to remote core, retrieve the echo");

	/* Create RPMsg endpoint */
	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
			       RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			       rpmsg_endpoint_cb, rpmsg_service_unbind);

	if (ret) {
		VIRTIO_DEV_MASTER_DEBUG_E("Failed to create RPMsg endpoint.\r\n");
		return ret;
	}

	while (!is_rpmsg_ept_ready(&lept))
	{
		VIRTIO_DEV_MASTER_DEBUG_I("start to wait platform_poll \r\n");
		platform_poll(priv);
	}

	VIRTIO_DEV_MASTER_DEBUG_I("RPMSG endpoint is binded with remote.\r\n");
	for (i = 1; i <= MSG_LIMIT; i++) {


		if (i < MSG_LIMIT)
			ret = rpmsg_send(&lept, HELLO_MSG, strlen(HELLO_MSG));
		else
			ret = rpmsg_send(&lept, BYE_MSG, strlen(BYE_MSG));

		if (ret < 0) {
			VIRTIO_DEV_MASTER_DEBUG_E("Failed to send data...\r\n");
			break;
		}
		VIRTIO_DEV_MASTER_DEBUG_I("rpmsg sample test: message %d sent\r\n", i);

		do {
			platform_poll(priv);
		} while ((rnum < i) && !err_cnt);

	}

	VIRTIO_DEV_MASTER_DEBUG_I("**********************************\r\n");
	VIRTIO_DEV_MASTER_DEBUG_I(" Test Results: Error count = %d\r\n", err_cnt);
	VIRTIO_DEV_MASTER_DEBUG_I("**********************************\r\n");

	while (!ept_deleted)
	{
		platform_poll(priv);
	}
	rpmsg_destroy_ept(&lept);
	VIRTIO_DEV_MASTER_DEBUG_I("Quitting application .. rpmsg sample test end.\r\n");

	return 0;
}

int rpmsg_sample_ping(struct rpmsg_device *rdev, void *priv)
{
	metal_assert(rdev);
    metal_assert(priv);
	int ret;
	
	VIRTIO_DEV_MASTER_DEBUG_I("Starting rpmsg_sample_ping application...\r\n");

	ret = app(rdev, priv);
	if (ret != 0)
    {
        VIRTIO_DEV_MASTER_DEBUG_E("Rpmsg_sample_ping application error,code:0x%x",ret);
        return ret;
    }

	VIRTIO_DEV_MASTER_DEBUG_I("Stopping rpmsg_sample_ping application...\r\n");

	return ret;
}

