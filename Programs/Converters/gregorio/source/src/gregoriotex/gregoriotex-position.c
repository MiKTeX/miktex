/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file contains the logic for positioning signs on neumes.
 *
 * Copyright (C) 2008-2019 The Gregorio Project (see CONTRIBUTORS.md)
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
 * this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "config.h"
#include <stdlib.h>
#include <assert.h>
#include "bool.h"
#include "struct.h"

#include "gregoriotex.h"

/* (loose) naming convention, employing camel case to be TeX-csname-compliant:
 * {specific-glyph-shape}{note-position}{note-shape}{first-ambitus}{second-ambitus}
 */
OFFSET_CASE(FinalPunctum);
OFFSET_CASE(FinalDeminutus);
OFFSET_CASE(PenultBeforePunctumWide);
OFFSET_CASE(PenultBeforeDeminutus);
OFFSET_CASE(AntepenultBeforePunctum);
OFFSET_CASE(AntepenultBeforeDeminutus);
OFFSET_CASE(InitialPunctum);
OFFSET_CASE(InitioDebilis);
OFFSET_CASE(PorrNonAuctusInitialWide);
OFFSET_CASE(PorrNonAuctusInitialOne);
OFFSET_CASE(PorrAuctusInitialAny);
OFFSET_CASE(FinalInclinatum);
OFFSET_CASE(FinalInclinatumDeminutus);
OFFSET_CASE(FinalStropha);
OFFSET_CASE(FinalQuilisma);
OFFSET_CASE(FinalOriscus);
OFFSET_CASE(PenultBeforePunctumOne);
OFFSET_CASE(FinalUpperPunctum);
OFFSET_CASE(InitialOriscus);
OFFSET_CASE(InitialQuilisma);
OFFSET_CASE(TorcResNonAuctusSecondWideWide);
OFFSET_CASE(TorcResNonAuctusSecondOneWide);
OFFSET_CASE(TorcResDebilisNonAuctusSecondAnyWide);
OFFSET_CASE(FinalLineaPunctum);
OFFSET_CASE(TorcResQuilismaNonAuctusSecondWideWide);
OFFSET_CASE(TorcResOriscusNonAuctusSecondWideWide);
OFFSET_CASE(TorcResQuilismaNonAuctusSecondOneWide);
OFFSET_CASE(TorcResOriscusNonAuctusSecondOneWide);
OFFSET_CASE(TorcResNonAuctusSecondWideOne);
OFFSET_CASE(TorcResDebilisNonAuctusSecondAnyOne);
OFFSET_CASE(TorcResQuilismaNonAuctusSecondWideOne);
OFFSET_CASE(TorcResOriscusNonAuctusSecondWideOne);
OFFSET_CASE(TorcResNonAuctusSecondOneOne);
OFFSET_CASE(TorcResQuilismaNonAuctusSecondOneOne);
OFFSET_CASE(TorcResOriscusNonAuctusSecondOneOne);
OFFSET_CASE(TorcResAuctusSecondWideAny);
OFFSET_CASE(TorcResDebilisAuctusSecondAnyAny);
OFFSET_CASE(TorcResQuilismaAuctusSecondWideAny);
OFFSET_CASE(TorcResOriscusAuctusSecondWideAny);
OFFSET_CASE(TorcResAuctusSecondOneAny);
OFFSET_CASE(TorcResQuilismaAuctusSecondOneAny);
OFFSET_CASE(TorcResOriscusAuctusSecondOneAny);
OFFSET_CASE(ConnectedPenultBeforePunctumWide);
OFFSET_CASE(ConnectedPenultBeforePunctumOne);
OFFSET_CASE(InitialConnectedPunctum);
OFFSET_CASE(InitialConnectedVirga);
OFFSET_CASE(InitialConnectedQuilisma);
OFFSET_CASE(InitialConnectedOriscus);
OFFSET_CASE(FinalConnectedPunctum);
OFFSET_CASE(FinalConnectedAuctus);
OFFSET_CASE(FinalVirgaAuctus);
OFFSET_CASE(FinalConnectedVirga);
OFFSET_CASE(InitialVirga);
OFFSET_CASE(SalicusOriscusWide);
OFFSET_CASE(SalicusOriscusOne);
OFFSET_CASE(LeadingPunctum);
OFFSET_CASE(LeadingQuilisma);
OFFSET_CASE(LeadingOriscus);
OFFSET_CASE(Flat);
OFFSET_CASE(Sharp);
OFFSET_CASE(Natural);

static __inline const char *note_before_last_note_case_ignoring_deminutus(
        const gregorio_note *const current_note)
{
    assert(current_note->next);
    if ((current_note->u.note.pitch - current_note->next->u.note.pitch) == 1
            || (current_note->u.note.pitch -
                    current_note->next->u.note.pitch) == -1) {
        if (!current_note->previous || current_note->u.note.pitch -
                current_note->previous->u.note.pitch > 1) {
            return ConnectedPenultBeforePunctumOne;
        } else {
            return PenultBeforePunctumOne;
        }
    } else {
        if (!current_note->previous || current_note->u.note.pitch -
                current_note->previous->u.note.pitch > 1) {
            return ConnectedPenultBeforePunctumWide;
        } else {
            return PenultBeforePunctumWide;
        }
    }
}

static __inline const char *note_before_last_note_case(
        const gregorio_glyph *const current_glyph,
        const gregorio_note *const current_note)
{
    if ((current_glyph->u.notes.liquescentia & L_DEMINUTUS)
            && current_note->next) {
        return PenultBeforeDeminutus;
    } else {
        return note_before_last_note_case_ignoring_deminutus(current_note);
    }
}

/* num can be FinalPunctum or FinalUpperPunctum according if the last note is a
 * standard punctum or a smaller punctum (for pes, porrectus and torculus
 * resupinus */
static __inline const char *last_note_case(
        const gregorio_glyph *const current_glyph, const char* offset_pos,
        gregorio_note *current_note, bool no_ambitus_one)
{
    if (current_glyph->u.notes.liquescentia & L_DEMINUTUS) {
        /*
         * may seem strange, but it is unlogical to typeset a small horizontal
         * episema at the end of a flexus deminutus 
         */
        return FinalDeminutus;
    }
    if (!current_note->previous
            || (!no_ambitus_one && (offset_pos == FinalUpperPunctum
                    || current_note->previous->u.note.pitch -
            current_note->u.note.pitch == 1 ||
            current_note->u.note.pitch - current_note->previous->u.note.pitch
            == 1))) {
        return offset_pos;
    }
    if (current_note->previous->u.note.pitch < current_note->u.note.pitch) {
        if (current_glyph->u.notes.liquescentia &
                (L_AUCTUS_ASCENDENS | L_AUCTUS_DESCENDENS)) {
            return FinalConnectedAuctus;
        } else {
            return FinalConnectedVirga;
        }
    } else {
        return FinalConnectedPunctum;
    }
}

static __inline const char *first_note_case(
        const gregorio_note *const current_note,
        const gregorio_glyph *const current_glyph)
{
    bool ambitus_one = true;
    if (current_glyph->u.notes.liquescentia >= L_INITIO_DEBILIS) {
        return InitioDebilis;
    } else {
        if (current_note->next && current_note->next->u.note.pitch -
                current_note->u.note.pitch != 1 &&
                current_note->u.note.pitch - current_note->next->u.note.pitch
                != 1) {
            ambitus_one = false;
        }
        switch (current_note->u.note.shape) {
        case S_ORISCUS_ASCENDENS:
        case S_ORISCUS_DESCENDENS:
            return ambitus_one ? InitialOriscus : InitialConnectedOriscus;

        case S_QUILISMA:
            return ambitus_one ? InitialQuilisma : InitialConnectedQuilisma;

        default:
            if (ambitus_one) {
                return InitialPunctum;
            }
            if (current_note->u.note.pitch > current_note->next->u.note.pitch) {
                return InitialConnectedVirga;
            } else {
                return InitialConnectedPunctum;
            }
        }
    }
}

