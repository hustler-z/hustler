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
 * ipi_latency_demo.c
 * This demo measures the IPI latency between the APU and RPU.
 * This demo does the follwing steps:
 *
 *  1. Get the shared memory device I/O region.
 *  2. Get the TTC timer device I/O region.
 *  3. Get the IPI device I/O region.
 *  4. Register IPI interrupt handler.
 *  5. Write to shared memory to indicate demo starts,get the ticks of start.
 *  6. Wait the RPU IPI interrupt,then send IPI interrupt to RPU. 
 *  7. Repeat step 5 for 1000 times
 *  8. Accumulate APU to RPU and RPU to APU ticks counter values.
 *  9. Write shared memory to indicate RPU about demo finishes and kick
 *     IPI to notify.
 * 10. Clean up: disable IPI interrupt, deregister the IPI interrupt handler.
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
#include "fgeneric_timer.h"

#include "fdebug.h"
#define LIBMETAL_IPLA_APU_DEBUG_TAG "LIBMETAL_IPLA_APU"
#define LIBMETAL_IPLA_APU_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(LIBMETAL_IPLA_APU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_IPLA_APU_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(LIBMETAL_IPLA_APU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_IPLA_APU_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(LIBMETAL_IPLA_APU_DEBUG_TAG, format, ##__VA_ARGS__)

#define NS_PER_SEC 1000000000

/* Shared memory offset */
#define SHM_DEMO_CNTRL_OFFSET 0x0

#define DEMO_STATUS_IDLE 0x0
#define DEMO_STATUS_START 0x1 /* Status value to indicate demo start */

#define ITERATIONS 1000

static u32 target_cpu_mask;

struct channel_s
{
	struct metal_device *ipi_dev;	/* IPI metal device */
	struct metal_io_region *ipi_io; /* IPI metal i/o region */
	struct metal_device *shm_dev;	/* Shared memory metal device */
	struct metal_io_region *shm_io; /* Shared memory metal i/o region */
	struct metal_device *ttc_dev;	/* TTC metal device */
	struct metal_io_region *ttc_io; /* TTC metal i/o region */
	uint32_t ipi_mask;				/* RPU IPI mask */
	atomic_int remote_nkicked;		/* 0 - kicked from remote */
};

/**
 * @brief ipi_irq_handler() - IPI interrupt handler
 *        It will clear the notified flag to mark it's got an IPI interrupt.
 *        It will stop the RPU->APU timer and will clear the notified
 *        flag to mark it's got an IPI interrupt
 *
 * @param[in] vect_id - IPI interrupt vector ID
 * @param[in/out] priv - communication channel data for this application.
 *
 * @return - If the IPI interrupt is triggered by its remote, it returns
 *           METAL_IRQ_HANDLED. It returns METAL_IRQ_NOT_HANDLED, if it is
 *           not the interrupt it expected.
 *
 */
static void ipi_irq_handler(s32 vect_id, void *priv)
{
	(void)vect_id;
	struct channel_s *ch = (struct channel_s *)priv;

	if (ch)
	{
		atomic_flag_clear(&ch->remote_nkicked);
	}
}

/**
 * @brief measure_ipi_latency() - Measure latency of IPI
 *        Repeatedly kick IPI to notify the remote and then wait for IPI kick
 *        from RPU and measure the latency. Similarly, measure the latency
 *        from RPU to APU. report the total latency in useconds.
 *        Notes:
 *        - RPU will repeatedly wait for IPI from APU until APU
 *          notifies remote demo has finished by setting the value in the
 *          shared memory.
 *
 * @param[in] ch - channel information, which contains the IPI i/o region,
 *                 shared memory i/o region and the ttc timer i/o region.
 * @return - 0 on success, error code if failure.
 */
