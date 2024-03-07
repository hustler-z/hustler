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
 * FilePath: jtag_walk_through_cxx_example.cpp
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:46:03
 * Description:  This file is for jtag cxx debug
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/10/30   first release
 */

/***************************** Include Files *********************************/
#include <iostream>
#include <vector>

static void bubbleSortCXX(std::vector<int> &array)
{
    int size = array.size();

    for (int i = 0; i < size - 1; i++)
    {
        for (int j = 0; j < size - i - 1; j++)
        {
            if (array[j] > array[j + 1])
            {
                std::swap(array[j], array[j + 1]);
            }
        }
    }
}

static void printArrayCXX(const std::vector<int> &array)
{
    for (int i = 0; i < array.size(); i++)
    {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;
}

extern "C" int JtagWalkThroughCXXExample()
{
    std::vector<int> array = {5, 2, 8, 1, 9, 3, 7, 4, 6};

    std::cout << "Original Array: " << std::endl;
    printArrayCXX(array);

    bubbleSortCXX(array);

    std::cout << "Sorted Array: " << std::endl;
    printArrayCXX(array);

    return 0;
}