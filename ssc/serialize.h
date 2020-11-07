/* serialize.h
 * All the serialization stuff
 * 
 * Copyright 2015-2020 Akash Rawal
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


/**
 * \addtogroup ssc_msg_iter
 * \{
 * 
 * This section describes a message iterator, used internally by 
 * autogenerated serializers to serialize the data.
 * 
 * Iteration is generally done a 'segment' at a time.
 * Iterator is moved forward by a given number of bytes and submessages.
 * (Here is where bounds checking is done to prevent reading past
 * the end of the message.) This is to minimize the number of
 * assertions (and hence the number of failure points) in the code.
 * FIXME: Summarise the interface?
 * 
 * You probably don't need to learn more than this unless 
 * you are dealing with its internals. 
 * FIXME: What part not needed to read?
 */

///A structure that you can take help of to count how large the 
///serialized data will be.
typedef struct 
{
	///Size of the memory block
	size_t n_bytes;
	///No. of submessages
	size_t n_submsgs;
} SscDLen;

/**Initializes the counter
 * \param self A pointer to structure of type SscDLen
 */
#define ssc_dlen_zero(self) \
	(self)->n_bytes = (self)->n_submsgs = 0

/**A structure to iterate over message. This can be used in
 * [de]serialization. 
 */
typedef struct
{
	///Current position in memory block
	char *bytes;
	///A pointer that points just after the last byte in memory block
	char *bytes_lim;
	///Current position in array of messages
	MmcMsg **submsgs;
	///A pointer that points just after the last element in array of 
	///submessages
	MmcMsg **submsgs_lim;
} SscMsgIter;

/**A segment popped off an iterator
 */
typedef struct
{
	///Current position in byte stream
	char *bytes;
	///Current position in block stream
	MmcMsg **submsgs;
} SscSegment;

/**Initializes the iterator to the start of the given message.
 * \param self The iterator
 * \param msg The message whose contents to iterate
 */
void ssc_msg_iter_init(SscMsgIter *self, MmcMsg *msg);

/**Gets a segment from a iterator, advancing its position forward. 
 * \param self The iterator
 * \param n_bytes The number of bytes to 'read' from the byte stream
 * \param n_submsgs The number of submsgs to 'read' from the block stream
 * \param res Pointer to the resulting segment struct
 * \return an MdslStatus to state whether the operation was successful.
 */
MdslStatus ssc_msg_iter_get_segment
	(SscMsgIter *self, size_t n_bytes, size_t n_submsgs, 
	 SscSegment *res);

/**Determines whether the iterator is at the end.*/
static inline int ssc_msg_iter_at_end(SscMsgIter *self)
{
	return (((self)->bytes_lim - (self)->bytes) 
	 + ((self)->submsgs_lim - (self)->submsgs) ? 0 : 1);
}

//writing unsigned integers
/**Stores a 1-byte unsigned char to the current segment position
 * and increments it accordingly.
 * \param seg Pointer to the segment
 * \param val Value to store
 */
static inline void ssc_segment_write_uint8(SscSegment *seg, uint8_t val)
{
	*(seg->bytes) = val;
	seg->bytes += 1;
}
		
/**Assigns a 16-bit unsigned integer to current segment position
 * after converting to little endian and increments it accordingly.
 * \param seg Pointer to the segment
 * \param val Value to store
 */
static inline void ssc_segment_write_uint16(SscSegment *seg, uint16_t val)
{
	ssc_uint16_store_le(seg->bytes, val);
	seg->bytes += 2;
}

/**Assigns a 32-bit unsigned integer to current segment position
 * after converting to little endian and increments it accordingly.
 * \param seg Pointer to the segment
 * \param val Value to store
 */
static inline void ssc_segment_write_uint32(SscSegment *seg, uint32_t val)
{
	ssc_uint32_store_le(seg->bytes, val);
	seg->bytes += 4;
}

/**Assigns a 64-bit unsigned integer to current segment position
 * after converting to little endian and increments it accordingly.
 * \param seg Pointer to the segment
 * \param val Value to store
 */
static inline void ssc_segment_write_uint64(SscSegment *seg, uint64_t val)
{
	ssc_uint64_store_le(seg->bytes, val);
	seg->bytes += 8;
}

