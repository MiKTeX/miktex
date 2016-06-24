%{
/* otp.y: Grammar for OTP files.

This file is part of the Omega project, which
is based in the web2c distribution of TeX.

Copyright (c) 1994--2000 John Plaice and Yannis Haralambous

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#if defined(MIKTEX_WINDOWS)
#  define INPUT INPUTx
#endif
#include "otp.h"
#include "routines.h"
#include "yystype.h"
#if defined(MIKTEX_WINDOWS)
#  undef INPUT
#endif
int k, len;

void yyerror(msg)
char *msg;
{
fprintf(stderr, "line %d: %s\n", line_number, msg);
}
%}

%token NUMBER
%token ID
%token STRING
%token LEFTARROW
%token RIGHTARROW
%token INPUT
%token OUTPUT
%token ALIASES
%token STATES
%token TABLES
%token EXPRESSIONS
%token PUSH
%token POP
%token DIV
%token MOD
%token BEG
%token END

%left '+' '-'
%left '*' DIV MOD

%%

File :
	Input
	Output
	Tables
	States
	Aliases
	Expressions
    ;

Input :
	/* Empty */
	{ input_bytes=2; }
    |	INPUT NUMBER ';'
	{ input_bytes=$2.yint; }
    ;

Output :
	/* Empty */
	{ output_bytes=2; }
    |	OUTPUT NUMBER ';'
	{ output_bytes=$2.yint; }
    ;

Tables :
	/* Empty */
    |	TABLES MoreTables
    ;

MoreTables :
	OneTable
    |   MoreTables OneTable
    ;

OneTable :
	ID '[' NUMBER ']'
	{ store_table($1.ystring, $3.yint); }
	'=' '{' Numbers '}' ';'
    ;

Numbers :
	/* Empty */
    |	MoreNumbers
    ;

MoreNumbers :
	NUMBER
	{ add_to_table($1.yint); }
    |   MoreNumbers ',' NUMBER
	{ add_to_table($3.yint); }
    ;

States :
	/* Empty */
    |	STATES MoreStates ';'
    ;

MoreStates :
	ID
	{ store_state($1.ystring); }
    |   MoreStates ',' ID
	{ store_state($3.ystring); }
    ;

Aliases :
	/* Empty */
    |	ALIASES MoreAliases
    ;

MoreAliases :
	OneAlias
    |	MoreAliases OneAlias
    ;

OneAlias :
	ID '=' OneCompleteLeft ';'
	{ store_alias($1.ystring, $3.yleft); }
    ;

OneCompleteLeft :
	STRING
	{ $$.yleft = StringLeft($1.ystring); }
    |   OneLeft '<' NUMBER ',' NUMBER '>'
	{ $$.yleft = CompleteLeft($1.yleft, $3.yint, $5.yint); }
    |	OneLeft '<' NUMBER ',' '>'
	{ $$.yleft = PlusLeft($1.yleft, $3.yint); }
    |	OneLeft
	{ $$.yleft = $1.yleft; }
    ;

OneLeft :
	NUMBER
	{ $$.yleft = SingleLeft($1.yint); }
    |	NUMBER '-' NUMBER
	{ $$.yleft = DoubleLeft($1.yint, $3.yint); }
    |   '.'
	{ $$.yleft = WildCard(); }
    |   '^' '(' ChoiceLeft ')'
	{ $$.yleft = NotChoiceLeft($3.ylleft); }
    |   '(' ChoiceLeft ')'
	{ $$.yleft = ChoiceLeft($2.ylleft); }
    |	'{' ID '}'
	{ $$.yleft = lookup_alias($2.ystring); }
    ;

ChoiceLeft :
	OneLeft
	{ $$.ylleft = llist1($1.yleft); }
    |	ChoiceLeft '|' OneLeft
	{ $$.ylleft = lappend1($1.ylleft, $3.yleft); }
    ;

Expressions :
    	EXPRESSIONS MoreExpressions
	{
	  for(cur_state=0; cur_state<no_states; cur_state++) {
		  if ((states[cur_state].no_exprs)==0) {
        	     out_int(OTP_LEFT_START, 0);
		  } else {
        	     out_int(OTP_LEFT_RETURN, 0);
                  }
		  out_int(OTP_RIGHT_CHAR, 1);
		  out_int(OTP_STOP, 0);
	  }
	}
    ;

MoreExpressions :
	OneExpr
    |	MoreExpressions OneExpr
    ;

OneExpr :
	LeftState 
	{ states[cur_state].no_exprs++; }
	TotalLeft 
	{ out_left($3.ylleft); right_offset=0; } 
	RIGHTARROW Right 
	{ right_offset=OTP_PBACK_OFFSET; } 
	PushBack RightState ';'
	{ fill_in_left(); }
    ;

PushBack :
    	/* Empty */
    |	LEFTARROW Right
    ;

LeftState :
	/* Empty */
	{ cur_state = 0; }
    |   '<' ID '>'
	{ cur_state = lookup_state($2.ystring); }
    ;

TotalLeft :
	BegLeft Left EndLeft
	{ $$.ylleft = lappend($1.ylleft, lappend($2.ylleft, $3.ylleft)); }
|	BegLeft EndLeft
	{ $$.ylleft = lappend($1.ylleft, $2.ylleft); }
    ;

BegLeft :
	/* Empty */
	{ $$.ylleft = nil; }
    |   BEG
	{ $$.ylleft = llist1(BeginningLeft()); }
    ;

EndLeft :
	/* Empty */
	{ $$.ylleft = nil; }
    |   END
	{ $$.ylleft = llist1(EndLeft()); }
    ;

Left :
	OneCompleteLeft
	{ $$.ylleft = llist1($1.yleft); }
    |	Left OneCompleteLeft
	{ $$.ylleft = lappend1($1.ylleft, $2.yleft); }
    ;

Right :
	/* Empty */
    |	Right OneRight
    ;

OneRight :
	STRING
	{
	 len=strlen($1.ystring);
	 for (k=0; k<len; k++) {
            out_right(OTP_RIGHT_NUM, ($1.ystring)[k]);
         }
	}
    |	NUMBER
	{ out_right(OTP_RIGHT_NUM, $1.yint); }
    |	'\\' NUMBER
	{ out_right(OTP_RIGHT_CHAR, $2.yint); }
    |	'\\' '$'
	{ out_right(OTP_RIGHT_LCHAR, 0); }
    |   '\\' '(' '$' '-' NUMBER ')'
	{ out_right(OTP_RIGHT_LCHAR, $5.yint); }
    |   '\\' '*'
	{
	 out_right(OTP_RIGHT_SOME, 0); 
	 out_int(0,0);
	}
    |   '\\' '(' '*' '+' NUMBER ')'
	{
	 out_right(OTP_RIGHT_SOME, $5.yint);
	 out_int(0, 0);
	}
    |   '\\' '(' '*' '-' NUMBER ')'
	{
	 out_right(OTP_RIGHT_SOME, 0);
	 out_int(0, $5.yint);
	}
    |   '\\' '(' '*' '+' NUMBER '-' NUMBER ')'
	{
	 out_right(OTP_RIGHT_SOME, $5.yint);
	 out_int(0, $7.yint);
	}
    |	'#' OneRightExpr
	{ out_right(OTP_RIGHT_OUTPUT, 0); }
    ;

RestRightExpr :
	OneRightExpr
    |	RestRightExpr '+' OneRightExpr
	{ out_int(OTP_ADD, 0); }
    |	RestRightExpr '-' OneRightExpr
	{ out_int(OTP_SUB, 0); }
    |	RestRightExpr '*' OneRightExpr
	{ out_int(OTP_MULT, 0); }
    |	RestRightExpr DIV OneRightExpr
	{ out_int(OTP_DIV, 0); }
    |	RestRightExpr MOD OneRightExpr
	{ out_int(OTP_MOD, 0); }
    |	ID
	{ out_int(OTP_PUSH_NUM, lookup_table($1.ystring)); }
	'[' RestRightExpr ']'
	{ out_int(OTP_LOOKUP, 0); }
    ;

OneRightExpr :
	NUMBER
	{ out_int(OTP_PUSH_NUM, $1.yint); }
    |	'\\' NUMBER
	{ out_int(OTP_PUSH_CHAR, $2.yint); }
    |	'\\' '$'
	{ out_int(OTP_PUSH_LCHAR, 0); }
    |   '\\' '(' '$' '-' NUMBER ')'
	{ out_int(OTP_PUSH_LCHAR, $5.yint); }
    |	'(' RestRightExpr ')'
    ;

RightState :
	/* Empty */
    |   '<' ID '>'
	{ out_int(OTP_STATE_CHANGE, lookup_state($2.ystring)); }
    |   '<' PUSH ID '>'
	{ out_int(OTP_STATE_PUSH, lookup_state($3.ystring)); }
    |   '<' POP '>'
	{ out_int(OTP_STATE_POP, 0); }
    ;
%%
