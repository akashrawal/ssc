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
	int states[64];
	int elements[64];
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
	int mask[64];
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


#if 0
static char* test_strings[] = 
{
	"",
	"aaaa",
	"bbbb"
};

int test_dict_insert()
{
	int ordering[64];
	int states[64];

	int n_strings = sizeof(test_strings) / sizeof(char *);
	int i, j;

	for (i = 0; i < n_strings; i++)
		ordering[i] = i;

	states[0] = -1;	
	i = 0;
	while (i >= 0)
	{
		int lim = n_strings - i;	
		states[i]++;
		
		if (states[i] < lim)
		{
			i++;
			states[i] = -1;
		}
		else
		{
			i--;
			continue;
		}

		int target = i + states[i - 1];
		int tmp = ordering[i - 1];
		ordering[i - 1] = ordering[target];
		ordering[target] = tmp;

		printf("Selection: ");
		for (j = 0; j < i; j++)
		{
			printf("%d ", ordering[j]);
		} 

		printf("\n");
		ordering[target] = ordering[i - 1];
		ordering[i - 1] = tmp;

	}

	return 1;
}
#endif //0

int main()
{
	//run_test(test_dict_insert());

	run_test(test_permutator(2));
	run_test(test_permutator(4));
	
	return 0;
}

