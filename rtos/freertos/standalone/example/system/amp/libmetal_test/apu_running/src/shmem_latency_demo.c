/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * @Modify History: 
 *  Ver   Who  			Date   		Changes
 * ----- ------  		-------- 	--------------------------------------
 * 1.0  liushengming    2023/03/16  init
 */
/*****************************************************************************
 * shmem_latency_demo.c
 * This demo demonstrates the shared mem. latency between the APU and RPU.
 * This demo does so via the following steps:
 *
 *  1. Get the shared memory device I/O region.
 *  1. Get the TTC timer device I/O region.
 *  2. Get the IPI device I/O region.
 *  3. Register IPI interrupt handler.
 *  4. Write to shared memory to indicate demo starts
 *  5. Reset the APU to RPU TTC counter, write data to the shared memory, then
 *     kick IPI to notify the remote.
 *  6. When it receives IPI interrupt, the IPI interrupt handler marks the
 *     remote has kicked.
 *  7. Accumulate APU to RPU and RPU to APU counter values.
 *  8. Repeat step 5, 6 and 7 for 1000 times
 *  9. Write shared memory to indicate RPU about demo finishes and kick
 *     IPI to notify.
 * 10. Clean up: disable IPI interrupt, deregister the IPI interrupt handler.
 */
#include <unistd.h>
#include <metal/errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <metal/atomic.h>
#include <metal/cpu.h>
#include <metal/alloc.h>
#include <metal/io.h>
#include <metal/device.h>
#include <metal/irq.h>
#include "common.h"
#include "fgeneric_timer.h"
#include "sdkconfig.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fsleep.h"

#include "fdebug.h"
#define LIBMETAL_SHLA_APU_DEBUG_TAG "LIBMETAL_SHLA_APU"
#define LIBMETAL_SHLA_APU_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(LIBMETAL_SHLA_APU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_SHLA_APU_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(LIBMETAL_SHLA_APU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_SHLA_APU_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(LIBMETAL_SHLA_APU_DEBUG_TAG, format, ##__VA_ARGS__)

#define NS_PER_SEC 1000000000

/* Shared memory offset */
#define SHM_DEMO_CNTRL_OFFSET 0x0 /* Shared memory for the demo status */
#define SHM_BUFF_OFFSET_TX 0x1000 /* Shared memory TX buffer start offset */
#define SHM_BUFF_OFFSET_RX 0x2000 /* Shared memory RX buffer start offset */

#define DEMO_STATUS_IDLE 0x0
#define DEMO_STATUS_START 0x1 /* Status value to indicate demo start */

#define ITERATIONS 1000

#define BUF_SIZE_MAX 4096
#define PKG_SIZE_MIN 16
#define PKG_SIZE_MAX 1024

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

struct msg_hdr_s
{
	uint32_t index;
	uint32_t len;
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
	struct channel_s *ch = (struct channel_s *)priv;
	(void)vect_id;
	if (ch)
	{
		atomic_flag_clear(&ch->remote_nkicked);
	}
}

/**
 * @brief measure_shmem_latency() - Measure latency of using shared memory
 *        and IPI with libmetal.
 *        Repeatedly send a message to RPU and then detect IPI from RPU
 *        and measure the latency. Similarly, measure the latency from RPU
 *        to APU. Each iteration, record this latency and after the loop
 *        has finished, report the total latency in nanseconds.
 *        Notes:
 *        - RPU will repeatedly wait for IPI from APU until APU
 *          notifies remote demo has finished by setting the value in the
 *          shared memory.
 *
 * @param[in] ch - channel information, which contains the IPI i/o region,
 *                 shared memory i/o region and the ttc timer i/o region.
 * @return - 0 on success, error code if failure.
 */
