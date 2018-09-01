/* private.h
 * Unit test for library-private stuff
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

#include <ssc/incl.h> //< Private header file

#include <stdarg.h>

#define run_test(x) \
	do { \
		fprintf(stderr, "Running test %s\n", #x); \
		int res = x; \
		if (! res) \
		{ \
			ssc_error("Test %s failed", #x); \
		} \
	} while (0)

static int avl_tree_check_sanity(SscMiniAvlNode *mem, int x, int l, int r)
{
	if (!x)
		return 0;
	if (mem[x].key <= l || mem[x].key >= r)
	{
		ssc_error("Range error on node %d (key %d not in (%d, %d))",
				x, (int) mem[x].key, l, r);
	}
	int lh = avl_tree_check_sanity(mem, mem[x].left, l, mem[x].key);
	int rh = avl_tree_check_sanity(mem, mem[x].right, mem[x].key, r);
	int check_height = (lh > rh ? lh : rh) + 1;	
	if (mem[x].height != check_height)
	{
		ssc_error("Incorrect height of node %d (%d vs %d)", 
				x, (int) mem[x].height, check_height);
		abort();
	}
	int bf = rh - lh;
	if (bf > 1 || bf < -1)
	{
		ssc_error("Unbalanced node %d (lh = %d, rh = %d)",
				x, lh, rh);
		abort();
	}

	return check_height;
}

static int avl_tree_check_correctness
	(SscMiniAvlNode *mem, int root, int *elements)
{
	avl_tree_check_sanity(mem, root, -1, 256);

	int i;

	for (i = 0; i < 256; i++)
	{
		int iter = root;
		while (iter)
		{
			if (i < mem[iter].key)
				iter = mem[iter].left;
			else if (i > mem[iter].key)
				iter = mem[iter].right;
			else
				break;
		}
		if (elements[i] != iter)
		{
			ssc_error("Contents check failed: %d --> %d vs %d", 
					i, elements[i], iter);
		}
	}

	return 0;
}

#define INSERT 1
#define REMOVE 2
#define STOP -1

static int avl_tree_test(int op, ...)
{
	static int elements[256];
	static SscMiniAvlNode mem[257];
	int i;
	int freelist = 1;;

	for (i = 0; i < 256; i++)
	{
		elements[i] = 0;
		mem[i + 1].left = mem[i + 1].right = mem[i + 1].height = 0;
		mem[i + 1].key = i + 2;
	}

	int root = 0;

	va_list arglist;
	va_start(arglist, op);
	while (op >= 0)
	{
		int arg = va_arg(arglist, int);

		ssc_warn("TMP: Doing (%d, %d)", op, arg);

		if (op == INSERT)
		{
			int alloc = freelist;
			freelist = mem[freelist].key;
			int tray = alloc;

			root = ssc_mini_avl_set_rec(mem, root, arg, &tray);
			if (tray != elements[arg])
			{
				ssc_error("Invalid response for set(%d) (%d vs expected %d)",
						arg, tray, elements[arg]);
			}
			if (tray == 0)
			{
				elements[arg] = alloc;
			}
			else
			{
				mem[alloc].key = freelist;
				freelist = alloc;
			}

			avl_tree_check_correctness(mem, root, elements);
		}
		else if (op == REMOVE)
		{
			int tray = 0;

			root = ssc_mini_avl_unset_rec(mem, root, arg, &tray);
			if (tray != elements[arg])
			{
				ssc_error("Invalid response for unset(%d) (%d vs expected %d)",
						arg, tray, elements[arg]);
			}
			if (tray)
			{
				mem[tray].key = freelist;
				freelist = tray;
				elements[arg] = 0;
			}

			avl_tree_check_correctness(mem, root, elements);
			
		}

		op = va_arg(arglist, int);
	}
	va_end(arglist);

	return 1;
}

int main()
{
	run_test(avl_tree_test(INSERT, 0, INSERT, 1, INSERT, 2,
				REMOVE, 1, REMOVE, 2, REMOVE, 0, STOP));	
	run_test(avl_tree_test(REMOVE, 0, INSERT, 0, INSERT, 0,
				REMOVE, 0, REMOVE, 0, STOP));
	
	return 0;
}

