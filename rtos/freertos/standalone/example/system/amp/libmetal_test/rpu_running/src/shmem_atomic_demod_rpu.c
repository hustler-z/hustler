/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Modify History:
 *  Ver   Who           Date         Changes
 * ----- ------         --------    --------------------------------------
 * 1.0  huanghe         2022-02-17  init
 * 1.1  liushengming    2023/03/09  modified function of atomic_shmem_demod
 */

/*****************************************************************************
 * shmem_atomic_demod_rpu.c - Shared memory atomic operation demo
 * This task will:
 *  1. Get the shared memory device I/O region.
 *  2. Get the IPI device I/O region.
 *  3. Register IPI interrupt handler.
 *  4. Wait for the APU to kick IPI to start the demo
 *  5. Once notification is received, start atomic add by
 *     1 for 5000 times over the shared memory
 *  6. Trigger IPI to notify the remote it has finished calculation.
 *  7. Clean up: Disable IPI interrupt, deregister the IPI interrupt handler.
 */
#include <metal/shmem.h>
#include <metal/atomic.h>
#include <metal/device.h>
#include <metal/io.h>
#include <sys/time.h>
#include <stdio.h>
#include "common.h"
#include "sys_init.h"
#include "finterrupt.h"
#include "sdkconfig.h"
#include "fsleep.h"
#include "rpu_running.h"
#include "fdebug.h"
#define LIBMETAL_ATSH_RPU_DEBUG_TAG "LIBMETAL_ATSH_RPU"
#define LIBMETAL_ATSH_RPU_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(LIBMETAL_ATSH_RPU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_ATSH_RPU_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(LIBMETAL_ATSH_RPU_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_ATSH_RPU_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(LIBMETAL_ATSH_RPU_DEBUG_TAG, format, ##__VA_ARGS__)

#define ATOMIC_INT_OFFSET 0x0 /* shared memory offset for atomic operation */
#define ITERATIONS 5000

static atomic_int remote_nkicked; /* is remote kicked, 0 - kicked, 1 - not-kicked */

static void ipi_irq_handler(s32 vect_id, void *priv)
{
    (void)vect_id;
    (void)priv;
    atomic_flag_clear(&remote_nkicked);
}

/**
 * @brief   atomic_add_shmemd() - Shared memory atomic operation demo
 *          This task will:
 *          * Wait for the remote to write to shared memory.
 *          * Once it receives the notification via polling, start atomic add by
 *            1 for 5000 times to first 32 bits of memory in the shared memory
 *            which is pointed to by shm_io.
 *          * Write to shared mem to notify the remote once it finishes
 *            calculation.
 *
 * @param[in] ipi_io - IPI metal i/o region
 * @param[in] shm_io - shared memory metal i/o region
 * @return - If setup failed, return the corresponding error number. Otherwise
 *          return 0 on success.
 */
int atomic_add_shmemd(struct metal_io_region *ipi_io,
                      struct metal_io_region *shm_io)
{
    atomic_int *shm_int;
    int i;

    LIBMETAL_ATSH_RPU_DEBUG_I("Starting atomic add on shared memory demo.\n");
    shm_int = (atomic_int *)metal_io_virt(shm_io, ATOMIC_INT_OFFSET);

    /* Wait for notification from the remote to start the demo */
    wait_for_notified(&remote_nkicked);

    /* Do atomic add over the shared memory */
    for (i = 0; i < ITERATIONS; i++)
    {
        //(*shm_int)++;
        atomic_fetch_add(shm_int, 1);
    }

    /* Write to IPI trigger register to notify the remote it has finished
     * the atomic operation. */

    InterruptCoreInterSend(CONFIG_IPI_IRQ_NUM, CONFIG_TARGET_CPU_MASK);

    LIBMETAL_ATSH_RPU_DEBUG_I("Shared memory with atomics test finished.\n");
    return 0;
}

int atomic_shmem_demod(void)
{
    struct metal_device *ipi_dev = NULL;
    struct metal_device *shm_dev = NULL;
    struct metal_io_region *ipi_io = NULL, *shm_io = NULL;
    int ipi_irq;
    int ret = 0;

    LIBMETAL_ATSH_RPU_DEBUG_I("atomic operation over shared memory.");

    /* Open shared memory device */
    ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &shm_dev);
    if (ret)
    {
        LIBMETAL_ATSH_RPU_DEBUG_E("Failed to open device %s.\n", SHM_DEV_NAME);
        goto out;
    }

    /* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &ipi_dev);
	if (ret)
	{
		LIBMETAL_ATSH_RPU_DEBUG_E("Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}
    
    /* Get shared memory device IO region */
    if (!shm_dev)
    {
        ret = -ENODEV;
        goto out;
    }
    shm_io = metal_device_io_region(shm_dev, 0);
    if (!shm_io)
    {
        LIBMETAL_ATSH_RPU_DEBUG_E("Failed to map io region for %s.\n", shm_dev->name);
        ret = -ENODEV;
        goto out;
    }

    /* Get IPI device IO region */
	if (!ipi_dev)
	{
		ret = -ENODEV;
		goto out;
	}
	ipi_io = metal_device_io_region(ipi_dev, 0);
	if (!ipi_io)
	{
		LIBMETAL_ATSH_RPU_DEBUG_E("Failed to map io region for %s.\n", ipi_dev->name);
		ret = -ENODEV;
		goto out;
	}
    
    /* Get the IPI IRQ from the opened IPI device */
    InterruptSetPriority(CONFIG_IPI_IRQ_NUM, CONFIG_IPI_IRQ_NUM_PRIORITY);
    InterruptInstall(CONFIG_IPI_IRQ_NUM, ipi_irq_handler, NULL, NULL);
    /* initialize remote_nkicked */
    atomic_init(&remote_nkicked, 1);
    /* Enable IPI interrupt */
    InterruptUmask(CONFIG_IPI_IRQ_NUM);

    /* Run atomic operation demo */
    ret = atomic_add_shmemd(ipi_io, shm_io);

    /* disable IPI interrupt */
    InterruptMask(CONFIG_IPI_IRQ_NUM);
    /* unregister IPI irq handler */
out:
    if (shm_dev)
    {
        metal_device_close(shm_dev);
    }
    if (ipi_dev)
    {
        metal_device_close(ipi_dev);
    }
    return ret;
}