static __inline const char *fused_single_note_case(
        const gregorio_glyph *const glyph, const char *const unfused_case,
        const char *const fused_case) {
    int fuse_to_next_glyph = glyph->u.notes.fuse_to_next_glyph;
    if (fuse_to_next_glyph < -1 || fuse_to_next_glyph > 1) {
        return fused_case;
    }
    return unfused_case;
}

static __inline gregorio_vposition below_if_auctus(
        const gregorio_glyph *const glyph)
{
    if (glyph->u.notes.liquescentia &
            (L_AUCTUS_ASCENDENS | L_AUCTUS_DESCENDENS)) {
        return VPOS_BELOW;
    }
    return VPOS_ABOVE;
}

static __inline gregorio_vposition above_if_h_episema(
        const gregorio_note *const note)
{
    if (note && note->h_episema_above) {
        return VPOS_ABOVE;
    }
    return VPOS_BELOW;
}

static __inline gregorio_vposition above_if_fused_next_h_episema(
        const gregorio_glyph *glyph)
{
    const gregorio_glyph *next = gregorio_next_non_texverb_glyph(glyph);
    gregorio_assert(next && next->type == GRE_GLYPH
            && is_fused(next->u.notes.liquescentia),
            above_if_fused_next_h_episema,
            "expected this glyph to be fused to the next one",
            return VPOS_BELOW);
    return above_if_h_episema(next->u.notes.first_note);
}

static __inline gregorio_vposition above_if_either_h_episema(
        const gregorio_note *const note)
{
    if ((note->previous && note->previous->h_episema_above)
            || (note->next && note->next->h_episema_above)) {
        return VPOS_ABOVE;
    }
    return VPOS_BELOW;
}

static __inline gregorio_vposition below_if_next_ambitus_allows(
        const gregorio_note *const note)
{
    assert(note->next);

    if (!note->next || note->u.note.pitch - note->next->u.note.pitch > 2) {
        return VPOS_BELOW;
    }
    return VPOS_ABOVE;
}

static __inline void low_high_set_lower(const gregorio_glyph *const glyph,
        gregorio_note *const note)
{
    if ((glyph->u.notes.liquescentia & L_DEMINUTUS) ||
            !(glyph->u.notes.liquescentia
                & (L_AUCTUS_ASCENDENS | L_AUCTUS_DESCENDENS))) {
        note->is_lower_note = true;
    }
}

static __inline void low_high_set_upper(const gregorio_glyph *const glyph,
        gregorio_note *const note)
{
    if ((glyph->u.notes.liquescentia & L_DEMINUTUS) ||
            !(glyph->u.notes.liquescentia
                & (L_AUCTUS_ASCENDENS | L_AUCTUS_DESCENDENS))) {
        note->is_upper_note = true;
    }
}

static __inline void high_low_set_upper(const gregorio_glyph *const glyph,
        gregorio_note *const note)
{
    if (glyph->u.notes.liquescentia & L_DEMINUTUS) {
        note->is_upper_note = true;
    }
}

static __inline void high_low_set_lower(const gregorio_glyph *const glyph,
        gregorio_note *const note)
{
    if (glyph->u.notes.liquescentia & L_DEMINUTUS) {
        note->is_lower_note = true;
    }
}

/* a function that finds the good sign (additional line, vepisema or
 * hepisema) number, according to the gregoriotex convention (described in
 * gregoriotex.tex)
 * this function is REALLY a pain in the ass, but it is sadly necessary */
/*
 *
 * For the first note of a porrectus (flexus), this table summarizes the sign
 * number (amb2 is the ambitus after the diagonal stroke):
 *
 *  Porrectus       | Porrectus       | Porrectus    | Porrectus
 *  non-auctus      | non-auctus      | auctus       | Flexus
 *  amb2=1          | amb2>1          |              |
 * =================|=================|==============|==============
 *  PorrNonAuctus   | PorrNonAuctus   | PorrAuctus   | PorrAuctus
 *   ~InitialOne    |  ~InitialWide   |  ~InitialAny |  ~InitialAny
 *
 * For the second note of a torculus resupinus (flexus), this table summarizes
 * the number (amb1 is the ambitus before the diagonal stroke and amb2 is the
 * ambitus after the diagonal stroke):
 *
 *       |            | Torculus    | Torculus    | Torculus   | Torculus
 *       |            | Resupinus   | Resupinus   | Resupinus  | Resupinus
 *       | Initial    | non-auctus  | non-auctus  | auctus     | Flexus
 *  amb1 | Note       | amb2=1      | amb2>1      |            |
 * ======|============|=============|=============|============|============
 *  1    | Punctum    | TorcRes     | TorcRes     | TorcRes    | TorcRes
 *       |            |  ~NonAuctus |  ~NonAuctus |  ~Auctus   |  ~Auctus
 *       |            |  ~Second    |  ~Second    |  ~Second   |  ~Second
 *       |            |  ~OneOne    |  ~OneWide   |  ~OneAny   |  ~OneAny
 * ------+------------+-------------+-------------+------------+------------
 *  1    | I. Debilis | TorcRes     | TorcRes     | TorcRes    | TorcRes
 *       |            |  ~Debilis   |  ~Debilis   |  ~Debilis  |  ~Debilis
 *       |            |  ~NonAuctus |  ~NonAuctus |  ~Auctus   |  ~Auctus
 *       |            |  ~Second    |  ~Second    |  ~Second   |  ~Second
 *       |            |  ~AnyOne    |  ~AnyWide   |  ~AnyAny   |  ~AnyAny
 * ------+------------+-------------+-------------+------------+------------
 *  1    | Quilisma   | TorcRes     | TorcRes     | TorcRes    | TorcRes
 *       |            |  ~Quilisma  |  ~Quilisma  |  ~Quilisma |  ~Quilisma
 *       |            |  ~NonAuctus |  ~NonAuctus |  ~Auctus   |  ~Auctus
 *       |            |  ~Second    |  ~Second    |  ~Second   |  ~Second
 *       |            |  ~OneOne    |  ~OneWide   |  ~OneAny   |  ~OneAny
 * ------+------------+-------------+-------------+------------+------------
 *  1    | Oriscus    | TorcRes     | TorcRes     | TorcRes    | TorcRes
 *       |            |  ~Oriscus   |  ~Oriscus   |  ~Oriscus  |  ~Oriscus
 *       |            |  ~NonAuctus |  ~NonAuctus |  ~Auctus   |  ~Auctus
 *       |            |  ~Second    |  ~Second    |  ~Second   |  ~Second
 *       |            |  ~OneOne    |  ~OneWide   |  ~OneAny   |  ~OneAny
 * ======|============|=============|=============|============|============
 *  >1   | Punctum    | TorcRes     | TorcRes     | TorcRes    | TorcRes
 *       |            |  ~NonAuctus |  ~NonAuctus |  ~Auctus   |  ~Auctus
 *       |            |  ~Second    |  ~Second    |  ~Second   |  ~Second
 *       |            |  ~WideOne   |  ~WideWide  |  ~WideAny  |  ~WideAny
 * ------+------------+-------------+-------------+------------+------------
 *  >1   | I. Debilis | TorcRes     | TorcRes     | TorcRes    | TorcRes
 *       |            |  ~Debilis   |  ~Debilis   |  ~Debilis  |  ~Debilis
 *       |            |  ~NonAuctus |  ~NonAuctus |  ~Auctus   |  ~Auctus
 *       |            |  ~Second    |  ~Second    |  ~Second   |  ~Second
 *       |            |  ~AnyOne    |  ~AnyWide   |  ~AnyAny   |  ~AnyAny
 * ------+------------+-------------+-------------+------------+------------
 *  >1   | Quilisma   | TorcRes     | TorcRes     | TorcRes    | TorcRes
 *       |            |  ~Quilisma  |  ~Quilisma  |  ~Quilisma |  ~Quilisma
 *       |            |  ~NonAuctus |  ~NonAuctus |  ~Auctus   |  ~Auctus
 *       |            |  ~Second    |  ~Second    |  ~Second   |  ~Second
 *       |            |  ~WideOne   |  ~WideWide  |  ~WideAny  |  ~WideAny
 * ------+------------+-------------+-------------+------------+------------
 *  >1   | Oriscus    | TorcRes     | TorcRes     | TorcRes    | TorcRes
 *       |            |  ~Oriscus   |  ~Oriscus   |  ~Oriscus  |  ~Oriscus
 *       |            |  ~NonAuctus |  ~NonAuctus |  ~Auctus   |  ~Auctus
 *       |            |  ~Second    |  ~Second    |  ~Second   |  ~Second
 *       |            |  ~WideOne   |  ~WideWide  |  ~WideAny  |  ~WideAny
 *
 */
