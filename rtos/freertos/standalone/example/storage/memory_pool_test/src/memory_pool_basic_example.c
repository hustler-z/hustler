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
 * FilePath: memory_pool_basic_example.c
 * Date: 2023-07-31 11:23:42
 * LastEditTime: 2023-08-1 18:47:54
 * Description:  This file is for memory pool basic test example function implementation.
 *
 * Modify History:
 *  Ver       Who         Date           Changes
 * -----     ------     --------     --------------------------------------
 *  1.0    liqiaozhong  2023/8/1     add memory pool basic test example
 */

/***************************** Include Files *********************************/
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ftypes.h"
#include "fdebug.h"

#include "fmemory_pool.h"

#include "memory_pool_basic_example.h"
/***************** Macros (Inline Functions) Definitions *********************/
#define FMEMP_DEBUG_TAG "FMEMORYPOOL_EXAMPLE"
#define FMEMP_ERROR(format, ...) FT_DEBUG_PRINT_E(FMEMP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEMP_WARRN(format, ...) FT_DEBUG_PRINT_W(FMEMP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEMP_INFO(format, ...) FT_DEBUG_PRINT_I(FMEMP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEMP_DEBUG(format, ...) FT_DEBUG_PRINT_D(FMEMP_DEBUG_TAG, format, ##__VA_ARGS__)
/**************************** Type Definitions *******************************/
typedef struct
{
    u32 key;
    u64 val;
    char name[14];
} Dummy;
/************************** Variable Definitions *****************************/
#if defined(__aarch64__) /* aarch64 needs more space */
static u8 tlsf_buf[9000] __attribute__((aligned(4))) = {0};
#else
static u8 tlsf_buf[5000] __attribute__((aligned(4))) = {0};
#endif
/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/

/******************************* Function ************************************/
/* get memory pool usage condition */
static void MemoryPoolUsage(FMemp *memp)
{
    u32 total = 0, used = 0, max_used = 0;

    FMemProbe(memp, &total, &used, &max_used);
    printf("used: %d, total: %d, peak: %d\r\n", used, total, max_used);
}

/* to tell if addr is aligned */
static boolean MemoryPoolIsAligned(void *addr, u32 alignment)
{
    printf("%p %s by %d\r\n", addr,
           ((uintptr)addr % alignment == 0) ? "is aligned" : "is not aligned",
           alignment);
}

/* function of memory pool basic test example */
int MemoryPoolBasicExample(void)
{
    tlsf_t *tlsf;
    FError ret = FMEMP_SUCCESS;
    FMemp memp;
    Dummy *dummy1;
    u8 *dummy0;
    u8 *dummy2;
    u8 *dummy4;
    u32 alignment;
    u64 *dummy3;

    /* memory pool init */
    memset(&memp, 0, sizeof(memp));
    ret = FMempInit(&memp, &tlsf_buf[0], &tlsf_buf[0] + sizeof(tlsf_buf));
    if (ret != FMEMP_SUCCESS)
    {
        FMEMP_ERROR("Memory pool init failed: 0x%x", ret);
        goto err_ret;
    }
    MemoryPoolUsage(&memp);

    /* test FMempMalloc api */
    dummy0 = FMEMP_MALLOC(&memp, sizeof(dummy0));
    if (dummy0)
    {
        dummy0[0] = 0xaa;
        printf("Malloc success.\r\n");
    }
    else
    {
        FMEMP_ERROR("FMempMalloc() failed.");
        ret = 1;
        goto err_ret;
    }

    /* test FMempMallocAlign api */
    alignment = 128;
    dummy1 = FMEMP_MALLOC_ALIGN(&memp, sizeof(dummy1), alignment);
    if (dummy1)
    {
        dummy1->key = 17;
        MemoryPoolIsAligned(dummy1, alignment);
    }
    else
    {
        FMEMP_ERROR("FMempMallocAlign() failed.");
        ret = 1;
        goto err_ret;
    }

    alignment = 64;
    dummy2 = FMEMP_MALLOC_ALIGN(&memp, sizeof(dummy2), alignment);
    if (dummy2)
    {
        dummy2[0] = 0xbb;
        MemoryPoolIsAligned(dummy2, alignment);
    }
    else
    {
        FMEMP_ERROR("FMempMallocAlign() failed.");
        ret = 1;
        goto err_ret;
    }

    alignment = 32;
    dummy3 = FMEMP_MALLOC_ALIGN(&memp, sizeof(u64) * 5, alignment);
    if (dummy3)
    {
        dummy3[1] = 2023;
        MemoryPoolIsAligned(dummy3, alignment);
    }
    else
    {
        FMEMP_ERROR("FMempMallocAlign() failed.");
        ret = 1;
        goto err_ret;
    }

    /* test FMempCalloc api */
    dummy4 = FMEMP_CALLOC(&memp, 4, sizeof(u16));
    if (dummy4)
    {
        printf("After calloc --space: %d %d %d %d\r\n", dummy4[0], dummy4[1], dummy4[2], dummy4[3]);
    }
    else
    {
        FMEMP_ERROR("FMempCalloc() failed.");
        ret = 1;
        goto err_ret;
    }

    /* test FMempRealloc api */
    dummy2 = FMEMP_REALLOC(&memp, dummy2, sizeof(u8) * 8);
    if (dummy2)
    {
        dummy2[7] = 0xcc;
        printf("Realloc success.\r\n");
    }
    else
    {
        FMEMP_ERROR("FMempRealloc() failed.");
        ret = 1;
        goto err_ret;
    }

    /* test FMempFree api */
    FMemListAll(&memp);
err_ret:
    if (&memp)
    {
        if (dummy0) /* free all memroy */
            FMEMP_FREE(&memp, dummy0);
        if (dummy1)
            FMEMP_FREE(&memp, dummy1);
        if (dummy2)
            FMEMP_FREE(&memp, dummy2);
        if (dummy3)
            FMEMP_FREE(&memp, dummy3);
        if (dummy4)
            FMEMP_FREE(&memp, dummy4);

        printf("FMempFree() done.\r\n");
        FMemListAll(&memp);

        FMempRemove(&memp); /* destory memory pool */
    }

    if (ret == 0)
    {
        printf("%s@%d: Memory pool basic example [success].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: Memory pool basic example [failure].\r\n", __func__, __LINE__);
    }

    return ret;
}