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
 * shmem_latency_demod.c
 * This is the remote side of the IPI latency measurement demo.
 * This demo does the follwing steps:
 *
 *  1. Get the shared memory device libmetal I/O region.
 *  1. Get the TTC timer device libemtal I/O region.
 *  2. Get IPI device libmetal I/O region and the IPI interrupt vector.
 *  3. Register IPI interrupt handler.
 *  6. When it receives IPI interrupt, the IPI interrupt handler marked the
 *     remote has kicked.
 *  7. Check the shared memory to see if demo is on. If the demo is on,
 *     copy data from the shared memory to local memory, stop the APU to RPU
 *     timer. Reset the RPU to APU TTC counter, copy data from local memory
 *     to shared memory, kick IPI to notify the remote.
 *  8. If the shared memory indicates the demo is off, cleanup resource:
 *     disable IPI interrupt and deregister the IPI interrupt handler.
 */

#include <unistd.h>
#include <metal/atomic.h>
#include <metal/io.h>
#include <metal/device.h>
#include <metal/irq.h>
#include "common.h"
#include "fgeneric_timer.h"
#include "sdkconfig.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fsleep.h"
#include "rpu_running.h"
#include "fdebug.h"
#define LIBMETAL_SHLA_RPU_DEBUG_TAG "LIBMETAL_SHLA_RPU"
#define LIBMETAL_SHLA_RPU_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(LIBMETAL_SHLA_RPU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_SHLA_RPU_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(LIBMETAL_SHLA_RPU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_SHLA_RPU_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(LIBMETAL_SHLA_RPU_DEBUG_TAG, format, ##__VA_ARGS__)

#define TTC_CNT_APU_TO_RPU 2 /* APU to RPU TTC counter ID */
#define TTC_CNT_RPU_TO_APU 3 /* RPU to APU TTC counter ID */

#define TTC_CLK_FREQ_HZ 1000000

/* Shared memory offset */
#define SHM_DEMO_CNTRL_OFFSET 0x0 /* Shared memory for the demo status */
#define SHM_BUFF_OFFSET_RX 0x1000 /* Shared memory RX buffer start offset */
#define SHM_BUFF_OFFSET_TX 0x2000 /* Shared memory TX buffer start offset */

#define DEMO_STATUS_IDLE 0x0
#define DEMO_STATUS_START 0x1 /* Status value to indicate demo start */

#define BUF_SIZE_MAX 4096

