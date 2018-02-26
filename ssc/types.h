/* types.h
 * Making data portable
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
/* ********************************************************************/ 
 
/* NOTE:
 * 1. Little endian will be the byte order for internet domain links.
 * 2. Signed integers will be represented in two's complement form.
 */
 
#include <ssc/generated.h>

/**
 * \addtogroup ssc_types
 * \{
 * 
 * This section documents functions for converting native datatypes
 * to portable form
 * 
 * The portable form for integers is in little endian.
 * For signed integers two's complement form is used.
 * 
 * IEEE 754 format is used to store floating point numbers in unsigned
 * integers.
 */

//byte order conversion
//TODO: Generate endianness independent code when
//      SSC_UINT_UNKNOWN_ENDIAN is defined

static inline uint16_t ssc_uint16_swap_le_be(uint16_t v)
{
	uint16_t r;
	uint8_t *av = (uint8_t *) &v;
	uint8_t *ar = (uint8_t *) &r;

	ar[0] = av[1];
	ar[1] = av[0];

	return r;
}

static inline uint32_t ssc_uint32_swap_le_be(uint32_t v)
{
	uint32_t r;
	uint8_t *av = (uint8_t *) &v;
	uint8_t *ar = (uint8_t *) &r;

	ar[0] = av[3];
	ar[1] = av[2];
	ar[2] = av[1];
	ar[3] = av[0];

	return r;
}

static inline uint64_t ssc_uint64_swap_le_be(uint64_t v)
{
	uint64_t r;
	uint8_t *av = (uint8_t *) &v;
	uint8_t *ar = (uint8_t *) &r;

	ar[0] = av[7];
	ar[1] = av[6];
	ar[2] = av[5];
	ar[3] = av[4];
	ar[4] = av[3];
	ar[5] = av[2];
	ar[6] = av[1];
	ar[7] = av[0];

	return r;
}

//TODO: How to use autotools for mixed endian systems?

/**Converts a 16-bit integer from native byte order to little endian
 * byte order and vice versa.
 * 
 * This function is symmetric, so it works both ways.
 * 
 * On little endian systems this function is implemented as a macro.
 * 
 * \param val an int16_t value in little-endian or native byte order
 * \return val converted to the opposite byte order
 */
static inline uint16_t ssc_uint16_swap_native_le(uint16_t v)
{
#ifdef SSC_UINT_BIG_ENDIAN
	return ssc_uint16_swap_le_be(v);
#else
	return v;
#endif
}

/**Converts a 32-bit integer from native byte order to little endian
 * byte order.
 * 
 * On little endian systems this function is implemented as a macro.
 * 
 * \param val an integer in native byte order
 * \return val converted to little endian byte order
 */
static inline uint32_t ssc_uint32_to_le(uint32_t v)
{
#ifdef SSC_UINT_BIG_ENDIAN
	return ssc_uint32_swap_le_be(v);
#else
	return v;
#endif
}

/**Converts a 32-bit integer from little endian byte order to native
 * byte order.
 * 
 * On little endian systems this function is implemented as a macro.
 * 
 * \param val an integer in little endian byte order
 * \return val converted to native byte order
 */
static inline uint32_t ssc_uint32_from_le(uint32_t v)
{
#ifdef SSC_UINT_BIG_ENDIAN
	return ssc_uint32_swap_le_be(v);
#else
	return v;
#endif
}

/**Converts a 64-bit integer from native byte order to little endian
 * byte order.
 * 
 * On little endian systems this function is implemented as a macro.
 * 
 * \param val an integer in native byte order
 * \return val converted to little endian byte order
 */
static inline uint64_t ssc_uint64_to_le(uint64_t v)
{
#ifdef SSC_UINT_BIG_ENDIAN
	return ssc_uint64_swap_le_be(v);
#else
	return v;
#endif
}

/**Converts a 64-bit integer from little endian byte order to native
 * byte order.
 * 
 * On little endian systems this function is implemented as a macro.
 * 
 * \param val an integer in little endian byte order
 * \return val converted to native byte order
 */
static inline uint64_t ssc_uint64_from_le(uint64_t v)
{
#ifdef SSC_UINT_BIG_ENDIAN
	return ssc_uint64_swap_le_be(v);
#else
	return v;
#endif
}


//Unaligned Load/store macros

#ifdef SSC_UINT_BIG_ENDIAN
#define SSC_BYTE_SIGNIFICANCE(type, idx) ((sizeof(type)/8) - (idx))
#else
#define SSC_BYTE_SIGNIFICANCE(type, idx) (idx)
#endif

