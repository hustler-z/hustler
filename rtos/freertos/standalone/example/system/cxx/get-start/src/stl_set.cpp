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
 * FilePath: stl_set.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/* C++ program to demonstrate various functions of */
/* STL */
#include <iostream>
#include <iterator>
#include <set>
using namespace std;

extern "C" int FStlSetExample(void)
{
	/* empty set container */
	set<int, greater<int> > s1;

	/* insert elements in random order */
	s1.insert(40);
	s1.insert(30);
	s1.insert(60);
	s1.insert(20);
	s1.insert(50);

	/* only one 50 will be added to the set */
	s1.insert(50);
	s1.insert(10);

	/* printing set s1 */
	set<int, greater<int> >::iterator itr;
	cout << "\nThe set s1 is : \n";
	for (itr = s1.begin(); itr != s1.end(); itr++) {
		cout << *itr << " ";
	}
	cout << endl;

	/* assigning the elements from s1 to s2 */
	set<int> s2(s1.begin(), s1.end());

	/* print all elements of the set s2 */
	cout << "\nThe set s2 after assign from s1 is : \n";
	for (itr = s2.begin(); itr != s2.end(); itr++) {
		cout << *itr << " ";
	}
	cout << endl;

	/* remove all elements up to 30 in s2 */
	cout << "\ns2 after removal of elements less than 30 "
			":\n";
	s2.erase(s2.begin(), s2.find(30));
	for (itr = s2.begin(); itr != s2.end(); itr++) {
		cout << *itr << " ";
	}

	/* remove element with value 50 in s2 */
	int num;
	num = s2.erase(50);
	cout << "\ns2.erase(50) : ";
	cout << num << " removed\n";
	for (itr = s2.begin(); itr != s2.end(); itr++) {
		cout << *itr << " ";
	}

	cout << endl;

	/* lower bound and upper bound for set s1 */
	cout << "s1.lower_bound(40) : "
		<< *s1.lower_bound(40) << endl;
	cout << "s1.upper_bound(40) : "
		<< *s1.upper_bound(40) << endl;

	/* lower bound and upper bound for set s2 */
	cout << "s2.lower_bound(40) : "
		<< *s2.lower_bound(40) << endl;
	cout << "s2.upper_bound(40) : "
		<< *s2.upper_bound(40) << endl;

	return 0;
}
