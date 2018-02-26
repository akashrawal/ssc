/* msg.h
 * Stuff to help serialize message to a series of bytes
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

#include "incl.h"

size_t ssc_msg_count(MmcMsg *msg)
{
	int i;
	size_t res = 1;

	for (i = 0; i < msg->submsgs_len; i++)
		res += ssc_msg_count(msg->submsgs[i]);
	
	return res;
}

void ssc_msg_create_layout(MmcMsg *msg, size_t len, uint32_t *layout)
{
	int i, j;
	MmcMsg **qdata;
	int qlim;
	
	//Initialize queue
	qdata = mmc_alloc(sizeof(MmcMsg) * len);
	qdata[0] = msg;
	qlim = 1;

	//Create layout element for root element
	layout[0] = msg->mem_len;
	if (msg->submsgs_len)
		layout[0] |= SSC_MSG_SUBMSG;

	//Iterate for each message in queue
	for (i = 0; i < len; i++)
	{
		MmcMsg *curmsg = qdata[i];

		//Process all submessages
		for (j = 0; j < curmsg->submsgs_len; j++)
		{
			MmcMsg *submsg = curmsg->submsgs[j];
			uint32_t layout_el;

			//Create layout element for submessage
			layout_el = submsg->mem_len;
			if (submsg->submsgs_len)
				layout_el |= SSC_MSG_SUBMSG;
			if (j < (curmsg->submsgs_len - 1))
				layout_el |= SSC_MSG_SIBLING;

			//Push it onto queue
			qdata[qlim] = submsg;
			layout[qlim] = ssc_uint32_to_le(layout_el);

			qlim++;
		}
	}

	//Free the queue
	free(qdata);
}

MmcMsg *ssc_msg_alloc_by_layout(size_t len, uint32_t *layout)
{
	int i, j;
	MmcMsg **qdata = NULL;
	int qlim;
	MmcMsg *res;

	//Sanity checks
	if (len < 1)
		return NULL;
	if (ssc_uint32_from_le(layout[len - 1]) & SSC_MSG_ALL)
		return NULL;
	
	//Initialize queue
	qdata = mmc_tryalloc(sizeof(MmcMsg) * len);
	if (! qdata)
		return NULL;
	qlim = 1;

	//Imitate tree traversal to find metadata for all nodes
	//Allocate all messages
	for (i = 0; i < qlim; i++)
	{
		size_t mem_len;
		size_t submsgs_len;
		uint32_t layout_el;

		layout_el = ssc_uint32_from_le(layout[i]);

		//Get size of the memory
		mem_len = layout_el & (~SSC_MSG_ALL);

		//Count submessages
		submsgs_len = 0;
		if (layout_el & SSC_MSG_SUBMSG)
		{
			//Queue must never overflow
			if (qlim >= len)
				break;

			int b = qlim;
			while (ssc_uint32_from_le(layout[qlim]) & SSC_MSG_SIBLING)
				qlim++;
			qlim++;
			submsgs_len = qlim - b;
		}

		//Create message
		qdata[i] = mmc_msg_try_newa(mem_len, submsgs_len);
		if (! qdata[i])
			break;
	}

	//If allocation stopped halfway, free all messages and quit
	//Allocation can stop because of failed malloc(),
	//or invalid tree information bits.
	if (i < len)
	{
		for (j = 0; j < i; j++)
			mmc_msg_unref(qdata[j]);
		free(qdata);
		return NULL;
	}

	//Assemble the tree
	qlim = 1;
	for (i = 0; i < len; i++)
	{
		MmcMsg *curmsg = qdata[i];

		for (j = 0; j < curmsg->submsgs_len; j++)
		{
			curmsg->submsgs[j] = qdata[qlim];
			qlim++;
		}
	}

	//Free the queue
	res = qdata[0];
	free(qdata);
	return res;
}

size_t ssc_msg_get_blocks(MmcMsg *msg, size_t len, SscMBlock *data)
{
	int i, j;
	MmcMsg **qdata;
	int qlim;
	int dc;
	
	//Initialize queue
	qdata = mmc_alloc(sizeof(MmcMsg) * len);
	qdata[0] = msg;
	qlim = 1;

	//Initialize output pointer
	dc = 0;

	//Iterate for each message in queue
	for (i = 0; i < len; i++)
	{
		MmcMsg *curmsg = qdata[i];

		//Push submessages onto queue
		for (j = 0; j < curmsg->submsgs_len; j++)
		{
			qdata[qlim] = curmsg->submsgs[j];
			qlim++;
		}

		//Output memory block if large enough
		if (curmsg->mem_len > 0)
		{
			data[dc].mem = curmsg->mem;
			data[dc].len = curmsg->mem_len;
			dc++;
		}
	}

	//Free the queue
	free(qdata);

	return dc;
}



