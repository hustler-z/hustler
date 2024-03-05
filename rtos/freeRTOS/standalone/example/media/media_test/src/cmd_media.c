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
 * FilePath: cmd_media.c
 * Date: 2023-08-10 14:53:42
 * LastEditTime: 2029-08-10 17:48:42
 * Description:  This file is for defining the test commands
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2023/08/10  Modify the format and establish the version
 */

/***************************** Include Files *********************************/

#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "ftypes.h"
#include "fdebug.h"
#include "../src/shell.h"
#include "ferror_code.h"

#include "fdc_common_hw.h"
#include "fdcdp.h"
#include "media_test_example.h"

/************************** Variable Definitions *****************************/
/************************** Function Prototypes ******************************/
static void FMediaCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("    media run \r\n");
    printf("        -- probe media,light the screen\r\n");
   
}

static FError FMediaCmdEntry(int argc, char *argv[])
{
    FError ret ;
    if (argc < 2)
    {
        FMediaCmdUsage();
        return -1;
    }
    if (!strcmp(argv[1], "run"))
    {
        ret = FMediaExample();
    }

    return  ret;
}


SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), media, FMediaCmdEntry, test media);