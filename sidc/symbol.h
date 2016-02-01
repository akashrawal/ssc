/* symbol.h
 * Symbol database
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

//Allocator for semantic objects
typedef struct _SscAllocator SscAllocator;

mmc_rc_declare(SscAllocator, ssc_allocator)

SscAllocator *ssc_allocator_new();
void *ssc_allocator_alloc(SscAllocator *allocator, size_t size);

/////////////////////
//Symbol table

//Forward declarations
typedef struct _SscSymbol SscSymbol;

//Base type
typedef enum
{
	SSC_TYPE_FUNDAMENTAL_NONE = 0, //< Not fundamental type
	SSC_TYPE_FUNDAMENTAL_INT8,
	SSC_TYPE_FUNDAMENTAL_INT16,
	SSC_TYPE_FUNDAMENTAL_INT32,
	SSC_TYPE_FUNDAMENTAL_INT64,
	SSC_TYPE_FUNDAMENTAL_UINT8,
	SSC_TYPE_FUNDAMENTAL_UINT16,
	SSC_TYPE_FUNDAMENTAL_UINT32,
	SSC_TYPE_FUNDAMENTAL_UINT64,
	SSC_TYPE_FUNDAMENTAL_FLT32,
	SSC_TYPE_FUNDAMENTAL_FLT64,
	SSC_TYPE_FUNDAMENTAL_STRING,
	SSC_TYPE_FUNDAMENTAL_MSG
} SscTypeFundamentalID;

typedef enum
{
	SSC_TYPE_OPTIONAL = -2,
	SSC_TYPE_SEQ = -1,
	SSC_TYPE_NONE = 0
	//and > 0 for array
} SscTypeComplexity;


typedef struct 
{
	SscSymbol *sym;
	SscTypeFundamentalID fid; //If sym is not null this is invalid
	int complexity;
} SscType;

#define ssc_type_is_fundamental(typeptr) ((typeptr)->sym ? 1 : 0)

//Variable
typedef struct
{
	SscType type;
	char name[];
} SscVar;

//Function
typedef struct
{	
	SscVar **in, **out;
	size_t in_len, out_len;
	char name[];
} SscFn;

//Structure
typedef struct 
{
	SscVar **fields;
	size_t fields_len;
} SscStruct;

//interface
typedef struct 
{
	SscSymbol *parent;
	SscFn **fns;
	size_t fns_len;
} SscInterface;

//Integer constant
typedef int64_t ssc_integer;

///////////////
//Symbol database interface
typedef enum
{
	SSC_SYMBOL_STRUCT = 0,
	SSC_SYMBOL_INTERFACE = 1,
	SSC_SYMBOL_STRING = 2,
	SSC_SYMBOL_INTEGER = 3
} SscSymbolType;

extern const char *ssc_symbol_names[];

struct _SscSymbol
{
	SscSymbol *next;
	
	SscSymbolType type;
	union 
	{
		SscStruct xstruct;
		SscInterface xiface;
		char *xstr;
		ssc_integer xint;
	} v;
	
	char name[];
};

//File states
typedef enum
{
	SSC_FILE_UNREC,
	SSC_FILE_PARSING,
	SSC_FILE_PARSED,
	SSC_FILE_BAD
} SscFileState;

typedef struct 
{
	//Symbols should be allocated by the allocator
	SscSymbol *list;
	SscAllocator *allocator;
} SscFileData;

typedef struct _SscSymbolDB SscSymbolDB;

mmc_rc_declare(SscSymbolDB, ssc_symbol_db)

SscSymbolDB *ssc_symbol_db_new();
	
SscFileState ssc_symbol_db_get_file_state
	(SscSymbolDB *db, const char *filename);
	
SscFileData ssc_symbol_db_get_file_data
	(SscSymbolDB *db, const char *filename);

void ssc_symbol_db_register_file_parsing
	(SscSymbolDB *db, const char *filename);

void ssc_symbol_db_register_file_parsed
	(SscSymbolDB *db, const char *filename, SscFileData data);
	
void ssc_symbol_db_register_file_bad
	(SscSymbolDB *db, const char *filename);

SscSymbol *ssc_symbol_db_lookup(SscSymbolDB *db, const char *name);


