/* serialize.c
 * All the serialization stuff
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


void ssc_msg_iter_init(SscMsgIter *self, MmcMsg *msg)
{
	self->bytes = msg->mem;
	self->bytes_lim = self->bytes + msg->mem_len;
	self->submsgs = msg->submsgs;
	self->submsgs_lim = self->submsgs + msg->submsgs_len;
}

MdslStatus ssc_msg_iter_get_segment
	(SscMsgIter *self, size_t n_bytes, size_t n_submsgs, 
	 SscSegment *res)
{
	if (self->bytes + n_bytes > self->bytes_lim
		|| self->submsgs + n_submsgs > self->submsgs_lim)
		return MDSL_FAILURE;
	
	res->bytes = self->bytes;
	res->submsgs = self->submsgs;
	self->bytes += n_bytes;
	self->submsgs += n_submsgs;
	
	return MDSL_SUCCESS;
}

//Floating point values
void ssc_segment_write_flt32(SscSegment *seg, SscValFlt val)
{
	uint32_t inter;
	
	switch (val.type)
	{
	case SSC_FLT_ZERO:
	case SSC_FLT_NORMAL:
		inter = ssc_double_to_flt32(val.val);
		break;
	case SSC_FLT_NAN:
		inter = ssc_flt32_nan;
		break;
	case SSC_FLT_INFINITE:
		inter = ssc_flt32_infinity;
		break;
	case SSC_FLT_NEG_INFINITE:
		inter = ssc_flt32_neg_infinity;
	default:
		ssc_error("Invalid type %d", val.type);
	}
	
	ssc_segment_write_uint32(seg, inter);
}

void ssc_segment_read_flt32(SscSegment *seg, SscValFlt *val)
{
	uint32_t inter = ssc_segment_read_uint32(seg);
	val->type = ssc_flt32_classify(inter);
	val->val = ssc_double_from_flt32(inter);
}

void ssc_segment_write_flt64(SscSegment *seg, SscValFlt val)
{
	uint64_t inter;
	
	switch (val.type)
	{
	case SSC_FLT_ZERO:
	case SSC_FLT_NORMAL:
		inter = ssc_double_to_flt64(val.val);
		break;
	case SSC_FLT_NAN:
		inter = ssc_flt64_nan;
		break;
	case SSC_FLT_INFINITE:
		inter = ssc_flt64_infinity;
		break;
	case SSC_FLT_NEG_INFINITE:
		inter = ssc_flt64_neg_infinity;
	default:
		ssc_error("Invalid type %d", val.type);
	}
	
	ssc_segment_write_uint64(seg, inter);
}

void ssc_segment_read_flt64(SscSegment *seg, SscValFlt *val)
{
	uint64_t inter = ssc_segment_read_uint64(seg);
	val->type = ssc_flt64_classify(inter);
	val->val = ssc_double_from_flt64(inter);
}

//Strings
void ssc_segment_write_string(SscSegment *seg, char *val)
{
	MmcMsg *submsg;
	size_t len;
	
	//Copy string into submessage
	len = strlen(val);
	submsg = mmc_msg_newa(len, 0);
	memcpy(submsg->mem, val, len);
		
	//Add to segment
	*seg->submsgs = submsg;
	seg->submsgs++;
}

char *ssc_segment_read_string(SscSegment *seg)
{
	MmcMsg *submsg;
	int len, i;
	char *check, *res;
	
	//Fetch it
	submsg = *seg->submsgs;
	len = submsg->mem_len;
	
	//get string and verify
	if (submsg->submsgs_len > 0)
		return NULL;
	check = (char *) submsg->mem;
	for (i = 0; i < len; i++)	
		if (check[i] == '\0')
			return NULL;
	res = mdsl_tryalloc(len + 1);
	memcpy(res, check, len);
	res[len] = '\0';
	
	//Increment
	seg->submsgs++;
	
	return res;
}

void ssc_segment_write_msg(SscSegment *seg, MmcMsg *msg)
{
	*seg->submsgs = msg;
	mmc_msg_ref(msg);
	seg->submsgs++;
}

MmcMsg *ssc_segment_read_msg(SscSegment *seg)
{
	MmcMsg *res = *seg->submsgs;
	mmc_msg_ref(res);
	seg->submsgs++;
	return res;
}

uint16_t ssc_get_fn_idx(MmcMsg *msg)
{
	uint16_t res;
	
	if (msg->mem_len < 2)
		return SSC_FN_IDX_INVALID;
	
	res = ssc_uint16_load_le(msg->mem);
	
	return res;
}
