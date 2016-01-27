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

mmc_rc_declare(SscAllocator, ssc_allocator);

SscAllocator *ssc_allocator_new();
void *ssc_allocator_alloc();

/////////////////////
//Symbol table

//Forward declarations
typedef struct _SscSymbol SscSymbol;

//Base type
typedef enum
{
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
	SSC_TYPE_NORMAL = 0
	//and > 0 for array
} SscTypeComplexity;


typedef struct 
{
	SscSymbol *sym;
	SscTypeFundamentalID fid; //If sym is not null this is invalid
	int complexity
} SscType;

#define ssc_type_is_fundamental(typeptr) ((typeptr)->sym ? 1 : 0)

//Variable
typedef struct _SscVar SscVar;
struct _SscVar
{
	SscType type;
	SscVar *next;
	char name[];
};

//Structure
typedef struct _SscStruct SscStruct;
struct _SscStruct
{
	SscSemStr *name;
	SscVar *fields;
	char name[];
};

//Function
typedef struct
{	
	SscVar *in, *out;
	char name[];
} SscFn;

//interface
typedef struct _SscIface SscIface;
struct _SscIface
{
	SscIface *parent;
	SscFn *fns;
	char name[];
};

//Symbol database interface
struct _SscSymbol
{
	SscSymbol *next;
	
	char *name;
	//Only one of the following is not null
	SscStruct *sym_struct;
	SscIface *sym_iface;
};

//File states
typedef enum
{
	SSC_FILE_NOT_PARSED,
	SSC_FILE_PARSING,
	SSC_FILE_PARSED,
	SSC_FILE_BAD
} SscFileState;

typedef struct 
{
	char *name;
	SscSymbol *list;
	SscAllocator *allocator;
} SscFileData;

typedef struct _SscSymbolDB SscSymbolDB;

mmc_rc_declare(SscSymbolDB, ssc_symbol_db)

SscSymbolDB *ssc_symbol_db_new();

void ssc_symbol_db_register_file_parsing
	(SscSymbolDB *db, const char *filename);

void ssc_symbol_db_register_file_parsed
	(SscSymbolDB *db, const char *filename, SscFileData data);
	
void ssc_symbol_db_register_file_bad
	(SscSymbolDB *db, const char *filename);
	
SscFileState ssc_symbol_db_get_file_state
	(SscSymbolDB *db, const char *filename);
	
SscFileData ssc_symbol_db_get_file_data
	(SscSymbolDB *db, const char *filename);

SscSymbol *ssc_symbol_db_lookup(SscSymbolDB *db, const char *name);

SscSymbol *ssc_symbol_db_list_by_file
	(SscSymbolDB *db, const char *filename);
