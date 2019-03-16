/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file implements the Gregorio data structures.
 *
 * Copyright (C) 2006-2019 The Gregorio Project (see CONTRIBUTORS.md)
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
 * @brief This file contains a set of function to manipulate the gregorio
 * structure.
 *
 * It starts by simple add/delete functions for almost all
 * structs, and ends with more complex functions for manipulating
 * horizontal episema, keys, etc.
 *
 * The first functions are not commented, but they always act like
 * this : we give them a pointer to the pointer to the current element
 * (by element I mean a struct which can be gregorio_note,
 * gregorio_element, etc.), they add an element and the update the
 * pointer to the element so that it points to the new element (may
 * seem a bit strange).
 *
 * All the delete functions are recursive and free all memory.
 *
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include "struct.h"
#include "unicode.h"
#include "messages.h"
#include "support.h"
#include "characters.h"
#include "support.h"

unsigned short tex_position_id = 0;

gregorio_clef_info gregorio_default_clef = {
    /*.line =*/ 3,
    /*.secondary_line =*/ 0,
    /*.pitch_difference =*/ 0,
    /*.clef =*/ CLEF_C,
    /*.flatted =*/ false,
    /*.secondary_clef =*/ CLEF_C, /* not used since secondary_line is 0 */
    /*.secondary_flatted =*/ false,
};

static size_t hepisema_adjustments_capacity = 0;
static unsigned short hepisema_adjustments_last = 0;
static gregorio_hepisema_adjustment *hepisema_adjustments = NULL;

static size_t texverbs_capacity = 0;
static unsigned short texverbs_last = 0;
static char **texverbs = NULL;

void gregorio_struct_init(void)
{
    hepisema_adjustments_capacity = 8;
    hepisema_adjustments = gregorio_grow_buffer(NULL,
            &hepisema_adjustments_capacity, gregorio_hepisema_adjustment);
    hepisema_adjustments[0].vbasepos = HVB_AUTO;
    hepisema_adjustments[0].nudge = NULL;

    texverbs_capacity = 32;
    texverbs = gregorio_grow_buffer(NULL, &texverbs_capacity, char *);
    texverbs[0] = NULL;
}

void gregorio_struct_destroy(void)
{
    size_t i;
    gregorio_hepisema_adjustment *adj;
    char **texverb;

    for (i = 0, adj = hepisema_adjustments; i <= hepisema_adjustments_last;
            ++i, ++adj) {
        if (adj->nudge) {
            free(adj->nudge);
        }
    }
    free(hepisema_adjustments);

    for (i = 0, texverb = texverbs; i <= texverbs_last; ++i, ++texverb) {
        if (*texverb) {
            /* If the code falls here, this is a programming error. */
            /* LCOV_EXCL_START */
            gregorio_fail(gregorio_struct_destroy, "unfreed texverb");
            free(*texverb);
        }
        /* LCOV_EXCL_STOP */
    }
    free(texverbs);
}

static unsigned short register_texverb(char *const texverb)
{
    if (texverbs_last == USHRT_MAX) {
        /* It's not reasonable to trigger this condition while testing */
        /* LCOV_EXCL_START */
        gregorio_message(_("too many texverbs"), "register_texverb",
                VERBOSITY_ERROR, 0);
        return 0;
        /* LCOV_EXCL_STOP */
    }
    ++texverbs_last;
    if (texverbs_last >= texverbs_capacity) {
        texverbs = gregorio_grow_buffer(texverbs, &texverbs_capacity, char *);
    }
    texverbs[texverbs_last] = texverb;
    return texverbs_last;
}

static void free_one_texverb(unsigned short index)
{
    if (index) {
        gregorio_assert(index <= texverbs_last, gregorio_texverb,
                "array index out of bounds", return);
        free(texverbs[index]);
        texverbs[index] = NULL;
    }
}

static gregorio_note *create_and_link_note(gregorio_note **current_note,
        const gregorio_scanner_location *const loc)
{
    gregorio_note *note = gregorio_calloc(1, sizeof(gregorio_note));
    note->previous = *current_note;
    note->next = NULL;
    if (*current_note) {
        (*current_note)->next = note;
    }
    *current_note = note;
    note->src_line = loc->first_line;
    note->src_column = loc->first_column;
    note->src_offset = loc->first_offset;

    return note;
}

void gregorio_position_h_episema_above(gregorio_note *note, signed char height,
        bool connect)
{
    assert(note && (note->type == GRE_NOTE || note->type == GRE_BAR));
    note->h_episema_above = height;
    note->h_episema_above_connect = connect;
}

static void set_h_episema_above(gregorio_note *note, signed char height,
        grehepisema_size size, bool connect)
{
    gregorio_position_h_episema_above(note, height, connect);
    note->h_episema_above_size = size;
}

void gregorio_position_h_episema_below(gregorio_note *note, signed char height,
        bool connect)
{
    assert(note && (note->type == GRE_NOTE || note->type == GRE_BAR));
    note->h_episema_below = height;
    note->h_episema_below_connect = connect;
}

static void set_h_episema_below(gregorio_note *note, signed char height,
        grehepisema_size size, bool connect)
{
    gregorio_position_h_episema_below(note, height, connect);
    note->h_episema_below_size = size;
}

void gregorio_add_note(gregorio_note **current_note, signed char pitch,
        gregorio_shape shape, gregorio_sign signs,
        gregorio_liquescentia liquescentia, gregorio_note *prototype,
        const gregorio_scanner_location *const loc)
{
    gregorio_note *element = create_and_link_note(current_note, loc);
    element->type = GRE_NOTE;
    element->u.note.pitch = pitch;
    element->u.note.shape = shape;
    element->signs = signs;
    element->special_sign = _NO_SIGN;
    element->u.note.liquescentia = liquescentia;
    if (prototype) {
        set_h_episema_above(element, prototype->h_episema_above,
                prototype->h_episema_above_size,
                prototype->h_episema_above_connect);
        set_h_episema_below(element, prototype->h_episema_below,
                prototype->h_episema_below_size,
                prototype->h_episema_below_connect);
        element->he_adjustment_index[SO_OVER] =
                prototype->he_adjustment_index[SO_OVER];
        element->he_adjustment_index[SO_UNDER] =
                prototype->he_adjustment_index[SO_UNDER];
        element->high_ledger_line = prototype->high_ledger_line;
        element->high_ledger_specificity = prototype->high_ledger_specificity;
        element->low_ledger_line = prototype->low_ledger_line;
        element->low_ledger_specificity = prototype->low_ledger_specificity;
    }
    element->texverb = 0;
    element->choral_sign = NULL;
}

