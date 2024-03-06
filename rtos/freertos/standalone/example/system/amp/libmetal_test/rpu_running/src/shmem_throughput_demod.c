/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * @Modify History: 
 *  Ver   Who  		Date   		Changes
 * ----- ------  	-------- 	--------------------------------------
 * 1.0 liushengming 2023/03/17	init
 */
/*****************************************************************************
 * shmem_throughput_demo_task.c
 * This is the remote side of the shared memory throughput demo.
 * This demo does the following steps:
 *
 *  1. Get the shared memory device libmetal I/O region.
 *  1. Get the TTC timer device libemtal I/O region.
 *  2. Get IPI device libmetal I/O region and the IPI interrupt vector.
 *  3. Register IPI interrupt handler.
 *  6. Download throughput measurement:
 *     Start TTC RPU counter, wait for IPI kick, check if data is available,
 *     if yes, read as much data as possible from shared memory. It will
 *     iterates untill 1000 packages have been received, stop TTC RPU counter
 *     and kick IPI to notify the remote. Repeat for different package size.
 *  7. Upload throughput measurement:
 *     Start TTC RPU counter, write data to shared memory and kick IPI to
 *     notify remote. It will iterate for 1000 times, stop TTC RPU counter.
 *     wait for APU IPI kick to know APU has finished receiving packages.
 *     Kick IPI to notify it TTC RPU conter value is ready to read.
 *     Repeat for different package size.
 *  8. Cleanup resource:
 *     disable IPI interrupt and deregister the IPI interrupt handler.
 *
 * Here is the Shared memory structure of this demo:
 * |0x0   - 0x03         | number of APU to RPU buffers available to RPU |
 * |0x04  - 0x1FFFFF     | address array for shared buffers from APU to RPU |
 * |0x200000 - 0x200004  | number of RPU to APU buffers available to APU |
 * |0x200004 - 0x3FFFFF  | address array for shared buffers from RPU to APU |
 * |0x400000 - 0x7FFFFF  | APU to RPU buffers |
 * |0x800000 - 0xAFFFFF  | RPU to APU buffers |
 */

#include <unistd.h>
#include <metal/atomic.h>
#include <metal/io.h>
#include <metal/device.h>
#include <metal/irq.h>
#include <metal/alloc.h>
#include "common.h"
#include "fgeneric_timer.h"
#include "sdkconfig.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fsleep.h"
#include "rpu_running.h"
#include "fdebug.h"
#define LIBMETAL_SHTH_RPU_DEBUG_TAG "LIBMETAL_SHTH_RPU"
#define LIBMETAL_SHTH_RPU_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(LIBMETAL_SHTH_RPU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_SHTH_RPU_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(LIBMETAL_SHTH_RPU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_SHTH_RPU_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(LIBMETAL_SHTH_RPU_DEBUG_TAG, format, ##__VA_ARGS__)

#define TTC_CNT_APU_TO_RPU 2 /* APU to RPU TTC counter ID */
#define TTC_CNT_RPU_TO_APU 3 /* RPU to APU TTC counter ID */

/* Shared memory offsets */
#define SHM_DESC_OFFSET_RX 0x0
#define SHM_BUFF_OFFSET_RX 0x400000
#define SHM_DESC_OFFSET_TX 0x200000
#define SHM_BUFF_OFFSET_TX 0x800000

/* Shared memory descriptors offset */
#define SHM_DESC_AVAIL_OFFSET 0x00
#define SHM_DESC_ADDR_ARRAY_OFFSET 0x04

#define BUF_SIZE_MAX 1024
#define PKG_SIZE_MAX 1024
#define PKG_SIZE_MIN 16
#define TOTAL_DATA_SIZE (BUF_SIZE_MAX * PKG_SIZE_MAX)

#define MB (1024 * 1024) /* Mega Bytes */
#define KB (1024) 

