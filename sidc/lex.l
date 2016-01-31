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

%option outfile="lex.yy.c"
%option header-file="lex.yy.h"
%option prefix="ssc_yy"
%option batch
%option never-interactive
%option reentrant
%option noyywrap
%option extra-type = "SscParser *"
%option bison-bridge
%option nounistd

%x COMMENT STRING

%{
#include <sidc/incl.h>
#include "parser.tab.h"

#include <string.h>
#include <ctype.h>

#define parser (ssc_yyget_extra(yyscanner))
#define YYSTYPE SscYYSType

%}


%%

	/*Comments*/
"/*" { BEGIN(COMMENT); }
<COMMENT>"/*" { ssc_parser_warn(parser, "/* within comment"); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>[^\*]* { /*Do nothing*/ }
<COMMENT>. { /*Do nothing*/ }
"//"[^\n]* { /*Do nothing*/ }

	/*Whitespace*/
[ \t\n]* { /*do nothing*/ }

	/*Punctuation and operators*/
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

	/*Keywords*/
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


	/*Terminal symbols with valuable lexemes*/
[A-Za-z_][A-Za-z0-9_]* { 
	yylval->xstr = ssc_parser_strdup(parser, yytext);
	return VAL_ID; 
}

[0-9][A-Za-z0-9_]* {
	if (ssc_parser_read_int(parser, yytext, &(yylval->xint)) 
			!= MMC_SUCCESS)
		return -1;
	return VAL_NUM;
}

	/*String*/
"\"" { BEGIN(STRING); }
	/*Backslash escaped characters*/
<STRING>"\\\"" { ssc_parser_strbuf_add(parser, "\""); }
<STRING>"\\\\" { ssc_parser_strbuf_add(parser, "\\"); }
<STRING>"\\a" { ssc_parser_strbuf_add(parser, "\a"); }
<STRING>"\\b" { ssc_parser_strbuf_add(parser, "\b"); }
<STRING>"\\f" { ssc_parser_strbuf_add(parser, "\f"); }
<STRING>"\\n" { ssc_parser_strbuf_add(parser, "\n"); }
<STRING>"\\r" { ssc_parser_strbuf_add(parser, "\r"); }
<STRING>"\\t" { ssc_parser_strbuf_add(parser, "\t"); }
<STRING>"\\v" { ssc_parser_strbuf_add(parser, "\v"); }
<STRING>"\\"[0-7]{1,3} { ssc_parser_strbuf_add_by_num(parser, 8, yytext); }
<STRING>"\\x"[0-9a-fA-F]{1,2} { ssc_parser_strbuf_add_by_num(parser, 16, yytext); }
<STRING>"\"" { BEGIN(INITIAL); yylval->xstr = ssc_parser_strbuf_end(parser); return VAL_STRING; }
<STRING>. { ssc_parser_strbuf_add(parser, yytext); }

	/*Error*/
. { ssc_parser_warn(parser, "Unrecognised character %s", yytext); 
	return -1; }

%%