static int measure_ipi_latency(struct channel_s *ch)
{
	u32 i;
	u64 tstart,tend,tdiff;
	u64 temp;

	LIBMETAL_IPLA_APU_DEBUG_I("Starting IPI latency task\n");
	/* write to shared memory to indicate demo has started */
	metal_io_write32(ch->shm_io, SHM_DEMO_CNTRL_OFFSET, DEMO_STATUS_START);

	/* Get start counter */
	tstart = GenericTimerRead(GENERIC_TIMER_ID0);
	for (i = 1; i <= ITERATIONS; i++)
	{
		/* Kick IPI to notify the remote */
		InterruptCoreInterSend(CONFIG_IPI_IRQ_NUM, target_cpu_mask);

		wait_for_notified(&ch->remote_nkicked);
	}
	tend = GenericTimerRead(GENERIC_TIMER_ID0);

	/* write to shared memory to indicate demo has finished */
	metal_io_write32(ch->shm_io, SHM_DEMO_CNTRL_OFFSET, DEMO_STATUS_IDLE);
	/* Kick IPI to notify the remote */
	InterruptCoreInterSend(CONFIG_IPI_IRQ_NUM, target_cpu_mask);
	tdiff = tend - tstart;

	/* report avg latencies */
	LIBMETAL_IPLA_APU_DEBUG_I("IPI latency result with %i iterations:\n", ITERATIONS);
	temp = tdiff / ITERATIONS * NS_PER_SEC / GenericTimerFrequecy();
	LIBMETAL_IPLA_APU_DEBUG_I("RPU to APU average latency: %lu ns. \n", temp);
	LIBMETAL_IPLA_APU_DEBUG_I("Finished IPI latency task\n");
	return 0;
}

int ipi_latency_demo(u32 core_mask)
{
	target_cpu_mask = core_mask;
	struct metal_device *dev;
	struct metal_io_region *io;
	struct channel_s ch;
	int ret = 0;

	memset(&ch, 0, sizeof(ch));

	/* Open shared memory device */
	ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &dev);
	if (ret)
	{
		LIBMETAL_IPLA_APU_DEBUG_E("Failed to open device %s.\n", SHM_DEV_NAME);
		goto out;
	}

	/* Get shared memory device IO region */
	io = metal_device_io_region(dev, 0);
	if (!io)
	{
		LIBMETAL_IPLA_APU_DEBUG_E("Failed to map io region for %s.\n", dev->name);
		ret = -ENODEV;
		goto out;
	}
	ch.shm_dev = dev;
	ch.shm_io = io;

	/* Open TTC device */
	ret = metal_device_open(BUS_NAME, TTC_DEV_NAME, &dev);
	if (ret)
	{
		LIBMETAL_IPLA_APU_DEBUG_E("Failed to open device %s.\n", TTC_DEV_NAME);
		goto out;
	}

	/* Get TTC IO region */
	io = metal_device_io_region(dev, 0);
	if (!io)
	{
		LIBMETAL_IPLA_APU_DEBUG_E("Failed to map io region for %s.\n", dev->name);
		ret = -ENODEV;
		goto out;
	}
	ch.ttc_dev = dev;
	ch.ttc_io = io;

	/* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &dev);
	if (ret)
	{
		LIBMETAL_IPLA_APU_DEBUG_E("Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}

	/* Get IPI device IO region */
	io = metal_device_io_region(dev, 0);
	if (!io)
	{
		LIBMETAL_IPLA_APU_DEBUG_E("Failed to map io region for %s.\n", dev->name);
		ret = -ENODEV;
		goto out;
	}
	ch.ipi_dev = dev;
	ch.ipi_io = io;

	/* Get the IPI IRQ from the sdkconfig.h */
    InterruptSetPriority(CONFIG_IPI_IRQ_NUM, CONFIG_IPI_IRQ_NUM_PRIORITY);
    /* Register IPI irq handler */
    InterruptInstall(CONFIG_IPI_IRQ_NUM, ipi_irq_handler, &ch, NULL);

	/* initialize remote_nkicked */
	atomic_init(&ch.remote_nkicked, 1);

	/* Enable IPI interrupt */
    InterruptUmask(CONFIG_IPI_IRQ_NUM);
	
	/* Run atomic operation demo */
	ret = measure_ipi_latency(&ch);

	/* disable IPI interrupt */
    InterruptMask(CONFIG_IPI_IRQ_NUM);

out:
	if (ch.ttc_dev)
		metal_device_close(ch.ttc_dev);
	if (ch.shm_dev)
		metal_device_close(ch.shm_dev);
	if (ch.ipi_dev)
		metal_device_close(ch.ipi_dev);
	return ret;
}
