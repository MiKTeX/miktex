/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file implements the score parser.
 *
 * Gregorio score determination from gabc utilities.
 * Copyright (C) 2016 The Gregorio Project (see CONTRIBUTORS.md)
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
#include <assert.h>
#include "bool.h"
#include "struct.h"
#include "struct_iter.h"
#include "gabc.h"
#include "gabc-score-determination.h"
#include "messages.h"

void fix_custos(gregorio_score *score_to_check)
{
    gregorio_syllable *current_syllable;
    gregorio_element *current_element;
    gregorio_element *custo_element;
    char pitch = 0;
    char pitch_difference = 0;
    int newkey;
    int current_key;
    if (!score_to_check || !score_to_check->first_syllable
            || !score_to_check->first_voice_info) {
        return;
    }
    current_key = gregorio_calculate_new_key(
            score_to_check->first_voice_info->initial_clef);
    current_syllable = score_to_check->first_syllable;
    while (current_syllable) {
        current_element = (current_syllable->elements)[0];
        while (current_element) {
            if (current_element->type == GRE_CUSTOS) {
                custo_element = current_element;
                pitch = custo_element->u.misc.pitched.pitch;
                /* we look for the key */
                while (current_element) {
                    if (current_element->type == GRE_CLEF) {
                        pitch = gregorio_determine_next_pitch(current_syllable,
                                current_element, NULL, NULL);
                        newkey = gregorio_calculate_new_key(
                                current_element->u.misc.clef);
                        pitch_difference = (char) newkey - (char) current_key;
                        pitch -= pitch_difference;
                        current_key = newkey;
                    }
                    if (!custo_element->u.misc.pitched.force_pitch) {
                        while (pitch < LOWEST_PITCH) {
                            pitch += 7;
                        }
                        while (pitch > score_to_check->highest_pitch) {
                            pitch -= 7;
                        }
                        custo_element->u.misc.pitched.pitch = pitch;
                    }
                    assert(custo_element->u.misc.pitched.pitch >= LOWEST_PITCH 
                            && custo_element->u.misc.pitched.pitch
                            <= score_to_check->highest_pitch);
                    current_element = current_element->next;
                }
            }
            if (current_element) {
                if (current_element->type == GRE_CLEF) {
                    current_key = gregorio_calculate_new_key(
                            current_element->u.misc.clef);
                }
                current_element = current_element->next;
            }
        }
        current_syllable = current_syllable->next_syllable;
    }
}

/*
 * A function that checks the score integrity.
 */

bool check_score_integrity(gregorio_score *score_to_check)
{
    bool good = true;

    gregorio_assert(score_to_check, check_score_integrity, "score is NULL",
            return false);

    if (score_to_check->first_syllable
            && score_to_check->first_syllable->elements
            && *(score_to_check->first_syllable->elements)) {
        gregorio_character *ch;
        if ((score_to_check->first_syllable->elements)[0]->type
                == GRE_END_OF_LINE) {
            gregorio_message(
                    "line break is not supported on the first syllable",
                    "check_score_integrity", VERBOSITY_ERROR, 0);
            good = false;
        }
        if (gregorio_get_clef_change(score_to_check->first_syllable)) {
            gregorio_message(
                    "clef change is not supported on the first syllable",
                    "check_score_integrity", VERBOSITY_ERROR, 0);
            good = false;
        }
        /* check first syllable for elision at the beginning */
        for (ch = score_to_check->first_syllable->text; ch;
                ch = ch->next_character) {
            if (ch->is_character) {
                break;
            } else if (ch->cos.s.style == ST_VERBATIM
                    || ch->cos.s.style == ST_SPECIAL_CHAR) {
                break;
            } else if (ch->cos.s.style == ST_ELISION) {
                gregorio_message(
                        _("score initial may not be in an elision"),
                        "check_score_integrity", VERBOSITY_ERROR, 0);
                break;
            }
        }
    }

    return good;
}

/*
 * Another function to be improved: this one checks the validity of the voice_infos.
 */

