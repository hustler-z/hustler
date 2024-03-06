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
 * FilePath: stl_string.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/* C++ program to print all permutations with */
/* duplicates allowed using rotate() in STL */
#include <bits/stdc++.h>
using namespace std;

/* Function to print permutations of string str, */
/* out is used to store permutations one by one */
void permute(string str, string out)
{
	/* When size of str becomes 0, out has a */
	/* permutation (length of out is n) */
	if (str.size() == 0)
	{
		cout << out << endl;
		return;
	}

	/* One be one move all characters at */
	/* the beginning of out (or result) */
	for (int i = 0; i < str.size(); i++)
	{
		/* Remove first character from str and */
		/* add it to out */
		permute(str.substr(1), out + str[0]);

		/* Rotate string in a way second character */
		/* moves to the beginning. */
		rotate(str.begin(), str.begin() + 1, str.end());
	}
}

/* Driver code */
extern "C" int FStlStringExample(void)
{
	string str = "ABC";
	permute(str, "");
	return 0;
}
