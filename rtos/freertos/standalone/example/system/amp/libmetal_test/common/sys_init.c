/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: sys_init.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:56:09
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#include "sys_init.h"
#include "sdkconfig.h"
#include "fcache.h"
#include "finterrupt.h"
#include "ftypes.h"
#include "fparameters.h"
#include "fcpu_info.h"
#include "fmmu.h"

#include "common.h"
#define METAL_INTERNAL
#include <metal/errno.h>
#include <string.h>
#include <metal/io.h>
#include <metal/alloc.h>
#include <metal/device.h>

#include "fdebug.h"
#define LIBMETAL_SYS_DEBUG_TAG "LIBMETAL_SYS"
#define LIBMETAL_SYS_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(LIBMETAL_SYS_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_SYS_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(LIBMETAL_SYS_DEBUG_TAG, format, ##__VA_ARGS__)
#define LIBMETAL_SYS_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(LIBMETAL_SYS_DEBUG_TAG, format, ##__VA_ARGS__)

#define IPI_IRQ_VECT_ID 0

#define SHM_MEM_REGIONS_SIZE 0x2000000
#define IPI_MEM_REGIONS_SIZE 0x1000
#define TTC_MEM_REGIONS_SIZE 0x1000

#define SHM_BASE_ADDR CONFIG_SHM_BASE_ADDR
#define TTC0_BASE_ADDR SHM_BASE_ADDR+SHM_MEM_REGIONS_SIZE
#define IPI_BASE_ADDR SHM_BASE_ADDR+TTC_MEM_REGIONS_SIZE

/* Default generic I/O region page shift */
/* Each I/O region can contain multiple pages.
 * In baremetal system, the memory mapping is flat, there is no
 * virtual memory.
 * We can assume there is only one page in the whole baremetal system.
 */
#define DEFAULT_PAGE_SHIFT (-1UL)
#define DEFAULT_PAGE_MASK (-1UL)

const metal_phys_addr_t metal_phys[] =
{
    SHM_BASE_ADDR, /**< shared memory base address */
    IPI_BASE_ADDR,	/**< shared memory base address */
	TTC0_BASE_ADDR, /**< base TTC0 address */
};

static struct metal_device metal_dev_table[] =
{
    {
        /* Shared memory management device */
        .name = SHM_DEV_NAME,
        .bus = NULL,
        .num_regions = 1,
        .regions = {
            {
                .virt = (void *)SHM_BASE_ADDR,
                .physmap = &metal_phys[0],
                .size = SHM_MEM_REGIONS_SIZE,
                .page_shift = DEFAULT_PAGE_SHIFT,
                .page_mask = DEFAULT_PAGE_MASK,
#ifdef __aarch64__
                .mem_flags = MT_NORMAL | MT_P_RW_U_RW,
#else
                .mem_flags = MT_NORMAL | MT_P_RW_U_RW,
#endif
                .ops = {NULL},
            }
        },
        .node = {NULL},
        .irq_num = 0,
        .irq_info = NULL,
    },
    {
        /* Shared memory management device */
        .name = IPI_DEV_NAME,
        .bus = NULL,
        .num_regions = 1,
        .regions = {
            {
                .virt = (void *)IPI_BASE_ADDR,
                .physmap = &metal_phys[1],
                .size = IPI_MEM_REGIONS_SIZE,
                .page_shift = DEFAULT_PAGE_SHIFT,
                .page_mask = DEFAULT_PAGE_MASK,
#ifdef __aarch64__
                .mem_flags = MT_NORMAL | MT_P_RW_U_RW,
#else
                .mem_flags = MT_NORMAL | MT_P_RW_U_RW,
#endif
                .ops = {NULL},
            }
        },
        .node = {NULL},
        .irq_num = 1,
        .irq_info = (void *)IPI_IRQ_VECT_ID,
    },
    {
        /* Shared memory management device */
        .name = TTC_DEV_NAME,
        .bus = NULL,
        .num_regions = 1,
        .regions = {
            {
                .virt = (void *)TTC0_BASE_ADDR,
                .physmap = &metal_phys[2],
                .size = TTC_MEM_REGIONS_SIZE,
                .page_shift = DEFAULT_PAGE_SHIFT,
                .page_mask = DEFAULT_PAGE_MASK,
#ifdef __aarch64__
                .mem_flags = MT_NORMAL | MT_P_RW_U_RW,
#else
                .mem_flags = MT_NORMAL | MT_P_RW_U_RW,
#endif
                .ops = {NULL},
            }
        },
        .node = {NULL},
        .irq_num = 1,
        .irq_info = NULL,
    }
};

