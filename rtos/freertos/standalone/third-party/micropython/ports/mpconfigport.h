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
 * FilePath: mpconfigport.h
 * Date: 2023-12-07 14:53:41
 * LastEditTime: 2023-12-07 17:36:39
 * Description:  This file is for set the config
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2023/12/07   Modify the format and establish the version
 */

#ifndef MP_CONFIG_PORT_H
#define MP_CONFIG_PORT_H
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
// options to control how MicroPython is built

// Use the minimal starting configuration (disables all optional features).
#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_MINIMUM)

// You can disable the built-in MicroPython compiler by setting the following
// config option to 0.  If you do this then you won't get a REPL prompt, but you
// will still be able to execute pre-compiled scripts, compiled with mpy-cross.
#define MICROPY_ENABLE_COMPILER     (1)

#define MICROPY_QSTR_EXTRA_POOL           mp_qstr_frozen_const_pool
#define MICROPY_ENABLE_GC                 (1)
#define MICROPY_MODULE_FROZEN_MPY         (1)
#define MICROPY_ENABLE_EXTERNAL_IMPORT    (1)

#define MICROPY_ALLOC_PATH_MAX            (256)

// Use the minimum headroom in the chunk allocator for parse nodes.
#define MICROPY_ALLOC_PARSE_CHUNK_INIT    (16)

// type definitions for the specific machine
typedef intptr_t mp_int_t; // must be pointer size
typedef uintptr_t mp_uint_t; // must be pointer size
typedef long mp_off_t;

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

#define MICROPY_HW_BOARD_NAME "sdk"
#define MICROPY_HW_MCU_NAME "phytium"

#define MICROPY_PY_BUILTINS_BYTEARRAY         (1)
#define MICROPY_PY_ARRAY            (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW            (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW_ITEMSIZE (1)
#define MICROPY_PY_BUILTINS_NEXT2      (1)
#define MICROPY_PY_BUILTINS_RANGE_BINOP (1)
#define MICROPY_PY_BUILTINS_HELP       (1)
#define MICROPY_PY_BUILTINS_HELP_MODULES (1)

#define MICROPY_MIN_USE_STDOUT (1)
#define MICROPY_HEAP_SIZE      (25600) // heap size 25 kilobytes

#define MICROPY_ENABLE_FINALISER    (1)
#define MICROPY_VFS                    (1)
#define MICROPY_VFS_FAT              (1) /* already included in cflags. */
#define MICROPY_FATFS                  (1)
#define MICROPY_FATFS_ENABLE_LFN       (1)
#define MICROPY_FATFS_LFN_CODE_PAGE    437 /* 1=SFN/ANSI 437=LFN/U.S.(OEM) */
#define MICROPY_FATFS_USE_LABEL        (1)
#define MICROPY_FATFS_RPATH            (2)
#define MICROPY_FATFS_MULTI_PARTITION  (1)

