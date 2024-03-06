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
 * FilePath: jtag_walk_through_c_example.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for jtag c debug
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/10/30   first release
 */

/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>

#include "ftypes.h"
#include "fparameters.h"
#include "fkernel.h"
#include "fdebug.h"

static void bubbleSort(int array[], int size) 
{
    int i, j, temp;

    for (i = 0; i < size - 1; i++) 
    {
        for (j = 0; j < size - i - 1; j++) 
        {
            if (array[j] > array[j + 1]) 
            {
                temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }

    /* add breakpoint here and modify memory by debugger */
    __asm__ __volatile__(".global JtagPostSort\n JtagPostSort:");
}

static void printArray(int array[], int size) 
{
    for (int i = 0; i < size; i++) 
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}

/******************************************************/

static u32 *gpio_io_reg = (u32 *)(uintptr)FGPIO0_BASE_ADDR;

int JtagTouchRegistersExample(void)
{
    FtDumpHexWord((const u32*)gpio_io_reg, 0x10);

    *(gpio_io_reg + 0x04U) |= BIT(0);
    *(gpio_io_reg + 0x00U) |= BIT(0);

    FtDumpHexWord((const u32*)gpio_io_reg, 0x10);

    /* add breakpoint here and modify register value by debugger */
    __asm__ __volatile__(".global JtagTouchRegisters\n JtagTouchRegisters:");

    FtDumpHexWord((const u32*)gpio_io_reg, 0x10);

    return 0;
}

/******************************************************/

#define MEMORY_BULK_SIZE  32U
static u8 memory_bulk[MEMORY_BULK_SIZE] = {0};

int JtagTouchMemoryExample(void)
{
    u32 loop;

    memset(memory_bulk, 0xff, MEMORY_BULK_SIZE);
    FtDumpHexByte((const u8 *)memory_bulk, MEMORY_BULK_SIZE);

    for (loop = 0; loop < MEMORY_BULK_SIZE; loop++)
    {
        memory_bulk[loop] = loop;
    }

    FtDumpHexByte((const u8 *)memory_bulk, MEMORY_BULK_SIZE);

    /* add breakpoint here and modify memory by debugger */
    __asm__ __volatile__(".global JtagTouchMemory\n JtagTouchMemory:");

    FtDumpHexByte((const u8 *)memory_bulk, MEMORY_BULK_SIZE);

    return 0;
}

int JtagWalkThroughCExample(void) 
{
    int array[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    int size = sizeof(array) / sizeof(array[0]);

    printf("Original Array:\n");
    printArray(array, size);

    bubbleSort(array, size);

    printf("Sorted Array:\n");
    printArray(array, size);

    printf("Touch registers in jtag \r\n");
    JtagTouchRegistersExample();

    printf("Touch memory in jtag \r\n");
    JtagTouchMemoryExample();

    return 0;
}

int JtagUnhandledSoftBrk(void)
{
    printf("Unhandled software breakpoints:\n");
    __asm__ __volatile__ ("udf #1");
    return 0;
}