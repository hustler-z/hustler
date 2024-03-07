/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: cmd_cyptopp.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for crypto++ example function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"

#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "cryptopp_example.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

/* usage info function for cxx example */
static void FCryptoExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("crypto crc32\r\n");
    printf("-- Demonstrate crc32 algorithm from crypto++\r\n");
    printf("cxx oop\r\n");
    printf("-- Demonstrate oop related feature of c++\r\n");
    printf("cxx stl\r\n");
    printf("-- Demonstrate stl related feature of c++\n");
}

/* entry function for crypto++ example */
static int FCryptoExampleEntry(int argc, char *argv[])
{
    int ret = 0;
    u32 id; 
    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FCryptoExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "crc32"))
    {
        ret = FCryptoPPCrc32Example();
    }
    else if (!strcmp(argv[1], "md5"))
    {
        ret = FCryptoPPMd5Example();
    }
    else if (!strcmp(argv[1], "md4"))
    {
        ret = FCryptoPPMd4Example();
    }
    else if (!strcmp(argv[1], "adler32"))
    {
        ret = FCryptoPPAdler32Example();
    }

    if (0 == ret)
    {
        printf("\r\n%s@%d: crypto++ example [success].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("\r\n%s@%d: crypto++ example [failure].\r\n", __func__, __LINE__);
    }

    return ret;
}


/* register command for cxx example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), crypto, FCryptoExampleEntry, crypto++ example);
#endif