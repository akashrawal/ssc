/* parselib.c
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
 
#include "incl.h"

#include <stdarg.h>
#include <stddef.h>
#include <errno.h>
//LATER: refactor all includes

typedef struct 
{
	char *mem;
	size_t len, alloc_len;
} StrBuf;

static void strbuf_init(StrBuf *s)
{
	s->len = 0;
	s->alloc_len = 16;
	s->mem = (char *) mmc_alloc(s->alloc_len);
}

static void strbuf_add(StrBuf *s, const char *str)
{
	size_t str_len = strlen(str);
	
	//Enlarge buffer as necessary
	if (s->len + str_len >= s->alloc_len)
	{
		do { s->alloc_len *= 2; }
		while (s->len + str_len >= s->alloc_len);
			
		s->mem = (char *) mmc_realloc(s->mem, s->alloc_len);
	}
	
	//copy
	memcpy(s->mem + s->len, str, str_len);
	
	//update length
	s->len += str_len;
}

static void strbuf_destroy(StrBuf *s)
{
	free(s->mem);
}

//Parser
struct _SscParser
{
	//Database to commit to 
	SscSymbolDB *db;
	
	//Allocators
	SscAllocator *int_alloc, *final_alloc;
	
	//Index of all symbols parser can refer
	//(Reference also adds to this index)
	SscBst *index;
	
	//List of parsed symbols
	struct
	{
		SscSymbol *head, *tail;
	} symlist;
	
	//Logging
	FILE *log;
	int count[SSC_LOG_N];
	
	//String buffer
	StrBuf strbuf;
	
	//Filename
	char filename[];
};

const static char *log_msg[SSC_LOG_N] 
		= {"error", "warning", "message", "debug"};

void ssc_parser_log
	(SscParser *parser, SscParserLogType type, const char *fmt, ...)
{
	va_list arglist;
	
	fprintf(parser->log, "%s: %s: ", parser->filename, log_msg[type]);
	
	va_start(arglist, fmt);
	vfprintf(parser->log, fmt, arglist);
	va_end(arglist);
	
	fprintf(parser->log, "\n");
	
	parser->count[type]++;
}

static void ssc_parser_summarise_log(SscParser *parser)
{
	int total_log = 0, i;
	
	for (i = 0; i < SSC_LOG_N; i++)
		total_log += parser->count[i];
	
	if (total_log)
	{
		fprintf(parser->log, "%s: ", parser->filename);
		
		for (i = 0; i < SSC_LOG_N; i++)
			fprintf(parser->log, "%s%d %s(s)", 
				i ? ", " : "", i, log_msg[i]);
	}
	
	fprintf(parser->log, "\n");
}

//Allocates intermediate semantic structures
void *ssc_parser_alloc_int(SscParser *parser, size_t size)
{
	return ssc_allocator_alloc(parser->int_alloc, size);
}

//Allocates final semantic structures
void *ssc_parser_alloc_final(SscParser *parser, size_t size)
{
	return ssc_allocator_alloc(parser->final_alloc, size);
}

//Allocates a new symbol. Returns NULL in case of name clashes.
SscSymbol *ssc_parser_alloc_symbol
	(SscParser *parser, const char *name, SscSymbolType type)
{
	SscSymbol *sym;
	
	//Check for name clashes. In that case we should return NULL.
	if (ssc_bst_lookup(parser->index, name))
	{
		ssc_parser_error(parser, "Name clash with %s", name);
		return NULL;
	}
	if (ssc_symbol_db_lookup(parser->db, name))
	{
		ssc_parser_error(parser, 
			"Name clash with %s outside current scope", name);
		return NULL;
	}
	
	//Create symbol
	sym = ssc_parser_alloc_final
		(parser, sizeof(SscSymbol) + strlen(name) + 1);
	strcpy(sym->name, name);
	sym->type = type;
	
	//Add to list
	sym->next = NULL;
	if (parser->symlist.tail)
		parser->symlist.tail->next = sym;
	else
		parser->symlist.head = sym;
	parser->symlist.tail = sym;
	
	//Add to list
	ssc_bst_insert(parser->index, name, sym);
	
	return sym;
}

//Searches for a symbol within references, returns NULL if fails
SscSymbol *ssc_parser_lookup(SscParser *parser, const char *name)
{
	return ssc_bst_lookup(parser->index, name);
}

//Searches for a symbol, checks its type, and prints a message 
//if type does not match
SscSymbol *ssc_parser_lookup_expecting
	(SscParser *parser, const char *name, SscSymbolType type)
{
	SscSymbol *sym = ssc_parser_lookup(parser, name);
	
	if (! sym)
	{
		ssc_parser_error(parser, "%s not defined in current scope",
			name);
		return NULL;
	}
	
	if (sym->type != type)
	{
		ssc_parser_error(parser, "%s was expected to be %s",
			ssc_symbol_names[type]);
		return NULL;
	}
	
	return sym;
}

///////////////////////////////
//Integer related functions
static MmcStatus ssc_read_int_with_base
	(SscParser *parser, int base, const char *matched_text, ssc_integer *res)
{
	ssc_integer val = 0;
	int i;
	
	//Parse in loop
	for (i = 0; matched_text[i]; i++)
	{
		int digit;
		
		if (matched_text[i] >= 'a' && matched_text[i] <= 'f')
			digit = matched_text[i] - 'a' + 10;
		else if (matched_text[i] >= 'A' && matched_text[i] <= 'F')
			digit = matched_text[i] - 'A' + 10;
		else if (matched_text[i] >= '0' && matched_text[i] <= '9')
			digit = matched_text[i] - '0';
		else
			digit = 16;
		
		if (digit >= base)
		{
			ssc_parser_error(parser, "Unrecognized character %c", 
				matched_text[i]);
			return MMC_FAILURE;
		}
		
		val *= base;
		val += digit;
	}
	
	*res = val;
	return MMC_SUCCESS;
}

MmcStatus ssc_parser_read_int
	(SscParser *parser, const char *matched_text, ssc_integer *res)
{
	size_t len = strlen(matched_text);
	int base = 10;
	int start = 0;
	
	//Find base
	if (len >= 2)
	{
		if (matched_text[0] == '0' && (matched_text[1] == 'x' || matched_text[1] == 'X'))
		{
			base = 16;
			start = 2;
		}
	}
	else if (len >= 1)
	{
		if (matched_text[0] == '0')
		{
			base = 8;
			start = 1;
		}
	}
	
	return ssc_read_int_with_base(parser, base, matched_text + start, res);
}

//////////////////////////////////////
//String Management

#define s (parser->strbuf)

void ssc_parser_strbuf_add(SscParser *parser, const char *str)
{
	strbuf_add(&s, str);
}

MmcStatus ssc_parser_strbuf_add_by_num
	(SscParser *parser, int base, const char *matched_text)
{
	char str[2];
	ssc_integer val;
	
	if (ssc_read_int_with_base(parser, base, matched_text, &val)
			!= MMC_SUCCESS)
		return MMC_FAILURE;
	
	if (val == 0)
	{
		ssc_parser_error(parser, "Strings cannot contain null bytes");
		return MMC_FAILURE;
	}
		
	str[0] = val;
	str[1] = 0;
	
	strbuf_add(&s, str);
	
	return MMC_SUCCESS;
}

char *ssc_parser_strbuf_end(SscParser *parser)
{
	char *res;
	
	res = ssc_parser_alloc_int(parser, s.len + 1);
	memcpy(res, s.mem, s.len);
	res[s.len] = 0;
	
	strbuf_destroy(&s);
	strbuf_init(&s);
	
	return res;
}

#undef s

char *ssc_parser_strdup(SscParser *parser, const char *str)
{
	char *res;
	size_t len;
	
	len = strlen(str);
	res = ssc_parser_alloc_int(parser, len + 1);
	memcpy(res, str, len + 1);
	
	return res;
}

char *ssc_parser_strcat
	(SscParser *parser, const char *str1, const char *str2)
{
	char *res;
	
	res = ssc_parser_alloc_int(parser, strlen(str1) + strlen(str2) + 1);
	strcpy(res, str1);
	strcat(res, str2);
	
	return res;
}


//
//Integer constant
MmcStatus ssc_parser_add_integer_constant
	(SscParser *parser, const char *name, ssc_integer val)
{
	SscSymbol *sym;
	
	sym = ssc_parser_alloc_symbol(parser, name, SSC_SYMBOL_INTEGER);
	if (! sym)
		return MMC_FAILURE;
	
	sym->v.xint = val;
	
	return MMC_SUCCESS;
}

//
//String constant
MmcStatus ssc_parser_add_string_constant
	(SscParser *parser, const char *name, const char *val)
{
	SscSymbol *sym;
	
	sym = ssc_parser_alloc_symbol(parser, name, SSC_SYMBOL_STRING);
	if (! sym)
		return MMC_FAILURE;
	
	sym->v.xstr = ssc_parser_alloc_final(parser, strlen(val) + 1);
	strcpy(sym->v.xstr, val);
	
	return MMC_SUCCESS;
}
	

//List
SscRList *ssc_parser_rlist_prepend
	(SscParser *parser, SscRList *prev, void *data)
{
	SscRList *node;
	
	node = ssc_parser_alloc_int(parser, sizeof(SscRList));
	node->prev = prev;
	node->d.data = data;
	
	return node;
}


static MmcStatus ssc_parser_rlist_to_array_checked (SscParser *parser, 
	SscRList *rlist, size_t name_offset, 
	void ***array_res, size_t *len_res,
	const char *clash_msg, const char *pname)
{
	size_t len, i;
	SscRList *iter;
	SscBst *index;
	void **array;
	
	//Count and check for name clashes
	index = ssc_bst_new();
	len = 0;
	for (iter = rlist; iter; iter = iter->prev)
	{
		char *fname = (char *) MMC_PTR_ADD(iter->d.data, name_offset);
		if (ssc_bst_insert(index, fname, fname) != MMC_SUCCESS)
		{
			ssc_parser_error(parser, clash_msg, fname, pname);
			break;
		}
		
		len++;
	}
	ssc_bst_unref(index);
	
	if (iter)
		return MMC_FAILURE;
	
	//Create array
	array = ssc_parser_alloc_final(parser, sizeof(void *) * len);
	
	//Copy data into array
	for (iter = rlist, i = len - 1; iter; iter = iter->prev, i--)
		array[i] = iter->d.data;
		
	*array_res = array;
	*len_res = len;
	return MMC_SUCCESS;
}

//variable
SscVar *ssc_parser_new_var
	(SscParser *parser, SscType type, const char *name)
{
	SscVar *var;
	
	var = (SscVar *) mmc_alloc(sizeof(SscVar) + strlen(name) + 1);
	var->type = type;
	strcpy(var->name, name);
	
	return var;
}


//Struct
MmcStatus ssc_parser_add_struct
	(SscParser *parser, const char *name, SscRList *fields)
{
	SscSymbol *sym;
	
	sym = ssc_parser_alloc_symbol(parser, name, SSC_SYMBOL_STRUCT);
	if (! sym)
		return MMC_FAILURE;
	
	if (ssc_parser_rlist_to_array_checked
			(parser, fields, offsetof(SscVar, name), 
			(void ***) &(sym->v.xstruct.fields), &(sym->v.xstruct.fields_len),
			"Name clash for field %s in struct %s", name)
		!= MMC_SUCCESS)
	{
		return MMC_FAILURE;
	}
	
	return MMC_SUCCESS;
}

//Function
SscFn *ssc_parser_new_fn(SscParser *parser, 
	const char *name, SscRList *args, SscRList *out_args)
{
	SscFn *fn;
	
	fn = (SscFn *) ssc_parser_alloc_final(parser, sizeof(SscFn));
	
	if (ssc_parser_rlist_to_array_checked
			(parser, args, offsetof(SscVar, name), 
			(void ***) &(fn->in), &(fn->in_len) ,
			"Name clash for argument %s in function %s", name)
		!= MMC_SUCCESS)
	{
		return NULL;
	}
	
	if (ssc_parser_rlist_to_array_checked
			(parser, out_args, offsetof(SscVar, name), 
			(void ***) &(fn->out), &(fn->out_len) ,
			"Name clash for out-argument %s in function %s", name)
		!= MMC_SUCCESS)
	{
		return NULL;
	}
	
	return fn;
}

//Interface
MmcStatus ssc_parser_add_interface(SscParser *parser, 
	const char *name, const char *parent, SscRList *fns)
{
	SscSymbol *sym, *psym;
	
	if (parent)
	{
		psym = ssc_parser_lookup_expecting
			(parser, parent, SSC_SYMBOL_INTERFACE);
		if (psym == NULL)
			return MMC_FAILURE;
	}
	else
		psym = NULL;
	
	sym = ssc_parser_alloc_symbol(parser, name, SSC_SYMBOL_INTERFACE);
	if (! sym)
		return MMC_FAILURE;
	sym->v.xiface.parent = psym;
	
	if (ssc_parser_rlist_to_array_checked
			(parser, fns, offsetof(SscFn, name), 
			(void ***) &(sym->v.xiface.fns), &(sym->v.xiface.fns_len),
			"Name clash for function %s in interface %s", name)
		!= MMC_SUCCESS)
	{
		return MMC_FAILURE;
	}
	
	return MMC_SUCCESS;
}
	
//Reference
MmcStatus ssc_parser_exec_ref(SscParser *parser, const char *name)
{
	SscSymbol *iter;
	SscFileData file_data;
	
	//Ensure that the file is there
	if (ssc_parser_parse_needed(parser->db, parser->log, name)
		!= MMC_SUCCESS)
	{
		ssc_parser_error(parser, "Could not refer to file %s", name);
		return MMC_FAILURE;
	}
	
	//Import the symbols in our context
	file_data = ssc_symbol_db_get_file_data(parser->db, name);
	for (iter = file_data.list; iter; iter = iter->next)
	{
		ssc_bst_insert(parser->index, iter->name, iter);
	}
	
	return MMC_SUCCESS;
}

static SscParser *ssc_parser_new
	(SscSymbolDB *db, FILE *log, const char *filename)
{
	SscParser *parser;
	int i;
	
	parser = (SscParser *) mmc_alloc
		(sizeof(SscParser) + strlen(filename) + 1);
		
	parser->db = db;
	ssc_symbol_db_ref(db);
	
	parser->int_alloc = ssc_allocator_new();
	parser->final_alloc = ssc_allocator_new();
	
	parser->index = ssc_bst_new();
	
	parser->symlist.head = parser->symlist.tail = NULL;
	
	parser->log = log;
	for (i = 0; i < SSC_LOG_N; i++)
		parser->count[i] = 0;
	
	strbuf_init(&(parser->strbuf));
	
	strcpy(parser->filename, filename);
	
	return parser;
}

static void ssc_parser_destroy(SscParser *parser)
{
	strbuf_destroy(&(parser->strbuf));
	
	ssc_allocator_unref(parser->int_alloc);
	ssc_allocator_unref(parser->final_alloc);
	
	ssc_symbol_db_unref(parser->db);
	
	free(parser);
}

//Interface presented by parser
//All errors and warnings are printed to log stream.
MmcStatus ssc_parser_parse_needed
	(SscSymbolDB *db, FILE *log, const char *filename)
{
	SscParser *parser = ssc_parser_new(db, log, filename);
	SscFileState file_state;
	
	//Get file state
	file_state = ssc_symbol_db_get_file_state(db, filename);
	
	//Take action according to file status
	if (file_state == SSC_FILE_PARSED)
	{
		//We are done
		goto success;
	}
	else if (file_state == SSC_FILE_PARSING)
	{
		//Cyclic references
		ssc_parser_error(parser, "Cyclic reference detected");
		goto fail;
	}
	else if (file_state == SSC_FILE_BAD)
	{
		//Parsing failed already, wont parse again
		ssc_parser_error
			(parser, "Parsing has already failed before");
		goto fail;
	}
	else
	{
		//Parse the file
		FILE *input;
		yyscan_t yyscanner;
		int res;
		SscFileData file_data;
		
		//register parsing
		ssc_symbol_db_register_file_parsing(db, filename);
		
		//Open file
		//TODO: Use filename like this?
		input = fopen(filename, "r");
		if (input)
		{
			//Initialize scanner 
			ssc_yylex_init_extra(parser, &yyscanner);
			ssc_yyset_in(yyscanner, input);
			
			//Run parser
			res = ssc_yyparse(yyscanner, parser);
			if (res == 2)
			{
				ssc_error("Bison parser ran out of memory. Aborting.");
			}
			
			ssc_parser_summarise_log(parser);
			
			//free scanner
			ssc_yylex_destroy(yyscanner);
			fclose(input);
			
			//Commit symbols, register parsed
			file_data.list = parser->symlist.head;
			file_data.allocator = parser->final_alloc;
			ssc_symbol_db_register_file_parsed(db, filename, file_data);
		}
		else
		{
			ssc_parser_error(parser, "Cannot open file: %s", 
				strerror(errno));
		}
		
		//Error handling
		if (res == 1)
			goto fail;
	}
	
success:
	ssc_parser_destroy(parser);
	return MMC_SUCCESS;

fail:
	ssc_parser_destroy(parser);
	return MMC_FAILURE;
}

