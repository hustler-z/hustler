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
 * FilePath: namespace.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/* A C++ program to show more than one namespaces */
/* with different names. */
#include <iostream>
using namespace std;

/* first name space */
namespace first
{
    int func() { return 5; }
}

/* second name space */
namespace second
{
    int func() { return 10; }
}

extern "C" int FNamespaceExample(void)
{
    /* member function of namespace */
    /* accessed using scope resolution operator */
    cout << first::func() << "\n";
    cout << second::func() << "\n";
    return 0;
}
