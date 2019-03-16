/*
 * Gregorio is a program that translates gabc files to GregorioTeX.
 * This file provides functions for determining elements from notes.
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

#include "config.h"
#include <stdio.h>
#include "bool.h"
#include "struct.h"
#include "messages.h"

#include "gabc.h"

#define SINGLE_NOTE_GLYPH \
         G_PUNCTUM: \
    case G_VIRGA: \
    case G_BIVIRGA: \
    case G_TRIVIRGA: \
    case G_VIRGA_REVERSA: \
    case G_STROPHA: \
    case G_STROPHA_AUCTA: \
    case G_DISTROPHA: \
    case G_DISTROPHA_AUCTA: \
    case G_TRISTROPHA: \
    case G_TRISTROPHA_AUCTA

#define PUNCTA_INCLINATA_ASCENDENS_GLYPH \
         G_2_PUNCTA_INCLINATA_ASCENDENS: \
    case G_3_PUNCTA_INCLINATA_ASCENDENS: \
    case G_4_PUNCTA_INCLINATA_ASCENDENS: \
    case G_5_PUNCTA_INCLINATA_ASCENDENS

#define PUNCTA_INCLINATA_DESCENDENS_GLYPH \
         G_2_PUNCTA_INCLINATA_DESCENDENS: \
    case G_3_PUNCTA_INCLINATA_DESCENDENS: \
    case G_4_PUNCTA_INCLINATA_DESCENDENS: \
    case G_5_PUNCTA_INCLINATA_DESCENDENS

static __inline signed char glyph_note_ambitus(
        const gregorio_glyph *const current_glyph,
        const gregorio_glyph *const previous_glyph)
{
    return current_glyph->u.notes.first_note->u.note.pitch -
            gregorio_glyph_last_note(previous_glyph)->u.note.pitch;
}

/*
 * 
 * A function that will be called several times: it adds an element to the
 * current element current_element: the current_element in the determination,
 * it will be updated to the element that we will add first_glyph: the
 * first_glyph of the element that we will add current_glyph: the last glyph
 * that will be in the element
 * 
 */

static void close_element(gregorio_element **current_element,
        gregorio_glyph **const first_glyph,
        const gregorio_glyph *const current_glyph)
{
    gregorio_add_element(current_element, *first_glyph);
    if (*first_glyph && (*first_glyph)->previous) {
        (*first_glyph)->previous->next = NULL;
        (*first_glyph)->previous = NULL;
    }
    *first_glyph = current_glyph->next;
}

/*
 * 
 * inline function to automatically do two or three things
 * 
 */
static __inline void cut_before(gregorio_glyph *current_glyph,
                              gregorio_glyph **first_glyph,
                              gregorio_element **current_element)
{
    if (*first_glyph != current_glyph) {
        close_element(current_element, first_glyph, current_glyph);
        /* yes, this is changing value close_element sets for first_glyph */
        *first_glyph = current_glyph;
    }
}

/*
 * 
 * The big function of the file, but rather simple I think.
 * 
 */