/**
 * Extern global variables
 */
struct metal_device *ipi_dev = NULL;
struct metal_device *shm_dev = NULL;
struct metal_device *ttc_dev = NULL;

/**
 * @brief EnableCaches() - Enable caches
 */

void EnableCaches()
{
    FCacheICacheEnable();
    FCacheDCacheEnable();
}

/**
 * @brief DisableCaches() - Disable caches
 */
void DisableCaches()
{
    FCacheDCacheDisable();
    FCacheICacheDisable();
}

/**
 * @brief open_metal_devices() - Open registered libmetal devices.
 *        This function opens all the registered libmetal devices.
 *
 * @return 0 - succeeded, non-zero for failures.
 */
s32 FOpenMetalDevice(void)
{
    int ret;

    /* Open shared memory device */
    ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &shm_dev);
    if (ret)
    {
        goto out;
    }

    /* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &ipi_dev);
	if (ret)
	{
		goto out;
	}

	/* Open TTC device */
	ret = metal_device_open(BUS_NAME, TTC_DEV_NAME, &ttc_dev);
	if (ret)
	{
		goto out;
	}
out:
    return ret;
}

/**
 * @brief FCloseMetalDevice() - close libmetal devices
 *        This function closes all the libmetal devices which have
 *        been opened.
 *
 */
void FCloseMetalDevice(void)
{
    /* Close shared memory device */
    if (shm_dev)
    {
        metal_device_close(shm_dev);
    }

    /* Close IPI device */
	if (ipi_dev)
    {
        metal_device_close(ipi_dev);
    }

	/* Close TTC device */
	if (ttc_dev)
    {
        metal_device_close(ttc_dev);
    }
}

/**
 * @brief platform_register_metal_device() - Statically Register libmetal
 *        devices.
 *        This function registers the IPI, shared memory and
 *        TTC devices to the libmetal generic bus.
 *        Libmetal uses bus structure to group the devices. Before you can
 *        access the device with libmetal device operation, you will need to
 *        register the device to a libmetal supported bus.
 *        For non-Linux system, libmetal only supports "generic" bus, which is
 *        used to manage the memory mapped devices.
 *
 * @return 0 - succeeded, non-zero for failures.
 */

int FRegisterMetalDevice(void)
{
    u32 i;
    int ret;
    struct metal_device *dev;
    LIBMETAL_SYS_DEBUG_I("RegisterMetalDevice nums:%d.\r\n",sizeof(metal_dev_table) / sizeof(struct metal_device));
    for ( i = 0; i < sizeof(metal_dev_table) / sizeof(struct metal_device);i++)
    {
        dev = &metal_dev_table[i];
        ret = metal_register_generic_device(dev);
        if (ret)
        {
            return ret;
        }
    }
    return 0;
}

int FLibmetalSysInit()
{
    struct metal_init_params metal_param = METAL_INIT_DEFAULTS;
    int ret;

    /* Initialize libmetal environment */
    ret = metal_init(&metal_param);
    if (ret)
    {
        LIBMETAL_SYS_DEBUG_E("%s: failed to register devices: %d\n", __func__, ret);
        return ret;
    }
    return 0;
}

/**
 * @brief FLibmetalSysCleanup() - system cleanup
 *        This function finish the libmetal environment
 *        and disable caches.
 *
 * @return 0 - succeeded, non-zero for failures.
 */
void FLibmetalSysCleanup()
{
    /* Finish libmetal environment */
    metal_finish();
    //DisableCaches();
}
