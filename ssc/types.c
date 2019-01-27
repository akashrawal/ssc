/* types.c
 * Making data portable
 * 
 * Copyright 2015-2019 Akash Rawal
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
/* ********************************************************************/ 
 
/* NOTE:
 * 1. Integers will be in little endian byte order
 * 2. Signed integers will be represented in two's complement form.
 */

#include "incl.h"

#include <limits.h>
#include <math.h>


//Returns a 32-bit integer representing the given float value.
uint32_t ssc_float_to_flt32(float v)
{
	int float_type;
	uint32_t res;
	
	//Find the type of floating point
	float_type = fpclassify(v); 
	
	//Check for NaN
	if (float_type == FP_NAN)
	{
		res = 0x7fffffff;
	}
	//Zero
	else if (float_type == FP_ZERO)
	{
		res = 0x00000000;
	}
	else
	{
		int sign;
		
		//Extract sign
		sign = signbit(v);
		if (sign)
			sign = 1;
		
		//+/- infinity
		if (float_type == FP_INFINITE)
		{
			res = 0x7f800000;
		}
		else
		{
			//Normal number
			int exponent;
			float fraction;
			
			//Decompose the number
			fraction = frexpf(v, &exponent);
			
			
			//Approximate to infinity in case number is too large
			if (exponent > 128)
			{
				res = 0x7f800000;
			}
			else
			{
				//Make fraction positive
				if (sign)
					fraction = -fraction;
				
				//Extract the mantissa
				if (exponent < -125)
				{
					//Subnormal
					fraction *= (1 << (24 + exponent + 125));
					res = fraction;
					//Exponent area is already zero, no need to attach it
				}
				else
				{
					//Normal
					fraction *= (1 << 24);
					res = (uint32_t) fraction;
					//Remove the preceeding 1
					res &= 0x007fffff;
					//Attach the exponent
					res |= ((exponent + 126) << 23);
				}
			}
		}
		
		//Add sign bit
		res |= (sign << 31);
	}
	
	//Change byte order and return
	return res;
}

//Retrieves float value from 32-bit integer. 
float ssc_float_from_flt32(uint32_t v)
{
	float res;
	uint32_t exp_region, mantissa;
	int sign;
	int exponent;
	
	//Dissect it
	sign = (v & 0x80000000) ? -1 : 1;
	exp_region = v & 0x7f800000;
	mantissa = v & 0x007fffff;
	
	//Infinity and NaN
	if (exp_region == 0x7f800000)
	{
		if (mantissa)
		{
			//NaN
#ifdef NAN
			return NAN;
#else
			//Is this portable?
			return 0.0 / 0.0;
#endif
		}
		else
		{
			//infinity
			return sign * HUGE_VALF;
		}
	}
	//Subnormal numbers
	else if (! exp_region)
	{
		res = ldexpf((float) mantissa, -149);
		res *= sign;
	}
	else
	{
		//Find mantissa and exponent
		exponent = exp_region >> 23;
		mantissa |= 1 << 23;
		//Assemble them together
		res = ldexpf((float) mantissa, exponent - 150);
		res *= sign;
	}
	
	//return
	return res;
}

//Returns a 32-bit integer representing the given double value.
uint32_t ssc_double_to_flt32(double v)
{
	int float_type;
	uint32_t res;
	
	//Find the type of floating point
	float_type = fpclassify(v); 
	
	//Check for NaN
	if (float_type == FP_NAN)
	{
		res = 0x7fffffff;
	}
	//Zero
	else if (float_type == FP_ZERO)
	{
		res = 0x00000000;
	}
	else
	{
		int sign;
		
		//Extract sign
		sign = signbit(v);
		if (sign)
			sign = 1;
		
		//+/- infinity
		if (float_type == FP_INFINITE)
		{
			res = 0x7f800000;
		}
		else
		{
			//Normal number
			int exponent;
			double fraction;
			
			//Decompose the number
			fraction = frexp(v, &exponent);
			
			
			//Approximate to infinity in case number is too large
			if (exponent > 128)
			{
				res = 0x7f800000;
			}
			else
			{
				//Make fraction positive
				if (sign)
					fraction = -fraction;
				
				//Extract the mantissa
				if (exponent < -125)
				{
					//Subnormal
					fraction *= (1 << (24 + exponent + 125));
					res = fraction;
					//Exponent area is already zero, no need to attach it
				}
				else
				{
					//Normal
					fraction *= (1 << 24);
					res = (uint32_t) fraction;
					//Remove the preceeding 1
					res &= 0x007fffff;
					//Attach the exponent
					res |= ((exponent + 126) << 23);
				}
			}
		}
		
		//Add sign bit
		res |= (sign << 31);
	}
	
	//Change byte order and return
	return res;
}

