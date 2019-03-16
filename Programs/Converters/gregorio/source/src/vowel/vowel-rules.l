%{
/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file implements the vowel rule lexer.
 *
 * Copyright (C) 2015-2019 The Gregorio Project (see CONTRIBUTORS.md)
 *
 * This file is part of Gregorio.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct.h"
#include "messages.h"
#include "support.h"

#include "vowel.h"

#include "vowel-rules.h"
#include "vowel-rules-y.h"

#define YY_NO_INPUT

static __inline void save_lval(void)
{
    gregorio_vowel_rulefile_lval =
            gregorio_malloc(gregorio_vowel_rulefile_leng + 1);
    strncpy(gregorio_vowel_rulefile_lval, gregorio_vowel_rulefile_text,
            gregorio_vowel_rulefile_leng);
    gregorio_vowel_rulefile_lval[gregorio_vowel_rulefile_leng] = '\0';
}

static __inline void invalid(void)
{
    gregorio_messagef("gregorio_vowel_rulefile_lex", VERBOSITY_WARNING, 0,
            _("invalid character in vowel file: %c"),
            *gregorio_vowel_rulefile_text);
}
%}

%x chars
%x lang
%x langname

%option stack
%option pointer
%option nounput
%option noyy_push_state
%option noyy_pop_state
%option noyy_top_state
%option align
%option noread
%option nomain
%option noalways-interactive
%option nonever-interactive
%option prefix="gregorio_vowel_rulefile_"
%option noyywrap
%option 8bit

%%

<INITIAL>language               { BEGIN(lang); return LANGUAGE; }
<INITIAL>vowel                  { BEGIN(chars); return VOWEL; }
<INITIAL>prefix                 { BEGIN(chars); return PREFIX; }
<INITIAL>suffix                 { BEGIN(chars); return SUFFIX; }
<INITIAL>secondary              { BEGIN(chars); return SECONDARY; }
<INITIAL>alias                  { BEGIN(lang); return ALIAS; }
<*>;                            { BEGIN(INITIAL); return SEMICOLON; }
<lang>to                        { return TO; }
<lang>"["                       { BEGIN(langname); }
<langname>[^\]]+                { save_lval(); return NAME; }
<langname>"]"                   { BEGIN(lang); }
<chars>[^;,# \t\n\r\v\f]+       { save_lval(); return CHARACTERS; }
<*>#[^\n\r]*                    { }
<INITIAL>\xEF\xBB\xBF           { }
<*>[\ \t\n\r\v\f]               { }
<*>.                            { invalid(); return INVALID; }

%%

