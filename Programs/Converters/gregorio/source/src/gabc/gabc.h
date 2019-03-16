/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This header prototypes gabc-format handling data structures and entry points.
 *
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

#ifndef GABC_H
#define GABC_H

#include "struct.h"

/* functions to read gabc */
gregorio_note *gabc_det_notes_from_string(char *str, char *macros[10],
        gregorio_scanner_location *loc, const gregorio_score *score);
void gabc_det_notes_finish(void);
gregorio_element *gabc_det_elements_from_string(char *str, int *current_key,
        char *macros[10], gregorio_scanner_location *loc,
        gregorio_shape *punctum_inclinatum_orientation,
        const gregorio_score *const score);
gregorio_glyph *gabc_det_glyphs_from_notes(gregorio_note *current_note,
        int *current_key, gregorio_shape *punctum_inclinatum_orientation,
        const gregorio_score *score);
void gabc_digest(const void *buf, size_t size);
int gabc_score_determination_lex_destroy(void);
int gabc_notes_determination_lex_destroy(void);

/* see comments on gregorio_add_note_to_a_glyph for meaning of these
 * variables */
typedef enum gabc_determination {
    DET_NO_END,
    DET_END_OF_CURRENT,
    DET_END_OF_PREVIOUS,
    DET_END_OF_BOTH
} gabc_determination;

static __inline void gabc_update_location(gregorio_scanner_location *const loc,
        const char *const bytes, const size_t length)
{
    size_t i;

    /* to be compatible with LilyPond, this algorithm is based on Lilypond's
     * Source_file::get_counts */

    /* possible future enhancement: make the tabstop size configurable */

    loc->first_line = loc->last_line;
    loc->first_column = loc->last_column;
    loc->first_offset = loc->last_offset;

    for (i = 0; i < length; ++i) {
        if (bytes[i] == '\n') {
            ++loc->last_line;
            loc->last_column = 0;
            loc->last_offset = 0;
        } else if (((unsigned char)bytes[i] & 0xc0u) != 0x80u) {
            /* if two highest bits are 1 and 0, it's a continuation byte,
             * so count everything else, which is either a single-byte
             * character or the first byte of a multi-byte sequence */

            if (bytes[i] == '\t') {
                loc->last_column = (loc->last_column / 8 + 1) * 8;
            } else {
                ++loc->last_column;
            }
            ++loc->last_offset;
        }
    }
}

#endif