//reading unsigned integers
/**Retrives a 1-byte unsigned char from the current segment position
 * and increments it accordingly.
 * \param seg Pointer to the segment
 * \res the result
 */
static inline uint8_t ssc_segment_read_uint8(SscSegment *seg)
{
	uint8_t r = *seg->bytes;
	seg->bytes += 1;
	return r;
}

/**Retrives a 16-bit unsigned integer from current segment position
 * after converting to host byte order and increments it accordingly.
 * \param seg Pointer to the segment
 * \res the result
 */
static inline uint16_t ssc_segment_read_uint16(SscSegment *seg)
{
	uint16_t r = ssc_uint16_load_le(seg->bytes);
	seg->bytes += 2;
	return r;
}

/**Retrives a 32-bit unsigned integer from current segment position
 * after converting to host byte order and increments it accordingly.
 * \param seg Pointer to the segment
 * \res the result
 */
static inline uint32_t ssc_segment_read_uint32(SscSegment *seg)
{
	uint32_t r = ssc_uint32_load_le(seg->bytes);
	seg->bytes += 4;
	return r;
}

/**Retrives a 64-bit unsigned integer from current segment position
 * after converting to host byte order and increments it accordingly.
 * \param seg Pointer to the segment
 * \res the result
 */
static inline uint64_t ssc_segment_read_uint64(SscSegment *seg)
{
	uint64_t r = ssc_uint64_load_le(seg->bytes);
	seg->bytes += 8;
	return r;
}

#ifdef SSC_INT_2_COMPLEMENT
#define ssc_segment_write_int8 ssc_segment_write_uint8
#define ssc_segment_write_int16 ssc_segment_write_uint16
#define ssc_segment_write_int32 ssc_segment_write_uint32
#define ssc_segment_write_int64 ssc_segment_write_uint64
#define ssc_segment_read_int8 ssc_segment_read_uint8
#define ssc_segment_read_int16 ssc_segment_read_uint16
#define ssc_segment_read_int32 ssc_segment_read_uint32
#define ssc_segment_read_int64 ssc_segment_read_uint64
#else

//writing signed integers
/**Assigns a signed char to current segment position
 * and increments it accordingly.
 * \param seg Pointer to the segment (type SscSegment *)
 * \param lval Value to store (type char)
 */
#define ssc_segment_write_int8(seg, lval) \
	ssc_segment_write_uint8(seg, ssc_int8_to_2_complement(lval))

/**Assigns a 16-bit signed integer to current segment position
 * after converting to little endian and increments it accordingly.
 * \param seg Pointer to the segment (type SscSegment *)
 * \param lval Variable of type uint16_t holding the value to assign
 *        (Must be a variable. To write a constant assign it to a variable
 *        first)
 */
#define ssc_segment_write_int16(seg, lval) \
do { \
	uint16_t lval_2c = ssc_int16_to_2_complement(lval); \
	ssc_segment_write_uint16(seg, lval_2c); \
} while(0)

/**Assigns a 32-bit signed integer to current segment position
 * after converting to little endian and increments it accordingly.
 * \param seg Pointer to the segment (type SscSegment *)
 * \param lval Variable of type uint32_t holding the value to assign
 *        (Must be a variable. To write a constant assign it to a variable
 *        first)
 */
#define ssc_segment_write_int32(seg, lval) \
do { \
	uint32_t lval_2c = ssc_int32_to_2_complement(lval); \
	ssc_segment_write_uint32(seg, lval_2c); \
} while(0)

/**Assigns a 64-bit signed integer to current segment position
 * after converting to little endian and increments it accordingly.
 * \param seg Pointer to the segment (type SscSegment *)
 * \param lval Variable of type uint64_t holding the value to assign
 *        (Must be a variable. To write a constant assign it to a variable
 *        first)
 */
#define ssc_segment_write_int64(seg, lval) \
do { \
	uint64_t lval_2c = ssc_int64_to_2_complement(lval); \
	ssc_segment_write_uint64(seg, lval_2c); \
} while(0)

//reading signed integers
/**Retrives a signed character from current segment position
 * after converting to host byte order and increments it accordingly.
 * \param seg Pointer to the segment (type SscSegment *)
 * \param lval Variable of type char to store the result
 */
