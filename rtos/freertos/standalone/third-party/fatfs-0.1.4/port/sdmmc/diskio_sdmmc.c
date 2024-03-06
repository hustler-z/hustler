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
 * FilePath: diskio_sdmmc.c
 * Date: 2022-07-21 13:21:43
 * LastEditTime: 2022-07-21 13:21:44
 * Description:  This file is for fatfs port to sdmmc card
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/3   init commit
 * 2.0   zhugengyu  2023/9/27   adaptor to fsl_sdmmc
 */

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include "fparameters.h"
#include "fdebug.h"
#include "fkernel.h"
#include "diskio.h"
#include "ffconf.h"
#include "ff.h"

#include "sdkconfig.h"

#if defined(CONFIG_FATFS_SDMMC_FSDIF_TF) || defined(CONFIG_FATFS_SDMMC_FSDIF_EMMC)
#include "fsdif_timing.h"
#endif

#include "fsl_partition.h"
#include "fsl_sdmmc.h"

#define FF_DEBUG_TAG "DISKIO-SDMMC"
#define FF_ERROR(format, ...)   FT_DEBUG_PRINT_E(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_INFO(format, ...)    FT_DEBUG_PRINT_I(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_WARN(format, ...)    FT_DEBUG_PRINT_W(FF_DEBUG_TAG, format, ##__VA_ARGS__)

typedef struct
{
    DWORD id;
    DWORD pdrv;
    DWORD sector_beg; /* start sector used in filesystem */
    DWORD sector_sz;  /* size of each sector */
    DWORD sector_cnt; /* total num of sectors used in filesystem */
    boolean init_ok;
    sdmmchost_config_t config;
    /* two type of instances, only one is valid */
#if defined(CONFIG_FATFS_SDMMC_FSDIF_EMMC)
    sdmmc_mmc_t mmc;
#endif
#if defined(CONFIG_FATFS_SDMMC_FSDMMC_TF) || defined(CONFIG_FATFS_SDMMC_FSDIF_TF)
    sdmmc_sd_t sd;
#endif
} ff_sdmmc_disk;

static ff_sdmmc_disk sdif_tf_disk = 
{
    .pdrv = FF_DRV_NOT_USED
};

static ff_sdmmc_disk sdif_emmc_disk = 
{
    .pdrv = FF_DRV_NOT_USED
};

static ff_sdmmc_disk sdmmc_tf_disk = 
{
    .pdrv = FF_DRV_NOT_USED
};

