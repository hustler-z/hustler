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
 * FilePath: spiffs_qspi_mount_example.c
 * Date: 2023-07-31 11:23:42
 * LastEditTime: 2023-10-08 18:47:54
 * Description:  This file is for sfud qspi mount spiffs file system example function implementation.
 *
 * Modify History:
 *  Ver       Who         Date           Changes
 * -----     ------     --------     --------------------------------------
 *  1.0    liqiaozhong  2023/8/1     add sfud spiffs mount example
 *  1.1    zhangyan     2023/8/8     modify
 *  1.2    liqiaozhong  2023/10/8    divide example into spi and qspi parts
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include <string.h>
#include <stdio.h>

#include "sfud.h"
#include "spiffs_port.h"

#include "ftypes.h"
#include "fdebug.h"
#include "fkernel.h"
#include "fparameters.h"

#include "fqspi_spiffs_port.h"

#include "spiffs_qspi_mount_example.h"
/***************** Macros (Inline Functions) Definitions *********************/
#define MOUNT_CONTROLLER_ID  SFUD_FQSPI0_INDEX /* Default Use QSPI0 id */

#define MOUNT_FLASH_ADDR     0x360000 /* this address should be at least 0x360000 to protect firmware content */
#define SPIFFS_SIZE          0x80000
#define SPIFFS_RW_BUF_SIZE   64
#define SPIFFS_FORMAT        TRUE /* format before mount or not */

#define FSFUD_DEBUG_TAG "FSFUD_EXAMPLE"
#define FSFUD_ERROR(format, ...) FT_DEBUG_PRINT_E(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSFUD_WARRN(format, ...) FT_DEBUG_PRINT_W(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSFUD_INFO(format, ...) FT_DEBUG_PRINT_I(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSFUD_DEBUG(format, ...) FT_DEBUG_PRINT_D(FSFUD_DEBUG_TAG, format, ##__VA_ARGS__)
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
/* 一个页大小两倍的一个RAM缓冲区, 用来加载和维护SPIFFS的逻辑页 */
static volatile u8 spiffs_work_buf[FSPIFFS_LOG_PAGE_SIZE * 2] = {0};
static volatile u8 spiffs_fds_buf[32 * 4] = {0};
static volatile u8 spiffs_cache_buf[(FSPIFFS_LOG_PAGE_SIZE + 32) * 4] = {0};
static u8 spiffs_rw_buf[SPIFFS_RW_BUF_SIZE] = {0};
static FSpiffs spiffs_instance;
static spiffs_config spiffs_config_p;

enum
{
    FSPIFFS_EXAMPLE_OK = 0,
    FSPIFFS_EXAMPLE_INVALID_PRAR,
    FSPIFFS_EXAMPLE_INIT_FAILED,
    FSPIFFS_EXAMPLE_ALREADY_INITED,
    FSPIFFS_EXAMPLE_MOUNT_FAILED,
    FSPIFFS_EXAMPLE_FORMAT_FAILED,
    FSPIFFS_EXAMPLE_NOT_YET_MOUNT,
    FSPIFFS_EXAMPLE_OPEN_FILE_FAILED,
    FSPIFFS_EXAMPLE_WRITE_FILE_FAILED,
    FSPIFFS_EXAMPLE_READ_FILE_FAILED,
    FSPIFFS_EXAMPLE_REMOVE_FILE_FAILED,
    FSPIFFS_EXAMPLE_CLOSE_FILE_FAILED,
};
/************************** Function Prototypes ******************************/