#define ssc_segment_read_int8(seg, lval) \
do { \
	ssc_segment_read_uint8(seg, lval); \
	lval = ssc_int8_to_2_complement(lval); \
} while(0)

/**Retrives a 16-bit signed integer from current segment position
 * after converting to host byte order and increments it accordingly.
 * \param seg Pointer to the segment (type SscSegment *)
 * \param lval Variable of type uint16_t to store the result
 */
#define ssc_segment_read_int16(seg, lval) \
do { \
	ssc_segment_read_uint16(seg, lval); \
	lval = ssc_int16_to_2_complement(lval); \
} while(0)

/**Retrives a 32-bit signed integer from current segment position
 * after converting to host byte order and increments it accordingly.
 * \param seg Pointer to the segment (type SscSegment *)
 * \param lval Variable of type uint32_t to store the result
 */
#define ssc_segment_read_int32(seg, lval) \
do { \
	ssc_segment_read_uint32(seg, lval); \
	lval = ssc_int32_to_2_complement(lval); \
} while(0)

/**Retrives a 64-bit signed integer from current segment position
 * after converting to host byte order and increments it accordingly.
 * \param seg Pointer to the segment (type SscSegment *)
 * \param lval Variable of type uint64_t to store the result
 */
#define ssc_segment_read_int64(seg, lval) \
do { \
	ssc_segment_read_uint64(seg, lval); \
	lval = ssc_int64_to_2_complement(lval); \
} while(0)

#endif

/**Structure that you can use to portably store any floating point value
 * that IEEE 754 supports.
 * MDL type flt32 and flt64 map to this type.
 */
typedef struct
{
	///Type of floating point value stored
	SscFltType type;
	/**The floating point value stored. It should store correct value 
	 * when _type_ is SSC_FLT_ZERO or SSC_FLT_NORMAL, otherwise
	 * it may store any value.
	 */
	double val;
} SscValFlt;

/**Stores the given floating point value at current segment position 
 * in IEEE 754 32-bit format and increments the position accordingly.
 * \param seg Pointer to the segment
 * \param val The value to store
 */
void ssc_segment_write_flt32(SscSegment *seg, SscValFlt val);

/**Retrives a floating point value from current segment position
 * in IEEE 754 32-bit format and increments the position accordingly. 
 * \param seg Pointer to the segment
 * \param val  Pointer indicating where to store the value
 */
void ssc_segment_read_flt32(SscSegment *seg, SscValFlt *val);

/**Stores the given floating point value at current segment position 
 * in IEEE 754 64-bit format and increments the position accordingly.
 * \param seg Pointer to the segment
 * \param val The value to store
 */
void ssc_segment_write_flt64(SscSegment *seg, SscValFlt val);

/**Retrives a floating point value from current segment position
 * in IEEE 754 64-bit format and increments the position accordingly. 
 * \param seg Pointer to the segment
 * \param val  Pointer indicating where to store the value
 */
void ssc_segment_read_flt64(SscSegment *seg, SscValFlt *val);

/**Adds a null-terminated string to the current segment position 
 * and increments the segment accordingly.
 * \param seg Pointer to the segment.
 * \param val The null-terminated string to add
 */
void ssc_segment_write_string(SscSegment *seg, char *val);

/**Retrieves a null-terminated string from the current segment position 
 * and increments the segment accordingly.
 * \param seg Pointer to the segment.
 * \return The string just read off or NULL if operation failed. 
 *         Use free() to free it. 
 */
char *ssc_segment_read_string(SscSegment *seg);

/**Adds a message to the current segment position and increments 
 * the segment appropriately.
 * \param seg Pointer to the segment.
 * \param msg The message to add
 */
void ssc_segment_write_msg(SscSegment *seg, MmcMsg *msg);

/**Retrives a message off the segment and increments its position 
 * accordingly.
 * \param seg The segment.
 * \param msg A reference to the message. When done, drop reference
 *            using mmc_msg_unref().
 */
MmcMsg *ssc_segment_read_msg(SscSegment *seg);


#define SSC_FN_IDX_INVALID (0xFFFF)

/**Retrives a function index
 * \param msg The message representing a function call
 * \return Function index
 */
uint16_t ssc_get_fn_idx(MmcMsg *msg);



///\}


