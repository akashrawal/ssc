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

//Writes C base type for the type, ignoring complexity
void ssc_gen_base_type(SscType type, FILE *output);

//Writes C variable for given variable
void ssc_var_gen(SscVar *var, FILE *output);

//Writes code to count size of a given list of variables
void ssc_var_list_code_for_count
	(SscVarList list, const char *prefix, FILE *c_file);
	
//Writes code to serialize a given list of variables
void ssc_var_list_code_for_write
	(SscVarList list, const char *prefix, FILE *c_file);

//Writes code to deserialize a given list of variables: main code
void ssc_var_list_code_for_read
	(SscVarList list, const char *prefix, FILE *c_file);

//Writes code to deserialize a given list of variables: 
//error handling part
void ssc_var_list_code_for_read_fail
	(SscVarList list, const char *prefix, int with_free, FILE *c_file);

//Writes code to free a given list of variables
void ssc_var_list_code_for_free
	(SscVarList list, const char *prefix, FILE *c_file);
	

