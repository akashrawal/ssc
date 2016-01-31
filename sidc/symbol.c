/* symbol.c
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

#include "incl.h"

//Allocator for semantic objects
//TODO: Optimize this to reduce calls to malloc
typedef struct _SscAllocHandle SscAllocHandle;
struct _SscAllocHandle
{
	SscAllocHandle *next;
};

struct _SscAllocator
{
	MmcRC parent;
	
	SscAllocHandle *allocated;
};

mmc_rc_define(SscAllocator, ssc_allocator)

SscAllocator *ssc_allocator_new()
{
	SscAllocator *allocator = mmc_new(SscAllocator);
	
	mmc_rc_init(allocator);
	allocator->allocated = NULL;
	
	return allocator;
}

void *ssc_allocator_alloc(SscAllocator *allocator, size_t size)
{
	void *mem;
	SscAllocHandle *handle;
	
	handle = (SscAllocHandle *) mmc_alloc2
		(sizeof(SscAllocHandle), size, &mem);
	handle->next = allocator->allocated;
	allocator->allocated = handle;
	
	return mem;
}

static void ssc_allocator_destroy(SscAllocator *allocator)
{
	SscAllocHandle *iter, *next;
	
	for (iter = allocator->allocated; iter; iter = next)
	{
		next = iter->next;
		
		free(iter);
	}
	
	free(allocator);
}

/////////////////////
//Symbol table

typedef struct _SscFileList SscFileList;
struct _SscFileList 
{
	SscFileData data;
	
	SscFileList *next;
	SscFileState state;
	char name[];
};

struct _SscSymbolDB 
{
	MmcRC parent;
	
	SscBst *sym_index;
	SscBst *file_index;
	
	SscFileList *file_list;
};

mmc_rc_define(SscSymbolDB, ssc_symbol_db)

SscSymbolDB *ssc_symbol_db_new()
{
	SscSymbolDB *db = mmc_new(SscSymbolDB);
	
	mmc_rc_init(db);
	
	db->sym_index = ssc_bst_new();
	db->file_index = ssc_bst_new();
	
	db->file_list = NULL;
	
	return db;
}

SscFileState ssc_symbol_db_get_file_state
	(SscSymbolDB *db, const char *filename)
{
	SscFileList *file;
	
	file = (SscFileList *) ssc_bst_lookup(db->file_index, filename);
	
	if (! file)
		return SSC_FILE_UNREC;
	
	return file->state;
}
	
SscFileData ssc_symbol_db_get_file_data
	(SscSymbolDB *db, const char *filename)
{
	SscFileList *file;
	
	file = (SscFileList *) ssc_bst_lookup(db->file_index, filename);
	
	if (file)
		if (file->state != SSC_FILE_PARSED)
			file = NULL;
	
	if (! file)
		ssc_error("Attempted to get data for unparsed file \"%s\"",
			filename);
	
	return file->data;
}

void ssc_symbol_db_register_file_parsing
	(SscSymbolDB *db, const char *filename)
{
	SscFileList *file;
	
	file = (SscFileList *) mmc_alloc
		(sizeof(SscFileList) + strlen(filename) + 1);
		
	file->data.allocator = NULL;
	file->data.list = NULL;
	
	file->next = db->file_list;
	db->file_list = file;
	file->state = SSC_FILE_PARSING;
	strcpy(file->name, filename);
	
	if (ssc_bst_insert(db->file_index, filename, file) 
		!= MMC_SUCCESS)
		ssc_error("Attempted to register already parsing file \"%s\"",
			filename);
}

void ssc_symbol_db_register_file_parsed
	(SscSymbolDB *db, const char *filename, SscFileData data)
{
	SscFileList *file;
	SscSymbol *sym_iter;
	
	file = (SscFileList *) ssc_bst_lookup(db->file_index, filename);
	
	if (file)
		if (file->state != SSC_FILE_PARSING)
			file = NULL;
	
	if (! file)
		ssc_error("Attempted register non-parsing file \"%s\""
			"as parsed",
			filename);
	
	file->state = SSC_FILE_PARSED;
	file->data = data;
	ssc_allocator_ref(data.allocator);
	
	//Add all file symbols
	for (sym_iter = data.list; sym_iter; sym_iter = sym_iter->next)
	{
		if (ssc_bst_insert(db->sym_index, sym_iter->name, sym_iter)
			!= MMC_SUCCESS)
				ssc_error("Duplicate symbol \"%s\"", sym_iter->name);
	}
}
	
void ssc_symbol_db_register_file_bad
	(SscSymbolDB *db, const char *filename)
{	
	SscFileList *file;
	
	file = (SscFileList *) ssc_bst_lookup(db->file_index, filename);
	
	if (file)
		if (file->state != SSC_FILE_PARSING)
			file = NULL;
	
	if (! file)
		ssc_error("Attempted register non-parsing file \"%s\""
			"as bad",
			filename);
	
	file->state = SSC_FILE_BAD;
}
	


SscSymbol *ssc_symbol_db_lookup(SscSymbolDB *db, const char *name)
{
	SscSymbol *sym;
	
	sym = ssc_bst_lookup(db->sym_index, name);
	
	return sym;
}
	
static void ssc_symbol_db_destroy(SscSymbolDB *db)
{
	SscFileList *file_iter, *file_next;
	
	ssc_bst_unref(db->file_index);
	ssc_bst_unref(db->sym_index);
	
	for (file_iter = db->file_list; file_iter; file_iter = file_next)
	{
		file_next = file_iter->next;
		
		if (file_iter->state == SSC_FILE_PARSED)
			ssc_allocator_unref(file_iter->data.allocator);
		
		free(file_iter);
	}
	
	free(db);
}
