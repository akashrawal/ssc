/* parselib.h
 * Parser support functions
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

//Parser
typedef struct _SscParser SscParser;

//Semantic type

typedef struct _SscRList SscRList;
struct _SscRList
{
	SscRList *prev; //According to production rules
	union 
	{
		void *data;
		SscVar *xvar;
		SscFn *xfn;
	} d;
};

typedef union 
{
	ssc_integer xint;
	char *xstr;
	SscType xtype;
	SscRList *xrl;
	SscFn *xfn;
} SscYYSType;

//Flex scanner interface
typedef void * yyscan_t;
extern int ssc_yylex(SscYYSType *yylval_param ,yyscan_t yyscanner);
//int ssc_yylex_init(yyscan_t *yyscanner);
int ssc_yylex_init_extra(SscParser *user_defined, yyscan_t *yyscanner);
int ssc_yylex_destroy(yyscan_t yyscanner);
void ssc_yyset_in(FILE *input, yyscan_t yyscanner);

//Bison interface
#include "parser.tab.h"

//Logging
typedef enum 
{
	//Syced with table in source file
	SSC_LOG_ERROR = 0,
	SSC_LOG_WARN = 1,
	SSC_LOG_MSG = 2,
	SSC_LOG_DEBUG = 3,
	SSC_LOG_N = 4
} SscParserLogType;

void ssc_parser_log
	(SscParser *parser, SscParserLogType type, const char *fmt, ...);

#define ssc_parser_error(parser, ...) \
	ssc_parser_log((parser), SSC_LOG_ERROR, __VA_ARGS__)

#define ssc_parser_warn(parser, ...) \
	ssc_parser_log((parser), SSC_LOG_WARN, __VA_ARGS__)

#define ssc_parser_msg(parser, ...) \
	ssc_parser_log((parser), SSC_LOG_MSG, __VA_ARGS__)

#define ssc_parser_debug(parser, ...) \
	ssc_parser_log((parser), SSC_LOG_DEBUG, __VA_ARGS__)


//Allocates intermediate semantic structures
void *ssc_parser_alloc_int(SscParser *parser, size_t size);

//Allocates final semantic structures
void *ssc_parser_alloc_final(SscParser *parser, size_t size);

//Allocates a new symbol. Returns NULL in case of name clashes.
SscSymbol *ssc_parser_alloc_symbol
	(SscParser *parser, const char *name, SscSymbolType type);

//Searches for a symbol within references, returns NULL if fails
SscSymbol *ssc_parser_lookup(SscParser *parser, const char *name);

//Searches for a symbol, checks its type, and prints reasonable 
//message if anything goes wrong (apart from returning NULL)
SscSymbol *ssc_parser_lookup_expecting
	(SscParser *parser, const char *name, SscSymbolType type);

///////////////////////////////
//Integer related functions
MmcStatus ssc_parser_read_int
	(SscParser *parser, const char *instr, ssc_integer *res);

//////////////////////////////////////
//String management
char *ssc_parser_strdup(SscParser *parser, const char *str);
char *ssc_parser_strcat
	(SscParser *parser, const char *str1, const char *str2);
MmcStatus ssc_parser_read_string
	(SscParser *parser, const char *instr, char **res);

//
//Integer constant
MmcStatus ssc_parser_add_integer_constant
	(SscParser *parser, const char *name, ssc_integer val);

//
//String constant
MmcStatus ssc_parser_add_string_constant
	(SscParser *parser, const char *name, const char *val);

//List
SscRList *ssc_parser_rlist_prepend
	(SscParser *parser, SscRList *prev, void *data);

//variable
SscVar *ssc_parser_new_var
	(SscParser *parser, SscType type, const char *name);

//Struct
MmcStatus ssc_parser_add_struct
	(SscParser *parser, const char *name, SscRList *fields);

//Function
SscFn *ssc_parser_new_fn(SscParser *parser, 
	const char *name, SscRList *args, SscRList *out_args);

//Interface
MmcStatus ssc_parser_add_interface(SscParser *parser, 
	const char *name, const char *parent, SscRList *fns);
	
//Reference
MmcStatus ssc_parser_exec_ref(SscParser *parser, const char *name);

//Interface presented by parser
//All errors and warnings are printed to log stream.
MmcStatus ssc_parser_parse_needed
	(SscSymbolDB *db, FILE *log, const char *filename);

