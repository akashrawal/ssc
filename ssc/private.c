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


static inline int avl_do_hbf(MiniAvlTreeNode *mem, int x)
{
	int l = mem[x].left;
	int lh = l ? mem[l].height : 0;
	int r = mem[x].right;
	int rh = r ? mem[r].height : 0;
	mem[x].height = (lh > rh ? lh : rh) + 1;

	return rh - lh;
}

static inline int avl_rotate_l(MiniAvlTreeNode *mem, int x)
{
	int r = mem[x].right;
	int b = mem[r].left;

	mem[r].left = x;
	mem[x].right = b;

	avl_do_hbf(mem, x);
	avl_do_hbf(mem, r);

	return r;
}

static inline int avl_rotate_r(MiniAvlTreeNode *mem, int x)
{
	int r = mem[x].left;
	int b = mem[r].right;

	mem[r].right = x;
	mem[x].left = b;

	avl_do_hbf(mem, x);
	avl_do_hbf(mem, r);

	return r;
}

static inline int avl_balance1(MiniAvlTreeNode *mem, int x)
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

int ssc_mini_avl_set_rec(MiniAvlTreeNode *mem, int x, int key, int *tray)
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

static int avl_remove_min(MiniAvlTreeNode *mem, int x, int *tray)
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

int ssc_mini_avl_unset_rec(MiniAvlTreeNode *mem, int x, int key, int *tray)
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




//We use hash + AVL trees for sizes: 4, 8, 16, 32, 64.
//We use DIT for size 256.

#define SIZED_MAP(size) \
typedef struct \
{ \
	uint8_t lim, freelist; \
	uint8_t htable[size]; \
	MiniAvlTreeNode avl_nodes[size]; \
} SizedMap ## size; \

