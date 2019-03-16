/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This header prototypes the lyric handling data structures and entry points.
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

#ifndef CHARACTERS_H
#define CHARACTERS_H

#include "bool.h"
#include "struct.h"

/*
 * 
 * The three next defines are the possible values of center_is_determined.
 * 
 * CENTER_NOT_DETERMINED means that the plugin has encountered no { nor } (in
 * the gabc syntax). CENTER_HALF_DETERMINED means that the plugin has
 * encountered a { but no }, and we will try to determine a middle starting
 * after the {. CENTER_FULLY_DETERMINED means that lex has encountered a { and
 * a }, so we won't determine the middle, it is considered done.
 * CENTER_DETERMINING_MIDDLE is used internally in the big function to know
 * where we are in the middle determination.
 * 
 */

typedef enum gregorio_center_determination {
    CENTER_NOT_DETERMINED = 0,
    CENTER_HALF_DETERMINED,
    CENTER_FULLY_DETERMINED,
    CENTER_DETERMINING_MIDDLE
} gregorio_center_determination;

/* this is a temporary structure that will be used for style determination */

typedef struct det_style {
    grestyle_style style;
    struct det_style *previous_style;
    struct det_style *next_style;
} det_style;

typedef enum gregorio_write_text_phase {
    WTP_NORMAL,
    WTP_FIRST_SYLLABLE
} gregorio_write_text_phase;

void gregorio_write_text(gregorio_write_text_phase phase,
        const gregorio_character *current_character, FILE *f,
        void (*printverb) (FILE *, const grewchar *),
        void (*printchar) (FILE *, grewchar),
        void (*begin) (FILE *, grestyle_style),
        void (*end) (FILE *, grestyle_style),
        void (*printspchar) (FILE *, const grewchar *));

void gregorio_write_first_letter_alignment_text(gregorio_write_text_phase phase,
        const gregorio_character *current_character, FILE *f,
        void (*printverb) (FILE *, const grewchar *),
        void (*printchar) (FILE *, grewchar),
        void (*begin) (FILE *, grestyle_style),
        void (*end) (FILE *, grestyle_style),
        void (*printspchar) (FILE *, const grewchar *));

void gregorio_set_centering_language(char *language);

void gregorio_rebuild_characters(gregorio_character **param_character,
        gregorio_center_determination center_is_determined, bool skip_initial);

void gregorio_rebuild_first_syllable(gregorio_character **param_character,
        bool separate_initial);

void gregorio_set_first_word(gregorio_character **character);

#endif
