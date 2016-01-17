/* types.h
 * Making data portable
 * 
 * Copyright 2015 Akash Rawal
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
 
#ifndef SSC_TYPES_H
#define SSC_TYPES_H
 
#include "includes.h"

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

//Common stuff used by all conversion logic
/**Copies a 16-bit wide unsigned integer from h_ptr, converts it to
 * little endian byte order, and then stores it at le_ptr.
 * \param le_ptr Pointer to store converted value
 * \param h_ptr Pointer where value to be converted is stored
 */
#define ssc_uint16_copy_to_le(le_ptr, h_ptr) \
do { \
	((char *) (le_ptr))[SSC_UINT16_BYTE_0_SIGNIFICANCE] = ((char *) (h_ptr))[0]; \
	((char *) (le_ptr))[SSC_UINT16_BYTE_1_SIGNIFICANCE] = ((char *) (h_ptr))[1]; \
} while (0)

/**Copies a 16-bit wide unsigned integer from le_ptr, converts it to
 * host byte order, and then stores it at h_ptr.
 * \param le_ptr Pointer where value to be converted is stored 
 * \param h_ptr Pointer to store converted value
 */
#define ssc_uint16_copy_from_le(le_ptr, h_ptr) \
do { \
	((char *) (h_ptr))[0] = ((char *) (le_ptr))[SSC_UINT16_BYTE_0_SIGNIFICANCE]; \
	((char *) (h_ptr))[1] = ((char *) (le_ptr))[SSC_UINT16_BYTE_1_SIGNIFICANCE]; \
} while (0)

/**Copies a 32-bit wide unsigned integer from h_ptr, converts it to
 * little endian byte order, and then stores it at le_ptr.
 * \param le_ptr Pointer to store converted value
 * \param h_ptr Pointer where value to be converted is stored
 */
#define ssc_uint32_copy_to_le(le_ptr, h_ptr) \
do { \
	((char *) (le_ptr))[SSC_UINT32_BYTE_0_SIGNIFICANCE] = ((char *) (h_ptr))[0]; \
	((char *) (le_ptr))[SSC_UINT32_BYTE_1_SIGNIFICANCE] = ((char *) (h_ptr))[1]; \
	((char *) (le_ptr))[SSC_UINT32_BYTE_2_SIGNIFICANCE] = ((char *) (h_ptr))[2]; \
	((char *) (le_ptr))[SSC_UINT32_BYTE_3_SIGNIFICANCE] = ((char *) (h_ptr))[3]; \
} while (0)

/**Copies a 32-bit wide unsigned integer from le_ptr, converts it to
 * host byte order, and then stores it at h_ptr.
 * \param le_ptr Pointer where value to be converted is stored 
 * \param h_ptr Pointer to store converted value
 */
#define ssc_uint32_copy_from_le(le_ptr, h_ptr) \
do { \
	((char *) (h_ptr))[0] = ((char *) (le_ptr))[SSC_UINT32_BYTE_0_SIGNIFICANCE]; \
	((char *) (h_ptr))[1] = ((char *) (le_ptr))[SSC_UINT32_BYTE_1_SIGNIFICANCE]; \
	((char *) (h_ptr))[2] = ((char *) (le_ptr))[SSC_UINT32_BYTE_2_SIGNIFICANCE]; \
	((char *) (h_ptr))[3] = ((char *) (le_ptr))[SSC_UINT32_BYTE_3_SIGNIFICANCE]; \
} while (0)

/**Copies a 64-bit wide unsigned integer from h_ptr, converts it to
 * little endian byte order, and then stores it at le_ptr.
 * \param le_ptr Pointer to store converted value
 * \param h_ptr Pointer where value to be converted is stored
 */
#define ssc_uint64_copy_to_le(le_ptr, h_ptr) \
do { \
	((char *) (le_ptr))[SSC_UINT64_BYTE_0_SIGNIFICANCE] = ((char *) (h_ptr))[0]; \
	((char *) (le_ptr))[SSC_UINT64_BYTE_1_SIGNIFICANCE] = ((char *) (h_ptr))[1]; \
	((char *) (le_ptr))[SSC_UINT64_BYTE_2_SIGNIFICANCE] = ((char *) (h_ptr))[2]; \
	((char *) (le_ptr))[SSC_UINT64_BYTE_3_SIGNIFICANCE] = ((char *) (h_ptr))[3]; \
	((char *) (le_ptr))[SSC_UINT64_BYTE_4_SIGNIFICANCE] = ((char *) (h_ptr))[4]; \
	((char *) (le_ptr))[SSC_UINT64_BYTE_5_SIGNIFICANCE] = ((char *) (h_ptr))[5]; \
	((char *) (le_ptr))[SSC_UINT64_BYTE_6_SIGNIFICANCE] = ((char *) (h_ptr))[6]; \
	((char *) (le_ptr))[SSC_UINT64_BYTE_7_SIGNIFICANCE] = ((char *) (h_ptr))[7]; \
} while (0)

/**Copies a 64-bit wide unsigned integer from le_ptr, converts it to
 * host byte order, and then stores it at h_ptr.
 * \param le_ptr Pointer where value to be converted is stored 
 * \param h_ptr Pointer to store converted value
 */
