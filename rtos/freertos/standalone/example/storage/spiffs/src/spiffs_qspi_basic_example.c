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
 * FilePath: spiffs_qspi_basic_example.c
 * Date: 2023-07-31 11:23:42
 * LastEditTime: 2023-10-08 18:47:54
 * Description:  This file is for sfud spiffs basic example function implementation.
 *
 * Modify History:
 *  Ver       Who         Date           Changes
 * -----     ------     --------     --------------------------------------
 *  1.0    liqiaozhong  2023/8/1     sfud spiffs basic example
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

#include "ftypes.h"
#include "fdebug.h"
#include "fkernel.h"
#include "fassert.h"
#include "fparameters.h"

#include "sfud.h"
#include "spiffs_port.h"

#include "fqspi_spiffs_port.h"

#include "spiffs_qspi_basic_example.h"
/***************** Macros (Inline Functions) Definitions *********************/
#define MOUNT_CONTROLLER_ID  SFUD_FQSPI0_INDEX /* Default Use QSPI0 id */

#define MOUNT_FLASH_ADDR     0x360000 /* this address should be at least 0x360000 to protect firmware content */
#define SPIFFS_SIZE          0x80000
#define SPIFFS_RW_BUF_SIZE   64
#define SPIFFS_FORMAT        TRUE

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
        FSFUD_ERROR("initialize spiffs failed.");
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

        printf("Format spiffs in progress...\r\n");
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
    printf("spiffs is unmounted.\r\n");
}

static int SpiffsCreateFile(const char *file_name)
{
    FASSERT((file_name) && (strlen(file_name) > 0));
    if (FALSE == spiffs_instance.fs_ready)
    {
        printf("Please mount file system first!!!\r\n");
        return FSPIFFS_EXAMPLE_NOT_YET_MOUNT;
    }

    s32_t ret = 0;

    /* create file */
    ret = SPIFFS_creat(&spiffs_instance.fs, file_name, 0);
    if (ret < 0)
    {
        printf("Failed to create file %s\r\n", file_name);
        return FSPIFFS_EXAMPLE_OPEN_FILE_FAILED;
    }

    /* open file */
    spiffs_file fd = SPIFFS_open(&spiffs_instance.fs, file_name, SPIFFS_RDONLY, 0);
    if (0 > fd)
    {
        printf("Failed to open file %s errno %d\r\n", file_name, SPIFFS_errno(&spiffs_instance.fs));
        return FSPIFFS_EXAMPLE_OPEN_FILE_FAILED;
    }

    /* check file status */
    static spiffs_stat status;
    memset(&status, 0, sizeof(status));
    ret = SPIFFS_fstat(&spiffs_instance.fs, fd, &status);
    if (ret < 0)
    {
        printf("Failed to get status of file %s errno %d\r\n", file_name, SPIFFS_errno(&spiffs_instance.fs));
        ret = FSPIFFS_EXAMPLE_OPEN_FILE_FAILED;
        goto err_exit;
    }

    if (0 != strcmp(status.name, file_name))
    {
        printf("Created file name %s != %s\r\n", status.name, file_name);
        ret = FSPIFFS_EXAMPLE_OPEN_FILE_FAILED;
        goto err_exit;
    }

    if (0 != status.size)
    {
        printf("Invalid file size %d\r\n", status.size);
        ret = FSPIFFS_EXAMPLE_OPEN_FILE_FAILED;
        goto err_exit;
    }

    printf("Create file %s success !!!\r\n", file_name);

err_exit:
    (void)SPIFFS_close(&spiffs_instance.fs, fd);
    return ret;
}

