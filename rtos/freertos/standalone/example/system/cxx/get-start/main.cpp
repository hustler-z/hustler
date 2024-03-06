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
 * FilePath: main.c
 * Date: 2023-05-25 14:53:41
 * LastEditTime: 2023-05-27 17:36:39
 * Description:  This file is for the c++ test example functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/5/27  first release
 */

#include <iostream>
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell_port.h"
#endif

#include "cpp_test_example.h"

using namespace std;

int main()
{
    int ret = 0;

#ifdef CONFIG_USE_LETTER_SHELL
    /* if shell command is enabled, register example entry as shell command */
    LSUserShellLoop();    
#else

    /* if shell command is not enabled, run example one by one */
    cout << "Hello baremetal C++" << endl;

    /* C++ basic features */
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

    /* C++ OOP features */
    RUN_CASE(FAbstractionExample, failed);
    RUN_CASE(FInheritanceExample, failed);
    RUN_CASE(FPolymorphismExample, failed);
    RUN_CASE(FDynamicBindingExample, failed);
    RUN_CASE(FStaticObjectsExample, failed);
    RUN_CASE(FFriendFunctionExample, failed);
    RUN_CASE(FStaticMemberExample, failed);

    /* C++ STL features */
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

    cout << "C++ test finished !!!" << endl;
failed:
#endif

    return ret;
}