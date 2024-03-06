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
 * FilePath: polymorphism.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/* C++ program to demonstrate */
/* function overloading or */
/* Compile-time Polymorphism */
#include <bits/stdc++.h>

using namespace std;
class Geeks {
public:
	/* Function with 1 int parameter */
	void func(int x)
	{
		cout << "value of x is " << x << endl;
	}

	/* Function with same name but */
	/* 1 double parameter */
	void func(double x)
	{
		cout << "value of x is " << x << endl;
	}

	/* Function with same name and */
	/* 2 int parameters */
	void func(int x, int y)
	{
		cout << "value of x and y is " << x << ", " << y
			<< endl;
	}
};

/* Driver code */
extern "C" int FPolymorphismExample(void)
{
	Geeks obj1;

	/* Function being called depends */
	/* on the parameters passed */
	/* func() is called with int value */
	obj1.func(7);

	/* func() is called with double value */
	obj1.func(9.132);

	/* func() is called with 2 int values */
	obj1.func(85, 64);
	return 0;
}