/**Loads a 16-bit wide little-endian unsigned integer
 * from given memory location, converting it to host byte order.
 * \param le Pointer to the little endian value
 * \return The value in host byte order
 */
static inline uint16_t ssc_uint16_load_le(void *le)
{
	uint8_t *le_a = (uint8_t *) le;
	union 
	{
		uint16_t v;
		uint8_t a[sizeof(uint16_t)];
	} h;
	h.a[0] = le_a[SSC_BYTE_SIGNIFICANCE(uint16_t, 0)];
	h.a[1] = le_a[SSC_BYTE_SIGNIFICANCE(uint16_t, 1)];
	return h.v;
}

/**Loads a 32-bit wide little-endian unsigned integer
 * from given memory location, converting it to host byte order.
 * \param le Pointer to the little endian value
 * \return The value in host byte order
 */
static inline uint32_t ssc_uint32_load_le(void *le)
{
	uint8_t *le_a = (uint8_t *) le;
	union 
	{
		uint32_t v;
		uint8_t a[sizeof(uint32_t)];
	} h;
	h.a[0] = le_a[SSC_BYTE_SIGNIFICANCE(uint32_t, 0)];
	h.a[1] = le_a[SSC_BYTE_SIGNIFICANCE(uint32_t, 1)];
	h.a[2] = le_a[SSC_BYTE_SIGNIFICANCE(uint32_t, 2)];
	h.a[3] = le_a[SSC_BYTE_SIGNIFICANCE(uint32_t, 3)];
	return h.v;
}

/**Loads a 64-bit wide little-endian unsigned integer
 * from given memory location, converting it to host byte order.
 * \param le Pointer to the little endian value
 * \return The value in host byte order
 */
static inline uint64_t ssc_uint64_load_le(void *le)
{
	uint8_t *le_a = (uint8_t *) le;
	union 
	{
		uint64_t v;
		uint8_t a[sizeof(uint64_t)];
	} h;
	h.a[0] = le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 0)];
	h.a[1] = le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 1)];
	h.a[2] = le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 2)];
	h.a[3] = le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 3)];
	h.a[4] = le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 4)];
	h.a[5] = le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 5)];
	h.a[6] = le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 6)];
	h.a[7] = le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 7)];
	return h.v;
}

/**Stores a 16-bit wide integer in little-endian byte order,
 * after converting it from host byte order
 * \param le Pointer to the little endian value
 * \param v Value in host byte order to store
 */
static inline void ssc_uint16_store_le(void *le, uint16_t v)
{
	uint8_t *le_a = (uint8_t *) le;
	union 
	{
		uint16_t v;
		uint8_t a[sizeof(uint16_t)/8];
	} h;
	h.v = v;
	le_a[SSC_BYTE_SIGNIFICANCE(uint16_t, 0)] = h.a[0];
	le_a[SSC_BYTE_SIGNIFICANCE(uint16_t, 1)] = h.a[1];
}

/**Stores a 32-bit wide integer in little-endian byte order,
 * after converting it from host byte order
 * \param le Pointer to the little endian value
 * \param v Value in host byte order to store
 */
static inline void ssc_uint32_store_le(void *le, uint32_t v)
{
	uint8_t *le_a = (uint8_t *) le;
	union 
	{
		uint32_t v;
		uint8_t a[sizeof(uint32_t)/8];
	} h;
	h.v = v;
	le_a[SSC_BYTE_SIGNIFICANCE(uint32_t, 0)] = h.a[0];
	le_a[SSC_BYTE_SIGNIFICANCE(uint32_t, 1)] = h.a[1];
	le_a[SSC_BYTE_SIGNIFICANCE(uint32_t, 2)] = h.a[2];
	le_a[SSC_BYTE_SIGNIFICANCE(uint32_t, 3)] = h.a[3];
}

/**Stores a 64-bit wide integer in little-endian byte order,
 * after converting it from host byte order
 * \param le Pointer to the little endian value
 * \param v Value in host byte order to store
 */
static inline void ssc_uint64_store_le(void *le, uint64_t v)
{
	uint8_t *le_a = (uint8_t *) le;
	union 
	{
		uint64_t v;
		uint8_t a[sizeof(uint64_t)/8];
	} h;
	h.v = v;
	le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 0)] = h.a[0];
	le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 1)] = h.a[1];
	le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 2)] = h.a[2];
	le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 3)] = h.a[3];
	le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 4)] = h.a[4];
	le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 5)] = h.a[5];
	le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 6)] = h.a[6];
	le_a[SSC_BYTE_SIGNIFICANCE(uint64_t, 7)] = h.a[7];
}