static int SpiffsWriteFile(const char *file_name, const char *str)
{
    FASSERT((file_name) && (strlen(file_name) > 0));
    FASSERT(str);
    int ret = FSPIFFS_EXAMPLE_OK;
    const u32 wr_len = strlen(str) + 1;

    spiffs_file fd = SPIFFS_open(&spiffs_instance.fs, file_name, SPIFFS_RDWR | SPIFFS_TRUNC, 0);
    if (0 > fd)
    {
        printf("Failed to open file %s errno %d\r\n", file_name, SPIFFS_errno(&spiffs_instance.fs));
        return FSPIFFS_EXAMPLE_OPEN_FILE_FAILED;
    }

    int result = SPIFFS_write(&spiffs_instance.fs, fd, (void *)str, wr_len);
    if (result < 0)
    {
        printf("Failed to write file %s errno %d\r\n", file_name, SPIFFS_errno(&spiffs_instance.fs));
        ret = FSPIFFS_EXAMPLE_WRITE_FILE_FAILED;
        goto err_exit;
    }

    /* check file status */
    static spiffs_stat status;
    memset(&status, 0, sizeof(status));
    result = SPIFFS_fstat(&spiffs_instance.fs, fd, &status);
    if (result < 0)
    {
        printf("Failed to get status of file %s errno %d\r\n", file_name, SPIFFS_errno(&spiffs_instance.fs));
        ret = FSPIFFS_EXAMPLE_WRITE_FILE_FAILED;
        goto err_exit;
    }

    if (status.size != wr_len)
    {
        printf("File write size %ld != %ld\r\n", status.size, wr_len);
        ret = FSPIFFS_EXAMPLE_WRITE_FILE_FAILED;
        goto err_exit;
    }

    /* flush all pending writes from cache to flash */
    (void)SPIFFS_fflush(&spiffs_instance.fs, fd);
    printf("Write file %s with %d bytes success !!!\r\n", file_name, wr_len);
err_exit:
    (void)SPIFFS_close(&spiffs_instance.fs, fd);
    return ret;
}
/* function of sfud spiffs basic example */

static int SpiffsReadFile(const char *file_name)
{
    FASSERT((file_name) && (strlen(file_name) > 0));
    int ret = FSPIFFS_EXAMPLE_OK;
    int result = SPIFFS_OK;

    if (FALSE == spiffs_instance.fs_ready)
    {
        printf("Please mount file system first !!!\r\n");
        return FSPIFFS_EXAMPLE_NOT_YET_MOUNT;
    }

    /* check file status */
    static spiffs_stat status;

    spiffs_flags open_flags = 0;

    /* open the file in read-only mode */
    open_flags = SPIFFS_RDWR;
    spiffs_file fd = SPIFFS_open(&spiffs_instance.fs, file_name, open_flags, 0);
    if (0 > fd)
    {
        printf("Failed to open file %s errno %d\r\n", file_name, SPIFFS_errno(&spiffs_instance.fs));
        return FSPIFFS_EXAMPLE_OPEN_FILE_FAILED;
    }

    /* check file status */
    memset(&status, 0, sizeof(status));
    result = SPIFFS_fstat(&spiffs_instance.fs, fd, &status);
    if (result < 0)
    {
        printf("Failed to get status of file %s errno %d\r\n", file_name, SPIFFS_errno(&spiffs_instance.fs));
        ret = FSPIFFS_EXAMPLE_OPEN_FILE_FAILED;
        goto err_exit;
    }

    s32_t offset = SPIFFS_lseek(&spiffs_instance.fs, fd, 0, SPIFFS_SEEK_END);
    printf("File size: %ld\r\n", offset);
    if ((s32_t)status.size != offset)
    {
        printf("File %s spiffs:%ld! = fs:%ld\r\n", file_name, status.size, offset);
        ret = FSPIFFS_EXAMPLE_OPEN_FILE_FAILED;
        goto err_exit;
    }

    memset(spiffs_rw_buf, 0, SPIFFS_RW_BUF_SIZE);

    /* seek to offset and start read */
    if (0 > SPIFFS_lseek(&spiffs_instance.fs, fd, 0, SPIFFS_SEEK_SET))
    {
        printf("Seek file failed !!!\r\n");
        ret = FSPIFFS_EXAMPLE_READ_FILE_FAILED;
        goto err_exit;
    }

    printf("Read %s from position %ld\r\n", file_name, SPIFFS_tell(&spiffs_instance.fs, fd));

    s32_t read_len = min((s32_t)SPIFFS_RW_BUF_SIZE, (s32_t)status.size);
    s32_t read_bytes = SPIFFS_read(&spiffs_instance.fs, fd, (void *)spiffs_rw_buf, read_len);
    if (read_bytes < 0)
    {
        printf("Failed to read file %s errno %d\r\n", file_name, SPIFFS_errno(&spiffs_instance.fs));
        ret = FSPIFFS_EXAMPLE_READ_FILE_FAILED;
        goto err_exit;
    }

    printf("Read file %s with %d bytes successfully !!!\r\n",
           file_name, read_bytes);
    FtDumpHexByte(spiffs_rw_buf, read_bytes);

err_exit :
    /* close file */
    (void)SPIFFS_close(&spiffs_instance.fs, fd);
    return ret;
}

