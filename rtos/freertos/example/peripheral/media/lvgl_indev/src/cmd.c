/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: cmd.c
 * Date: 2022-08-25 16:22:40
 * LastEditTime: 2023-07-07 15:40:40
 * Description:  This file is for providing the demo commond
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2022/12/20  Modify the format and establish the version
 *  1.1  Wangzq     2023/07/07  Add the multi-display config
 */
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"
#include "FreeRTOS.h"
#include "../src/shell.h"

#include "fdc_common_hw.h"
#include "fdcdp.h"
#include "fdp_hw.h"

#include "lv_port_disp.h"
#include "lv_obj.h"
#include "lv_conf.h"
#include "lv_indev_creat.h"
#include "lv_indev_test.h"

static void FFreeRTOSMediaCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("    Media init \r\n");
    printf("        -- init the dp \r\n");;
    printf("    Media lvgl-init \r\n");
    printf("        -- init the lvgl and set the para for demo\r\n");
    printf("    Media init-kb <id>\r\n");
    printf("        -- init the keyborad,test id is 0\r\n");
    printf("    Media init-ms <id>\r\n");
    printf("        -- init the mouse,test id is 1\r\n");
    printf("    Media demo \r\n");
    printf("        -- a test demo to use keyboard and mouse\r\n");
    printf("    Media lsusb \r\n");
    printf("        -- list the usb device\r\n");
}

static int MediaCmdEntry(int argc, char *argv[])
{
    u32 id ;
    u32 usb_id = 0;
    u32 channel;
    static boolean inited = FALSE;
    if (argc < 2)
    {
        FFreeRTOSMediaCmdUsage();
        return -1;
    }
    if (!strcmp(argv[1], "init"))
    {
            BaseType_t task_ret = FFreeRTOSMediaInitCreate();
  
            if (pdPASS != task_ret)
            {
                return -2;
            }
            inited = TRUE;
    }
    if (!strcmp(argv[1], "lvgl-init"))
    {
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        BaseType_t task_ret = FFreeRTOSlVGLConfigCreate();

        if (pdPASS != task_ret)
        {
            return -2;
        }
    }
    if (!strcmp(argv[1], "init-kb"))
    {
        if (argc < 3)
        {
            return -2;
        }
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        usb_id = (uint8_t)simple_strtoul(argv[2], NULL, 10);
        BaseType_t task_ret = FFreeRTOSInitKbCreate(usb_id);

        if (pdPASS != task_ret)
        {
            return -2;
        }
    }
    if (!strcmp(argv[1], "init-ms"))
    {
        if (argc < 3)
        {
            return -2;
        }
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        usb_id = (uint8_t)simple_strtoul(argv[2], NULL, 10);
        BaseType_t task_ret = FFreeRTOSInitMsCreate(usb_id);

        if (pdPASS != task_ret)
        {
            return -2;
        }
    }
    if (!strcmp(argv[1], "demo"))
    {
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        BaseType_t task_ret = FFreeRTOSDemoCreate();

        if (pdPASS != task_ret)
        {
            return -2;
        }
    }
    if (!strcmp(argv[1], "deinit"))
    {
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        if (argc < 3)
        {
            return -2;
        }

        channel = (uint8_t)simple_strtoul(argv[2], NULL, 10);
        FFreeRTOSMediaChannelDeinit(channel);
    }

    if (!strcmp(argv[1], "lsusb"))
    {
        FFreeRTOSListUsbDev(argc - 1, &argv[1]);
    }
    return 0;

}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), Media, MediaCmdEntry, test freertos media driver);