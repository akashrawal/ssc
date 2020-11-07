/* main.c
 * Test an interface with one function
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


void test_iface_impl
	(SscServant *servant, MmcReplier *replier, int method_id, void *argp,
	 void *user_data)
{
	switch(method_id)
	{
	case TestIface__increment__ID:
		{
			TestIface__increment__in_args *args = argp;
			TestIface__increment__out_args  out_args;

			out_args.out = args->in + 1;

			ssc_servant_return(servant, method_id, replier, &out_args);
		}
		break;
	case TestIface__decrement__ID:
		{
			TestIface__decrement__in_args *args = argp;
			TestIface__decrement__out_args  out_args;

			out_args.out = args->in - 1;

			ssc_servant_return(servant, method_id, replier, &out_args);
		}
		break;
	}
}


int main()
{
	//Create servant
	SscServant *servant = ssc_servant_new(TestIface, test_iface_impl, NULL);

	TEST_CALL(TestIface, increment, 
		{ in_args.in = 1; }, { ssc_assert(out_args.out == 2, "Test failed"); });

	TEST_CALL(TestIface, decrement, 
		{ in_args.in = 2; }, { ssc_assert(out_args.out == 1, "Test failed"); });

	//Garbage collect
	mmc_servant_unref((MmcServant *) servant);
	
	return 0;
}