static ff_sdmmc_disk * get_sdmmc_disk(BYTE pdrv)
{
    if (sdif_tf_disk.pdrv == pdrv)
    {
        return &sdif_tf_disk;
    }
    else if (sdif_emmc_disk.pdrv == pdrv)
    {
        return &sdif_emmc_disk;
    }
    else if (sdmmc_tf_disk.pdrv == pdrv)
    {
        return &sdmmc_tf_disk;
    }
    else
    {
        return NULL;
    }
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

static DSTATUS sdmmc_disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    ff_sdmmc_disk *disk = get_sdmmc_disk(pdrv);

    if (NULL == disk)
    {
        return STA_NOINIT;
    }

	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

static DSTATUS sdmmc_disk_initialize(
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    ff_sdmmc_disk *disk = get_sdmmc_disk(pdrv);

    if (FALSE == disk->init_ok) /* TODO: why will fatfs try to init disk twice */
    {
        return RES_ERROR;
    }
    
	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

static DRESULT sdmmc_disk_read (
                BYTE pdrv,		/* Physical drive nmuber to identify the drive */
                BYTE *buff,		/* Data buffer to store read data */
                DWORD sector,	/* Start sector in LBA */
                UINT count		/* Number of sectors to read */
)
{
    ff_sdmmc_disk *disk = get_sdmmc_disk(pdrv);
    DRESULT status = RES_PARERR;

    if ((NULL != disk) && (disk->init_ok) && (sector < disk->sector_cnt))
    {
        if ((kSDMMCHOST_CARD_TYPE_STANDARD_SD == disk->config.cardType) ||
            (kSDMMCHOST_CARD_TYPE_MICRO_SD == disk->config.cardType))
        {
#if defined(CONFIG_FATFS_SDMMC_FSDMMC_TF) || defined(CONFIG_FATFS_SDMMC_FSDIF_TF)
            if ((kStatus_Success != SD_ReadBlocks(&(disk->sd.card), buff, sector + disk->sector_beg, count)))
            {
                FF_ERROR("read sdmmc sector [%d-%d] failed", 
                         sector + disk->sector_beg, 
                         sector + count);
                status = RES_ERROR;                
            }
            else
            {
                status = RES_OK;
            }
#endif
        }
        else if (kSDMMCHOST_CARD_TYPE_EMMC == disk->config.cardType)
        {
#if defined(CONFIG_FATFS_SDMMC_FSDIF_EMMC)
            if ((kStatus_Success != MMC_ReadBlocks(&(disk->mmc.card), buff, sector + disk->sector_beg , count)))
            {
                FF_ERROR("read sdmmc sector [%d-%d] failed", 
                         sector + disk->sector_beg, 
                         sector + disk->sector_beg + count);
                status = RES_ERROR;                
            }
            else
            {
                status = RES_OK;
            }
#endif
        }
    }

    return status;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

static DRESULT sdmmc_disk_write (
                    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
                    const BYTE *buff,	/* Data to be written */
                    DWORD sector,		/* Start sector in LBA */
                    UINT count			/* Number of sectors to write */
)
{
    ff_sdmmc_disk *disk = get_sdmmc_disk(pdrv);
    DRESULT status = RES_PARERR;

    if ((NULL != disk) && (disk->init_ok) && (sector < disk->sector_cnt))
    {
        if ((kSDMMCHOST_CARD_TYPE_STANDARD_SD == disk->config.cardType) ||
            (kSDMMCHOST_CARD_TYPE_MICRO_SD == disk->config.cardType))
        {
#if defined(CONFIG_FATFS_SDMMC_FSDMMC_TF) || defined(CONFIG_FATFS_SDMMC_FSDIF_TF)
            if ((kStatus_Success != SD_WriteBlocks(&(disk->sd.card), buff, sector, count)))
            {
                FF_ERROR("write sdmmc sector [%d-%d] failed", sector, sector + count);
                status = RES_ERROR;                
            }
            else
            {
                status = RES_OK;
            }
#endif
        }
        else if (kSDMMCHOST_CARD_TYPE_EMMC == disk->config.cardType)
        {
#if defined(CONFIG_FATFS_SDMMC_FSDIF_EMMC)
            if ((kStatus_Success != MMC_WriteBlocks(&(disk->mmc.card), buff, sector, count)))
            {
                FF_ERROR("write sdmmc sector [%d-%d] failed", sector, sector + count);
                status = RES_ERROR;                
            }
            else
            {
                status = RES_OK;
            }
#endif
        }
    }

    return status;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT sdmmc_disk_ioctl (
            BYTE pdrv,		/* Physical drive nmuber (0..) */
            BYTE cmd,		/* Control code */
            void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
    ff_sdmmc_disk *disk = get_sdmmc_disk(pdrv);

	res = RES_PARERR;
    if (NULL == disk)
    {
        return res;
    }

    switch (cmd)
    {
        case CTRL_SYNC:			/* Nothing to do */
            res = RES_OK;
        break;

        case GET_SECTOR_COUNT:	/* Get number of sectors on the drive */
            *(DWORD*)buff = disk->sector_cnt;
            res = RES_OK;
        break;

        case GET_SECTOR_SIZE:	/* Get size of sector for generic read/write */
            *(WORD*)buff = disk->sector_sz;
            res = RES_OK;
        break;

        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 4 * disk->sector_sz; /* This is not flash storage that can be erase by command, return 1 */
            res = RES_OK;
        break;
    }

    return res;
}

static const ff_diskio_driver_t sdmmc_disk_drv = 
{
    .init = &sdmmc_disk_initialize,
    .status = &sdmmc_disk_status,
    .read = &sdmmc_disk_read,
    .write = &sdmmc_disk_write,
    .ioctl = &sdmmc_disk_ioctl
};

#ifdef CONFIG_FATFS_SDMMC_FSDIF_TF

void ff_diskio_register_fsdif_tf(BYTE pdrv)
{
    ff_sdmmc_disk *disk = &sdif_tf_disk; 
    sdmmc_sd_t *tf = &(disk->sd);
    sdmmchost_config_t *config = &(disk->config);

    memset(config, 0, sizeof(*config));
    memset(tf, 0, sizeof(*tf));

#if defined(CONFIG_TARGET_PHYTIUMPI)
    config->hostId = FSDIF0_ID;
#else
    config->hostId = FSDIF1_ID;
#endif
    config->hostType = kSDMMCHOST_TYPE_FSDIF;
    config->cardType = kSDMMCHOST_CARD_TYPE_MICRO_SD;
    config->enableDMA = TRUE;
    config->enableIrq = TRUE;
    config->timeTuner = FSdifGetTimingSetting;
    config->endianMode = kSDMMCHOST_EndianModeLittle;
    config->maxTransSize = 1024 * 512;
    config->defBlockSize = 512;
    config->cardClock = SD_CLOCK_50MHZ;
    config->isUHSCard = FALSE;

    if (kStatus_Success != SD_CfgInitialize(tf, config))
    {
        FF_ERROR("Init SD failed!!!");
        return;
    }    
    
    /* for phytium-pi, use the 3rd partiton for filesystem */
    if (CONFIG_FATFS_SDMMC_PARTITION > 0)
    {
        static sdmmc_partition_info s_part_info;

        memset(&s_part_info, 0, sizeof(s_part_info));

        if (kStatus_Success != SDMMC_LookupPartition(&(tf->host), &s_part_info))
        {
            FF_ERROR("Check SD Partition failed !!!");
            return;      
        }

        if ((CONFIG_FATFS_SDMMC_PARTITION > FSL_PARTITION_MAX_NUM) || 
            (s_part_info.parts[CONFIG_FATFS_SDMMC_PARTITION].blk_count == 0))
        {
            FF_ERROR("Invalid SD Partition !!!");
            return;             
        }

        /* use the selected partion in filesystem */
        disk->sector_beg = s_part_info.parts[CONFIG_FATFS_SDMMC_PARTITION].blk_offset;
        disk->sector_cnt = s_part_info.parts[CONFIG_FATFS_SDMMC_PARTITION].blk_count;
        disk->sector_sz = tf->card.blockSize;  
    }
    else
    {
        /* use the while storage in filesystem */
        disk->sector_beg = 0U;
        disk->sector_cnt = tf->card.blockCount;
        disk->sector_sz = tf->card.blockSize;  
    }
  
#if defined(CONFIG_TARGET_PHYTIUMPI)
    disk->id = FSDIF0_ID;
#else
    disk->id = FSDIF1_ID;
#endif
    disk->pdrv = pdrv; /* assign volume for disk */
    disk->init_ok = TRUE;

    ff_diskio_register(pdrv, &sdmmc_disk_drv);

    FF_INFO("Create tf disk as driver-%d", disk->pdrv);
    printf("drv-%d init ok, disk capacity %.0fMB, sector size %d\r\n", 
            pdrv,
            ((double)disk->sector_cnt * (double)disk->sector_sz) / SZ_1M , 
            disk->sector_sz);    
}

#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIF_EMMC

void ff_diskio_register_fsdif_emmc(BYTE pdrv)
{
    ff_sdmmc_disk *disk = &sdif_emmc_disk; 

    sdmmc_mmc_t *emmc = &(disk->mmc);
    sdmmchost_config_t *config = &(disk->config);

    memset(config, 0, sizeof(*config));
    memset(emmc, 0, sizeof(*emmc));

    config->hostId = FSDIF0_ID;
    config->hostType = kSDMMCHOST_TYPE_FSDIF;
    config->cardType = kSDMMCHOST_CARD_TYPE_EMMC;
    config->enableDMA = TRUE;
    config->enableIrq = TRUE;
    config->timeTuner = FSdifGetTimingSetting;
    config->endianMode = kSDMMCHOST_EndianModeLittle;
    config->maxTransSize = 10 * 512;
    config->defBlockSize = 512;
    config->cardClock = MMC_CLOCK_HS200;

    if (kStatus_Success != MMC_CfgInitialize(emmc, config))
    {
        FF_ERROR("Init eMMC failed !!!");
        return;
    }

    disk->id = FSDIF0_ID;
    disk->pdrv = pdrv; /* assign volume for ram disk */

    disk->sector_beg = 0U;
    disk->sector_cnt = emmc->card.userPartitionBlocks;
    disk->sector_sz = emmc->card.blockSize;

    disk->init_ok = TRUE;

    ff_diskio_register(pdrv, &sdmmc_disk_drv);

    FF_INFO("Create emmc disk as driver-%d", disk->pdrv);
    printf("drv-%d init ok, disk capacity %.0fMB, sector size %d\r\n", 
            pdrv,
            ((double)disk->sector_cnt * (double)disk->sector_sz) / SZ_1M , 
            disk->sector_sz);    
}

#endif

#ifdef CONFIG_FATFS_SDMMC_FSDMMC_TF

void ff_diskio_register_fsdmmc_tf(BYTE pdrv)
{
    ff_sdmmc_disk *disk = &sdmmc_tf_disk; 
    sdmmc_sd_t *tf = &disk->sd;
    sdmmchost_config_t *tf_config = &(disk->config);

    memset(tf_config, 0, sizeof(*tf_config));
    memset(tf, 0, sizeof(*tf));

    tf_config->hostId = FSDMMC0_ID;
    tf_config->hostType = kSDMMCHOST_TYPE_FSDMMC;
    tf_config->cardType = kSDMMCHOST_CARD_TYPE_MICRO_SD;
    tf_config->enableDMA = TRUE;
    tf_config->enableIrq = TRUE;
    tf_config->timeTuner = NULL;
    tf_config->endianMode = kSDMMCHOST_EndianModeLittle;
    tf_config->maxTransSize = 1 * 512;
    tf_config->defBlockSize = 512;
    tf_config->cardClock = SD_CLOCK_50MHZ;
    tf_config->isUHSCard = FALSE;    

    if (kStatus_Success != SD_CfgInitialize(tf, tf_config))
    {
        FF_ERROR("Init SD failed !!!");
        return;
    }

    disk->id = FSDMMC0_ID;
    disk->pdrv = pdrv; /* assign volume for ram disk */

    disk->sector_beg = 0U;
    disk->sector_cnt = tf->card.blockCount;
    disk->sector_sz = tf->card.blockSize;

    disk->init_ok = TRUE;

    ff_diskio_register(pdrv, &sdmmc_disk_drv);

    FF_INFO("Create tf card as driver-%d", disk->pdrv);
    printf("drv-%d init ok, disk capacity %.0fMB, sector size %d\r\n", 
            pdrv,
            ((double)disk->sector_cnt * (double)disk->sector_sz) / SZ_1M , 
            disk->sector_sz);    
}

#endif