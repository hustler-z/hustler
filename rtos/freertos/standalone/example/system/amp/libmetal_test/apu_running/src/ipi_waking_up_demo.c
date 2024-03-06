/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * @Modify History: 
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2023/03/21  init
 */
#include <unistd.h>
#include <stdio.h>
#include <metal/errno.h>
#include <sys/types.h>
#include <metal/atomic.h>
#include <metal/io.h>
#include <metal/device.h>
#include <metal/irq.h>
#include "common.h"
#include "sdkconfig.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fsleep.h"

#include "fdebug.h"
#define LIBMETAL_IPWU_APU_DEBUG_TAG "LIBMETAL_IPWU_APU"
#define LIBMETAL_IPWU_APU_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(LIBMETAL_IPWU_APU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_IPWU_APU_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(LIBMETAL_IPWU_APU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_IPWU_APU_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(LIBMETAL_IPWU_APU_DEBUG_TAG, format, ##__VA_ARGS__)

#define IPI_APU_DEMO_CNTRL_OFFSET 0x500
#define IPI_RPU_DEMO_CNTRL_OFFSET 0x504

#define DEMO_APU_STATUS_IDLE 0x11
#define DEMO_APU_STATUS_START 0x22 /* Status value to indicate demo start */
#define DEMO_RPU_STATUS_IDLE 0x33
#define DEMO_RPU_STATUS_START 0x44 /* Status value to indicate demo start */

#define DEMO_SYNC_IRQ_NUM 0x0
#define DEMO_SYNC_IRQ_NUM_PRIORITY 0x0

static u32 target_cpu_mask;

struct channel_s
{
	struct metal_device *ipi_dev;	/* IPI metal device */
	struct metal_io_region *ipi_io; /* IPI metal i/o region */
	uint32_t ipi_mask;				/* RPU IPI mask */
	atomic_int remote_nkicked;		/* 0 - kicked from remote */
};

/**
 * @brief ipi_irq_handler() - IPI interrupt handler
 *        It will clear the notified flag to mark it's got an IPI interrupt.
 *
 * @param[in] vect_id - IPI interrupt vector ID
 * @param[in/out] priv - communication channel data for this application.
 *
 */
static void ipi_irq_handler(s32 vect_id, void *priv)
{
	struct channel_s *ch = (struct channel_s *)priv;

	(void)vect_id;
	if (ch)
	{
        atomic_flag_clear(&ch->remote_nkicked);
	}
}

/**
 * @brief measure_ipi_waking_up() -  wait for interrupt.
 *
 * @param[in] ch - channel information
 * @return void.
 */
static void measure_ipi_waking_up(struct channel_s *ch)
{

	metal_io_write32(ch->ipi_io,IPI_APU_DEMO_CNTRL_OFFSET,DEMO_APU_STATUS_IDLE);
	/* wait rpu DEMO_STATUS_IDLE */
	while (!(metal_io_read32(ch->ipi_io, IPI_RPU_DEMO_CNTRL_OFFSET) == DEMO_RPU_STATUS_IDLE));
	/* waiting RPU ipi */
	wait_for_notified(&ch->remote_nkicked);
	/* complete waking up */
	metal_io_write32(ch->ipi_io,IPI_APU_DEMO_CNTRL_OFFSET,DEMO_APU_STATUS_START);
	/* Kick IPI to notify the remote */
	InterruptCoreInterSend(DEMO_SYNC_IRQ_NUM, target_cpu_mask);/*apu first*/
}

int ipi_waking_up_demo(u32 core_mask)
{
	target_cpu_mask = core_mask;
	struct metal_device *dev;
	struct metal_io_region *io;
	struct channel_s ch;
	int ipi_irq;
	int ret = 0;

	memset(&ch, 0, sizeof(ch));

	/* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &dev);
	if (ret)
	{
		LIBMETAL_IPWU_APU_DEBUG_E("Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}

	/* Get IPI device IO region */
	io = metal_device_io_region(dev, 0);
	if (!io)
	{
		LIBMETAL_IPWU_APU_DEBUG_E("Failed to map io region for %s.\n", dev->name);
		ret = -ENODEV;
		goto out;
	}
	ch.ipi_dev = dev;
	ch.ipi_io = io;

	/* Get the IPI IRQ from the sdkconfig.h */
    InterruptSetPriority(DEMO_SYNC_IRQ_NUM, DEMO_SYNC_IRQ_NUM_PRIORITY);
    /* Register IPI irq handler */
    InterruptInstall(DEMO_SYNC_IRQ_NUM, ipi_irq_handler, &ch, NULL);

	/* initialize remote_nkicked */
	atomic_init(&ch.remote_nkicked, 1);

	/* Enable IPI interrupt */
    InterruptUmask(DEMO_SYNC_IRQ_NUM);
	
	/* Run atomic operation demo */
	measure_ipi_waking_up(&ch);

	/* disable IPI interrupt */
    InterruptMask(DEMO_SYNC_IRQ_NUM);

out:
	if (ch.ipi_dev)
		metal_device_close(ch.ipi_dev);
	return ret;
}