//Two's complement conversions for signed integers
#if INT8_MIN == -128
#define SSC_INT_2_COMPLEMENT
#endif

/**Converts a signed char to two's complement form
 * \param val signed char to convert
 * \return two's complement form of val
 */
static inline unsigned char ssc_int8_to_2_complement(char val)
{
#ifndef SSC_INT_2_COMPLEMENT
	if (val < 0)
	{
		unsigned char res;
		res = -val;   //< Take absolute value
		res = ~ res;  //< one's complement
		res += 1;     //< two's complement
		return res;
	}
#endif
	return (unsigned char) val;
}

/**Converts a 16-bit signed integer to two's complement form
 * \param val int16_t to convert
 * \return two's complement form of val
 */
static inline uint16_t ssc_int16_to_2_complement(int16_t val)
{
#ifndef SSC_INT_2_COMPLEMENT
	if (val < 0)
	{
		uint16_t res;
		res = -val;   //< Take absolute value
		res = ~ res;  //< one's complement
		res += 1;     //< two's complement
		return res;
	}
#endif
	return (uint16_t) val;
}

/**Converts a 32-bit signed integer to two's complement form
 * \param val int32_t to convert
 * \return two's complement form of val
 */
static inline uint32_t ssc_int32_to_2_complement(int32_t val)
{
#ifndef SSC_INT_2_COMPLEMENT
	if (val < 0)
	{
		uint32_t res;
		res = -val;   //< Take absolute value
		res = ~ res;  //< one's complement
		res += 1;     //< two's complement
		return res;
	}
#endif
	return (uint32_t) val;
}

/**Converts a 64-bit signed integer to two's complement form
 * \param val int64_t to convert
 * \return two's complement form of val
 */
static inline uint64_t ssc_int64_to_2_complement(int64_t val)
{
#ifndef SSC_INT_2_COMPLEMENT
	if (val < 0)
	{
		uint64_t res;
		res = -val;   //< Take absolute value
		res = ~ res;  //< one's complement
		res += 1;     //< two's complement
		return res;
	}
#endif
	return (uint64_t) val;
}

/**Converts a signed char in two's complement form to native form
 * \param val A two's complement representation of signed char
 * \return Native representation of val
 */
static inline char ssc_int8_from_2_complement(unsigned char val)
{
#ifndef SSC_INT_2_COMPLEMENT
	//Check for negative value
	if (val & (1 << 7))
	{
		char res;
		//Follow inverse operations
		val -= 1;
		val = ~val;
		res = -val;
		return res;
	}
#endif
	return (char) val;
}

/**Converts a 16-bit integer in two's complement form to native form
 * \param val A two's complement representation of a 16-bit signed integer
 * \return Native representation of val
 */
static inline int16_t ssc_int16_from_2_complement(uint16_t val)
{
#ifndef SSC_INT_2_COMPLEMENT
	//Check for negative value
	if (val & (1 << 15))
	{
		int16_t res;
		//Follow inverse operations
		val -= 1;
		val = ~val;
		res = -val;
		return res;
	}
#endif
	return (int16_t) val;
}

/**Converts a 32-bit integer in two's complement form to native form
 * \param val A two's complement representation of a 32-bit signed integer
 * \return Native representation of val
 */
static inline int32_t ssc_int32_from_2_complement(uint32_t val)
{
#ifndef SSC_INT_2_COMPLEMENT
	//Check for negative value
	if (val & (1 << 31))
	{
		int32_t res;
		//Follow inverse operations
		val -= 1;
		val = ~val;
		res = -val;
		return res;
	}
#endif
	return (int32_t) val;
}

/**Converts a 64-bit integer in two's complement form to native form
 * \param val A two's complement representation of a 64-bit signed integer
 * \return Native representation of val
 */
static inline int64_t ssc_int64_from_2_complement(uint64_t val)
{
#ifndef SSC_INT_2_COMPLEMENT
	//Check for negative value
	if (val & (1LL << 63))
	{
		int64_t res;
		//Follow inverse operations
		val -= 1;
		val = ~val;
		res = -val;
		return res;
	}
#endif
	return (int64_t) val;
}


//Convenience macros for signed integers
///Convenient macro for converting 16-bit signed integer to portable form.
#define ssc_int16_to_le(val) (ssc_uint16_swap_native_le(ssc_int16_to_2_complement(val)))

///Convenient macro for converting 32-bit signed integer to portable form.
#define ssc_int32_to_le(val) (ssc_uint32_to_le(ssc_int32_to_2_complement(val)))

///Convenient macro for converting 64-bit signed integer to portable form.
#define ssc_int64_to_le(val) (ssc_uint64_to_le(ssc_int64_to_2_complement(val)))

