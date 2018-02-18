/* interface.h
 * Stuff dedicated to make interfaces work
 * 
 * Copyright 2015 Akash Rawal
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
 
 
//Messages with a prefixed integer
int ssc_read_prefix(MmcMsg *msg);
MmcMsg *ssc_create_prefixed_empty_msg(uint8_t prefix);
//TODO: decide whether use this macro in above functions. 
#define SSC_PREFIX_SIZE 1

//Type for servant-side stubs
typedef MmcStatus (* SscReadMsgFn) (MmcMsg *msg, void *args);
typedef MmcMsg*  (* SscCreateReplyFn) (void *out_args);
typedef void (* SscArgsFreeFn) (void *args);
typedef struct
{
	size_t args_size;
	SscReadMsgFn read_msg;
	SscCreateReplyFn create_reply;
	SscArgsFreeFn in_args_free;
} SscSStub;

//Base type for skeleton objects
typedef struct
{
	int n_fns;
	SscSStub *sstubs;
} SscSkel;

//Caller context (for returning)
typedef struct _SscCallerCtx SscCallerCtx;
typedef void (*SscReturnFn)(SscCallerCtx *ctx, MmcMsg *reply_msg);
struct _SscCallerCtx
{
	int method_id;
	SscReturnFn return_fn;
};

//Servant type 
typedef struct _SscServant SscServant;
typedef void (* SscImplFn) 
	(SscServant *servant, SscCallerCtx *ctx, void *args);

struct _SscServant
{
	MmcRC parent;
	const SscSkel *skel;
	
	void *user_data;
	SscImplFn impl[]; 
};

mmc_rc_declare(SscServant, ssc_servant);

/**Creates a new servant using given skeleton.
 * \param skel Skeleton of the interface to expose
 * \return A newly created servant. Remember to add implementations.
 */
SscServant *ssc_servant_new(const SscSkel *skel);

/**Sends a message to the servant.
 * The servant will deserialize the message with the skeleton and call the 
 * correct implementation function. 
 * (If that fails an error message is sent back)
 * \param servant The servant to dispatch the message to.
 * \param msg The message.\
 * \param ctx Caller context,
 *            assign the callback function and pass the struct here.
 */
void ssc_servant_call(SscServant *servant, MmcMsg *msg, SscCallerCtx *ctx);

/**Returns the reply from the implementation. Used by implementation.
 * \param servant The servant that received message
 * \param ctx Caller context received by the implementation when receiving a 
 *            message.
 * \param args Pointer to struct containing return arguments, or NULL if 
 *             there aren't any.
 */
void ssc_servant_return(SscServant *servant, SscCallerCtx *ctx, void *args);
