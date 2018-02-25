/* interface.c
 * Dealing with interfaces
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

#include "incl.h"

//TODO: Test code for zero arguments

//Writes header code for given interface

typedef struct
{
	char *sn; //Struct name
	char *sf; //Serialization function name
	char *df; //Deserialization function name
	char *ff; //Free function name
} ArgsType;

static ArgsType args_in = 
{
	"__in_args",
	"__create_msg",
	"__read_msg",
	"__in_args_free"
};

static ArgsType args_out =
{
	"__out_args", 
	"__create_reply", 
	"__read_reply",
	"__out_args_free"
};


static void ssc_arglist_gen_declaration
	(SscVarList args, 
	const char *name_prefix, const ArgsType args_type,
	FILE *h_file)
{
	int i;
	
	//Structure definition
	fprintf(h_file, "typedef struct\n{\n");
	
	for (i = 0; i < args.len; i++)
	{
		fprintf(h_file, "\t");
		ssc_var_gen(args.a[i], h_file);
		fprintf(h_file, ";\n");
	}
	
	fprintf(h_file, "} %s%s;\n\n", name_prefix, args_type.sn);
	
	//Function to free the structure
	fprintf(h_file, 
		"void %s%s(%s%s *value);\n\n",
		name_prefix, args_type.ff, name_prefix, args_type.sn);
	
	//Function to serialize a structure into a message
	fprintf(h_file, 
		"MmcMsg *%s%s(%s%s *value);\n\n",
		name_prefix, args_type.sf, name_prefix, args_type.sn);
		
	//Function to deserialize a message to get back structure
	fprintf(h_file, 
		"MmcStatus %s%s(MmcMsg *msg, %s%s *value);\n\n",
		name_prefix, args_type.df, name_prefix, args_type.sn);
}

static void ssc_arglist_gen_code
	(SscVarList args, 
	const char *name_prefix, const ArgsType args_type,
	int prefix_val, 
	FILE *c_file)
{
	
	//Function to free the structure
	fprintf(c_file, 
		"void %s%s(%s%s *value)\n"
		"{\n",
		name_prefix, args_type.ff, name_prefix, args_type.sn);
	
	ssc_var_list_code_for_free(args, "value->", c_file);
	
	fprintf(c_file, "}\n\n");
	
	//Function to serialize a structure into a message
	fprintf(c_file, 
		"MmcMsg *%s%s(%s%s *value)\n"
		"{\n"
		"    SscSegment seg[1];\n"
		"    SscMsgIter msg_iter[1];\n"
		"    SscDLen size = {%d + SSC_PREFIX_SIZE, %d};\n"
		"    MmcMsg *msg;\n"
		"    \n",
		name_prefix, args_type.sf, name_prefix, args_type.sn, 
		(int) args.base_size.n_bytes, 
		(int) args.base_size.n_submsgs);
	if (! args.constsize)
	{
		fprintf(c_file, 
		"    //Size computation\n");
		
		ssc_var_list_code_for_count(args, "value->", c_file);
		
		fprintf(c_file, 
		"    \n");
	}
	//TODO: Honor SSC_PREFIX_SIZE > 1
	fprintf(c_file, 
		"    msg = mmc_msg_newa(size.n_bytes, size.n_submsgs);\n"
		"    \n"
		"    ssc_msg_iter_init(msg_iter, msg);\n"
		"    ssc_msg_iter_get_segment(msg_iter, %d + SSC_PREFIX_SIZE, %d, seg);\n"
		"    \n"
		"    ssc_segment_write_uint8(seg, %d); //name_prefix\n"
		"    \n",
			(int) args.base_size.n_bytes, 
			(int) args.base_size.n_submsgs,
			prefix_val);
	ssc_var_list_code_for_write(args, "value->", c_file);
	fprintf(c_file, 
		"    \n"
		"    return msg;\n"
		"}\n\n");
	
	//Function to deserialize a message to get back structure
	fprintf(c_file, 
		"MmcStatus %s%s(MmcMsg *msg, %s%s *value)\n"
		"{\n"
		"    SscSegment seg[1];\n"
		"    SscMsgIter msg_iter[1];\n"
		"    uint8_t prefix_val;\n"
		"    \n"
		"    ssc_msg_iter_init(msg_iter, msg);\n"
		"    if (ssc_msg_iter_get_segment(msg_iter, SSC_PREFIX_SIZE, 0, seg) < 0)\n"
		"        goto _ssc_return;\n"
		"    prefix_val = ssc_segment_read_uint8(seg);\n"
		"    if (prefix_val != %d)\n"
		"        goto _ssc_return;\n"
		"    if (ssc_msg_iter_get_segment(msg_iter, %d, %d, seg) < 0)\n"
		"        goto _ssc_return;\n"
		"    \n",
		name_prefix, args_type.df, name_prefix, args_type.sn,
		prefix_val, 
		(int) args.base_size.n_bytes, 
		(int) args.base_size.n_submsgs);
		
	ssc_var_list_code_for_read(args, "value->", c_file);
	
	fprintf(c_file,
		"    if (! ssc_msg_iter_at_end(msg_iter))\n"
		"        goto _ssc_destroy_n_return;\n"
		"    \n"
		"    return MMC_SUCCESS;\n"
		"    \n"
		"_ssc_destroy_n_return:\n");
	
	ssc_var_list_code_for_read_fail(args, "value->", 1, c_file);
	
	fprintf(c_file, 
		"_ssc_return:\n"
		"    return MMC_FAILURE;\n"
		"}\n\n");
}

//Count all functions (including those in parent interfaces
static int ssc_count_all_fns(SscSymbol *value)
{
	int res;
	SscSymbol *iter;
	
	res = 0;
	iter = value;
	while (iter)
	{
		res += iter->v.xiface.fns_len;
		
		iter = iter->v.xiface.parent;
	}
	
	return res;
}

//Array containing names of all functions
mmc_declare_array(char *, SscFnList, ssc_fn_list);

static void ssc_fn_list_create(SscFnList *fl, SscSymbol *value)
{
	SscInterface *iface = &(value->v.xiface);
	int i;
	
	ssc_fn_list_init(fl);
	
	for (i = 0; i < iface->fns_len; i++)
	{
		char *name = malloc
			(strlen(value->name) + 2 
			+ strlen(iface->fns[i]->name) + 1);
		strcpy(name, value->name);
		strcat(name, "__");
		strcat(name, iface->fns[i]->name);
		
		ssc_fn_list_append(fl, name);
	}
}

static void ssc_fn_list_create_with_parents
	(SscFnList *fl, SscSymbol *value)
{
	SscSymbol *one_iface;
	int i;
	int start;
	
	start = ssc_count_all_fns(value);
	ssc_fn_list_init(fl);
	ssc_fn_list_resize(fl, start);
	
	one_iface = value;
	while (one_iface)
	{
		SscInterface *iface = &(value->v.xiface);
		start -= iface->fns_len;
		
		for (i = 0; i < iface->fns_len; i++)
		{
			char *name = malloc
				(strlen(value->name) + 2 
				+ strlen(iface->fns[i]->name) + 1);
			strcpy(name, value->name);
			strcat(name, "__");
			strcat(name, iface->fns[i]->name);
			
			fl->data[start + i] = name;
		}
		
		one_iface = one_iface->v.xiface.parent;
	}
}

static void ssc_fn_list_destroy(SscFnList *fl)
{
	int i;
	int len = ssc_fn_list_size(fl);
	
	for (i = 0; i < len; i++)
		free(fl->data[i]);
	
	free(fl->data);
}

void ssc_iface_gen_declaration(SscSymbol *value, FILE *h_file)
{
	SscInterface *iface = &(value->v.xiface);
	int i;
	int cum_n_fns;
	SscFnList fl[1];
	
	//Separator comment
	fprintf(h_file, 
		"//Interface %s\n"
		"\n",
		value->name);
	
	//Prevent multiple declarations
	fprintf(h_file, 
		"#ifndef SSC_INTERFACE__%s__DECLARED\n"
		"#define SSC_INTERFACE__%s__DECLARED\n\n",
		value->name, value->name);
	
	//Write skeleton declaration
	fprintf(h_file, 
		"extern SscSkel %s[1];\n"
		"\n",
		value->name);
	
	//Count the cumulative number of functions (including the parents)
	//We need this to give each function an id
	cum_n_fns = ssc_count_all_fns(value);
	
	//Generate names of all functions
	ssc_fn_list_create(fl, value);
	
	//Write declaration for each function.
	for (i = 0; i < iface->fns_len; i++)
	{
		//Separator comment
		fprintf(h_file, 
		"//Function %s::%s\n"
		"#define %s__ID (%d)\n"
		"\n", 
			value->name, iface->fns[i]->name,
			fl->data[i], (int) (cum_n_fns - iface->fns_len + i));
		
		ssc_arglist_gen_declaration
			(iface->fns[i]->in, fl->data[i], args_in, h_file);
		ssc_arglist_gen_declaration
			(iface->fns[i]->out, fl->data[i], args_out, h_file);
	}
	
	//Prevent multiple declarations: end
	fprintf(h_file, 
		"#endif //SSC_INTERFACE__%s__DECLARED\n\n",
		value->name);
	
	//Garbage collection
	ssc_fn_list_destroy(fl);
}

//Writes C code for given structure
void ssc_iface_gen_code
	(SscSymbol *value, FILE *c_file)
{
	SscInterface *iface = &(value->v.xiface);
	int i;
	SscFnList fl[1];
	int fl_len;
	
	//Generate names of all functions including parent functions
	ssc_fn_list_create_with_parents(fl, value);
	fl_len = ssc_fn_list_size(fl);
	
	//Write separator comment
	fprintf(c_file, 
		"//Interface %s\n"
		"\n",
		value->name);
	
	//Write skeleton definition
	fprintf(c_file, 
		"static SscSStub ssc_sstub_array__%s[%d] = {\n",
		value->name, (int) ssc_fn_list_size(fl));
	for (i = 0; i < fl_len; i++)
	{
		fprintf(c_file, 
		"    {\n"
		"        sizeof(%s__in_args),\n"
		"        (SscReadMsgFn) %s__read_msg,\n"
		"        (SscCreateReplyFn) %s__create_reply,\n"
		"        (SscArgsFreeFn) %s__in_args_free\n"
		"    }\n",
			fl->data[i],
			fl->data[i],
			fl->data[i],
			fl->data[i]);
	}
	fprintf(c_file, 
		"};\n"
		"SscSkel %s[1] = {{\n"
		"    %d,\n"
		"    ssc_sstub_array__%s\n"
		"}};\n\n",
		value->name,
		fl_len, 
		value->name);
	
	//Write code for each function.
	for (i = 0; i < iface->fns_len; i++)
	{
		int base = fl_len - iface->fns_len;
		
		//Separator comment
		fprintf(c_file, 
		"//function %s::%s\n"
		"\n", 
			value->name, iface->fns[i]->name);
	
		//Write code for functions
		ssc_arglist_gen_code
			(iface->fns[i]->in, fl->data[base + i], args_in, base + i, c_file);
		ssc_arglist_gen_code
			(iface->fns[i]->out, fl->data[base + i], args_out, 0, c_file);
	}
	
	//Garbage collection
	ssc_fn_list_destroy(fl);
}