struct channel_s
{
	struct metal_io_region *ipi_io; /* IPI metal i/o region */
	struct metal_io_region *shm_io; /* Shared memory metal i/o region */
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
 *
 * @param[in] vect_id - IPI interrupt vector ID
 * @param[in/out] priv - communication channel data for this application.
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
 * @brief measure_shmem_latencyd() - measure shmem latency with libmetal
 *        Loop until APU tells RPU to stop via shared memory.
 *        In loop, wait for interrupt (interrupt handler stops APU to
 *        RPU TTC counter). Then reset count on RPU to APU TTC counter
 *        and kick IPI to notify APU.
 *
 * @param[in] ch - channel information
 * @return - 0 on success, error code if failure.
 */
static int measure_shmem_latencyd(struct channel_s *ch)
{
	void *lbuf = NULL;
	struct msg_hdr_s *msg_hdr;
	int ret = 0;

	/* allocate memory for receiving data */
	lbuf = metal_allocate_memory(BUF_SIZE_MAX);
	if (!lbuf)
	{
		LIBMETAL_SHLA_RPU_DEBUG_E("Failed to allocate memory.\r\n");
		return -1;
	}

	LIBMETAL_SHLA_RPU_DEBUG_I("Starting IPI latency demo:\r\n");
	while (1)
	{
		wait_for_notified(&ch->remote_nkicked);
		if (metal_io_read32(ch->shm_io, SHM_DEMO_CNTRL_OFFSET) == DEMO_STATUS_START)
		{
			/* Read message header from shared memory */
			metal_io_block_read(ch->shm_io, SHM_BUFF_OFFSET_RX,
								lbuf, sizeof(struct msg_hdr_s));
			msg_hdr = (struct msg_hdr_s *)lbuf;

			/* Check if the message header is valid */
			if (msg_hdr->len > (BUF_SIZE_MAX - sizeof(*msg_hdr)))
			{
				LIBMETAL_SHLA_RPU_DEBUG_E("wrong msg: length invalid: %u, %u.\n",
						BUF_SIZE_MAX - sizeof(*msg_hdr),
						msg_hdr->len);
				ret = -EINVAL;
				goto out;
			}
			/* Read message */
			metal_io_block_read(ch->shm_io,
								SHM_BUFF_OFFSET_RX + sizeof(*msg_hdr),
								lbuf + sizeof(*msg_hdr), msg_hdr->len);

			/* Copy the message back to the other end */
			metal_io_block_write(ch->shm_io, SHM_BUFF_OFFSET_TX, msg_hdr, sizeof(*msg_hdr) + msg_hdr->len);

			/* Kick IPI to notify the remote */
			InterruptCoreInterSend(CONFIG_IPI_IRQ_NUM, CONFIG_TARGET_CPU_MASK);
		}
		else
		{
			break;
		}
	}

out:
	metal_free_memory(lbuf);
	return ret;
}

int shmem_latency_demod()
{
	struct channel_s ch;
	struct metal_device *ipi_dev = NULL;
    struct metal_device *shm_dev = NULL;
    struct metal_device *ttc_dev = NULL;
	int ipi_irq;
	int ret = 0;

	memset(&ch, 0, sizeof(ch));

	/* Open shared memory device */
    ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &shm_dev);
    if (ret)
    {
        LIBMETAL_SHLA_RPU_DEBUG_E("Failed to open device %s.\n", SHM_DEV_NAME);
        goto out;
    }

    /* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &ipi_dev);
	if (ret)
	{
		LIBMETAL_SHLA_RPU_DEBUG_E("Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}

    /* Open TTC device */
	ret = metal_device_open(BUS_NAME, TTC_DEV_NAME, &ttc_dev);
	if (ret)
	{
		LIBMETAL_SHLA_RPU_DEBUG_E("Failed to open device %s.\n", TTC_DEV_NAME);
		goto out;
	}
	
	/* Get shared memory device IO region */
	if (!shm_dev)
	{
		ret = -ENODEV;
		goto out;
	}
	ch.shm_io = metal_device_io_region(shm_dev, 0);
	if (!ch.shm_io)
	{
		LIBMETAL_SHLA_RPU_DEBUG_E("Failed to map io region for %s.\n", shm_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Get TTC IO region */
	ch.ttc_io = metal_device_io_region(ttc_dev, 0);
	if (!ch.ttc_io)
	{
		LIBMETAL_SHLA_RPU_DEBUG_E("Failed to map io region for %s.\n", ttc_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Get IPI device IO region */
	ch.ipi_io = metal_device_io_region(ipi_dev, 0);
	if (!ch.ipi_io)
	{
		LIBMETAL_SHLA_RPU_DEBUG_E("Failed to map io region for %s.\n", ipi_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Get the IPI IRQ from the sdkconfig.h */
    InterruptSetPriority(CONFIG_IPI_IRQ_NUM, CONFIG_IPI_IRQ_NUM_PRIORITY);
    /* Register IPI irq handler */
    InterruptInstall(CONFIG_IPI_IRQ_NUM, ipi_irq_handler, &ch, NULL);

	/* initialize remote_nkicked */
	atomic_init(&ch.remote_nkicked, 1);
	/* Enable IPI interrupt */
    InterruptUmask(CONFIG_IPI_IRQ_NUM);

	/* Run atomic operation demo */
	ret = measure_shmem_latencyd(&ch);

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