static gregorio_vposition advise_positioning(const gregorio_glyph *const glyph,
        gregorio_note *const note, int i, const gtex_type type)
{
    gregorio_vposition h_episema = VPOS_AUTO, v_episema = VPOS_AUTO;
    bool v_episema_below_is_lower = false, done;
    const gregorio_glyph *previous;

    /* no need to clear is_lower_note/is_upper_note because we used calloc */

    switch (type) {
    case T_PES:
    case T_QUILISMA_PES:
        /*
         * in the case of a pes, we put the episema just under the bottom note 
         */
        if (i == 1) {
            if (glyph->u.notes.liquescentia & L_INITIO_DEBILIS) {
                note->gtex_offset_case = InitioDebilis;
                h_episema = above_if_h_episema(note->next);
            } else {
                note->is_lower_note = true;
                if (note->u.note.shape == S_QUILISMA) {
                    note->gtex_offset_case = FinalQuilisma;
                } else {
                    note->gtex_offset_case = FinalPunctum;
                }
                h_episema = VPOS_BELOW;
            }
            v_episema = VPOS_BELOW;
        } else { /* i=2  */
            if (!(glyph->u.notes.liquescentia & L_INITIO_DEBILIS)) {
                note->is_upper_note = true;
            }
            if (glyph->u.notes.liquescentia & L_DEMINUTUS) {
                note->gtex_offset_case = FinalDeminutus;
            } else {
                note->gtex_offset_case = FinalUpperPunctum;
            }
            h_episema = VPOS_ABOVE;
            v_episema = below_if_auctus(glyph);
        }
        break;
    case T_PES_QUADRATUM:
    case T_PES_QUASSUS:
    case T_QUILISMA_PES_QUADRATUM:
        if (i == 1) {
            note->gtex_offset_case = first_note_case(note, glyph);
            h_episema = above_if_h_episema(note->next);
            v_episema = VPOS_BELOW;
        } else { /* i=2 */
            if (glyph->u.notes.liquescentia & L_DEMINUTUS) {
                note->gtex_offset_case = InitioDebilis;
            } else {
                note->gtex_offset_case = last_note_case(glyph, FinalPunctum,
                        note, true);
            }
            h_episema = VPOS_ABOVE;
            v_episema = VPOS_BELOW;
        }
        break;
    case T_FLEXUS:
    case T_FLEXUS_ORISCUS:
        if (i == 1) {
            high_low_set_upper(glyph, note);
            if (type == T_FLEXUS_ORISCUS) {
                note->gtex_offset_case = InitialConnectedOriscus;
            } else {
                if (note->next && note->u.note.pitch - 
                        note->next->u.note.pitch == 1) {
                    note->gtex_offset_case = InitialVirga;
                } else {
                    note->gtex_offset_case = InitialConnectedVirga;
                }
            }
            h_episema = VPOS_ABOVE;
            if (glyph->u.notes.liquescentia & L_DEMINUTUS) {
                v_episema = below_if_next_ambitus_allows(note);
            } else {
                v_episema = VPOS_BELOW;
            }
        } else { /* i=2 */
            high_low_set_lower(glyph, note);
            note->gtex_offset_case = last_note_case(glyph, FinalPunctum, note,
                    false);
            if (glyph->u.notes.liquescentia & L_DEMINUTUS) {
                h_episema = VPOS_BELOW;
            } else {
                h_episema = above_if_h_episema(note->previous);
            }
            v_episema = VPOS_BELOW;
        }
        break;
    case T_TORCULUS_RESUPINUS_FLEXUS:
        done = true;
        switch (i) {
        case 1:
            note->gtex_offset_case = first_note_case(note, glyph);
            h_episema = above_if_h_episema(note->next);
            v_episema = VPOS_BELOW;
            break;
        case HEPISEMA_FIRST_TWO:
        case 2:
            if (glyph->u.notes.liquescentia & L_INITIO_DEBILIS) {
                note->gtex_offset_case = TorcResDebilisAuctusSecondAnyAny;
            } else {
                if (note->u.note.pitch - note->previous->u.note.pitch == 1) {
                    switch (note->previous->u.note.shape) {
                    case S_QUILISMA:
                        note->gtex_offset_case =
                                TorcResQuilismaAuctusSecondOneAny;
                        break;
                    case S_ORISCUS_ASCENDENS:
                    case S_ORISCUS_DESCENDENS:
                        note->gtex_offset_case =
                                TorcResOriscusAuctusSecondOneAny;
                        break;
                    default:
                        note->gtex_offset_case = TorcResAuctusSecondOneAny;
                        break;
                    }
                } else {
                    switch (note->previous->u.note.shape) {
                    case S_QUILISMA:
                        note->gtex_offset_case =
                                TorcResQuilismaAuctusSecondWideAny;
                        break;
                    case S_ORISCUS_ASCENDENS:
                    case S_ORISCUS_DESCENDENS:
                        note->gtex_offset_case =
                                TorcResOriscusAuctusSecondWideAny;
                        break;
                    default:
                        note->gtex_offset_case = TorcResAuctusSecondWideAny;
                        break;
                    }
                }
            }
            h_episema = VPOS_ABOVE;
            v_episema = VPOS_BELOW;
            v_episema_below_is_lower = true;
            break;
        default:
            --i;
            done = false;
            break;
        }

        if (done) {
            break;
        }
        /* else fallthrough to the next case! */
    case T_PORRECTUS_FLEXUS:
        switch (i) {
        case HEPISEMA_FIRST_TWO:
        case 1:
            note->gtex_offset_case = PorrAuctusInitialAny;
            h_episema = VPOS_ABOVE;
            v_episema = VPOS_BELOW;
            v_episema_below_is_lower = true;
            break;
        case 2:
            if (glyph->u.notes.liquescentia & L_DEMINUTUS) {
                note->gtex_offset_case = AntepenultBeforeDeminutus;
            } else {
                note->gtex_offset_case = AntepenultBeforePunctum;
            }
            h_episema = above_if_either_h_episema(note);
            v_episema = VPOS_BELOW;
            break;
        case 3:
            high_low_set_upper(glyph, note);
            h_episema = VPOS_ABOVE;
            if ((glyph->u.notes.liquescentia & L_DEMINUTUS) && note->next) {
                v_episema = below_if_next_ambitus_allows(note);
                note->gtex_offset_case = PenultBeforeDeminutus;
            } else {
                v_episema = VPOS_BELOW;
                note->gtex_offset_case = note_before_last_note_case(glyph,
                        note);
            }
            break;
        default:
            high_low_set_lower(glyph, note);
            note->gtex_offset_case = last_note_case(glyph, FinalPunctum, note,
                    false);
            h_episema = above_if_h_episema(note->previous);
            v_episema = VPOS_BELOW;
            break;
        }
        break;
    case T_TORCULUS_RESUPINUS:
        done = true;
        switch (i) {
        case 1:
            note->gtex_offset_case = first_note_case(note, glyph);
            h_episema = above_if_h_episema(note->next);
            v_episema = VPOS_BELOW;
            break;
        case HEPISEMA_FIRST_TWO:
        case 2:
            if (glyph->u.notes.liquescentia &
                    (L_AUCTUS_ASCENDENS | L_AUCTUS_DESCENDENS)) {
                /* auctus */
                if (glyph->u.notes.liquescentia & L_INITIO_DEBILIS) {
                    note->gtex_offset_case = TorcResDebilisAuctusSecondAnyAny;
                } else {
                    if (note->u.note.pitch -
                            note->previous->u.note.pitch == 1) {
                        switch (note->previous->u.note.shape) {
                        case S_QUILISMA:
                            note->gtex_offset_case =
                                    TorcResQuilismaAuctusSecondOneAny;
                            break;
                        case S_ORISCUS_ASCENDENS:
                        case S_ORISCUS_DESCENDENS:
                            note->gtex_offset_case =
                                    TorcResOriscusAuctusSecondOneAny;
                            break;
                        default:
                            note->gtex_offset_case = TorcResAuctusSecondOneAny;
                            break;
                        }
                    } else {
                        switch (note->previous->u.note.shape) {
                        case S_QUILISMA:
                            note->gtex_offset_case =
                                    TorcResQuilismaAuctusSecondWideAny;
                            break;
                        case S_ORISCUS_ASCENDENS:
                        case S_ORISCUS_DESCENDENS:
                            note->gtex_offset_case =
                                    TorcResOriscusAuctusSecondWideAny;
                            break;
                        default:
                            note->gtex_offset_case = TorcResAuctusSecondWideAny;
                            break;
                        }
                    }
                }
            } else if (note->next->next->u.note.pitch -
                    note->next->u.note.pitch == 1) {
                /* non-auctus with a second ambitus of 1 */
                if (glyph->u.notes.liquescentia & L_INITIO_DEBILIS) {
                    note->gtex_offset_case =
                            TorcResDebilisNonAuctusSecondAnyOne;
                } else {
                    if (note->u.note.pitch -
                            note->previous->u.note.pitch == 1) {
                        switch (note->previous->u.note.shape) {
                        case S_QUILISMA:
                            note->gtex_offset_case =
                                    TorcResQuilismaNonAuctusSecondOneOne;
                            break;
                        case S_ORISCUS_ASCENDENS:
                        case S_ORISCUS_DESCENDENS:
                            note->gtex_offset_case =
                                    TorcResOriscusNonAuctusSecondOneOne;
                            break;
                        default:
                            note->gtex_offset_case =
                                    TorcResNonAuctusSecondOneOne;
                            break;
                        }
                    } else {
                        switch (note->previous->u.note.shape) {
                        case S_QUILISMA:
                            note->gtex_offset_case =
                                    TorcResQuilismaNonAuctusSecondWideOne;
                            break;
                        case S_ORISCUS_ASCENDENS:
                        case S_ORISCUS_DESCENDENS:
                            note->gtex_offset_case =
                                    TorcResOriscusNonAuctusSecondWideOne;
                            break;
                        default:
                            note->gtex_offset_case =
                                    TorcResNonAuctusSecondWideOne;
                            break;
                        }
                    }
                }
            } else {
                /* non-auctus with a second ambitus of at least 2 */
                if (glyph->u.notes.liquescentia & L_INITIO_DEBILIS) {
                    note->gtex_offset_case =
                            TorcResDebilisNonAuctusSecondAnyWide;
                } else {
                    if (note->u.note.pitch -
                            note->previous->u.note.pitch == 1) {
                        switch (note->previous->u.note.shape) {
                        case S_QUILISMA:
                            note->gtex_offset_case =
                                    TorcResQuilismaNonAuctusSecondOneWide;
                            break;
                        case S_ORISCUS_ASCENDENS:
                        case S_ORISCUS_DESCENDENS:
                            note->gtex_offset_case =
                                    TorcResOriscusNonAuctusSecondOneWide;
                            break;
                        default:
                            note->gtex_offset_case =
                                    TorcResNonAuctusSecondOneWide;
                            break;
                        }
                    } else {
                        switch (note->previous->u.note.shape) {
                        case S_QUILISMA:
                            note->gtex_offset_case =
                                    TorcResQuilismaNonAuctusSecondWideWide;
                            break;
                        case S_ORISCUS_ASCENDENS:
                        case S_ORISCUS_DESCENDENS:
                            note->gtex_offset_case =
                                    TorcResOriscusNonAuctusSecondWideWide;
                            break;
                        default:
                            note->gtex_offset_case =
                                    TorcResNonAuctusSecondWideWide;
                            break;
                        }
                    }
                }
            }
            h_episema = VPOS_ABOVE;
            v_episema = VPOS_BELOW;
            v_episema_below_is_lower = true;
            break;
        default:
            --i;
            done = false;
            break;
        }

        if (done) {
            break;
        }
        /* else fallthrough to the next case! */
    case T_PORRECTUS: /* or porrectus-like flexus */
        switch (i) {
        case HEPISEMA_FIRST_TWO:
        case 1:
            if (glyph->u.notes.fuse_to_next_glyph ||
                    (glyph->u.notes.liquescentia
                        & (L_AUCTUS_ASCENDENS | L_AUCTUS_DESCENDENS))) {
                /* auctus */
                note->gtex_offset_case = PorrAuctusInitialAny;
            } else if (note->next->next->u.note.pitch
                    - note->next->u.note.pitch == 1) {
                /* non-auctus with a second ambitus of 1 */
                note->gtex_offset_case = PorrNonAuctusInitialOne;
            } else {
                /* non-auctus with a second ambitus of at least 2 */
                note->gtex_offset_case = PorrNonAuctusInitialWide;
            }
            h_episema = VPOS_ABOVE;
            v_episema = VPOS_BELOW;
            v_episema_below_is_lower = true;
            break;
        case 2:
            low_high_set_lower(glyph, note);
            if (glyph->u.notes.liquescentia & L_DEMINUTUS) {
                note->gtex_offset_case = PenultBeforeDeminutus;
            } else if (glyph->u.notes.liquescentia
                        & (L_AUCTUS_ASCENDENS | L_AUCTUS_DESCENDENS)) {
                note->gtex_offset_case = PenultBeforePunctumWide;
            } else {
                note->gtex_offset_case = FinalPunctum;
            }
            h_episema = above_if_h_episema(note->previous);
            if (h_episema == VPOS_BELOW && !note->next) {
                /* handle porrectus-like flexus */
                h_episema = above_if_fused_next_h_episema(glyph);
            }
            v_episema = VPOS_BELOW;
            break;
        default: /* case 3 */
            low_high_set_upper(glyph, note);
            note->gtex_offset_case = last_note_case(glyph, FinalUpperPunctum,
                    note, true);
            h_episema = VPOS_ABOVE;
            v_episema = below_if_auctus(glyph);
            break;
        }
        break;
    case T_SCANDICUS:
        switch (i) {
        case 1:
            note->gtex_offset_case = first_note_case(note, glyph);
            h_episema = VPOS_BELOW;
            v_episema = VPOS_BELOW;
            break;
        case 2:
            low_high_set_lower(glyph, note);
            if (note->u.note.pitch - note->previous->u.note.pitch == 1) {
                note->gtex_offset_case = FinalPunctum;
            } else {
                note->gtex_offset_case = FinalConnectedPunctum;
            }
            h_episema = VPOS_BELOW;
            v_episema = VPOS_BELOW;
            break;
        default:
            low_high_set_upper(glyph, note);
            note->gtex_offset_case = last_note_case(glyph, FinalPunctum, note,
                    true);
            h_episema = VPOS_ABOVE;
            v_episema = VPOS_BELOW;
            break;
        }
        break;
    case T_SALICUS_FLEXUS:
        done = true;
        v_episema = VPOS_BELOW;
        switch (i) {
        case 3:
            note->gtex_offset_case = note_before_last_note_case(glyph, note);
            h_episema = VPOS_ABOVE;
            break;
        case 4:
            note->gtex_offset_case = last_note_case(glyph, FinalPunctum, note,
                    false);
            h_episema = VPOS_BELOW;
            break;
        default:
            done = false;
            break;
        }
        if (done) {
            break;
        }
        /* else fallthrough to the next case! */
    case T_SALICUS:
        v_episema = VPOS_BELOW;
        switch (i) {
        case 1:
            note->gtex_offset_case = InitialConnectedPunctum;
            h_episema = above_if_h_episema(note->next);
            break;
        case 2:
            if (note->next->u.note.pitch - note->u.note.pitch == 1) {
                note->gtex_offset_case = SalicusOriscusOne;
            } else {
                note->gtex_offset_case = SalicusOriscusWide;
            }
            h_episema = VPOS_ABOVE;
            break;
        default:
            note->gtex_offset_case = last_note_case(glyph, FinalPunctum, note,
                    false);
            if (glyph->u.notes.liquescentia & L_DEMINUTUS) {
                v_episema = VPOS_ABOVE;
            }
            h_episema = VPOS_ABOVE;
            break;
        }
        break;
    case T_TORCULUS_LIQUESCENS:
        done = true;
        switch (i) {
        case 1:
            note->gtex_offset_case = first_note_case(note, glyph);
            h_episema = above_if_h_episema(note->next);
            v_episema = VPOS_BELOW;
            break;
        case 2:
            note->gtex_offset_case =
                    note_before_last_note_case_ignoring_deminutus(note);
            h_episema = VPOS_ABOVE;
            v_episema = VPOS_BELOW;
            break;
        default:
            --i;
            done = false;
            break;
        }

        if (done) {
            break;
        }
        /* else fallthrough to the next case! */
    case T_ANCUS:
        switch (i) {
        case 1:
            note->gtex_offset_case = first_note_case(note, glyph);
            h_episema = above_if_h_episema(note->next);
            v_episema = VPOS_BELOW;
            break;
        case 2:
            note->is_upper_note = true;
            note->gtex_offset_case = note_before_last_note_case(glyph, note);
            h_episema = VPOS_ABOVE;
            v_episema = below_if_next_ambitus_allows(note);
            break;
        default:
            note->is_lower_note = true;
            note->gtex_offset_case = last_note_case(glyph, FinalPunctum, note,
                    false);
            h_episema = VPOS_BELOW;
            v_episema = VPOS_BELOW;
            break;
        }
        break;
    case T_TORCULUS:
        switch (i) {
        case 1:
            note->gtex_offset_case = first_note_case(note, glyph);
            h_episema = above_if_h_episema(note->next);
            v_episema = VPOS_BELOW;
            break;
        case 2:
            high_low_set_upper(glyph, note);
            note->gtex_offset_case = note_before_last_note_case(glyph, note);
            h_episema = VPOS_ABOVE;
            if (glyph->u.notes.liquescentia & L_DEMINUTUS) {
                v_episema = below_if_next_ambitus_allows(note);
            } else {
                v_episema = VPOS_BELOW;
            }
            break;
        default:
            high_low_set_lower(glyph, note);
            note->gtex_offset_case = last_note_case(glyph, FinalPunctum, note,
                    false);
            h_episema = above_if_h_episema(note->previous);
            v_episema = VPOS_BELOW;
            break;
        }
        break;
    default: /* case T_ONE_NOTE */
        previous = gregorio_previous_non_texverb_glyph(glyph);
        if ((!previous || previous->type != GRE_GLYPH
                    || previous->u.notes.fuse_to_next_glyph < 0)
                && (glyph->u.notes.fuse_to_next_glyph > 0)) {
            h_episema = above_if_fused_next_h_episema(glyph);
        } else {
            h_episema = VPOS_ABOVE;
        }
        v_episema = VPOS_BELOW;
        switch (note->u.note.shape) {
        case S_PUNCTUM_INCLINATUM_DEMINUTUS:
            note->gtex_offset_case = FinalInclinatumDeminutus;
            break;
        case S_PUNCTUM_INCLINATUM_AUCTUS:
        case S_PUNCTUM_INCLINATUM_ASCENDENS:
        case S_PUNCTUM_INCLINATUM_DESCENDENS:
            note->gtex_offset_case = FinalInclinatum;
            break;
        case S_STROPHA:
            note->gtex_offset_case = FinalStropha;
            break;
        case S_QUILISMA:
            note->gtex_offset_case = fused_single_note_case(glyph,
                    FinalQuilisma, LeadingQuilisma);
            break;
        case S_ORISCUS_ASCENDENS:
        case S_ORISCUS_DESCENDENS:
        case S_ORISCUS_DEMINUTUS:
            note->gtex_offset_case = fused_single_note_case(glyph, FinalOriscus,
                    LeadingOriscus);
            break;
        case S_VIRGA:
            note->gtex_offset_case = InitialVirga;
            break;
        case S_VIRGA_REVERSA:
            if (glyph->u.notes.liquescentia == L_AUCTUS_DESCENDENS) {
                note->gtex_offset_case = FinalVirgaAuctus;
            } else {
                note->gtex_offset_case = InitialVirga;
            }
            break;
        case S_LINEA_PUNCTUM:
            note->gtex_offset_case = FinalLineaPunctum;
            break;
        case S_LINEA:
            note->gtex_offset_case = FinalPunctum;
            break;
        case S_FLAT:
            note->gtex_offset_case = Flat;
            break;
        case S_SHARP:
            note->gtex_offset_case = Sharp;
            break;
        case S_NATURAL:
            note->gtex_offset_case = Natural;
            break;
        default:
            note->gtex_offset_case = last_note_case(glyph,
                    fused_single_note_case(glyph, FinalPunctum, LeadingPunctum),
                    note, true);
            break;
        }
        break;
    }

    if (note->signs & _V_EPISEMA) {
        if (note->v_episema_height) {
            if (note->v_episema_height > note->u.note.pitch) {
                /* above is always higher because of GregorioTeX's design */
                note->v_episema_height += (int)VPOS_ABOVE;
            }
        }
        else {
            note->v_episema_height = note->u.note.pitch + (int)v_episema;
            if ((v_episema == VPOS_BELOW && v_episema_below_is_lower)
                    || v_episema == VPOS_ABOVE) {
                /* above is always higher because of GregorioTeX's design */
                note->v_episema_height += (int)v_episema;
            }
        }
    }

    return h_episema;
}

