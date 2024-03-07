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
 * FilePath: stl_priority_queue.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/* C++ program to demonstrate the use of priority_queue */
#include <iostream>
#include <queue>
using namespace std;

/* driver code */
extern "C" int FStlPriorityQueueExample(void)
{
	int arr[6] = { 10, 2, 4, 8, 6, 9 };

	/* defining priority queue */
	priority_queue<int> pq;

	/* printing array */
	cout << "Array: ";
	for (auto i : arr) {
		cout << i << ' ';
	}
	cout << endl;
	/* pushing array sequentially one by one */
	for (int i = 0; i < 6; i++) {
		pq.push(arr[i]);
	}

	/* printing priority queue */
	cout << "Priority Queue: ";
	while (!pq.empty()) {
		cout << pq.top() << ' ';
		pq.pop();
	}

	return 0;
}