///Convenient macro for converting 16-bit signed integer to native form.
#define ssc_int16_from_le(val) (ssc_int16_from_2_complement(ssc_uint16_swap_native_le(val)))

///Convenient macro for converting 32-bit signed integer to portable form.
#define ssc_int32_from_le(val) (ssc_int32_from_2_complement(ssc_uint32_from_le(val)))

///Convenient macro for converting 64-bit signed integer to portable form.
#define ssc_int64_from_le(val) (ssc_int64_from_2_complement(ssc_uint64_from_le(val)))


//Floating point numbers

/**Type of floating point number, as returned by ssc_flt32_classify()
 * and ssc_flt64_classify()
 */
typedef enum
{
	///An ordinary nonzero floating point number
	SSC_FLT_NORMAL,
	///Positive infinity
	SSC_FLT_INFINITE,
	///Negative infinity
	SSC_FLT_NEG_INFINITE,
	///Zero
	SSC_FLT_ZERO,
	///Not a number (NaN)
	SSC_FLT_NAN
} SscFltType;

///32-bit floating point number representing zero
#define ssc_flt32_zero             ((uint32_t) 0x00000000)

///32-bit floating point number representing infinity
#define ssc_flt32_infinity         ((uint32_t) 0x7f800000)

///32-bit floating point number representing negative infinity
#define ssc_flt32_neg_infinity     ((uint32_t) 0xff800000)

///32-bit floating point number representing NaN
#define ssc_flt32_nan              ((uint32_t) 0x7fffffff)


///64-bit floating point number representing zero
#define ssc_flt64_zero             ((uint64_t) 0x0000000000000000LL)

///64-bit floating point number representing infinity
#define ssc_flt64_infinity         ((uint64_t) 0x7ff0000000000000LL)

///64-bit floating point number representing negative infinity
#define ssc_flt64_neg_infinity     ((uint64_t) 0xfff0000000000000LL)

///64-bit floating point number representing NaN
#define ssc_flt64_nan              ((uint64_t) 0x7fffffffffffffffLL)



/**Returns a 32-bit integer representing the given float value.
 * 
 * Small inaccuracies may occur due to format conversion.
 * 
 * The return value is in IEEE 754 single precision floating point form.
 * 
 * The result is in host byte order and must be converted to 
 * little endian before sending.
 * 
 * \param v The float value to convert.
 * \return Integer representing the given float value.
 */
uint32_t ssc_float_to_flt32(float v);

/**Retrieves float value from 32-bit integer. 
 * 
 * This function does the opposite of ssc_float_to_flt32().
 * 
 * \param v 32-bit integer representing a floating point value
 * \return The same number in native float type.
 */
float ssc_float_from_flt32(uint32_t v);

/**Returns a 32-bit integer representing the given double value.
 * 
 * This function is same as ssc_float_to_flt32(), except that
 * it takes a double value as its argument
 * 
 * \param v The double value to convert.
 * \return Integer representing the given float value.
 */
uint32_t ssc_double_to_flt32(double v);

/**Retrieves double value from 32-bit integer. 
 * 
 * This function does the opposite of ssc_double_to_flt32().
 * 
 * \param v 32-bit integer representing a floating point value
 * \return The same number in native double type.
 */
double ssc_double_from_flt32(uint32_t v);

/**Returns a 32-bit integer representing the given double value.
 * 
 * Small inaccuracies may occur due to format conversion.
 * 
 * The return value is in IEEE 754 single precision floating point form.
 * 
 * The result is in host byte order and must be converted to 
 * little endian before sending.
 * 
 * \param v The double value to convert.
 * \return Integer representing the given double value.
 */
uint64_t ssc_double_to_flt64(double v);

/**Retrieves double value from 64-bit integer. 
 * 
 * This function does the opposite of ssc_double_to_flt64().
 * 
 * \param v 64-bit machine-independent floating point number
 * \return The same number in native double type.
 */
double ssc_double_from_flt64(uint64_t v);

/**Finds the type of floating point value in given 32-bit integer.
 * 
 * This function is useful in case your machine's floating point 
 * implementation does not support infinity or NaN.
 * 
 * \param val The 32-bit floating point number to test
 * \return Type of floating point number
 */
SscFltType ssc_flt32_classify(uint32_t val);

/**Finds the type of floating point value in given 64-bit integer.
 * 
 * This function is useful in case your machine's floating point 
 * implementation does not support infinity or NaN.
 * 
 * \param val The 64-bit floating point number to test
 * \return Type of floating point number
 */
SscFltType ssc_flt64_classify(uint64_t val);

///\}