static __inline char compute_h_episema_height(
        const gregorio_glyph *const glyph, const gregorio_note *const note,
        const gregorio_vposition vpos)
{
    char height = note->u.note.pitch;

    if (note->signs & _V_EPISEMA) {
        if ((vpos == VPOS_ABOVE && note->v_episema_height >= height)
                || (vpos == VPOS_BELOW && note->v_episema_height <= height)) {
            height = note->v_episema_height;
        }
    }

    if (vpos == VPOS_ABOVE && note->choral_sign
            && !choral_sign_here_is_low(glyph, note, NULL)) {
        ++height;
        if (!is_on_a_line(note->u.note.pitch)) {
            ++height;
        }
    }

    return height + (int)vpos;
}

static bool is_bridgeable_space(const gregorio_element *const element)
{
    if (element->type == GRE_SPACE) {
        switch (element->u.misc.unpitched.info.space) {
        case SP_NEUMATIC_CUT:
        case SP_LARGER_SPACE:
        case SP_NEUMATIC_CUT_NB:
        case SP_LARGER_SPACE_NB:
            return true;
        default:
            /* do nothing */
            break;
        }
    }

    return false;
}

typedef struct height_computation {
    const gregorio_vposition vpos;
    const gregorio_sign_orientation orientation;
    bool (*const is_applicable)(const gregorio_note *);
    bool (*const is_shown)(const gregorio_note *);
    bool (*const is_connected)(const gregorio_note *);
    grehepisema_size (*const get_size)(const gregorio_note *);
    bool (*const is_better_height)(signed char, signed char);
    void (*const position)(gregorio_note *, signed char, bool);

    bool active;
    char height;
    bool connected;
    const gregorio_element *start_element;
    const gregorio_glyph *start_glyph;
    gregorio_note *start_note;
    const gregorio_element *last_connected_element;
    const gregorio_glyph *last_connected_glyph;
    gregorio_note *last_connected_note;
    unsigned short adjustment_index;
} height_computation;

