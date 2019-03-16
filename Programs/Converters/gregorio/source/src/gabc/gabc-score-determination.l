%{
/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file implements the score lexer.
 *
 * Gregorio score determination in gabc input.
 * Copyright (C) 2006-2019 The Gregorio Project (see CONTRIBUTORS.md)
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
#include "bool.h"
#include "support.h"

#include "gabc.h"
#include "gabc-score-determination.h"
#include "gabc-score-determination-y.h"

static bool eof_found = false;

#define START_STYLE(STYLE) \
    if (*styles & SB_ ## STYLE) { \
        gregorio_messagef("gabc_score_determination_lex", VERBOSITY_ERROR, 0, \
                _("style already started: %s"), gabc_score_determination_text); \
    } else { \
        *styles ^= SB_ ## STYLE; \
        return STYLE ## _BEGIN; \
    }

#define END_STYLE(STYLE) \
    if (*styles & SB_ ## STYLE) { \
        *styles ^= SB_ ## STYLE; \
        return STYLE ## _END; \
    } else { \
        gregorio_messagef("gabc_score_determination_lex", VERBOSITY_ERROR, 0, \
                _("style not started: %s"), gabc_score_determination_text); \
    }

#define RETURN_CHARACTERS \
    gabc_score_determination_lval.text = \
            gregorio_strdup(gabc_score_determination_text); \
    return CHARACTERS

#define RETURN_SPACE \
    gabc_score_determination_lval.text = gregorio_strdup(" "); \
    return CHARACTERS

#define YY_NO_INPUT

#define YY_INPUT(buf,result,max_size) \
    if ( YY_CURRENT_BUFFER_LVALUE->yy_is_interactive ) { \
        int c = '*'; \
        int n; \
        for (n = 0; n < max_size \
                && (c = getc(gabc_score_determination_in)) != EOF \
                && c != '\n'; ++n ) { \
            buf[n] = (char) c; \
        } \
        if (c == '\n') { \
            buf[n++] = (char) c; \
        } \
        if (c == EOF && ferror(gabc_score_determination_in)) { \
            YY_FATAL_ERROR("input in flex scanner failed"); \
        } \
        result = n; \
    } else { \
        errno=0; \
        while ((result = fread(buf, 1, max_size, gabc_score_determination_in)) \
                == 0 && ferror(gabc_score_determination_in)) { \
            if (errno != EINTR) { \
                YY_FATAL_ERROR("input in flex scanner failed"); \
                break; \
            } \
            errno = 0; \
            clearerr(gabc_score_determination_in); \
        } \
    } \
    gabc_digest(buf, result)

#define YY_USER_ACTION gabc_update_location(&gabc_score_determination_lloc, \
        gabc_score_determination_text, gabc_score_determination_leng);

%}

%x attribute
%x score
%x notes
%x sp
%x verb
%x comments
%x inicomments
%x alt
%x protrusion_value
%x protrusion_end

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
%option prefix="gabc_score_determination_"
%option noyywrap
%option 8bit


/* The expression for attribute below is rather messy because we allow
for (a) single-line values, ending with a semicolon at end of line or a
double semicolon, (b) multi-line values, which end at a double
semicolon. */

%%
<INITIAL>^(\xBB|\xEF|\xBF)+ {
        /* BOM written by a lot of windows softwares when they write UTF-8 */
    }
<INITIAL>^[\n\r]+ {
        /* ignoring empty lines */
    }
<INITIAL>^[\%#] {
        BEGIN(inicomments);
    }
<inicomments>(\n|\r)+ {
        BEGIN(INITIAL);
    }
<inicomments>[^\n\r]+ {
        /* ignored */
    }
<INITIAL>:(\ )? {
        BEGIN(attribute);
        return COLON;
    }
<attribute>;;?[\n\r \t]*[\n\r] {
         BEGIN(INITIAL);
         return SEMICOLON;
    }
<attribute>[^;]+ {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return ATTRIBUTE;
    }
<attribute>; {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return ATTRIBUTE;
}
<INITIAL>def-m[0-9] {
         gabc_score_determination_lval.character = gabc_score_determination_text[5];
         return DEF_MACRO;
    }
<INITIAL>name {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
         return NAME;
    }
<INITIAL>score-copyright {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return SCORE_COPYRIGHT;
    }
<INITIAL>gabc-copyright {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return GABC_COPYRIGHT;
    }
<INITIAL>mode {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return MODE;
    }
<INITIAL>mode-modifier {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return MODE_MODIFIER;
    }
<INITIAL>mode-differentia {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return MODE_DIFFERENTIA;
    }
<INITIAL>annotation {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return ANNOTATION;
    }
<INITIAL>author {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return AUTHOR;
    }
<INITIAL>language {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return LANGUAGE;
    }
<INITIAL>staff-lines {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return STAFF_LINES;
    }
<INITIAL>nabc-lines {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return NABC_LINES;
    }
