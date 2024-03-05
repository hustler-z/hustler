/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * @Modify History: 
 *  Ver   Who  			Date   		Changes
 * ----- ------  		-------- 	--------------------------------------
 * 1.0  liushengming    2023/03/14  init
 */
/*****************************************************************************
 * ipi_latency_demod.c
 * This is the remote side of the IPI latency measurement demo.
 * This demo does the follwing steps:
 *
 *  1. Open the shared memory device.
 *  1. Open the TTC timer device.
 *  2. Open the IPI device.
 *  3. Register IPI interrupt handler.
 *  6. When it receives APU IPI interrupt, the IPI interrupt handler to clear atomic flag.
 *  7. Check the shared memory to see if demo is on. If the demo is on,
 *     reset the RPU to APU TTC counter and kick IPI to notify the remote.
 *  8. If the shared memory indicates the demo is off, cleanup resource:
 *     disable IPI interrupt and deregister the IPI interrupt handler.
 */

#include <unistd.h>
#include <stdio.h>
#include <metal/cpu.h>
#include <sys/types.h>
#include <metal/atomic.h>
#include <metal/io.h>
#include <metal/device.h>
#include <metal/irq.h>
#include <metal/errno.h>
#include "common.h"
#include "sdkconfig.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fsleep.h"
#include "rpu_running.h"
#include "fgeneric_timer.h"

#include "fdebug.h"
#define LIBMETAL_IPLA_RPU_DEBUG_TAG "LIBMETAL_IPLA_RPU"
#define LIBMETAL_IPLA_RPU_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(LIBMETAL_IPLA_RPU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_IPLA_RPU_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(LIBMETAL_IPLA_RPU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_IPLA_RPU_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(LIBMETAL_IPLA_RPU_DEBUG_TAG, format, ##__VA_ARGS__)

#define NS_PER_SEC 1000000000

/* Shared memory offset */
#define SHM_DEMO_CNTRL_OFFSET 0x0

#define DEMO_STATUS_IDLE 0x0
#define DEMO_STATUS_START 0x1 /* Status value to indicate demo start */

struct channel_s
{
	struct metal_io_region *ipi_io; /* IPI metal i/o region */
	struct metal_io_region *shm_io; /* Shared memory metal i/o region */
	struct metal_io_region *ttc_io; /* TTC metal i/o region */
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
 * @brief measure_ipi_latencyd() - measure IPI latency with libmetal
 *        Loop until APU tells RPU to stop via shared memory.
 *        In loop, wait for interrupt.
 *
 * @param[in] ch - channel information
 * @return - 0 on success, error code if failure.
 */
static int measure_ipi_latencyd(struct channel_s *ch)
{
    u32 i=0,j=0;
    u64 tstart,tend,tdiff;
	u64 temp;

	wait_for_notified(&ch->remote_nkicked);
	/* Get start counter */
	tstart = GenericTimerRead(GENERIC_TIMER_ID0);
    while (1)
    {
		i++;
		if (metal_io_read32(ch->shm_io, SHM_DEMO_CNTRL_OFFSET) == DEMO_STATUS_START)
		{
			/* Kick IPI to notify the remote */
			InterruptCoreInterSend(CONFIG_IPI_IRQ_NUM, CONFIG_TARGET_CPU_MASK);
		}
		else
		{
			break;
		}
		wait_for_notified(&ch->remote_nkicked);
    }
    tend = GenericTimerRead(GENERIC_TIMER_ID0);
    tdiff = tend - tstart;
	temp = tdiff / i * NS_PER_SEC / GenericTimerFrequecy();
	/* report avg latencies */
	LIBMETAL_IPLA_RPU_DEBUG_I("IPI latency result with %u iterations:\n", i);
	LIBMETAL_IPLA_RPU_DEBUG_I("APU to RPU average latency: %lu ns.\n", temp);
	return 0;
}

int ipi_latency_demod()
{
	struct channel_s ch;
    struct metal_device *ipi_dev = NULL;
    struct metal_device *shm_dev = NULL;
    struct metal_device *ttc_dev = NULL;
	int ret = 0;

    /* Open shared memory device */
    ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &shm_dev);
    if (ret)
    {
        LIBMETAL_IPLA_RPU_DEBUG_E("Failed to open device %s.\n", SHM_DEV_NAME);
        goto out;
    }

    /* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &ipi_dev);
	if (ret)
	{
		LIBMETAL_IPLA_RPU_DEBUG_E("Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}

    /* Open TTC device */
	ret = metal_device_open(BUS_NAME, TTC_DEV_NAME, &ttc_dev);
	if (ret)
	{
		LIBMETAL_IPLA_RPU_DEBUG_E("Failed to open device %s.\n", TTC_DEV_NAME);
		goto out;
	}

	memset(&ch, 0, sizeof(ch));

	/* Get shared memory device IO region */
	if (!shm_dev)
	{
		ret = -ENODEV;
		goto out;
	}
	ch.shm_io = metal_device_io_region(shm_dev, 0);
	if (!ch.shm_io)
	{
		LIBMETAL_IPLA_RPU_DEBUG_E("Failed to map io region for %s.\n", shm_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Get TTC IO region */
	if (!ttc_dev)
	{
		ret = -ENODEV;
		goto out;
	}
	ch.ttc_io = metal_device_io_region(ttc_dev, 0);
	if (!ch.ttc_io)
	{
		LIBMETAL_IPLA_RPU_DEBUG_E("Failed to map io region for %s.\n", ttc_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Get IPI device IO region */
	if (!ipi_dev)
	{
		ret = -ENODEV;
		goto out;
	}
	ch.ipi_io = metal_device_io_region(ipi_dev, 0);
	if (!ch.ipi_io)
	{
		LIBMETAL_IPLA_RPU_DEBUG_E("Failed to map io region for %s.\n", ipi_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Get the IPI IRQ from the opened IPI device */
    InterruptSetPriority(CONFIG_IPI_IRQ_NUM, CONFIG_IPI_IRQ_NUM_PRIORITY);
    InterruptInstall(CONFIG_IPI_IRQ_NUM, ipi_irq_handler, &ch, NULL);

	/* initialize remote_nkicked */
	atomic_init(&ch.remote_nkicked, 1);
	/* Enable IPI interrupt */
    InterruptUmask(CONFIG_IPI_IRQ_NUM);

	/* Run atomic operation demo */
	ret = measure_ipi_latencyd(&ch);

    /* disable IPI interrupt */
    InterruptMask(CONFIG_IPI_IRQ_NUM);

out:
    if (shm_dev)
    {
        metal_device_close(shm_dev);
    }
    if (ipi_dev)
    {
        metal_device_close(ipi_dev);
    }
    if (ttc_dev)
    {
        metal_device_close(ttc_dev);
    }
	return ret;
}