static bool is_h_episema_above_applicable(const gregorio_note *const note)
{
    return note->h_episema_above || !note->is_lower_note;
}

static bool is_h_episema_below_applicable(const gregorio_note *const note)
{
    return note->h_episema_below || !note->is_upper_note;
}

bool gtex_is_h_episema_above_shown(const gregorio_note *const note)
{
    return note->h_episema_above;
}

bool gtex_is_h_episema_below_shown(const gregorio_note *const note)
{
    return note->h_episema_below;
}

static bool is_h_episema_above_connected(const gregorio_note *const note)
{
    return note->h_episema_above_connect;
}

static bool is_h_episema_below_connected(const gregorio_note *const note)
{
    return note->h_episema_below_connect;
}

static grehepisema_size get_h_episema_above_size(
        const gregorio_note *const note)
{
    return note->h_episema_above_size;
}

static grehepisema_size get_h_episema_below_size(
        const gregorio_note *const note)
{
    return note->h_episema_below_size;
}

static bool is_h_episema_above_better_height(const signed char new_height,
        const signed char old_height)
{
    return new_height > old_height;
}

static bool is_h_episema_below_better_height(const signed char new_height,
        const signed char old_height)
{
    return new_height < old_height;
}

static void adjust_h_episema_above_if_better(gregorio_note *const note,
        signed char potential_height)
{
    if (is_h_episema_above_better_height(potential_height,
                note->h_episema_above)) {
        note->h_episema_above = potential_height;
    }
}

