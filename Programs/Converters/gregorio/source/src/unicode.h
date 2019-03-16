/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This header prototypes the UTF-8 support functions.
 *
 * Copyright (C) 2008-2019 The Gregorio Project (see CONTRIBUTORS.md)
 *
 * This file is part of Gregorio.
 *
 * Gregorio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gregorio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gregorio.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UNICODE_H_FIRST_PART
#define UNICODE_H_FIRST_PART
#define UNICODE_H

#include <stdio.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif

typedef uint32_t grewchar;

void gregorio_print_unichar(FILE *f, grewchar to_print);
void gregorio_print_unistring(FILE *f, const grewchar *first_char);
grewchar *gregorio_build_grewchar_string_from_buf(const char *buf);

static __inline size_t gregorio_wcstrlen(const grewchar *wstr)
{
    size_t length = 0;

    while (*(wstr++)) {
        ++length;
    }

    return length;
}

/* this macro is for portability under windows, where L'x' is only two-bytes
 * long, and thus needs to be cast to a 4-bytes integer. */
#define GL(wc) ((const grewchar) L'wc')

#endif

/* we enter the second part only if struct.h has already been included, because
 * we need gregorio_character */

#ifdef STRUCT_H

#ifndef UNICODE_H_SECOND_PART
#define UNICODE_H_SECOND_PART

gregorio_character *gregorio_build_char_list_from_buf(const char *buf);

#endif

#endif