<INITIAL>oriscus-orientation {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return ORISCUS_ORIENTATION;
    }
<INITIAL>[A-Za-z0-9_]+(-[A-Za-z0-9_]+)* {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return OTHER_HEADER;
    }
<INITIAL>%%+(\n|\r)+ {
       BEGIN(score);
       return END_OF_DEFINITIONS;
    }
<INITIAL>. {
        gregorio_messagef("det_score", VERBOSITY_ERROR, 0,
                _("unrecognized character: \"%c\" in definition part"),
                gabc_score_determination_text[0]);
    }
<score>[\n\r][\n\r \t]* {
        RETURN_SPACE;
    }
<score>[^-,;:.\{\}\(\[\]<%\n\r]+ {
        RETURN_CHARACTERS;
    }
<score>- {
        return HYPHEN;
    }
<score>[,;:.] {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return PROTRUDING_PUNCTUATION;
    }
<score><nlba> {
        return NLBA_B;
    }
<score><\/nlba> {
        return NLBA_E;
    }
<score><i> {
        START_STYLE(I);
    }
<score><\/i> {
        END_STYLE(I);
    }
<score><tt> {
        START_STYLE(TT);
    }
<score><\/tt> {
        END_STYLE(TT);
    }
<score><ul> {
        START_STYLE(UL);
    }
<score><\/ul> {
        END_STYLE(UL);
    }
<score><c> {
        START_STYLE(C);
    }
<score><\/c> {
        END_STYLE(C);
    }
<score><b> {
        START_STYLE(B);
    }
<score><\/b> {
        END_STYLE(B);
    }
<score><sc> {
        START_STYLE(SC);
    }
<score><\/sc> {
        END_STYLE(SC);
    }
<score><e> {
        START_STYLE(ELISION);
    }
<score><\/e> {
        END_STYLE(ELISION);
    }
<score><sp> {
        BEGIN(sp);
        return SP_BEGIN;
    }
<sp><\/sp> {
        BEGIN(score);
        return SP_END;
    }
<sp>[\n\r][\n\r \t]* {
        RETURN_SPACE;
    }
<sp>[^<\{\}\n\r]+ {
        RETURN_CHARACTERS;
    }
<score>\% {
        BEGIN(comments);
    }
<comments>(\n|\r)+ {
        BEGIN(score);
    }
<comments>[^\n\r]+ {
        /* ignored */
    }
<score><v> {
        BEGIN(verb);
        return VERB_BEGIN;
    }
<verb><\/v> {
        BEGIN(score);
        return VERB_END;
    }
<verb,alt>[^<]+ {
        RETURN_CHARACTERS;
    }
<verb,score,alt>< {
        RETURN_CHARACTERS;
    }
<score>\{ {
        return CENTER_BEGIN;
    }
<score>\} {
        return CENTER_END;
    }
<score><alt> {
        BEGIN(alt);
        return ALT_BEGIN;
    }
<alt><\/alt> {
        BEGIN(score);
        return ALT_END;
    }
<score><eu>  {
        return EUOUAE_B;
    }
<score><\/eu>  {
        return EUOUAE_E;
    }
<score>\[\/] {
        return TRANSLATION_CENTER_END;
    }
<score>\[ {
        return TRANSLATION_BEGIN;
    }
<score>\] {
        return TRANSLATION_END;
    }
<score><[\n\r \t]*clear[\n\r \t]*\/?[\n\r \t]*> {
        return CLEAR;
    }
<score><[\n\r \t]*pr[\n\r \t]*\/?[\n\r \t]*> {
        return PROTRUSION;
    }
<score><[\n\r \t]*pr[\n\r \t]*:[\n\r \t]* {
        BEGIN(protrusion_value);
        return PROTRUSION;
    }
<protrusion_value>([0-9]*\.?[0-9]+|[0-9]+\.) {
        BEGIN(protrusion_end);
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return PROTRUSION_VALUE;
    }
<protrusion_end>[\n\r \t]*\/?[\n\r \t]*> {
        BEGIN(score);
        return PROTRUSION_END;
    }
<score>\( {
        BEGIN(notes);
        return OPENING_BRACKET;
    }
<notes>[^|\)]+ {
        gabc_score_determination_lval.text =
                gregorio_strdup(gabc_score_determination_text);
        return NOTES;
    }
<notes>\| {
        return NABC_CUT;
    }
<notes>\) {
        BEGIN(score);
        return CLOSING_BRACKET;
    }
<notes>\)(\ |\t|\n|\r)+ {
        BEGIN(score);
        return CLOSING_BRACKET_WITH_SPACE;
    }
<<EOF>> {
        if (!eof_found) {
            eof_found = true;
            return END_OF_FILE;
        } else {
            yyterminate();
        }
    }
.|\n {
        gregorio_messagef("gabc_score_determination_lex", VERBOSITY_ERROR, 0,
                _("unrecognized character: \"%c\""),
                gabc_score_determination_text[0]);
    }
%%

