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
 * FilePath: dynamic_binding.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/* C++ Program to Demonstrate the Concept of Dynamic binding
   with the help of virtual function */
#include <iostream>
using namespace std;
 
class GFG {
public:
    void call_Function() /* function that call print */
    {
        print();
    }
    void print() /* the display function */
    {
        cout << "Printing the Base class Content" << endl;
    }
};
class GFG2 : public GFG /* GFG2 inherit a publicly */
{
public:
    void print() /* GFG2's display */
    {
        cout << "Printing the Derived class Content"
             << endl;
    }
};

extern "C" int FDynamicBindingExample(void)
{
    GFG geeksforgeeks; /* Creating GFG's pbject */
    geeksforgeeks.call_Function(); /* Calling call_Function */ 
    GFG2 geeksforgeeks2; /*creating GFG2 object */ 
    geeksforgeeks2.call_Function(); /* calling call_Function */
                                    /* for GFG2 object */ 
    return 0;
}