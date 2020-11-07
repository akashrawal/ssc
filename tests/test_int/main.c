/* main.c
 * Integer test
 * 
 * Copyright 2015-2020 Akash Rawal
 * This file is part of Modular Middleware.
 * 
 * Modular Middleware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Modular Middleware is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Modular Middleware.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <tests/libtest.h>
#include "idl.h"

TestStruct testcases[] = 
{
	{
		0, 
		0, 
		0, 
		0, 
		0, 
		0, 
		0, 
		0
	},
	{
		UINT8_MAX,
		UINT16_MAX,
		UINT32_MAX,
		UINT64_MAX,
		INT8_MAX,
		INT16_MAX,
		INT32_MAX,
		INT64_MAX
	},
	{
		0, 
		0, 
		0, 
		0, 
		INT8_MIN,
		INT16_MIN,
		INT32_MIN,
		INT64_MIN
	}
};

int TestStruct__equal(TestStruct *a, TestStruct *b)
{
	return (a->t1 == b->t1)
		&& (a->t2 == b->t2)
		&& (a->t3 == b->t3)
		&& (a->t4 == b->t4)
		&& (a->t5 == b->t5)
		&& (a->t6 == b->t6)
		&& (a->t7 == b->t7)
		&& (a->t8 == b->t8);
}

int main()
{
	test_struct_drive();
	return 0;
}
