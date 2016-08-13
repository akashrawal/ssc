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

const char *ssc_symbol_names[] = 
{
	"struct",
	"interface",
	"string",
	"integer"
};

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

SscDLen ssc_type_fundamental_sizes[13] =
	{
		{0, 0},
		{1, 0},
		{2, 0},
		{4, 0},
		{8, 0},
		{1, 0},
		{2, 0},
		{4, 0},
		{8, 0},
		{4, 0},
		{8, 0},
		{0, 1},
		{0, 1}
	};

char *ssc_type_fundamental_names[13] =
	{
		"<SSC_TYPE_FUNDAMENTAL_NONE>",
		"uint8", 
		"uint16",
		"uint32", 
		"uint64",
		"int8",
		"int16",
		"int32",
		"int64",
		"flt32",
		"flt64",
		"string",
		"msg"
	};

//TODO: delete this
/*
static void ssc_symbol_db_calc_var_list_size
	(SscSymbolDB *db, SscVarList *listptr)
{
	int constsize = 1;
	SscDLen size = {0, 0};
	int i;
	
	for (i = 0; i < listptr->len; i++)
	{
		SscDLen unit_size;
		int unit_constsize;
		
		unit_size = ssc_type_calc_base_size(listptr->a[i]->type);
		unit_constsize = ssc_type_is_constsize(listptr->a[i]->type);
		
		size.n_bytes += unit_size.n_bytes; 
		size.n_submsgs += unit_size.n_submsgs;
		if (unit_constsize == 0)
			constsize = 0;
	}
	
	
	//Now return our findings
	listptr->base_size = size;
	listptr->constsize = constsize;
}
* */

static void ssc_calc_var_list_size
	(SscVarList *listptr)
{
	int constsize = 1;
	SscDLen size = {0, 0};
	int i;
	
	for (i = 0; i < listptr->len; i++)
	{
		SscDLen unit_size;
		int unit_constsize;
		
		unit_size = ssc_type_calc_base_size(listptr->a[i]->type);
		unit_constsize = ssc_type_is_constsize(listptr->a[i]->type);
		
		size.n_bytes += unit_size.n_bytes; 
		size.n_submsgs += unit_size.n_submsgs;
		if (unit_constsize == 0)
			constsize = 0;
	}
	
	
	//Now return our findings
	listptr->base_size = size;
	listptr->constsize = constsize;
}

SscDLen ssc_base_type_calc_base_size(SscType type)
{
	if (type.sym)
	{
		//varlist->constsize == -1 is used to signify 
		//that size is not calculated yet.
		if (type.sym->v.xstruct.fields.constsize == -1)
			ssc_calc_var_list_size(&(type.sym->v.xstruct.fields));
			
		return type.sym->v.xstruct.fields.base_size;
	}
	else
	{
		return ssc_type_fundamental_sizes[type.fid];
	}
}

int ssc_base_type_is_constsize(SscType type)
{
	if (type.sym)
	{
		if (type.sym->v.xstruct.fields.constsize == -1)
			ssc_calc_var_list_size(&(type.sym->v.xstruct.fields));
			
		if (type.sym->v.xstruct.fields.constsize)
			return 1;
		else 
			return 0;
	}
	
	return 1;
}

SscDLen ssc_type_calc_base_size(SscType type)
{
	SscDLen res;
	
	if (type.complexity == SSC_TYPE_OPTIONAL)
	{
		res.n_bytes = 1;
		res.n_submsgs = 0;
		return res;
	}
	else if (type.complexity == SSC_TYPE_SEQ)
	{
		res.n_bytes = 4;
		res.n_submsgs = 0;
		return res;
	}
	
	res = ssc_base_type_calc_base_size(type);
	
	if (type.complexity > 0)
	{
		res.n_bytes *= type.complexity;
		res.n_submsgs *= type.complexity;
	}
	
	return res;
}

int ssc_type_is_constsize(SscType type)
{
	if (type.complexity == SSC_TYPE_OPTIONAL)
		return 0;
	else if (type.complexity == SSC_TYPE_SEQ)
		return 0;
	
	return ssc_base_type_is_constsize(type);
}

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
	
	//initialize varlist->constsize = -1
	for (sym_iter = data.list; sym_iter; sym_iter = sym_iter->next)
	{
		if (sym_iter->type == SSC_SYMBOL_STRUCT)
		{
			sym_iter->v.xstruct.fields.constsize = -1;
		}
		else if (sym_iter->type == SSC_SYMBOL_INTERFACE)
		{
			int i;
			for (i = 0; i < sym_iter->v.xiface.fns_len; i++)
			{
				sym_iter->v.xiface.fns[i]->in.constsize = -1;
				sym_iter->v.xiface.fns[i]->out.constsize = -1;
			}
		}
	}
	
	//Add all file symbols
	for (sym_iter = data.list; sym_iter; sym_iter = sym_iter->next)
	{
		//Add to index and check for duplicates
		if (ssc_bst_insert(db->sym_index, sym_iter->name, sym_iter)
			!= MMC_SUCCESS)
				ssc_error("Duplicate symbol \"%s\"", sym_iter->name);
		
		//Calculate sizes as applicable
		if (sym_iter->type == SSC_SYMBOL_STRUCT)
		{
			ssc_calc_var_list_size
				(&(sym_iter->v.xstruct.fields));
		}
		else if (sym_iter->type == SSC_SYMBOL_INTERFACE)
		{
			int i;
			for (i = 0; i < sym_iter->v.xiface.fns_len; i++)
			{
				ssc_calc_var_list_size
					(&(sym_iter->v.xiface.fns[i]->in));
				ssc_calc_var_list_size
					(&(sym_iter->v.xiface.fns[i]->out));
			}
		}
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
