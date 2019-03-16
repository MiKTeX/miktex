/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file implements the score parser.
 *
 * Gregorio score determination from gabc utilities.
 * Copyright (C) 2016-2019 The Gregorio Project (see CONTRIBUTORS.md)
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
#include <assert.h>
#include "bool.h"
#include "struct.h"
#include "struct_iter.h"
#include "gabc.h"
#include "gabc-score-determination.h"
#include "messages.h"
#include "support.h"

void gabc_suppress_extra_custos_at_linebreak(gregorio_score *score)
{
    gregorio_syllable *syllable;
    gregorio_element **custos = NULL;

    for (syllable = score->first_syllable; syllable;
            syllable = syllable->next_syllable) {
        gregorio_element **element;
        for (element = syllable->elements; element && *element;
                element = &((*element)->next)) {
            switch ((*element)->type) {
            case GRE_CUSTOS:
                if (!((*element)->u.misc.pitched.force_pitch)) {
                    /* save the encountered non-forced custos */
                    custos = element;
                } else {
                    /* forget the (previous) custos */
                    custos = NULL;
                }
                break;
            case GRE_CLEF:
            case GRE_BAR:
                /* remember the custos if only these appear before linebreak */
                break;
            case GRE_END_OF_LINE:
                if (custos) {
                    /* suppress the custos when linebreak follows */
                    gregorio_free_one_element(custos);
                }
                /* fall through */
            default:
                /* forget the custos */
                custos = NULL;
                break;
            }
        }
    }
}

void gabc_fix_custos_pitches(gregorio_score *score_to_check)
{
    gregorio_syllable *current_syllable;
    gregorio_element *current_element;
    gregorio_element *custos_element;
    int newkey;
    int current_key;

    if (!score_to_check || !score_to_check->first_syllable
            || !score_to_check->first_voice_info) {
        return;
    }

    current_key = gregorio_calculate_new_key(
            score_to_check->first_voice_info->initial_clef);
    for (current_syllable = score_to_check->first_syllable; current_syllable;
            current_syllable = current_syllable->next_syllable) {
        for (current_element = (current_syllable->elements)[0]; current_element;
                current_element = current_element->next) {
            if (current_element->type == GRE_CLEF) {
                newkey = gregorio_calculate_new_key(
                        current_element->u.misc.clef);
                current_element->u.misc.clef.pitch_difference =
                        (signed char) newkey - (signed char) current_key;
                current_key = newkey;
            }
        }
    }

    custos_element = NULL;
    for (current_syllable = score_to_check->first_syllable; current_syllable;
            current_syllable = current_syllable->next_syllable) {
        for (current_element = (current_syllable->elements)[0]; current_element;
                current_element = current_element->next) {
            switch (current_element->type) {
            case GRE_CUSTOS:
                if (current_element->u.misc.pitched.force_pitch) {
                    /* forget about the preceding custos if a forced one is
                     * encountered */
                    custos_element = NULL;
                } else {
                    /* the pitch is not forced, so it may need to be adjusted */
                    custos_element = current_element;
                    custos_element->u.misc.pitched.pitch =
                            gregorio_determine_next_pitch(current_syllable,
                                    current_element, NULL, NULL);
                }
                break;

            case GRE_ELEMENT:
                /* if it's an element, forget any preceding custos */
                custos_element = NULL;
                break;

            case GRE_CLEF:
                if (custos_element) {
                    /* adjust the preceding custos for the clef */
                    custos_element->u.misc.pitched.pitch =
                            gregorio_adjust_pitch_into_staff(score_to_check,
                            custos_element->u.misc.pitched.pitch
                            - current_element->u.misc.clef.pitch_difference);
                }
                break;

            default:
                /* to prevent the warning */
                break;
            }
        }
    }
}

/*
 * A function that checks the score integrity.
 */

bool gabc_check_score_integrity(gregorio_score *score_to_check)
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

bool gabc_check_infos_integrity(gregorio_score *score_to_check)
{
    if (!score_to_check->name) {
        gregorio_message(_("no name specified, put `name:...;' at the "
                "beginning of the file, can be dangerous with some output "
                "formats"), "det_score", VERBOSITY_WARNING, 0);
    }
    return true;
}

