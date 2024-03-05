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
 * FilePath: cmd_sd.c
 * Date: 2023-04-05 15:22:56
 * LastEditTime: 2023-04-07 13:49:25
 * Description:  This file is for the SD example cmd catalogue.
 *
 * Modify History:
 *  Ver      Who        Date         Changes
 * -----    ------     --------    --------------------------------------
 *  1.0   liqiaozhong  2023/4/7    first commit with init and RW examples
 *  2.0   zhugengyu    2023/10/10  replace sdmmc component as fsl_sdmmc
 *  2.1   zhugengyu    2023/10/23  add sdio card detect examples 
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"
#include <string.h>
#include <stdio.h>

#include "ftypes.h"

#if defined(CONFIG_FSL_SDMMC_USE_FSDMMC)
#include "sdmmc_tf_detect_example.h"
#include "sdmmc_tf_read_write_example.h"
#endif

#if defined(CONFIG_FSL_SDMMC_USE_FSDIF)
#include "sdif_tf_detect_example.h"
#include "sdif_tf_read_write_example.h"
#include "sdif_tf_bench_example.h"
#include "sdif_tf_partition_example.h"

#include "sdif_emmc_detect_example.h"
#include "sdif_emmc_read_write_example.h"
#include "sdif_emmc_bench_example.h"

#include "sdif_sdio_detect_example.h"
#endif

enum
{
    FSD_EXAMPLE_OK = 0,
    FSD_EXAMPLE_NOT_YET_INIT,
    FSD_EXAMPLE_INIT_FAILED,
    FSD_EXAMPLE_INVALID_PARAM,
    FSD_EXAMPLE_READ_WRITE_FAILED,
};
/* usage info function for xxx example */
static void FSdExampleUsage(void)
{
    printf("Usage:\r\n");
#if defined(CONFIG_FSL_SDMMC_USE_FSDIF)
    printf("sd sdif_tf_detect_example [uhs]\r\n");
    printf("-- run sdif TF card detect example.\r\n");
    printf("sd sdif_tf_read_write_example [uhs]\r\n");
    printf("-- run sdif TF card read and write example.\r\n");
    printf("sd sdif_tf_partition_example [uhs]\r\n");
    printf("-- run sdif TF card partition detect example.\r\n");
    printf("sd sdif_tf_bench_example [uhs]\r\n");
    printf("-- speed bench of sdif TF card read and write example.\r\n");
    printf("sd sdif_emmc_detect_example\r\n");
    printf("-- run sdif eMMC card detect example.\r\n");
    printf("sd sdif_emmc_read_write_example\r\n");
    printf("-- run sdif eMMC card read and write example.\r\n");
    printf("sd sdif_emmc_bench_example\r\n");
    printf("-- speed bench of sdif eMMC card read and write example.\r\n");
    printf("sd sdif_sdio_detect_example\r\n");
    printf("-- run sdif SDIO card detect example.\r\n");
#endif
#if defined(CONFIG_FSL_SDMMC_USE_FSDMMC)
    printf("sd sdmmc_tf_detect_example\r\n");
    printf("-- run sdmmc TF card detect example.\r\n");
    printf("sd sdmmc_tf_read_write_example\r\n");
    printf("-- run sdmmc TF card read and write example.\r\n");
#endif
}

/* entry function for SD example */
static int FSdExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FSdExampleUsage();
        return FSD_EXAMPLE_INVALID_PARAM;
    }

#if defined(CONFIG_FSL_SDMMC_USE_FSDIF)
    /* parser example input args and run example */
    if (!strcmp(argv[1], "sdif_tf_detect_example"))
    {
        if ((argc >= 3) && (!strcmp(argv[2], "uhs")))
            ret = FSdifUHSICardDetectExample();
        else
            ret = FSdifSDHCCardDetectExample();
    }
    else if (!strcmp(argv[1], "sdif_tf_read_write_example"))
    {
        if ((argc >= 3) && (!strcmp(argv[2], "uhs")))
            ret = FSdifUHSICardReadWriteExample();
        else
            ret = FSdifSDHCCardReadWriteExample();
    }
    else if (!strcmp(argv[1], "sdif_tf_bench_example"))
    {
        if ((argc >= 3) && (!strcmp(argv[2], "uhs")))
            ret = FSdifUHSICardBenchExample();
        else
            ret = FSdifSDHCCardBenchExample();
    }
    else if (!strcmp(argv[1], "sdif_tf_partition_example"))
    {
        if ((argc >= 3) && (!strcmp(argv[2], "uhs")))
            ret = FSdifUHSICardDetectPartitionExample();
        else
            ret = FSdifSDHCCardDetectPartitionExample();
    }
    else if (!strcmp(argv[1], "sdif_emmc_detect_example"))
    {
        ret = FSdifEmmcDetectExample(); 
    }
    else if (!strcmp(argv[1], "sdif_emmc_read_write_example"))
    {
        ret = FSdifEmmcReadWriteExample();
    }
    else if (!strcmp(argv[1], "sdif_emmc_bench_example"))
    {
        ret = FSdifEmmcBenchExample();
    }
    else if (!strcmp(argv[1], "sdif_sdio_detect_example"))
    {
        ret = FSdifSdioCardDetectExample();
    }
#endif

#if defined(CONFIG_FSL_SDMMC_USE_FSDMMC)
    if (!strcmp(argv[1], "sdmmc_tf_detect_example"))
    {
        ret = FSdmmcTfCardDetectExample();
    }
    else if (!strcmp(argv[1], "sdmmc_tf_read_write_example"))
    {
        ret = FSdmmcTfCardReadWriteExample();
    }
#endif

    return ret;
}

SHELL_EXPORT_EXIT_MSG(sd) =
{
    {0, "Success."},
    {1, "Instance not init."},
    {2, "Instance init fail."},
    {3, "Invalid parameters."},
    {4, "SD read or write fail."}
};
/* register command for SD example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), sd, FSdExampleEntry, SD example);
#endif