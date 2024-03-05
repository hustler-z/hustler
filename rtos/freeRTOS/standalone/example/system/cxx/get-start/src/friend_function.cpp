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
 * FilePath: friend_function.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/* C++ Program to demonstrate the
   functioning of a friend class */ 
#include <iostream>
using namespace std;

class GFG {
private:
	int private_variable;

protected:
	int protected_variable;

public:
	GFG()
	{
		private_variable = 10;
		protected_variable = 99;
	}

	/* friend class declaration */
	friend class F;
};

/* Here, class F is declared as a */
/* friend inside class GFG. Therefore, */
/* F is a friend of class GFG. Class F */
/* can access the private members of */
/* class GFG. */
class F {
public:
	void display(GFG& t)
	{
		cout << "The value of Private Variable = "
			<< t.private_variable << endl;
		cout << "The value of Protected Variable = "
			<< t.protected_variable;
	}
};

/* Driver code */
extern "C" int FFriendFunctionExample(void)
{
	GFG g;
	F fri;
	fri.display(g);
	return 0;
}
