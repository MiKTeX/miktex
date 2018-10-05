%{
/* otp.l: Lexical analysis for OTP files

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
#include "routines.h"
#if defined(MIKTEX_WINDOWS)
#  undef INPUT
#endif
#include "yystype.h"
#include "y_tab.h"
#define YY_NO_UNPUT
int line_number = 1;
int i,j,last;
char newtext[10000];
extern YYSTYPE yylval;
%}


hexnumber	@\"[a-fA-F0-9]+
octnumber	@'[0-7]+
decnumber	[0-9]+
id		([a-zA-Z][_a-zA-Z0-9]*)
string		\"([^\"]|\"\")*\"

%%
[ \t]		{}
"%".*\n		{ line_number++;}
\n		{ line_number++;}
{hexnumber}	{
		yylval.yint=0;
		for (i=2; i<yyleng; i++) {
			j = yytext[i];
			if ((j>='0')&&(j<='9')) {
				yylval.yint = (yylval.yint*16)+(j-'0');
			} else if ((j>='a')&&(j<='f')) {
				yylval.yint = (yylval.yint*16)+(j-'a'+10);
			} else { /* ((j>='A') && (j>='F')) */
				yylval.yint = (yylval.yint*16)+(j-'A'+10);
			}
		}
		if (yylval.yint>65535) {
		   fprintf(stderr, "line %d: value too large (%x)\n",
			   line_number, yylval.yint);
		   exit(1);
		}
		return(NUMBER);
		}
{octnumber}	{
		yylval.yint=0;
		for (i=2; i<yyleng; i++) {
			j = yytext[i];
			yylval.yint = (yylval.yint*8)+(j-'0');
		}
		if (yylval.yint>65535) {
		   fprintf(stderr, "line %d: value too large (%o)\n",
			   line_number, yylval.yint);
		   exit(1);
		}
		return(NUMBER);
		}
{decnumber}	{
		yylval.yint=0;
		for (i=0; i<yyleng; i++) {
			j = yytext[i];
			yylval.yint = (yylval.yint*10)+(j-'0');
		}
		if (yylval.yint>65535) {
		    fprintf(stderr, "line %d: value too large (%d)\n",
			    line_number, yylval.yint);
		    exit(1);
		}
		return(NUMBER);
		}
"`"[\x20-\x7e]"'" {
		yylval.yint=yytext[1]; 
		return(NUMBER);
}
{id}		{ yylval.ystring=xstrdup(yytext); return(ID); }
{string}	{
		last=yyleng-2;
		i=0;
		j=1;
		while (j<=last) {
		   newtext[i]=yytext[j];
		   if ((yytext[j]=='"') && (yytext[j+1]=='"')) j=j+2;
		   else j++;
		   i++;
		}
		newtext[i]='\0';
		yylval.ystring = xstrdup(newtext);
		return(STRING);
		}
=>		return(RIGHTARROW);
"<"=		return(LEFTARROW);
input:		return(INPUT);
output:		return(OUTPUT);
aliases:	return(ALIASES);
states:		return(STATES);
tables:		return(TABLES);
expressions:	return(EXPRESSIONS);
push:		return(PUSH);
pop:		return(POP);
div:		return(DIV);
mod:		return(MOD);
beg:		return(BEG);
end:		return(END);
.		return(yytext[0]);

%%

