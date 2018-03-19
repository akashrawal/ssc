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

#define run_test(x) \
	do { \
		fprintf(stderr, "Running test %s\n", #x); \
		x; \
	} while (0)

void *generate_pattern
	(uint16_t template, size_t bitstart, size_t bitlen, size_t inversion)
{
	uint8_t *ar;
	int i;

	ssc_assert(bitlen >= inversion, "Assertion failure");
	inversion += bitstart;

	//Rotate template, so that bitstart works
	{
		size_t r = bitstart % 16;
		if (r)
			template = template >> r | template << (16 - r);
	}

	//Allocate pattern
	size_t alloc_size = (bitstart + bitlen) / 8 + 1;
	ar = mmc_alloc(alloc_size);

	for (i = 0; i < alloc_size; i++)
	{
		uint8_t byte = template >> ((i & 1) * 8);

		//Handle inversion
		if (i > inversion / 8)
			byte = ~byte;
		else if (i == inversion / 8)
			byte ^= 0xff >> (inversion - (i * 8));

		//Copy to pattern
		ar[i] = byte;
	}

	return ar;
}

void test_bitwise_match_case
	(size_t a_bitstart, size_t b_bitstart, size_t bitlen, size_t inversion)
{
	uint16_t templates[] = { 0x5555, 0x3333, 0x0f0f, 0x00ff };
	int i;

	fprintf(stderr, "  Testcase(%zu, %zu, %zu, %zu)\n",
			a_bitstart, b_bitstart, bitlen, inversion);

	for (i = 0; i < sizeof(templates) / sizeof(templates[0]); i++)
	{
		fprintf(stderr, "    pattern %d\n", i);

		void *a = generate_pattern
			(templates[i], a_bitstart, bitlen, bitlen);
		void *b = generate_pattern
			(templates[i], b_bitstart, bitlen, inversion);

		size_t res = bitwise_match(a, a_bitstart, b, b_bitstart, bitlen);
		ssc_assert(res == inversion, "Test failed, res = %zu, inversion = %zu",
				res, inversion);

		free(a);
		free(b);
	}
}

void test_bitwise_match()
{
	size_t steps[] = { 0, 1, 7, 8, 9, 15 }, i, j;
	size_t inversion, bitlen, a_bitstart, b_bitstart;
	size_t steps_len = sizeof(steps) / sizeof(steps[0]);
	for (bitlen = 0; bitlen <= 9; bitlen++)
	{
		for (inversion = 0; inversion <= bitlen; inversion++)
		{
			for (i = 0; i < steps_len; i++)
			{
				for (j = 0; j < steps_len; j++)
				{
					a_bitstart = steps[i];
					b_bitstart = steps[j];
					test_bitwise_match_case
						(a_bitstart, b_bitstart, bitlen, inversion);
				}
			}
		}
	}
}

int main()
{
	run_test(test_bitwise_match());
	
	return 0;
}