static void set_oriscus_descending(const gregorio_note_iter_position *const p,
        void *const ignored __attribute__((unused)))
{
    switch(p->note->u.note.shape) {
    case S_ORISCUS_UNDETERMINED:
        p->note->u.note.shape = S_ORISCUS_DESCENDENS;
        break;
    case S_ORISCUS_SCAPUS_UNDETERMINED:
        p->note->u.note.shape = S_ORISCUS_SCAPUS_DESCENDENS;
        gregorio_assert_only(p->glyph->u.notes.glyph_type
                != G_PES_DESCENDENS_ORISCUS, set_oriscus_descending,
                "glyph type should not be G_PES_DESCENDENS_ORISCUS");
        if (p->glyph->u.notes.glyph_type == G_PES_ASCENDENS_ORISCUS) {
            p->glyph->u.notes.glyph_type = G_PES_DESCENDENS_ORISCUS;
        }
        break;
    default:
        break;
    }
}

static void set_oriscus_ascending(const gregorio_note_iter_position *const p,
        void *const ignored __attribute__((unused)))
{
    switch(p->note->u.note.shape) {
    case S_ORISCUS_UNDETERMINED:
        p->note->u.note.shape = S_ORISCUS_ASCENDENS;
        break;
    case S_ORISCUS_SCAPUS_UNDETERMINED:
        p->note->u.note.shape = S_ORISCUS_SCAPUS_ASCENDENS;
        gregorio_assert_only(p->glyph->u.notes.glyph_type
                != G_PES_DESCENDENS_ORISCUS, set_oriscus_ascending,
                "glyph type should not be G_PES_DESCENDENS_ORISCUS");
        break;
    default:
        break;
    }
}

/* data must be (gregorio_note_iter_position *) */
static void oriscus_orientation_visit(
        const gregorio_note_iter_position *const p, void *const data)
{
    gregorio_note *const note = p->note;
    gregorio_note_iter_position *const oriscus =
        (gregorio_note_iter_position *const)data;

    if (oriscus->note && note->u.note.pitch != oriscus->note->u.note.pitch) {
        if (note->u.note.pitch <= oriscus->note->u.note.pitch) {
            /* descending (or undetermined oriscus in unison) */
            gregorio_from_note_to_note(oriscus, p, false,
                    set_oriscus_descending, NULL, GRESTRUCT_NONE, NULL);
        } else {
            /* ascending */
            gregorio_from_note_to_note(oriscus, p, false,
                    set_oriscus_ascending, NULL, GRESTRUCT_NONE, NULL);
        }
        oriscus->syllable = NULL,
        oriscus->element = NULL,
        oriscus->note = NULL;
        oriscus->glyph = NULL;
    }

    if (!oriscus->note) {
        switch (note->u.note.shape) {
        case S_ORISCUS_UNDETERMINED:
        case S_ORISCUS_SCAPUS_UNDETERMINED:
            *oriscus = *p;
            break;

        default:
            break;
        }
    }
}

void gabc_determine_oriscus_orientation(const gregorio_score *const score)
{
    gregorio_note_iter_position oriscus = {
        /* .syllable = */ NULL,
        /* .element = */ NULL,
        /* .glyph = */ NULL,
        /* .note = */ NULL
    };

    gregorio_for_each_note(score, oriscus_orientation_visit, NULL,
            GRESTRUCT_NONE, &oriscus);

    if (oriscus.note) {
        gregorio_from_note_to_note(&oriscus, NULL, true,
                set_oriscus_descending, NULL, GRESTRUCT_NONE, NULL);
    }
}

typedef struct {
    gregorio_note_iter_position first, previous;
    gregorio_shape orientation;
    int running;
    int count;
    bool unison;
} punctum_inclinatum_vars;

/* data must be (gregorio_shape *) */
static void set_shape(const gregorio_note_iter_position *const p,
        void *const data)
{
    p->note->u.note.shape = *((gregorio_shape *)data);
}

static __inline void set_punctum_inclinatum_orientation(
        punctum_inclinatum_vars *const v)
{
    if (v->orientation == S_UNDETERMINED) {
        if (v->unison && (v->count > 1 || (v->count == 1 && v->running == 0))) {
            v->orientation = S_PUNCTUM_INCLINATUM_STANS;
        } else if (v->running > 0) {
            v->orientation = S_PUNCTUM_INCLINATUM_ASCENDENS;
        } else {
            v->orientation = S_PUNCTUM_INCLINATUM_DESCENDENS;
        }
    }
}

