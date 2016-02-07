/* codegen.c
 * Code generator
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


////////////////////////////////////////
//lvalue generation

//Tells whether optional of the base type is baseless
int ssc_optional_type_is_baseless(SscType type)
{
	if (! type.sym)
	{
		if ((type.fid == SSC_TYPE_FUNDAMENTAL_STRING)
		    || (type.fid == SSC_TYPE_FUNDAMENTAL_MSG))
			return 1;
	}
	
	return 0;
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
	//Optional
	else if (var->type.complexity == SSC_TYPE_OPTIONAL)
	{
		if (ssc_optional_type_is_baseless(var->type))
		{
			fprintf(c_file, "%s%s", prefix, var->name);
		}
		else
		{
			fprintf(c_file, "*(%s%s)",prefix, var->name);
		}
	}
}

//Writes test expression for optional variables
void ssc_var_code_optional_test_exp
	(SscVar *var, const char *prefix, FILE *c_file)
{
	fprintf(c_file, "%s%s",prefix, var->name);
}

///////////////////////////////////////
//Base type

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


//Tells whether the base type requires to be freed
int ssc_base_type_requires_free(SscType type)
{
	if (type.sym)
		return 1;
	
	if ((type.fid == SSC_TYPE_FUNDAMENTAL_STRING)
		|| (type.fid == SSC_TYPE_FUNDAMENTAL_MSG))
		return 1;
	
	return 0;
}

//Tells whether reading a base type can fail
int ssc_base_type_read_can_fail(SscType type)
{
	if (type.sym)
		return 1;
	if (type.fid == SSC_TYPE_FUNDAMENTAL_STRING)
		return 1;
	
	return 0;
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

/////////////////////////////////
//Variable

//Tells whether reading a type can fail
int ssc_type_read_can_fail(SscType type)
{
	if (type.complexity == SSC_TYPE_SEQ 
		|| type.complexity == SSC_TYPE_OPTIONAL)
		return 1;
	return ssc_base_type_read_can_fail(type);
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
	//Optional
	else if (var->type.complexity == SSC_TYPE_OPTIONAL)
	{
		ssc_gen_base_type(var->type, output);
		
		if (ssc_optional_type_is_baseless(var->type))
		{
			fprintf(output, " %s", var->name);
		}
		else
		{
			fprintf(output, "* %s", var->name);
		}
	}
}

//Writes code for assigning null values to optional variables
void ssc_var_code_for_optional_null
	(SscVar *var, const char *prefix, FILE *c_file)
{	
	fprintf(c_file, "%s%s = NULL;\n", prefix, var->name);
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
		baseless = ssc_optional_type_is_baseless(var->type);
		
		if (requires_free || ! baseless)
		{
			fprintf(c_file, "    if (");
			ssc_var_code_optional_test_exp(var, prefix, c_file);
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

//Writes code to calculate dynamic size of a variable
//and increment (SscDLen size;) variable with it
void ssc_var_code_for_count
	(SscVar *var, const char *prefix, FILE *c_file)
{
	fprintf(c_file,
			"    //%s%s\n",  prefix, var->name);
	if (var->type.complexity == SSC_TYPE_NONE)
	{
		if (! ssc_base_type_is_constsize(var->type))
		{
			//Only userdefined types
			fprintf(c_file, 
				"    {\n"
				"        SscDLen onesize;\n"
				"        onesize = %s__count(&(", 
				var->type.sym->name);
			ssc_var_code_base_exp(var, prefix, c_file);
			fprintf(c_file, 
				"));\n"
				"        size.n_bytes += onesize.n_bytes;\n"
				"        size.n_submsgs += onesize.n_submsgs;\n"
				"    }\n");
		}
	}
	else if (var->type.complexity > 0)
	{
		if (! ssc_base_type_is_constsize(var->type))
		{
			//Only userdefined types
			fprintf(c_file, 
				"    {\n"
				"        SscDLen onesize;\n"
				"        int _i;\n"
				"        for (_i = 0; _i < %d; _i++)\n"
				"        {\n"
				"            onesize = %s__count(&(",
				var->type.complexity, 
				var->type.sym->name);
			ssc_var_code_base_exp(var, prefix, c_file);
			fprintf(c_file,
				"));\n"
				"            size.n_bytes += onesize.n_bytes;\n"
				"            size.n_submsgs += onesize.n_submsgs;\n"
				"        }\n"
				"    }\n");
		}
	}
	else if (var->type.complexity == SSC_TYPE_SEQ)
	{
		//Find the base size
		SscDLen base_size = ssc_base_type_calc_base_size(var->type);
		
		//We need to add up base size
		fprintf(c_file, 
			"    size.n_bytes += %d * %s%s.len;\n"
			"    size.n_submsgs += %d * %s%s.len;\n",
			(int) base_size.n_bytes, prefix, var->name,
			(int) base_size.n_submsgs, prefix, var->name);
		
		if (! ssc_base_type_is_constsize(var->type))
		{
			//Only userdefined types
			fprintf(c_file, 
				"    {\n"
				"        SscDLen onesize;\n"
				"        int _i;\n"
				"        for (_i = 0; _i < %s%s.len; _i++)\n"
				"        {\n"
				"            onesize = %s__count(&(",
				prefix, var->name, 
				var->type.sym->name);
			ssc_var_code_base_exp(var, prefix, c_file);
			fprintf(c_file,
				"));\n"
				"            size.n_bytes += onesize.n_bytes;\n"
				"            size.n_submsgs += onesize.n_submsgs;\n"
				"        }\n"
				"    }\n");
		}
	}
	else //optional
	{
		//Find the base size
		SscDLen base_size = ssc_base_type_calc_base_size(var->type);
		
		fprintf(c_file, 
			"    if (");
		ssc_var_code_optional_test_exp(var, prefix, c_file);
		fprintf(c_file, 
			")\n"
			"    {\n"
			"        size.n_bytes += %d;\n"
			"        size.n_submsgs += %d;\n",
			(int) base_size.n_bytes, (int) base_size.n_submsgs);
			
		if (! ssc_base_type_is_constsize(var->type))
		{
			
			//Only userdefined types
			
			fprintf(c_file, 
				"        {\n"
				"            SscDLen onesize;\n"
				"            onesize = %s__count(&(", 
				var->type.sym->name);
			ssc_var_code_base_exp(var, prefix, c_file);
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

//Writes code to serialize a given variable
void ssc_var_code_for_write
	(SscVar *var, const char *prefix, FILE *c_file)
{
	fprintf(c_file,
		"    //%s%s\n",  prefix, var->name);
	
	//Simple types
	if (var->type.complexity == SSC_TYPE_NONE)
	{
		fprintf(c_file, "    ");
		ssc_var_code_for_base_write(var, prefix, "seg", c_file);
	}
	//Arrays
	else if (var->type.complexity > 0)
	{
		fprintf(c_file, 
			"    {\n"
			"        int _i;\n"
			"        for (_i = 0; _i < %d; _i++)\n"
			"        {\n"
			"            ",
			var->type.complexity);
		ssc_var_code_for_base_write(var, prefix, "seg", c_file);
		fprintf(c_file,
			"        }\n"
			"    }\n");
			
	}
	//Sequences
	else if (var->type.complexity == SSC_TYPE_SEQ)
	{
		//Find the base size
		SscDLen base_size = ssc_base_type_calc_base_size(var->type);
		
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
			prefix, var->name, 
			(int) base_size.n_bytes, prefix, var->name,
			(int) base_size.n_submsgs, prefix, var->name,
			prefix, var->name);
		ssc_var_code_for_base_write(var, prefix, "&sub_seg", c_file);
		fprintf(c_file,
			"        }\n"
			"    }\n");
	}
	//optional
	else if (var->type.complexity == SSC_TYPE_OPTIONAL)
	{
		//Find the base size
		SscDLen base_size = ssc_base_type_calc_base_size(var->type);
		
		fprintf(c_file, 
			"    if (");
		ssc_var_code_optional_test_exp(var, prefix, c_file);
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
		ssc_var_code_for_base_write(var, prefix, "&sub_seg", c_file);
		fprintf(c_file,
			"    }\n"
			"    else\n"
			"    {\n"
			"        ssc_segment_write_uchar(seg, 0);\n"
			"    }\n");
	}
	fprintf(c_file, "\n");
}

//Writes code to read a variable, 
//jumping to _ssc_fail_<var_name> if it fails
void ssc_var_code_for_read
	(SscVar *var, const char *prefix, FILE *c_file)
{
	fprintf(c_file,
		"    //%s%s\n",  prefix, var->name);
	
	//Simple types
	if (var->type.complexity == SSC_TYPE_NONE)
	{
		fprintf(c_file, 
				"    ");
		if (ssc_var_code_for_base_read(var, prefix, "seg", c_file))
		{
			fprintf(c_file,
				"    {\n"
				"        goto _ssc_fail_%s;\n"
				"    }\n", var->name);
		}
	}
	//Arrays
	else if (var->type.complexity > 0)
	{
		fprintf(c_file, 
			"    {\n"
			"        int _i;\n"
			"        for (_i = 0; _i < %d; _i++)\n"
			"        {\n"
			"            ",
			var->type.complexity);
		if (ssc_var_code_for_base_read(var, prefix, "seg", c_file))
		{
			fprintf(c_file, 
			"            {\n");
			if (ssc_base_type_requires_free(var->type))
			{
				fprintf(c_file, 
			"                for (_i--; _i >= 0; _i--)\n"
			"                {\n"
			"                    ");
				ssc_var_code_for_base_free(var, prefix, c_file);
				fprintf(c_file, 
			"                }\n");
			}
			fprintf(c_file,
			"                goto _ssc_fail_%s;\n"
			"            }\n", var->name);
		}
		fprintf(c_file,
			"        }\n"
			"    }\n");
			
	}
	//Sequences
	else if (var->type.complexity == SSC_TYPE_SEQ)
	{
		//Find the base size
		SscDLen base_size = ssc_base_type_calc_base_size(var->type);
			
		fprintf(c_file, 
			"    {\n"
			"        int _i;\n"
			"        SscSegment sub_seg;\n"
			"        ssc_segment_read_uint32(seg, %s%s.len);\n"
			"        if (ssc_dstream_get_segment(dstream, "
			"%d * %s%s.len, %d * %s%s.len, &sub_seg) < 0)\n"
			"            goto _ssc_fail_%s;\n"
			"        if (! (%s%s.data = (", 
			prefix, var->name, 
			(int) base_size.n_bytes, prefix, var->name,
			(int) base_size.n_submsgs, prefix, var->name,
			var->name, 
			prefix, var->name);
		ssc_gen_base_type(var->type, c_file);
		fprintf(c_file, " *) ssc_tryalloc(sizeof(");
		ssc_gen_base_type(var->type, c_file);
		fprintf(c_file, ") * %s%s.len)))\n"
			"            goto _ssc_fail_%s;\n"
			"        for (_i = 0; _i < %s%s.len; _i++)\n"
			"        {\n"
			"            ",
			prefix, var->name, 
			var->name, 
			prefix, var->name);
		if (ssc_var_code_for_base_read(var, prefix, "&sub_seg", c_file))
		{
			fprintf(c_file, 
			"            {\n");
			if (ssc_base_type_requires_free(var->type))
			{
				fprintf(c_file, 
			"                for (_i--; _i >= 0; _i--)\n"
			"                {\n"
			"                    ");
				ssc_var_code_for_base_free(var, prefix, c_file);
				fprintf(c_file, 
			"                }\n");
			}
			fprintf(c_file,
			"                free(%s%s.data);\n"
			"                goto _ssc_fail_%s;\n"
			"            }\n", prefix, var->name, 
				var->name);
		}
		fprintf(c_file,
			"        }\n"
			"    }\n");
		
	}
	//optional
	else if (var->type.complexity == SSC_TYPE_OPTIONAL)
	{
		//Find the base size
		SscDLen base_size = ssc_base_type_calc_base_size(var->type);
		
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
			var->name);
		if (! ssc_optional_type_is_baseless(var->type))
		{
			fprintf(c_file, 
			"            if (! (%s%s = (",
				prefix, var->name);
			ssc_gen_base_type(var->type, c_file);
			fprintf(c_file, " *) ssc_tryalloc(sizeof(");
			ssc_gen_base_type(var->type, c_file);
			fprintf(c_file, "))))\n"
			"            goto _ssc_fail_%s;\n",
				var->name);
		}
		fprintf(c_file, 
			"            ");
		if (ssc_var_code_for_base_read(var, prefix, "&sub_seg", c_file))
		{
			fprintf(c_file, 
			"            {\n");
			if (ssc_base_type_requires_free(var->type))
			{
				fprintf(c_file, 
			"                ");
				ssc_var_code_for_base_free(var, prefix, c_file);
			}
			if (! ssc_optional_type_is_baseless(var->type))
				fprintf(c_file,
			"                free(%s%s);\n",
					prefix, var->name);
			
			fprintf(c_file,
			"                goto _ssc_fail_%s;\n"
			"            }\n", var->name);
		}
		fprintf(c_file,
			"        }\n"
			"        else\n"
			"        {\n"
			"            ");
		ssc_var_code_for_optional_null(var, prefix, c_file);
		fprintf(c_file, 
			"        }\n"
			"    }\n");
	}
	fprintf(c_file, "\n");
}

/////////////////////////////////
//Variable list

//Writes code to count dynamic size of a given list of variables
void ssc_var_list_code_for_count
	(SscVarList list, const char *prefix, FILE *c_file)
{
	int i;
	
	for (i = 0; i < list.len; i++)
	{
		ssc_var_code_for_count(list.a[i], prefix, c_file);
	}
}

//Writes code to serialize a given list of variables
void ssc_var_list_code_for_write
	(SscVarList list, const char *prefix, FILE *c_file)
{
	int i;
	
	for (i = 0; i < list.len; i++)
	{
		ssc_var_code_for_write(list.a[i], prefix, c_file);
	}
}

//Writes code to deserialize a given list of variables: main code
void ssc_var_list_code_for_read
	(SscVarList list, const char *prefix, FILE *c_file)
{
	int i;
	
	for (i = 0; i < list.len; i++)
	{
		ssc_var_code_for_read(list.a[i], prefix, c_file);
	}
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

