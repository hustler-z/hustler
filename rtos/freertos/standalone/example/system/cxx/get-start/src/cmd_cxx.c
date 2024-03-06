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
 * FilePath: cmd_cxx.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for c++ example function implmentation
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

#include "cpp_test_example.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

/* usage info function for cxx example */
static void FCxxExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("cxx basic\r\n");
    printf("-- Demonstrate advance feature of c++ over c\r\n");
    printf("cxx oop\r\n");
    printf("-- Demonstrate oop related feature of c++\r\n");
    printf("cxx stl\r\n");
    printf("-- Demonstrate stl related feature of c++\n");
}

/* entry function for adc example */
static int FCxxExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        FCxxExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "basic"))
    {
        RUN_CASE(FDefaultArgumentsExample, failed);
        RUN_CASE(FSmartPointerExample, failed);
        RUN_CASE(FFunctorsExample, failed);
        RUN_CASE(FRttiExample, failed);
        RUN_CASE(FNamespaceExample, failed);
        RUN_CASE(FFunctionOverloadingExample, failed);
        RUN_CASE(FOperatorOverloadingExample, failed);
        RUN_CASE(FPassByReferenceExample, failed);
        RUN_CASE(FAutoTypeExample, failed);
        RUN_CASE(FNullptrExample, failed);
    }
    else if (!strcmp(argv[1], "oop"))
    {
        RUN_CASE(FAbstractionExample, failed);
        RUN_CASE(FInheritanceExample, failed);
        RUN_CASE(FPolymorphismExample, failed);
        RUN_CASE(FDynamicBindingExample, failed);
        RUN_CASE(FStaticObjectsExample, failed);
        RUN_CASE(FFriendFunctionExample, failed);
        RUN_CASE(FStaticMemberExample, failed);
    }
    else if (!strcmp(argv[1], "stl"))
    {
        RUN_CASE(FStlVectorExample, failed);
        RUN_CASE(FStlListExample, failed);
        RUN_CASE(FStlDequeueExample, failed);
        RUN_CASE(FStlQueueExample, failed);
        RUN_CASE(FStlPriorityQueueExample, failed);
        RUN_CASE(FStlStackExample, failed);
        RUN_CASE(FStlSetExample, failed);
        RUN_CASE(FStlMapExample, failed);
        RUN_CASE(FStlHeapExample, failed);
        RUN_CASE(FStlStringExample, failed);
        RUN_CASE(FStlSortExample, failed);
    }

    printf("\r\n%s@%d: cxx example [success].\r\n", __func__, __LINE__);
    return ret;
failed:
    printf("\r\n%s@%d: cxx example [failure].\r\n", __func__, __LINE__);
    return ret;
}


/* register command for cxx example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), cxx, FCxxExampleEntry, cxx example);
#endif