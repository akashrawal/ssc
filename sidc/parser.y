/* parser.y
 * Parser for SID language 
 * Copyright 2015-2018 Akash Rawal
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

%define api.value.type {SscYYSType}
%define api.pure full
%define api.prefix {ssc_yy}
%param {yyscan_t yyscanner}
%parse-param {SscParser *parser}
%defines

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

%{
#include <sidc/incl.h>

//Fundamental type
#define ssc_ftype(lhs, tfid) \
	do { \
		lhs.xtype.sym = NULL; \
		lhs.xtype.fid = SSC_TYPE_FUNDAMENTAL_ ## tfid; \
		lhs.xtype.complexity = SSC_TYPE_NONE; \
	} while(0)

static void ssc_yyerror
	(yyscan_t yyscanner, SscParser *parser, const char *str)
{
	ssc_parser_error(parser, "%s", str);
}

%}

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
struct: KW_STRUCT VAL_ID LCURLY field_list RCURLY {
			if (ssc_parser_add_struct(parser, $2.xstr, $4.xrl)
				!= MMC_SUCCESS)
				YYABORT;
		}
	;

field_list: %empty { $$.xrl = NULL; }
	| field_list_open SC { $$.xrl = $1.xrl; }
	;

field_list_open: field_list type VAL_ID { 
			SscVar *var = ssc_parser_new_var
				(parser, $2.xtype, $3.xstr);
			$$.xrl = ssc_parser_rlist_prepend
				(parser, $1.xrl, var);
		}
	| field_list_open COMMA VAL_ID {
			SscVar *var = ssc_parser_new_var
				(parser, $1.xrl->d.xvar->type, $3.xstr);
			$$.xrl = ssc_parser_rlist_prepend
				(parser, $1.xrl, var);
		}
	;

//Interface
interface: KW_INTERFACE VAL_ID LCURLY fn_list RCURLY {
			if (ssc_parser_add_interface
						(parser, $2.xstr, NULL, $4.xrl)
					!= MMC_SUCCESS)
				YYABORT;
		}
	| KW_INTERFACE VAL_ID COLON VAL_ID LCURLY fn_list RCURLY{
			if (ssc_parser_add_interface
						(parser, $2.xstr, $4.xstr, $6.xrl)
					!= MMC_SUCCESS)
				YYABORT;
		}
	;
	
fn_list: %empty { $$.xrl = NULL; }
	| fn_list fn_def {
			$$.xrl = ssc_parser_rlist_prepend
				(parser, $1.xrl, $2.xfn);
		}
	;

fn_def: VAL_ID LPAREN args RPAREN SC {
			$$.xfn = ssc_parser_new_fn
				(parser, $1.xstr, $3.xrl, NULL);
		}
	| VAL_ID LPAREN args RPAREN COLON LPAREN args RPAREN SC {
			$$.xfn = ssc_parser_new_fn
				(parser, $1.xstr, $3.xrl, $7.xrl);
		}
	;

args : %empty { $$.xrl = NULL; }
	| args_nonempty { $$.xrl = $1.xrl; }
	;

args_nonempty: type VAL_ID { 
			SscVar *var = ssc_parser_new_var
				(parser, $1.xtype, $2.xstr);
			$$.xrl = ssc_parser_rlist_prepend
				(parser, NULL, var);
		}
	| args_nonempty VAL_ID {
			SscVar *var = ssc_parser_new_var
				(parser, $1.xrl->d.xvar->type, $2.xstr);
			$$.xrl = ssc_parser_rlist_prepend
				(parser, $1.xrl, var);
		}
	| args_nonempty COMMA type VAL_ID { 
			SscVar *var = ssc_parser_new_var
				(parser, $3.xtype, $4.xstr);
			$$.xrl = ssc_parser_rlist_prepend
				(parser, $1.xrl, var);
		}
	;

//Reference
ref: KW_REF string_exp {
			if (ssc_parser_exec_ref(parser, $2.xstr) != MMC_SUCCESS)
				YYABORT;
		}

//Constants
constant: KW_INTEGER VAL_ID EQUAL integer_exp {
			if (ssc_parser_add_integer_constant
						(parser, $2.xstr, $4.xint)
					!= MMC_SUCCESS)
				YYABORT;
		}
	| KW_STRING VAL_ID EQUAL string_exp {
			if (ssc_parser_add_string_constant
						(parser, $2.xstr, $4.xstr)
					!= MMC_SUCCESS)
				YYABORT;
		}
	;

//Types
type: base_type { $$.xtype = $1.xtype; }
	| KW_ARRAY LPAREN integer_exp RPAREN base_type {
			if ($3.xint <= 0)
			{
				ssc_parser_error(parser, "Array size should be > 0");
				YYABORT;
			}
			$$.xtype = $5.xtype;
			$$.xtype.complexity = $3.xint;
		}
	| KW_SEQ base_type { 
			$$.xtype = $2.xtype; 
			$$.xtype.complexity = SSC_TYPE_SEQ;
		} 
	| KW_OPTIONAL base_type { 
			$$.xtype = $2.xtype; 
			$$.xtype.complexity = SSC_TYPE_OPTIONAL;
		} 
	;

base_type: KW_INT8 { ssc_ftype($$, INT8); }
	| KW_INT16 { ssc_ftype($$, INT16); }
	| KW_INT32 { ssc_ftype($$, INT32); }
	| KW_INT64 { ssc_ftype($$, INT64); }
	| KW_UINT8 { ssc_ftype($$, UINT8); }
	| KW_UINT16 { ssc_ftype($$, UINT16); }
	| KW_UINT32 { ssc_ftype($$, UINT32); }
	| KW_UINT64 { ssc_ftype($$, UINT64); }
	| KW_FLT32 { ssc_ftype($$, FLT32); }
	| KW_FLT64 { ssc_ftype($$, FLT64); }
	| KW_STRING { ssc_ftype($$, STRING); }
	| KW_MSG { ssc_ftype($$, MSG); }
	| VAL_ID { 
			SscSymbol *sym = ssc_parser_lookup_expecting
				(parser, $1.xstr, SSC_SYMBOL_STRUCT);
			if (! sym)
				YYABORT;
			
			$$.xtype.sym = sym;
			$$.xtype.fid = SSC_TYPE_FUNDAMENTAL_NONE;
			$$.xtype.complexity = SSC_TYPE_NONE;
		}
	;

//Integer expressions
integer_exp: sum { $$.xint = $1.xint; }
	;

sum: product { $$.xint = $1.xint; }
	| sum PLUS product { $$.xint = $1.xint + $3.xint; }
	| sum MINUS product { $$.xint = $1.xint - $3.xint; }
	;

product: integer_val { $$.xint = $1.xint; }
	| product MULT integer_val { $$.xint = $1.xint * $3.xint; }
	| product DIV integer_val { $$.xint = $1.xint / $3.xint; }
	| product MOD integer_val { $$.xint = $1.xint % $3.xint; }
	;
	
integer_val: VAL_NUM { $$.xint = $1.xint; }
	| VAL_ID { 
			SscSymbol *sym = ssc_parser_lookup_expecting
				(parser, $1.xstr, SSC_SYMBOL_INTEGER);
			if (! sym)
				YYABORT;
			
			$$.xint = sym->v.xint;
		}
	| LPAREN integer_exp RPAREN { $$.xint = $2.xint; }
	| MINUS integer_val { $$.xint = - $2.xint; }
	;

//String expressions
string_exp: string_val { $$.xstr = $1.xstr; }
	| string_exp string_val { $$.xstr = ssc_parser_strcat(parser, $1.xstr, $2.xstr); }
	;

string_val: VAL_STRING { $$.xstr = $1.xstr; }
	| VAL_ID {
			SscSymbol *sym = ssc_parser_lookup_expecting
				(parser, $1.xstr, SSC_SYMBOL_STRING);
			if (! sym)
				YYABORT;
			
			$$.xstr = sym->v.xstr;
		}
			
	;

%%