static void add_pitched_item_as_note(gregorio_note **current_note,
        gregorio_type type, signed char pitch,
        const gregorio_scanner_location *const loc)
{
    gregorio_note *element = create_and_link_note(current_note, loc);
    element->type = type;
    element->u.note.pitch = pitch;
}

void gregorio_add_end_of_line_as_note(gregorio_note **current_note,
        bool eol_ragged, bool eol_forces_custos, bool eol_forces_custos_on,
        const gregorio_scanner_location *const loc)
{
    gregorio_note *element = create_and_link_note(current_note, loc);
    element->type = GRE_END_OF_LINE;
    element->u.other.eol_ragged = eol_ragged;
    element->u.other.eol_forces_custos = eol_forces_custos;
    element->u.other.eol_forces_custos_on = eol_forces_custos_on;
}

void gregorio_add_custos_as_note(gregorio_note **current_note,
        const gregorio_scanner_location *const loc)
{
    gregorio_note *element = create_and_link_note(current_note, loc);
    element->type = GRE_CUSTOS;
}

void gregorio_add_manual_custos_as_note(gregorio_note **current_note,
        signed char pitch, const gregorio_scanner_location *const loc)
{
    add_pitched_item_as_note(current_note, GRE_MANUAL_CUSTOS, pitch, loc);
}

void gregorio_add_suppress_custos_as_note(gregorio_note **current_note,
        const gregorio_scanner_location *const loc)
{
    gregorio_note *element = create_and_link_note(current_note, loc);
    element->type = GRE_SUPPRESS_CUSTOS;
}

void gregorio_add_clef_as_note(gregorio_note **current_note,
        gregorio_clef clef, signed char clef_line, bool flatted,
        const gregorio_scanner_location *const loc)
{
    gregorio_note *element = create_and_link_note(current_note, loc);
    element->type = GRE_CLEF;
    element->u.clef.clef = clef;
    element->u.clef.line = clef_line;
    element->u.clef.flatted = flatted;
}

void gregorio_add_secondary_clef_to_note(gregorio_note *current_note,
        gregorio_clef clef, signed char clef_line, bool flatted)
{
    gregorio_not_null(current_note, gregorio_add_secondary_clef_to_note,
            return);
    if (current_note->type != GRE_CLEF) {
        gregorio_message(_("trying to add a secondary clef to something that "
                    "is not a clef"), "gregorio_add_secondary_clef_to_note",
                VERBOSITY_ERROR, 0);
        return;
    }

    if (current_note->u.clef.secondary_line) {
        gregorio_message(_("secondary clef already exists"),
                "gregorio_add_secondary_clef_to_note", VERBOSITY_ERROR, 0);
        return;
    }

    current_note->u.clef.secondary_clef = clef;
    current_note->u.clef.secondary_line = clef_line;
    current_note->u.clef.secondary_flatted = flatted;
}

void gregorio_add_bar_as_note(gregorio_note **current_note, gregorio_bar bar,
        const gregorio_scanner_location *const loc)
{
    gregorio_note *element = create_and_link_note(current_note, loc);
    element->type = GRE_BAR;
    element->u.other.bar = bar;
}

void gregorio_add_space_as_note(gregorio_note **current_note,
        const gregorio_space space, char *factor,
        const gregorio_scanner_location *const loc)
{
    gregorio_note *element = create_and_link_note(current_note, loc);
    element->type = GRE_SPACE;
    element->u.other.space = space;
    element->u.other.ad_hoc_space_factor = factor;
}

unsigned short gregorio_add_texverb_as_note(gregorio_note **current_note,
        char *str, gregorio_type type,
        const gregorio_scanner_location *const loc)
{
    gregorio_note *element;
    /* gregorio_not_null(str, gregorio_add_texverb_as_note, return NULL); */
    element = create_and_link_note(current_note, loc);
    assert(type == GRE_TEXVERB_GLYPH || type == GRE_TEXVERB_ELEMENT
           || type == GRE_ALT);
    element->type = type;
    element->texverb = register_texverb(str);
    return element->texverb;
}

void gregorio_add_nlba_as_note(gregorio_note **current_note, gregorio_nlba type,
        const gregorio_scanner_location *const loc)
{
    gregorio_note *element = create_and_link_note(current_note, loc);
    element->type = GRE_NLBA;
    element->u.other.nlba = type;
}

void gregorio_start_autofuse(gregorio_note **current_note,
        const gregorio_scanner_location *const loc)
{
    gregorio_note *element = create_and_link_note(current_note, loc);
    element->type = GRE_AUTOFUSE_START;
}

void gregorio_end_autofuse(gregorio_note **current_note,
        const gregorio_scanner_location *const loc)
{
    gregorio_note *element = create_and_link_note(current_note, loc);
    element->type = GRE_AUTOFUSE_END;
}

static __inline void change_note_texverb(gregorio_note *note, char *str)
{
    if (note->texverb) {
        gregorio_change_texverb(note->texverb, str);
    } else {
        note->texverb = register_texverb(str);
    }
}

void gregorio_add_texverb_to_note(gregorio_note *current_note, char *str)
{
    size_t len;
    char *res;
    gregorio_not_null(str, gregorio_add_texverb_as_note, return);
    if (current_note) {
        const char *texverb = gregorio_texverb(current_note->texverb);
        if (texverb) {
            len = strlen(texverb) + strlen(str) + 1;
            res = gregorio_malloc(len);
            strcpy(res, texverb);
            strcat(res, str);
            free(str);
            change_note_texverb(current_note, res);
        } else {
            change_note_texverb(current_note, str);
        }
    }
}

void gregorio_add_cs_to_note(gregorio_note *const*const current_note,
        char *const str, const bool nabc)
{
    if (*current_note) {
        (*current_note)->choral_sign = str;
        (*current_note)->choral_sign_is_nabc = nabc;
    }
}

void gregorio_add_special_sign(gregorio_note *note, gregorio_sign sign)
{
    if (!note || note->type != GRE_NOTE) {
        gregorio_message(_("trying to add a special sign to something that is "
                    "not a note"), "gregorio_add_special_sign", VERBOSITY_ERROR,
                0);
        return;
    }
    note->special_sign = sign;
}

