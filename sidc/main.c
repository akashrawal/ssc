/* main.c
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

#include "incl.h"



#include <errno.h>

//TODO: Improve this prototype

int main(int argc, char *argv[])
{
	SscSymbolDB *db;
	
	
	char *c_file, *h_file, *infile, *outprefix;
	
	//Read arguments
	if (argc != 2 && argc != 3)
	{
		printf("Usage: %s filename [output_prefix]\n", argv[0]);
		exit(1);
	}
	infile = argv[1];
	if (argc == 3)
	{
		outprefix = argv[2];
	}
	else
	{
		outprefix = infile;
		char *tmp = outprefix;

		tmp = strrchr(outprefix, '/');
		if (tmp)
			outprefix = tmp + 1;
#ifdef _WIN32
		tmp = strrchr(outprefix, '\\');
		if (tmp)
			outprefix = tmp + 1;
#endif
	}
	
	//Compute file names
	{
		int outprefix_len;
		
		outprefix_len = strlen(outprefix);
		
		c_file = mdsl_alloc(outprefix_len + 3);
		strcpy(c_file, outprefix);
		strcpy(c_file + outprefix_len, ".c");
		
		h_file = mdsl_alloc(outprefix_len + 3);
		strcpy(h_file, outprefix);
		strcpy(h_file + outprefix_len, ".h");
	}
	
	//Create database
	db = ssc_symbol_db_new();
	
	//Parse the input file
	if (ssc_parser_parse_needed(db, stderr, infile)	
			!= MDSL_SUCCESS)
	{
		fprintf(stderr, "Parsing failed, exiting\n");
		exit(1);
	}
	
	//Produce output
	{
		FILE *h_stream, *c_stream;
		SscSequencer *seqr;
		SscSymbolArray symbols;
		SscSymbol *iter;
		SscFileData file_data;
		int i; 
		
		//Deal with scanning, parsing, semantics, 
		//and generate symbol tree
		seqr = ssc_sequencer_new(db);
		ssc_sequencer_process_file(seqr, infile);
		symbols = ssc_sequencer_destroy(seqr);
		file_data = ssc_symbol_db_get_file_data(db, infile);
		
		//Open output files
		h_stream = fopen(h_file, "w");
		if (! h_stream)
		{
			fprintf(stderr, "Cannot open %s: %s",
				h_file, strerror(errno));
			exit(1);
		}
		c_stream = fopen(c_file, "w");
		if (! c_stream)
		{
			fprintf(stderr, "Cannot open %s: %s",
				c_file, strerror(errno));
			exit(1);
		}
		
		//Write boilerplates
		fprintf(c_stream, "#include <ssc/ssc.h>\n\n");
		
		//Generate declarations
		for (i = 0; i < symbols.len; i++)
		{
			if (symbols.d[i]->type == SSC_SYMBOL_STRUCT)
			{
				ssc_struct_gen_declaration(symbols.d[i], h_stream);
				ssc_struct_gen_declaration(symbols.d[i], c_stream);
			}
			else if (symbols.d[i]->type == SSC_SYMBOL_INTERFACE)		
			{
				ssc_iface_gen_declaration(symbols.d[i], h_stream);
				ssc_iface_gen_declaration(symbols.d[i], c_stream);
			}
		}
		
		//Generate code
		for (iter = file_data.list; iter; iter = iter->next)
		{
			if (iter->type == SSC_SYMBOL_STRUCT)
				ssc_struct_gen_code(iter, c_stream);
			else if (iter->type == SSC_SYMBOL_INTERFACE)
				ssc_iface_gen_code(iter, c_stream);
		}
		
		free(symbols.d);
		fclose(h_stream);
		fclose(c_stream);
	}
	
	
	
	//Free file names
	free(c_file);
	free(h_file);
	
	//Free symbol database
	ssc_symbol_db_unref(db);
	
	return 0;
}


