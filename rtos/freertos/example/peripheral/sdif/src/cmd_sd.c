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
 * FilePath: cmd_sd.c
 * Date: 2022-07-12 09:33:12
 * LastEditTime: 2022-07-12 09:33:12
 * Description:  This file is for providing user command functions.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    first commit
 */
/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"

#include "FreeRTOS.h"

#include "../src/shell.h"
#include "sdif_read_write.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
static void SdCmdUsage()
{
    printf("Usage:\r\n");
    printf("    sd tf\r\n");
    printf("        -- Demo read and write by sdmmc\r\n");
    printf("    sd emmc\r\n");
    printf("        -- Demo read and write by sdmmc\r\n");
}

static int SdCmdEntry(int argc, char *argv[])
{
    int ret = 0;
    BaseType_t task_ret = 0U;

    if (argc < 2)
    {
        SdCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "tf"))
    {
        task_ret = FFreeRTOSTfWriteRead();
    }
    else if (!strcmp(argv[1], "emmc"))
    {
        task_ret = FFreeRTOSeMMCWriteRead();
    }

    if (pdPASS != task_ret)
    {
        return -2;
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), sd, SdCmdEntry, test freertos sd rw);