static gregorio_element *gabc_det_elements_from_glyphs(
        gregorio_glyph *current_glyph)
{
    /* the last element we have successfully added to the list of elements */
    gregorio_element *current_element = NULL;
    /* the first element, that we will return at the end. We have to consider
     * it because the gregorio_element struct does not have previous_element
     * element. */
    gregorio_element *first_element = NULL;
    /* the first_glyph of the element that we are currently determining */
    gregorio_glyph *first_glyph = current_glyph;
    /* the last real (GRE_GLYPH) that we have processed */
    gregorio_glyph *previous_glyph = NULL;
    /* boolean necessary to determine some cases */
    bool do_not_cut = false;
    bool force_cut = false;

    gregorio_not_null(current_glyph, gabc_det_elements_from_glyphs, return NULL);
    /* first we go to the first glyph in the chained list of glyphs (maybe to
     * suppress ?) */
    gregorio_go_to_first_glyph(&current_glyph);

    while (current_glyph) {
        if (current_glyph->type != GRE_GLYPH) {
            force_cut = false;
            /* we must not cut after a glyph-level space */
            if (current_glyph->type == GRE_SPACE) {
                switch (current_glyph->u.misc.unpitched.info.space) {
                case SP_ZERO_WIDTH:
                case SP_HALF_SPACE:
                case SP_INTERGLYPH_SPACE:
                    if (!current_glyph->next) {
                        close_element(&current_element, &first_glyph, current_glyph);
                    }
                    current_glyph = current_glyph->next;
                    do_not_cut = true;
                    continue;
                default:
                    /* any other space should be handled normally */
                    break;
                }
            } else if (current_glyph->type == GRE_TEXVERB_GLYPH) {
                /* we must not cut after a texverb */
                if (!current_glyph->next) {
                    close_element(&current_element, &first_glyph, current_glyph);
                }
                current_glyph = current_glyph->next;
                do_not_cut = true;
                continue;
            }
            /* clef change or space or end of line */
            cut_before(current_glyph, &first_glyph, &current_element);
            /* if statement to make neumatic cuts not appear in elements, as
             * there is always one between elements, unless the next element
             * is a space */
            if (current_glyph->type != GRE_SPACE
                    || current_glyph->u.misc.unpitched.info.space
                    != SP_NEUMATIC_CUT
                    || (current_glyph->next
                        && current_glyph->next->type == GRE_SPACE)) {
                /* clef change or space other than neumatic cut */
                if (!first_element) {
                    first_element = current_element;
                }
                gregorio_add_misc_element(&current_element, current_glyph->type,
                                          &(current_glyph->u.misc),
                                          current_glyph->texverb);
            }
            first_glyph = current_glyph->next;
            previous_glyph = NULL;
            current_glyph->texverb = 0;
            gregorio_free_one_glyph(&current_glyph);
            continue;
        }

        if (is_fused(current_glyph->u.notes.liquescentia)) {
            do_not_cut = true;
        } else if (force_cut) {
            cut_before(current_glyph, &first_glyph, &current_element);
            previous_glyph = NULL;
        }
        force_cut = false;

        switch (current_glyph->u.notes.glyph_type) {
        case PUNCTA_INCLINATA_ASCENDENS_GLYPH:
        case G_PUNCTUM_INCLINATUM:
        case G_ALTERATION:
            if (!do_not_cut) {
                cut_before(current_glyph, &first_glyph, &current_element);
                do_not_cut = true;
            }
            break;

        case G_STROPHA_AUCTA:
        case G_STROPHA:
            if (current_glyph->u.notes.liquescentia
                    & (L_AUCTUS_ASCENDENS | L_AUCTUS_DESCENDENS)) {
                force_cut = true;
            }
            /* fall through */
        case PUNCTA_INCLINATA_DESCENDENS_GLYPH:
            /* we don't cut before, so we don't do anything else */
            if (do_not_cut) {
                do_not_cut = false;
            }
            break;

        default:
            if (previous_glyph && previous_glyph->type == GRE_GLYPH
                    && !is_tail_liquescentia(
                        previous_glyph->u.notes.liquescentia)) {
                bool break_early = false;
                signed char ambitus;

                switch (previous_glyph->u.notes.glyph_type) {
                case SINGLE_NOTE_GLYPH:
                    /* we determine the last pitch */
                    ambitus = glyph_note_ambitus(current_glyph, previous_glyph);
                    if (ambitus == 0) {
                        do_not_cut = false;
                        break_early = true;
                    }
                    break;

                case PUNCTA_INCLINATA_DESCENDENS_GLYPH:
                case G_PUNCTUM_INCLINATUM:
                    switch (current_glyph->u.notes.glyph_type) {
                    case SINGLE_NOTE_GLYPH:
                        ambitus = glyph_note_ambitus(current_glyph, previous_glyph);
                        if (ambitus > -2 && ambitus < 2) {
                            do_not_cut = false;
                            break_early = true;
                        }
                        break;

                    default:
                        /* do nothing in particular */
                        break;
                    }
                    break;

                case G_PODATUS:
                    if (current_glyph->u.notes.glyph_type == G_VIRGA) {
                        ambitus = glyph_note_ambitus(current_glyph, previous_glyph);
                        if (ambitus >= 0) {
                            do_not_cut = false;
                            break_early = true;
                        }
                    }
                    break;

                default:
                    /* do nothing in particular */
                    break;
                }

                if (break_early) {
                    break;
                }
            }

            if (do_not_cut) {
                do_not_cut = false;
            } else {
                cut_before(current_glyph, &first_glyph, &current_element);
            }
        }

        if (gregorio_glyph_last_note(current_glyph)->signs
                & (_PUNCTUM_MORA | _AUCTUM_DUPLEX)) {
            force_cut = true;
        }

        /* we must determine the first element, that we will return */
        if (!first_element && current_element) {
            first_element = current_element;
        }
        if (!current_glyph->next) {
            close_element(&current_element, &first_glyph, current_glyph);
        }

        previous_glyph = current_glyph;
        current_glyph = current_glyph->next;
    } /* end of while */

    /*
     * we must determine the first element, that we will return 
     */
    if (!first_element && current_element) {
        first_element = current_element;
    }
    return first_element;
}

/*
 * 
 * Two "hat" functions, they permit to have a good API. They amost don't do
 * anything except calling the det_elements_from_glyphs.
 * 
 * All those functions change the current_key, according to the possible new
 * values (with key changes)
 * 
 */

static gregorio_element *gabc_det_elements_from_notes(
        gregorio_note *current_note, int *current_key,
        gregorio_shape *const punctum_inclinatum_orientation,
        const gregorio_score *const score)
{
    gregorio_element *final = NULL;
    gregorio_glyph *tmp = gabc_det_glyphs_from_notes(current_note, current_key,
            punctum_inclinatum_orientation, score);
    final = gabc_det_elements_from_glyphs(tmp);
    return final;
}

gregorio_element *gabc_det_elements_from_string(char *const str,
        int *const current_key, char *macros[10],
        gregorio_scanner_location *const loc,
        gregorio_shape *const punctum_inclinatum_orientation,
        const gregorio_score *const score)
{
    gregorio_element *final;
    gregorio_note *tmp;
    tmp = gabc_det_notes_from_string(str, macros, loc, score);
    final = gabc_det_elements_from_notes(tmp, current_key,
            punctum_inclinatum_orientation, score);
    return final;
}

