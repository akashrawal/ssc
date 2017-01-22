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

//Protocol?
//Breadth-first order. Appearantly, that is easier. 

#define SSC_MSG_SUBMSG  (((uint32_t) 1) << 30)
#define SSC_MSG_SIBLING (((uint32_t) 1) << 31)
#define SSC_MSG_ALL     (SSC_MSG_SUBMSG | SSC_MSG_SIBLING)

//Used to create arrays of memory blocks
typedef struct
{
	void *mem;
	size_t len;
} SscMBlock;

size_t ssc_msg_count(MmcMsg *msg);

void ssc_msg_create_layout
	(MmcMsg *msg, size_t len, uint32_t *layout, SscMBlock *data);

MmcMsg *ssc_msg_alloc_by_layout
	(size_t len, uint32_t *layout, SscMBlock *data);


