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
 * FilePath: static_objects.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

#include <string>
#include <iostream>

using namespace std;

class Object {
public:
    int id;
    string name;
    Object(Object &obj)
    {
        id = obj.id + 1;
        name = obj.name;
        cout << "[" << name << "]-" << id << ":Constructor is executed" << endl;
    }
    Object(const char *s, int v)
    {
        name = s;
        id = v;
        cout << "[" << name << "]-" << id << ":Constructor is executed" << endl;
    }
    ~Object() 
    { 
        cout << "[" << name << "]-" << id << ":Destructor is executed" << endl; 
    }
    void Print() const 
    {
        cout << "[" << name << "]-" << id << endl;
    }
};

static Object global_obj_1("global-obj", 1);
static Object global_obj_2(global_obj_1);
extern "C" int FStaticObjectsExample(void)
{
    static Object local_obj_1("local-obj", 1);
    static Object local_obj_2(local_obj_1);

    Object temp_obj("temp-obj", 1);

    global_obj_1.Print();
    global_obj_2.Print();
    local_obj_1.Print();
    local_obj_2.Print();
    temp_obj.Print();
    return 0;
}