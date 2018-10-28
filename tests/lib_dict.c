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

static char *test_strings_5[] = 
{
	"aaa",
	"aa",
	"aaaa",
	"bbb",
	NULL
};

typedef struct
{
	SscDict *dict;
	int n_strings;
	char **strings;
	char *check_ds[64];
} DictChecker;

void dict_checker_init(DictChecker *c, char** strings)
{
	for (c->n_strings = 0; strings[c->n_strings]; c->n_strings++)
		;
	c->strings = strings;
	c->dict = ssc_dict_new();

	int i;
	for (i = 0; i < c->n_strings; i++)
		c->check_ds[i] = NULL;
}

void dict_checker_destroy(DictChecker *c)
{
	ssc_dict_unref(c->dict);
}

void dict_checker_check(DictChecker *c, char op, int idx)
{
	int i;
	char *str = c->strings[idx];
	char *rhs = (op == 'I') ? str : NULL;

	FILE *stream;

	size_t isize = 0;
	char* istate = NULL;
	stream = open_memstream(&istate, &isize);
	ssc_dict_fdump(c->dict, stream);
	fclose(stream);

	char *res = ssc_dict_set(c->dict, str, strlen(str), rhs);
	char *cres = c->check_ds[idx];
	c->check_ds[idx] = rhs;

	size_t fsize = 0;
	char* fstate = NULL;
	stream = open_memstream(&fstate, &fsize);
	ssc_dict_fdump(c->dict, stream);
	fclose(stream);

	ssc_assert(res == cres,
			"Wrong answer, str=%s, rhs=%p, res=%p, cres=%p, "
			"istate:\n%s\n"
			"fstate:\n%s\n",
			str, rhs, res, cres, istate, fstate);
	
	for (i = 0; i < c->n_strings; i++)
	{
		res = ssc_dict_get(c->dict, c->strings[i], strlen(c->strings[i]));
		ssc_assert(res == c->check_ds[i],
				"Inconsistent datastructures, i=%d, res=%p, cres=%p"
				"istate:\n%s\n"
				"fstate:\n%s\n",
				i, res, c->check_ds[i], istate, fstate);
	}

	free(istate);
	free(fstate);
}

int test_dict_insert(char** strings)
{
	int n_strings;
	for (n_strings = 0; strings[n_strings]; n_strings++)
		;

	Permutator p[1];
	permutator_init(p, n_strings);

	while (permutator_next(p))
	{
		DictChecker c[1];

		dict_checker_init(c, strings);

		int i;
		for (i = 0; i < n_strings; i++)
			dict_checker_check(c, 'I', p->elements[i]);

		dict_checker_destroy(c);
	}

	return 1;
}


int test_dict_insert_delete(char** strings, int n_ops)
{
	int i;
	int n_strings;
	for (n_strings = 0; strings[n_strings]; n_strings++)
		;

	uint64_t lim = 1;
	for (i = 0; i < n_ops; i++)
	{
		uint64_t c1 = lim;
		lim *= (n_strings * 2);
		uint64_t c2 = lim;
		c2 /= (n_strings * 2);
		ssc_assert(c1 == c2, "Overflow detected, too many test cases");
	}

	uint64_t cid;
	for (cid = 0; cid < lim; cid++)
	{
		uint64_t opbuf = cid;

		DictChecker c[1];
		dict_checker_init(c, strings);

		for (i = 0; i < n_ops; i++)
		{
			int op = opbuf % 2;
			opbuf /= 2;
			int strid = opbuf % n_strings;
			opbuf /= n_strings;

			dict_checker_check(c, op ? 'D' : 'I', strid);
		}

		dict_checker_destroy(c);
	}
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

	run_test(test_dict_insert_delete(test_strings_5, 4));
	run_test(test_dict_insert_delete(test_strings_4, 4));
	
	return 0;
}

