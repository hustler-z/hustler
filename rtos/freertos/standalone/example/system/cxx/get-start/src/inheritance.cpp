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
 * FilePath: inheritance.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

/* Example: define member function without argument within the class */

#include<iostream>
#include<cstring>

using namespace std;

class Person
{
	int id;
	char name[20];
	
	public:
		void set_p()
		{
			cout<<"Enter the Id:"<<endl;
			id = 1;
			cout<<"Enter the Name:"<<endl;
			memcpy(name, "David", strlen("David") + 1);;
		}
	
		void display_p()
		{
			cout<<endl<<id<<"\t"<<name<<"\t";
		}
};

class Student: private Person
{
	char course[20];
	int fee;
	
	public:
	void set_s()
	{
		set_p();
		cout<<"Enter the Course Name:"<<endl;
		memcpy(course, "Programming", strlen("Programming") + 1);
		cout<<"Enter the Course Fee:"<<endl;
		fee = 50;
	}
		
	void display_s()
	{
		display_p();
		cout<<course<<"\t"<<fee<<endl;
	}
};

extern "C" int FInheritanceExample(void)
{
	Student s;
	s.set_s();
	s.display_s();
	return 0;
}
