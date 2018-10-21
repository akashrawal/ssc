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

#include <string.h>

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

static inline void array_swap(int *arr, int i, int j)
{
	int tmp = arr[i];
	arr[i] = arr[j];
	arr[j] = tmp;
}

//Iterates over all permutations
typedef struct 
{
	int n, i;
	int states[16];
	int elements[16];
} Permutator;

void permutator_init(Permutator *p, int n)
{
	p->n = n;
	p->i = 0;
	p->states[0] = -1;

	int i;
	for (i = 0; i < n; i++)
		p->elements[i] = i;
}

int permutator_next(Permutator *p)
{
	while (p->i >= 0)
	{
		int lim = p->n - p->i;	
		p->states[p->i]++;
		
		if (p->states[p->i] < lim)
		{
			array_swap(p->elements, p->i, p->i + p->states[p->i]);
			p->i++;
			p->states[p->i] = -1;
			if (p->i >= p->n)
				return 1;
		}
		else
		{
			p->i--;
			array_swap(p->elements, p->i, p->i + p->states[p->i]);
		}
	}
	return 0;
}

int test_permutator(int n)
{
	int *logs;
	int n_elements;
	int i, j, k;

	n_elements = 1;
	for (i = 2; i <= n; i++)
		n_elements *= i;

	logs = (int *) mmc_alloc(sizeof(int) * n * n_elements);

	Permutator p[1];

	permutator_init(p, n);

	i = 0;
	int mask[16];
	while (permutator_next(p))
	{
		ssc_assert((i/n) < n_elements, "Overflow");

		//No digits should repeat
		for (j = 0; j < n; j++)
			mask[j] = -1;
		for (j = 0; j < n; j++)
		{
			ssc_assert (p->elements[j] >= 0 && p->elements[j] < n,
					"Out of range");
			mask[p->elements[j]] = j;
		}
		for (j = 0; j < n; j++)
			ssc_assert(mask[j] >= 0, "Number %d not used", j);

		//Copy number
		for (j = 0; j < n; j++)
			logs[i + j] = p->elements[j];

		//Print number
		fprintf(stderr, "  ");
		for (j = 0; j < n; j++)
			fprintf(stderr, " %d", p->elements[j]);
		fprintf(stderr, "\n");

		i += n;
	}
	ssc_assert((i / n) == n_elements, "Incorrect number of elements");

	//Check for duplicates
	for (i = 0; i < n_elements; i++)
	{
		int *na = logs + (i * n);
		for (j = i + 1; j < n_elements; j++)
		{
			int *nb = logs + (j * n);
			for (k = 0; k < n; k++)
			{
				if (na[k] != nb[k])
					break;
			}
			ssc_assert(k < n, "Values %d and %d are equal", i, j);
		}
	}

	free(logs);

	return 1;
}


static char* test_strings_1[] = 
{
	"",
	"aaaa",
	"bbbb",
	NULL
};

static char* test_strings_2[] = 
{
	"aaa",
	"aab",
	"abb",
	"bbb",
	NULL
};

static char* test_strings_3[] = 
{
	"aaaa",
	"aabb",
	"bbaa",
	"bbbb",
	NULL
};

static char* test_strings_4[] = 
{
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
	"aaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbb",
	"bbbbbbbbbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaaaaaaaaaa",
	"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
	NULL
};

int test_dict_insert(char** strings)
{
	Permutator p[1];

	int n_strings;
	for (n_strings = 0; strings[n_strings]; n_strings++)
		;

	permutator_init(p, n_strings);

	while (permutator_next(p))
	{
		SscDict *dict = ssc_dict_new();

		int i, j;
		for (i = 0; i < n_strings; i++)
		{
			char *key = strings[p->elements[i]];
			ssc_dict_set(dict, key, strlen(key), key);

			for (j = 0; j <= i; j++)
			{
				char *ckey = strings[p->elements[j]];
				char *cckey = ssc_dict_get(dict, ckey, strlen(ckey));
				ssc_assert(ckey == cckey,
						"Wrong answer, j = %d, ckey=%s, cckey=%p",
						j, ckey, cckey);
			}
		}

		ssc_dict_unref(dict);
	}

	return 1;
}

static char *test_strings_u[] = 
{
	"aaa",
	"aa",
	"aaaa",
	"bbb",
	NULL
};

int test_dict_insert_delete(char** strings, ...)
{
	int i, j;
	char* check_ds[64];
	int n_strings;
	for (n_strings = 0; strings[n_strings]; n_strings++)
		;

	//Initialize both dictionary and check data structure
	SscDict *dict = ssc_dict_new();
	for (i = 0; i < n_strings; i++)
		check_ds[i] = NULL;

	va_list arglist;
	va_start(arglist, strings);

	const char *cmdstr;
	for (i = 0; (cmdstr = va_arg(arglist, const char *)); i++)
	{
		int strnum = atoi(cmdstr + 1);
		char* str = strings[strnum];
		char* rhs;
		if (cmdstr[0] == 'I')
			rhs = str;
		else if (cmdstr[0] == 'D')
			rhs = NULL;
		else
			ssc_error("Bad command %c", cmdstr[0]);

		ssc_debug("set %s --> %p", str, rhs);

		char *r = ssc_dict_set(dict, str, strlen(str), rhs);
		char *check_r = check_ds[strnum];
		check_ds[strnum] = rhs;
		ssc_assert(r == check_r, "Wrong answer, i=%d, str=%s", i, str);

		ssc_dict_dump(dict);
		for (j = 0; j < n_strings; j++)
		{
			r = ssc_dict_get(dict, strings[j], strlen(strings[j]));
			check_r = check_ds[j];
			ssc_assert(r == check_r,
					"Inconsistent datastructures, i=%d, j=%d", i, j);
		}
	}

	ssc_dict_unref(dict);

	return 1;
}

int main()
{

#if 0
	run_test(test_permutator(2));
	run_test(test_permutator(4));
#endif

	run_test(test_dict_insert(test_strings_1));
	run_test(test_dict_insert(test_strings_2));
	run_test(test_dict_insert(test_strings_3));
	run_test(test_dict_insert(test_strings_4));

	run_test(test_dict_insert_delete(test_strings_u,
				"I0", "D1", "D2", "D3", "D0", NULL));
	run_test(test_dict_insert_delete(test_strings_u,
				"I0", "I2", "D2", "D0", NULL));
	run_test(test_dict_insert_delete(test_strings_u,
				"I0", "I2", "D0", "D2", NULL));
	
	return 0;
}

