/* sequencer.h
 * Determines the sequence of symbol declarations in generated code
 * 
 * Copyright 2015-2019 Akash Rawal
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

typedef struct
{
	size_t len;
	SscSymbol **d;
} SscSymbolArray;

typedef struct _SscSequencer SscSequencer;

SscSequencer *ssc_sequencer_new(SscSymbolDB *db);

void ssc_sequencer_process_symbol(SscSequencer *seqr, SscSymbol *sym);

void ssc_sequencer_process_file(SscSequencer *seqr, const char *filename);

SscSymbolArray ssc_sequencer_destroy(SscSequencer *seqr);

