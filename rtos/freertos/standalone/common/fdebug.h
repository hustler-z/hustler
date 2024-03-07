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
 * FilePath: fdebug.h
 * Date: 2021-04-07 09:53:07
 * LastEditTime: 2022-02-17 18:04:58
 * Description:  This file is for showing debug api.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhugengyu  2022/10/27   rename file name
 */

#ifndef FDEBUG_H
#define FDEBUG_H

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "ftypes.h"
#include "fprintk.h"
#ifdef CONFIG_USE_AMP
    #include "fsmp.h"
#endif

#include "fcpu_info.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    FT_LOG_NONE,   /* No log output */
    FT_LOG_ERROR,  /* Critical errors, software module can not recover on its own */
    FT_LOG_WARN,   /* Error conditions from which recovery measures have been taken */
    FT_LOG_INFO,   /* Information messages which describe normal flow of events */
    FT_LOG_DEBUG,  /* Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    FT_LOG_VERBOSE /* Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} ft_log_level_t;

#define LOG_COLOR_BLACK "30"
#define LOG_COLOR_RED "31"
#define LOG_COLOR_GREEN "32"
#define LOG_COLOR_BROWN "33"
#define LOG_COLOR_BLUE "34"
#define LOG_COLOR_PURPLE "35"
#define LOG_COLOR_CYAN "36"
#define LOG_COLOR(COLOR) "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR) "\033[1;" COLOR "m"
#define LOG_RESET_COLOR "\033[0m"
#define LOG_COLOR_E LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D LOG_COLOR(LOG_COLOR_CYAN)
#define LOG_COLOR_V LOG_COLOR(LOG_COLOR_PURPLE)

/* select debug log level */
#ifdef CONFIG_LOG_VERBOS
#define LOG_LOCAL_LEVEL FT_LOG_VERBOSE
#elif defined(CONFIG_LOG_ERROR)
#define LOG_LOCAL_LEVEL FT_LOG_ERROR
#elif defined(CONFIG_LOG_WARN)
#define LOG_LOCAL_LEVEL FT_LOG_WARN
#elif defined(CONFIG_LOG_INFO)
#define LOG_LOCAL_LEVEL FT_LOG_INFO
#elif defined(CONFIG_LOG_DEBUG)
#define LOG_LOCAL_LEVEL FT_LOG_DEBUG
#else
#define LOG_LOCAL_LEVEL FT_LOG_DEBUG
#endif

#define LOG_FORMAT(letter, format) LOG_COLOR_##letter " %s: " format LOG_RESET_COLOR "\r\n"

#define PORT_KPRINTF f_printk

#if defined(CONFIG_LOG_DISPALY_CORE_NUM)
#define DISPALY_CORE_NUM() \
        do {u32 cpu_id;                                                           \
        GetCpuId(&cpu_id);                                                    \
        PORT_KPRINTF("cpu%d:", cpu_id);                                      } while(0)
#else
#define DISPALY_CORE_NUM()
#endif


#ifdef CONFIG_USE_AMP
#define LOG_SPIN_LOCK() SpinLock();
#define LOG_SPIN_UNLOCK() SpinUnlock() ;
#else
#define LOG_SPIN_LOCK()
#define LOG_SPIN_UNLOCK()
#endif

#ifndef CONFIG_LOG_EXTRA_INFO
#define EARLY_LOGE(tag, format, ...) FtDumpLogInfo(tag, FT_LOG_ERROR, LOG_COLOR_RED,format,##__VA_ARGS__)
#define EARLY_LOGI(tag, format, ...) FtDumpLogInfo(tag, FT_LOG_INFO,  LOG_COLOR_GREEN,format, ##__VA_ARGS__)
#define EARLY_LOGD(tag, format, ...) FtDumpLogInfo(tag, FT_LOG_DEBUG, LOG_COLOR_CYAN,format, ##__VA_ARGS__)
#define EARLY_LOGW(tag, format, ...) FtDumpLogInfo(tag, FT_LOG_WARN, LOG_COLOR_BROWN,format, ##__VA_ARGS__)
#define EARLY_LOGV(tag, format, ...) FtDumpLogInfo(tag, FT_LOG_VERBOSE, LOG_COLOR_PURPLE, format,##__VA_ARGS__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#define EARLY_LOGE(tag, format, ...) FtDumpExtraLogInfo(tag, FT_LOG_ERROR, LOG_COLOR_RED,__FILENAME__, __LINE__,format,##__VA_ARGS__)
#define EARLY_LOGI(tag, format, ...) FtDumpExtraLogInfo(tag, FT_LOG_INFO,  LOG_COLOR_GREEN, __FILENAME__,__LINE__,format, ##__VA_ARGS__)
#define EARLY_LOGD(tag, format, ...) FtDumpExtraLogInfo(tag, FT_LOG_DEBUG, LOG_COLOR_CYAN,__FILENAME__, __LINE__,format, ##__VA_ARGS__)
#define EARLY_LOGW(tag, format, ...) FtDumpExtraLogInfo(tag, FT_LOG_WARN, LOG_COLOR_BROWN,__FILENAME__, __LINE__,format, ##__VA_ARGS__)
#define EARLY_LOGV(tag, format, ...) FtDumpExtraLogInfo(tag, FT_LOG_VERBOSE, LOG_COLOR_PURPLE,__FILENAME__,__LINE__, format,##__VA_ARGS__)
#endif
/* do not compile log if define CONFIG_LOG_NONE */
#ifndef CONFIG_LOG_NONE
#define FT_DEBUG_PRINT_I(TAG, format, ...) EARLY_LOGI(TAG, format, ##__VA_ARGS__)
#define FT_DEBUG_PRINT_E(TAG, format, ...) EARLY_LOGE(TAG, format, ##__VA_ARGS__)
#define FT_DEBUG_PRINT_D(TAG, format, ...) EARLY_LOGD(TAG, format, ##__VA_ARGS__)
#define FT_DEBUG_PRINT_W(TAG, format, ...) EARLY_LOGW(TAG, format, ##__VA_ARGS__)
#define FT_DEBUG_PRINT_V(TAG, format, ...) EARLY_LOGV(TAG, format, ##__VA_ARGS__)
#else
#define FT_DEBUG_PRINT_I(TAG, format, ...)
#define FT_DEBUG_PRINT_E(TAG, format, ...)
#define FT_DEBUG_PRINT_D(TAG, format, ...)
#define FT_DEBUG_PRINT_W(TAG, format, ...)
#define FT_DEBUG_PRINT_V(TAG, format, ...)
#endif

#define FT_RAW_PRINTF(format, ...) PORT_KPRINTF(format, ##__VA_ARGS__)

_WEAK void FtDumpLogInfo(const char *tag,  u32 log_level, const char *log_tag_letter,const char *fmt, ...);
_WEAK void FtDumpExtraLogInfo(const char *tag,  u32 log_level, const char *log_tag_letter,const char *FILENAME,u32 line,const char *fmt, ...);
void FtDumpHexWord(const u32 *ptr, u32 buflen);
void FtDumpHexByte(const u8 *ptr, u32 buflen);
void FtDumpHexByteDebug(const u8 *ptr, u32 buflen) ;

#ifdef __cplusplus
}
#endif

#endif // !
