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
 
/*
typedef enum
{
	//Terminal symbols with lexemes
	SSC_TOKEN_ID,
	SSC_TOKEN_NUM,
	
	//Punctuation type terminal symbols
	SSC_TOKEN_STR,
	SSC_TOKEN_LPAREN,
	SSC_TOKEN_RPAREN,
	SSC_TOKEN_LCURLY,
	SSC_TOKEN_RCURLY,
	SSC_TOKEN_COLON,
	SSC_TOKEN_SEMICOLON,
	SSC_TOKEN_ASSIGN,
	
	//Terminal symbols representing keywords 
	SSC_TOKEN_KW_INT8,
	SSC_TOKEN_KW_INT16,
	SSC_TOKEN_KW_INT32,
	SSC_TOKEN_KW_INT64,
	SSC_TOKEN_KW_UINT8,
	SSC_TOKEN_KW_UINT16,
	SSC_TOKEN_KW_UINT32,
	SSC_TOKEN_KW_UINT64,
	SSC_TOKEN_KW_FLT32,
	SSC_TOKEN_KW_FLT64,
	SSC_TOKEN_KW_STR,
	SSC_TOKEN_KW_MSG,
	SSC_TOKEN_KW_ARRAY,
	SSC_TOKEN_KW_SEQ,
	SSC_TOKEN_KW_OPT,
	SSC_TOKEN_KW_STRUCT,
	SSC_TOKEN_KW_KLASS,
	SSC_TOKEN_KW_REF,
	
	//Nonterminal symbols
	SSC_TOKEN_BASE_TYPE,
	SSC_TOKEN_TYPE,
	SSC_TOKEN_FIELD,
	SSC_TOKEN_
} SscToken;
*/

//Semantic data type
typedef enum 
{
	SSC_SEM_STR,
	SSC_SEM_NUM,
	SSC_SEM_BASE_TYPE,
	SSC_SEM_TYPE,
	SSC_SEM_FIELD,
	SSC_SEM_TUPLE,
	SSC_SEM_STRUCT,
	SSC_SEM_FN,
	SSC_SEM_IFACE,
} SscSemType;
 
typedef struct _SscSem SscSem;
struct _SscSem
{
	SscSemType type;
};

typedef struct 
{
	
};

typedef struct
{
	MmcRC parent;
	
	
} SscSymbolDB;

