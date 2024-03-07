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
 * FilePath: cpp_test_example.h
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ example definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

#ifndef CPP_TEST_EXAMPLE_H
#define CPP_TEST_EXAMPLE_H

/* use C linkage to support use C++ method in C */
#ifdef __cplusplus
extern "C" {
#endif

#define RUN_CASE(func, label) \
        if (0 != (ret = func())) { \
            goto label; \
        }

/* C++ Features over C */
int FDefaultArgumentsExample(void);
int FSmartPointerExample(void);
int FFunctorsExample(void);
int FRttiExample(void);
int FNamespaceExample(void);
int FFunctionOverloadingExample(void);
int FOperatorOverloadingExample(void);
int FPassByReferenceExample(void);
int FAutoTypeExample(void);
int FNullptrExample(void);

/* C++ OOP */
int FAbstractionExample(void);
int FInheritanceExample(void);
int FPolymorphismExample(void);
int FDynamicBindingExample(void);
int FStaticObjectsExample(void);
int FFriendFunctionExample(void);
int FStaticMemberExample(void);

/* C++ STL */
int FStlVectorExample(void);
int FStlListExample(void);
int FStlDequeueExample(void);
int FStlQueueExample(void);
int FStlPriorityQueueExample(void);
int FStlStackExample(void);
int FStlSetExample(void);
int FStlMapExample(void);
int FStlHeapExample(void);
int FStlStringExample(void);
int FStlSortExample(void);

#ifdef __cplusplus
}
#endif

#endif