static int measure_shmem_latency(struct channel_s *ch)
{
	uint32_t apu_to_rpu_sum = 0, rpu_to_apu_sum = 0;
	int i;
	size_t s;
	struct msg_hdr_s *msg_hdr;
	void *lbuf;
	int ret;
	u64 tstart,tend,tdiff;

	LIBMETAL_SHLA_APU_DEBUG_I("Starting IPI latency task\n");
	/* allocate memory for receiving data */
	lbuf = metal_allocate_memory(BUF_SIZE_MAX);
	if (!lbuf)
	{
		LIBMETAL_SHLA_APU_DEBUG_E("Failed to allocate memory.\r\n");
		return -1;
	}
	memset(lbuf, 0xA, BUF_SIZE_MAX);

	/* write to shared memory to indicate demo has started */
	metal_io_write32(ch->shm_io, SHM_DEMO_CNTRL_OFFSET, DEMO_STATUS_START);

	for (s = PKG_SIZE_MIN; s <= PKG_SIZE_MAX; s <<= 1)
	{
		/* Get start counter */
		tstart = GenericTimerRead(GENERIC_TIMER_ID0);
		for (i = 1; i <= ITERATIONS; i++)
		{
			/* prepare data */
			msg_hdr = lbuf;
			msg_hdr->index = i;
			msg_hdr->len = s - sizeof(*msg_hdr);
			/* Copy data to the shared memory */
			ret = metal_io_block_write(ch->shm_io, SHM_BUFF_OFFSET_TX, lbuf, s);
			if ((size_t)ret != s)
			{
				LIBMETAL_SHLA_APU_DEBUG_E("Write shm failure: %lu,%lu\n",
						s, (size_t)ret);
				ret = -1;
				goto out;
			}
			/* Kick IPI to notify the remote */
			InterruptCoreInterSend(CONFIG_IPI_IRQ_NUM, target_cpu_mask);
			/* irq handler stops timer for rpu->apu irq */
			wait_for_notified(&ch->remote_nkicked);
			/* Read message */
			metal_io_block_read(ch->shm_io, SHM_BUFF_OFFSET_RX, lbuf, s);
			msg_hdr = lbuf;
			if (msg_hdr->len != (s - sizeof(*msg_hdr)))
			{
				LIBMETAL_SHLA_APU_DEBUG_E("Read shm failure: %lu,%lu\n",
						s, msg_hdr->len + sizeof(*msg_hdr));
				ret = -1;
				goto out;
			}
		}
		/* Stop RPU to APU TTC counter */
		tend = GenericTimerRead(GENERIC_TIMER_ID0);
		tdiff = tend - tstart;
		/* report avg latencies */
		LIBMETAL_SHLA_APU_DEBUG_I("package size %lu latency result:\n", s);
		LIBMETAL_SHLA_APU_DEBUG_I("APU to RPU and return APU average latency: %lu ns \n",
				tdiff / ITERATIONS * NS_PER_SEC / GenericTimerFrequecy());
	}

	/* write to shared memory to indicate demo has finished */
	metal_io_write32(ch->shm_io, SHM_DEMO_CNTRL_OFFSET, 0);
	/* Kick IPI to notify the remote */
	InterruptCoreInterSend(CONFIG_IPI_IRQ_NUM, target_cpu_mask);

	LIBMETAL_SHLA_APU_DEBUG_I("Finished shared memory latency task\n");

out:
	metal_free_memory(lbuf);
	return 0;
}

int shmem_latency_demo(u32 core_mask)
{
	target_cpu_mask = core_mask;
	struct metal_device *dev;
	struct metal_io_region *io;
	struct channel_s ch;
	int ipi_irq;
	int ret = 0;

	memset(&ch, 0, sizeof(ch));

	/* Open shared memory device */
	ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &dev);
	if (ret)
	{
		LIBMETAL_SHLA_APU_DEBUG_E("Failed to open device %s.\n", SHM_DEV_NAME);
		goto out;
	}

	/* Get shared memory device IO region */
	io = metal_device_io_region(dev, 0);
	if (!io)
	{
		LIBMETAL_SHLA_APU_DEBUG_E("Failed to map io region for %s.\n", dev->name);
		ret = -ENODEV;
		goto out;
	}
	ch.shm_dev = dev;
	ch.shm_io = io;

	/* Open TTC device */
	ret = metal_device_open(BUS_NAME, TTC_DEV_NAME, &dev);
	if (ret)
	{
		LIBMETAL_SHLA_APU_DEBUG_E("Failed to open device %s.\n", TTC_DEV_NAME);
		goto out;
	}

	/* Get TTC IO region */
	io = metal_device_io_region(dev, 0);
	if (!io)
	{
		LIBMETAL_SHLA_APU_DEBUG_E("Failed to map io region for %s.\n", dev->name);
		ret = -ENODEV;
		goto out;
	}
	ch.ttc_dev = dev;
	ch.ttc_io = io;

	/* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &dev);
	if (ret)
	{
		LIBMETAL_SHLA_APU_DEBUG_E("Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}

	/* Get IPI device IO region */
	io = metal_device_io_region(dev, 0);
	if (!io)
	{
		LIBMETAL_SHLA_APU_DEBUG_E("Failed to map io region for %s.\n", dev->name);
		ret = -ENODEV;
		goto out;
	}
	ch.ipi_dev = dev;
	ch.ipi_io = io;

	
	/* Get the IPI IRQ from the sdkconfig.h */
    InterruptSetPriority(CONFIG_IPI_IRQ_NUM, CONFIG_IPI_IRQ_NUM_PRIORITY);
	
	/* initialize remote_nkicked */
	atomic_init(&ch.remote_nkicked, 1);

	/* Register IPI irq handler */

    InterruptInstall(CONFIG_IPI_IRQ_NUM, ipi_irq_handler, &ch, NULL);
	/* Enable IPI interrupt */
    InterruptUmask(CONFIG_IPI_IRQ_NUM);
	
	/* Run atomic operation demo */
	ret = measure_shmem_latency(&ch);

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