//Retrieves double value from 32-bit integer. 
double ssc_double_from_flt32(uint32_t v)
{
	double res;
	uint32_t exp_region, mantissa;
	int sign;
	int exponent;
	
	
	//Dissect it
	sign = (v & 0x80000000) ? -1 : 1;
	exp_region = v & 0x7f800000;
	mantissa = v & 0x007fffff;
	
	//Infinity and NaN
	if (exp_region == 0x7f800000)
	{
		if (mantissa)
		{
			//NaN
#ifdef NAN
			return NAN;
#else
			//Is this portable?
			return 0.0 / 0.0;
#endif
		}
		else
		{
			//infinity
			return sign * HUGE_VALF;
		}
	}
	//Subnormal numbers
	else if (! exp_region)
	{
		res = ldexp((double) mantissa, -149);
		res *= sign;
	}
	else
	{
		//Find mantissa and exponent
		exponent = exp_region >> 23;
		mantissa |= 1 << 23;
		//Assemble them together
		res = ldexp((double) mantissa, exponent - 150);
		res *= sign;
	}
	
	//return
	return res;
}

//Returns a 32-bit integer representing the given double value.
uint64_t ssc_double_to_flt64(double v)
{
	int float_type;
	uint64_t res;
	
	//Find the type of floating point
	float_type = fpclassify(v); 
	
	//Check for NaN
	if (float_type == FP_NAN)
	{
		res = 0x7fffffffffffffffLL;
	}
	//Zero
	else if (float_type == FP_ZERO)
	{
		res = 0x0000000000000000LL;
	}
	else
	{
		int sign;
		
		//Extract sign
		sign = signbit(v);
		if (sign)
			sign = 1;
		
		//+/- infinity
		if (float_type == FP_INFINITE)
		{
			res = 0x7ff0000000000000LL;
		}
		else
		{
			//Normal number
			int exponent;
			double fraction;
			
			//Decompose the number
			fraction = frexp(v, &exponent);
			
			
			//Approximate to infinity in case number is too large
			if (exponent > 1024)
			{
				res = 0x7ff0000000000000LL;
			}
			else
			{
				//Make fraction positive
				if (sign)
					fraction = -fraction;
				
				//Extract the mantissa
				if (exponent < -1021)
				{
					//Subnormal
					fraction *= (1LL << (53 + exponent + 1021));
					res = fraction;
					//Exponent area is already zero, no need to attach it
				}
				else
				{
					//Normal
					fraction *= (1LL << 53);
					res = (uint64_t) fraction;
					//Remove the preceeding 1
					res &= 0x000fffffffffffffLL;
					//Attach the exponent
					res |= ((uint64_t) (exponent + 1022) << 52);
				}
			}
		}
		
		//Add sign bit
		res |= ((uint64_t) sign << 63);
	}
	
	//return
	return res;
}

//Retrieves double value from 64-bit integer. 
double ssc_double_from_flt64(uint64_t v)
{
	double res;
	uint64_t exp_region, mantissa;
	int sign;
	int exponent;
	
	//Dissect it
	sign = (v & 0x8000000000000000LL) ? -1 : 1;
	exp_region = v & 0x7ff0000000000000LL;
	mantissa = v & 0x000fffffffffffffLL;
	
	//Infinity and NaN
	if (exp_region == 0x7ff0000000000000LL)
	{
		if (mantissa)
		{
			//NaN
#ifdef NAN
			return NAN;
#else
			//Is this portable?
			return 0.0 / 0.0;
#endif
		}
		else
		{
			//infinity
			return sign * HUGE_VALF;
		}
	}
	//Subnormal numbers
	else if (! exp_region)
	{
		res = ldexp((double) mantissa, -1074);
		res *= sign;
	}
	else
	{
		//Find mantissa and exponent
		exponent = exp_region >> 52;
		mantissa |= 1LL << 52;
		//Assemble them together
		res = ldexp((double) mantissa, exponent - 1075);
		res *= sign;
	}
	
	//return
	return res;
}

//Finds the type of floating point value in given 32-bit integer.
SscFltType ssc_flt32_classify(uint32_t val)
{
	switch (val)
	{
	case 0x00000000:	
		return SSC_FLT_ZERO;
	case 0x7f800000:
		return SSC_FLT_INFINITE;
	case 0xff800000:
		return SSC_FLT_NEG_INFINITE;
	default:
		if ((val & 0x7f800000) == 0x7f800000)
			return SSC_FLT_NAN;
		return SSC_FLT_NORMAL;
	}
}

//Finds the type of floating point value in given 64-bit integer.
SscFltType ssc_flt64_classify(uint64_t val)
{
	switch (val)
	{
	case 0x0000000000000000LL:	
		return SSC_FLT_ZERO;
	case 0x7ff0000000000000LL:
		return SSC_FLT_INFINITE;
	case 0xfff0000000000000LL:
		return SSC_FLT_NEG_INFINITE;
	default:
		if ((val & 0x7ff0000000000000LL) == 0x7ff0000000000000LL)
			return SSC_FLT_NAN;
		return SSC_FLT_NORMAL;
	}
}