static void adjust_h_episema_below_if_better(gregorio_note *const note,
        signed char potential_height)
{
    if (is_h_episema_below_better_height(potential_height,
                note->h_episema_below)) {
        note->h_episema_below = potential_height;
    }
}

static __inline bool has_high_ledger_line(const signed char height,
        bool is_sign, const gregorio_score *const score)
{
    if (is_sign) {
        return height > score->high_ledger_line_pitch;
    }
    return height >= score->high_ledger_line_pitch;
}

static __inline bool has_low_ledger_line(const signed char height, bool is_sign)
{
    if (is_sign) {
        return height < LOW_LEDGER_LINE_PITCH;
    }
    return height <= LOW_LEDGER_LINE_PITCH;
}

static __inline void start_h_episema(height_computation *const h,
        const gregorio_element *const element,
        const gregorio_glyph *const glyph, gregorio_note *const note)
{
    h->start_element = element;
    h->start_glyph = glyph;
    h->start_note = note;
    h->active = true;
    h->height = compute_h_episema_height(glyph, note, h->vpos);
}

static __inline void position_h_episema(gregorio_note *const note,
        const height_computation *const h, const bool connect,
        const bool high_ledger_line, const bool low_ledger_line)
{
    const unsigned short adjustment_index =
            note->he_adjustment_index[h->orientation];

    h->position(note, h->height, connect);
    if (adjustment_index) {
        gregorio_hepisema_adjustment *adj = gregorio_get_hepisema_adjustment(
                adjustment_index);
        if (adj->pitch_extremum == NO_PITCH
                || h->is_better_height(h->height, adj->pitch_extremum)) {
            adj->pitch_extremum = h->height;
        }
    }
    if (!note->high_ledger_specificity && !note->high_ledger_line) {
        note->high_ledger_line = high_ledger_line;
    }
    if (!note->low_ledger_specificity && !note->low_ledger_line) {
        note->low_ledger_line = low_ledger_line;
    }
}

static __inline void next_has_ledger_line(
        const height_computation *const h, bool *high_ledger_line,
        bool *low_ledger_line)
{
    const gregorio_element *element = h->last_connected_element;
    const gregorio_glyph *glyph = h->last_connected_glyph;
    gregorio_note *note = h->last_connected_note;
    bool eat_upper_note = note->is_lower_note, keep_going = false;

    if (!*high_ledger_line || !*low_ledger_line) {
        /* going forwards, we must iterate through any lower notes */
        do {
            note = note->next;
            if (!note) {
                do {
                    glyph = glyph->next;
                    if (!glyph) {
                        do {
                            element = element->next;
                            if (!element) {
                                return;
                            }
                        } while (element->type != GRE_ELEMENT);
                        glyph = element->u.first_glyph;
                    }
                } while (glyph->type != GRE_GLYPH);
                note = glyph->u.notes.first_note;
            }

            *high_ledger_line = *high_ledger_line || note->high_ledger_line;
            *low_ledger_line = *low_ledger_line || note->low_ledger_line;

            if (keep_going) {
                keep_going = false;
            }
            if (eat_upper_note && note->is_upper_note) {
                eat_upper_note = false;
                keep_going = true;
            }
        } while ((!*high_ledger_line || !*low_ledger_line)
                && (note->is_lower_note || keep_going));
    }
}

static __inline void previous_has_ledger_line(
        const height_computation *const h, bool *high_ledger_line,
        bool *low_ledger_line)
{
    const gregorio_element *element = h->start_element;
    const gregorio_glyph *glyph = h->start_glyph;
    gregorio_note *note = h->start_note;
    bool eat_lower_note = note->is_upper_note, keep_going = false;

    if (!*high_ledger_line || !*low_ledger_line) {
        /* going backwards, we must iterate through any upper notes */
        do {
            note = note->previous;
            if (!note) {
                do {
                    glyph = glyph->previous;
                    if (!glyph) {
                        do {
                            element = element->previous;
                            if (!element) {
                                return;
                            }
                        } while (element->type != GRE_ELEMENT);
                        for (glyph = element->u.first_glyph; glyph->next;
                                glyph = glyph->next) {
                            /* just iterate to find the last glyph */
                        }
                    }
                } while (glyph->type != GRE_GLYPH);
                note = gregorio_glyph_last_note(glyph);
            }
            *high_ledger_line = *high_ledger_line || note->high_ledger_line;
            *low_ledger_line = *low_ledger_line || note->low_ledger_line;

            if (keep_going) {
                keep_going = false;
            }
            if (eat_lower_note && note->is_lower_note) {
                eat_lower_note = false;
                keep_going = true;
            }
        } while ((!*high_ledger_line || !*low_ledger_line)
                && (note->is_upper_note || keep_going));
    }
}

static __inline void set_h_episema_height(const height_computation *const h,
        gregorio_note *const end, const gregorio_score *const score)
{
    gregorio_note *last_note = NULL;

    const gregorio_element *element = h->start_element;
    const gregorio_glyph *glyph = h->start_glyph;
    gregorio_note *note = h->start_note;

    bool high_ledger_line = has_high_ledger_line(h->height, true, score)
            || has_high_ledger_line(h->height - h->vpos, false, score);
    bool low_ledger_line = has_low_ledger_line(h->height, true)
            || has_low_ledger_line(h->height - h->vpos, false);

    next_has_ledger_line(h, &high_ledger_line, &low_ledger_line);
    previous_has_ledger_line(h, &high_ledger_line, &low_ledger_line);

    for ( ; element; element = element->next) {
        if (element->type == GRE_ELEMENT) {
            for (glyph = glyph? glyph : element->u.first_glyph; glyph;
                    glyph = glyph->next) {
                if (glyph && glyph->type == GRE_GLYPH) {
                    for (note = note? note : glyph->u.notes.first_note; note;
                            note = note->next) {
                        if (end && note == end) {
                            if (last_note) {
                                position_h_episema(last_note, h, false,
                                        high_ledger_line, low_ledger_line);
                            }
                            return;
                        }
                        if (h->is_applicable(note)) {
                            position_h_episema(note, h, true, high_ledger_line,
                                    low_ledger_line);
                            last_note = note;
                        }
                    }
                }
            }
        } else if (!is_bridgeable_space(element)) {
            break;
        }
    }

    if (last_note) {
        position_h_episema(last_note, h, false, high_ledger_line,
                low_ledger_line);
    }
}

