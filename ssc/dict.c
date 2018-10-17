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

#define MAX_NODE_LEN 32

typedef struct 
{
	void *value;
	SscByteMap next;
	uint8_t len;
	uint8_t ekey[];
} DictNode;

struct _SscDict
{
	MmcRC parent;
	DictNode *root;
};


mmc_rc_define(SscDict, ssc_dict);

mmc_declare_array(DictNode *, DictNodeArray, dict_node_array);

static DictNode *alloc_node(const uint8_t *ekey, size_t len)
{
	DictNode *dn = (DictNode *) mmc_alloc(sizeof(DictNode) + len);
	int i;
	for (i = 0; i < len; i++)
		dn->ekey[i] = ekey[i];
	dn->len = len;
	dn->value = NULL;
	ssc_byte_map_init(&(dn->next));

	return dn;
}

static DictNode *create_node
	(SscDict *dict, const void *key, size_t key_len)
{
	const uint8_t *ekey = (const uint8_t *) key;

	//Find the node to graft a branch
	DictNode *target_ptr_node = NULL;
	int target_ptr_chr = 0;
	DictNode *target = dict->root;
	int target_offset = 0;

	int ekey_offset = 0;

	while (target && ekey_offset < key_len)
	{
		int i;
		int remains = key_len - ekey_offset;
		if (target->len >= remains)
		{
			for (i = 0; i < remains; i++)
			{
				if (target->ekey[i] != ekey[ekey_offset + i])
					break;
			}
		}
		else
		{
			for (i = 0; i < target->len; i++)
			{
				if (target->ekey[i] != ekey[ekey_offset + i])
					break;
			}
			if (i == target->len)
			{
				DictNode *next = ssc_byte_map_get
					(&(target->next), ekey[target->len]);
				if (next)
				{
					target_ptr_node = target;
					target_ptr_chr = ekey[target->len];
					target = next;
					ekey_offset += target->len + 1;
					continue;
				}
			}
		}
		
		target_offset += i;
		ekey_offset += i;
		break;
	}
	
	//Graft the starting point for the new branch
	DictNode *start_node;
	if (target)
	{
		if (target_offset == target->len)
		{
			start_node = target;
		}
		else
		{
			//Splice target, second part --> start_node
			DictNode *p1 = alloc_node(target->ekey, target_offset);
			DictNode *p2 = alloc_node(target->ekey + target_offset + 1, 
					target->len - target_offset - 1);
			ssc_byte_map_set(&(p1->next), target->ekey[target_offset], p2);
			start_node = p1;
			p2->next = target->next;
			p2->value = target->value;
			if (target_ptr_node)
				ssc_byte_map_set(&(target_ptr_node->next), target_ptr_chr, p1);
			else
				dict->root = p1;
			start_node = p1;
			free(target);
		}
	}
	else
	{
		ssc_assert(target_offset == 0, "Assertion failure");
		start_node = NULL;	
	}

	//Grow the new branch

	if (start_node == NULL)
	{
		int run_len = key_len - ekey_offset;
		if (run_len > MAX_NODE_LEN)
			run_len = MAX_NODE_LEN;
		
		start_node = alloc_node(ekey + ekey_offset, run_len);
		ssc_assert(dict->root == NULL, "Assertion failure");
		dict->root = start_node;
		ekey_offset += run_len;
	}

	DictNode *res = start_node;

	while (ekey_offset < key_len)
	{
		int run_len = key_len - ekey_offset;
		if (run_len > MAX_NODE_LEN + 1)
			run_len = MAX_NODE_LEN + 1;

		DictNode *ext = alloc_node(ekey + ekey_offset + 1, run_len - 1);
		ssc_byte_map_set(&(res->next), ekey[ekey_offset], ext);
		res = ext;
		ekey_offset += run_len;
	}

	return res;
}


void *ssc_dict_set
	(SscDict *dict, const void *key, size_t key_len, void *value)
{
	//TODO: deletion

	DictNode *node = create_node(dict, key, key_len);
	void *old_value = node->value;
	node->value = value;
	return old_value;
}



static void ssc_dict_destroy(SscDict *dict)
{
	DictNodeArray stack[1];

	dict_node_array_init(stack);

	if (dict->root)
		dict_node_array_append(stack, dict->root);

	while (dict_node_array_size(stack) > 0)
	{
		DictNode *node = dict_node_array_pop(stack);

		uint8_t keys[256];
		DictNode *values[256];
		int n_tuples = ssc_byte_map_get_tuples
			(&(node->next), keys, (void **) values);

		int i;
		for (i = 0; i < n_tuples; i++)
			dict_node_array_append(stack, values[i]);
	}

	free(stack->data);
}

SscDict *ssc_dict_new()
{
	SscDict *dict = (SscDict *) mmc_alloc(sizeof(SscDict));

	mmc_rc_init(dict);

	dict->root = NULL;

	return dict;
}


