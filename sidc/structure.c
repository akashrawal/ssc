/* structure.c
 * Dealing with structures
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

//Writes C code for given structure

void ssc_struct_gen_declaration(SscSymbol *value, FILE *h_file)
{
	SscVarList fields;
	int i;
	
	//Get details
	fields = value->v.xstruct.fields;
	
	//Separator comment
	fprintf(h_file, "//%s\n", value->name);
	
	//Prevent multiple declarations
	fprintf(h_file, 
		"#ifndef SSC_STRUCT__%s__DECLARED\n"
		"#define SSC_STRUCT__%s__DECLARED\n\n",
		value->name, value->name);
	
	//Structure definition
	fprintf(h_file, "typedef struct\n{\n");
	
	for (i = 0; i < fields.len; i++)
	{
		fprintf(h_file, "\t");
		ssc_var_gen(fields.a[i], h_file);
		fprintf(h_file, ";\n");
	}
	
	fprintf(h_file, "} %s;\n\n", value->name);
	
	if (! fields.constsize)
	{
		//Structure is not of constant size, have to write functions
		fprintf(h_file, 
			"SscDLen %s__count(%s *value);\n\n",
			value->name, value->name);
	}
	
	//Serialization function
	fprintf(h_file, 
		"void %s__write\n"
		"    (%s *value, SscSegment *seg, SscMsgIter *msg_iter);\n\n",
		value->name, value->name);
	
	//Deserialization function
	fprintf(h_file, 
		"int %s__read\n"
		"    (%s *value, SscSegment *seg, SscMsgIter *msg_iter);\n\n",
		value->name, value->name);
	
	//Function to free the structure
	fprintf(h_file, 
		"void %s__free(%s *value);\n\n",
		value->name, value->name);
	
	//Function to serialize a structure into a message
	fprintf(h_file, 
		"MmcMsg *%s__serialize(%s *value);\n\n",
		value->name, value->name);
		
	//Function to deserialize a message to get back structure
	fprintf(h_file, 
		"MmcStatus %s__deserialize(MmcMsg *msg, %s *value);\n\n",
		value->name, value->name);
	
	//Prevent multiple declarations: end
	fprintf(h_file, 
		"#endif //SSC_STRUCT__%s__DECLARED\n\n",
		value->name);
}

void ssc_struct_gen_code
	(SscSymbol *value, FILE *c_file)
{
	SscVarList fields;
	
	//Get details
	fields = value->v.xstruct.fields;
	
	//Separator comment
	fprintf(c_file, "//%s\n", value->name);
	
	if (! fields.constsize)
	{
		//Structure is not of constant size, have to write functions
		
		fprintf(c_file, 
			"SscDLen %s__count(%s *value)\n"
			"{\n"
			"    SscDLen size = {0, 0};\n\n",
			value->name, value->name);
		
		ssc_var_list_code_for_count(fields, "value->", c_file);
		
		fprintf(c_file, 
			"\n    return size;\n"
			"}\n\n");	
	}
	
	//Serialization function
	fprintf(c_file, 
		"void %s__write\n"
		"    (%s *value, SscSegment *seg, SscMsgIter *msg_iter)\n"
		"{\n",
		value->name, value->name);
	
	ssc_var_list_code_for_write(fields, "value->", c_file);
	
	fprintf(c_file, "}\n\n");
	
	//Deserialization function
	fprintf(c_file, 
		"MmcStatus %s__read\n"
		"    (%s *value, SscSegment *seg, SscMsgIter *msg_iter)\n"
		"{\n",
		value->name, value->name);
	
	ssc_var_list_code_for_read
		(fields, "value->", c_file);
	fprintf(c_file, "\n    return 0;\n\n");
	ssc_var_list_code_for_read_fail(fields, "value->", 0, c_file);
	fprintf(c_file, "\n    return -1;\n}\n\n");
	
	//Function to free the structure
	fprintf(c_file, 
		"void %s__free(%s *value)\n"
		"{\n",
		value->name, value->name);
	
	ssc_var_list_code_for_free(fields, "value->", c_file);
	
	fprintf(c_file, "}\n\n");
	
	//Function to serialize a structure into a message
	fprintf(c_file, 
		"MmcMsg *%s__serialize(%s *value)\n"
		"{\n"
		"    SscSegment seg;\n"
		"    SscMsgIter msg_iter;\n"
		"    SscDLen dlen = {%d, %d};\n"
		"    MmcMsg *msg;\n"
		"    \n",
		value->name, value->name, 
		(int) fields.base_size.n_bytes, 
			(int) fields.base_size.n_submsgs);
	if (! fields.constsize)
	{
		fprintf(c_file, 
		"    //Size computation\n"
		"    {\n"
		"        SscDLen dynamic;\n"
		"        dynamic = %s__count(value);\n"
		"        dlen.n_bytes += dynamic.n_bytes;\n"
		"        dlen.n_submsgs += dynamic.n_submsgs;\n"
		"    }\n"
		"    \n",
		value->name);
	}
	fprintf(c_file, 
		"    msg = mmc_msg_newa(dlen.n_bytes, dlen.n_submsgs);\n"
		"    \n"
		"    ssc_msg_iter_init(&msg_iter, msg);\n"
		"    ssc_msg_iter_get_segment(&msg_iter, %d, %d, &seg);\n"
		"    \n"
		"    %s__write(value, &seg, &msg_iter);\n"
		"    \n"
		"    return msg;\n"
		"}\n\n",
		(int) fields.base_size.n_bytes, 
			(int) fields.base_size.n_submsgs, 
		value->name);
	
	//Function to deserialize a message to get back structure
	fprintf(c_file, 
		"int %s__deserialize(MmcMsg *msg, %s *value)\n"
		"{\n"
		"    SscSegment seg;\n"
		"    SscMsgIter msg_iter;\n"
		"    \n"
		"    ssc_msg_iter_init(&msg_iter, msg);\n"
		"    if (ssc_msg_iter_get_segment(&msg_iter, %d, %d, &seg) \n"
		"            == MMC_FAILURE)\n"
		"        goto _ssc_return;\n"
		"    \n"
		"    if (%s__read(value, &seg, &msg_iter) < 0)\n"
		"        goto _ssc_return;\n"
		"    \n"
		"    if (! ssc_msg_iter_at_end(&msg_iter))\n"
		"        goto _ssc_destroy_n_return;\n"
		"    \n"
		"    return MMC_SUCCESS;\n"
		"    \n"
		"_ssc_destroy_n_return:\n"
		"    %s__free(value);\n"
		"_ssc_return:\n"
		"    return MMC_FAILURE;\n"
		"}\n\n",
		value->name, value->name,
		(int) fields.base_size.n_bytes, 
			(int) fields.base_size.n_submsgs,
		value->name,
		value->name);
}