static int SpiffsListAll(void)
{
    int ret = FSPIFFS_EXAMPLE_OK;
    int result = SPIFFS_OK;

    if (FALSE == spiffs_instance.fs_ready)
    {
        printf("Please mount file system first !!!\r\n");
        return FSPIFFS_EXAMPLE_NOT_YET_MOUNT;
    }

    static spiffs_DIR dir;
    static struct spiffs_dirent entry;

    memset(&dir, 0, sizeof(dir));
    memset(&entry, 0, sizeof(entry));

    struct spiffs_dirent *cur_entry = &entry;
    (void)SPIFFS_opendir(&spiffs_instance.fs, "/", &dir);

    while (NULL != (cur_entry = SPIFFS_readdir(&dir, cur_entry)))
    {
        printf("-- %s file-id: [0x%04x] page-id: [%d] file-size: %d\r\n",
               cur_entry->name,
               cur_entry->pix,
               cur_entry->obj_id,
               cur_entry->size);
    }

    (void)SPIFFS_closedir(&dir);
    return ret;
}

static int SpiffsRemoveFile(const char *file_prefix_name)
{
    FASSERT((file_prefix_name) && (strlen(file_prefix_name) > 0));
    int ret = FSPIFFS_EXAMPLE_OK;
    int result = SPIFFS_OK;

    if (FALSE == spiffs_instance.fs_ready)
    {
        printf("Please mount file system first !!!\r\n");
        return FSPIFFS_EXAMPLE_NOT_YET_MOUNT;
    }

    static spiffs_DIR dir;
    static struct spiffs_dirent entry;

    memset(&dir, 0, sizeof(dir));
    memset(&entry, 0, sizeof(entry));

    struct spiffs_dirent *cur_entry = &entry;
    spiffs_file fd = -1;
    (void)SPIFFS_opendir(&spiffs_instance.fs, "/", &dir);

    while (NULL != (cur_entry = SPIFFS_readdir(&dir, cur_entry)))
    {
        if (0 == strncmp(file_prefix_name, (const char *)cur_entry->name, strlen(file_prefix_name)))
        {
            /* find one file match with file_prefix_name */
            fd = SPIFFS_open_by_dirent(&spiffs_instance.fs, cur_entry, SPIFFS_RDWR, 0);
            if (fd < 0)
            {
                printf("Failed to open file %s errno %d\r\n", cur_entry->name, SPIFFS_errno(&spiffs_instance.fs));
                ret = FSPIFFS_EXAMPLE_OPEN_FILE_FAILED;
                break;
            }

            result = SPIFFS_fremove(&spiffs_instance.fs, fd);
            if (result < SPIFFS_OK)
            {
                printf("Failed to remove file %s errno %d\r\n", cur_entry->name, SPIFFS_errno(&spiffs_instance.fs));
                ret = FSPIFFS_EXAMPLE_REMOVE_FILE_FAILED;
                break;
            }

            result = SPIFFS_close(&spiffs_instance.fs, fd);
            if (result < SPIFFS_OK)
            {
                printf("Failed to close file %s errno %d\r\n", cur_entry->name, SPIFFS_errno(&spiffs_instance.fs));
                ret = FSPIFFS_EXAMPLE_CLOSE_FILE_FAILED;
                break;
            }
        }
    }

    (void)SPIFFS_closedir(&dir);
    if (FSPIFFS_EXAMPLE_OK == ret)
    {
        printf("Remove dir/file with prefix %s success !!!\r\n", file_prefix_name);
    }

    return ret;
}

