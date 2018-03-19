/* private.h
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

size_t bitwise_match
	(void *a, size_t a_bitstart, void *b, size_t b_bitstart, size_t bitlen)
{
	uint8_t *ar = (uint8_t *) a;
	uint8_t *br = (uint8_t *) b;
	size_t res = 0;

	//Shift ar and br to simplify algorithm
	{
		size_t d;

		d = a_bitstart / 8;
		ar += d;
		a_bitstart -= d * 8;

		d = b_bitstart / 8;
		br += d;
		b_bitstart -= d * 8;
	}
	
	//Compare whole bytes
	while (bitlen >= 8)
	{
		uint8_t as = (ar[0] << a_bitstart) | (ar[1] >> (8 - a_bitstart));
		uint8_t bs = (br[0] << b_bitstart) | (br[1] >> (8 - b_bitstart));

		if (as != bs)
			break;
		
		bitlen -= 8;
		ar++;
		br++;
		res += 8;
	}

	//Now, either bitlen < 8 (and ar and br are suitably adjusted)
	//or first element of ar and br differ.
	//In either case, we only need to check first octet.
	if (bitlen)
	{
		//Load atleast what is needed.
		uint8_t as = ar[0] << a_bitstart;
		if (bitlen > (8 - a_bitstart))
			as |= ar[1] >> (8 - a_bitstart);
		uint8_t bs = br[0] << b_bitstart;
		if (bitlen > (8 - b_bitstart))
			bs |= br[1] >> (8 - b_bitstart);

		//Compute difference
		uint8_t diff = as ^ bs;
		if (bitlen < 8)
			diff |= 0xff >> bitlen;

		//MSB
		res += 7;
		while (diff >>= 1)
			res--;
	}

	return res;
}
