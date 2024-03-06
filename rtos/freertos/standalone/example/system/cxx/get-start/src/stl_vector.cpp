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
 * FilePath: stl_vector.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/* C++ program to illustrate the */
/* Modifiers in vector */
#include <bits/stdc++.h>
#include <vector>
using namespace std;

extern "C" int FStlVectorExample(void)
{
	/* Assign vector */
	vector<int> v;

	/* fill the vector with 10 five times */
	v.assign(5, 10);

	cout << "The vector elements are: ";
	for (int i = 0; i < v.size(); i++)
		cout << v[i] << " ";

	/* inserts 15 to the last position */
	v.push_back(15);
	int n = v.size();
	cout << "\nThe last element is: " << v[n - 1];

	/* removes last element */
	v.pop_back();

	/* prints the vector */
	cout << "\nThe vector elements are: ";
	for (int i = 0; i < v.size(); i++)
		cout << v[i] << " ";

	/* inserts 5 at the beginning */
	v.insert(v.begin(), 5);

	cout << "\nThe first element is: " << v[0];

	/* removes the first element */
	v.erase(v.begin());

	cout << "\nThe first element is: " << v[0];

	/* inserts at the beginning */
	v.emplace(v.begin(), 5);
	cout << "\nThe first element is: " << v[0];

	/* Inserts 20 at the end */
	v.emplace_back(20);
	n = v.size();
	cout << "\nThe last element is: " << v[n - 1];

	/* erases the vector */
	v.clear();
	cout << "\nVector size after erase(): " << v.size();

	/* two vector to perform swap */
	vector<int> v1, v2;
	v1.push_back(1);
	v1.push_back(2);
	v2.push_back(3);
	v2.push_back(4);

	cout << "\n\nVector 1: ";
	for (int i = 0; i < v1.size(); i++)
		cout << v1[i] << " ";

	cout << "\nVector 2: ";
	for (int i = 0; i < v2.size(); i++)
		cout << v2[i] << " ";

	/* Swaps v1 and v2 */
	v1.swap(v2);

	cout << "\nAfter Swap \nVector 1: ";
	for (int i = 0; i < v1.size(); i++)
		cout << v1[i] << " ";

	cout << "\nVector 2: ";
	for (int i = 0; i < v2.size(); i++)
		cout << v2[i] << " ";

	return 0;
}

/* C++ program to demonstrate the use of push_heap() */
/* function */
void print(vector<int> &vc) {
	for (auto i : vc) {
		cout << i << " ";
	}
	cout << endl;
}

extern "C" int FStlHeapExample(void)
{
	vector <int> vc{20,30,40,10};

	make_heap(vc.begin(), vc.end());
	cout << "Initial Heap: ";
	print(vc);

	vc.push_back(50);
	cout << "Heap just after push_back(): ";
	print(vc);
	push_heap(vc.begin(), vc.end());
	cout << "Heap after push_heap(): ";
	print(vc);

	return 0;
}
