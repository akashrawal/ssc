/* main.c
 * Just a prototype for WYSIWYG feel
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
	TestIface__increment__in_args *args = argp;
	TestIface__increment__out_args  out_args;

	out_args.out = args->in + 1;

	ssc_servant_return(servant, method_id, replier, &out_args);
}


void test_call(SscServant *servant, int val)
{
	TestReplier replier;
	TestIface__increment__in_args in_args;
	TestIface__increment__out_args out_args;

	//Initialize caller context
	test_replier_init(&replier);

	//Call the 'remote' procedure
	in_args.in = val;
	MmcMsg *msg = TestIface__increment__create_msg(&in_args);
	mmc_servant_call((MmcServant *) servant, msg, (MmcReplier *) &replier);
	mmc_msg_unref(msg);

	//Deserialize the reply returned
	if (TestIface__increment__read_reply(replier.reply, &out_args) == MDSL_FAILURE)
		ssc_error("Failed to deserialize the reply");
	mmc_msg_unref(replier.reply);

	//Verify the reply
	if (out_args.out != val + 1)
		ssc_error("Invalid reply fron servant, f(%d) = %d",
			val, (int)out_args.out);
}

int main()
{
	//Create servant
	SscServant *servant = ssc_servant_new(TestIface, test_iface_impl, NULL);

	//Test calls
	test_call(servant, 1);

	//Garbage collect
	mmc_servant_unref((MmcServant *) servant);
	
	return 0;
}
