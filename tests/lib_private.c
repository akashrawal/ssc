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

#define avl_l1(op, base) op, (base), op, ((base) + 1)
#define avl_l2(op, base) avl_l1(op, base), avl_l1(op, (base) + 2)
#define avl_l3(op, base) avl_l2(op, base), avl_l2(op, (base) + 4)
#define avl_l4(op, base) avl_l3(op, base), avl_l3(op, (base) + 8)
#define avl_l5(op, base) avl_l4(op, base), avl_l4(op, (base) + 16)
#define avl_l6(op, base) avl_l5(op, base), avl_l5(op, (base) + 32)
#define avl_l7(op, base) avl_l6(op, base), avl_l6(op, (base) + 64)


int byte_map_test(int start, int len, int stride)
{
	void *cdata[256];
	uint8_t targets[256];
	int i, j, k;
	for (i = 0; i < 256; i++)
	{
		targets[i] = i;
		cdata[i] = NULL;
	}

	SscByteMap m[1];
	ssc_byte_map_init(m);

	for (k = 0; k < 2; k++)
	{
		for (i = 0; i < len; i++)
		{
			int onekey = (start + i * stride) % 256;
			void *oneval = k ? NULL : targets + onekey;

			ssc_byte_map_set(m, onekey, oneval);
			cdata[onekey] = oneval;

			for (j = 0; j < 256; j++)
			{
				void *val = ssc_byte_map_get(m, j);
				void *cval = cdata[j];
				if (val != cval)
				{
					ssc_error("Inconsistent values "
						"(k = %d, onekey = %d, j = %d, val = %p, cval = %p)", 
						k, onekey, j, val, cval);
				}
			}
		}
	}

	ssc_byte_map_clear(m);

	return 1;
}


int main()
{
	run_test(avl_tree_test(INSERT, 0, INSERT, 1, INSERT, 2,
				REMOVE, 1, REMOVE, 2, REMOVE, 0, STOP));	
	run_test(avl_tree_test(REMOVE, 0, INSERT, 0, INSERT, 0,
				REMOVE, 0, REMOVE, 0, STOP));
	run_test(avl_tree_test(REMOVE, 255, INSERT, 255, INSERT, 255,
				REMOVE, 255, REMOVE, 255, STOP));
	run_test(avl_tree_test(avl_l7(INSERT, 0), avl_l7(INSERT, 128), 
				avl_l7(REMOVE, 0), avl_l7(REMOVE, 128), STOP));

	run_test(byte_map_test(0, 2, 1));
	run_test(byte_map_test(0, 256, 1));
	run_test(byte_map_test(0, 5, 5));
	run_test(byte_map_test(0, 5, 11));
	run_test(byte_map_test(0, 5, 23));
	run_test(byte_map_test(0, 5, 59));
	run_test(byte_map_test(0, 50, 5));
	run_test(byte_map_test(0, 50, 11));
	run_test(byte_map_test(0, 50, 23));
	run_test(byte_map_test(0, 50, 59));
	
	return 0;
}