/******************************* Function ************************************/
static int SfudSpiffsInit(void)
{
    int ret = 0;

    memset(&spiffs_config_p, 0, sizeof(spiffs_config_p));
    spiffs_config_p = *FSpiffsGetDefaultConfig(FSPIFFS_PORT_TO_FQSPI);
    spiffs_config_p.phys_addr = MOUNT_FLASH_ADDR; /* may use part of flash */
    spiffs_config_p.phys_size = SPIFFS_SIZE;

    memset(&spiffs_instance, 0, sizeof(spiffs_instance));
    spiffs_instance.fs_addr = MOUNT_FLASH_ADDR;
    spiffs_instance.fs_size = SPIFFS_SIZE;
    spiffs_instance.type = FSPIFFS_PORT_TO_FQSPI;

    if (MOUNT_CONTROLLER_ID >= FSPI_NUM + FQSPI_NUM || MOUNT_CONTROLLER_ID < FSPI_NUM)
    {
        FSFUD_ERROR("MOUNT_CONTROLLER_ID wrong.");
        return FSPIFFS_EXAMPLE_INVALID_PRAR;
    }

    ret = FSpiffsInitialize(&spiffs_instance, MOUNT_CONTROLLER_ID);

    if (ret)
    {
        FSFUD_ERROR("initialize spiffs failed");
        return FSPIFFS_EXAMPLE_INIT_FAILED;
    }

    return ret;
}

static int SfudSpiffMount(void)
{
    int ret = 0;

    if (SPIFFS_FORMAT)
    {
        ret = SPIFFS_mount(&spiffs_instance.fs,
                           &spiffs_config_p,
                           (u8_t *)spiffs_work_buf,
                           (u8_t *)spiffs_fds_buf,
                           sizeof(spiffs_fds_buf),
                           (u8_t *)spiffs_cache_buf,
                           sizeof(spiffs_cache_buf),
                           NULL);

        /* try mount to get status of filesystem  */
        if ((SPIFFS_OK != ret) && (SPIFFS_ERR_NOT_A_FS != ret))
        {
            /* if not a valid filesystem, continue to format,
                other error cannot handle, just exit */
            FSFUD_ERROR("Mount spiffs failed: %d.", ret);
            return FSPIFFS_EXAMPLE_MOUNT_FAILED;
        }

        /* must be unmounted prior to formatting */
        SPIFFS_unmount(&spiffs_instance.fs);

        printf("Format spiffs in progress ...\r\n");
        ret = SPIFFS_format(&spiffs_instance.fs);
        if (SPIFFS_OK != ret)
        {
            FSFUD_ERROR("Format spiffs failed: %d\r\n", ret);
            return FSPIFFS_EXAMPLE_FORMAT_FAILED;
        }
    }

    /* real mount */
    ret = SPIFFS_mount(&spiffs_instance.fs,
                       &spiffs_config_p,
                       (u8_t *)spiffs_work_buf,
                       (u8_t *)spiffs_fds_buf,
                       sizeof(spiffs_fds_buf),
                       (u8_t *)spiffs_cache_buf,
                       sizeof(spiffs_cache_buf),
                       NULL);
    if (ret)
    {
        FSFUD_ERROR("Mount spiffs failed: %d, you must format the medium first.", ret);
        return FSPIFFS_EXAMPLE_MOUNT_FAILED;
    }
    else
    {
        printf("Mount spiffs success.\r\n");
        spiffs_instance.fs_ready = TRUE;
    }

    return ret;
}

static void SpiffsUnmount(void)
{
    SPIFFS_unmount(&spiffs_instance.fs);
    spiffs_instance.fs_ready = FALSE;
    printf("spiffs is unmounted\r\n");
    return;
}

static void SpiffsDeInit(void)
{
    FSpiffsDeInitialize(&spiffs_instance);
    printf("spiffs is deinited !!!\r\n");
    return;
}

/* function example used in cmd */
int FSpiffsQspiMountExample(void)
{
    FError ret = FT_SUCCESS;

    /*sfud spiffs init*/
    ret = SfudSpiffsInit();
    if (ret != FSPIFFS_EXAMPLE_OK)
    {
        FSFUD_ERROR("SfudSpiffsInit failed.\n");
    }

    /*sfud spiffs momunt*/
    ret = SfudSpiffMount();
    if (ret != FSPIFFS_EXAMPLE_OK)
    {
        FSFUD_ERROR("SfudSpiffMount failed.\n");
    }

    /*sfud spiffs unmount*/
    SpiffsUnmount();

    /*sfud spiffs deinit*/
    SpiffsDeInit();

    if (ret == 0)
    {
        printf("%s@%d: Sfud qspi spiffs mount example [success].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: Sfud qspi spiffs mount example [failure].\r\n", __func__, __LINE__);
    }

    return ret;
}