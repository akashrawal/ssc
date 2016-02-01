/* sequencer.h
 * Determines the sequence of symbol declarations in generated code
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

typedef struct
{
	SscSymbolDB *db;
	SscBst *index;
	SscSymbolArray *r;
	size_t len, alloc_len;
} SscSequencer;

#define ssc_array_sizeof(n) \
	(sizeof(SscSymbolArray) + ((n) * sizeof(void *)))

static void ssc_sequencer_init(SscSequencer *seqr, SscSymbolDB *db)
{
	seqr->db = db;
	ssc_symbol_db_ref(db);
	
	seqr->index = ssc_bst_new();
	seqr->len = 0;
	seqr->alloc_len = 16;
	seqr->r = mmc_alloc(ssc_array_sizeof(seqr->alloc_len));
}

static SscSymbolArray *ssc_sequencer_destroy(SscSequencer *seqr)
{
	ssc_symbol_db_unref(seqr->db);
	ssc_bst_unref(seqr->index);
	return seqr->r;
}

static void ssc_sequencer_append_to_array
	(SscSequencer *seqr, SscSymbol *sym)
{
	//Enlarge array if necessary
	if (seqr->len == seqr->alloc_len)
	{
		seqr->alloc_len *= 2;
		seqr->r = mmc_realloc
			(seqr->r, ssc_array_sizeof(seqr->alloc_len));
	}
	
	seqr->r->d[seqr->len] = sym;
	seqr->len++;
	ssc_bst_insert(seqr->index, sym->name, sym);
}

static void ssc_sequencer_process_symbol
	(SscSequencer *seqr, SscSymbol *sym);

static void ssc_sequencer_process_varlist
	(SscSequencer *seqr, SscVar **vars, size_t vars_len)
{
	int i; 
	
	for (i = 0; i < vars_len; i++)
	{
		if (vars[i]->type.sym)
			ssc_sequencer_process_symbol(seqr, vars[i]->type.sym);
	}
}

static void ssc_sequencer_process_symbol
	(SscSequencer *seqr, SscSymbol *sym)
{
	//Check the index
	void *index_entry;
	int i;
	
	index_entry = ssc_bst_lookup(seqr->index, sym->name);
	if (index_entry)
	{
		//Done
		return;
	}
	
	//Process this symbol
	if (sym->type == SSC_SYMBOL_INTEGER
		|| sym->type == SSC_SYMBOL_STRING)
	{
		//Nothing to do
	}
	else if (sym->type == SSC_SYMBOL_STRUCT)
	{
		//For each member
		ssc_sequencer_process_varlist
			(seqr, sym->v.xstruct.fields, sym->v.xstruct.fields_len);
	}
	else if (sym->type == SSC_SYMBOL_INTERFACE)
	{
		for (i = 0; i < sym->v.xiface.fns_len; i++)
		{
			SscFn *fn = sym->v.xiface.fns[i];
			
			ssc_sequencer_process_varlist(seqr, fn->in, fn->in_len);
			ssc_sequencer_process_varlist(seqr, fn->out, fn->out_len);
		}
	}
	
	//Indicate that we are done
	ssc_sequencer_append_to_array(seqr, sym);
}

SscSymbolArray *ssc_compute_sequence
	(SscSymbolDB *db, const char *filename)
{
	SscSequencer seqr[1];
	SscSymbol *iter;
	SscFileData file_data;
	
	ssc_sequencer_init(seqr, db);
	
	file_data = ssc_symbol_db_get_file_data(db, filename);
	
	for (iter = file_data.list; iter; iter = iter->next)
	{
		ssc_sequencer_process_symbol(seqr, iter);
	}
	
	return ssc_sequencer_destroy(seqr);
}
