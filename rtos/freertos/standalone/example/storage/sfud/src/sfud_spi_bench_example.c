/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: sfud_spi_bench_example.c
 * Date: 2023-07-31 11:23:42
 * LastEditTime: 2023-10-08 18:47:54
 * Description:  This file is for sfud spi bench example function implementation.
 *
 * Modify History:
 *  Ver       Who         Date           Changes
 * -----     ------     --------     --------------------------------------
 *  1.0    liqiaozhong  2023/8/1     add sfud spi bench example 
 *  1.1    zhangyan     2023/8/8     modify
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include <string.h>
#include <stdio.h>

#include "sfud.h"

#include "ftypes.h"
#include "fdebug.h"
#include "fkernel.h"
#include "fgeneric_timer.h"
#include "finterrupt.h"

#include "sfud_spi_bench_example.h"
/***************** Macros (Inline Functions) Definitions *********************/
#if defined CONFIG_TARGET_E2000Q || defined CONFIG_TARGET_E2000D
#define SFUD_CONTROLLER_ID  SFUD_FSPIM2_INDEX /* E2000 Default Use SPI2 id  */
#else
#define SFUD_CONTROLLER_ID  SFUD_FSPIM0_INDEX /* Default Use SPI0 id */
#endif

#define SYS_TICKRATE_HZ  100

#define FSFUD_DEBUG_TAG "FSFUD_EXAMPLE"
#define FSFUD_ERROR(format, ...) FT_DEBUG_PRINT_E(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSFUD_WARRN(format, ...) FT_DEBUG_PRINT_W(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSFUD_INFO(format, ...) FT_DEBUG_PRINT_I(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSFUD_DEBUG(format, ...) FT_DEBUG_PRINT_D(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static const sfud_flash *flash = NULL;
/************************** Function Prototypes ******************************/

/******************************* Function ************************************/
static void SetupSystick(void)
{    
    GenericTimerStop(GENERIC_TIMER_ID0);
	GenericTimerSetTimerValue(GENERIC_TIMER_ID0, GenericTimerFrequecy() / SYS_TICKRATE_HZ);
	GenericTimerStart(GENERIC_TIMER_ID0);
}

static inline u32 FSpimGetTick(void)
{
    return GenericTimerRead(GENERIC_TIMER_ID0);
}

static inline u32 FSpimTickCastSeconds(u32 tick)
{
    return (u32)(tick / (u32)GenericTimerFrequecy());
}

static inline u32 FSpimTickCastMilliSec(u32 tick)
{
    return (u32)(tick % (u32) GenericTimerFrequecy() / (((u32)GenericTimerFrequecy() * 1 + 999) / 1000));
}

/* function of add sfud spi bench example */
int FSfudSpiBenchExample(void)
{
    FError ret = FT_SUCCESS;
    u32 addr = 0;
    u32 start_time;
    u32 time_cast;
    fsize_t write_size = SFUD_WRITE_MAX_PAGE_SIZE;
    fsize_t read_size = SFUD_WRITE_MAX_PAGE_SIZE;
    u8 *write_data = malloc(write_size);
    u8 *read_data = malloc(read_size);
    fsize_t cur_op_size;

    if (SFUD_CONTROLLER_ID == SFUD_FQSPI0_INDEX)
    {
        FSFUD_ERROR("Cannot bench on qspi to avoid damaging firmware.");
        return SFUD_ERR_NOT_FOUND;
    }

    /* this function will probe all spi and qspi controllers on board */
    if (sfud_init())
    {
        printf("Sfud init fail!");
        return SFUD_ERR_INIT_FAILED;
    }

    flash = sfud_get_device(SFUD_CONTROLLER_ID);
    if (flash == NULL || !flash->init_ok)
    {
        FSFUD_ERROR("Flash on spi-%d is not found!", SFUD_CONTROLLER_ID);
        return SFUD_ERR_NOT_FOUND;
    }
    u32 size = flash->chip.capacity;

    SetupSystick();

    if (write_data && read_data)
    {
        for (fsize_t i = 0; i < write_size; i ++)
        {
            write_data[i] = i & 0xFF;
        }

        /* benchmark testing */
        printf("Erasing the %s %ld MB data, waiting...\r\n", flash->name, size / SZ_1M);
        start_time = FSpimGetTick();
        ret = sfud_chip_erase(flash);
        if (ret == SFUD_SUCCESS)
        {
            time_cast = FSpimGetTick() - start_time;
            printf("Erase benchmark success, total time: %d.%03dS.\r\n",
                   FSpimTickCastSeconds(time_cast),
                   FSpimTickCastMilliSec(time_cast));
        }
        else
        {
            FSFUD_ERROR("Erase benchmark has an error. Error code: %d.", ret);
            goto exit_free;
        }

        /* write test */
        printf("Writing the %s %ld bytes data, waiting...\n", flash->name, size);
        start_time = FSpimGetTick();
        for (fsize_t i = 0; i < size; i += write_size)
        {
            if (i + write_size <= size)
            {
                cur_op_size = write_size;
            }
            else
            {
                cur_op_size = size - i;
            }

            ret = sfud_write(flash, addr + i, cur_op_size, write_data);
            if (ret != SFUD_SUCCESS)
            {
                FSFUD_ERROR("Writing %s failed, already wr for %lu bytes, write %d each time.", flash->name, i, write_size);
                goto exit_free;
            }
        }

        if (ret == SFUD_SUCCESS)
        {
            time_cast = FSpimGetTick() - start_time;
            printf("Write benchmark success, total time: %d.%03dS.\r\n",
                   FSpimTickCastSeconds(time_cast),
                   FSpimTickCastMilliSec(time_cast));
        }
        else
        {
            FSFUD_ERROR("Write benchmark has an error. Error code: %d.", ret);
            goto exit_free;
        }

        /* read test */
        printf("Reading the %s %ld bytes data, waiting...\n", flash->name, size);
        start_time = FSpimGetTick();
        for (fsize_t i = 0; i < size; i += read_size)
        {
            if (i + read_size <= size)
            {
                cur_op_size = read_size;
            }
            else
            {
                cur_op_size = size - i;
            }

            ret = sfud_read(flash, addr + i, cur_op_size, read_data);
            if (ret != SFUD_SUCCESS)
            {
                FSFUD_ERROR("Read %s failed, already rd for %lu bytes, read %d each time.", flash->name, i, read_size);
                goto exit_free;
            }

            /* data check */
            if (memcmp(write_data, read_data, cur_op_size))
            {
                FSFUD_ERROR("Data check ERROR! Please check you flash by other command.");
                ret = SFUD_ERR_READ;
                goto exit_free;
            }
        }

        if (ret == SFUD_SUCCESS)
        {
            time_cast = FSpimGetTick() - start_time;
            printf("Read benchmark success, total time: %d.%03dS.\r\n",
                   FSpimTickCastSeconds(time_cast),
                   FSpimTickCastMilliSec(time_cast));
        }
        else
        {
            FSFUD_ERROR("Read benchmark has an error. Error code: %d.\n", ret);
            goto exit_free;
        }
    }
    else
    {
        FSFUD_ERROR("Allocate buffer failed.");
        goto exit_free;
    }

exit_free:
    if (write_data)
        free(write_data);
    if (read_data)
        free(read_data);

    if (ret == 0)
    {
        printf("%s@%d: Sfud spi bench example success !!! \r\n", __func__, __LINE__);
        printf("[system_example_pass]\r\n");
    }
    else
    {
        printf("%s@%d: Sfud spi bench example failed !!!, ret = %d \r\n", __func__, __LINE__, ret);
    }

    return ret;
}