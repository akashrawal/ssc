/* parselib.h
 * Parser support functions
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


//Semantic intermediate data structures

//Integer
#define ssc_sem_int(ptr) (*((int64_t *) (ptr)))

//String (literal and identifier both)
#define ssc_sem_str(ptr) ((char *) (ptr))

//Variable list
typedef struct
{
	SscVar *head, *tail;	
} SscSemVarList; 

//Function list
typedef struct 
{
	SscFn *head, *tail;
} SscSemFnList;

//Parser
typedef struct
{
	SscSymbolDB *db;
	SscAllocator *int_alloc, *final_alloc;
	SscBst *index;
	struct
	{
		SscSymbol *head, *tail;
	} symlist;
	
	char name[];
} SscParser;

//Functions for parser functioning

//Allocates intermediate semantic structures
void *ssc_parser_alloc_int(SscParser *parser, size_t size);
#define ssc_parser_new_int(parser, type) \
	((type *) (ssc_parser_alloc_int((parser), sizeof(type))))

//Allocates final semantic structures
void *ssc_parser_alloc_final(SscParser *parser, size_t size);
#define ssc_parser_new_final(parser, type) \
	((type *) (ssc_parser_alloc_final((parser), sizeof(type))))

//Add a new symbol, must be allocated final
void ssc_parser_add_symbol(SscParser *parser, SscSymbol symbol);

//Searches for a symbol
SscSymbol *ssc_parser_lookup(SscParser *parser, const char *name);

//Bison interface
int ssc_yyparse(/*Arguments?*/);

//Interface presented by parser
//All errors and warnings are printed to log stream.
//returns nonzero if successful, 0 if failed
int ssc_parser_parse_needed
	(SscSymbolDB *db, FILE *input, FILE *log, const char *filename);

