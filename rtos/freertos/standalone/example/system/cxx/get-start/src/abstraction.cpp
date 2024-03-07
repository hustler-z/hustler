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
 * FilePath: abstraction.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/*  C++ Program to Demonstrate the
	working of Abstraction */
#include <iostream>
using namespace std;

class implementAbstraction {
private:
	int a, b;

public:
	/* method to set values of
	   private members */
	void set(int x, int y)
	{
		a = x;
		b = y;
	}

	void display()
	{
		cout << "a = " << a << endl;
		cout << "b = " << b << endl;
	}
};

extern "C" int FAbstractionExample(void)
{
	implementAbstraction obj;
	obj.set(10, 20);
	obj.display();
	return 0;
}