// qstr in hash ,number of bytes used to store qstr hash, 2 means 65536 in qstr.c
#define MICROPY_QSTR_BYTES_IN_HASH  (2)
//parser
#define MICROPY_COMP_MODULE_CONST   (0)
#define MICROPY_COMP_CONST          (0)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (0)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (0)
//emit
#define MICROPY_EMIT_X64            (0)
#define MICROPY_EMIT_THUMB          (0)
#define MICROPY_EMIT_INLINE_THUMB   (0)
#define MICROPY_ENABLE_SOURCE_LINE  (1)
//debug
#define MICROPY_MEM_STATS           (0)
#define MICROPY_DEBUG_PRINTERS      (0)
// GC ctrl,otherwise mp_state_ctx err
#define MICROPY_GC_ALLOC_THRESHOLD  (0)
#define MICROPY_PY_GC               (1)
//repl ctrl
#define MICROPY_REPL_EVENT_DRIVEN   (0)
#define MICROPY_REPL_AUTO_INDENT    (1)
#define MICROPY_KBD_EXCEPTION       (1)
#define MICROPY_HELPER_REPL         (1)
//doc string
#define MICROPY_ENABLE_DOC_STRING   (0)
//err
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_TERSE)
//sync
#define MICROPY_PY_ASYNC_AWAIT      (0)
// control over Python builtins//
//str
#define MICROPY_PY_FUNCTION_ATTRS   (1)
#define MICROPY_PY_BUILTINS_STR_UNICODE (1)
#define MICROPY_PY_BUILTINS_STR_CENTER (1)
#define MICROPY_PY_BUILTINS_STR_PARTITION (1)
#define MICROPY_PY_BUILTINS_STR_SPLITLINES (1)
#define MICROPY_PY_ATTRTUPLE        (1)
//builtins,functions and err
#define MICROPY_PY_BUILTINS_INPUT (1)
#define MICROPY_PY_BUILTINS_POW3 (1)
#define MICROPY_PY_BUILTINS_ENUMERATE (1)
#define MICROPY_PY_BUILTINS_FILTER  (1)
#define MICROPY_PY_BUILTINS_FROZENSET (1)
#define MICROPY_PY_BUILTINS_REVERSED (1)
#define MICROPY_PY_BUILTINS_SET     (1)
#define MICROPY_PY_BUILTINS_HELP    (1)
#define MICROPY_PY_BUILTINS_HELP_MODULES (1)
#define MICROPY_PY_BUILTINS_SLICE   (1)
#define MICROPY_PY_BUILTINS_PROPERTY (1)
#define MICROPY_PY_BUILTINS_MIN_MAX (1)
// Whether to set __file__ for imported modules
#define MICROPY_PY___FILE__         (1)
// array
#define MICROPY_PY_BUILTINS_BYTEARRAY (1)
#define MICROPY_PY_ARRAY            (1)
#define MICROPY_PY_ARRAY_SLICE_ASSIGN (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW (1)
#define MICROPY_PY_BUILTINS_COMPLEX (1)
//collection
#define MICROPY_PY_COLLECTIONS      (1)
#define MICROPY_PY_COLLECTIONS_ORDEREDDICT (1)
//math
#define MICROPY_PY_MATH             (1)
#define MICROPY_PY_MATH_SPECIAL_FUNCTIONS (1)
//mem
#define MICROPY_PY_MICROPYTHON_MEM_INFO (1)
//stream
#define MICROPY_STREAMS_NON_BLOCK   (1)
//internal func
#define MICROPY_USE_INTERNAL_ERRNO  (1)
#define MICROPY_USE_INTERNAL_PRINTF (0)
//struct
#define MICROPY_PY_STRUCT           (1)
//sys
#define MICROPY_PY_SYS              (1)
//frozen
#define MICROPY_MODULE_FROZEN_MPY   (1)
//compat
#define MICROPY_CPYTHON_COMPAT      (1)
// System Module  //
#define MICROPY_PY_IO               (1)
//thread
#define MICROPY_PY_THREAD                           (0)
#define MICROPY_PY_THREAD_GIL                       (0)
//cmath
#define MICROPY_PY_CMATH            (0)
//binascii
#define MICROPY_PY_BINASCII                         (1)
#define MICROPY_PY_BUILTINS_BYTES_HEX               (1)
//json
#define MICROPY_PY_JSON                             (1)
//re
#define MICROPY_PY_RE                               (1)
//heapq
#define MICROPY_PY_HEAPQ                            (1)
//hashlib
#define MICROPY_PY_HASHLIB                          (1)
//os
#define MICROPY_PY_OS                               (1)
#define MICROPY_PY_OS_DUPTERM                       (0)
#define MICROPY_PY_OS_SYSTEM        (0)
#define MICROPY_PY_OS_SEP           (1)
//sync
#define MICROPY_PY_OS_SYNC          (1)
//select
#define MICROPY_PY_SELECT           (1)
//socket
#define MICROPY_PY_SOCKET           (1)
//ssl
#define MICROPY_PY_SSL              (1)
//time
#define MICROPY_PY_TIME             (0)
//errno
#define MICROPY_PY_ERRNO            (1)
//micropython lib //
#define MICROPY_PY_BLUETOOTH        (0)
//frame
#define MICROPY_PY_FRAMEBUF         (0)
//machine
#define MICROPY_PY_MACHINE          (1)
// #define MICROPY_PY_MACHINE_PIN_MAKE_NEW mp_pin_make_new
//network
#define MICROPY_PY_NETWORK          (0)
//ctype
#define MICROPY_PY_UCTYPES          (0)
//cryptolib
#define MICROPY_PY_CRYPTOLIB            (0)

// fatfs configuration
#define MICROPY_FATFS_ENABLE_LFN            (1)
#define MICROPY_FATFS_RPATH                 (2)
#define MICROPY_FATFS_MAX_SS                (4096)
#define MICROPY_FATFS_LFN_CODE_PAGE         437 /* 1=SFN/ANSI 437=LFN/U.S.(OEM) */
#define MICROPY_FATFS_NORTC            (1)

//builtins
#define MICROPY_PY_BUILTINS_NEXT2            (1)
#define MICROPY_PY_BUILTINS_FLOAT            (1)
#define MICROPY_PY_BUILTINS_PROPERTY            (1)
#define MICROPY_PY_BUILTINS_COMPILE            (1)
#define MICROPY_PY_BUILTINS_EVAL_EXEC            (1)
#define MICROPY_PY_BUILTINS_EXECFILE            (1)
#define MICROPY_FLOAT_IMPL                          (MICROPY_FLOAT_IMPL_FLOAT)
#define MP_NEED_LOG2 (1)

#define MICROPY_PY_ASYNCIO                  (1)
#define MICROPY_READER_VFS                  (1)
#define MICROPY_PY_RANDOM                   (1)

#define MP_STATE_PORT MP_STATE_VM

#ifdef __cplusplus
}
#endif

#endif
