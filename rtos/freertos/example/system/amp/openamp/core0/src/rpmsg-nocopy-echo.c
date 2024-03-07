/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020, STMicroelectronics
 */

/*
 * This is a sample demonstration application that showcases usage of rpmsg
 * This application is meant to run on the remote CPU running baremetal code.
 * This application echoes back data that was sent to it by the master core.
 */

#include <stdio.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rpmsg_service.h"
#include "fdebug.h"

#define     NO_COPY_SLAVE_DEBUG_TAG "    SLAVE_04"
#define     NO_COPY_SLAVE_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( NO_COPY_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     NO_COPY_SLAVE_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( NO_COPY_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     NO_COPY_SLAVE_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( NO_COPY_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)

#define SHUTDOWN_MSG	0xEF56A55A

static struct rpmsg_endpoint lept;
static int shutdown_req;

struct rpmsg_rcv_msg {
	void *data;
	size_t len;
	struct rpmsg_endpoint *ept;
	struct rpmsg_rcv_msg *next;
};

struct rpmsg_rcv_msg *rpmsg_list;

/*-----------------------------------------------------------------------------
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------
 */
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	(void)src;
	(void)priv;
	struct rpmsg_rcv_msg *rpmsg_node, *p_msg;

	/* On reception of a shutdown we signal the application to terminate */
	if ((*(unsigned int *)data) == SHUTDOWN_MSG) {
		NO_COPY_SLAVE_DEBUG_I("shutdown message is received.\r\n");
		shutdown_req = 1;
		return RPMSG_SUCCESS;
	}

	rpmsg_node = metal_allocate_memory(sizeof(*rpmsg_node));
	if (!rpmsg_node) {
		NO_COPY_SLAVE_DEBUG_E("rpmsg_node allocation failed\r\n");
		return -1;
	}
	rpmsg_hold_rx_buffer(ept, data);
	rpmsg_node->ept = ept;
	rpmsg_node->data = data;
	rpmsg_node->len = len;
	rpmsg_node->next = NULL;

	if (!rpmsg_list)
		rpmsg_list = rpmsg_node;
	else {
		p_msg = rpmsg_list;
		while (p_msg->next)
			p_msg = p_msg->next;
		p_msg->next = rpmsg_node;
	}

	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	NO_COPY_SLAVE_DEBUG_I("unexpected Remote endpoint destroy\r\n");
	shutdown_req = 1;
}

/*-----------------------------------------------------------------------------
 *  Application
 *-----------------------------------------------------------------------------
 */
static int app(struct rpmsg_device *rdev, void *priv)
{
	int ret;
	struct rpmsg_rcv_msg *rpmsg_node;
	shutdown_req = 0;
	/* Initialize RPMSG framework */
	NO_COPY_SLAVE_DEBUG_I("Try to create rpmsg endpoint.\r\n");

	rpmsg_list = NULL;
	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
			       RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			       rpmsg_endpoint_cb,
			       rpmsg_service_unbind);
	if (ret) {
		NO_COPY_SLAVE_DEBUG_E("Failed to create endpoint.\r\n");
		return -1;
	}

	NO_COPY_SLAVE_DEBUG_I("Successfully created rpmsg endpoint.\r\n");
	while (1) {
		platform_poll(priv);
		/* we got a shutdown request, exit */
		if (shutdown_req) {
			break;
		}
		while (rpmsg_list) {
			/* Send data back to master */
			ret = rpmsg_send(rpmsg_list->ept, rpmsg_list->data,
					 rpmsg_list->len);
			if (ret < 0) {
				NO_COPY_SLAVE_DEBUG_E("rpmsg_send failed\r\n");
				return ret;
			}
			rpmsg_release_rx_buffer(rpmsg_list->ept,
						rpmsg_list->data);
			rpmsg_node = rpmsg_list;
			rpmsg_list = rpmsg_list->next;
			metal_free_memory(rpmsg_node);
		}
	}
	rpmsg_destroy_ept(&lept);

	return 0;
}

/*-----------------------------------------------------------------------------
 *  Application entry point
 *-----------------------------------------------------------------------------
 */
int rpmsg_nocopy_echo(struct rpmsg_device *rdev, void *priv)
{
	metal_assert(rdev);
    metal_assert(priv);
	int ret;

	NO_COPY_SLAVE_DEBUG_I("Starting rpmsg_nocopy_echo application...\r\n");

	ret = app(rdev, priv);
	if (ret != 0)
    {
        NO_COPY_SLAVE_DEBUG_E("Rpmsg_nocopy_echo application error,code:0x%x",ret);
        return ret;
    }

	NO_COPY_SLAVE_DEBUG_I("Stopping rpmsg_nocopy_echo application...\r\n");

	return ret;
}