struct channel_s
{
	struct metal_io_region *ipi_io; /* IPI metal i/o region */
	struct metal_io_region *shm_io; /* Shared memory metal i/o region */
	struct metal_io_region *ttc_io; /* TTC metal i/o region */
	u32 ipi_mask;				/* RPU IPI mask */
	atomic_int remote_nkicked;		/* 0 - kicked from remote */
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
 * @brief measure_shmem_throughputd() - measure shmem throughpput with libmetal
 *        - Download throughput measurement:
 *          Start TTC RPU counter, wait for IPI kick, check if data is
 *          available, if yes, read as much data as possible from shared
 *          memory. It will iterates untill 1000 packages have been received,
 *          stop TTC RPU counter and kick IPI to notify the remote. Repeat
 *          for different package size.
 *        - Upload throughput measurement:
 *          Start TTC RPU counter, write data to shared memory and kick IPI
 *          to notify remote. It will iterate for 1000 times, stop TTC RPU
 *          counter.Wait for APU IPI kick to know APU has received all the
 *          packages. Kick IPI to notify it TTC RPU conter value is ready to
 *          read. Repeat for different package size.
 *
 * @param[in] ch - channel information
 * @return - 0 on success, error code if failure.
 */
static int measure_shmem_throughputd(struct channel_s *ch)
{
	void *lbuf = NULL;
	int ret = 0;
	u64 s, i;
	u64 tstart,tend,tdiff;
	u32 rx_count, rx_avail, tx_count, iterations;
	unsigned long tx_avail_offset, rx_avail_offset;
	unsigned long tx_addr_offset, rx_addr_offset;
	unsigned long tx_data_offset, rx_data_offset;
	u32 buf_phy_addr_32;
	
	u32 *rpu_tx_count = NULL;
	u32 *rpu_rx_count = NULL;

	/* allocate memory for receiving data */
	lbuf = metal_allocate_memory(BUF_SIZE_MAX);
	if (!lbuf)
	{
		LIBMETAL_SHTH_RPU_DEBUG_E("Failed to allocate memory.\r\n");
		return -1;
	}
	memset(lbuf, 0xA, BUF_SIZE_MAX);

	/* allocate memory for saving counter values */
	for (s = PKG_SIZE_MIN, i = 0; s <= PKG_SIZE_MAX; s <<= 1, i++)
		;
	rpu_tx_count = metal_allocate_memory(i * sizeof(u32));
	rpu_rx_count = metal_allocate_memory(i * sizeof(u32));

	if (!rpu_tx_count || !rpu_rx_count)
	{
		LIBMETAL_SHTH_RPU_DEBUG_E("Failed to allocate memory.\r\n");
		ret = -ENOMEM;
		goto out;
	}

	/* Clear shared memory */
	metal_io_block_set(ch->shm_io, 0, 0, metal_io_region_size(ch->shm_io));

	LIBMETAL_SHTH_RPU_DEBUG_I("Starting shared mem throughput demo\n");

	/* for each data size, measure block receive throughput */
	for (s = PKG_SIZE_MIN, i = 0; s <= PKG_SIZE_MAX; s <<= 1, i++)
	{
		rx_count = 0;
		iterations = TOTAL_DATA_SIZE / s;
		/* Set rx buffer address offset */
		rx_avail_offset = SHM_DESC_OFFSET_RX + SHM_DESC_AVAIL_OFFSET;
		rx_addr_offset = SHM_DESC_OFFSET_RX + SHM_DESC_ADDR_ARRAY_OFFSET;
		rx_data_offset = SHM_DESC_OFFSET_RX + SHM_BUFF_OFFSET_RX;
		wait_for_notified(&ch->remote_nkicked);
		
		/* Get start counter */
		tstart = GenericTimerRead(GENERIC_TIMER_ID0);
		while (1)
		{
			rx_avail = metal_io_read32(ch->shm_io, rx_avail_offset);
			while (rx_count != rx_avail)
			{
				/* Get the buffer location from the shared memory rx address array.*/
				buf_phy_addr_32 = metal_io_read32(ch->shm_io, rx_addr_offset);
				rx_data_offset = metal_io_phys_to_offset( ch->shm_io, (metal_phys_addr_t)buf_phy_addr_32);
				if (rx_data_offset == METAL_BAD_OFFSET)
				{
					LIBMETAL_SHTH_RPU_DEBUG_E( "[%u]failed to get rx offset: 0x%x, 0x%lx.\n",
												rx_count, buf_phy_addr_32,
												metal_io_phys(ch->shm_io, rx_addr_offset));
					ret = -EINVAL;
					goto out;
				}
				rx_addr_offset += sizeof(buf_phy_addr_32);
				/* Read data from shared memory */
				metal_io_block_read(ch->shm_io, rx_data_offset, lbuf, s);
				rx_count++;
			}
			if (rx_count < iterations)
				/* Need to wait for more data */
				wait_for_notified(&ch->remote_nkicked);
			else
				break;
		}
		/* Stop RPU TTC counter */
		tend = GenericTimerRead(GENERIC_TIMER_ID0);
		rpu_rx_count[i] = tend - tstart;
		
		/* Kick IPI to notify RPU TTC counter value is ready */
		InterruptCoreInterSend(CONFIG_IPI_IRQ_NUM, CONFIG_TARGET_CPU_MASK);
		/* Clear remote kicked flag -- 0 is kicked */
		atomic_init(&ch->remote_nkicked, 1);
	}

	/* for each data size, measure send throughput */
	for (s = PKG_SIZE_MIN, i = 0; s <= PKG_SIZE_MAX; s <<= 1, i++)
	{
		tx_count = 0;
		iterations = TOTAL_DATA_SIZE / s;

		/* Set tx buffer address offset */
		tx_avail_offset = SHM_DESC_OFFSET_TX + SHM_DESC_AVAIL_OFFSET;
		tx_addr_offset = SHM_DESC_OFFSET_TX + SHM_DESC_ADDR_ARRAY_OFFSET;
		tx_data_offset = SHM_DESC_OFFSET_TX + SHM_BUFF_OFFSET_TX;
		/* Wait for APU to signal it is ready for the measurement */
		wait_for_notified(&ch->remote_nkicked);
		/* Get start counter */
		tstart = GenericTimerRead(GENERIC_TIMER_ID0);
		while (tx_count < iterations)
		{
			/* Write data to the shared memory*/
			metal_io_block_write(ch->shm_io, tx_data_offset, lbuf, s);

			/* Write to the address array to tell the other end
			 * the buffer address.
			 */
			buf_phy_addr_32 = (u32)metal_io_phys(ch->shm_io, tx_data_offset);
			metal_io_write32(ch->shm_io, tx_addr_offset, buf_phy_addr_32);
			tx_data_offset += s;
			tx_addr_offset += sizeof(buf_phy_addr_32);

			/* Increase number of available buffers */
			tx_count++;
			metal_io_write32(ch->shm_io, tx_avail_offset, tx_count);
			/* Kick IPI to notify remote data is ready in the shared memory */
			InterruptCoreInterSend(CONFIG_IPI_IRQ_NUM, CONFIG_TARGET_CPU_MASK);
			
		}
		// LIBMETAL_SHTH_RPU_DEBUG_I("iterations is %d , tx_count is %d \r\n",iterations,tx_count) ;
		/* Stop counter */
		tend = GenericTimerRead(GENERIC_TIMER_ID0);
		/* Wait for IPI kick to know when the remote is ready
		 * to read the TTC counter value */
		wait_for_notified(&ch->remote_nkicked);
		/* Kick IPI to notify RPU TTC counter value is ready */
		InterruptCoreInterSend(CONFIG_IPI_IRQ_NUM, CONFIG_TARGET_CPU_MASK);
		rpu_tx_count[i] = tend - tstart;
	}
	fsleep_millisec(200);/* wait apu printf over. */
	/* Print the measurement result */
	for (s = PKG_SIZE_MIN, i = 0; s <= PKG_SIZE_MAX; s <<= 1, i++)
	{
		LIBMETAL_SHTH_RPU_DEBUG_I("Shared memory throughput of pkg size %lu : \n", s);

		if ( (s * iterations * GenericTimerFrequecy() / rpu_tx_count[i] / MB) <= 2 )
		{
			LIBMETAL_SHTH_RPU_DEBUG_I("    RPU send: 0x%llx, %llu KB/s \n", rpu_tx_count[i], s * iterations * GenericTimerFrequecy() / rpu_tx_count[i] / KB);
		}
		else
		{
			LIBMETAL_SHTH_RPU_DEBUG_I("    RPU send: 0x%llx, %llu MB/s \n", rpu_tx_count[i], s * iterations * GenericTimerFrequecy() / rpu_tx_count[i] / MB);
		}

		if ( (s * iterations * GenericTimerFrequecy() / rpu_rx_count[i] / MB) <= 2)
		{
			LIBMETAL_SHTH_RPU_DEBUG_I("    RPU receive: 0x%llx, %llu KB/s \n", rpu_rx_count[i], s * iterations * GenericTimerFrequecy() / rpu_rx_count[i] / KB);
		}
		else
		{
			LIBMETAL_SHTH_RPU_DEBUG_I("    RPU receive: 0x%llx, %llu MB/s \n", rpu_rx_count[i], s * iterations * GenericTimerFrequecy() / rpu_rx_count[i] / MB);
		}
	}

out:
	if (lbuf)
		metal_free_memory(lbuf);
	if (rpu_tx_count)
		metal_free_memory(rpu_tx_count);
	if (rpu_rx_count)
		metal_free_memory(rpu_rx_count);
	return ret;
}

int shmem_throughput_demod()
{
	struct channel_s ch;
	struct metal_device *ipi_dev = NULL;
    struct metal_device *shm_dev = NULL;
    struct metal_device *ttc_dev = NULL;
	int ipi_irq;
	int ret = 0;

	/* Open shared memory device */
    ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &shm_dev);
    if (ret)
    {
        LIBMETAL_SHTH_RPU_DEBUG_E("Failed to open device %s.\n", SHM_DEV_NAME);
        goto out;
    }

    /* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &ipi_dev);
	if (ret)
	{
		LIBMETAL_SHTH_RPU_DEBUG_E("Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}

    /* Open TTC device */
	ret = metal_device_open(BUS_NAME, TTC_DEV_NAME, &ttc_dev);
	if (ret)
	{
		LIBMETAL_SHTH_RPU_DEBUG_E("Failed to open device %s.\n", TTC_DEV_NAME);
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
		LIBMETAL_SHTH_RPU_DEBUG_E("Failed to map io region for %s.\n", shm_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Get TTC IO region */
	ch.ttc_io = metal_device_io_region(ttc_dev, 0);
	if (!ch.ttc_io)
	{
		LIBMETAL_SHTH_RPU_DEBUG_E("Failed to map io region for %s.\n", ttc_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Get IPI device IO region */
	ch.ipi_io = metal_device_io_region(ipi_dev, 0);
	if (!ch.ipi_io)
	{
		LIBMETAL_SHTH_RPU_DEBUG_E("Failed to map io region for %s.\n", ipi_dev->name);
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
	ret = measure_shmem_throughputd(&ch);

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