static void fix_oriscus_liquescentia(gregorio_note *const note,
        const bool legacy_oriscus_orientation)
{
    if (legacy_oriscus_orientation) {
        switch (note->u.note.liquescentia) {
        case L_AUCTUS_ASCENDENS:
            note->u.note.liquescentia =
                    (note->u.note.liquescentia & ~TAIL_LIQUESCENTIA_MASK)
                    | L_AUCTUS_DESCENDENS;
            /* fall through */
        case L_AUCTUS_DESCENDENS:
            note->u.note.shape = S_ORISCUS_DESCENDENS;
            break;
        case L_DEMINUTUS:
            note->u.note.shape = S_ORISCUS_DEMINUTUS;
            break;
        default:
            note->u.note.shape = S_ORISCUS_ASCENDENS;
            break;
        }
    } else {
        note->u.note.liquescentia &= ((~TAIL_LIQUESCENTIA_MASK) | L_DEMINUTUS);
        if (note->u.note.liquescentia & L_DEMINUTUS) {
            note->u.note.shape = S_ORISCUS_DEMINUTUS;
        }
    }
}

static void fix_oriscus_scapus_liquescentia(gregorio_note *const note,
        const bool legacy_oriscus_orientation)
{
    if (legacy_oriscus_orientation) {
        switch (note->u.note.liquescentia) {
        case L_AUCTUS_ASCENDENS:
            note->u.note.liquescentia =
                    (note->u.note.liquescentia & ~TAIL_LIQUESCENTIA_MASK)
                    | L_AUCTUS_DESCENDENS;
            /* fall through */
        case L_AUCTUS_DESCENDENS:
            note->u.note.shape = S_ORISCUS_SCAPUS_DESCENDENS;
            break;
        default:
            note->u.note.shape = S_ORISCUS_SCAPUS_ASCENDENS;
            break;
        }
    } else {
        note->u.note.liquescentia &= ~TAIL_LIQUESCENTIA_MASK;
    }
}

void gregorio_change_shape(gregorio_note *const note,
        const gregorio_shape shape, const bool legacy_oriscus_orientation)
{
    if (!note || note->type != GRE_NOTE) {
        gregorio_message(_("trying to change the shape of something that is "
                    "not a note"), "change_shape", VERBOSITY_ERROR, 0);
        return;
    }

    note->u.note.shape = shape;
    switch (shape) {
    case S_STROPHA:
    case S_DISTROPHA:
    case S_TRISTROPHA:
        note->u.note.liquescentia &= L_AUCTUS_ASCENDENS | L_INITIO_DEBILIS;
        break;

    case S_ORISCUS_UNDETERMINED:
    case S_ORISCUS_ASCENDENS:
    case S_ORISCUS_DESCENDENS:
        fix_oriscus_liquescentia(note, legacy_oriscus_orientation);
        break;

    case S_ORISCUS_SCAPUS_UNDETERMINED:
    case S_ORISCUS_SCAPUS_ASCENDENS:
    case S_ORISCUS_SCAPUS_DESCENDENS:
        fix_oriscus_scapus_liquescentia(note, legacy_oriscus_orientation);
        break;

    default:
        break;
    }
}

void gregorio_add_cavum(gregorio_note *const note)
{
    if (!note || note->type != GRE_NOTE) {
        gregorio_message(_("trying to add cavum to something that is "
                    "not a note"), "change_shape", VERBOSITY_ERROR, 0);
        return;
    }
    note->u.note.is_cavum = true;
}

void gregorio_add_tail_liquescentia(gregorio_note *note,
        gregorio_liquescentia liq, const bool legacy_oriscus_orientation)
{
    if (!note || note->type != GRE_NOTE) {
        gregorio_message(_("trying to make a liquescence on something that "
                    "is not a note"), "add_liquescentia", VERBOSITY_ERROR, 0);
        return;
    }

    note->u.note.liquescentia =
        (note->u.note.liquescentia & ~TAIL_LIQUESCENTIA_MASK)
        | (liq & TAIL_LIQUESCENTIA_MASK);

    switch (note->u.note.shape) {
    case S_STROPHA:
    case S_DISTROPHA:
    case S_TRISTROPHA:
        if (note->u.note.liquescentia & L_AUCTUS_DESCENDENS) {
            note->u.note.liquescentia =
                (note->u.note.liquescentia & ~TAIL_LIQUESCENTIA_MASK)
                | L_AUCTUS_ASCENDENS;
        }
        break;

    case S_ORISCUS_UNDETERMINED:
    case S_ORISCUS_ASCENDENS:
    case S_ORISCUS_DESCENDENS:
    case S_ORISCUS_DEMINUTUS:
        fix_oriscus_liquescentia(note, legacy_oriscus_orientation);
        break;

    case S_ORISCUS_SCAPUS_UNDETERMINED:
    case S_ORISCUS_SCAPUS_ASCENDENS:
    case S_ORISCUS_SCAPUS_DESCENDENS:
        fix_oriscus_scapus_liquescentia(note, legacy_oriscus_orientation);
        break;

    default:
        break;
    }
}

static void apply_auto_h_episema(gregorio_note *const note,
        const grehepisema_size size, const bool disable_bridge)
{
    if (note->h_episema_above == HEPISEMA_NONE
            && note->h_episema_below == HEPISEMA_NONE) {
        /* if both are unset, set both to auto */
        set_h_episema_above(note, HEPISEMA_AUTO, size, !disable_bridge);
        set_h_episema_below(note, HEPISEMA_AUTO, size, !disable_bridge);
    } else if (note->h_episema_above == HEPISEMA_AUTO
            && note->h_episema_below == HEPISEMA_AUTO) {
        /* if both are auto, then force both */
        /* the upper episema keeps its settings */
        note->h_episema_above = HEPISEMA_FORCED;

        set_h_episema_below(note, HEPISEMA_FORCED, size, !disable_bridge);
    } else {
        /* force whichever is not already forced */
        if (note->h_episema_above != HEPISEMA_FORCED) {
            set_h_episema_above(note, HEPISEMA_FORCED, size, !disable_bridge);
        }
        if (note->h_episema_below != HEPISEMA_FORCED) {
            set_h_episema_below(note, HEPISEMA_FORCED, size, !disable_bridge);
        }
    }
}

/**********************************
 *
 * Activate_isolated_h_episema is used when we see an "isolated"
 * horizontal episema: when we type ab__ lex see a then b then _ then _, so
 * we must put the _ on the a (kind of backward process), and say the
 * the episema on the b is a multi episema. Here n is the length of
 * the isolated episema we found (can be up to 4).
 *
 *********************************/
static void gregorio_activate_isolated_h_episema(gregorio_note *note,
        const grehepisema_size size, const bool disable_bridge, int n)
{
    gregorio_not_null(note, gregorio_activate_isolated_h_episema, return);
    if (note->type != GRE_NOTE) {
        gregorio_message(ngt_("isolated horizontal episema after something "
                    "that is not a note, ignored",
                    "isolated horizontal episema after something that is not "
                    "a note, ignored", n), "activate_h_isolated_episema",
                VERBOSITY_ERROR, 0);
        return;
    }
    for (; n > 0; --n) {
        note = note->previous;
        if (!note || note->type != GRE_NOTE) {
            gregorio_message(_("found more horizontal episema than notes "
                        "able to be under"), "activate_h_isolated_episema",
                    VERBOSITY_ERROR, 0);
            return;
        }
    }
    apply_auto_h_episema(note, size, disable_bridge);
}