static __inline bool is_connected_left(const grehepisema_size size) {
    return size == H_NORMAL || size == H_SMALL_LEFT;
}

static __inline bool is_connected_right(const grehepisema_size size) {
    return size == H_NORMAL || size == H_SMALL_RIGHT;
}

static __inline bool is_connectable_interglyph_ambitus(const signed char first,
        const signed char second)
{
    return abs(first - second) < 3;
}

static __inline bool is_connectable_interglyph_notes(
        const gregorio_note *const first, const gregorio_note *const second)
{
    return first && second && is_connectable_interglyph_ambitus(
            first->u.note.pitch, second->u.note.pitch);
}

static __inline bool has_space_to_left(const gregorio_note *const note) {
    switch (note->u.note.shape) {
    case S_PUNCTUM_INCLINATUM_ASCENDENS:
    case S_PUNCTUM_INCLINATUM_DESCENDENS:
    case S_PUNCTUM_INCLINATUM_DEMINUTUS:
    case S_PUNCTUM_INCLINATUM_AUCTUS:
        return !is_connectable_interglyph_notes(note->previous, note);

    default:
        return !note->previous;
    }
}

static __inline void end_h_episema(height_computation *const h,
        gregorio_note *const end, const gregorio_score *const score)
{
    signed char proposed_height;

    if (h->active) {
        /* don't let the episema clash with the note before or after */
        if (is_connected_left(h->get_size(h->start_note))) {
            if (h->start_note->previous) {
                if (!has_space_to_left(h->start_note)
                        && h->start_note->previous->type == GRE_NOTE
                        && is_connected_right(h->get_size(
                                h->start_note->previous))) {
                    proposed_height = h->start_note->previous->u.note.pitch + h->vpos;
                    if (h->is_better_height(proposed_height, h->height)) {
                        h->height = proposed_height;
                    }
                }
            } else {
                const gregorio_glyph *previous =
                        gregorio_previous_non_texverb_glyph(h->start_glyph);
                if (previous && previous->type == GRE_GLYPH
                        && is_fused(h->start_glyph->u.notes.liquescentia)) {
                    gregorio_note *note = gregorio_glyph_last_note(
                            h->start_glyph->previous);
                    if (note->type == GRE_NOTE
                            && is_connected_right(h->get_size(note))) {
                        proposed_height = note->u.note.pitch + h->vpos;
                        if (h->is_better_height(proposed_height, h->height)) {
                            h->height = proposed_height;
                        }
                    }
                }
            }
        }
        if (end && end->type == GRE_NOTE && is_connected_left(h->get_size(end))
                && !(end->u.note.liquescentia & L_DEMINUTUS)
                && h->last_connected_note
                && is_connected_right(h->get_size(h->last_connected_note))) {
            if (h->last_connected_note->next) {
                if (h->last_connected_note->next->type == GRE_NOTE
                        && !has_space_to_left(h->last_connected_note->next)) {
                    gregorio_note *note;
                    /* this loop checks that it's within the same glyph */
                    for (note = end->previous; note; note = note->previous) {
                        if (note == h->start_note) {
                            proposed_height = end->u.note.pitch + h->vpos;
                            if (h->is_better_height(proposed_height,
                                        h->height)) {
                                h->height = proposed_height;
                            }
                            break;
                        }
                    }
                }
            } else if (is_fused(end->u.note.liquescentia) && !end->previous) {
                proposed_height = end->u.note.pitch + h->vpos;
                if (h->is_better_height(proposed_height, h->height)) {
                    h->height = proposed_height;
                }
            }
        }

        set_h_episema_height(h, end, score);

        h->active = false;
        h->height = 0;
        h->connected = false;
        h->start_element = NULL;
        h->start_glyph = NULL;
        h->start_note = NULL;
        h->last_connected_element = NULL;
        h->last_connected_glyph = NULL;
        h->last_connected_note = NULL;
    }
}

static __inline void compute_h_episema(height_computation *const h,
        const gregorio_element *const element,
        const gregorio_glyph *const glyph, gregorio_note *const note,
        const int i, const gtex_type type, const gregorio_score *const score)
{
    grehepisema_size size;

    if (h->is_applicable(note)) {
        if (h->is_shown(note)) {
            size = h->get_size(note);

            if (h->active) {
                const signed char next_height = compute_h_episema_height(
                        glyph, note, h->vpos);
                if (h->connected && is_connected_left(size)
                        && (i != 1 || is_connectable_interglyph_ambitus(
                                next_height, h->height))) {
                    if (h->is_better_height(next_height, h->height)) {
                        h->height = next_height;
                    }
                }
                else {
                    end_h_episema(h, note, score);
                    start_h_episema(h, element, glyph, note);
                }
            } else {
                start_h_episema(h, element, glyph, note);
            }

            h->connected = h->is_connected(note) && is_connected_right(size);
            h->last_connected_element = element;
            h->last_connected_glyph = glyph;
            h->last_connected_note = note;
        } else {
            end_h_episema(h, note, score);
        }
    } else if (!h->is_shown(note)) {
        /* special handling for porrectus shapes because of their shape:
         * the lower note of the porrectus stroke is normally not applicable,
         * but we have to end the episema on the upper note if the episema
         * on the lower note is not shown. */
        switch(type) {
        case T_PORRECTUS:
        case T_PORRECTUS_FLEXUS:
            if (i == 2) {
                end_h_episema(h, note, score);
            }
            break;

        case T_TORCULUS_RESUPINUS:
        case T_TORCULUS_RESUPINUS_FLEXUS:
            if (i == 3) {
                end_h_episema(h, note, score);
            }
            break;

        default:
            /* do nothing */
            break;
        }
    }
}

static __inline void compute_note_positioning(height_computation *const above,
        height_computation *const below, const gregorio_element *const element,
        const gregorio_glyph *const glyph, gregorio_note *const note,
        const int i, const gtex_type type, const gregorio_score *const score)
{
    gregorio_vposition default_vpos = advise_positioning(glyph, note, i, type);

    if (note->h_episema_above == HEPISEMA_AUTO
            && note->h_episema_below == HEPISEMA_AUTO) {
        if (default_vpos == VPOS_BELOW) {
            note->h_episema_above = HEPISEMA_NONE;
        }
        else { /* default_vpos == VPOS_ABOVE */
            note->h_episema_below = HEPISEMA_NONE;
        }
    }

    compute_h_episema(above, element, glyph, note, i, type, score);
    compute_h_episema(below, element, glyph, note, i, type, score);
}

