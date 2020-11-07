/* libtest.c
 * Test support library
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

#include "libtest.h"

static void test_replier_return_fn(MmcReplier *p_replier, MmcMsg *reply)
{
	TestReplier *replier = (TestReplier *) p_replier;
	mmc_msg_ref(reply);
	replier->reply = reply;
}

void test_replier_init(TestReplier *replier)
{
	memset(replier, 0, sizeof(TestReplier));
	replier->parent.call = test_replier_return_fn;
}

void test_struct_driver_fn(void *data, size_t stride, size_t len,
		TestSerializeFn serialize_fn, TestDeserializeFn deserialize_fn,
		TestEqualFn equal_fn, TestFreeFn free_fn)
{
	size_t i;
	void *test_data = mdsl_alloc(stride);
	for (i = 0; i < len; i++)
	{
		fprintf(stderr, "Testcase %zu\n", i);

		void *cur_data = MDSL_PTR_ADD(data, stride * i);
		memset(test_data, 0, stride);

		MmcMsg *msg = (*serialize_fn)(cur_data);
		if ((*deserialize_fn)(msg, test_data) != MDSL_SUCCESS)
		{
			fprintf(stderr, "  Deserialize failed.\n");
			abort();
		}
		mmc_msg_unref(msg);
		if (! (*equal_fn)(cur_data, test_data))
		{
			fprintf(stderr, "  Equal failed.\n");
			abort();
		}
		(* free_fn)(test_data);
	}
	free(test_data);
}
