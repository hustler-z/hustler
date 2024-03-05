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
 * FilePath: rtti.cpp
 * Date: 2022-02-25 14:53:42
 * LastEditTime: 2022-02-28 17:46:03
 * Description:  This file is for c++ feature test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/6/2   first release
 */

#include <iostream>
#include <typeinfo>

class IInterface
{
public:
	virtual ~IInterface() {}
	virtual void f(){};
};

class Impl1 : public IInterface
{
};

class Impl2 : public IInterface
{
};

template <typename T>
class Number
{
	T value;

public:
	Number(T n)
	{
		value = n;
	}
	T get_value()
	{
		return value;
	};
};

extern "C" int FRttiExample(void)
{
	/* builtins */
	char c, *p;
	int i;
	long l;
	float f;
	double d;

	printf("--- builtins ---\n");
	printf("char's name: %s\n", typeid(c).name());
	printf("char *'s name: %s\n", typeid(p).name());
	printf("int's name: %s\n", typeid(i).name());
	printf("long's name: %s\n", typeid(l).name());
	printf("float's name: %s\n", typeid(f).name());
	printf("double's name: %s\n", typeid(d).name());

	/* classes */
	IInterface ib, *ptr;
	Impl1 i1;
	Impl2 i2;

	printf("--- polymorphic types ---\n");
	ptr = &ib;
	printf("*ptr(ib)'s name: %s\n", typeid(*ptr).name());
	ptr = &i1;
	printf("*ptr(i1)'s name: %s\n", typeid(*ptr).name());
	ptr = &i2;
	printf("*ptr(i2)'s name: %s\n", typeid(*ptr).name());

	Impl1 *ip1 = dynamic_cast<Impl1 *>(ptr);
	if (ip1)
	{
		printf("Impl2 ptr did cast to Impl1 ptr\n");
	}
	else
	{
		printf("Impl2 ptr did not cast to Impl1 ptr\n");
	}

	Impl2 *ip2 = dynamic_cast<Impl2 *>(ptr);
	if (ip2)
	{
		printf("Impl2 ptr did cast to Impl2 ptr\n");
	}
	else
	{
		printf("Impl2 ptr did not cast to Impl2 ptr\n");
	}

	if (typeid(IInterface) == typeid(Impl1))
	{
		printf("Impl1 is the same as IInterface\n");
	}
	else
	{
		printf("Impl1 is not the same as IInterface\n");
	}
	if (typeid(Impl2) == typeid(Impl1))
	{
		printf("Impl1 is the same as Impl2\n");
	}
	else
	{
		printf("Impl1 is not the same as Impl2\n");
	}

	/* templates */
	Number<int> ni(10);
	Number<double> nd(3.14);

	printf("--- templates ---\n");
	printf("ni's name: %s\n", typeid(ni).name());
	printf("nd's name: %s\n", typeid(nd).name());
	if (typeid(Number<double>) == typeid(Number<int>))
	{
		printf("Number<double> is the same as Number<int>\n");
	}
	else
	{
		printf("Number<double> is not the same as Number<int>\n");
	}

	return 0;
}

/* C++ program to demonstrate working of auto */
/* and type inference */

using namespace std;

extern "C" int FAutoTypeExample(void)
{
	/* auto a; this line will give error */
	/* because 'a' is not initialized at */
	/* the time of declaration */
	/* a=33; */

	/* see here x ,y,ptr are */
	/* initialised at the time of */
	/* declaration hence there is */
	/* no error in them */
	auto x = 4;
	auto y = 3.37;
	auto z = 3.37f;
	auto c = 'a';
	auto ptr = &x;
	auto pptr = &ptr; /* pointer to a pointer */
	cout << typeid(x).name() << endl
		 << typeid(y).name() << endl
		 << typeid(z).name() << endl
		 << typeid(c).name() << endl
		 << typeid(ptr).name() << endl
		 << typeid(pptr).name() << endl;

	return 0;
}
