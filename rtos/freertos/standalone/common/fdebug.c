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
 * FilePath: fdebug.c
 * Date: 2021-04-25 16:44:23
 * LastEditTime: 2022-02-17 18:04:50
 * Description:  This file is for providing debug functions.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/10/27   rename file name
 */
#include <string.h>
#include <stdarg.h>
#include "fdebug.h"
#include "fprintk.h"
#include "stdio.h"

#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
void FtDumpHexByte(const u8 *ptr, u32 buflen)
{
    u8 *buf = (u8 *)ptr;
    fsize_t i, j;

    for (i = 0; i < buflen; i += 16)
    {
        f_printk("%p: ", ptr + i);

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
            {
                f_printk("%02X ", buf[i + j]);
            }
            else
            {
                f_printk("   ");
            }
        f_printk(" ");

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
            {
                f_printk("%c", (char)(__is_print(buf[i + j]) ? buf[i + j] : '.'));
            }
        f_printk("\r\n");
    }
}

void FtDumpHexByteDebug(const u8 *ptr, u32 buflen)
{
    u8 *buf = (u8 *)ptr;
    fsize_t i, j;

    for (i = 0; i < buflen; i += 16)
    {
        f_printk("%x: ", ptr + i);

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
            {
                f_printk("%x ", buf[i + j]);
            }
            else
            {
                f_printk("   ");
            }
        f_printk(" ");

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
            {
                f_printk("%c", (char)(__is_print(buf[i + j]) ? buf[i + j] : '.'));
            }
        f_printk("\r\n");
    }
}

_WEAK void FtDumpExtraLogInfo(const char *tag,  u32 log_level, const char *log_tag_letter, const char *file_name, u32 line, const char *fmt, ...)
{
    va_list ap;
    char buf[256] = {0};
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    do
    {
        if (LOG_LOCAL_LEVEL < log_level)
        {
            return;
        }
        DISPALY_CORE_NUM();
        if (strcmp(log_tag_letter, "31") == 0)
        {
            PORT_KPRINTF("\033[31m%s:%s @%s:%d\033[0m \r\n", tag, buf, file_name, line);
        }
        else if (strcmp(log_tag_letter, "32") == 0)
        {
            PORT_KPRINTF("\033[32m%s:%s @%s:%d\033[0m \r\n", tag, buf, file_name, line);
        }
        else if (strcmp(log_tag_letter, "33") == 0)
        {
            PORT_KPRINTF("\033[33m%s:%s @%s:%d\033[0m \r\n", tag, buf, file_name, line);
        }
        else if (strcmp(log_tag_letter, "36") == 0)
        {
            PORT_KPRINTF("\033[36m%s:%s @%s:%d\033[0m \r\n", tag, buf, file_name, line);
        }
        else
        {
            PORT_KPRINTF("\033[35m%s:%s @%s:%d\033[0m \r\n", tag, buf, file_name, line);
        }
    }
    while (0);


}
_WEAK void FtDumpLogInfo(const char *tag,  u32 log_level, const char *log_tag_letter, const char *fmt, ...)
{
    va_list ap;
    char buf[256] = {0};
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    do
    {
        if (LOG_LOCAL_LEVEL < log_level)
        {
            return;
        }
        LOG_SPIN_LOCK();
        DISPALY_CORE_NUM();
        if (strcmp(log_tag_letter, "31") == 0)
        {
            PORT_KPRINTF("\033[31m%s:%s\033[0m \r\n", tag, buf);
        }
        else if (strcmp(log_tag_letter, "32") == 0)
        {
            PORT_KPRINTF("\033[32m%s:%s\033[0m \r\n", tag, buf);
        }
        else if (strcmp(log_tag_letter, "33") == 0)
        {
            PORT_KPRINTF("\033[33m%s:%s\033[0m \r\n", tag, buf);
        }
        else if (strcmp(log_tag_letter, "36") == 0)
        {
            PORT_KPRINTF("\033[36m%s:%s\033[0m \r\n", tag, buf);
        }
        else
        {
            PORT_KPRINTF("\033[35m%s:%s\033[0m \r\n", tag, buf);
        }
        LOG_SPIN_UNLOCK();
    }
    while (0);

}

void FtDumpHexWord(const u32 *ptr, u32 buflen)
{
    u32 *buf = (u32 *)ptr;
    u8 *char_data = (u8 *)ptr;
    fsize_t i, j;
    buflen = buflen / 4;
    for (i = 0; i < buflen; i += 4)
    {
        f_printk("%p: ", ptr + i);

        for (j = 0; j < 4; j++)
        {
            if (i + j < buflen)
            {
                f_printk("%lx ", buf[i + j]);
            }
            else
            {
                f_printk("   ");
            }
        }

        f_printk(" ");

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
            {
                f_printk("%c", (char)(__is_print(char_data[i + j]) ? char_data[i + j] : '.'));
            }

        f_printk("\r\n");
    }
}