void gregorio_add_h_episema(gregorio_note *note,
        grehepisema_size size, gregorio_vposition vposition,
        bool disable_bridge, unsigned int *nbof_isolated_episema)
{
    if (!note || (note->type != GRE_NOTE && note->type != GRE_BAR)) {
        gregorio_message(_("trying to add a horizontal episema on something "
                    "that is not a note"), "add_h_episema",
                VERBOSITY_ERROR, 0);
        return;
    }
    gregorio_not_null(nbof_isolated_episema, gregorio_add_h_episema, return);
    if (vposition && *nbof_isolated_episema) {
        gregorio_message(_("trying to add a forced horizontal episema on a "
                    "note which already has an automatic horizontal "
                    "episema"), "add_h_episema", VERBOSITY_ERROR, 0);
        return;
    }

    if (vposition || !*nbof_isolated_episema) {
        switch (vposition) {
        case VPOS_ABOVE:
            set_h_episema_above(note, HEPISEMA_FORCED, size, !disable_bridge);
            break;

        case VPOS_BELOW:
            set_h_episema_below(note, HEPISEMA_FORCED, size, !disable_bridge);
            break;

        default: /* VPOS_AUTO */
            apply_auto_h_episema(note, size, disable_bridge);
            *nbof_isolated_episema = 1;
            break;
        }
    } else {
        gregorio_activate_isolated_h_episema(note, size, disable_bridge,
                (*nbof_isolated_episema)++);
    }
}

void gregorio_add_sign(gregorio_note *note, gregorio_sign sign,
        gregorio_vposition vposition)
{
    if (!note) {
        gregorio_message(_("trying to add a sign to something that is "
                    "not a note"), "gregorio_add_sign", VERBOSITY_ERROR, 0);
        return;
    }
    switch (sign) {
    case _PUNCTUM_MORA:
        switch (note->signs) {
        case _NO_SIGN:
            note->signs = _PUNCTUM_MORA;
            break;
        case _V_EPISEMA:
            note->signs = _V_EPISEMA_PUNCTUM_MORA;
            break;
        case _PUNCTUM_MORA:
            note->signs = _AUCTUM_DUPLEX;
            break;
        case _V_EPISEMA_PUNCTUM_MORA:
            note->signs = _V_EPISEMA_AUCTUM_DUPLEX;
            break;
        default:
            break;
        }

        note->mora_vposition = vposition;
        break;

    case _V_EPISEMA:
        switch (note->signs) {
        case _NO_SIGN:
            note->signs = _V_EPISEMA;
            break;
        case _PUNCTUM_MORA:
            note->signs = _V_EPISEMA_PUNCTUM_MORA;
            break;
        case _AUCTUM_DUPLEX:
            note->signs = _V_EPISEMA_AUCTUM_DUPLEX;
            break;
        default:
            break;
        }

        if (note->type == GRE_NOTE && vposition) {
            note->v_episema_height = note->u.note.pitch + vposition;
        }
        break;

    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        gregorio_fail2(gregorio_add_sign, "unexpected sign to add: %s",
                gregorio_sign_to_string(sign));
        break;
        /* LCOV_EXCL_STOP */
    }
}

void gregorio_go_to_first_note(gregorio_note **note)
{
    gregorio_note *tmp;
    gregorio_not_null_ptr(note, gregorio_go_to_first_note, return);
    tmp = *note;
    while (tmp->previous) {
        tmp = tmp->previous;
    }
    *note = tmp;
}

static __inline void free_one_note(gregorio_note *note)
{
    if (note->type == GRE_SPACE) {
        free(note->u.other.ad_hoc_space_factor);
    }
    free_one_texverb(note->texverb);
    free(note->choral_sign);
    free(note);
}

void gregorio_free_one_note(gregorio_note **note)
{
    gregorio_note *next = NULL;
    gregorio_not_null_ptr(note, gregorio_free_one_note, return);
    if ((*note)->next) {
        (*note)->next->previous = (*note)->previous;
        next = (*note)->next;
    }
    if ((*note)->previous) {
        (*note)->previous->next = (*note)->next;
    }
    free_one_note(*note);
    *note = next;
}

static void gregorio_free_notes(gregorio_note **note)
{
    gregorio_note *tmp;
    while (*note) {
        tmp = (*note)->next;
        free_one_note(*note);
        *note = tmp;
    }
}

static gregorio_glyph *create_and_link_glyph(gregorio_glyph **current_glyph)
{
    gregorio_glyph *glyph = gregorio_calloc(1, sizeof(gregorio_glyph));
    glyph->previous = *current_glyph;
    glyph->next = NULL;
    if (*current_glyph) {
        (*current_glyph)->next = glyph;
    }
    *current_glyph = glyph;

    return glyph;
}

void gregorio_add_glyph(gregorio_glyph **current_glyph,
        gregorio_glyph_type type, gregorio_note *first_note,
        gregorio_liquescentia liquescentia, bool is_cavum)
{
    gregorio_glyph *next_glyph = create_and_link_glyph(current_glyph);
    next_glyph->type = GRE_GLYPH;
    next_glyph->u.notes.glyph_type = type;
    next_glyph->u.notes.liquescentia = liquescentia;
    next_glyph->u.notes.is_cavum = is_cavum;
    next_glyph->u.notes.first_note = first_note;
}

void gregorio_add_clef_as_glyph(gregorio_glyph **current_glyph,
        gregorio_clef_info clef, unsigned short texverb)
{
    gregorio_glyph *next_glyph = create_and_link_glyph(current_glyph);
    next_glyph->type = GRE_CLEF;
    next_glyph->u.misc.clef = clef;
    next_glyph->texverb = texverb;
}

void gregorio_add_pitched_element_as_glyph(gregorio_glyph **current_glyph,
        gregorio_type type, signed char pitch, bool force_pitch,
        unsigned short texverb)
{
    gregorio_glyph *next_glyph = create_and_link_glyph(current_glyph);
    assert(type == GRE_CUSTOS);
    next_glyph->type = type;
    next_glyph->u.misc.pitched.pitch = pitch;
    next_glyph->u.misc.pitched.force_pitch = force_pitch;
    next_glyph->texverb = texverb;
}

