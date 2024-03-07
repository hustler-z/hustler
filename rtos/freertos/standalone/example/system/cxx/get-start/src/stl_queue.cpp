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
 * FilePath: stl_queue.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/* CPP code to illustrate Queue in */
/* Standard Template Library (STL) */
#include <iostream>
#include <queue>

using namespace std;

/* Print the queue */
void showq(queue<int> gq)
{
	queue<int> g = gq;
	while (!g.empty()) {
		cout << '\t' << g.front();
		g.pop();
	}
	cout << '\n';
}

/* Driver Code */
extern "C" int FStlQueueExample(void)
{
	queue<int> gquiz;
	gquiz.push(10);
	gquiz.push(20);
	gquiz.push(30);

	cout << "The queue gquiz is : ";
	showq(gquiz);

	cout << "\ngquiz.size() : " << gquiz.size();
	cout << "\ngquiz.front() : " << gquiz.front();
	cout << "\ngquiz.back() : " << gquiz.back();

	cout << "\ngquiz.pop() : ";
	gquiz.pop();
	showq(gquiz);

	return 0;
}
