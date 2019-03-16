/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This header prototypes the vowel handling data structures and entry points.
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

#ifndef _VOWEL_H
#define _VOWEL_H

#include <stdio.h>
#include "bool.h"
#include "unicode.h"

/* Use unicode 0xfffe, which is guaranteed not to be a character */
#define GREVOWEL_ELISION_MARK ((grewchar)0xfffe)

typedef enum rulefile_parse_status {
    RFPS_NOT_FOUND = 0,
    RFPS_FOUND,
    RFPS_ALIASED
} rulefile_parse_status;

int gregorio_vowel_rulefile_parse(const char *filename, char **language,
        rulefile_parse_status *status);
int gregorio_vowel_rulefile_lex_destroy(void);
void gregorio_vowel_tables_init(void);
void gregorio_vowel_tables_load(const char *filename, char **language,
        rulefile_parse_status *status);
void gregorio_vowel_tables_free(void);
void gregorio_vowel_table_add(const char *vowels);
void gregorio_prefix_table_add(const char *prefix);
void gregorio_suffix_table_add(const char *suffix);
void gregorio_secondary_table_add(const char *secondary);
bool gregorio_find_vowel_group(const grewchar *subject, int *start, int *end);

#endif