void gregorio_add_unpitched_element_as_glyph(gregorio_glyph **current_glyph,
        gregorio_type type, gregorio_extra_info *info, gregorio_sign sign,
        unsigned short texverb)
{
    gregorio_glyph *next_glyph = create_and_link_glyph(current_glyph);
    assert(type != GRE_NOTE && type != GRE_GLYPH && type != GRE_ELEMENT
           && type != GRE_CLEF && type != GRE_CUSTOS);
    next_glyph->type = type;
    next_glyph->u.misc.unpitched.info = *info;
    next_glyph->u.misc.unpitched.special_sign = sign;
    next_glyph->texverb = texverb;

    /* this was copied into the glyph, so we need to clear it to avoid a
     * double-free */
    info->ad_hoc_space_factor = NULL;
}

void gregorio_go_to_first_glyph(gregorio_glyph **glyph)
{
    gregorio_glyph *tmp;
    gregorio_not_null_ptr(glyph, gregorio_go_to_first_glyph, return);
    tmp = *glyph;
    while (tmp->previous) {
        tmp = tmp->previous;
    }
    *glyph = tmp;
}

static __inline void free_one_glyph(gregorio_glyph *glyph)
{
    free_one_texverb(glyph->texverb);
    switch (glyph->type) {
    case GRE_GLYPH:
        gregorio_free_notes(&glyph->u.notes.first_note);
        break;
    case GRE_SPACE:
        free(glyph->u.misc.unpitched.info.ad_hoc_space_factor);
        break;
    default:
        /* nothing to do */
        break;
    }
    free(glyph);
}

void gregorio_free_one_glyph(gregorio_glyph **glyph)
{
    gregorio_glyph *next = NULL;
    gregorio_not_null_ptr(glyph, gregorio_free_one_glyph, return);
    if ((*glyph)->next) {
        (*glyph)->next->previous = NULL;
        next = (*glyph)->next;
    }
    if ((*glyph)->previous) {
        (*glyph)->previous->next = NULL;
    }
    free_one_glyph(*glyph);
    *glyph = next;
}

static void gregorio_free_glyphs(gregorio_glyph **glyph)
{
    gregorio_glyph *next_glyph;
    gregorio_not_null(glyph, gregorio_free_glyphs, return);
    while (*glyph) {
        next_glyph = (*glyph)->next;
        free_one_glyph(*glyph);
        *glyph = next_glyph;
    }
}

static gregorio_element *create_and_link_element(gregorio_element
                                                 **current_element)
{
    gregorio_element *element = gregorio_calloc(1, sizeof(gregorio_element));
    element->previous = *current_element;
    element->next = NULL;
    if (*current_element) {
        (*current_element)->next = element;
    }
    *current_element = element;

    return element;
}

void gregorio_add_element(gregorio_element **current_element,
        gregorio_glyph *first_glyph)
{
    gregorio_element *next = create_and_link_element(current_element);
    next->type = GRE_ELEMENT;
    next->u.first_glyph = first_glyph;
}

void gregorio_add_misc_element(gregorio_element **current_element,
        gregorio_type type, gregorio_misc_element_info *info,
        unsigned short texverb)
{
    gregorio_element *special = create_and_link_element(current_element);
    special->type = type;
    special->u.misc = *info;
    special->texverb = texverb;

    if (type == GRE_SPACE) {
        /* this was copied into the glyph, so we need to clear it to avoid a
         * double-free */
        info->unpitched.info.ad_hoc_space_factor = NULL;
    }
}

static __inline void free_one_element(gregorio_element *element)
{
    size_t i;
    free_one_texverb(element->texverb);
    for (i = 0; i < element->nabc_lines; i++) {
        free(element->nabc[i]);
    }
    free(element->nabc);
    switch (element->type) {
    case GRE_ELEMENT:
        gregorio_free_glyphs(&element->u.first_glyph);
        break;
    case GRE_SPACE:
        free(element->u.misc.unpitched.info.ad_hoc_space_factor);
        break;
    default:
        /* nothing to do */
        break;
    }
    free(element);
}

void gregorio_free_one_element(gregorio_element **element)
{
    gregorio_element *next = NULL;
    gregorio_element *to_free;
    gregorio_not_null_ptr(element, gregorio_free_one_element, return);
    to_free = *element;
    if (to_free->next) {
        to_free->next->previous = NULL;
        next = to_free->next;
    }
    if (to_free->previous) {
        to_free->previous->next = NULL;
    }
    /* The previous line is probably optimized out */
    /* LCOV_EXCL_STOP */
    free_one_element(to_free);
    *element = next;
}

static void gregorio_free_elements(gregorio_element **element)
{
    gregorio_element *next;
    if (!element || !*element) {
        return;
    }
    while (*element) {
        next = (*element)->next;
        free_one_element(*element);
        *element = next;
    }
}

void gregorio_add_character(gregorio_character **current_character,
        grewchar wcharacter)
{
    gregorio_character *element =
        (gregorio_character *) gregorio_calloc(1, sizeof(gregorio_character));
    element->is_character = 1;
    element->cos.character = wcharacter;
    element->next_character = NULL;
    element->previous_character = *current_character;
    if (*current_character) {
        (*current_character)->next_character = element;
    }
    *current_character = element;
}

static void gregorio_free_one_character(gregorio_character *current_character)
{
    free(current_character);
}

void gregorio_free_characters(gregorio_character *current_character)
{
    gregorio_character *next_character;
    gregorio_not_null(current_character, gregorio_free_characters, return);
    while (current_character) {
        next_character = current_character->next_character;
        gregorio_free_one_character(current_character);
        current_character = next_character;
    }
}

void gregorio_go_to_first_character(const gregorio_character **character)
{
    const gregorio_character *tmp;
    gregorio_not_null_ptr(character, gregorio_free_characters, return);
    tmp = *character;
    while (tmp->previous_character) {
        tmp = tmp->previous_character;
    }
    *character = tmp;
}

void gregorio_begin_style(gregorio_character **current_character,
        grestyle_style style)
{
    gregorio_character *element =
        (gregorio_character *) gregorio_calloc(1, sizeof(gregorio_character));
    element->is_character = 0;
    element->cos.s.type = ST_T_BEGIN;
    element->cos.s.style = style;
    element->previous_character = *current_character;
    element->next_character = NULL;
    if (*current_character) {
        (*current_character)->next_character = element;
    }
    *current_character = element;
}

void gregorio_end_style(gregorio_character **current_character,
        grestyle_style style)
{
    gregorio_character *element =
        (gregorio_character *) gregorio_calloc(1, sizeof(gregorio_character));
    element->is_character = 0;
    element->cos.s.type = ST_T_END;
    element->cos.s.style = style;
    element->next_character = NULL;
    element->previous_character = *current_character;
    if (*current_character) {
        (*current_character)->next_character = element;
    }
    *current_character = element;
}