#define ssc_uint64_copy_from_le(le_ptr, h_ptr) \
do { \
	((char *) (h_ptr))[0] = ((char *) (le_ptr))[SSC_UINT64_BYTE_0_SIGNIFICANCE]; \
	((char *) (h_ptr))[1] = ((char *) (le_ptr))[SSC_UINT64_BYTE_1_SIGNIFICANCE]; \
	((char *) (h_ptr))[2] = ((char *) (le_ptr))[SSC_UINT64_BYTE_2_SIGNIFICANCE]; \
	((char *) (h_ptr))[3] = ((char *) (le_ptr))[SSC_UINT64_BYTE_3_SIGNIFICANCE]; \
	((char *) (h_ptr))[4] = ((char *) (le_ptr))[SSC_UINT64_BYTE_4_SIGNIFICANCE]; \
	((char *) (h_ptr))[5] = ((char *) (le_ptr))[SSC_UINT64_BYTE_5_SIGNIFICANCE]; \
	((char *) (h_ptr))[6] = ((char *) (le_ptr))[SSC_UINT64_BYTE_6_SIGNIFICANCE]; \
	((char *) (h_ptr))[7] = ((char *) (le_ptr))[SSC_UINT64_BYTE_7_SIGNIFICANCE]; \
} while (0)

//16-bit
#ifndef SSC_UINT16_LITTLE_ENDIAN
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
uint16_t ssc_uint16_swap_native_le(uint16_t val);
#else 
#define ssc_uint16_swap_native_le(val) ((uint16_t) (val))
#endif

//32-bit
#ifndef SSC_UINT32_LITTLE_ENDIAN
/**Converts a 32-bit integer from native byte order to little endian
 * byte order.
 * 
 * On little endian systems this function is implemented as a macro.
 * 
 * \param val an integer in native byte order
 * \return val converted to little endian byte order
 */
uint32_t ssc_uint32_to_le(uint32_t val);

/**Converts a 32-bit integer from little endian byte order to native
 * byte order.
 * 
 * On little endian systems this function is implemented as a macro.
 * 
 * \param val an integer in little endian byte order
 * \return val converted to native byte order
 */
uint32_t ssc_uint32_from_le(uint32_t val);

#else
#define ssc_uint32_to_le(val) ((uint32_t) (val))
#define ssc_uint32_from_le(val) ((uint32_t) (val))
#endif

//64-bit
#ifndef SSC_UINT64_LITTLE_ENDIAN
/**Converts a 64-bit integer from native byte order to little endian
 * byte order.
 * 
 * On little endian systems this function is implemented as a macro.
 * 
 * \param val an integer in native byte order
 * \return val converted to little endian byte order
 */
uint64_t ssc_uint64_to_le(uint64_t val);

/**Converts a 64-bit integer from little endian byte order to native
 * byte order.
 * 
 * On little endian systems this function is implemented as a macro.
 * 
 * \param val an integer in little endian byte order
 * \return val converted to native byte order
 */
uint64_t ssc_uint64_from_le(uint64_t val);

#else
#define ssc_uint64_to_le(val) ((uint64_t) (val))
#define ssc_uint64_from_le(val) ((uint64_t) (val))
#endif

//Two's complement conversions for signed integers
#ifndef SSC_INT_2_COMPLEMENT
/**Converts a signed char to two's complement form
 * \param val signed char to convert
 * \return two's complement form of val
 */
unsigned char ssc_char_to_2_complement(char val)

/**Converts a 16-bit signed integer to two's complement form
 * \param val int16_t to convert
 * \return two's complement form of val
 */
uint16_t ssc_int16_to_2_complement(int16_t val);

/**Converts a 32-bit signed integer to two's complement form
 * \param val int32_t to convert
 * \return two's complement form of val
 */
uint32_t ssc_int32_to_2_complement(int32_t val);

/**Converts a 64-bit signed integer to two's complement form
 * \param val int64_t to convert
 * \return two's complement form of val
 */
uint64_t ssc_int64_to_2_complement(int64_t val);

/**Converts a signed char in two's complement form to native form
 * \param val A two's complement representation of signed char
 * \return Native representation of val
 */
char ssc_char_from_2_complement(unsigned char val);

/**Converts a 16-bit integer in two's complement form to native form
 * \param val A two's complement representation of a 16-bit signed integer
 * \return Native representation of val
 */
int16_t ssc_int16_from_2_complement(uint16_t val);

/**Converts a 32-bit integer in two's complement form to native form
 * \param val A two's complement representation of a 32-bit signed integer
 * \return Native representation of val
 */
int32_t ssc_int32_from_2_complement(uint32_t val);

/**Converts a 64-bit integer in two's complement form to native form
 * \param val A two's complement representation of a 64-bit signed integer
 * \return Native representation of val
 */
int64_t ssc_int64_from_2_complement(uint64_t val);

#else
//no-op macros for 2's complement systems
#define ssc_char_to_2_complement(val)    ((unsigned char) (val))
#define ssc_int16_to_2_complement(val)   ((uint16_t)      (val))
#define ssc_int32_to_2_complement(val)   ((uint32_t)      (val))
#define ssc_int64_to_2_complement(val)   ((uint64_t)      (val))
#define ssc_char_from_2_complement(val)  ((char)          (val))
#define ssc_int16_from_2_complement(val) ((int16_t)       (val))
#define ssc_int32_from_2_complement(val) ((int32_t)       (val))
#define ssc_int64_from_2_complement(val) ((int64_t)       (val))

#endif

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

#endif //SSC_TYPES_H
