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
 * FilePath: functors.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/* C++ program to demonstrate working of */
/* functors. */
#include <bits/stdc++.h>
using namespace std;

/* A Functor */
class increment
{
private:
	int num;
public:
	increment(int n) : num(n) { }

	/* This operator overloading enables calling */
	/* operator function () on objects of increment */
	int operator () (int arr_num) const 
	{
		return num + arr_num;
	}
};

/* Driver code */
extern "C" int FFunctorsExample(void)
{
	int arr[] = {1, 2, 3, 4, 5};
	int n = sizeof(arr)/sizeof(arr[0]);
	int to_add = 5;

	transform(arr, arr+n, arr, increment(to_add));

	for (int i=0; i<n; i++)
		cout << arr[i] << " ";

    return 0;
}