static __inline void finalize_punctum_inclinatum_orientation(
        punctum_inclinatum_vars *const v)
{
    if (v->first.note) {
        set_punctum_inclinatum_orientation(v);
        gregorio_assert_only(v->orientation != S_UNDETERMINED,
                punctum_inclinatum_orientation_end_item,
                "orientation should not be S_UNDETERMINED");
        gregorio_from_note_to_note(&v->first, &v->previous, true, set_shape,
                NULL, GRESTRUCT_NONE, &v->orientation);
    }

    v->first.syllable = NULL;
    v->first.element = NULL;
    v->first.glyph = NULL;
    v->first.note = NULL;
    v->unison = true;
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
                    if (v->count > 0) {
                        v->unison = false;
                    }
                } else if (v->previous.note->u.note.pitch
                        > p->note->u.note.pitch) {
                    -- v->running;
                    if (v->count > 0) {
                        v->unison = false;
                    }
                }
            }
            if (!v->first.note) {
                v->first = *p;
            }
        }
        ++ v->count;
    } else { /* non-inclinatum or determined inclinatum */
        bool is_punctum_inclinatum;

        /* shape can't be S_PUNCTUM_INCLINATUM_UNDETERMINED here */
        switch (shape) {
        case S_PUNCTUM_INCLINATUM_ASCENDENS:
            v->orientation = S_PUNCTUM_INCLINATUM_ASCENDENS;
            is_punctum_inclinatum = true;
            break;
        case S_PUNCTUM_INCLINATUM_STANS:
            v->orientation = S_PUNCTUM_INCLINATUM_STANS;
            is_punctum_inclinatum = true;
            break;
        case S_PUNCTUM_INCLINATUM_DEMINUTUS:
        case S_PUNCTUM_INCLINATUM_AUCTUS:
            v->unison = false;
            /* fall through */
        case S_PUNCTUM_INCLINATUM_DESCENDENS:
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
            }
            finalize_punctum_inclinatum_orientation(v);
        }

        if (is_punctum_inclinatum) {
            ++ v->count;
            /* and leave orientation alone */
        } else {
            v->running = 0;
            v->count = 0;
            v->orientation = S_UNDETERMINED;
        }
    }

    v->previous = *p;
}

/* data must be (punctum_inclinatum_vars *) */
static void punctum_inclinatum_orientation_end_item(
        const gregorio_note_iter_position *const p __attribute__((__unused__)),
        const gregorio_note_iter_item_type item_type,
        void *const data)
{
    punctum_inclinatum_vars *const v = (punctum_inclinatum_vars *)data;

    gregorio_assert_only(item_type == GRESTRUCT_SYLLABLE,
            punctum_inclinatum_orientation_end_item,
            "item type should be GRESTRUCT_SYLLABLE");

    finalize_punctum_inclinatum_orientation(v);
    v->count = 0;
}

void gabc_determine_punctum_inclinatum_orientation(
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
        /* .count = */ 0,
        /* .unison = */ true,
    };

    gregorio_for_each_note(score, punctum_inclinatum_orientation_visit,
            punctum_inclinatum_orientation_end_item, GRESTRUCT_SYLLABLE, &v);

    if (v.first.note) {
        set_punctum_inclinatum_orientation(&v);
        gregorio_from_note_to_note(&v.first, &v.previous, true, set_shape, NULL,
                GRESTRUCT_NONE, &v.orientation);
    }
}

typedef struct note_stack {
    gregorio_note *note;
    struct note_stack *prev;
} note_stack;

static void note_stack_push(note_stack **const stack, gregorio_note *note) {
    note_stack *item = gregorio_malloc(sizeof(note_stack));
    item->note = note;
    item->prev = *stack;
    *stack = item;
}

static gregorio_note *note_stack_pop(note_stack **const stack) {
    note_stack *item = *stack;
    if (item) {
        gregorio_note *note = item->note;
        note_stack *prev = item->prev;
        free(item);
        *stack = prev;
        return note;
    }
    return NULL;
}

static void note_stack_clear(note_stack **const stack) {
    note_stack *item = *stack;
    while (item) {
        note_stack *prev = item->prev;
        free(item);
        item = prev;
    }
    *stack = NULL;
}

typedef struct {
    note_stack *high, *low;
    gregorio_note *prev_note;
    signed char high_ledger_line_pitch;
    bool running_high, running_low;
} ledger_line_vars;

static __inline void clear_ledger_line_vars(ledger_line_vars *const v) {
    note_stack_clear(&v->high);
    note_stack_clear(&v->low);
    v->prev_note = NULL;
    v->running_high = false;
    v->running_low = false;
}

