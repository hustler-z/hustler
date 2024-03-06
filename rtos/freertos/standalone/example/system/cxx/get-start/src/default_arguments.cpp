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
 * FilePath: default_arguments.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/* CPP Program to demonstrate Function overloading in
   Default Arguments */
#include <iostream>
using namespace std;

/* A function with default arguments, it can be called with
   2 arguments or 3 arguments or 4 arguments. */
int sum(int x, int y, int z = 0, int w = 0)
{
	return (x + y + z + w);
}
int sum(int x, int y, float z = 0, float w = 0)
{
	return (x + y + z + w);
}

extern "C" int FDefaultArgumentsExample(void)
{
	cout << sum(10, 15, (float)1.0) << endl;
	cout << sum(10, 15, 25) << endl;
	cout << sum(10, 15, 25, 30) << endl;
	return 0;
}
