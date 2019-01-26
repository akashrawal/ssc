/* interface.h
 * Stuff dedicated to make interfaces work
 * 
 * Copyright 2015-2018 Akash Rawal
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

#include "incl.h"


//Messages with a prefixed integer
int ssc_read_prefix(MmcMsg *msg)
{
	int res = -1;
	
	if (msg->mem_len >= 1)
		res = *((uint8_t *) msg->mem);
		
	return res;
}

MmcMsg *ssc_create_prefixed_empty_msg(uint8_t prefix)
{
	MmcMsg *msg;
	
	msg = mmc_msg_newa(1, 0);
	*((uint8_t *) msg->mem) = prefix;
	
	return msg;
}

//Servant type 

void ssc_servant_call(SscServant *servant, MmcMsg *msg, SscCallerCtx *ctx)
{
	int id;
	void *args = NULL;
	SscSStub *sstub;
	MmcMsg *reply_msg = NULL;
	
	//Find which function
	id = ssc_read_prefix(msg);
	if (id < 0 || id >= servant->skel->n_fns)
		goto fail;
	sstub = servant->skel->sstubs + id;
	
	//Assertion (should be removed later)
	if (sstub->args_size)
	{
		if (! sstub->read_msg)
			ssc_error("Assertion failure "
				"(! sstub->read_msg)");
	}
	else
	{
		if (sstub->read_msg || sstub->in_args_free)
			ssc_error("Assertion failure "
				"(sstub->read_msg || sstub->in_args_free)");
	}
	
	//Deserialize the arguments
	if (sstub->args_size)
	{
		args = mdsl_tryalloc(sstub->args_size);
		if (!args)
			goto fail;
		if ((* sstub->read_msg)(msg, args) != MDSL_SUCCESS)
			goto fail;
	}
	
	//Call the function
	//TODO: Return error for unimplemented functions.
	(* servant->impl[id])(servant, ctx, args);
	
	//Free arguments
	if (sstub->in_args_free)
		(* sstub->in_args_free) (args);
	if (args)
		free(args);
	
	return;
	
	
fail:
	//Send error message back
	reply_msg = ssc_create_prefixed_empty_msg(1);
	(* ctx->return_fn)(ctx, reply_msg);
	mmc_msg_unref(reply_msg);
	
	if (args)
		free(args);
}

void ssc_servant_return(SscServant *servant, SscCallerCtx *ctx, void *args)
{
	MmcMsg *reply_msg = (* servant->skel->sstubs[ctx->method_id].create_reply)
		(args);
	(* ctx->return_fn)(ctx, reply_msg);
	mmc_msg_unref(reply_msg);
}

static void ssc_servant_destroy(SscServant *servant)
{
	free(servant);	
}

SscServant *ssc_servant_new(const SscSkel *skel)
{
	SscServant *servant;
	int i;
	
	servant = (SscServant *) mdsl_alloc
		(sizeof(SscServant) + (sizeof(void *) * skel->n_fns));

	mdsl_rc_init(servant);
	
	servant->skel = skel;
	servant->user_data = NULL;
	for (i = 0; i < skel->n_fns; i++)
		servant->impl[i] = NULL;
	
	return servant;
}

mdsl_rc_define(SscServant, ssc_servant);

