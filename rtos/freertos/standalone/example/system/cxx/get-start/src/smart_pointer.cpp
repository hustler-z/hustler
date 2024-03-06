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
 * FilePath: smart_pointer.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/* C++ program to demonstrate the working of Smart Pointer */
#include <iostream>
using namespace std;

class SmartPtr {
	int* ptr; /* Actual pointer */
public:
	/* Constructor: Refer https: www.geeksforgeeks.org/g-fact-93/ */
	/* for use of explicit keyword */
	explicit SmartPtr(int* p = NULL) 
    { 
        ptr = p; 
        cout << "allocate ptr " << ptr << endl;
    }

	/* Destructor */
	~SmartPtr() 
    { 
        delete (ptr); 
        cout << "free ptr " << ptr << endl;
    }

	/* Overloading dereferencing operator */
	int& operator*() { return *ptr; }
};

extern "C" int FSmartPointerExample(void)
{
	SmartPtr ptr(new int());
	*ptr = 20;
	cout << "smart ptr 0x" << &ptr << " = " << *ptr << endl;

	/* We don't need to call delete ptr: when the object */
	/* ptr goes out of scope, the destructor for it is automatically */
	/* called and destructor does delete ptr. */

	return 0;
}
