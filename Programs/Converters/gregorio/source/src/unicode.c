/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file contains functions providing UTF-8 support.
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

#include "config.h"
#include <stdio.h>
#include <string.h> /* for strlen */
#include <stdlib.h>
#include "struct.h"
#include "unicode.h"
#include "messages.h"
#include "support.h"

/*
 * 
 * This file contains some functions to handle UTF-8 correctly. We chose to
 * embed them instead of relying on locale or glib, for simplification: with
 * these functions we don't rely on anything locale-dependant (such as
 * wchar_t). The reason is that Windows XP (...) is not UTF-8 and thus would
 * need the glib in oder to run, which make dependencies much bigger.
 * 
 */

/* an utf8 version of mbstowcs */

/* expects a buffer of size n + 1 */
static bool gregorio_mbstowcs(grewchar *dest, const char *src, size_t n)
{
    bool success = true;
    unsigned char bytes_to_come;
    grewchar result = 0;
    unsigned char c;
    size_t res = 0; /* number of bytes we've done so far */
    gregorio_not_null(src, gregorio_mbstowcs, return false);
    gregorio_not_null(dest, gregorio_mbstowcs, return false);
    while (success && *src && res < n) {
        c = (unsigned char) (*src);
        if (c < 128) { /* 0100xxxx */
            /* one-byte symbol */
            bytes_to_come = 0;
            result = c;
        } else if (c >= 240) { /* 1111xxxx */
            /* start of a four-byte symbol */
            /* printf("%d\n", c); */
            bytes_to_come = 3;
            result = (result << 3) | (c & 7);
        } else if (c >= 224) { /* 1110xxxx */
            /* start of a three-byte symbol */
            /* printf("%d\n", c); */
            bytes_to_come = 2;
            result = (result << 4) | (c & 15);
        } else if (c >= 192) { /* 1100xxxx */
            /* start of a two-byte symbol */
            /* printf("%d\n", c); */
            bytes_to_come = 1;
            result = (result << 5) | (c & 31);
        } else {
            /* printf("%s %d %d\n", src, res, c); */
            gregorio_message(_("malformed UTF-8 sequence1"),
                    "gregorio_mbstowcs", VERBOSITY_ERROR, 0);
            success = false;
            break;
        }
        while (bytes_to_come > 0) {
            bytes_to_come--;
            src++;
            c = (unsigned char) (*src);
            if (c < 192 && c >= 128) /* 1000xxxx */
            {
                result = (result << 6) | (c & 63);
            } else {
                gregorio_message(_("malformed UTF-8 sequence2"),
                        "gregorio_mbstowcs", VERBOSITY_ERROR, 0);
                success = false;
                break;
            }
        }
        dest[res] = result;

        res++;
        result = 0;
        src++;
    }
    dest[res] = 0;
    return success;
}

/* the value returned by this function must be freed! */
grewchar *gregorio_build_grewchar_string_from_buf(const char *const buf)
{
    size_t len;
    grewchar *gwstring;
    /* this doesn't currently happen, but it's safer to keep this check */
    if (buf == NULL) {
        return NULL; /* LCOV_EXCL_LINE */
    }
    len = strlen(buf); /* to get the length of the syllable in ASCII */
    gwstring = (grewchar *) gregorio_malloc((len + 1) * sizeof(grewchar));
    gregorio_mbstowcs(gwstring, buf, len); /* converting into grewchar */
    /* no need to check the return code; if it failed, the error state would
     * already be in place */
    return gwstring;
}

/* the function to build a gregorio_character list from a buffer. */

gregorio_character *gregorio_build_char_list_from_buf(const char *const buf)
{
    int i = 0;
    grewchar *gwstring;
    gregorio_character *current_character = NULL;
    /* this doesn't currently happen, but it's safer to keep this check */
    if (buf == NULL) {
        return NULL; /* LCOV_EXCL_LINE */
    }
    gwstring = gregorio_build_grewchar_string_from_buf(buf);
    /* we add the corresponding characters in the list of gregorio_characters */
    while (gwstring[i]) {
        gregorio_add_character(&current_character, gwstring[i]);
        i++;
    }
    free(gwstring);
    gregorio_go_to_first_character_c(&current_character);
    return current_character;
}

void gregorio_print_unichar(FILE *f, const grewchar to_print)
{
    if (to_print <= 0x7F) {
        fprintf(f, "%c", (unsigned char) to_print);
        return;
    }
    if (to_print >= 0x80 && to_print <= 0x7FF) {
        fprintf(f, "%c%c", 0xC0 | (to_print >> 6), 0x80 | (to_print & 0x3F));
        return;
    }
    if ((to_print >= 0x800 && to_print <= 0xD7FF) ||
        (to_print >= 0xE000 && to_print <= 0xFFFF)) {
        fprintf(f, "%c%c%c", 0xE0 | (to_print >> 12),
                0x80 | ((to_print >> 6) & 0x3F), 0x80 | (to_print & 0x3F));
        return;
    }
    if (to_print >= 0x10000 && to_print <= 0x10FFFF) {
        fprintf(f, "%c%c%c%c", 0xF0 | (to_print >> 18),
                0x80 | ((to_print >> 12) & 0x3F),
                0x80 | ((to_print >> 6) & 0x3F), 0x80 | (to_print & 0x3F));
    }
}

void gregorio_print_unistring(FILE *f, const grewchar *first_char)
{
    while (*first_char != 0) {
        gregorio_print_unichar(f, *first_char);
        first_char++;
    }
}
