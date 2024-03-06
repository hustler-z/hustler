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
 * FilePath: common.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:55:34
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */


#ifndef COMMON_H
#define COMMON_H

#include <metal/atomic.h>
#include <metal/alloc.h>
#include <metal/irq.h>
#include <metal/errno.h>
#include <metal/sys.h>
#include <metal/cpu.h>
#include <metal/io.h>
#include <metal/device.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Devices names */
#define BUS_NAME        "generic"
#define SHM_DEV_NAME    "shm"
#define IPI_DEV_NAME    "ipi"
#define TTC_DEV_NAME    "ttc"

extern struct metal_device *shm_dev ;
extern struct metal_device *ipi_dev ;
extern struct metal_device *ttc_dev ;

static inline void wait_for_interrupt()
{
    metal_asm volatile("wfi");
}


/**
 * @breif wait_for_notified() - Loop until notified bit
 *        in channel is set.
 *
 * @param[in] notified - pointer to the notified variable
 */
static inline void  wait_for_notified(atomic_int *notified)
{
    unsigned int flags;

    do
    {
        flags = metal_irq_save_disable();
        if (!atomic_flag_test_and_set(notified))
        {
            metal_irq_restore_enable(flags);
            break;
        }
        wait_for_interrupt();
        metal_irq_restore_enable(flags);
    }
    while (1);
}

#ifdef __cplusplus
}
#endif

#endif // !