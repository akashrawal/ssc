/* libtest.h
 * Test support library
 * 
 * Copyright 2015-2019 Akash Rawal
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

#include <ssc/ssc.h>

//Caller context for testing
typedef struct
{
	SscCallerCtx parent;
	MmcMsg *reply;
} TestCallerCtx;

void test_caller_ctx_init(TestCallerCtx *ctx);

//Driver for struct-based tests
typedef MmcMsg * (*TestSerializeFn) (void *value);
typedef MdslStatus (*TestDeserializeFn) (MmcMsg *msg, void *value);
typedef int (*TestEqualFn) (void *a, void *b);
typedef void (*TestFreeFn) (void *x);

void test_struct_driver_fn(void *data, size_t stride, size_t len,
		TestSerializeFn serialize_fn, TestDeserializeFn deserialize_fn,
		TestEqualFn equal_fn, TestFreeFn free_fn);

#define test_struct_drive() \
	do { \
		size_t stride = sizeof(testcases[0]); \
		size_t len = sizeof(testcases) / stride; \
		test_struct_driver_fn(testcases, stride, len, \
				(TestSerializeFn) TestStruct__serialize, \
				(TestDeserializeFn) TestStruct__deserialize, \
				(TestEqualFn) TestStruct__equal, \
				(TestFreeFn) TestStruct__free \
				); \
	} while (0)

