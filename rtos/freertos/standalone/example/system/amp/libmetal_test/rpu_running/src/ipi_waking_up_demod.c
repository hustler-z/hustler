/*
 * @Copyright : (C) 2022 Phytium Information Technology, Inc. 
 * All Rights Reserved.
 *  
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it  
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
 * either version 1.0 of the License, or (at your option) any later version. 
 *  
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details. 
 *  
 * 
 * @FilePath: ipi_waking_up_demod.c
 * @Date: 2023-03-21 14:52:29
 * @LastEditTime: 2023-03-21 14:52:29
 * @Description:  This file is for waiting APU waking up RPU,used to synchronize APU and RPU demo
 * 
 * @Modify History: 
 *  Ver     Who         Date        Changes
 * -----    ------      --------    --------------------------------------
 * 1.0  liushengming    2023/0321   init
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
#include "rpu_running.h"
#include "fdebug.h"
#define LIBMETAL_IPWU_RPU_DEBUG_TAG "LIBMETAL_RPU_WAKING_UP"
#define LIBMETAL_IPWU_RPU_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(LIBMETAL_IPWU_RPU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_IPWU_RPU_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(LIBMETAL_IPWU_RPU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_IPWU_RPU_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(LIBMETAL_IPWU_RPU_DEBUG_TAG, format, ##__VA_ARGS__)

#define IPI_APU_DEMO_CNTRL_OFFSET 0x500
#define IPI_RPU_DEMO_CNTRL_OFFSET 0x504

#define DEMO_APU_STATUS_IDLE 0x11
#define DEMO_APU_STATUS_START 0x22 /* Status value to indicate demo start */
#define DEMO_RPU_STATUS_IDLE 0x33
#define DEMO_RPU_STATUS_START 0x44 /* Status value to indicate demo start */

#define DEMO_SYNC_IRQ_NUM 0x0
#define DEMO_SYNC_IRQ_NUM_PRIORITY 0x0

struct channel_s
{
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
inline static void measure_ipi_waking_up(struct channel_s *ch)
{
	metal_io_write32(ch->ipi_io,IPI_RPU_DEMO_CNTRL_OFFSET,DEMO_RPU_STATUS_IDLE);
	while (!(metal_io_read32(ch->ipi_io, IPI_APU_DEMO_CNTRL_OFFSET) == DEMO_APU_STATUS_IDLE));
	
	/* Kick IPI to notify the remote */
	InterruptCoreInterSend(DEMO_SYNC_IRQ_NUM, CONFIG_TARGET_CPU_MASK);
	/* init ok,waiting apu interrupt*/
	wait_for_notified(&ch->remote_nkicked);
	metal_io_write32(ch->ipi_io,IPI_RPU_DEMO_CNTRL_OFFSET,DEMO_RPU_STATUS_START);/* rpu next */
}

int ipi_waking_up_demod()
{
	struct channel_s ch;
    struct metal_device *ipi_dev = NULL;
	int ipi_irq;
	int ret = 0;

    /* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &ipi_dev);
	if (ret)
	{
		LIBMETAL_IPWU_RPU_DEBUG_E("Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}

	memset(&ch, 0, sizeof(ch));

	/* Get IPI device IO region */
    if (!ipi_dev)
	{
		ret = -ENODEV;
		goto out;
	}
	ch.ipi_io = metal_device_io_region(ipi_dev, 0);
	if (!ch.ipi_io)
	{
		LIBMETAL_IPWU_RPU_DEBUG_E("Failed to map io region for %s.\n", ipi_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Get the IPI IRQ from the opened IPI device */
    InterruptSetPriority(DEMO_SYNC_IRQ_NUM, DEMO_SYNC_IRQ_NUM_PRIORITY);
    InterruptInstall(DEMO_SYNC_IRQ_NUM, ipi_irq_handler, &ch, NULL);

	/* initialize remote_nkicked */
	atomic_init(&ch.remote_nkicked, 1);
	/* Enable IPI interrupt */
    InterruptUmask(DEMO_SYNC_IRQ_NUM);

	/* Run ipi waking up demo */
	measure_ipi_waking_up(&ch);

    /* disable IPI interrupt */
    InterruptMask(DEMO_SYNC_IRQ_NUM);

out:
    if (ipi_dev)
    {
        metal_device_close(ipi_dev);
    }
	return ret;
}