/* lex.l
 * Lexical scanner for SID language
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

%option prefix=ssc_yy
%option reentrant

%x COMMENT STRING

%{
#include <sidc/incl.h>
#include "parser.tab.h"
%}


%%

//Comments
"/*" { BEGIN(COMMENT); }
<COMMENT>"/*" { /*Warn about / * in comment*/ }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>. { /*Do nothing*/ }
"//"[^\n]* { /*Do nothing*/ }

//Whitespace
[ \t\n]* { /*do nothing*/ }

//Punctuation and operators
"{" { return LCURLY; }
"}" { return RCURLY; }
"(" { return LPAREN; }
")" { return RPAREN; }
"," { return COMMA; }
";" { return SC; }
":" { return COLON; }
"/" { return DIV; }
"*" { return MULT; }
"+" { return PLUS; }
"-" { return MINUS; }
"%" { return MOD; }
"=" { return EQUAL; }

//Keywords
"int8" { return KW_INT8; }
"int16" { return KW_INT16; }
"int32" { return KW_INT32; }
"int64" { return KW_INT64; }
"uint8" { return KW_UINT8; }
"uint16" { return KW_UINT16; }
"uint32" { return KW_UINT32; }
"uint64" { return KW_UINT64; }
"flt32" { return KW_FLT32; }
"flt64" { return KW_FLT64; }
"string" { return KW_STRING; }
"msg" { return KW_MSG; }
"array" { return KW_ARRAY; }
"seq" { return KW_SEQ; }
"optional" { return KW_OPTIONAL; }
"struct" { return KW_STRUCT; }
"interface" { return KW_INTERFACE; }
"ref" { return KW_REF; }
"integer" { return KW_INTEGER; }


//Terminal symbols with valuable lexemes
[A-Za-z_][A-Za-z0-9_]* { return VAL_ID; }
"0x"[0-9A-Fa-f]+ { return VAL_NUM; }
"0"[0-7]+ {return VAL_NUM; }
[0-9]+ { return VAL_NUM; }

//String
"\"" { BEGIN(STRING); }
//Backslash escaped characters
<STRING>"\\\"" { /*append " */ }
<STRING>"\\\\" { /*append \ */ }
<STRING>"\\a" { /*append \a*/ }
<STRING>"\\b" { /*append \b*/ }
<STRING>"\\f" { /*append \f*/ }
<STRING>"\\n" { /*append \n*/ }
<STRING>"\\r" { /*append \r*/ }
<STRING>"\\t" { /*append \t*/ }
<STRING>"\\v" { /*append \v*/ }
<STRING>"\\"[0-7]{1-3} { /*append indicated character*/ }
<STRING>"\\x"[0-9a-fA-F]{1-2} { /*append indicated character*/ }
<STRING>"\"" { BEGIN(INITIAL); return VAL_STRING; }

//Error
. { /*Unrecognised character $yytext*/ }

%%

