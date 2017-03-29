/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This header defines the Gregorio data structures and functions.
 *
 * Copyright (C) 2016 The Gregorio Project (see CONTRIBUTORS.md)
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

/**
 * @file
 * The file starts by the definition of all the structures used in
 * gregorio. As it is certainly the most important file for
 * understanding, read it carefully.
 */

#ifndef STRUCT_ITER_H
#define STRUCT_ITER_H

#include "struct.h"

typedef struct {
    gregorio_syllable *syllable;
    gregorio_element *element;
    gregorio_glyph *glyph;
    gregorio_note *note;
} gregorio_note_iter_position;

static __inline void gregorio_from_note_to_note(
        const gregorio_note_iter_position *const start,
        const gregorio_note_iter_position *const end,
        void (*const visit)(const gregorio_note_iter_position *, void *),
        void *data)
{
    gregorio_note_iter_position p = *start;

    while (p.syllable) {
        if (!p.element) {
            p.element = p.syllable->elements[0];
        }
        while (p.element) {
            if (p.element->type == GRE_ELEMENT) {
                if (!p.glyph) {
                    p.glyph = p.element->u.first_glyph;
                }
                while (p.glyph) {
                    if (p.glyph->type == GRE_GLYPH) {
                        if (!p.note) {
                            p.note = p.glyph->u.notes.first_note;
                        }
                        while (p.note) {
                            if (p.note->type == GRE_NOTE) {
                                visit(&p, data);
                            }

                            if (end && p.note == end->note) {
                                return;
                            }

                            p.note = p.note->next;
                        } /* note */
                    }
                    p.glyph = p.glyph->next;
                } /* glyph */
            }
            p.element = p.element->next;
        } /* element */
        p.syllable = p.syllable->next_syllable;
    } /* syllable */
}

static __inline void gregorio_for_each_note(const gregorio_score *score,
        void (*const visit)(const gregorio_note_iter_position *, void *),
        void *data)
{
    gregorio_note_iter_position p = {
        /* .syllable = */ NULL,
        /* .element = */ NULL,
        /* .glyph = */ NULL,
        /* .note = */ NULL
    };

    p.syllable = score->first_syllable;

    gregorio_from_note_to_note(&p, NULL, visit, data);
}

#endif
