/* parser.y
 * Parser for SID language
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

%{
#include <sidc/incl.h>
%}

%define api.value.type {void *}
%define api.pure full

//Punctuation and operators
%token LCURLY
%token RCURLY
%token LPAREN
%token RPAREN
%token COMMA
%token SC
%token COLON
%token DIV
%token MULT
%token PLUS
%token MINUS
%token MOD
%token EQUAL

//Keywords
%token KW_INT8
%token KW_INT16
%token KW_INT32
%token KW_INT64
%token KW_UINT8
%token KW_UINT16
%token KW_UINT32
%token KW_UINT64
%token KW_FLT32
%token KW_FLT64
%token KW_STRING
%token KW_MSG
%token KW_ARRAY
%token KW_SEQ
%token KW_OPTIONAL
%token KW_STRUCT
%token KW_INTERFACE
%token KW_REF
%token KW_INTEGER

//Terminal symbols with valuable lexemes
%token VAL_ID
%token VAL_STRING
%token VAL_NUM


%%

//Main rules
file: %empty 
	| file def SC  
	;

def: struct
	| interface
	| ref
	| constant
	;

//Structure
struct: KW_STRUCT VAL_ID LCURLY field_list RCURLY
	;

field_list: %empty
	| field_list field_def
	;

field_def: type field_ids SC
	;

field_ids: VAL_ID
	| field_ids COMMA VAL_ID
	;

//Interface
interface: KW_INTERFACE VAL_ID LCURLY fn_list RCURLY
	| KW_INTERFACE VAL_ID COLON VAL_ID LCURLY fn_list RCURLY
	;
	
fn_list: %empty
	| fn_list fn_def
	;

fn_def: VAL_ID LPAREN args RPAREN SC
	| VAL_ID LPAREN args RPAREN colon LPAREN args RPAREN SC
	;

args: %empty
	| arg_def
	| args COMMA arg_def
	;

arg_def: type arg_ids
	;

arg_ids: VAL_ID
	| arg_ids VAL_ID
	;

//Reference
ref: KW_REF string_exp

//Constants
constant: KW_INTEGER VAL_ID EQUAL integer_exp
	| KW_STRING VAL_ID EQUAL string_exp
	;

//Types
type: base_type
	| KW_ARRAY lparen integer_exp rparen base_type
	| KW_SEQ base_type
	| KW_OPTIONAL base_type
	;

base_type: KW_INT8
	| KW_INT16
	| KW_INT32
	| KW_INT64
	| KW_UINT8
	| KW_UINT16
	| KW_UINT32
	| KW_UINT64
	| KW_FLT32
	| KW_FLT64
	| KW_STRING
	| KW_MSG
	| VAL_ID
	;

//Integer expressions
integer_exp: sum
	;

sum: product
	| sum PLUS product
	| sum MINUS product
	;

product: integer_val
	| product MULT integer_val
	| product DIV integer_val
	| product MOD integer_val
	;
	
integer_val: VAL_INT
	| VAL_ID
	| LPAREN integer_exp RPAREN
	| MINUS integer_val
	;

//String expressions
string_exp: string_val
	| string_exp string_val
	;

string_val: VAL_STRING
	| VAL_ID
	;

%%