gregorio_character *gregorio_clone_characters(
        const gregorio_character *source)
{
    gregorio_character *target = NULL, *current = NULL;

    for (; source; source = source->next_character) {
        gregorio_character *character = (gregorio_character *)
                gregorio_calloc(1, sizeof(gregorio_character));
        *character = *source;
        character->next_character = NULL;

        if (current) {
            character->previous_character = current;
            current = current->next_character = character;
        } else {
            character->previous_character = NULL;
            target = current = character;
        }
    }

    return target;
}

void gregorio_add_syllable(gregorio_syllable **current_syllable,
        int number_of_voices, gregorio_element *elements[],
        gregorio_character *first_character,
        gregorio_character *first_translation_character,
        gregorio_word_position position, char *abovelinestext,
        gregorio_tr_centering translation_type, gregorio_nlba no_linebreak_area,
        gregorio_euouae euouae, const gregorio_scanner_location *const loc,
        const bool first_word, const bool clear)
{
    gregorio_syllable *next;
    gregorio_element **tab;
    int i;
    gregorio_not_null(elements, gregorio_add_syllable, return);
    gregorio_assert(number_of_voices == 1, gregorio_add_syllable,
            "gregorio only supports one voice", return);
    next = gregorio_calloc(1, sizeof(gregorio_syllable));
    next->position = position;
    next->no_linebreak_area = no_linebreak_area;
    next->euouae = euouae;
    next->text = first_character;
    next->translation = first_translation_character;
    next->translation_type = translation_type;
    next->abovelinestext = abovelinestext;
    next->first_word = first_word;
    next->clear = clear;
    if (loc) {
        next->src_line = loc->first_line;
        next->src_column = loc->first_column;
        next->src_offset = loc->first_offset;
    }
    next->next_syllable = NULL;
    next->previous_syllable = *current_syllable;
    tab = (gregorio_element **) gregorio_malloc(number_of_voices *
            sizeof(gregorio_element *));
    for (i = 0; i < number_of_voices; i++) {
        tab[i] = elements[i];
    }
    next->elements = tab;
    if (*current_syllable) {
        (*current_syllable)->next_syllable = next;
    }
    *current_syllable = next;
}

static void gregorio_free_one_syllable(gregorio_syllable **syllable,
        int number_of_voices)
{
    int i;
    gregorio_syllable *next;
    gregorio_not_null_ptr(syllable, gregorio_free_one_syllable, return);
    for (i = 0; i < number_of_voices; i++) {
        gregorio_free_elements((struct gregorio_element **)
                               &((*syllable)->elements[i]));
    }
    if ((*syllable)->text) {
        gregorio_free_characters((*syllable)->text);
    }
    if ((*syllable)->translation) {
        gregorio_free_characters((*syllable)->translation);
    }
    free((*syllable)->abovelinestext);
    next = (*syllable)->next_syllable;
    if (next) {
        next->previous_syllable = NULL;
    }
    free((*syllable)->elements);
    free(*syllable);
    *syllable = next;
}

static void gregorio_free_syllables(gregorio_syllable **syllable,
        int number_of_voices)
{
    gregorio_not_null_ptr(syllable, gregorio_free_one_syllable, return);
    while (*syllable) {
        gregorio_free_one_syllable(syllable, number_of_voices);
    }
}

gregorio_score *gregorio_new_score(void)
{
    gregorio_score *new_score = gregorio_calloc(1, sizeof(gregorio_score));
    new_score->number_of_voices = 1;
    gregorio_set_score_staff_lines(new_score, 4);
    return new_score;
}

static void gregorio_free_score_infos(gregorio_score *score)
{
    gregorio_not_null(score, gregorio_free_score_infos, return);
    /* don't free the strings coming from headers; they will be freed when the
     * headers themselves are freed */
    if (score->first_voice_info) {
        gregorio_free_voice_infos(score->first_voice_info);
    }
}

static void free_headers(gregorio_score *score) {
    gregorio_header *header = score->headers;
    while (header) {
        gregorio_header *next = header->next;
        free(header->name);
        free(header->value);
        free(header);
        header = next;
    }
}

void gregorio_free_score(gregorio_score *score)
{
    gregorio_not_null(score, gregorio_free_score, return);
    if (score->first_syllable) {
        gregorio_free_syllables(&(score->first_syllable),
                score->number_of_voices);
    }
    gregorio_free_score_infos(score);
    free_headers(score);
    free(score);
}

void gregorio_add_voice_info(gregorio_voice_info **current_voice_info)
{
    gregorio_voice_info *next = gregorio_calloc(1, sizeof(gregorio_voice_info));
    gregorio_assert(!*current_voice_info, gregorio_add_voice_info,
            "since gregorio only supports once voice, this should only be "
            "called once", (*current_voice_info)->next_voice_info = next);
    *current_voice_info = next;
}

void gregorio_free_voice_infos(gregorio_voice_info *voice_info)
{
    gregorio_voice_info *next;
    gregorio_not_null(voice_info, gregorio_free_voice_infos, return);
    while (voice_info) {
        next = voice_info->next_voice_info;
        free(voice_info);
        voice_info = next;
    }
}

void gregorio_set_score_annotation(gregorio_score *score, char *annotation)
{
    int annotation_num;
    gregorio_not_null(score, gregorio_set_score_annotation, return);
    /* save the annotation in the first spare place. */
    for (annotation_num = 0; annotation_num < MAX_ANNOTATIONS; ++annotation_num) {
        if (score->annotation[annotation_num] == NULL) {
            score->annotation[annotation_num] = annotation;
            break;
        }
    }
    if (annotation_num >= MAX_ANNOTATIONS) {
        gregorio_message(_("too many annotations"),
                "gregorio_set_annotation", VERBOSITY_WARNING, 0);
    }
}

void gregorio_set_score_staff_lines(gregorio_score *const score,
        const char staff_lines)
{
    gregorio_not_null(score, gregorio_set_score_staff_lines, return);
    if (staff_lines < 2 || staff_lines > 5) {
        gregorio_message(_("invalid number of staff lines"),
                "gregorio_set_score_staff_lines", VERBOSITY_ERROR, 0);
        return;
    }
    score->staff_lines = staff_lines;
    score->highest_pitch = LOWEST_PITCH + 4 + (2 * staff_lines);
    score->high_ledger_line_pitch = score->highest_pitch - 1;
    score->virgula_far_pitch = score->highest_pitch - 6;

    gregorio_assert(score->highest_pitch <= MAX_PITCH,
            gregorio_set_score_staff_lines, "highest pitch exceeds MAX_PITCH",
            return);
}