static __inline void adjust_ledger(const gregorio_note_iter_position *const p,
        const gregorio_ledger_specificity specificity, bool ledger_line,
        note_stack **const stack, bool *const running, gregorio_note *prev_note,
        const signed char high_ledger_line_pitch,
        bool (*extend_ledger)(gregorio_note *, const gregorio_note *,
            const gregorio_note *, signed char))
{
    if (specificity & LEDGER_DRAWN) {
        if (ledger_line) {
            gregorio_note *after = p->note;
            gregorio_note *note;
            /* process from this ledger backwards */
            while ((note = note_stack_pop(stack))) {
                if (!extend_ledger(note, NULL, after, high_ledger_line_pitch)) {
                    /* ledger has ended */
                    break;
                }
                after = note;
            }
            *running = true;
        } else {
            *running = false;
        }
        note_stack_clear(stack);
    } else {
        if (*running) {
            if (!extend_ledger(p->note, prev_note, NULL,
                    high_ledger_line_pitch)) {
                /* ledger has ended */
                note_stack_push(stack, p->note);
                *running = false;
            }
            /* else stack should be empty, keep it that way */
        } else {
            note_stack_push(stack, p->note);
        }
    }
}

static bool extend_high_ledger(gregorio_note *const note,
        const gregorio_note *const note_before,
        const gregorio_note *const note_after,
        const signed char high_ledger_line_pitch)
{
    bool extend = false;

    if (note_before) {
        extend = note_before->u.note.pitch < note->u.note.pitch
            || (note_before->u.note.pitch > high_ledger_line_pitch
                && note->u.note.pitch < high_ledger_line_pitch);
    } else if (note_after) {
        extend = note_after->u.note.pitch < note->u.note.pitch
            || (note_after->u.note.pitch > high_ledger_line_pitch
                && note->u.note.pitch < high_ledger_line_pitch);
    }

    if (extend) {
        note->high_ledger_line = true;
        note->high_ledger_specificity = LEDGER_DRAWN;
    }
    return extend;
}

static bool extend_low_ledger(gregorio_note *const note,
        const gregorio_note *const note_before,
        const gregorio_note *const note_after,
        const signed char high_ledger_line_pitch __attribute__((__unused__)))
{
    bool extend = false;

    if (note_before) {
        extend = note_before->u.note.pitch < note->u.note.pitch
            || (note_before->u.note.pitch > LOW_LEDGER_LINE_PITCH
                && note->u.note.pitch < LOW_LEDGER_LINE_PITCH);
    } else if (note_after) {
        extend = note_after->u.note.pitch < note->u.note.pitch
            || (note_after->u.note.pitch > LOW_LEDGER_LINE_PITCH
                && note->u.note.pitch < LOW_LEDGER_LINE_PITCH);
    }

    if (extend) {
        note->low_ledger_line = true;
        note->low_ledger_specificity = LEDGER_DRAWN;
    }
    return extend;
}

/* data must be (ledger_line_vars *) */
static void ledger_line_visit(const gregorio_note_iter_position *const p,
        void *const data)
{
    ledger_line_vars *const v = (ledger_line_vars *)data;

    adjust_ledger(p, p->note->high_ledger_specificity, p->note->high_ledger_line,
            &v->high, &v->running_high, v->prev_note, v->high_ledger_line_pitch,
            &extend_high_ledger);
    adjust_ledger(p, p->note->low_ledger_specificity, p->note->low_ledger_line,
            &v->low, &v->running_low, v->prev_note, v->high_ledger_line_pitch,
            &extend_low_ledger);

    v->prev_note = p->note;
}

/* data must be (ledger_line_vars *) */
static void ledger_line_end_item(
        const gregorio_note_iter_position *const p __attribute__((__unused__)),
        const gregorio_note_iter_item_type item_type, void *const data)
{
    if (item_type == GRESTRUCT_ELEMENT) {
        clear_ledger_line_vars((ledger_line_vars *)data);
    }
}

void gabc_determine_ledger_lines(const gregorio_score *const score)
{
    ledger_line_vars v;
    memset(&v, 0, sizeof v);
    v.high_ledger_line_pitch = score->high_ledger_line_pitch;

    gregorio_for_each_note(score, ledger_line_visit, ledger_line_end_item,
            GRESTRUCT_ELEMENT, &v);

    /* stacks should be cleared by ledger_line_end_item */
}
