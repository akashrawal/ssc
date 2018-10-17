/* private.c
 * Library-private stuff
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


static inline int avl_do_hbf(SscMiniAvlNode *mem, int x)
{
	int l = mem[x].left;
	int lh = l ? mem[l].height : 0;
	int r = mem[x].right;
	int rh = r ? mem[r].height : 0;
	mem[x].height = (lh > rh ? lh : rh) + 1;

	return rh - lh;
}

static inline int avl_rotate_l(SscMiniAvlNode *mem, int x)
{
	int r = mem[x].right;
	int b = mem[r].left;

	mem[r].left = x;
	mem[x].right = b;

	avl_do_hbf(mem, x);
	avl_do_hbf(mem, r);

	return r;
}

static inline int avl_rotate_r(SscMiniAvlNode *mem, int x)
{
	int r = mem[x].left;
	int b = mem[r].right;

	mem[r].right = x;
	mem[x].left = b;

	avl_do_hbf(mem, x);
	avl_do_hbf(mem, r);

	return r;
}

static inline int avl_balance1(SscMiniAvlNode *mem, int x)
{
	int bf = avl_do_hbf(mem, x);

	if (bf < -1)
	{
		if (avl_do_hbf(mem, mem[x].left) > 0)
			mem[x].left = avl_rotate_l(mem, mem[x].left);
		x = avl_rotate_r(mem, x);
	}
	else if (bf > 1)
	{
		if (avl_do_hbf(mem, mem[x].right) < 0)
			mem[x].right = avl_rotate_r(mem, mem[x].right);
		x = avl_rotate_l(mem, x);
	}

	return x;
}

int ssc_mini_avl_set_rec(SscMiniAvlNode *mem, int x, int key, int *tray)
{
	if (!x)
	{
		x = *tray;
		*tray = 0;
		mem[x].key = key;
		mem[x].left = mem[x].right = 0;
		mem[x].height = 1;
	}
	else if (key < mem[x].key)
	{
		mem[x].left = ssc_mini_avl_set_rec(mem, mem[x].left, key, tray);
		x = avl_balance1(mem, x);
	}
	else if (key > mem[x].key)
	{
		mem[x].right = ssc_mini_avl_set_rec(mem, mem[x].right, key, tray);
		x = avl_balance1(mem, x);
	}
	else
	{
		*tray = x;
	}
	
	return x;
}

static int avl_remove_min(SscMiniAvlNode *mem, int x, int *tray)
{
	if (mem[x].left)
	{
		mem[x].left = avl_remove_min(mem, mem[x].left, tray);
		return avl_balance1(mem, x);
	}
	else
	{
		*tray = x;
		return mem[x].right;
	}
}

int ssc_mini_avl_unset_rec(SscMiniAvlNode *mem, int x, int key, int *tray)
{
	if (!x)
	{
		return 0;
	}
	if (key < mem[x].key)
	{
		mem[x].left = ssc_mini_avl_unset_rec(mem, mem[x].left, key, tray);
		return avl_balance1(mem, x);
	}
	else if (key > mem[x].key)
	{
		mem[x].right = ssc_mini_avl_unset_rec(mem, mem[x].right, key, tray);
		return avl_balance1(mem, x);
	}
	else
	{
		*tray = x;
		if (mem[x].right)
		{
			int new_x = 0;
			mem[x].right = avl_remove_min(mem, mem[x].right, &new_x);
			mem[new_x].left = mem[x].left;
			mem[new_x].right = mem[x].right;
			new_x = avl_balance1(mem, new_x);
			return new_x;
		}
		else
		{
			return mem[x].left;	
		}
	}
}




//We use hash table + AVL trees for sizes: 5, 11, 23, 59
//We use DIT for size 256.

typedef struct
{
	uint8_t freelist;
} SizedMap;

typedef struct
{
	void **values;
	uint8_t *htable;
	SscMiniAvlNode *avl_nodes;
} SizedMapExt;

static inline size_t align_offset(size_t x)
{
	size_t ap = sizeof(void *) * 2; //< This is debated
	if (x % ap)
		x += ap - (x % ap);
	return x;
}

static inline SizedMap *sized_map_ext
	(int size, SizedMap *ds, SizedMapExt *dse)
{
	size_t values_offset = align_offset(sizeof(SizedMap));
	size_t htable_offset = align_offset(values_offset + sizeof(void *) * size);
	size_t avl_nodes_offset = align_offset(htable_offset + size);
	size_t alloc_size = avl_nodes_offset + sizeof(SscMiniAvlNode) * size;

	char *mem;
	if (!ds)
		ds = mmc_alloc(alloc_size);
	mem = (char *) ds;

	dse->values = (void **) (mem + values_offset);
	dse->htable = (uint8_t *) (mem + htable_offset);
	dse->avl_nodes = (SscMiniAvlNode *) (mem + avl_nodes_offset);

	return ds;
}

static SizedMap *sized_map_new(size_t size)
{
	SizedMap *ds;
	SizedMapExt dse;

	ds = sized_map_ext(size, NULL, &dse);

	int i;
	for (i = 0; i < size; i++)
	{
		dse.htable[i] = 0;
		dse.avl_nodes[i].key = i + 2;
		dse.values[i] = NULL;
	}
	dse.avl_nodes[size - 1].key = 0;
	ds->freelist = 1;
	return ds;
}

static int sized_map_insert(size_t size, SizedMap *ds, int key, void *value) 
{ 
	SizedMapExt dse;
	sized_map_ext(size, ds, &dse);

	int alloc, tray; 
	alloc = ds->freelist; 
	if (!alloc) 
		return -1; 
	ds->freelist = dse.avl_nodes[ds->freelist - 1].key; 
	tray = alloc; 
	dse.htable[key % size] = ssc_mini_avl_set_rec 
		(dse.avl_nodes - 1, dse.htable[key % size], key, &tray); 
	if (tray) 
	{ 
		dse.avl_nodes[alloc - 1].key = ds->freelist; 
		ds->freelist = alloc; 
		alloc = tray; 
	} 
	dse.values[alloc - 1] = value; 
	return tray ? 0 : 1; 
} 

static int sized_map_delete(size_t size, SizedMap *ds, int key) 
{ 
	SizedMapExt dse;
	sized_map_ext(size, ds, &dse);

	int tray = 0; 
	dse.htable[key % size] = ssc_mini_avl_unset_rec 
		(dse.avl_nodes - 1, dse.htable[key % size], key, &tray); 
	if (tray) 
	{ 
		dse.avl_nodes[tray - 1].key = ds->freelist; 
		ds->freelist = tray; 
		dse.values[tray - 1] = NULL; 
		return 1; 
	} 
	else 
	{ 
		return 0; 
	} 
} 

static void *sized_map_get(size_t size, SizedMap *ds, int key) 
{
	SizedMapExt dse;
	sized_map_ext(size, ds, &dse);

	int idx = dse.htable[key % size];
	while (idx)
	{
		if (key < dse.avl_nodes[idx - 1].key)
			idx = dse.avl_nodes[idx - 1].left;
		else if (key > dse.avl_nodes[idx - 1].key)
			idx = dse.avl_nodes[idx - 1].right;
		else
			return dse.values[idx - 1];
	}
	return NULL;
}

static void sized_map_transfer
	(size_t size, SizedMap *ds, size_t nsize, SizedMap *nds)
{
	SizedMapExt dse;
	sized_map_ext(size, ds, &dse);
	int i;
	for (i = 0; i < size; i++)
	{
		if (dse.values[i])
		{
			int key = dse.avl_nodes[i].key;
			void *value = dse.values[i];
			sized_map_insert(nsize, nds, key, value);
		}
	}
}

void sized_map_to_dit(size_t size, SizedMap *ds, void **dit)
{
	SizedMapExt dse;
	sized_map_ext(size, ds, &dse);
	int i; 
	for (i = 0; i < 256; i++)
	{
		dit[i] = NULL;
	}
	for (i = 0; i < size; i++)
	{
		if (dse.values[i])
			dit[dse.avl_nodes[i].key] = dse.values[i];
	}
}

void sized_map_from_dit(size_t size, SizedMap *ds, void **dit)
{
	int i; 
	for (i = 0; i < 256; i++)
	{
		if (dit[i])
			sized_map_insert(size, ds, i, dit[i]);
	}
}

int sized_map_get_tuples
	(size_t size, SizedMap *ds, uint8_t *keys, void **values)
{
	SizedMapExt dse;
	sized_map_ext(size, ds, &dse);
	int i, j;
	for (i = 0; i < size; i++)
	{
		if (dse.values[i])
		{
			keys[j] = dse.avl_nodes[i].key;
			values[j] = dse.values[i];
			j++;
		}
	}
	return j;
}

static const int mode_table[] = {0, 0, 5, 11, 23, 59, 256, -1};

void ssc_byte_map_init(SscByteMap *m)
{
	m->ptr = NULL;
	m->metainf = 0;
}

void ssc_byte_map_set(SscByteMap *m, uint8_t key, void *value)
{
	int mode = m->metainf % 16;
	int sec = m->metainf / 16;

	int asize = mode_table[mode];

	if (value)
	{
		if (mode == 0)
		{
			mode = 1;
			sec = key;
			m->ptr = value;
		}
		else if (mode == 1)
		{
			if (sec == key)
			{
				m->ptr = value;
			}
			else
			{
				int nasize = mode_table[mode + 1];
				void *value0 = m->ptr;
				m->ptr = sized_map_new(nasize);
				SizedMap *ds = (SizedMap *) m->ptr;
				sized_map_insert(nasize, ds, sec, value0);
				sized_map_insert(nasize, ds, key, value);
				sec = 2;
				mode = 2;
			}
		}
		else if (asize == 256)
		{
			void **dit = (void **) m->ptr;
			if (!dit[key])
				sec++;
			dit[key] = value;
		}
		else
		{
			SizedMap *ds = m->ptr;
			int res = sized_map_insert(asize, ds, key, value);
			if (res == 1)
			{
				sec++;
			}
			else if (res == -1)
			{
				int nasize = mode_table[mode + 1];
				if (nasize == 256)
				{
					void **dit = malloc(sizeof(void *) * 256);
					sized_map_to_dit(asize, ds, dit);
					free(ds);
					m->ptr = dit;
					dit[key] = value;
				}
				else
				{
					SizedMap *nds = sized_map_new(nasize);
					sized_map_transfer(asize, ds, nasize, nds);
					free(ds);
					m->ptr = nds;
					sized_map_insert(nasize, nds, key, value);
				}
				mode++;
				sec++;
			}
		}
	}
	else
	{
		if (mode == 0)
		{
			//Nothing to delete?
		}
		else if (mode == 1)
		{
			if (sec == key)
			{
				m->ptr = NULL;
				mode = 0;
				sec = 0;
			}
		}
		else if (asize == 256)
		{
			void **dit = (void **) m->ptr;
			if (dit[key])
			{
				int nasize = mode_table[mode - 2];
				sec--;
				dit[key] = NULL;
				if (sec <= nasize)
				{
					SizedMap *nds = sized_map_new(nasize);
					sized_map_from_dit(nasize, nds, dit);
					free(dit);
					m->ptr = nds;
					mode -= 2;
				}
			}
		}
		else
		{
			SizedMap *ds = m->ptr;
			int res = sized_map_delete(asize, ds, key);
			if (res == 1)
			{
				int nasize = mode_table[mode - 2];
				sec--;
				if (sec <= nasize)
				{
					if (nasize == 0)
					{
						mode = 0;
						sec = 0;
						free(ds);
						m->ptr = NULL;
					}
					else
					{
						SizedMap *nds = sized_map_new(nasize);
						sized_map_transfer(asize, ds, nasize, nds);
						free(ds);
						m->ptr = nds;
						mode -= 2;
					}
				}
			}
		}
	}

	m->metainf = mode | sec * 16;
}

void *ssc_byte_map_get(SscByteMap *m, uint8_t key)
{
	int mode = m->metainf % 16;
	int sec = m->metainf / 16;
	int asize = mode_table[mode];

	if (mode == 0)
	{
		return NULL;
	}
	else if (mode == 1)
	{
		if (sec == key)
			return m->ptr;
		return NULL;
	}
	else if (asize == 256)
	{
		void **dit = (void **) m->ptr;
		return dit[key];
	}
	else
	{
		SizedMap *ds = m->ptr;

		return sized_map_get(asize, ds, key);
	}
}

void ssc_byte_map_clear(SscByteMap *m)
{
	int mode = m->metainf % 16;
	if (mode >= 2)
	{
		if (m->ptr)
			free(m->ptr);
	}
}

int ssc_byte_map_get_size(SscByteMap *m)
{
	int mode = m->metainf % 16;
	int sec = m->metainf / 16;
	if (mode >= 2)
		return sec;
	else
		return mode;
}

int ssc_byte_map_get_tuples(SscByteMap *m, uint8_t *keys, void **values)
{
	int mode = m->metainf % 16;
	int sec = m->metainf / 16;
	int asize = mode_table[mode];

	if (mode == 0)
	{
		return 0;
	}
	else if (mode == 1)
	{
		keys[0] = sec;
		values[0] = m->ptr;
		return 1;
	}
	else if (asize == 256)
	{
		void **dit = (void **) m->ptr;
		int i, j;
		j = 0;
		for (i = 0; i < 256; i++)
		{
			if (dit[i])
			{
				keys[j] = i;
				values[j] = dit[i];
			}
		}

		ssc_assert(j == sec, "Assertion failure");

		return j;
	}
	else
	{
		SizedMap *ds = m->ptr;
		return sized_map_get_tuples(asize, ds, keys, values);
	}
}