void gregorio_add_score_header(gregorio_score *score, char *name, char *value)
{
    gregorio_header *header;
    gregorio_not_null(score, gregorio_add_score_header, return);
    header = (gregorio_header *)gregorio_malloc(sizeof(gregorio_header));
    header->name = name;
    header->value = value;
    header->next = NULL;
    if (score->last_header) {
        score->last_header->next = header;
    } else {
        score->headers = header;
    }
    score->last_header = header;
}

/**********************************
 *
 * A function to build an integer from a key, very useful to represent
 * it in the structure.
 *
 *The representation is :
 *
 * * 1 for a C key on the first (bottom) line
 * * 3 for a C key on the second line
 * * 5 for a C key on the third line (default key)
 * * 7 for a C key on the fourth line
 * * 9 for a C key on the fifth line
 *
 * * -2 for a F key on the first line
 * * 0 for a F key on the second line
 * * 2 for a F key on the third line
 * * 4 for a F key on the fourth line
 * * 6 for a F key on the fifth line
 *
 *********************************/

int gregorio_calculate_new_key(gregorio_clef_info clef)
{
    switch (clef.clef) {
    case CLEF_C:
        return (2 * clef.line) - 1;
        break;
    case CLEF_F:
        return (2 * clef.line) - 4;
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        gregorio_fail(gregorio_calculate_new_key, "can't calculate key");
        return NO_KEY;
        /* LCOV_EXCL_STOP */
    }
}

/* shape is an output parameter */
static __inline signed char next_pitch_from_glyph(const gregorio_glyph *glyph,
        gregorio_shape *const alterations, gregorio_shape *const shape)
{
    while (glyph) {
        if (glyph->type == GRE_GLYPH) {
            if (glyph->u.notes.glyph_type == G_ALTERATION) {
                const gregorio_note *note;
                for (note = glyph->u.notes.first_note; note;
                        note = note->next) {
                    switch (note->u.note.shape) {
                    case S_FLAT:
                    case S_SHARP:
                    case S_NATURAL:
                        if (note->u.note.pitch >= LOWEST_PITCH &&
                                note->u.note.pitch <= MAX_PITCH) {
                            alterations[note->u.note.pitch] =
                                    note->u.note.shape;
                        }
                        break;
                    default:
                        /* not reachable unless there's a programming error */
                        /* LCOV_EXCL_START */
                        gregorio_fail2(next_pitch_from_glyph,
                                "unrecognized alteration shape: %s",
                                gregorio_shape_to_string(note->u.note.shape));
                        break;
                        /* LCOV_EXCL_STOP */
                    }
                }
            } else if (glyph->u.notes.first_note) {
                assert(glyph->u.notes.first_note->type == GRE_NOTE);
                if (shape) {
                    *shape = alterations[
                            glyph->u.notes.first_note->u.note.pitch];
                }
                return glyph->u.notes.first_note->u.note.pitch;
            }
        }
        glyph = glyph->next;
    }
    return 0;
}

/* shape is an output parameter */
static signed char syllable_first_note(const gregorio_syllable *syllable,
        gregorio_shape *const alterations, gregorio_shape *const shape)
{
    gregorio_element *element;
    gregorio_glyph *glyph;
    signed char pitch;
    gregorio_not_null(syllable, syllable_first_note, return 0);
    element = syllable->elements[0];
    while (element) {
        if (element->type == GRE_CUSTOS) {
            return element->u.misc.pitched.pitch;
        }
        if (element->type == GRE_ELEMENT && element->u.first_glyph) {
            glyph = element->u.first_glyph;
            pitch = next_pitch_from_glyph(glyph, alterations, shape);
            if (pitch) {
                return pitch;
            }
        }
        element = element->next;
    }
    return 0;
}

/* shape is an output parameter */
signed char gregorio_determine_next_pitch(const gregorio_syllable *syllable,
        const gregorio_element *element, const gregorio_glyph *glyph,
        gregorio_shape *const shape)
{
    signed char pitch;
    gregorio_shape alterations[MAX_PITCH + 1];

    memset(alterations, 0, sizeof alterations);

    if (shape) {
        *shape = S_UNDETERMINED;
    }
    gregorio_not_null(element, gregorio_determine_next_pitch,
            return DUMMY_PITCH);
    gregorio_not_null(syllable, gregorio_determine_next_pitch,
            return DUMMY_PITCH);
    /* we first explore the next glyphs to find a note, if there is one */
    if (glyph) {
        glyph = glyph->next;
        pitch = next_pitch_from_glyph(glyph, alterations, shape);
        if (pitch) {
            return pitch;
        }
    }
    /* then we do the same with the elements */
    element = element->next;
    while (element) {
        if (element->type == GRE_CUSTOS) {
            return element->u.misc.pitched.pitch;
        }
        if (element->type == GRE_ELEMENT && element->u.first_glyph) {
            glyph = element->u.first_glyph;
            pitch = next_pitch_from_glyph(glyph, alterations, shape);
            if (pitch) {
                return pitch;
            }
        } /* I think this is optimized out; LCOV_EXCL_LINE */
        element = element->next;
    }

    /* then we do the same with the syllables */
    syllable = syllable->next_syllable;
    while (syllable) {
        /* we call another function that will return the pitch of the first
         * note if syllable has a note, and 0 else */
        pitch = syllable_first_note(syllable, alterations, shape);
        if (pitch) {
            return pitch;
        }
        syllable = syllable->next_syllable;
    }
    /* here it means that there is no next note, so we return a stupid value,
     * but it won' t be used */
    if (shape) {
        *shape = S_UNDETERMINED;
    }
    return DUMMY_PITCH;
}

/**********************************
 *
 * A function called after the entire score is determined : we check
 * if the first element is a key change, if it is the case we delete
 * it and we update the score->voice-info->initial_key. Works in
 * polyphony.
 *
 ******************************/