static int SpiffsStatus(void)
{
    int ret = FSPIFFS_EXAMPLE_OK;
    int result = SPIFFS_OK;

    if (FALSE == spiffs_instance.fs_ready)
    {
        printf("Please mount file system first !!!\r\n");
        return FSPIFFS_EXAMPLE_NOT_YET_MOUNT;
    }

    u32_t total, used;
    result = SPIFFS_info(&spiffs_instance.fs, &total, &used);
    if (result < SPIFFS_OK)
    {
        printf("Errno: %d\r\n",  SPIFFS_errno(&spiffs_instance.fs));
        ret = FSPIFFS_EXAMPLE_READ_FILE_FAILED;
    }

    printf("Space --> free: %d Bytes, used: %d Bytes, total: %d Bytes\r\n", (total - used), used, total);
    return ret;
}

static void SpiffsDeInit(void)
{
    FSpiffsDeInitialize(&spiffs_instance);
    printf("spiffs is deinited!!!\r\n");
}

/* example function used in cmd */
int FSpiffsQspiBasicExample(void)
{
    FError ret = FT_SUCCESS;
    const char *file_name = "test.txt";
    const char *str = "hello qspi nor flash spiffs";

    ret = SfudSpiffsInit();
    if (ret != FSPIFFS_EXAMPLE_OK)
    {
        FSFUD_ERROR("SfudSpiffsInit failed.\n");
    }

    ret = SfudSpiffMount();
    if (ret != FSPIFFS_EXAMPLE_OK)
    {
        FSFUD_ERROR("SfudSpiffMount failed.\n");
    }

    ret = SpiffsCreateFile(file_name);
    if (ret != FSPIFFS_EXAMPLE_OK)
    {
        FSFUD_ERROR("SpiffsCreateFile failed.\n");
    }

    ret = SpiffsWriteFile(file_name, str);
    if (ret != FSPIFFS_EXAMPLE_OK)
    {
        FSFUD_ERROR("SpiffsWriteFile failed.\n");
    }
    
    ret = SpiffsReadFile(file_name);
    if (ret != FSPIFFS_EXAMPLE_OK)
    {
        FSFUD_ERROR("SpiffsReadFile failed.\n");
    }

    ret =SpiffsListAll();
    if (ret != FSPIFFS_EXAMPLE_OK)
    {
        FSFUD_ERROR("SpiffsListAll failed.\n");
    }

    ret =SpiffsStatus();
    if (ret != FSPIFFS_EXAMPLE_OK)
    {
        FSFUD_ERROR("SpiffsStatus failed.\n");
    }

    ret =SpiffsRemoveFile(file_name);
    if (ret != FSPIFFS_EXAMPLE_OK)
    {
        FSFUD_ERROR("SpiffsRemoveFile failed.\n");
    }

    /*sfud spiffs unmount*/
    SpiffsUnmount();

    /*sfud spiffs deinit*/
    SpiffsDeInit();

    if (ret == 0)
    {
        printf("%s@%d: Sfud qspi spiffs basic example [success].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: Sfud qspi spiffs basic example [failure].\r\n", __func__, __LINE__);
    }

    return ret;
}