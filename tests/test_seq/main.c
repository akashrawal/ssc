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

int32_t array[] = {0, 1, 2, 3, 4, 5};
TestStruct testcases[] = 
{
	{{array, 2}},
	{{array + 2, 3}},
	{{NULL, 0}},
	{{array, 0}}
};

int TestStruct__equal(TestStruct *a, TestStruct *b)
{
	if (a->s.len != b->s.len)
		return 0;
	else
	{
		int i;
		for (i = 0; i < a->s.len; i++)
		{
			if (a->s.data[i] != b->s.data[i])
				return 0;
		}
	}
	
	return 1;
}

int main()
{
	test_struct_drive();
	return 0;
}