void gregorio_fix_initial_keys(gregorio_score *score,
        gregorio_clef_info default_clef)
{
    gregorio_element *element;
    gregorio_voice_info *voice_info;
    int i;
    char to_delete = 1;

    gregorio_not_null(score, gregorio_fix_initial_keys, return);
    gregorio_not_null(score->first_voice_info, gregorio_fix_initial_keys,
            return);
    if (!score->first_syllable) {
        /* valid but almost nonsense: a score with no syllables */
        return;
    }
    voice_info = score->first_voice_info;
    for (i = 0; i < score->number_of_voices; i++) {
        element = score->first_syllable->elements[i];
        if (element && element->type == GRE_CLEF) {
            voice_info->initial_clef = element->u.misc.clef;
            gregorio_free_one_element(&(score->first_syllable->elements[i]));
            gregorio_messagef("gregorio_fix_initial_keys", VERBOSITY_INFO, 0,
                    _("in voice %d the first element is a key definition, "
                    "considered as initial key"), i + 1);
        }
        voice_info = voice_info->next_voice_info;
    }

    /* then we suppress syllables that contain nothing anymore : case of (c2)
     * at beginning of files */

    for (i = 0; i < score->number_of_voices; i++) {
        if (score->first_syllable->elements[i]) {
            to_delete = 0;
            break;
        }
    }

    if (to_delete) {
        gregorio_free_one_syllable(&(score->first_syllable),
                                   score->number_of_voices);
    }
    /* finally we initialize voice infos that have no initial key to default
     * key */

    voice_info = score->first_voice_info;

    for (i = 0; i < score->number_of_voices; i++) {
        if (!voice_info->initial_clef.line) {
            voice_info->initial_clef = default_clef;
            gregorio_messagef("gregorio_fix_initial_keys", VERBOSITY_INFO, 0,
                    _("no initial key definition in voice %d, default key "
                    "definition applied"), i + 1);
        }
        voice_info = voice_info->next_voice_info;
    }
}

/**********************************
 *
 * A small function to determine if an element list contains only
 * special elements (bar, key-change, etc.), useful because the
 * representation (in xml for example) may vary according to it.
 *
 *********************************/

bool gregorio_is_only_special(gregorio_element *element)
{
    gregorio_not_null(element, gregorio_is_only_special, return 0);
    while (element) {
        if (element->type == GRE_ELEMENT) {
            return 0;
        }
        element = element->next;
    }
    return 1;
}

/* This should only be called when an unknown enum value is encountered; as
 * such, this is a programming error */
/* LCOV_EXCL_START */
const char *gregorio_unknown(int value) {
    static char buf[20];
    gregorio_snprintf(buf, sizeof buf, "?%d", value);
    return buf;
}
/* LCOV_EXCL_STOP */

/*
 * A small helper for the following function
 */

static __inline bool is_clef(gregorio_type x)
{
    return x == GRE_CLEF;
}

/*
 * This function is used in write_syllable, it detects if the syllable is like
 * (c4), (::c4), (z0c4) or (z0::c4). It returns the gregorio_element of the
 * clef change.
 */
gregorio_element *gregorio_get_clef_change(gregorio_syllable *syllable)
{
    gregorio_element *element;
    gregorio_assert(syllable && syllable->elements && syllable->elements[0],
            gregoriotex_syllable_is_clef_change, "invalid syllable",
            return NULL);
    element = syllable->elements[0];
    /* we just detect the foud cases */
    if (element->type == GRE_CUSTOS && element->next
            && (is_clef(element->next->type)) && !element->next->next) {
        return element->next;
    }
    if (element->type == GRE_BAR && element->next
            && (is_clef(element->next->type)) && !element->next->next) {
        return element->next;
    }
    if ((is_clef(element->type)) && !element->next) {
        return element;
    }
    if (element->type == GRE_CUSTOS && element->next
            && element->next->type == GRE_BAR && element->next->next
            && (is_clef(element->next->next->type))
            && !element->next->next->next) {
        return element->next->next;
    }
    return NULL;
}

unsigned short gregorio_add_hepisema_adjustment(
        const gregorio_hepisema_vbasepos vbasepos, char *const nudge)
{
    if (hepisema_adjustments_last == USHRT_MAX) {
        /* It's not reasonable to trigger this condition while testing */
        /* LCOV_EXCL_START */
        gregorio_message(_("too many horizontal episema adjustments"),
                "gregorio_add_hepisema_adjustment", VERBOSITY_ERROR, 0);
        return 0;
        /* LCOV_EXCL_STOP */
    }
    ++hepisema_adjustments_last;
    if (hepisema_adjustments_last >= hepisema_adjustments_capacity) {
        hepisema_adjustments = gregorio_grow_buffer(hepisema_adjustments,
                &hepisema_adjustments_capacity, gregorio_hepisema_adjustment);
    }
    hepisema_adjustments[hepisema_adjustments_last].vbasepos = vbasepos;
    hepisema_adjustments[hepisema_adjustments_last].nudge = nudge;
    hepisema_adjustments[hepisema_adjustments_last].pitch_extremum = NO_PITCH;
    return hepisema_adjustments_last;
}

gregorio_hepisema_adjustment *gregorio_get_hepisema_adjustment(
        const unsigned short index)
{
    gregorio_assert(index <= hepisema_adjustments_last,
            gregorio_get_hepisema_adjustment, "array index out of bounds",
            return &hepisema_adjustments[0]);
    return &hepisema_adjustments[index];
}

const char *gregorio_texverb(unsigned short index)
{
    gregorio_assert(index <= texverbs_last, gregorio_texverb,
            "array index out of bounds", return NULL);
    return texverbs[index];
}

void gregorio_change_texverb(unsigned short index, char *texverb)
{
    /* not allowed to change index 0 (== NULL) */
    gregorio_assert(index != 0, gregorio_texverb,
            "array index 0 may not be changed", return);
    gregorio_assert(index <= texverbs_last, gregorio_texverb,
            "array index out of bounds", return);
    if (texverbs[index]) {
        free(texverbs[index]);
    }
    texverbs[index] = texverb;
}

ENUM_TO_STRING(gregorio_type, GREGORIO_TYPE)
ENUM_TO_STRING(gregorio_shape, GREGORIO_SHAPE)
ENUM_TO_STRING(gregorio_bar, GREGORIO_BAR)
ENUM_TO_STRING(gregorio_sign, GREGORIO_SIGN)
ENUM_TO_STRING(gregorio_space, GREGORIO_SPACE)
ENUM_TO_STRING(gregorio_liquescentia, GREGORIO_LIQUESCENTIA)
ENUM_TO_STRING(grehepisema_size, GREHEPISEMA_SIZE)
ENUM_TO_STRING(gregorio_vposition, GREGORIO_VPOSITION)
ENUM_TO_STRING(gregorio_glyph_type, GREGORIO_GLYPH_TYPE)
ENUM_TO_STRING(grestyle_style, GRESTYLE_STYLE)
ENUM_TO_STRING(gregorio_tr_centering, GREGORIO_TR_CENTERING)
ENUM_TO_STRING(gregorio_nlba, GREGORIO_NLBA)
ENUM_TO_STRING(gregorio_euouae, GREGORIO_EUOUAE)
ENUM_TO_STRING(gregorio_word_position, GREGORIO_WORD_POSITION)
ENUM_TO_STRING(gregorio_hepisema_vbasepos, GREGORIO_HEPISEMA_VBASEPOS)
