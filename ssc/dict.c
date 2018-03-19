/* dict.h
 * Fast dictionary implementation
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

//Implementation is by a radix tree.
#if 0
typedef struct _DictNode DictNode;
struct _DictNode
{
	void *data;
	DictNode *sub[2];
	//Label size in number of bits
	uint8_t label_bitsize;
	//Label, upto 255 bits (between 31 bytes and 32 bytes)
	uint8_t label[]; 
};


struct _SscDict
{
	DictNode start[1];
};



static DictNode *add_node(SscDict *dict, const void *key, size_t key_len)
{
	size_t key_bitlen = key_len * 8;
	DictNode *cur = dict->start;
	size_t cur_bitoffset = 0;

	while (cur_bitoffset < key_bitlen)
	{
		size_t label_match_bitlen = _bitwise_match
			(key, key_bitlen, cur_bitoffset, cur->label, cur->label_bitsize);

		if (label_match_bitlen < cur->label_bitsize)
		{
			cur = split_node(cur, label_
		}
	}

	ssc_assert(cur_bitoffset > key_bitlen, "Assertion failure");

	return cur;
}

void *ssc_dict_set
	(SscDict *dict, const void *key, size_t key_len, void *data)
{
	
	return NULL;
}

static void ssc_dict_destroy(SscDict *dict)
{
}

mmc_rc_define(SscDict, ssc_dict);
#endif
