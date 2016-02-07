/* structure.c
 * Dealing with structures
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

#include <stdarg.h>

//These have to be kept in sync with enum SscTypeFundamentalID

const char *ssc_names[] =
{
	"uint8_t",
	"uint16_t",
	"uint32_t", 
	"uint64_t",
	"int8_t",
	"int16_t",
	"int32_t",
	"int64_t",
	"SscValFlt",
	"SscValFlt",
	"char*",
	"MmcMsg*",
	NULL
};

//Tells whether the base type requires to be freed
int ssc_base_type_requires_free(SscType type)
{
	if (! type.sym)
	{
		if ((type.fid == SSC_TYPE_FUNDAMENTAL_STRING)
		    || (type.fid == SSC_TYPE_FUNDAMENTAL_MSG))
			return 1;
	}
	else
	{
		return 1;
	}
	
	return 0;
}

//Tells whether reference of the base type is baseless
int ssc_ref_type_is_baseless(SscType type)
{
	if (! type.sym)
	{
		if ((type.fid == SSC_TYPE_FUNDAMENTAL_STRING)
		    || (type.fid == SSC_TYPE_FUNDAMENTAL_MSG))
			return 1;
	}
	
	return 0;
}

//Tells whether reading a type can fail
int ssc_type_read_can_fail(SscType type)
{
	if (type.complexity == SSC_TYPE_SEQ 
		|| type.complexity == SSC_TYPE_OPTIONAL)
		return 1;
	if (type.sym)
		return 1;
	if (type.fid == SSC_TYPE_FUNDAMENTAL_STRING)
		return 1;
	
	return 0;
}

//Writes C base type for the type, ignoring complexity
void ssc_gen_base_type(SscType type, FILE *output)
{	
	if (! type.sym)
	{
		fprintf(output, "%s", ssc_names[type.fid]);
	}
	else
	{
		fprintf(output, "%s", type.sym->name);
	}
}

//Writes C variable for given variable
void ssc_var_gen(SscVar *var, FILE *output)
{
	//For normal type
	if (var->type.complexity == SSC_TYPE_NONE)
	{
		ssc_gen_base_type(var->type, output);
		fprintf(output, " %s", var->name);
	}
	//For array
	else if (var->type.complexity > 0)
	{
		ssc_gen_base_type(var->type, output);
		fprintf(output, " %s[%d]", 
				var->name, var->type.complexity);
	}
	//Sequence
	else if (var->type.complexity == SSC_TYPE_SEQ)
	{
		fprintf(output, "struct {");
		ssc_gen_base_type(var->type, output);
		fprintf(output, "* data; uint32_t len;} %s",
				var->name);
	}
	//Reference
	else if (var->type.complexity == SSC_TYPE_OPTIONAL)
	{
		ssc_gen_base_type(var->type, output);
		
		if (ssc_ref_type_is_baseless(var->type))
		{
			fprintf(output, " %s", var->name);
		}
		else
		{
			fprintf(output, "* %s", var->name);
		}
	}
}

//Writes an expression for the base lvalue of a variable
void ssc_var_code_base_exp
	(SscVar *var, const char *prefix, FILE *c_file)
{
	//For normal type
	if (var->type.complexity == SSC_TYPE_NONE)
	{
		fprintf(c_file, "%s%s", prefix, var->name);
	}
	//For array
	else if (var->type.complexity > 0)
	{
		fprintf(c_file, "%s%s[_i]", prefix, var->name);
	}
	//Sequence
	else if (var->type.complexity == SSC_TYPE_SEQ)
	{
		fprintf(c_file, "%s%s.data[_i]", prefix, var->name);
	}
	//Reference
	else if (var->type.complexity == SSC_TYPE_OPTIONAL)
	{
		if (ssc_ref_type_is_baseless(var->type))
		{
			fprintf(c_file, "%s%s", prefix, var->name);
		}
		else
		{
			fprintf(c_file, "*(%s%s)",prefix, var->name);
		}
	}
}

//Writes test expression for reference variables
void ssc_var_code_ref_test_exp
	(SscVar *var, const char *prefix, FILE *c_file)
{
	fprintf(c_file, "%s%s",prefix, var->name);
}

//Writes code for serializing given base type. 
void ssc_var_code_for_base_write
	(SscVar *var, const char *prefix, const char *segment, 
	 FILE *c_file)
{
	if (var->type.sym)
	{
		fprintf(c_file, "%s__write(&(", var->type.sym->name);
		ssc_var_code_base_exp(var, prefix, c_file);
		fprintf(c_file, "), %s, dstream);\n", segment);
	}
	else
	{
		fprintf(c_file, "ssc_segment_write_%s(%s, ",
			ssc_type_fundamental_names[var->type.fid], 
			segment);
		ssc_var_code_base_exp(var, prefix, c_file);
		fprintf(c_file, ");\n");
	}
	
}

//Writes code for freeing given base type. 
void ssc_var_code_for_base_free
	(SscVar *var, const char *prefix, FILE *c_file)
{	
	if (! var->type.sym)
	{
		if (var->type.fid == SSC_TYPE_FUNDAMENTAL_STRING)
		{
			fprintf(c_file, "free(");
			ssc_var_code_base_exp(var, prefix, c_file);
			fprintf(c_file, ");\n");
		}
		else if (var->type.fid == SSC_TYPE_FUNDAMENTAL_MSG)
		{
			fprintf(c_file, "mmc_msg_unref(");
			ssc_var_code_base_exp(var, prefix, c_file);
			fprintf(c_file, ");\n");
		}
	}
	else
	{
		fprintf(c_file, "%s__free(&(", var->type.sym->name);
		ssc_var_code_base_exp(var, prefix, c_file);
		fprintf(c_file, "));\n");
	}
}

//Writes code for deserializing given base type. 
//Returns 1 if the code is failable (we have to write {error handler} 
//after that in that case)
int ssc_var_code_for_base_read
	(SscVar *var, const char *prefix, const char *segment, 
	 FILE *c_file)
{
	int failable = 0;
	
	if (! var->type.sym)
	{	
		if (var->type.fid == SSC_TYPE_FUNDAMENTAL_STRING)
		{
			fprintf(c_file, "if (! (");
			ssc_var_code_base_exp(var, prefix, c_file);
			fprintf(c_file, " = ssc_segment_read_string(%s)))\n",
			        segment);
			failable = 1;
		}
		else if (var->type.fid == SSC_TYPE_FUNDAMENTAL_FLT32
			|| var->type.fid == SSC_TYPE_FUNDAMENTAL_FLT64)
		{
			fprintf(c_file, "ssc_segment_read_%s(%s, &(",
				ssc_type_fundamental_names[var->type.fid],
				segment);
			ssc_var_code_base_exp(var, prefix, c_file);
			fprintf(c_file, "));\n");
		}
		else if (var->type.fid == SSC_TYPE_FUNDAMENTAL_MSG)
		{
			ssc_var_code_base_exp(var, prefix, c_file);
			fprintf(c_file, " = mmc_msg_read(%s, dstream);\n",
			        segment);
		}
		else
		{
			fprintf(c_file, "ssc_segment_read_%s(%s, ",
				ssc_type_fundamental_names[var->type.fid],
				segment);
			ssc_var_code_base_exp(var, prefix, c_file);
			fprintf(c_file, ");\n");
		}
		
	}
	else
	{
		fprintf(c_file, "if (%s__read(&(", var->type.sym->name);
		ssc_var_code_base_exp(var, prefix, c_file);
		fprintf(c_file, "), %s, dstream) < 0)\n", segment);
		failable = 1;
	}
	
	return failable;
}

//Writes code for assigning null values to references
void ssc_var_code_for_ref_null
	(SscVar *var, const char *prefix, FILE *c_file)
{	
	fprintf(c_file, "%s%s = NULL;\n", prefix, var->name);
}

//Writes code to count dynamic size of a given list of variables
void ssc_var_list_code_for_count
	(SscVarList list, const char *prefix, FILE *c_file)
{
	int i;
	
	for (i = 0; i < list.len; i++)
	{
		SscVar *iter = list.a[i];
		
		fprintf(c_file,
			"    //%s%s\n",  prefix, iter->name);
		if (iter->type.complexity == SSC_TYPE_NONE)
		{
			if (! ssc_base_type_is_constsize(iter->type))
			{
				//Only userdefined types
				fprintf(c_file, 
					"    {\n"
					"        SscDLen onesize;\n"
					"        onesize = %s__count(&(", 
					iter->type.sym->name);
				ssc_var_code_base_exp(iter, prefix, c_file);
				fprintf(c_file, 
					"));\n"
					"        size.n_bytes += onesize.n_bytes;\n"
					"        size.n_submsgs += onesize.n_submsgs;\n"
					"    }\n");
			}
		}
		else if (iter->type.complexity > 0)
		{
			if (! ssc_base_type_is_constsize(iter->type))
			{
				//Only userdefined types
				fprintf(c_file, 
					"    {\n"
					"        SscDLen onesize;\n"
					"        int _i;\n"
					"        for (_i = 0; _i < %d; _i++)\n"
					"        {\n"
					"            onesize = %s__count(&(",
					iter->type.complexity, 
					iter->type.sym->name);
				ssc_var_code_base_exp(iter, prefix, c_file);
				fprintf(c_file,
					"));\n"
					"            size.n_bytes += onesize.n_bytes;\n"
					"            size.n_submsgs += onesize.n_submsgs;\n"
					"        }\n"
					"    }\n");
			}
		}
		else if (iter->type.complexity == SSC_TYPE_SEQ)
		{
			//Find the base size
			SscDLen base_size = ssc_base_type_calc_base_size(iter->type);
			
			//We need to add up base size
			fprintf(c_file, 
				"    size.n_bytes += %d * %s%s.len;\n"
				"    size.n_submsgs += %d * %s%s.len;\n",
				(int) base_size.n_bytes, prefix, iter->name,
				(int) base_size.n_submsgs, prefix, iter->name);
			
			if (! ssc_base_type_is_constsize(iter->type))
			{
				//Only userdefined types
				fprintf(c_file, 
					"    {\n"
					"        SscDLen onesize;\n"
					"        int _i;\n"
					"        for (_i = 0; _i < %s%s.len; _i++)\n"
					"        {\n"
					"            onesize = %s__count(&(",
					prefix, iter->name, 
					iter->type.sym->name);
				ssc_var_code_base_exp(iter, prefix, c_file);
				fprintf(c_file,
					"));\n"
					"            size.n_bytes += onesize.n_bytes;\n"
					"            size.n_submsgs += onesize.n_submsgs;\n"
					"        }\n"
					"    }\n");
			}
		}
		else //reference
		{
			//Find the base size
			SscDLen base_size = ssc_base_type_calc_base_size(iter->type);
			
			fprintf(c_file, 
				"    if (");
			ssc_var_code_ref_test_exp(iter, prefix, c_file);
			fprintf(c_file, 
				")\n"
				"    {\n"
				"        size.n_bytes += %d;\n"
				"        size.n_submsgs += %d;\n",
				(int) base_size.n_bytes, (int) base_size.n_submsgs);
				
			if (! ssc_base_type_is_constsize(iter->type))
			{
				
				//Only userdefined types
				
				fprintf(c_file, 
					"        {\n"
					"            SscDLen onesize;\n"
					"            onesize = %s__count(&(", 
					iter->type.sym->name);
				ssc_var_code_base_exp(iter, prefix, c_file);
				fprintf(c_file, 
					"));\n"
					"            size.n_bytes += onesize.n_bytes;\n"
					"            size.n_submsgs += onesize.n_submsgs;\n"
					"        }\n");
			}
			fprintf(c_file, 
				"    }\n");
		}
		fprintf(c_file, "\n");
	}
}

//Writes code to serialize a given list of variables
void ssc_var_list_code_for_write
	(SscVarList list, const char *prefix, FILE *c_file)
{
	int i;
	
	for (i = 0; i < list.len; i++)
	{
		SscVar *iter = list.a[i];
		
		fprintf(c_file,
			"    //%s%s\n",  prefix, iter->name);
		
		//Simple types
		if (iter->type.complexity == SSC_TYPE_NONE)
		{
			fprintf(c_file, "    ");
			ssc_var_code_for_base_write(iter, prefix, "seg", c_file);
		}
		//Arrays
		else if (iter->type.complexity > 0)
		{
			fprintf(c_file, 
				"    {\n"
				"        int _i;\n"
				"        for (_i = 0; _i < %d; _i++)\n"
				"        {\n"
				"            ",
				iter->type.complexity);
			ssc_var_code_for_base_write(iter, prefix, "seg", c_file);
			fprintf(c_file,
				"        }\n"
				"    }\n");
				
		}
		//Sequences
		else if (iter->type.complexity == SSC_TYPE_SEQ)
		{
			//Find the base size
			SscDLen base_size = ssc_base_type_calc_base_size(iter->type);
			
			fprintf(c_file, 
				"    {\n"
				"        int _i;\n"
				"        SscSegment sub_seg;\n"
				"        \n"
				"        ssc_segment_write_uint32(seg, %s%s.len);\n"
				"        ssc_dstream_get_segment(dstream, "
				"%d * %s%s.len, %d * %s%s.len, &sub_seg);\n"
				"        for (_i = 0; _i < %s%s.len; _i++)\n"
				"        {\n"
				"            ",
				prefix, iter->name, 
				(int) base_size.n_bytes, prefix, iter->name,
				(int) base_size.n_submsgs, prefix, iter->name,
				prefix, iter->name);
			ssc_var_code_for_base_write(iter, prefix, "&sub_seg", c_file);
			fprintf(c_file,
				"        }\n"
				"    }\n");
		}
		//Reference
		else if (iter->type.complexity == SSC_TYPE_OPTIONAL)
		{
			//Find the base size
			SscDLen base_size = ssc_base_type_calc_base_size(iter->type);
			
			fprintf(c_file, 
				"    if (");
			ssc_var_code_ref_test_exp(iter, prefix, c_file);
			fprintf(c_file, 
				")\n"
				"    {\n"
				"        SscSegment sub_seg;\n"
				"        \n"
				"        ssc_segment_write_uchar(seg, 1);\n"
				"        ssc_dstream_get_segment(dstream, "
				"%d, %d, &sub_seg);\n"
				"        ", 
				(int) base_size.n_bytes, (int) base_size.n_submsgs);
			ssc_var_code_for_base_write(iter, prefix, "&sub_seg", c_file);
			fprintf(c_file,
				"    }\n"
				"    else\n"
				"    {\n"
				"        ssc_segment_write_uchar(seg, 0);\n"
				"    }\n");
		}
		fprintf(c_file, "\n");
	}
}

//Writes code to deserialize a given list of variables: main code
void ssc_var_list_code_for_read
	(SscVarList list, const char *prefix, FILE *c_file)
{
	int i;
	
	for (i = 0; i < list.len; i++)
	{
		SscVar *iter = list.a[i];
		
		fprintf(c_file,
			"    //%s%s\n",  prefix, iter->name);
		
		//Simple types
		if (iter->type.complexity == SSC_TYPE_NONE)
		{
			fprintf(c_file, 
					"    ");
			if (ssc_var_code_for_base_read(iter, prefix, "seg", c_file))
			{
				fprintf(c_file,
					"    {\n"
					"        goto _ssc_fail_%s;\n"
					"    }\n", iter->name);
			}
		}
		//Arrays
		else if (iter->type.complexity > 0)
		{
			fprintf(c_file, 
				"    {\n"
				"        int _i;\n"
				"        for (_i = 0; _i < %d; _i++)\n"
				"        {\n"
				"            ",
				iter->type.complexity);
			if (ssc_var_code_for_base_read(iter, prefix, "seg", c_file))
			{
				fprintf(c_file, 
				"            {\n");
				if (ssc_base_type_requires_free(iter->type))
				{
					fprintf(c_file, 
				"                for (_i--; _i >= 0; _i--)\n"
				"                {\n"
				"                    ");
					ssc_var_code_for_base_free(iter, prefix, c_file);
					fprintf(c_file, 
				"                }\n");
				}
				fprintf(c_file,
				"                goto _ssc_fail_%s;\n"
				"            }\n", iter->name);
			}
			fprintf(c_file,
				"        }\n"
				"    }\n");
				
		}
		//Sequences
		else if (iter->type.complexity == SSC_TYPE_SEQ)
		{
			//Find the base size
			SscDLen base_size = ssc_base_type_calc_base_size(iter->type);
				
			fprintf(c_file, 
				"    {\n"
				"        int _i;\n"
				"        SscSegment sub_seg;\n"
				"        ssc_segment_read_uint32(seg, %s%s.len);\n"
				"        if (ssc_dstream_get_segment(dstream, "
				"%d * %s%s.len, %d * %s%s.len, &sub_seg) < 0)\n"
				"            goto _ssc_fail_%s;\n"
				"        if (! (%s%s.data = (", 
				prefix, iter->name, 
				(int) base_size.n_bytes, prefix, iter->name,
				(int) base_size.n_submsgs, prefix, iter->name,
				iter->name, 
				prefix, iter->name);
			ssc_gen_base_type(iter->type, c_file);
			fprintf(c_file, " *) ssc_tryalloc(sizeof(");
			ssc_gen_base_type(iter->type, c_file);
			fprintf(c_file, ") * %s%s.len)))\n"
				"            goto _ssc_fail_%s;\n"
				"        for (_i = 0; _i < %s%s.len; _i++)\n"
				"        {\n"
				"            ",
				prefix, iter->name, 
				iter->name, 
				prefix, iter->name);
			if (ssc_var_code_for_base_read(iter, prefix, "&sub_seg", c_file))
			{
				fprintf(c_file, 
				"            {\n");
				if (ssc_base_type_requires_free(iter->type))
				{
					fprintf(c_file, 
				"                for (_i--; _i >= 0; _i--)\n"
				"                {\n"
				"                    ");
					ssc_var_code_for_base_free(iter, prefix, c_file);
					fprintf(c_file, 
				"                }\n");
				}
				fprintf(c_file,
				"                free(%s%s.data);\n"
				"                goto _ssc_fail_%s;\n"
				"            }\n", prefix, iter->name, 
					iter->name);
			}
			fprintf(c_file,
				"        }\n"
				"    }\n");
			
		}
		//Reference
		else if (iter->type.complexity == SSC_TYPE_OPTIONAL)
		{
			//Find the base size
			SscDLen base_size = ssc_base_type_calc_base_size(iter->type);
			
			fprintf(c_file, 
				"    {\n"
				"        SscSegment sub_seg;\n"
				"        char presence;\n"
				"        ssc_segment_read_uchar(seg, presence);\n"
				"        if (presence)\n"
				"        {\n"
				"            if (ssc_dstream_get_segment(dstream, "
				"%d, %d, &sub_seg) < 0)\n"
				"                goto _ssc_fail_%s;\n",
				(int) base_size.n_bytes, 
				(int) base_size.n_submsgs, 
				iter->name);
			if (! ssc_ref_type_is_baseless(iter->type))
			{
				fprintf(c_file, 
				"            if (! (%s%s = (",
					prefix, iter->name);
				ssc_gen_base_type(iter->type, c_file);
				fprintf(c_file, " *) ssc_tryalloc(sizeof(");
				ssc_gen_base_type(iter->type, c_file);
				fprintf(c_file, "))))\n"
				"            goto _ssc_fail_%s;\n",
					iter->name);
			}
			fprintf(c_file, 
				"            ");
			if (ssc_var_code_for_base_read(iter, prefix, "&sub_seg", c_file))
			{
				fprintf(c_file, 
				"            {\n");
				if (ssc_base_type_requires_free(iter->type))
				{
					fprintf(c_file, 
				"                ");
					ssc_var_code_for_base_free(iter, prefix, c_file);
				}
				if (! ssc_ref_type_is_baseless(iter->type))
					fprintf(c_file,
				"                free(%s%s);\n",
						prefix, iter->name);
				
				fprintf(c_file,
				"                goto _ssc_fail_%s;\n"
				"            }\n", iter->name);
			}
			fprintf(c_file,
				"        }\n"
				"        else\n"
				"        {\n"
				"            ");
			ssc_var_code_for_ref_null(iter, prefix, c_file);
			fprintf(c_file, 
				"        }\n"
				"    }\n");
		}
		fprintf(c_file, "\n");
	}
}

//writes code to free a variable
void ssc_var_code_for_free
	(SscVar *var, const char *prefix, FILE *c_file)
{
	fprintf(c_file, "    //%s\n", var->name);
	
	if (var->type.complexity == SSC_TYPE_NONE)
	{
		if (ssc_base_type_requires_free(var->type))
		{
			fprintf(c_file, "    ");
			ssc_var_code_for_base_free(var, prefix, c_file);
		}
	}
	else if (var->type.complexity > 0)
	{
		if (ssc_base_type_requires_free(var->type))
		{
			fprintf(c_file, 
		"    {\n"
		"        int _i;\n"
		"        for (_i = 0; _i < %d; _i++)\n"
		"        {\n"
		"            ",
				var->type.complexity);
			ssc_var_code_for_base_free(var, prefix, c_file);
			fprintf(c_file, 
		"        }\n"
		"    }\n");
		}	
	}
	else if (var->type.complexity == SSC_TYPE_SEQ)
	{
		if (ssc_base_type_requires_free(var->type))
		{
			fprintf(c_file, 
		"    {\n"
		"        int _i;\n"
		"        for (_i = 0; _i < %s%s.len; _i++)\n"
		"        {\n"
		"            ",
				prefix, var->name);
			ssc_var_code_for_base_free(var, prefix, c_file);
			fprintf(c_file, 
		"        }\n"
		"    }\n");
		}
		fprintf(c_file, 
		"    free(%s%s.data);\n",
			prefix, var->name);
	}
	else if (var->type.complexity == SSC_TYPE_OPTIONAL)
	{
		int requires_free, baseless;
		
		requires_free = ssc_base_type_requires_free(var->type);
		baseless = ssc_ref_type_is_baseless(var->type);
		
		if (requires_free || ! baseless)
		{
			fprintf(c_file, "    if (");
			ssc_var_code_ref_test_exp(var, prefix, c_file);
			fprintf(c_file, ")\n"
				"    {\n"
				"        ");
				
			if (requires_free)
			{
				ssc_var_code_for_base_free(var, prefix, c_file);
			}
			if (! baseless)
			{
				fprintf(c_file, "        free(%s%s);\n",
					prefix, var->name);
			}
			
			fprintf(c_file, "    }\n");
		}
	}
	fprintf(c_file, "\n");
}

//Writes code to deserialize a given list of variables: 
//error handling part
void ssc_var_list_code_for_read_fail
	(SscVarList list, const char *prefix, FILE *c_file)
{
	int i;
	int free_required = 0;
	
	for (i = list.len - 1; i >= 0; i--)
	{
		SscVar *iter = list.a[i];
		
		if (free_required)
			ssc_var_code_for_free(iter, prefix, c_file);
		
		if (ssc_type_read_can_fail(iter->type))
		{
			fprintf(c_file, 
		"    _ssc_fail_%s:\n", iter->name);
			free_required = 1;
		}
	}
}

//Writes code to free a given list of variables
void ssc_var_list_code_for_free
	(SscVarList list, const char *prefix, FILE *c_file)
{
	int i;
	for (i = 0; i < list.len; i++)
	{	
		ssc_var_code_for_free(list.a[i], prefix, c_file);
	}
}

//Writes C code for given structure
void ssc_struct_gen_code
	(SscSymbol *value, FILE *h_file, FILE *c_file)
{
	SscVarList fields;
	int i;
	
	//Get details
	fields = value->v.xstruct.fields;
	
	//Separator comment
	fprintf(h_file, "//%s\n", value->name);
	fprintf(c_file, "//%s\n", value->name);
	
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
	fprintf(h_file, 
		"void %s__write\n"
		"    (%s *value, SscSegment *seg, SscDStream *dstream);\n\n",
		value->name, value->name);
	fprintf(c_file, 
		"void %s__write\n"
		"    (%s *value, SscSegment *seg, SscDStream *dstream)\n"
		"{\n",
		value->name, value->name);
	
	ssc_var_list_code_for_write(fields, "value->", c_file);
	
	fprintf(c_file, "}\n\n");
	
	//Deserialization function
	fprintf(h_file, 
		"int %s__read\n"
		"    (%s *value, SscSegment *seg, SscDStream *dstream);\n\n",
		value->name, value->name);
	fprintf(c_file, 
		"int %s__read\n"
		"    (%s *value, SscSegment *seg, SscDStream *dstream)\n"
		"{\n",
		value->name, value->name);
	
	ssc_var_list_code_for_read
		(fields, "value->", c_file);
	fprintf(c_file, "\n    return 0;\n\n");
	ssc_var_list_code_for_read_fail(fields, "value->", c_file);
	fprintf(c_file, "\n    return -1;\n}\n\n");
	
	//Function to free the structure
	fprintf(h_file, 
		"void %s__free(%s *value);\n\n",
		value->name, value->name);
	fprintf(c_file, 
		"void %s__free(%s *value)\n"
		"{\n",
		value->name, value->name);
	
	ssc_var_list_code_for_free(fields, "value->", c_file);
	
	fprintf(c_file, "}\n\n");
	
	//Function to serialize a structure into a message
	fprintf(h_file, 
		"MmcMsg *%s__serialize(%s *value);\n\n",
		value->name, value->name);
	fprintf(c_file, 
		"MmcMsg *%s__serialize(%s *value)\n"
		"{\n"
		"    SscSegment seg;\n"
		"    SscDStream dstream;\n"
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
		"    msg = mmc_msg_new(dlen.n_bytes, dlen.n_submsgs);\n"
		"    \n"
		"    mmc_msg_iter(msg, &dstream);\n"
		"    ssc_dstream_get_segment(&dstream, %d, %d, &seg);\n"
		"    \n"
		"    %s__write(value, &seg, &dstream);\n"
		"    \n"
		"    return msg;\n"
		"}\n\n",
		(int) fields.base_size.n_bytes, 
			(int) fields.base_size.n_submsgs, 
		value->name);
	
	//Function to deserialize a message to get back structure
	fprintf(h_file, 
		"int %s__deserialize(MmcMsg *msg, %s *value);\n\n",
		value->name, value->name);
	fprintf(c_file, 
		"int %s__deserialize(MmcMsg *msg, %s *value)\n"
		"{\n"
		"    SscSegment seg;\n"
		"    SscDStream dstream;\n"
		"    \n"
		"    mmc_msg_iter(msg, &dstream);\n"
		"    if (ssc_dstream_get_segment(&dstream, %d, %d, &seg) < 0)\n"
		"        goto _ssc_return;\n"
		"    \n"
		"    if (%s__read(value, &seg, &dstream) < 0)\n"
		"        goto _ssc_return;\n"
		"    \n"
		"    if (! ssc_dstream_is_empty(&dstream))\n"
		"        goto _ssc_destroy_n_return;\n"
		"    \n"
		"    return 0;\n"
		"    \n"
		"_ssc_destroy_n_return:\n"
		"    %s__free(value);\n"
		"_ssc_return:\n"
		"    return -1;\n"
		"}\n\n",
		value->name, value->name,
		(int) fields.base_size.n_bytes, 
			(int) fields.base_size.n_submsgs,
		value->name,
		value->name);
}
