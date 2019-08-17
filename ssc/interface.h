/* interface.h
 * Stuff dedicated to make interfaces work
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
 
 
//Messages with a prefixed integer
int ssc_read_prefix(MmcMsg *msg);
MmcMsg *ssc_create_prefixed_empty_msg(uint8_t prefix);
//TODO: decide whether use this macro in above functions. 
#define SSC_PREFIX_SIZE 1

//Type for servant-side stubs
typedef MdslStatus (* SscReadMsgFn) (MmcMsg *msg, void *args);
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

//Servant type 
typedef struct _SscServant SscServant;
typedef void (* SscImplFn) 
	(SscServant *servant, MmcReplier *replier, int method_id, void *args);

struct _SscServant
{
	MmcServant parent;

	const SscSkel *skel;

	SscImplFn impl; 
	void *user_data;
};

/**Creates a new servant using given skeleton.
 * \param skel Skeleton of the interface to expose
 * \param impl Implementation function.
 * \param user_data User data to be passed to implementation function.
 * \return A newly created servant.
 */
SscServant *ssc_servant_new(const SscSkel *skel,
		SscImplFn impl, void *user_data);

/**Serializes the reply to method call and sends it back.
 * This is a convenience function for implementation to reply to 
 * a method call.
 * \param servant The servant that received message
 * \param method_id Method number for method that was called.
 * \param replier Callback object for replying to the message.
 * \param args Pointer to struct containing return arguments, or NULL if 
 *             there aren't any.
 */
void ssc_servant_return(SscServant *servant, 
		int method_id, MmcReplier *replier, void *args);