bool check_infos_integrity(gregorio_score *score_to_check)
{
    if (!score_to_check->name) {
        gregorio_message(_("no name specified, put `name:...;' at the "
                "beginning of the file, can be dangerous with some output "
                "formats"), "det_score", VERBOSITY_WARNING, 0);
    }
    return true;
}

/* data must be (gregorio_note **) */
static void oriscus_orientation_visit(
        const gregorio_note_iter_position *const p, void *const data)
{
    gregorio_note *const note = p->note;
    gregorio_note **const oriscus_ptr = (gregorio_note **const)data;
    gregorio_note *const oriscus = *oriscus_ptr;
    /* making oriscus const ensures we don't attempt to change *oriscus_ptr
     * via oriscus */

    if (oriscus) {
        if (note->u.note.pitch <= oriscus->u.note.pitch) {
            /* descending or unison */
            switch(oriscus->u.note.shape) {
            case S_ORISCUS_UNDETERMINED:
                oriscus->u.note.shape = S_ORISCUS_DESCENDENS;
                break;
            case S_ORISCUS_SCAPUS_UNDETERMINED:
                oriscus->u.note.shape = S_ORISCUS_SCAPUS_DESCENDENS;
                break;
            case S_ORISCUS_CAVUM_UNDETERMINED:
                oriscus->u.note.shape = S_ORISCUS_CAVUM_DESCENDENS;
                break;
            default:
                /* not reachable unless there's a
                 * programming error */
                /* LCOV_EXCL_START */
                gregorio_fail(oriscus_orientation_visit, "bad_shape");
                break;
                /* LCOV_EXCL_STOP */
            }
        } else { /* ascending */
            switch(oriscus->u.note.shape) {
            case S_ORISCUS_UNDETERMINED:
                oriscus->u.note.shape = S_ORISCUS_ASCENDENS;
                break;
            case S_ORISCUS_SCAPUS_UNDETERMINED:
                oriscus->u.note.shape = S_ORISCUS_SCAPUS_ASCENDENS;
                break;
            case S_ORISCUS_CAVUM_UNDETERMINED:
                oriscus->u.note.shape = S_ORISCUS_CAVUM_ASCENDENS;
                break;
            default:
                /* not reachable unless there's a
                 * programming error */
                /* LCOV_EXCL_START */
                gregorio_fail(oriscus_orientation_visit, "bad_shape");
                break;
                /* LCOV_EXCL_STOP */
            }
        }
        *oriscus_ptr = NULL;
    }

    switch (note->u.note.shape) {
    case S_ORISCUS_UNDETERMINED:
    case S_ORISCUS_SCAPUS_UNDETERMINED:
    case S_ORISCUS_CAVUM_UNDETERMINED:
        *oriscus_ptr = note;
        break;

    default:
        break;
    }
}

void determine_oriscus_orientation(const gregorio_score *const score)
{
    gregorio_note *oriscus = NULL;

    gregorio_for_each_note(score, oriscus_orientation_visit, &oriscus);

    if (oriscus) {
        /* oriscus at the end of the score */
        switch(oriscus->u.note.shape) {
        case S_ORISCUS_UNDETERMINED:
            oriscus->u.note.shape = S_ORISCUS_DESCENDENS;
            break;
        case S_ORISCUS_SCAPUS_UNDETERMINED:
            oriscus->u.note.shape = S_ORISCUS_SCAPUS_DESCENDENS;
            break;
        case S_ORISCUS_CAVUM_UNDETERMINED:
            oriscus->u.note.shape = S_ORISCUS_CAVUM_DESCENDENS;
            break;
        default:
            /* not reachable unless there's a programming error */
            /* LCOV_EXCL_START */
            gregorio_fail(determine_oriscus_orientation, "bad_shape");
            break;
            /* LCOV_EXCL_STOP */
        }
    }
}

typedef struct {
    gregorio_note_iter_position first, previous;
    gregorio_shape orientation;
    int running;
} punctum_inclinatum_vars;

/* data must be (gregorio_shape *) */
static void set_shape(const gregorio_note_iter_position *const p,
        void *const data)
{
    p->note->u.note.shape = *((gregorio_shape *)data);
}

