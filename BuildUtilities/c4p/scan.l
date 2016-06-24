/* scan.l: C4P Lexical Scanner Specification            -*- C++ -*-

   Copyright (C) 1991-2016 Christian Schenk

   This file is part of C4P.

   C4P is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
    
   C4P is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.
    
   You should have received a copy of the GNU General Public License
   along with C4P; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

%{
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#if defined(MIKTEX_WINDOWS)
#  include <io.h>
#endif

#include "common.h"
#include "gram.h"
#if defined(MIKTEX_WINDOWS)
#define YY_NO_UNISTD_H
#endif
  
extern YYSTYPE yylval;

namespace {
  int last_tok;
}

void comment ();
int string_or_char ();
%}

D               [0-9]
L               [A-Za-z]
E               [Ee][+-]?{D}+

%%

{L}({L}|{D})*   {
                  symbol_t * s = lookup(yytext);
                  if (s->s_kind == PASCAL_KEYWORD || s->s_kind == BUILD_IN_IDENTIFIER)
                  {
                    return last_tok = static_cast<int>(s->s_type);
                  }
                  yylval.sym = s;
                  return last_tok = IDENTIFIER;
                }

{D}+            {
                  std::istringstream str(yytext);
                  str >> yylval.value;
                  return last_tok = UNSIGNED_NUMBER;
                }

{D}+\.{D}+{E}   |
{D}+\.{D}+      |
{D}+{E}         {
                  std::istringstream str(yytext);
                  str >> yylval.fvalue;
                  return last_tok = REAL_CONSTANT;
                }

"<>"            { return last_tok = NEQ; }
"<="            { return last_tok = LEQ; }
">="            { return last_tok = GEQ; }
":="            { return last_tok = ASSIGN; }
".."            { return last_tok = DOTDOT; }

"(."            { return last_tok = '['; }
".)"            { return last_tok = ']'; }
"@"             { return last_tok = '^'; }

"{"             { comment(); }

[\r \t]+        {
                }

\n              {
                  yylineno++;
                }

'               {
                  return last_tok = string_or_char();
                }

.               { return last_tok = *yytext; }

%%

void comment()
{
   char c;
   while ((c = yyinput()) != '}' && c != 0)
   {
     if (c == '\n')
     {
       yylineno++;
     }
   }
}

namespace {
  char temp_buf[ 8192 ];
}

int string_or_char()
{
  unsigned len = 0;
  char c;
  while ((c = yyinput()) != 0)
  {
    if (c == '\'')
    {
      char c1 = yyinput();
      if (c1 != '\'')
      {
        if (c1 != 0)
        {
          unput(c1);
        }
        break;
      }
    }
    if (len >= sizeof(temp_buf))
    {
      c4p_error("internal error: string literal too long");
    }
    temp_buf[len++] = c;
  }
  temp_buf[len] = 0;
  if (len == 1)
  {
    yylval.value = temp_buf[0];
    return CHARACTER_CONSTANT;
  }
  else
  {
    yylval.value = static_cast<long>(new_string(temp_buf));
    return CHARACTER_STRING;
  }
}
