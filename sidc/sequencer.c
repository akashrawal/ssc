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

mmc_declare_array(SscSymbol *, SscSymbolVector, ssc_symbol_vector);

struct _SscSequencer
{
	SscSymbolDB *db;
	SscBst *index;
	SscSymbolVector vector;
};

SscSequencer *ssc_sequencer_new(SscSymbolDB *db)
{
	SscSequencer *seqr;
	
	seqr = mmc_new(SscSequencer);
	
	seqr->db = db;
	ssc_symbol_db_ref(db);
	
	seqr->index = ssc_bst_new();
	ssc_symbol_vector_init(&(seqr->vector));
	
	return seqr;
}

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

void ssc_sequencer_process_symbol(SscSequencer *seqr, SscSymbol *sym)
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
			(seqr, sym->v.xstruct.fields.a, sym->v.xstruct.fields.len);
	}
	else if (sym->type == SSC_SYMBOL_INTERFACE)
	{
		for (i = 0; i < sym->v.xiface.fns_len; i++)
		{
			SscFn *fn = sym->v.xiface.fns[i];
			
			ssc_sequencer_process_varlist(seqr, fn->in.a, fn->in.len);
			ssc_sequencer_process_varlist(seqr, fn->out.a, fn->out.len);
		}
	}
	
	//We are done. Update datastructures.
	ssc_bst_insert(seqr->index, sym->name, sym);
	ssc_symbol_vector_append(&(seqr->vector), sym);
}

void ssc_sequencer_process_file
	(SscSequencer *seqr, const char *filename)
{
	SscSymbol *iter;
	SscFileData file_data;
	
	file_data = ssc_symbol_db_get_file_data(seqr->db, filename);
	
	for (iter = file_data.list; iter; iter = iter->next)
	{
		ssc_sequencer_process_symbol(seqr, iter);
	}
}

SscSymbolArray ssc_sequencer_destroy(SscSequencer *seqr)
{
	SscSymbolArray res;
	
	res.len = ssc_symbol_vector_size(&(seqr->vector));
	res.d = seqr->vector.data;
	
	ssc_symbol_db_unref(seqr->db);
	ssc_bst_unref(seqr->index);
	free(seqr);
	
	return res;
}
