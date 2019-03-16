%{
/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file implements the vowel rule parser.
 *
 * Copyright (C) 2015-2019 The Gregorio Project (see CONTRIBUTORS.md)
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
#include <assert.h>
#include "bool.h"
#include "struct.h"
#include "unicode.h"
#include "messages.h"

#include "vowel.h"

#include "vowel-rules.h"
#include "vowel-rules-l.h"

/* NOTE: This parser might allocate a new value for language; this value MUST
 *       BE FREED after the parser returns (if the value of the language pointer
 *       changes, then free the pointer).  This parser DOES free the language
 *       pointer before changing it, if status points to RFPS_ALIASED. */

/* uncomment it if you want to have an interactive shell to understand the
 * details on how bison works for a certain input */
/*int gregorio_vowel_rulefile_debug=1;*/

static void gregorio_vowel_rulefile_error(const char *const filename,
        char **const language __attribute__((__unused__)),
        rulefile_parse_status *const status __attribute__((__unused__)),
        const char *const error_str)
{
    gregorio_messagef("gregorio_vowel_rulefile_parse", VERBOSITY_ERROR, 0,
            _("%s: %s"), filename, error_str);
}

/* this returns false until the language *after* the desired language */
static __inline bool match_language(char **language,
        rulefile_parse_status *status, char *const name)
{
    if (*status == RFPS_FOUND) {
        free(name);
        return true;
    }

    if (strcmp(*language, name) == 0) {
        *status = RFPS_FOUND;
    }

    free(name);
    return false;
}

static __inline void alias(char **const language,
        rulefile_parse_status *const status, char *const name,
        char *const target)
{
    if (strcmp(*language, name) == 0) {
        gregorio_messagef("alias", VERBOSITY_INFO, 0, _("Aliasing %s to %s"),
                name, target);
        if (*status == RFPS_ALIASED) {
            free(*language);
        }
        *language = target;
        *status = RFPS_ALIASED;
    } else {
        free(target);
    }
    free(name);
}

static __inline void add(const rulefile_parse_status *const status,
        void (*const fn)(const char *), char *const value)
{
    if (*status == RFPS_FOUND) {
        fn(value);
    }
    free(value);
}

#define _MATCH(NAME) if (match_language(language, status, NAME)) YYACCEPT
#define _ALIAS(NAME, TARGET) alias(language, status, NAME, TARGET)
#define _ADD(TABLE, CHARS) add(status, gregorio_##TABLE##_table_add, CHARS)

%}

%name-prefix "gregorio_vowel_rulefile_"
%parse-param { const char *const filename }
%parse-param { char **language }
%parse-param { rulefile_parse_status *const status }

%token LANGUAGE VOWEL PREFIX SUFFIX SECONDARY ALIAS SEMICOLON TO
%token NAME CHARACTERS INVALID

%%

rules
    :
    | rules rule
    ;

rule
    : LANGUAGE NAME SEMICOLON       { _MATCH($2); }
    | ALIAS NAME TO NAME SEMICOLON  { _ALIAS($2, $4); }
    | VOWEL vowels SEMICOLON
    | PREFIX prefixes SEMICOLON
    | SUFFIX suffixes SEMICOLON
    | SECONDARY secondaries SEMICOLON
    ;

vowels
    :
    | vowels CHARACTERS             { _ADD(vowel, $2); }
    ;

prefixes
    :
    | prefixes CHARACTERS           { _ADD(prefix, $2); }
    ;

suffixes
    :
    | suffixes CHARACTERS           { _ADD(suffix, $2); }
    ;

secondaries
    :
    | secondaries CHARACTERS        { _ADD(secondary, $2); }
    ;