static __inline int compute_fused_shift(const gregorio_glyph *glyph)
{
    int shift;
    gregorio_note *first_note, *prev_note;
    const gregorio_glyph *next, *previous;
    bool next_is_fused;

    if (!glyph || !is_fused(glyph->u.notes.liquescentia)
            || glyph->type != GRE_GLYPH
            || !(first_note = glyph->u.notes.first_note)
            || first_note->type != GRE_NOTE
            || !(previous = gregorio_previous_non_texverb_glyph(glyph))
            || previous->type != GRE_GLYPH
            || previous->u.notes.liquescentia & TAIL_LIQUESCENTIA_MASK
            || !previous->u.notes.first_note) {
        /* base conditions prevent fusion */
        return 0;
    }

    next = gregorio_next_non_texverb_glyph(glyph);
    next_is_fused = next && next->type == GRE_GLYPH
            && is_fused(next->u.notes.liquescentia);

    switch (glyph->u.notes.glyph_type) {
    case G_PORRECTUS:
    case G_PODATUS:
        if (next_is_fused) {
            return 0;
        }
        /* else it may be fused to the previous note */
        break;

    case G_PUNCTUM:
    case G_FLEXA:
        /* these may be fused to the previous note */
        break;

    default:
        /* anything else may not be fused to the previous note */
        return 0;
    }

    switch (first_note->u.note.shape) {
    case S_QUILISMA:
    case S_QUILISMA_QUADRATUM:
    case S_FLAT:
    case S_SHARP:
    case S_NATURAL:
        /* if this glyph starts with one of these, it's not fusible */
        return 0;

    default:
        /* anything else is potentially fusible */
        break;
    }

    switch (previous->u.notes.glyph_type) {
    case G_PUNCTUM:
    case G_FLEXA:
    case G_VIRGA_REVERSA:
        /* these are potentially fusible to this note */
        break;

    default:
        /* everything else is not fusible */
        return 0;
    }

    prev_note = gregorio_glyph_last_note(previous);

    gregorio_assert(prev_note->type == GRE_NOTE, compute_fused_shift,
            "previous note wasn't a note", return 0);

    shift = first_note->u.note.pitch - prev_note->u.note.pitch;
    gregorio_assert(shift >= -MAX_AMBITUS && shift <= MAX_AMBITUS,
            compute_fused_shift, "ambitus too large to fuse", return 0);

    if (shift > 0 && previous->u.notes.glyph_type == G_VIRGA_REVERSA) {
        /* virga reversa cannot fuse upwards */
        return 0;
    }

    if (shift < 0) {
        if (glyph->u.notes.glyph_type == G_PODATUS) {
            if (glyph->u.notes.liquescentia & L_DEMINUTUS) {
                /* podatus deminutus may be fused from above */
                return shift;
            }
            switch (first_note->u.note.shape) {
            case S_ORISCUS_ASCENDENS:
            case S_ORISCUS_DESCENDENS:
            case S_ORISCUS_SCAPUS_ASCENDENS:
            case S_ORISCUS_SCAPUS_DESCENDENS:
                return shift;
            default:
                /* otherwise, podatus may not be fused from above */
                return 0;
            }
        }

        /* the FLEXA check below checks for a porrectus-like flexus, which is
         * not fusible from above */
        if ((next_is_fused && glyph->u.notes.glyph_type == G_FLEXA)
                || glyph->u.notes.glyph_type == G_PORRECTUS
                || (previous->u.notes.glyph_type == G_PUNCTUM
                    && is_initio_debilis(previous->u.notes.liquescentia))) {
            /* may not be fused from above */
            return 0;
        }
    }

    return shift;
}

void gregoriotex_compute_positioning(
        const gregorio_element *const param_element,
        const gregorio_score *const score)
{
    height_computation above = {
        /*.vpos =*/ VPOS_ABOVE,
        /*.orientation =*/ SO_OVER,
        /*.is_applicable =*/ &is_h_episema_above_applicable,
        /*.is_shown =*/ &gtex_is_h_episema_above_shown,
        /*.is_connected =*/ &is_h_episema_above_connected,
        /*.get_size =*/ &get_h_episema_above_size,
        /*.is_better_height =*/ &is_h_episema_above_better_height,
        /*.position =*/ &gregorio_position_h_episema_above,

        /*.active =*/ false,
        /*.height =*/ 0,
        /*.connected =*/ false,
        /*.start_element =*/ NULL,
        /*.start_glyph =*/ NULL,
        /*.start_note =*/ NULL,
        /*.last_connected_element =*/ NULL,
        /*.last_connected_glyph =*/ NULL,
        /*.last_connected_note =*/ NULL,
        /*.adjustment_index =*/ 0,
    };
    height_computation below = {
        /*.vpos =*/ VPOS_BELOW,
        /*.orientation =*/ SO_UNDER,
        /*.is_applicable =*/ &is_h_episema_below_applicable,
        /*.is_shown =*/ &gtex_is_h_episema_below_shown,
        /*.is_connected =*/ &is_h_episema_below_connected,
        /*.get_size =*/ &get_h_episema_below_size,
        /*.is_better_height =*/ &is_h_episema_below_better_height,
        /*.position =*/ &gregorio_position_h_episema_below,

        /*.active =*/ false,
        /*.height =*/ 0,
        /*.connected =*/ false,
        /*.start_element =*/ NULL,
        /*.start_glyph =*/ NULL,
        /*.start_note =*/ NULL,
        /*.last_connected_element =*/ NULL,
        /*.last_connected_glyph =*/ NULL,
        /*.last_connected_note =*/ NULL,
        /*.adjustment_index =*/ 0,
    };
    int i;
    gtex_alignment ignored;
    gtex_type type;
    const gregorio_element *element;

    for (element = param_element; element; element = element->next) {
        if (element->type == GRE_ELEMENT) {
            gregorio_glyph *glyph;
            for (glyph = element->u.first_glyph; glyph;
                    glyph = glyph->next) {
                if (glyph->type == GRE_GLYPH) {
                    gregorio_note *note;
                    glyph->u.notes.fuse_to_next_glyph = compute_fused_shift(
                            gregorio_next_non_texverb_glyph(glyph));
                    i = 0;
                    gregoriotex_determine_glyph_name(glyph, &ignored, &type);
                    for (note = glyph->u.notes.first_note; note;
                            note = note->next) {
                        if (note->type == GRE_NOTE) {
                            compute_note_positioning(&above, &below, element,
                                    glyph, note, ++i, type, score);
                        }
                    }
                }
            }
        } else if (!is_bridgeable_space(element)) {
            end_h_episema(&above, NULL, score);
            end_h_episema(&below, NULL, score);
        }
    }
    end_h_episema(&above, NULL, score);
    end_h_episema(&below, NULL, score);
}

static __inline void adjust_hepisema(gregorio_note *const note,
        const gregorio_sign_orientation orientation,
        bool (*const is_shown)(const gregorio_note *),
        void (*const adjust_if_better)(gregorio_note *, signed char))
{
    const unsigned short adjustment_index =
            note->he_adjustment_index[orientation];

    if (adjustment_index && is_shown(note)) {
        gregorio_hepisema_adjustment *adj = gregorio_get_hepisema_adjustment(
                adjustment_index);

        adjust_if_better(note, adj->pitch_extremum);
    }
}

void gregoriotex_compute_cross_syllable_positioning(
        const gregorio_score *const score)
{
    gregorio_syllable *syllable;
    for (syllable = score->first_syllable; syllable;
            syllable = syllable->next_syllable) {
        int voice;
        for (voice = 0; voice < score->number_of_voices; ++voice) {
            gregorio_element *element;
            for (element = syllable->elements[voice]; element;
                    element = element->next) {
                if (element->type == GRE_ELEMENT) {
                    gregorio_glyph *glyph;
                    for (glyph = element->u.first_glyph; glyph;
                            glyph = glyph->next) {
                        if (glyph->type == GRE_GLYPH) {
                            gregorio_note *note;
                            for (note = glyph->u.notes.first_note; note;
                                    note = note->next) {
                                if (note->type == GRE_NOTE) {
                                    adjust_hepisema(note, SO_OVER,
                                            gtex_is_h_episema_above_shown,
                                            adjust_h_episema_above_if_better);
                                    adjust_hepisema(note, SO_UNDER,
                                            gtex_is_h_episema_below_shown,
                                            adjust_h_episema_below_if_better);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