/* data must be (punctum_inclinatum_vars *) */
static void punctum_inclinatum_orientation_visit(
        const gregorio_note_iter_position *const p, void *const data)
{
    const gregorio_shape shape = p->note->u.note.shape;
    punctum_inclinatum_vars *const v = (punctum_inclinatum_vars *)data;
    if (shape == S_PUNCTUM_INCLINATUM_UNDETERMINED) {
        if (v->orientation) {
            p->note->u.note.shape = v->orientation;
        } else {
            /* any cases not covered here will not change running */
            if (v->previous.note
                    && (v->first.note || v->previous.syllable == p->syllable)) {
                if (v->previous.note->u.note.pitch
                        < p->note->u.note.pitch) {
                    ++ v->running;
                } else if (v->previous.note->u.note.pitch
                        > p->note->u.note.pitch) {
                    -- v->running;
                }
            }
            if (!v->first.note) {
                v->first = *p;
            }
        }
    } else { /* non-inclinatum or determined inclinatum */
        bool is_punctum_inclinatum;

        /* shape can't be S_PUNCTUM_INCLINATUM_UNDETERMINED here */
        switch (shape) {
        case S_PUNCTUM_INCLINATUM_ASCENDENS:
            v->orientation = S_PUNCTUM_INCLINATUM_ASCENDENS;
            is_punctum_inclinatum = true;
            break;
        case S_PUNCTUM_INCLINATUM_DESCENDENS:
        case S_PUNCTUM_INCLINATUM_DEMINUTUS:
        case S_PUNCTUM_INCLINATUM_AUCTUS:
        case S_PUNCTUM_CAVUM_INCLINATUM:
        case S_PUNCTUM_CAVUM_INCLINATUM_AUCTUS:
            v->orientation = S_PUNCTUM_INCLINATUM_DESCENDENS;
            is_punctum_inclinatum = true;
            break;
        default:
            is_punctum_inclinatum = false;
            break;
        }

        if (v->first.note) {
            if (!is_punctum_inclinatum) {
                /* if v->first.note is not null,
                 * then v->previous.note is not null */
                if (v->previous.note->u.note.pitch
                        < p->note->u.note.pitch
                        && v->previous.syllable == p->syllable) {
                    ++ v->running;
                } else if (v->previous.note->u.note.pitch
                        > p->note->u.note.pitch) {
                    -- v->running;
                }
                v->orientation = (v->running > 0)
                    ? S_PUNCTUM_INCLINATUM_ASCENDENS
                    : S_PUNCTUM_INCLINATUM_DESCENDENS;
            }
            gregorio_from_note_to_note(&v->first, &v->previous, set_shape,
                    &v->orientation);
            v->first.syllable = NULL;
            v->first.element = NULL;
            v->first.glyph = NULL;
            v->first.note = NULL;
            v->running = 0;
        }

        if (!is_punctum_inclinatum) {
            v->orientation = S_UNDETERMINED;
        } /* otherwise, leave orientation alone */
    }

    v->previous = *p;
}

void determine_punctum_inclinatum_orientation(
        const gregorio_score *const score)
{
    punctum_inclinatum_vars v = {
        /* .first = */ {
            /* .syllable = */ NULL,
            /* .element = */ NULL,
            /* .glyph = */ NULL,
            /* .note = */ NULL
        },
        /* .previous = */ {
            /* .syllable = */ NULL,
            /* .element = */ NULL,
            /* .glyph = */ NULL,
            /* .note = */ NULL
        },
        /* .orientation = */ S_UNDETERMINED, /* because it's 0 */
        /* .running = */ 0,
    };

    gregorio_for_each_note(score, punctum_inclinatum_orientation_visit, &v);
    
    if (v.first.note) {
        v.orientation = (v.running > 0)
            ? S_PUNCTUM_INCLINATUM_ASCENDENS
            : S_PUNCTUM_INCLINATUM_DESCENDENS;
        gregorio_from_note_to_note(&v.first, &v.previous, set_shape,
                &v.orientation);
    }
}
