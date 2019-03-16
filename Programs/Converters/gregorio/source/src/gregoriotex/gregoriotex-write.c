/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file contains functions for writing GregorioTeX from Gregorio structures.
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

/**
 * @file
 * @brief The plugin which writes a GregorioTeX score.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "bool.h"
#include "struct.h"
#include "unicode.h"
#include "messages.h"
#include "characters.h"
#include "plugins.h"
#include "support.h"
#include "utf8strings.h"

#include "gregoriotex.h"

#define BUFSIZE 128

/* a structure containing the status */
typedef struct gregoriotex_status {
    bool point_and_click;

    /* true if the current_glyph will have an additional line under or not
     * (useful to determine the length of the bar in case of a flexa starting
     * at d */
    bool bottom_line;

    signed char top_height;
    signed char bottom_height;

    /* indicates if there is a translation on the line */
    bool translation;

    /* indicates if there is "above lines text" on the line */
    bool abovelinestext;

    bool suppressed_custos;
} gregoriotex_status;

#define UNDETERMINED_HEIGHT -127

#define MAX_AMBITUS 5
static const char *tex_ambitus[] = {
    "", "One", "Two", "Three", "Four", "Five"
};

#define SHAPE(NAME) static const char *const SHAPE_##NAME = #NAME
SHAPE(Ancus);
SHAPE(AncusLongqueue);
SHAPE(AscendensOriscus);
SHAPE(AscendensOriscusLineBL);
SHAPE(AscendensOriscusLineTL);
SHAPE(AscendensOriscusScapus);
SHAPE(AscendensOriscusScapusLongqueue);
SHAPE(AscendensOriscusScapusOpenqueue);
SHAPE(AscendensPunctumInclinatum);
SHAPE(DescendensOriscus);
SHAPE(DescendensOriscusLineBL);
SHAPE(DescendensOriscusLineTL);
SHAPE(DescendensOriscusScapus);
SHAPE(DescendensOriscusScapusLongqueue);
SHAPE(DescendensOriscusScapusOpenqueue);
SHAPE(DescendensPunctumInclinatum);
SHAPE(Flat);
SHAPE(Flexus);
SHAPE(FlexusLongqueue);
SHAPE(FlexusNobar);
SHAPE(FlexusOpenqueue);
SHAPE(FlexusOriscus);
SHAPE(FlexusOriscusInusitatus);
SHAPE(FlexusOriscusScapus);
SHAPE(FlexusOriscusScapusInusitatus);
SHAPE(FlexusOriscusScapusInusitatusLongqueue);
SHAPE(FlexusOriscusScapusInusitatusOpenqueue);
SHAPE(FlexusOriscusScapusLongqueue);
SHAPE(FlexusOriscusScapusOpenqueue);
SHAPE(Linea);
SHAPE(LineaPunctum);
SHAPE(Natural);
SHAPE(OriscusDeminutus);
SHAPE(Pes);
SHAPE(PesAscendensOriscus);
SHAPE(PesDescendensOriscus);
SHAPE(PesQuadratum);
SHAPE(PesQuadratumLongqueue);
SHAPE(PesQuadratumOpenqueue);
SHAPE(PesQuassus);
SHAPE(PesQuassusInusitatus);
SHAPE(PesQuassusInusitatusLongqueue);
SHAPE(PesQuassusInusitatusOpenqueue);
SHAPE(PesQuassusLongqueue);
SHAPE(PesQuassusOpenqueue);
SHAPE(Porrectus);
SHAPE(PorrectusFlexus);
SHAPE(PorrectusFlexusLongqueue);
SHAPE(PorrectusFlexusNobar);
SHAPE(PorrectusLongqueue);
SHAPE(PorrectusNobar);
SHAPE(Punctum);
SHAPE(PunctumAscendens);
SHAPE(PunctumDeminutus);
SHAPE(PunctumDescendens);
SHAPE(PunctumInclinatumAuctus);
SHAPE(PunctumInclinatumDeminutus);
SHAPE(PunctumLineBL);
SHAPE(PunctumLineTL);
SHAPE(Quilisma);
SHAPE(QuilismaPes);
SHAPE(QuilismaPesQuadratum);
SHAPE(QuilismaPesQuadratumLongqueue);
SHAPE(QuilismaPesQuadratumOpenqueue);
SHAPE(StansPunctumInclinatum);
SHAPE(Salicus);
SHAPE(SalicusFlexus);
SHAPE(SalicusLongqueue);
SHAPE(Scandicus);
SHAPE(Sharp);
SHAPE(Stropha);
SHAPE(StrophaAucta);
SHAPE(StrophaAuctaLongtail);
SHAPE(Torculus);
SHAPE(TorculusLiquescens);
SHAPE(TorculusLiquescensQuilisma);
SHAPE(TorculusQuilisma);
SHAPE(TorculusResupinus);
SHAPE(TorculusResupinusQuilisma);
SHAPE(Virga);
SHAPE(VirgaLongqueue);
SHAPE(VirgaOpenqueue);
SHAPE(VirgaReversa);
SHAPE(VirgaReversaAscendens);
SHAPE(VirgaReversaDescendens);
SHAPE(VirgaReversaLongqueue);
SHAPE(VirgaReversaLongqueueAscendens);
SHAPE(VirgaReversaLongqueueDescendens);
SHAPE(VirgaReversaOpenqueue);
SHAPE(VirgaReversaOpenqueueAscendens);
SHAPE(VirgaReversaOpenqueueDescendens);

#define LIQ(NAME) static const char *const LIQ_##NAME = #NAME
LIQ(Deminutus);
LIQ(Ascendens);
LIQ(Descendens);
LIQ(InitioDebilis);
LIQ(InitioDebilisDeminutus);
LIQ(InitioDebilisAscendens);
LIQ(InitioDebilisDescendens);
LIQ(Nothing);

#define FUSE(NAME) static const char *const FUSE_##NAME = #NAME
FUSE(Lower);
FUSE(LowerOblatus);
FUSE(Upper);
FUSE(UpperOblatus);
FUSE(Down);
FUSE(Up);

/* the value indicating to GregorioTeX that there is no flat */
#define NO_KEY_FLAT LOWEST_PITCH

static __inline signed char pitch_value(const signed char height) {
    /* right now height == pitch, but this function allows us to change
     * the offset easily */
    return height;
}

static __inline int bool_to_int(bool value) {
    return value? 1 : 0;
}

typedef enum queuetype {
    Q_ON_SPACE_BELOW_BOTTOM_LINE,
    Q_ON_SPACE_ABOVE_BOTTOM_LINE,
    Q_ON_BOTTOM_LINE,
    Q_ON_LINE_ABOVE_BOTTOM_LINE
} queuetype;

static queuetype adjusted_queuetype_of(const gregorio_note *const note,
        const gregorio_note *const queue_note, const signed char adjustment)
{
    switch (note->u.note.pitch + adjustment - LOWEST_PITCH) {
    case 0:
        return Q_ON_SPACE_BELOW_BOTTOM_LINE;
    case 1:
        return Q_ON_BOTTOM_LINE;
    case 2:
        return queue_note->low_ledger_line?
                Q_ON_SPACE_ABOVE_BOTTOM_LINE : Q_ON_SPACE_BELOW_BOTTOM_LINE;
    case 3:
        return queue_note->low_ledger_line?
                Q_ON_LINE_ABOVE_BOTTOM_LINE : Q_ON_BOTTOM_LINE;
    case 5:
    case 7:
    case 9:
    case 11:
    case 13:
        return Q_ON_LINE_ABOVE_BOTTOM_LINE;
    default:
        return Q_ON_SPACE_ABOVE_BOTTOM_LINE;
    }
}

static queuetype queuetype_of(const gregorio_note *const note) {
    return adjusted_queuetype_of(note, note, 0);
}

static grestyle_style gregoriotex_ignore_style = ST_NO_STYLE;
static grestyle_style gregoriotex_next_ignore_style = ST_NO_STYLE;

/*
 * The different liquescentiae are:
 * 'Nothing'
 * 'InitioDebilis'
 * 'Deminutus
 * 'Ascendens'
 * 'Descendens'
 * 'InitioDebilisDeminutus'
 * 'InitioDebilisAscendens'
 * 'InitioDebilisDescendens'
 *
 * They also are and must be the same as in squarize.py.
 */

static const char *gregoriotex_determine_liquescentia(
        gtex_glyph_liquescentia type, const gregorio_glyph *const glyph)
{
    gregorio_liquescentia liquescentia = glyph->u.notes.liquescentia;

    switch (type) {
    case LG_ALL:
        break;
    case LG_NO_INITIO:
        liquescentia &= ~L_INITIO_DEBILIS;
        break;
    case LG_ONLY_DEMINUTUS:
        liquescentia &= L_INITIO_DEBILIS | L_DEMINUTUS;
        break;
    case LG_FUSIBLE_INITIO:
        if (glyph->u.notes.fuse_to_next_glyph) {
            liquescentia &= L_INITIO_DEBILIS;
            break;
        }
        /* else fall through to next case */
    case LG_NONE:
        liquescentia = L_NO_LIQUESCENTIA;
        break;
    }

    /* now we convert liquescentia into the good GregorioTeX liquescentia
     * numbers */

    switch (liquescentia & ~L_FUSED) {
    case L_DEMINUTUS:
        return LIQ_Deminutus;
    case L_AUCTUS_ASCENDENS:
        return LIQ_Ascendens;
    case L_AUCTUS_DESCENDENS:
        return LIQ_Descendens;
    case L_INITIO_DEBILIS:
        return LIQ_InitioDebilis;
    case L_DEMINUTUS_INITIO_DEBILIS:
        return LIQ_InitioDebilisDeminutus;
    case L_AUCTUS_ASCENDENS_INITIO_DEBILIS:
        return LIQ_InitioDebilisAscendens;
    case L_AUCTUS_DESCENDENS_INITIO_DEBILIS:
        return LIQ_InitioDebilisDescendens;
    case L_NO_LIQUESCENTIA:
        /* break out and return "Nothing" */
        break;
    }

    return LIQ_Nothing;
}

static __inline int compute_ambitus(const gregorio_note *const current_note)
{
    int first = current_note->u.note.pitch;
    int second = current_note->next->u.note.pitch;
    int ambitus;
    if (first < second) {
        ambitus = second - first;
    } else {
        ambitus = first - second;
    }
    gregorio_assert2(ambitus >= 1 && ambitus <= MAX_AMBITUS, compute_ambitus,
            "unsupported ambitus: %d", ambitus, return 0);
    return ambitus;
}

static const char *compute_glyph_name(const gregorio_glyph *const glyph,
        const char *shape, const gtex_glyph_liquescentia ltype,
        bool is_single_note)
{
    static char buf[BUFSIZE];

    const char *liquescentia = gregoriotex_determine_liquescentia(ltype, glyph);
    const gregorio_note *current_note;
    int ambitus1, ambitus2, ambitus3, fuse_ambitus = 0;
    const char *fuse_head = "", *fuse_tail = "";
    const gregorio_glyph *previous = gregorio_previous_non_texverb_glyph(glyph);
    int fuse_to_next_note, fuse_from_previous_note =
            (previous && previous->type == GRE_GLYPH)
            ? previous->u.notes.fuse_to_next_glyph : 0;

    /* then we start making our formula */
    gregorio_assert(glyph, compute_glyph_name, "called with NULL pointer",
            return "");
    gregorio_assert(glyph->u.notes.first_note, compute_glyph_name,
            "called with a glyph that have no note", return "");

    fuse_to_next_note = glyph->u.notes.fuse_to_next_glyph;

    switch (glyph->u.notes.glyph_type) {
    case G_PODATUS:
    case G_PUNCTUM:
    case G_FLEXA:
        /* directionally head-fusible */
        if (fuse_from_previous_note < 0) {
            if (glyph->u.notes.first_note->u.note.shape != S_QUILISMA
                && glyph->u.notes.first_note->u.note.shape
                != S_QUILISMA_QUADRATUM) {
                if (fuse_from_previous_note < -1) {
                    fuse_head = FUSE_Lower;
                } else if (glyph->u.notes.first_note->u.note.shape
                        == S_ORISCUS_DESCENDENS
                        || glyph->u.notes.first_note->u.note.shape
                        == S_ORISCUS_SCAPUS_DESCENDENS) {
                    /* fuse_from_previous_note will be -1 here */
                    fuse_head = FUSE_LowerOblatus;
                }
            }
        } else if (fuse_from_previous_note > 0) {
            if (fuse_from_previous_note > 1) {
                fuse_head = FUSE_Upper;
            } else if (glyph->u.notes.first_note->u.note.shape
                    == S_ORISCUS_ASCENDENS
                    || glyph->u.notes.first_note->u.note.shape
                    == S_ORISCUS_SCAPUS_ASCENDENS) {
                /* fuse_from_previous_note will be 1 here */
                fuse_head = FUSE_UpperOblatus;
            }
        }
        break;

    default:
        /* not directionally head-fusible */
        break;
    }

    switch (glyph->u.notes.glyph_type) {
    case G_FLEXA:
        if (fuse_to_next_note <= 0) {
            /* a flexa is only fusible up */
            break;
        }
        /* else fall through */
    case G_VIRGA_REVERSA:
    case G_PUNCTUM:
        /* tail-fusible */
        if (fuse_to_next_note < 0) {
            fuse_tail = FUSE_Down;
            fuse_ambitus = -fuse_to_next_note;
        } else if (fuse_to_next_note > 0) {
            fuse_tail = FUSE_Up;
            fuse_ambitus = fuse_to_next_note;
        }

        if (*fuse_tail && liquescentia == LIQ_Nothing) {
            liquescentia = "";
        }
        break;

    default:
        /* not tail-fusible */
        break;
    }

    if (is_fused(glyph->u.notes.liquescentia)) {
        if (shape == SHAPE_AscendensOriscusScapus
                || shape == SHAPE_AscendensOriscusScapusLongqueue
                || shape == SHAPE_AscendensOriscusScapusOpenqueue) {
            shape = SHAPE_AscendensOriscus;
        } else if (shape == SHAPE_DescendensOriscusScapus
                || shape == SHAPE_DescendensOriscusScapusLongqueue
                || shape == SHAPE_DescendensOriscusScapusOpenqueue) {
            shape = SHAPE_DescendensOriscus;
        } else if (shape == SHAPE_FlexusOriscusScapus
                || shape == SHAPE_FlexusOriscusScapusLongqueue
                || shape == SHAPE_FlexusOriscusScapusOpenqueue) {
            shape = SHAPE_FlexusOriscus;
        } else if (shape == SHAPE_FlexusOriscusScapusInusitatus
                || shape == SHAPE_FlexusOriscusScapusInusitatusLongqueue
                || shape == SHAPE_FlexusOriscusScapusInusitatusOpenqueue) {
            shape = SHAPE_FlexusOriscusInusitatus;
        }
    }

    current_note = glyph->u.notes.first_note;
    if (is_single_note) {
        if (liquescentia == LIQ_Nothing) {
            liquescentia = "";
        }
        if (!(*fuse_tail) && fuse_head != FUSE_UpperOblatus
                && fuse_head != FUSE_LowerOblatus) {
            /* single note fused shapes have weird names */
            if (fuse_head == FUSE_Upper) {
                if (shape == SHAPE_Punctum) {
                    shape = SHAPE_PunctumLineBL;
                } else if (shape == SHAPE_AscendensOriscus) {
                    shape = SHAPE_AscendensOriscusLineBL;
                } else if (shape == SHAPE_DescendensOriscus) {
                    shape = SHAPE_DescendensOriscusLineBL;
                }
            } else if (fuse_head == FUSE_Lower) {
                if (shape == SHAPE_Punctum) {
                    shape = SHAPE_PunctumLineTL;
                } else if (shape == SHAPE_AscendensOriscus) {
                    shape = SHAPE_AscendensOriscusLineTL;
                } else if (shape == SHAPE_DescendensOriscus) {
                    shape = SHAPE_DescendensOriscusLineTL;
                }
            }
            fuse_head = "";
        }
        gregorio_snprintf(buf, BUFSIZE, "%s%s%s%s%s", fuse_head, shape,
                tex_ambitus[fuse_ambitus], liquescentia, fuse_tail);
        return buf;
    }
    gregorio_assert(current_note->next, compute_glyph_name,
            "called with a multi-note glyph that has only one note", return "");
    gregorio_assert((ambitus1 = compute_ambitus(current_note)),
            compute_glyph_name, "unexpected unison on multi-note glyph",
            return "");
    if (is_fused(glyph->u.notes.liquescentia)) {
        if (shape == SHAPE_Flexus || shape == SHAPE_FlexusLongqueue
                || shape == SHAPE_FlexusOpenqueue) {
            if (fuse_to_next_note) {
                fuse_head = "";
            }
            if (*fuse_head) {
                shape = SHAPE_Flexus;
            } else {
                shape = SHAPE_FlexusNobar;
            }
        } else if (shape == SHAPE_Porrectus
                || shape == SHAPE_PorrectusLongqueue) {
            shape = SHAPE_PorrectusNobar;
        }
    } else {
        if (fuse_to_next_note && (shape == SHAPE_FlexusLongqueue
                    || shape == SHAPE_FlexusOpenqueue)) {
            /* a porrectus-like flexus has no longqueue variant */
            shape = SHAPE_Flexus;
        }
    }
    if (shape == SHAPE_SalicusLongqueue && liquescentia != LIQ_Nothing) {
        /* the salicus queue is at the end of the glyph, and it doesn't exist
         * for the liquescent forms */
        shape = SHAPE_Salicus;
    }
    current_note = current_note->next;
    if (!current_note->next) {
        gregorio_snprintf(buf, BUFSIZE, "%s%s%s%s%s%s", fuse_head, shape,
                tex_ambitus[ambitus1], tex_ambitus[fuse_ambitus],
                liquescentia, fuse_tail);
        return buf;
    }
    gregorio_assert((ambitus2 = compute_ambitus(current_note)),
            compute_glyph_name, "unexpected unison on multi-note glyph",
            return "");
    current_note = current_note->next;
    if (!current_note->next) {
        gregorio_snprintf(buf, BUFSIZE, "%s%s%s%s%s%s%s", fuse_head, shape,
                tex_ambitus[ambitus1], tex_ambitus[ambitus2],
                tex_ambitus[fuse_ambitus], liquescentia, fuse_tail);
        return buf;
    }
    gregorio_assert((ambitus3 = compute_ambitus(current_note)),
            compute_glyph_name, "unexpected unison on multi-note glyph",
            return "");
    gregorio_snprintf(buf, BUFSIZE, "%s%s%s%s%s%s%s%s", fuse_head, shape,
            tex_ambitus[ambitus1], tex_ambitus[ambitus2], tex_ambitus[ambitus3],
            tex_ambitus[fuse_ambitus], liquescentia, fuse_tail);
    return buf;
}

static const char *fusible_queued_shape(const gregorio_note *const note,
        const gregorio_glyph *const glyph,
        const char *const base_shape, const char *const longqueue_shape,
        const char *const openqueue_shape)
{
    const char *name = NULL;
    if (glyph->u.notes.fuse_to_next_glyph < 0) {
        /* queue size depends on the following note if fused down */
        bool ambitus_one = (glyph->u.notes.fuse_to_next_glyph == -1);
        switch (adjusted_queuetype_of(note, note,
                    glyph->u.notes.fuse_to_next_glyph)) {
        case Q_ON_SPACE_BELOW_BOTTOM_LINE:
            if (ambitus_one) {
                name = openqueue_shape;
                break;
            }
            /* else fall through */
        case Q_ON_SPACE_ABOVE_BOTTOM_LINE:
            /* at ambitus one, long and short are swapped becuase the queue where
             * the second note is on a space is longer than on a line */
            name = ambitus_one? longqueue_shape : base_shape;
            break;
        case Q_ON_BOTTOM_LINE:
            if (ambitus_one) {
                name = openqueue_shape;
                break;
            }
            /* else fall through */
        case Q_ON_LINE_ABOVE_BOTTOM_LINE:
            /* at ambitus one, long and short are swapped becuase the queue where
             * the second note is on a line is shorter than on a space */
            name = ambitus_one? base_shape : longqueue_shape;
            break;
        }
    } else if (glyph->u.notes.fuse_to_next_glyph) {
        /* TODO (5.0?) handle queue size on upwards fusion */
        switch (queuetype_of(note)) {
        case Q_ON_SPACE_BELOW_BOTTOM_LINE:
        case Q_ON_SPACE_ABOVE_BOTTOM_LINE:
        case Q_ON_BOTTOM_LINE:
            name = base_shape;
            break;
        case Q_ON_LINE_ABOVE_BOTTOM_LINE:
            name = longqueue_shape;
            break;
        }
    } else {
        switch (queuetype_of(note)) {
        case Q_ON_SPACE_ABOVE_BOTTOM_LINE:
            name = base_shape;
            break;
        case Q_ON_SPACE_BELOW_BOTTOM_LINE:
        case Q_ON_BOTTOM_LINE:
            name = openqueue_shape;
            break;
        case Q_ON_LINE_ABOVE_BOTTOM_LINE:
            name = longqueue_shape;
            break;
        }
    }
    gregorio_not_null(name, fusible_queued_shape, return base_shape);
    return compute_glyph_name(glyph, name, LG_NONE, true);
}

static __inline char *code_point(const char *const shape, const bool is_cavum,
        char *const buf, const size_t bufsize)
{
    if (is_cavum) {
        gregorio_snprintf(buf, bufsize, "\\GreCavum{%s}", shape);
    } else {
        gregorio_snprintf(buf, bufsize, "\\GreCP%s", shape);
    }
    return buf;
}

static const char *determine_note_glyph_name(const gregorio_note *const note,
        const gregorio_glyph *glyph, gtex_alignment *type)
{
    static char buf[128], cpbuf[96];
    const char *name = "";

    gregorio_assert(note, determine_note_glyph_name, "called with NULL pointer",
            return "");

    *type = AT_ONE_NOTE;
    switch (note->u.note.shape) {
    case S_PUNCTUM_INCLINATUM_ASCENDENS:
        *type = AT_PUNCTUM_INCLINATUM;
        return SHAPE_AscendensPunctumInclinatum;
    case S_PUNCTUM_INCLINATUM_STANS:
        *type = AT_PUNCTUM_INCLINATUM;
        return SHAPE_StansPunctumInclinatum;
    case S_PUNCTUM_INCLINATUM_DESCENDENS:
        *type = AT_PUNCTUM_INCLINATUM;
        return SHAPE_DescendensPunctumInclinatum;
    case S_PUNCTUM_INCLINATUM_DEMINUTUS:
        return SHAPE_PunctumInclinatumDeminutus;
    case S_PUNCTUM_INCLINATUM_AUCTUS:
        return SHAPE_PunctumInclinatumAuctus;
    case S_PUNCTUM:
        return compute_glyph_name(glyph, SHAPE_Punctum, LG_FUSIBLE_INITIO, true);
    case S_PUNCTUM_AUCTUS_ASCENDENS:
        return SHAPE_PunctumAscendens;
    case S_PUNCTUM_AUCTUS_DESCENDENS:
        return SHAPE_PunctumDescendens;
    case S_PUNCTUM_DEMINUTUS:
        return SHAPE_PunctumDeminutus;
    case S_LINEA:
        return SHAPE_Linea;
    case S_LINEA_PUNCTUM:
        return SHAPE_LineaPunctum;
    case S_VIRGA:
        switch (queuetype_of(note)) {
        case Q_ON_SPACE_ABOVE_BOTTOM_LINE:
            return SHAPE_Virga;
        case Q_ON_SPACE_BELOW_BOTTOM_LINE:
        case Q_ON_BOTTOM_LINE:
            return SHAPE_VirgaOpenqueue;
        case Q_ON_LINE_ABOVE_BOTTOM_LINE:
            return SHAPE_VirgaLongqueue;
        } /* all cases return, so this line is not hit; LCOV_EXCL_LINE */
    case S_VIRGA_REVERSA:
        switch (note->u.note.liquescentia) {
        case L_AUCTUS_ASCENDENS:
            switch (queuetype_of(note)) {
            case Q_ON_SPACE_BELOW_BOTTOM_LINE:
            case Q_ON_SPACE_ABOVE_BOTTOM_LINE:
                name = SHAPE_VirgaReversaAscendens;
                break;
            case Q_ON_BOTTOM_LINE:
                name = SHAPE_VirgaReversaOpenqueueAscendens;
                break;
            case Q_ON_LINE_ABOVE_BOTTOM_LINE:
                name = SHAPE_VirgaReversaLongqueueAscendens;
                break;
            }
            if (note->u.note.pitch - LOWEST_PITCH == 3) {
                /* if we're on the 'd' line, the queue could be long or short */
                gregorio_snprintf(buf, sizeof buf,
                        "VirgaReversaAscendensOnDLine{%s}", code_point(name,
                            glyph->u.notes.is_cavum, cpbuf, sizeof cpbuf));
                return buf;
            }
            return name;
        case L_AUCTUS_DESCENDENS:
            switch (queuetype_of(note)) {
            case Q_ON_SPACE_BELOW_BOTTOM_LINE:
            case Q_ON_SPACE_ABOVE_BOTTOM_LINE:
                return SHAPE_VirgaReversaDescendens;
            case Q_ON_BOTTOM_LINE:
                return SHAPE_VirgaReversaOpenqueueDescendens;
            case Q_ON_LINE_ABOVE_BOTTOM_LINE:
                return SHAPE_VirgaReversaLongqueueDescendens;
            } /* all cases return, so this line is not hit; LCOV_EXCL_LINE */
        default:
            return fusible_queued_shape(note, glyph, SHAPE_VirgaReversa,
                    SHAPE_VirgaReversaLongqueue, SHAPE_VirgaReversaOpenqueue);
        }
    case S_ORISCUS_ASCENDENS:
        *type = AT_ORISCUS;
        return compute_glyph_name(glyph, SHAPE_AscendensOriscus, LG_NONE, true);
    case S_ORISCUS_DESCENDENS:
        *type = AT_ORISCUS;
        return compute_glyph_name(glyph, SHAPE_DescendensOriscus, LG_NONE, true);
    case S_ORISCUS_DEMINUTUS:
        *type = AT_ORISCUS;
        return SHAPE_OriscusDeminutus;
    case S_QUILISMA:
        *type = AT_QUILISMA;
        return compute_glyph_name(glyph, SHAPE_Quilisma, LG_NONE, true);
    case S_ORISCUS_SCAPUS_ASCENDENS:
        return fusible_queued_shape(note, glyph, SHAPE_AscendensOriscusScapus,
                SHAPE_AscendensOriscusScapusLongqueue,
                SHAPE_AscendensOriscusScapusOpenqueue);
    case S_ORISCUS_SCAPUS_DESCENDENS:
        return fusible_queued_shape(note, glyph, SHAPE_DescendensOriscusScapus,
                SHAPE_DescendensOriscusScapusLongqueue,
                SHAPE_DescendensOriscusScapusOpenqueue);
    case S_STROPHA:
        *type = AT_STROPHA;
        if (!(note->u.note.liquescentia &
                (L_AUCTUS_ASCENDENS | L_AUCTUS_DESCENDENS))) {
            return SHAPE_Stropha;
        }
        /* else fall through to next case */
    case S_STROPHA_AUCTA:
        *type = AT_STROPHA;
        switch (queuetype_of(note)) {
        case Q_ON_SPACE_BELOW_BOTTOM_LINE:
        case Q_ON_SPACE_ABOVE_BOTTOM_LINE:
            return SHAPE_StrophaAucta;
        case Q_ON_BOTTOM_LINE:
        case Q_ON_LINE_ABOVE_BOTTOM_LINE:
            return SHAPE_StrophaAuctaLongtail;
        } /* all cases return, so this line is not hit; LCOV_EXCL_LINE */
    case S_FLAT:
        return SHAPE_Flat;
    case S_SHARP:
        return SHAPE_Sharp;
    case S_NATURAL:
        return SHAPE_Natural;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        gregorio_fail2(determine_note_glyph_name,
                "called with unknown shape: %s",
                gregorio_shape_to_string(note->u.note.shape));
        return "";
        /* LCOV_EXCL_STOP */
    }
}

static __inline gregorio_note *first_note_of(
        const gregorio_glyph *const glyph) {
    assert(glyph->type == GRE_GLYPH);
    assert(glyph->u.notes.first_note);
    assert(glyph->u.notes.first_note->type == GRE_NOTE);
    return glyph->u.notes.first_note;
}

static __inline signed char first_pitch_of(const gregorio_glyph *const glyph) {
    return first_note_of(glyph)->u.note.pitch;
}

static __inline gregorio_note *second_note_of(
        const gregorio_glyph *const glyph) {
    assert(glyph->type == GRE_GLYPH);
    assert(glyph->u.notes.first_note);
    assert(glyph->u.notes.first_note->type == GRE_NOTE);
    assert(glyph->u.notes.first_note->next);
    assert(glyph->u.notes.first_note->next->type == GRE_NOTE);
    return glyph->u.notes.first_note->next;
}

static __inline signed char second_pitch_of(const gregorio_glyph *const glyph) {
    return second_note_of(glyph)->u.note.pitch;
}

static __inline gregorio_note *third_note_of(
        const gregorio_glyph *const glyph) {
    assert(glyph->type == GRE_GLYPH);
    assert(glyph->u.notes.first_note);
    assert(glyph->u.notes.first_note->type == GRE_NOTE);
    assert(glyph->u.notes.first_note->next);
    assert(glyph->u.notes.first_note->next->type == GRE_NOTE);
    assert(glyph->u.notes.first_note->next->next);
    assert(glyph->u.notes.first_note->next->next->type == GRE_NOTE);
    return glyph->u.notes.first_note->next->next;
}

static __inline signed char third_pitch_of(const gregorio_glyph *const glyph) {
    return third_note_of(glyph)->u.note.pitch;
}

static __inline const char *porrectus_shape(const gregorio_glyph *const glyph,
        const char *base_shape, const char *longqueue_shape) {
    const gregorio_note *const first_note = first_note_of(glyph);
    if ((glyph->u.notes.glyph_type == G_PORRECTUS
                && (glyph->u.notes.liquescentia & L_DEMINUTUS))
            || first_note->u.note.pitch - second_pitch_of(glyph) == 1) {
        switch (queuetype_of(first_note)) {
        case Q_ON_SPACE_BELOW_BOTTOM_LINE:
        case Q_ON_SPACE_ABOVE_BOTTOM_LINE:
        case Q_ON_BOTTOM_LINE:
            return base_shape;
        case Q_ON_LINE_ABOVE_BOTTOM_LINE:
            return longqueue_shape;
        }
    } /* must be optimized out; LCOV_EXCL_LINE */
    return base_shape;
}

static __inline const char *flexus_shape(const gregorio_glyph *const glyph,
        const signed char ambitus, const char *base_shape,
        const char *longqueue_shape, const char *openqueue_shape) {
    const bool ambitus_one = (ambitus == 1);
    switch (adjusted_queuetype_of(second_note_of(glyph), first_note_of(glyph),
                0)) {
    case Q_ON_SPACE_BELOW_BOTTOM_LINE:
        if (ambitus_one) {
            return openqueue_shape;
        }
        /* else fall through */
    case Q_ON_SPACE_ABOVE_BOTTOM_LINE:
        /* at ambitus one, long and short are swapped becuase the queue where
         * the second note is on a space is longer than on a line */
        return ambitus_one? longqueue_shape : base_shape;
    case Q_ON_BOTTOM_LINE:
        if (ambitus_one) {
            return openqueue_shape;
        }
        /* else fall through */
    case Q_ON_LINE_ABOVE_BOTTOM_LINE:
        /* at ambitus one, long and short are swapped becuase the queue where
         * the second note is on a line is shorter than on a space */
        return ambitus_one? base_shape : longqueue_shape;
    }
    /* not reachable unless there's a programming error */
    /* LCOV_EXCL_START */
    gregorio_fail(flexus_shape, "unexpected queue length");
    return base_shape;
    /* LCOV_EXCL_STOP */
}

static __inline const char *quadratum_shape(const gregorio_glyph *const glyph,
        const char *base_shape, const char *longqueue_shape,
        const char *openqueue_shape) {
    const bool ambitus_one =
            (second_pitch_of(glyph) - first_pitch_of(glyph) == 1);
    if (!is_tail_liquescentia(glyph->u.notes.liquescentia)) {
        switch (adjusted_queuetype_of(first_note_of(glyph),
                    second_note_of(glyph), 0)) {
        case Q_ON_SPACE_BELOW_BOTTOM_LINE:
            if (ambitus_one) {
                return openqueue_shape;
            }
            /* else fall through */
        case Q_ON_SPACE_ABOVE_BOTTOM_LINE:
            /* at ambitus one, long and short are swapped becuase the queue
             * where the first note is on a space is longer than on a line */
            return ambitus_one? longqueue_shape : base_shape;
        case Q_ON_BOTTOM_LINE:
            if (ambitus_one) {
                return openqueue_shape;
            }
            /* else fall through */
        case Q_ON_LINE_ABOVE_BOTTOM_LINE:
            /* at ambitus one, long and short are swapped becuase the queue
             * where the first note is on a line is shorter than on a space */
            return ambitus_one? base_shape : longqueue_shape;
        }
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        gregorio_fail(quadratum_shape, "unexpected queue length");
        /* fall out of the conditional */
    }
    /* LCOV_EXCL_STOP */
    return base_shape;
}

/* the function that calculates the number of the glyph. It also
 * calculates the type, used for determining the position of signs. Type is
 * very basic, it is only the global dimensions : torculus, one_note, etc. */

const char *gregoriotex_determine_glyph_name(const gregorio_glyph *const glyph,
        gtex_alignment *const type, gtex_type *const gtype)
{
    const char *shape = NULL;
    gtex_glyph_liquescentia ltype;
    signed char ambitus;
    const gregorio_note *second_note;
    gregorio_assert(glyph, gregoriotex_determine_glyph_name,
            "called with NULL pointer", return "");
    gregorio_assert(glyph->u.notes.first_note, gregoriotex_determine_glyph_name,
            "called with a glyph that has no note", return "");
    *gtype = T_ONE_NOTE;
    switch (glyph->u.notes.glyph_type) {
    case G_PODATUS:
        switch (glyph->u.notes.first_note->u.note.shape) {
        case S_QUILISMA:
            *type = AT_QUILISMA;
            /* the next if is because we made the choice that AUCTUS shapes
             * look like pes quadratum. */
            if (glyph->u.notes.liquescentia
                    & (L_AUCTUS_ASCENDENS | L_AUCTUS_DESCENDENS)) {
                *gtype = T_QUILISMA_PES_QUADRATUM;
                shape = SHAPE_QuilismaPesQuadratum;
            } else {
                *gtype = T_QUILISMA_PES;
                shape = SHAPE_QuilismaPes;
            }
            ltype = LG_NO_INITIO;
            break;
        case S_ORISCUS_ASCENDENS:
        case S_ORISCUS_SCAPUS_ASCENDENS:
            *type = AT_ORISCUS;
            *gtype = T_PES_QUASSUS;
            shape = quadratum_shape(glyph, SHAPE_PesQuassus,
                    SHAPE_PesQuassusLongqueue, SHAPE_PesQuassusOpenqueue);
            ltype = LG_NO_INITIO;
            break;
        case S_ORISCUS_DESCENDENS:
        case S_ORISCUS_SCAPUS_DESCENDENS:
            *type = AT_ORISCUS;
            *gtype = T_PES_QUASSUS;
            shape = quadratum_shape(glyph, SHAPE_PesQuassusInusitatus,
                    SHAPE_PesQuassusInusitatusLongqueue,
                    SHAPE_PesQuassusInusitatusOpenqueue);
            ltype = LG_NO_INITIO;
            break;
        default:
            *type = AT_ONE_NOTE;
            if (glyph->u.notes.liquescentia
                    & (L_AUCTUS_ASCENDENS | L_AUCTUS_DESCENDENS)) {
                *gtype = T_PES_QUADRATUM;
                shape = SHAPE_PesQuadratum;
            } else {
                *gtype = T_PES;
                shape = SHAPE_Pes;
            }
            ltype = LG_ALL;
            break;
        }
        break;
    case G_PES_QUADRATUM:
        switch (glyph->u.notes.first_note->u.note.shape) {
        case S_QUILISMA:
            *type = AT_QUILISMA;
            *gtype = T_QUILISMA_PES_QUADRATUM;
            shape = quadratum_shape(glyph, SHAPE_QuilismaPesQuadratum,
                   SHAPE_QuilismaPesQuadratumLongqueue,
                   SHAPE_QuilismaPesQuadratumOpenqueue);
            ltype = LG_NO_INITIO;
            break;
        default:
            *type = AT_ONE_NOTE;
            *gtype = T_PES_QUADRATUM;
            shape = quadratum_shape(glyph, SHAPE_PesQuadratum,
                   SHAPE_PesQuadratumLongqueue, SHAPE_PesQuadratumOpenqueue);
            ltype = LG_ALL;
            break;
        }
        break;
    case G_PES_ASCENDENS_ORISCUS:
        *type = AT_ONE_NOTE;
        *gtype = T_PES_ORISCUS;
        shape = SHAPE_PesAscendensOriscus;
        ltype = LG_ALL;
        break;
    case G_PES_DESCENDENS_ORISCUS:
        *type = AT_ONE_NOTE;
        *gtype = T_PES_ORISCUS;
        shape = SHAPE_PesDescendensOriscus;
        ltype = LG_ALL;
        break;
    case G_FLEXA:
        ambitus = first_pitch_of(glyph) - second_pitch_of(glyph);
        if (glyph->u.notes.liquescentia & L_DEMINUTUS) {
            *type = AT_FLEXUS_DEMINUTUS;
        } else {
            if (ambitus == 1) {
                *type = AT_FLEXUS_1;
            } else {
                *type = AT_FLEXUS;
            }
        }
        switch (glyph->u.notes.first_note->u.note.shape) {
        case S_ORISCUS_ASCENDENS:
            *gtype = T_FLEXUS_ORISCUS;
            shape = SHAPE_FlexusOriscusInusitatus;
            ltype = LG_NO_INITIO;
            break;

        case S_ORISCUS_DESCENDENS:
            *gtype = T_FLEXUS_ORISCUS;
            shape = SHAPE_FlexusOriscus;
            ltype = LG_NO_INITIO;
            break;

        case S_ORISCUS_SCAPUS_ASCENDENS:
            *gtype = T_FLEXUS_ORISCUS_SCAPUS;
            shape = flexus_shape(glyph, ambitus,
                    SHAPE_FlexusOriscusScapusInusitatus,
                    SHAPE_FlexusOriscusScapusInusitatusLongqueue,
                    SHAPE_FlexusOriscusScapusInusitatusOpenqueue);
            ltype = LG_NO_INITIO;
            break;

        case S_ORISCUS_SCAPUS_DESCENDENS:
            *gtype = T_FLEXUS_ORISCUS_SCAPUS;
            shape = flexus_shape(glyph, ambitus, SHAPE_FlexusOriscusScapus,
                    SHAPE_FlexusOriscusScapusLongqueue,
                    SHAPE_FlexusOriscusScapusOpenqueue);
            ltype = LG_NO_INITIO;
            break;

        default:
            *gtype = glyph->u.notes.fuse_to_next_glyph? T_PORRECTUS : T_FLEXUS;
            shape = flexus_shape(glyph, ambitus, SHAPE_Flexus,
                    SHAPE_FlexusLongqueue, SHAPE_FlexusOpenqueue);
            ltype = LG_NO_INITIO;
            break;
        }
        break;
    case G_TORCULUS:
        *gtype = T_TORCULUS;
        if (glyph->u.notes.first_note->u.note.shape == S_QUILISMA) {
            *type = AT_QUILISMA;
            shape = SHAPE_TorculusQuilisma;
            ltype = LG_NO_INITIO;
        } else {
            *type = AT_ONE_NOTE;
            shape = SHAPE_Torculus;
            ltype = LG_ALL;
        }
        break;
    case G_TORCULUS_LIQUESCENS:
        *gtype = T_TORCULUS_LIQUESCENS;
        if (glyph->u.notes.first_note->u.note.shape == S_QUILISMA) {
            *type = AT_QUILISMA;
            shape = SHAPE_TorculusLiquescensQuilisma;
        } else {
            *type = AT_ONE_NOTE;
            shape = SHAPE_TorculusLiquescens;
        }
        ltype = LG_ONLY_DEMINUTUS;
        break;
    case G_TORCULUS_RESUPINUS_FLEXUS:
        *type = AT_ONE_NOTE;
        *gtype = T_TORCULUS_RESUPINUS_FLEXUS;
        break;
    case G_PORRECTUS:
        *type = AT_PORRECTUS;
        *gtype = T_PORRECTUS;
        shape = porrectus_shape(glyph, SHAPE_Porrectus,
                SHAPE_PorrectusLongqueue);
        ltype = LG_NO_INITIO;
        break;
    case G_TORCULUS_RESUPINUS:
        *gtype = T_TORCULUS_RESUPINUS;
        if (glyph->u.notes.first_note->u.note.shape == S_QUILISMA) {
            *type = AT_QUILISMA;
            shape = SHAPE_TorculusResupinusQuilisma;
        } else {
            *type = AT_ONE_NOTE;
            shape = SHAPE_TorculusResupinus;
        }
        ltype = LG_ALL;
        break;
    case G_PORRECTUS_FLEXUS:
        *type = AT_PORRECTUS;
        *gtype = T_PORRECTUS_FLEXUS;
        shape = porrectus_shape(glyph, SHAPE_PorrectusFlexus,
                SHAPE_PorrectusFlexusLongqueue);
        ltype = LG_NO_INITIO;
        break;
    case G_PORRECTUS_NO_BAR:
        *type = AT_PORRECTUS;
        *gtype = T_TORCULUS_RESUPINUS;
        shape = SHAPE_PorrectusNobar;
        ltype = LG_NO_INITIO;
        break;
    case G_PORRECTUS_FLEXUS_NO_BAR:
        *type = AT_PORRECTUS;
        *gtype = T_TORCULUS_RESUPINUS_FLEXUS;
        shape = SHAPE_PorrectusFlexusNobar;
        ltype = LG_NO_INITIO;
        break;
    case G_ANCUS:
        *type = AT_ONE_NOTE;
        gregorio_assert(glyph->u.notes.liquescentia & L_DEMINUTUS,
                    gregoriotex_determine_glyph_name,
                    "encountered non-deminutus ancus", break);
        second_note = second_note_of(glyph);
        if (first_pitch_of(glyph) - second_note->u.note.pitch == 1) {
            *type = AT_FLEXUS_1;
        } else {
            *type = AT_FLEXUS;
        }
        *gtype = T_ANCUS;
        ambitus = first_pitch_of(glyph) - second_pitch_of(glyph);
        shape = flexus_shape(glyph, ambitus, SHAPE_Ancus,
                    SHAPE_AncusLongqueue, SHAPE_Ancus);
        ltype = LG_ONLY_DEMINUTUS;
        break;
    case G_SCANDICUS:
        *type = AT_ONE_NOTE;
        *gtype = T_SCANDICUS;
        shape = SHAPE_Scandicus;
        ltype = LG_NO_INITIO;
        break;
    case G_SALICUS:
        *type = AT_ONE_NOTE;
        *gtype = T_SALICUS;
        ambitus = third_pitch_of(glyph) - second_pitch_of(glyph);
        shape = flexus_shape(glyph, ambitus, SHAPE_Salicus,
                    SHAPE_SalicusLongqueue, SHAPE_Salicus);
        ltype = LG_NO_INITIO;
        break;
    case G_SALICUS_FLEXUS:
        *type = AT_ONE_NOTE;
        *gtype = T_SALICUS_FLEXUS;
        shape = SHAPE_SalicusFlexus;
        ltype = LG_NO_INITIO;
        break;
    case G_PUNCTUM_INCLINATUM:
    case G_PUNCTA_INCLINATA:
    case G_2_PUNCTA_INCLINATA_DESCENDENS:
    case G_3_PUNCTA_INCLINATA_DESCENDENS:
    case G_4_PUNCTA_INCLINATA_DESCENDENS:
    case G_5_PUNCTA_INCLINATA_DESCENDENS:
    case G_2_PUNCTA_INCLINATA_ASCENDENS:
    case G_3_PUNCTA_INCLINATA_ASCENDENS:
    case G_4_PUNCTA_INCLINATA_ASCENDENS:
    case G_5_PUNCTA_INCLINATA_ASCENDENS:
    case G_PUNCTUM:
    case G_STROPHA:
    case G_VIRGA:
    case G_VIRGA_REVERSA:
    case G_STROPHA_AUCTA:
    case G_DISTROPHA:
    case G_DISTROPHA_AUCTA:
    case G_TRISTROPHA:
    case G_TRISTROPHA_AUCTA:
    case G_BIVIRGA:
    case G_TRIVIRGA:
    case G_ALTERATION:
        *type = AT_ONE_NOTE;
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        gregorio_fail2(gregoriotex_determine_glyph_name,
                "called with unknown glyph: %s",
                gregorio_glyph_type_to_string(glyph->u.notes.glyph_type));
        break;
        /* LCOV_EXCL_STOP */
    }
    if (shape) {
        shape = compute_glyph_name(glyph, shape, ltype, false);
    }
    /* we fix *type with initio_debilis */
    if (*type == AT_ONE_NOTE) {
        if (is_initio_debilis(glyph->u.notes.liquescentia)) {
            *type = AT_INITIO_DEBILIS;
        }
    }

    return shape;
}

/**
 * This now does nothing useful, but we keep it here in case it may
 * be needed in future.
 */
static void gregoriotex_write_voice_info(FILE *f, gregorio_voice_info *voice_info)
{
    gregorio_assert(f && voice_info, gregoriotex_write_voice_info,
            "file or voice_info passed as NULL", return);
}

/* this function indicates if the syllable is the last of the line or score. */
static bool is_last_of_line(gregorio_syllable *syllable)
{
    gregorio_element *current_element = NULL;
    if (!(syllable->next_syllable) || ((syllable->next_syllable->elements)[0]
                && (syllable->next_syllable->elements)[0]->type
                == GRE_END_OF_LINE)) {
        /* the next syllable start by an end of line */
        return true;
    }
    if (syllable->elements) {
        current_element = (syllable->elements)[0];
        while (current_element) {
            if (current_element->type == GRE_END_OF_LINE) {
                /* we return true only if the end of line is the last element */
                if (!(current_element->next)) {
                    return true;
                } else {
                    return false;
                }
            } else {
                current_element = current_element->next;
            }
        }
    }
    return false;
}

/*
 * ! @brief Prints the beginning of each text style
 */
static void gtex_write_begin(FILE *f, grestyle_style style)
{
    if (style == gregoriotex_ignore_style) {
        return;
    }
    switch (style) {
    case ST_ITALIC:
        fprintf(f, "\\GreItalic{");
        break;
    case ST_SMALL_CAPS:
        fprintf(f, "\\GreSmallCaps{");
        break;
    case ST_BOLD:
        fprintf(f, "\\GreBold{");
        break;
    case ST_FORCED_CENTER:
    case ST_CENTER:
        fprintf(f, "}{");
        break;
    case ST_TT:
        fprintf(f, "\\GreTypewriter{");
        break;
    case ST_UNDERLINED:
        fprintf(f, "\\GreUnderline{");
        break;
    case ST_COLORED:
        fprintf(f, "\\GreColored{");
        break;
    case ST_ELISION:
        fprintf(f, "\\GreElision{");
        break;
    case ST_FIRST_WORD:
        fprintf(f, "\\GreFirstWord{");
        break;
    case ST_FIRST_SYLLABLE:
        fprintf(f, "\\GreFirstSyllable{");
        break;
    case ST_FIRST_SYLLABLE_INITIAL:
        fprintf(f, "\\GreFirstSyllableInitial{");
        break;
    case ST_PROTRUSION_FACTOR:
        fprintf(f, "\\GreProtrusion{");
        break;
    case ST_PROTRUSION:
        fprintf(f, "{");
        break;
    default:
        break;
    }
}

/**
 * @brief Ends each text style
 */
static void gtex_write_end(FILE *f, grestyle_style style)
{
    if (style == gregoriotex_ignore_style) {
        return;
    }
    switch (style) {
    case ST_FORCED_CENTER:
    case ST_CENTER:
    case ST_SYLLABLE_INITIAL:
        fprintf(f, "}{");
        break;
    case ST_INITIAL:
        break;
    default:
        fprintf(f, "}");
        break;
    }
}

static __inline void tex_escape_text(FILE *const f, const char *text)
{
    /* We escape these characters into \string\ddd (where ddd is the decimal
     * ASCII value of the character) for most escapes, and into \string\n for
     * newlines. We do it this way to get the "raw" string values through TeX
     * and into Lua, where the sequences become \ddd and \n respectively and
     * are translated into their byte values. Lua can then decide whether the
     * full strings should be evaluated by TeX as TeX or as strings */
    for (; *text; ++text) {
        switch (*text) {
        case '\\':
        case '{':
        case '}':
        case '~':
        case '%': /* currently, we'll never get %, but handle it anyway */
        case '#':
        case '"':
            /* these characters have special meaning to TeX */
            fprintf(f, "\\string\\%03d", *text);
            break;
        case '\n':
            fprintf(f, "\\string\\n");
            break;
        case '\r':
            /* ignore */
            break;
        default:
            /* UTF-8 multibyte sequences will fall into here, which is fine */
            fputc(*text, f);
            break;
        }
    }
}

static __inline void tex_escape_wtext(FILE *const f, const grewchar *text)
{
    /* We escape these characters into \string\ddd (where ddd is the decimal
     * ASCII value of the character) for most escapes, and into \string\n for
     * newlines. We do it this way to get the "raw" string values through TeX
     * and into Lua, where the sequences become \ddd and \n respectively and
     * are translated into their byte values. Lua can then decide whether the
     * full strings should be evaluated by TeX as TeX or as strings */
    for (; *text; ++text) {
        switch (*text) {
        case L'\\':
        case L'{':
        case L'}':
        case L'~':
        case L'%': /* currently, we'll never get %, but handle it anyway */
        case L'#':
        case L'"':
            /* these characters have special meaning to TeX */
            fprintf(f, "\\string\\%03d", *text);
            break;
        /* There is currently no way to get a carriage return or a newline into
         * an <sp>, but we'll leave this code here, but #if'd out, in case we
         * need this function in a different context */
#if 0
        case L'\n':
            fprintf(f, "\\string\\n");
            break;
        case L'\r':
            / * ignore * /
            break;
#endif
        default:
            gregorio_print_unichar(f, *text);
            break;
        }
    }
}

/*
 * ! @brief Writes GregorioTeX special characters. This function takes the
 * special characters as input (i.e. from gabc representation), and writes them
 * * in GregorioTeX form.
 */
static void gtex_write_special_char(FILE *f, const grewchar *special_char)
{
    fprintf(f, "\\GreSpecial{");
    tex_escape_wtext(f, special_char);
    fprintf(f, "}");
}

static void gtex_write_verb(FILE *f, const grewchar *first_char)
{
    gregorio_print_unistring(f, first_char);
}

static void gtex_print_char(FILE *f, const grewchar to_print)
{
    switch (to_print) {
    case L'*':
        fprintf(f, "\\GreStar{}");
        break;
    case L'%':
        /* there's currently no way to get a % into gabc, so this wont be hit,
         * but we leave it here for safety and possible future use */
        /* LCOV_EXCL_START */
        fprintf(f, "\\%%{}");
        break;
        /* LCOV_EXCL_STOP */
    case L'\\':
        fprintf(f, "\\textbackslash{}");
        break;
    case L'&':
        fprintf(f, "\\&{}");
        break;
    case L'#':
        fprintf(f, "\\#{}");
        break;
    case L'+':
        fprintf(f, "\\GreDagger{}");
        break;
    case L'_':
        fprintf(f, "\\_{}");
        break;
    case L'-':
        fprintf(f, "\\GreHyph{}");
        break;
    default:
        gregorio_print_unichar(f, to_print);
        break;
    }
    return;
}

/* a function to map the internal ST_* styles to gregoriotex styles as defined
 * in gregoriotex-syllables.tex */
static unsigned char gregoriotex_internal_style_to_gregoriotex(
        const grestyle_style style)
{
    switch (style) {
    case ST_ITALIC:
        return 1;
    case ST_BOLD:
        return 2;
    case ST_SMALL_CAPS:
        return 3;
    case ST_TT:
        return 4;
    case ST_UNDERLINED:
        return 5;
    case ST_COLORED:
        return 6;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        gregorio_fail2(gregoriotex_internal_style_to_gregoriotex,
                "unrecognized style: %s", grestyle_style_to_string(style));
        return 0;
        /* LCOV_EXCL_STOP */
    }
}

/*
 * A quite hacky function: when we have only one style (typically italic) and
 * when this style is on all the parts, then we return this style.
 *
 */
static grestyle_style gregoriotex_fix_style(
        const gregorio_character *current_char)
{
    grestyle_style possible_fixed_style = ST_NO_STYLE;
    grestyle_style in_fixed_style = ST_NO_STYLE;
    while (current_char) {
        if (!in_fixed_style) {
            /* before the first style */
            if (current_char->is_character) {
                /* got some character, so no future style can apply to the
                 * entire syllable */
                return ST_NO_STYLE;
            }
            if (current_char->cos.s.type == ST_T_BEGIN) {
                switch (current_char->cos.s.style) {
                case ST_VERBATIM:
                case ST_SPECIAL_CHAR:
                    /* these are pseudo-characters, and if they appear
                     * before the first style, then the style does not
                     * apply to the entire syllable */
                    return ST_NO_STYLE;

                case ST_ITALIC:
                case ST_BOLD:
                case ST_TT:
                case ST_SMALL_CAPS:
                case ST_UNDERLINED:
                case ST_COLORED:
                    if (possible_fixed_style) {
                        if (current_char->cos.s.style != possible_fixed_style) {
                            /* found a differing style */
                            return ST_NO_STYLE;
                        } else {
                            /* same style which may apply to the entire
                             * syllable */
                            in_fixed_style = possible_fixed_style;
                        }
                    } else {
                        /* we found a candidate fixed style */
                        in_fixed_style = possible_fixed_style =
                                current_char->cos.s.style;
                    }
                    break;

                default:
                    /* anything else is "transparent" before the first real
                     * or pseudo-character */
                    break;
                }
            }
            /* else it's an end of a style, which we treat as "transparent"
             * noise */
        } else {
            /* we have a possible style */
            if (!current_char->is_character) {
                if (current_char->cos.s.type == ST_T_BEGIN) {
                    switch (current_char->cos.s.style) {
                    case ST_ITALIC:
                    case ST_BOLD:
                    case ST_TT:
                    case ST_SMALL_CAPS:
                    case ST_UNDERLINED:
                    case ST_COLORED:
                        if (current_char->cos.s.style != possible_fixed_style) {
                            /* found a differing style */
                            return ST_NO_STYLE;
                        }
                        /* else it's a (nested) open of the same style, which
                         * doesn't change the style; however, nested styles
                         * (should) have been eliminated by now */
                        /* LCOV_EXCL_START */
                        gregorio_fail(gregoriotex_fix_style, "encountered a "
                                "nested style which should have been removed "
                                "by now");
                        break;
                        /* LCOV_EXCL_STOP */

                    default:
                        /* anything else is a pseudo-character or a
                         * "transparent" style, which don't affect the fixed
                         * style */
                        break;
                    }
                } else if (current_char->cos.s.type == ST_T_END) {
                    if (current_char->cos.s.style == possible_fixed_style) {
                        /* we closed the possible fixed style; we don't return
                         * yet to give it the chance to re-open */
                        in_fixed_style = ST_NO_STYLE;
                    }
                    /* else it's a close of something we don't care about or a
                     * style that wasn't opened, so for the purposes of this
                     * function, these are "transparent" and don't affect the
                     * fixed style */
                }
            }
            /* else it's a character, and that doesn't affect the fixed style */
        }
        current_char = current_char->next_character;
    }
    return possible_fixed_style;
}

/*
 * @brief Writes the translation.
 *
 * There is no special handling of translation text; that is, we just print the
 * entire string of text under the normal text line, without considering any
 * special centering or linebreaks.
 */
static __inline void write_translation(FILE *f,
        gregorio_character *translation)
{
    gregorio_not_null(translation, write_translation, return);
    gregorio_write_text(WTP_NORMAL, translation, f, &gtex_write_verb,
            &gtex_print_char, &gtex_write_begin, &gtex_write_end,
            &gtex_write_special_char);
}

/* a function to compute the height of the flat of a key
 * the flat is always on the line of the */

static char clef_flat_height(gregorio_clef clef, signed char line, bool flatted)
{
    char offset = 6;

    if (!flatted) {
        return NO_KEY_FLAT;
    }

    switch (clef) {
    case CLEF_C:
        switch (line) {
        case 1:
            offset = 2;
            break;
        case 2:
            offset = 4;
            break;
        case 3:
            offset = 6;
            break;
        case 4:
            offset = 8;
            break;
        case 5:
            offset = 10;
            break;
        default:
            /* not reachable unless there's a programming error */
            /* LCOV_EXCL_START */
            gregorio_fail2(clef_flat_height, "unknown line number: %d", line);
            break;
            /* LCOV_EXCL_STOP */
        }
        break;
    case CLEF_F:
        switch (line) {
        case 1:
            offset = 6;
            break;
        case 2:
            offset = 8;
            break;
        case 3:
            offset = 3;
            break;
        case 4:
            offset = 5;
            break;
        case 5:
            offset = 7;
            break;
        default:
            /* not reachable unless there's a programming error */
            /* LCOV_EXCL_START */
            gregorio_fail2(clef_flat_height, "unknown line number: %d", line);
            break;
            /* LCOV_EXCL_STOP */
        }
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        gregorio_fail2(clef_flat_height, "unknown clef type: %d", clef);
        break;
        /* LCOV_EXCL_STOP */
    }

    return pitch_value(LOWEST_PITCH + offset);
}

static __inline bool is_manual_custos(const gregorio_element *const element)
{
    return element->type == GRE_CUSTOS && element->u.misc.pitched.force_pitch;
}

OFFSET_CASE(BarStandard);
OFFSET_CASE(BarVirgula);
OFFSET_CASE(BarDivisioFinalis);

static void write_bar(FILE *f, const gregorio_score *const score,
        const gregorio_syllable *const syllable,
        const gregorio_element *const element,
        const unsigned char first_of_disc)
{
    const gregorio_bar type = element->u.misc.unpitched.info.bar;
    const gregorio_sign signs = element->u.misc.unpitched.special_sign;
    const bool is_inside_bar = element->next && !is_manual_custos(element->next)
            && element->next->type != GRE_END_OF_LINE;
    const bool has_text = !element->previous && syllable->text;
    /* the type number of function vepisemaorrare */
    const char *offset_case = BarStandard;
    signed char far_pitch_adjustment = 0;
    /* don't use "In" version of bars in the first argument of a GreDiscretionary */
    if (is_inside_bar && first_of_disc != 1) {
        fprintf(f, "\\GreIn");
    } else {
        fprintf(f, "\\Gre");
    }
    switch (type) {
    case B_VIRGULA:
        fprintf(f, "Virgula");
        offset_case = BarVirgula;
        break;
    case B_DIVISIO_MINIMA:
        fprintf(f, "DivisioMinima");
        break;
    case B_DIVISIO_MINOR:
        fprintf(f, "DivisioMinor");
        break;
    case B_DIVISIO_MAIOR:
        fprintf(f, "DivisioMaior");
        break;
    case B_DIVISIO_FINALIS:
        fprintf(f, "DivisioFinalis");
        offset_case = BarDivisioFinalis;
        break;
    case B_DIVISIO_MINOR_D1:
        fprintf(f, "Dominica{1}");
        break;
    case B_DIVISIO_MINOR_D2:
        fprintf(f, "Dominica{2}");
        break;
    case B_DIVISIO_MINOR_D3:
        fprintf(f, "Dominica{3}");
        break;
    case B_DIVISIO_MINOR_D4:
        fprintf(f, "Dominica{4}");
        break;
    case B_DIVISIO_MINOR_D5:
        fprintf(f, "Dominica{5}");
        break;
    case B_DIVISIO_MINOR_D6:
        fprintf(f, "Dominica{6}");
        break;
    case B_DIVISIO_MINOR_D7:
        fprintf(f, "Dominica{7}");
        break;
    case B_DIVISIO_MINOR_D8:
        fprintf(f, "Dominica{8}");
        break;
    case B_VIRGULA_HIGH:
        fprintf(f, "VirgulaHigh");
        offset_case = BarVirgula;
        break;
    case B_DIVISIO_MINIMA_HIGH:
        fprintf(f, "DivisioMinimaHigh");
        break;
    case B_DIVISIO_MAIOR_DOTTED:
        fprintf(f, "DivisioMaiorDotted");
        break;
    case B_DIVISIO_MINIMIS:
        fprintf(f, "DivisioMinimis");
        break;
    case B_DIVISIO_MINIMIS_HIGH:
        fprintf(f, "DivisioMinimisHigh");
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        gregorio_fail2(write_bar, "unknown bar type: %d", type);
        break;
        /* LCOV_EXCL_STOP */
    }
    fprintf(f, "{%c}", has_text? '1' : '0');
    switch (signs) {
    case _V_EPISEMA:
        fprintf(f, "{\\GreBarVEpisema{\\GreOCase%s}}", offset_case);
        break;
    case _BAR_H_EPISEMA:
        fprintf(f, "{\\GreBarBrace{\\GreOCase%s}}", offset_case);
        break;
    case _V_EPISEMA_BAR_H_EPISEMA:
        fprintf(f, "{\\GreBarBrace{\\GreOCase%s}"
                "\\GreBarVEpisema{\\GreOCase%s}}",
                offset_case, offset_case);
        break;
    default:
        fprintf(f, "{}");
        break;
    }
    switch (type) {
    case B_DIVISIO_MINIMIS_HIGH:
        ++ far_pitch_adjustment;
        /* fall through */

    case B_VIRGULA_HIGH:
    case B_DIVISIO_MINIMA_HIGH:
        ++ far_pitch_adjustment;
        /* fall through */

    case B_DIVISIO_MINIMIS:
        ++ far_pitch_adjustment;
        /* fall through */

    case B_VIRGULA:
    case B_DIVISIO_MINIMA:
        {
            char is_far = '0';
            const gregorio_element *e;
            const gregorio_syllable *s;
            const gregorio_glyph *g;
            const gregorio_note *n;
            signed char pitch;

            /* find the prior element */
            e = element->previous;
            if (!e) {
                for (s = syllable->previous_syllable; s; s = s->previous_syllable) {
                    /* loop to find the previous syllable with elements */
                    if (s->elements && *s->elements) {
                        for (e = *s->elements; e->next; e = e->next) {
                            /* just loop to find the last element */
                        }
                        break;
                    }
                }
            }
            if (e && e->type == GRE_ELEMENT) {
                g = e->u.first_glyph;
                if (g) {
                    while (g->next) {
                        /* loop to find the last glyph in the prior element */
                        g = g->next;
                    }
                }
                if (g && g->type == GRE_GLYPH && (n = g->u.notes.first_note)) {
                    while (n->next) {
                        /* loop to find the last note */
                        n = n->next;
                    }
                    pitch = n->u.note.pitch;
                    if (g->u.notes.liquescentia & L_DEMINUTUS && n->previous
                            && n->previous->u.note.pitch > pitch) {
                        pitch = n->previous->u.note.pitch;
                    }
                    if (pitch < score->virgula_far_pitch + far_pitch_adjustment) {
                        /* find next element */
                        e = element->next;
                        if (!e) {
                            for (s = syllable->next_syllable; s;
                                    s = s->next_syllable) {
                                /* loop to find the next syllable with elements */
                                if (s->elements && *s->elements) {
                                    e = *s->elements;
                                    break;
                                }
                            }
                        }
                        if (e) {
                            if (e->type == GRE_END_OF_LINE) {
                                is_far = '1';
                            } else if (e->type == GRE_ELEMENT) {
                                g = e->u.first_glyph;
                                if (g && g->type == GRE_GLYPH) {
                                    n = g->u.notes.first_note;
                                    if (n) {
                                        if (g->u.notes.glyph_type == G_PODATUS
                                                && n->next) {
                                            n = n->next;
                                        }
                                        if (n->u.note.pitch
                                                < score->virgula_far_pitch
                                                + far_pitch_adjustment) {
                                            is_far = '1';
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            fprintf(f, "{%c}%%\n", is_far);
        }
        break;

    default:
        fprintf(f, "%%\n");
        break;
    }
}

static __inline char *suppose_high_ledger_line(const gregorio_note *const note)
{
    return note->high_ledger_line? "\\GreSupposeHighLedgerLine" : "";
}

static __inline char *suppose_low_ledger_line(const gregorio_note *const note)
{
    return note->low_ledger_line? "\\GreSupposeLowLedgerLine" : "";
}

/*
 * ! @brief Writes augmentum duplexes (double dots) We suppose we are on the
 * last note. \n The algorithm is the following: if there is a previous note,
 * we consider that the two puncta of the augumentum duplex must correspond to
 * the last note and the previous note. If we are adding to a single note
 * glyph, which would be weird but sure why not, we just typeset two puncta
 * spaced of 2.
 */
static void write_auctum_duplex(FILE *f,
        const gregorio_note *const current_note)
{
    char pitch = current_note->u.note.pitch;
    char previous_pitch = 0;
    /* second_pitch is the second argument of the \augmentumduplex macro,
     * that's what this function is all about. */
    char second_pitch = 0;
    /* this variable will be set to 1 if we are on the note before the last
     * note of a podatus or a porrectus or a torculus resupinus */
    unsigned char special_punctum = 0;
    if (current_note->previous) {
        if (current_note->previous->u.note.pitch - current_note->u.note.pitch ==
                -1
                || current_note->previous->u.note.pitch -
                current_note->u.note.pitch == 1) {
            special_punctum = 1;
        }
        second_pitch = current_note->previous->u.note.pitch;
        previous_pitch = second_pitch;
    }

    if (!previous_pitch || previous_pitch == pitch) {
        if (is_on_a_line(pitch)) {
            second_pitch = pitch;
            special_punctum = 1;
        } else {
            second_pitch = pitch + 1;
        }
    }
    /* the first argument should always be the lowest one, that's what we do
     * here: */
    if (pitch > second_pitch) {
        previous_pitch = pitch;
        pitch = second_pitch;
        second_pitch = previous_pitch;
    }
    /* maybe the third argument should be changed */
    fprintf(f, "\\GreAugmentumDuplex{%d}{%d}{%d}%%\n", pitch_value(pitch),
            pitch_value(second_pitch), special_punctum);
}

/**
 * @brief Adds a dot.
 *
 * Writes \c \\GrePunctumMora in the gtex file, with the appropriate arguments. You might think this function
 * more straightforward than it actually is...
 */
static void write_punctum_mora(FILE *f, const gregorio_glyph *glyph,
        const gregorio_note *current_note, const int fuse_to_next_note)
{
    /* in this if we consider that the puncta are only on the last two notes
     * (maybe it would be useful to consider it more entirely, but it would be
     * really weird...) */
    /* the variable that will be set to true if we have to shift the punctum
     * inclinatum before the last note */
    bool shift_before = false;
    /* this variable will be set to 1 if we are on the note before the last
     * note of a podatus or a porrectus or a torculus resupinus */
    unsigned char special_punctum = 0;
    /* 0 if space is normal, 1 if there should be no space after a punctum */
    unsigned char no_space = 0;
    /* the pitch where to set the punctum */
    char pitch = current_note->u.note.pitch;
    /* a variable to know if we are on a punctum inclinatum or not */
    unsigned char punctum_inclinatum = 0;
    /* a temp variable */
    const gregorio_note *tmpnote;
    /* we go into this switch only if it is the note before the last note */
    if (current_note->next) {
        switch (glyph->u.notes.glyph_type) {
        case G_FLEXA:
        case G_TORCULUS:
        case G_TORCULUS_RESUPINUS_FLEXUS:
        case G_PORRECTUS_FLEXUS:
            if (!(glyph->u.notes.liquescentia & L_DEMINUTUS)) {
                shift_before = true;
            }
            if (is_between_lines(pitch)
                    && pitch - current_note->next->u.note.pitch == 1) {
                special_punctum = 2;
            }
            break;
        case G_PODATUS:
            if ((current_note->u.note.shape != S_PUNCTUM
                        && current_note->u.note.shape != S_QUILISMA)
                    || (glyph->u.notes.liquescentia
                        & (L_AUCTUS_DESCENDENS | L_AUCTUS_ASCENDENS))) {
                shift_before = true;
                /* fine tuning */
                if (current_note->next->u.note.pitch -
                        current_note->u.note.pitch == 1) {
                    if (is_on_a_line(current_note->u.note.pitch)) {
                        special_punctum = 1;
                    } else {
                        pitch = current_note->u.note.pitch - 1;
                    }
                }
            } else {
                /* case for f.g */
                if (current_note->next->u.note.pitch -
                        current_note->u.note.pitch == 1) {
                    special_punctum = 1;
                }
            }
            break;
        case G_PES_QUADRATUM:
            shift_before = true;
            if (current_note->next->u.note.pitch - current_note->u.note.pitch ==
                    1) {
                if (is_on_a_line(current_note->u.note.pitch)) {
                    special_punctum = 1;
                } else {
                    pitch = current_note->u.note.pitch - 1;
                }
            }
            break;
        case G_PORRECTUS:
        case G_TORCULUS_RESUPINUS:
            /* this case is only for the note before the previous note */
            if ((current_note->next->u.note.pitch -
                            current_note->u.note.pitch == -1
                            || current_note->next->u.note.pitch -
                            current_note->u.note.pitch == 1)
                    && !(current_note->next->next))
                special_punctum = 1;
            break;
        default:
            break;
        }
    }
    /* we enter here in any case */
    switch (glyph->u.notes.glyph_type) {
    case G_PUNCTA_INCLINATA:
    case G_2_PUNCTA_INCLINATA_DESCENDENS:
    case G_3_PUNCTA_INCLINATA_DESCENDENS:
    case G_4_PUNCTA_INCLINATA_DESCENDENS:
    case G_5_PUNCTA_INCLINATA_DESCENDENS:
    case G_TORCULUS:
        if (!current_note->next) {
            special_punctum = 1;
        }
        break;
    default:
        break;
    }
    switch (current_note->u.note.shape) {
    case S_PUNCTUM_INCLINATUM_ASCENDENS:
    case S_PUNCTUM_INCLINATUM_STANS:
    case S_PUNCTUM_INCLINATUM_DESCENDENS:
    case S_PUNCTUM_INCLINATUM_DEMINUTUS:
        punctum_inclinatum = 1;
        break;
    default:
        break;
    }
    /* when the punctum mora is on a note on a line, and the prior note is on
     * the space immediately above, the dot is placed on the space below the
     * line instead */
    if (current_note->previous
            && (current_note->previous->u.note.pitch -
                    current_note->u.note.pitch == 1)
            && is_on_a_line(current_note->u.note.pitch)
            && (current_note->previous->signs == _PUNCTUM_MORA
                    || current_note->previous->signs == _V_EPISEMA_PUNCTUM_MORA
                    || current_note->previous->choral_sign)) {
        special_punctum = 1;
    }

    if (current_note->mora_vposition) {
        special_punctum = current_note->mora_vposition == VPOS_BELOW? 1 : 0;
    }

    if (shift_before) {
        if (current_note->next->u.note.pitch - current_note->u.note.pitch == -1
                || current_note->next->u.note.pitch -
                current_note->u.note.pitch == 1) {
            fprintf(f, "\\GrePunctumMora{%d}{3}{%d}{%d}%%\n",
                    pitch_value(pitch), special_punctum, punctum_inclinatum);
        } else {
            fprintf(f, "\\GrePunctumMora{%d}{2}{%d}{%d}%%\n",
                    pitch_value(pitch), special_punctum, punctum_inclinatum);
        }
        return;
    }
    /* There are two special cases. The first: if the next glyph is a
     * ZERO_WIDTH_SPACE, and the current glyph is a PES, and the punctum mora
     * is on the first note, and the first note of the next glyph is at least
     * two (or three depending on something) pitches higher than the current
     * note.
     * You'll all have understood, this case is quite rare... but when it
     * appears, we pass 1 as a second argument of \punctummora so that it
     * removes the space introduced by the punctummora. */
    if (glyph->u.notes.glyph_type == G_PODATUS && glyph->next
            && glyph->next->type == GRE_SPACE
            && glyph->next->u.misc.unpitched.info.space == SP_ZERO_WIDTH
            && current_note->next && glyph->next->next
            && glyph->next->next->type == GRE_GLYPH
            && glyph->next->next->u.notes.first_note
            && (glyph->next->next->u.notes.first_note->u.note.pitch -
                    current_note->u.note.pitch > 1)) {
        fprintf(f, "\\GrePunctumMora{%d}{1}{%d}{%d}%%\n", pitch_value(pitch),
                special_punctum, punctum_inclinatum);
        return;
    }
    /* if there is a punctum or a auctum dumplex on a note after, we put a
     * zero-width punctum */
    tmpnote = current_note->next;
    while (tmpnote) {
        if (tmpnote->signs == _PUNCTUM_MORA || tmpnote->signs == _AUCTUM_DUPLEX
                || tmpnote->signs == _V_EPISEMA_PUNCTUM_MORA
                || tmpnote->signs == _V_EPISEMA_AUCTUM_DUPLEX
                || tmpnote->choral_sign) {
            no_space = 1;
            break;
        }
        tmpnote = tmpnote->next;
    }
    /* use a special no-space punctum mora for ascending fusion */
    if (fuse_to_next_note) {
        no_space = 1;
        if (fuse_to_next_note > 0) {
            special_punctum = 1;
        }
    }

    /* the normal operation */
    fprintf(f, "\\GrePunctumMora{%d}{%d}{%d}{%d}%%\n", pitch_value(pitch),
            no_space, special_punctum, punctum_inclinatum);
}

static __inline int get_punctum_inclinatum_space_case(
        const gregorio_note *const note)
{
    char temp;

    switch (note->u.note.shape) {
    case S_PUNCTUM_INCLINATUM_ASCENDENS:
        if (note->previous) {
            /* means that it is not the first note of the puncta inclinata
             * sequence */
            temp = note->previous->u.note.pitch - note->u.note.pitch;
            /* negative values = ascending ambitus */
            /* not sure we ever need to consider a larger ambitus here */
            switch (temp) {
            case 1:
            case 2:
            case 3:
            case 4:
                return 25;
            case -1:
            default:
                return 12;
            case -2:
                return 14;
            case -3:
            case -4:
                return 15;
            }
        }
        break;
    case S_PUNCTUM_INCLINATUM_STANS:
        if (note->previous) {
            /* means that it is not the first note of the puncta inclinata
             * sequence */
            return 26;
        }
        break;
    case S_PUNCTUM_INCLINATUM_DESCENDENS:
        if (note->previous) {
            /* means that it is not the first note of the puncta inclinata
             * sequence */
            temp = note->previous->u.note.pitch - note->u.note.pitch;
            /* negative values = ascending ambitus */
            /* not sure we ever need to consider a larger ambitus here */
            switch (temp) {
            case 1:
            default:
                return 3;
            case 2:
                return 10;
            case 3:
            case 4:
                return 11;
            case -1:
            case -2:
            case -3:
            case -4:
                return 24;
            }
        }
        break;
    case S_PUNCTUM_INCLINATUM_DEMINUTUS:
        if (note->previous) {
            /* means that it is not the first note of the puncta inclinata
             * sequence */
            temp = note->previous->u.note.pitch - note->u.note.pitch;
            if (temp < -2) {
                return 15;
            } else if (temp > 2) {
                return 11;
            } else {
                if (note->previous
                        && note->previous->u.note.shape ==
                        S_PUNCTUM_INCLINATUM_DEMINUTUS) {
                    if (temp < -1) {
                        /* really if the ambitus = 3rd at this point */
                        return 14;
                    } else if (temp > 1) {
                        /* really if the ambitus = 3rd at this point */
                        return 10;
                    } else {
                        /* temp == 0, so there is no ascending case */
                        return 8;
                    }
                } else {
                    if (temp < 0) {
                        /* puncta inclinatum followed by puncta inclinatum
                         * debilis */
                        return 13;
                    } else if (temp > 0) {
                        /* puncta inclinatum followed by puncta inclinatum
                         * debilis */
                        return 7;
                    }
                }
            }
        } /* LCOV_EXCL_LINE */
        break;
    case S_PUNCTUM_INCLINATUM_AUCTUS:
        if (note->previous) {
            /* means that it is not the first note of the puncta inclinata
             * sequence */
            temp = note->previous->u.note.pitch - note->u.note.pitch;
            if (temp < -1 || temp > 1) {
                /* this is the normal interglyph space, so we'll use it for
                 * either direction */
                return 1;
            } else {
                /* temp == 0, so there is no ascending case */
                /* we approximate that it is the same space */
                return 3;
            }
        }
        break;
    default:
        break;
    }

    return -1;
}

static __inline int get_punctum_inclinatum_to_nobar_space_case(
        const gregorio_glyph *const glyph)
{
    if (glyph->u.notes.glyph_type <= G_PUNCTA_INCLINATA) {
        const gregorio_glyph *next = gregorio_next_non_texverb_glyph(glyph);
        if (next && next->type == GRE_GLYPH
                && (next->u.notes.glyph_type == G_PUNCTUM
                    || (next->u.notes.glyph_type == G_FLEXA
                        && !next->u.notes.fuse_to_next_glyph))) {
            int descent;
            const gregorio_note *note = gregorio_glyph_last_note(glyph);
            descent = note->u.note.pitch -
                glyph->next->u.notes.first_note->u.note.pitch;
            /* a negative descent is an ascent */
            switch(descent) {
            case -1:
                return 19;
            case 1:
                return 16;
            case -2:
                return 20;
            case 2:
                return 17;
            case -3:
            case -4:
                return 21;
            case 3:
            case 4:
                return 18;
            }
        }
    }

    return -1;
}

static __inline void write_single_hepisema(FILE *const f, int hepisema_case,
        const gregorio_note *const note, gregorio_sign_orientation orientation,
        bool connect, char height, const grehepisema_size size, const int i,
        const gregorio_glyph *const glyph,
        const int porrectus_long_episema_index,
        bool (*const is_episema_shown)(const gregorio_note *))
{
    if (height) {
        const gregorio_hepisema_adjustment *adj =
                gregorio_get_hepisema_adjustment(
                        note->he_adjustment_index[orientation]);
        char ambitus = 0;
        char size_arg;

        switch (size) {
        case H_SMALL_LEFT:
            size_arg = 'l';
            connect = false;
            break;
        case H_SMALL_CENTRE:
            size_arg = 'c';
            connect = false;
            break;
        case H_SMALL_RIGHT:
            size_arg = 'r';
            break;
        default:
            size_arg = 'f';
            break;
        }

        if (i == porrectus_long_episema_index && note->next
                && is_episema_shown(note->next)) {
            ambitus = compute_ambitus(note);
        }

        if (i - 1 != porrectus_long_episema_index || !note->previous
                || !is_episema_shown(note->previous)) {
            if (connect) {
                const gregorio_glyph *next;
                if (!note->next
                        && (!(next = gregorio_next_non_texverb_glyph(glyph))
                            || next->type != GRE_SPACE
                            || next->u.misc.unpitched.info.space
                            != SP_ZERO_WIDTH)) {
                    /* not followed by a zero-width space */
                    /* try to fuse from punctum inclinatum to nobar glyph */
                    fprintf(f,
                            "\\GreHEpisemaBridge{%d}{%d}{%d}{%s%s}{%s}{%d}%%\n",
                            pitch_value(height), hepisema_case,
                            get_punctum_inclinatum_to_nobar_space_case(glyph),
                            suppose_high_ledger_line(note),
                            suppose_low_ledger_line(note),
                            adj->nudge? adj->nudge : "", adj->vbasepos);
                } else if (note->next
                        && (note->next->u.note.shape
                            == S_PUNCTUM_INCLINATUM_ASCENDENS
                            || note->next->u.note.shape
                            == S_PUNCTUM_INCLINATUM_DESCENDENS
                            || note->next->u.note.shape
                            == S_PUNCTUM_INCLINATUM_DEMINUTUS
                            || note->next->u.note.shape
                            == S_PUNCTUM_INCLINATUM_AUCTUS)) {
                    /* is a punctum inclinatum of some sort */
                    fprintf(f, "\\GreHEpisemaBridge{%d}{%d}{%d}{%s%s}{%s}{%d}%%\n",
                            pitch_value(height), hepisema_case,
                            get_punctum_inclinatum_space_case(note->next),
                            suppose_high_ledger_line(note),
                            suppose_low_ledger_line(note),
                            adj->nudge? adj->nudge : "", adj->vbasepos);
                }
            }
            fprintf(f, "\\GreHEpisema{%d}{\\GreOCase%s}{%d}{%d}{%c}{%d}"
                    "{%s%s}{%s}{%d}%%\n", pitch_value(height),
                    note->gtex_offset_case, ambitus, hepisema_case, size_arg,
                    pitch_value(height), suppose_high_ledger_line(note),
                    suppose_low_ledger_line(note), adj->nudge? adj->nudge : "",
                    adj->vbasepos);
        }
    }
}

/**
 * @brief A function that writes the good \c \\hepisema in GregorioTeX.
 * @param i The position of the note in the glyph.
 */
static void gregoriotex_write_hepisema(FILE *const f,
        const gregorio_note *const note, const int i, const gtex_type type,
        const gregorio_glyph *const glyph)
{
    int porrectus_long_episema_index = -1;

    gregorio_assert(note, gregoriotex_write_hepisema,
            "called with NULL pointer", return);

    switch (type) {
    case T_PORRECTUS:
    case T_PORRECTUS_FLEXUS:
        porrectus_long_episema_index = 1;
        break;
    case T_TORCULUS_RESUPINUS:
    case T_TORCULUS_RESUPINUS_FLEXUS:
        porrectus_long_episema_index = 2;
        break;
    default:
        /* do nothing */
        break;
    }

    write_single_hepisema(f, 1, note, SO_UNDER, note->h_episema_below_connect,
            note->h_episema_below, note->h_episema_below_size, i, glyph,
            porrectus_long_episema_index, &gtex_is_h_episema_below_shown);
    write_single_hepisema(f, 0, note, SO_OVER, note->h_episema_above_connect,
            note->h_episema_above, note->h_episema_above_size, i, glyph,
            porrectus_long_episema_index, &gtex_is_h_episema_above_shown);
}

/* a macro to write an additional line */

static void write_additional_line(FILE *f, int i, gtex_type type, bool bottom,
        const gregorio_note *current_note, const gregorio_score *const score)
{
    char ambitus = 0;
    gregorio_assert(current_note, write_additional_line, "called with no note",
            return);
    /* patch to get a line under the full glyph in the case of dbc (for
     * example) */
    switch (type) {
    case T_PORRECTUS:
    case T_PORRECTUS_FLEXUS:
        switch (i) {
        case 1:
            i = HEPISEMA_FIRST_TWO;
            break;
        case 2:
            if (current_note->previous->u.note.pitch > LOW_LEDGER_LINE_PITCH
                    && current_note->previous->u.note.pitch
                    < score->high_ledger_line_pitch) {
                i = HEPISEMA_FIRST_TWO;
                /* HEPISEMA_FIRST_TWO works only for first note */
                current_note = current_note->previous;
            } else {
                return;
            }
            break;
        case 3:
            if (bottom || current_note->previous->u.note.pitch
                    >= score->high_ledger_line_pitch) {
                /* we don't need to add twice the same line */
                return;
            }
            break;
        }
        break;
    case T_TORCULUS_RESUPINUS:
    case T_TORCULUS_RESUPINUS_FLEXUS:
        switch (i) {
        case 2:
            i = HEPISEMA_FIRST_TWO;
            break;
        case 3:
            if (current_note->previous->u.note.pitch > LOW_LEDGER_LINE_PITCH
                    && current_note->previous->u.note.pitch
                    < score->high_ledger_line_pitch) {
                i = HEPISEMA_FIRST_TWO;
                /* HEPISEMA_FIRST_TWO works only for first note */
                current_note = current_note->previous;
            } else {
                return;
            }
            break;
        case 4:
            if (bottom || current_note->previous->u.note.pitch
                    >= score->high_ledger_line_pitch) {
                /* we don't need to add twice the same line */
                return;
            }
            break;
        }
        break;
    default:
        break;
    }

    if (i == HEPISEMA_FIRST_TWO) {
        /* here we must compare the first note of the big bar with the second
         * one, but it may be tricky sometimes, because of the previous patch */
        gregorio_assert_only(!current_note->previous ||
                current_note->u.note.pitch
                > current_note->previous->u.note.pitch,
                write_additional_line, "HEPISEMA_FIRST_TWO on the wrong note");
        ambitus = current_note->u.note.pitch - current_note->next->u.note.pitch;
    }
    fprintf(f, "\\GreAdditionalLine{\\GreOCase%s}{%d}{%d}%%\n",
            current_note->gtex_offset_case, ambitus, bottom ? 3 : 2);
}

/*
 *
 * a function that writes the good value of \vepisema in GregorioTeX. i is the
 * position of the note in the glyph
 *
 */

static void write_vepisema(FILE *f, const gregorio_note *const note)
{
    fprintf(f, "\\GreVEpisema{%d}{\\GreOCase%s}%%\n",
            pitch_value(note->v_episema_height), note->gtex_offset_case);
}

/*
 * a function that writes the rare signs in GregorioTeX. i is the position of
 * the note in the glyph */

static void write_rare_sign(FILE *f, const gregorio_note *const current_note,
        const gregorio_sign rare)
{
    switch (rare) {
    case _ACCENTUS:
        fprintf(f, "\\GreAccentus{%d}{\\GreOCase%s}%%\n",
                pitch_value(current_note->u.note.pitch),
                current_note->gtex_offset_case);
        break;
    case _ACCENTUS_REVERSUS:
        fprintf(f, "\\GreReversedAccentus{%d}{\\GreOCase%s}%%\n",
                pitch_value(current_note->u.note.pitch),
                current_note->gtex_offset_case);
        break;
    case _CIRCULUS:
        fprintf(f, "\\GreCirculus{%d}{\\GreOCase%s}%%\n",
                pitch_value(current_note->u.note.pitch),
                current_note->gtex_offset_case);
        break;
    case _SEMI_CIRCULUS:
        fprintf(f, "\\GreSemicirculus{%d}{\\GreOCase%s}%%\n",
                pitch_value(current_note->u.note.pitch),
                current_note->gtex_offset_case);
        break;
    case _SEMI_CIRCULUS_REVERSUS:
        fprintf(f, "\\GreReversedSemicirculus{%d}{\\GreOCase%s}%%\n",
                pitch_value(current_note->u.note.pitch),
                current_note->gtex_offset_case);
        break;
    case _MUSICA_FICTA_FLAT:
        fprintf(f, "\\GreMusicaFictaFlat{%d}{\\GreOCase%s}%%\n",
                pitch_value(current_note->u.note.pitch),
                current_note->gtex_offset_case);
        break;
    case _MUSICA_FICTA_NATURAL:
        fprintf(f, "\\GreMusicaFictaNatural{%d}{\\GreOCase%s}%%\n",
                pitch_value(current_note->u.note.pitch),
                current_note->gtex_offset_case);
        break;
    case _MUSICA_FICTA_SHARP:
        fprintf(f, "\\GreMusicaFictaSharp{%d}{\\GreOCase%s}%%\n",
                pitch_value(current_note->u.note.pitch),
                current_note->gtex_offset_case);
        break;
        /* the cases of the bar signs are dealt in another function
         * (write_bar) */
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        assert(false);
        break;
        /* LCOV_EXCL_STOP */
    }
}

static __inline const char *alteration_name(
        const gregorio_shape next_note_alteration)
{
    switch (next_note_alteration) {
    case S_FLAT:
        return "Flat";
    case S_SHARP:
        return "Sharp";
    case S_NATURAL:
        return "Natural";
    default:
        return "";
    }
}

static const char *next_custos(const signed char next_note_pitch,
        const gregorio_shape next_note_alteration,
        gregoriotex_status *const status)
{
    static char buf[50];

    gregorio_snprintf(buf, sizeof buf, "%s\\GreNextCustos{%d}{%s}",
            status->suppressed_custos? "\\GreResetEolCustos" : "",
            pitch_value(next_note_pitch),
            alteration_name(next_note_alteration));

    status->suppressed_custos = false;

    return buf;
}

/*
 * function used when the glyph is only one note long
 */

static void write_note(FILE *f, gregorio_note *note,
        const gregorio_glyph *glyph, const char next_note_pitch,
        const gregorio_shape next_note_alteration,
        gregoriotex_status *const status)
{
    unsigned int initial_shape = note->u.note.shape;
    const char *shape;
    int space_case;
    static char cpbuf[96];
    /* type in the sense of GregorioTeX alignment type */
    gtex_alignment type = AT_ONE_NOTE;
    gregorio_assert(note, write_note, "called with NULL pointer",
            return);
    if (note->u.note.shape == S_PUNCTUM) {
        switch (note->u.note.liquescentia) {
        case L_AUCTUS_ASCENDENS:
            note->u.note.shape = S_PUNCTUM_AUCTUS_ASCENDENS;
            break;
        case L_AUCTUS_DESCENDENS:
            note->u.note.shape = S_PUNCTUM_AUCTUS_DESCENDENS;
            break;
        case L_INITIO_DEBILIS:
            if (glyph->u.notes.fuse_to_next_glyph > 0) {
                break;
            }
            /* else fall through to next case */
        case L_DEMINUTUS:
            /* this is a currenly unused, but we keep it as a fallback case */
            /* LCOV_EXCL_START */
            note->u.note.shape = S_PUNCTUM_DEMINUTUS;
            break;
            /* LCOV_EXCL_STOP */
        default:
            break;
        }
    }
    shape = determine_note_glyph_name(note, glyph, &type);
    note->u.note.shape = initial_shape;
    /* special things for puncta inclinata */
    space_case = get_punctum_inclinatum_space_case(note);
    if (space_case >= 0) {
        fprintf(f, "\\GreEndOfGlyph{%d}%%\n", space_case);
    }

    switch (note->u.note.shape) {
    case S_FLAT:
        fprintf(f, "\\GreFlat{%d}{0}", pitch_value(note->u.note.pitch));
        break;
    case S_NATURAL:
        fprintf(f, "\\GreNatural{%d}{0}", pitch_value(note->u.note.pitch));
        break;
    case S_SHARP:
        fprintf(f, "\\GreSharp{%d}{0}", pitch_value(note->u.note.pitch));
        break;
    default:
        fprintf(f, "\\GreGlyph{%s}{%d}{%s}{%d}",
                code_point(shape, glyph->u.notes.is_cavum, cpbuf, sizeof cpbuf),
                pitch_value(note->u.note.pitch),
                next_custos(next_note_pitch, next_note_alteration, status),
                type);
        break;
    }
}

static int gregoriotex_syllable_first_type(gregorio_syllable *syllable)
{
    int result = 0;
    gtex_alignment type = AT_ONE_NOTE;
    gtex_type gtype = T_ONE_NOTE;
    /* alteration says if there is a flat or a natural first in the next
     * syllable, see gregoriotex.tex for more details */
    int alteration = 0;
    gregorio_glyph *glyph;
    gregorio_element *element;
    gregorio_assert(syllable && syllable->elements,
            gregoriotex_syllable_first_type, "called with a NULL argument",
            return 0);
    for (element = syllable->elements[0]; element; element = element->next) {
        if (element->type == GRE_BAR) {
            switch (element->u.misc.unpitched.info.bar) {
            case B_NO_BAR:
            case B_VIRGULA:
            case B_VIRGULA_HIGH:
                result = 10;
                break;
            case B_DIVISIO_MINIMIS:
            case B_DIVISIO_MINIMIS_HIGH:
            case B_DIVISIO_MINIMA:
            case B_DIVISIO_MINIMA_HIGH:
                result = 11;
                break;
            case B_DIVISIO_MINOR:
                result = 12;
                break;
            case B_DIVISIO_MAIOR:
            case B_DIVISIO_MAIOR_DOTTED:
                result = 13;
                break;
            case B_DIVISIO_MINOR_D1:
            case B_DIVISIO_MINOR_D2:
            case B_DIVISIO_MINOR_D3:
            case B_DIVISIO_MINOR_D4:
            case B_DIVISIO_MINOR_D5:
            case B_DIVISIO_MINOR_D6:
            case B_DIVISIO_MINOR_D7:
            case B_DIVISIO_MINOR_D8:
                result = 14;
                break;
            case B_DIVISIO_FINALIS:
                result = 15;
                break;
            default:
                /* not reachable unless there's a programming error */
                /* LCOV_EXCL_START */
                gregorio_fail(gregoriotex_syllable_first_type, "invalid bar");
                result = 0;
                break;
                /* LCOV_EXCL_STOP */
            }
            return result;
        }
        if (element->type == GRE_ELEMENT && element->u.first_glyph) {
            for (glyph = element->u.first_glyph; glyph; glyph = glyph->next) {
                if (glyph->type == GRE_GLYPH && glyph->u.notes.first_note) {
                    switch (glyph->u.notes.glyph_type) {
                    case G_ALTERATION:
                        if (alteration == 0) {
                            switch (glyph->u.notes.first_note->u.note.shape) {
                            case S_FLAT:
                                alteration = 20;
                                break;
                            case S_NATURAL:
                                alteration = 40;
                                break;
                            case S_SHARP:
                                alteration = 60;
                                break;
                            default:
                                /* not reachable unless there's a programming error */
                                /* LCOV_EXCL_START */
                                gregorio_fail(gregoriotex_syllable_first_type,
                                        "invalid alteration");
                                break;
                                /* LCOV_EXCL_STOP */
                            }
                        }
                        continue;
                    case G_PUNCTA_INCLINATA:
                    case G_2_PUNCTA_INCLINATA_DESCENDENS:
                    case G_3_PUNCTA_INCLINATA_DESCENDENS:
                    case G_4_PUNCTA_INCLINATA_DESCENDENS:
                    case G_5_PUNCTA_INCLINATA_DESCENDENS:
                    case G_2_PUNCTA_INCLINATA_ASCENDENS:
                    case G_3_PUNCTA_INCLINATA_ASCENDENS:
                    case G_4_PUNCTA_INCLINATA_ASCENDENS:
                    case G_5_PUNCTA_INCLINATA_ASCENDENS:
                    case G_PUNCTUM:
                    case G_STROPHA:
                    case G_VIRGA:
                    case G_VIRGA_REVERSA:
                    case G_STROPHA_AUCTA:
                    case G_DISTROPHA:
                    case G_DISTROPHA_AUCTA:
                    case G_TRISTROPHA:
                    case G_TRISTROPHA_AUCTA:
                    case G_BIVIRGA:
                    case G_TRIVIRGA:
                        determine_note_glyph_name
                                (glyph->u.notes.first_note, glyph, &type);
                        break;
                    default:
                        gregoriotex_determine_glyph_name(glyph, &type, &gtype);
                        break;
                    }
                    return type + alteration;
                }
            }
        }
    }
    if (syllable->elements[0]) {
        /* if there is anything else in the next syllable */
        return 0;
    }
    /* the only thing that should reach this point is an empty next syllable
     * we treat that kind of syllable as a bar */
    return 16;
}

static __inline void write_low_choral_sign(FILE *const f,
        const gregorio_note *const note, int special)
{
    if (note->choral_sign_is_nabc) {
        fprintf(f, "\\GreLowChoralSign{%d}{\\GreNABCChar{",
                pitch_value(note->u.note.pitch));
        tex_escape_text(f, note->choral_sign);
        fprintf(f, "}}{%d}%%\n", special);
    } else {
        fprintf(f, "\\GreLowChoralSign{%d}{%s}{%d}%%\n",
                pitch_value(note->u.note.pitch), note->choral_sign,
                special);
    }
}

static __inline void write_high_choral_sign(FILE *const f,
        const gregorio_note *const note, int pitch_offset)
{
    if (note->choral_sign_is_nabc) {
        fprintf(f, "\\GreHighChoralSign{%d}{\\GreNABCChar{",
                pitch_value(note->u.note.pitch + pitch_offset));
        tex_escape_text(f, note->choral_sign);
        fprintf(f, "}}{\\GreOCase%s}%%\n", note->gtex_offset_case);
    } else {
        fprintf(f, "\\GreHighChoralSign{%d}{%s}{\\GreOCase%s}%%\n",
                pitch_value(note->u.note.pitch + pitch_offset),
                note->choral_sign, note->gtex_offset_case);
    }
}

static void write_choral_sign(FILE *f, const gregorio_glyph *glyph,
        const gregorio_note *current_note, bool low)
{
    bool kind_of_pes;
    /* false in the normal case (sign above the note), true in the case of it's
     * next to the note (same height as a punctum) */
    bool low_sign = choral_sign_here_is_low(glyph, current_note, &kind_of_pes);

    /* the low choral signs must be typeset after the punctum, whereas the high
     * must be typeset before the h episema */
    if ((low_sign && !low) || (!low_sign && low)) {
        return;
    }

    if (low_sign) {
        /* very approximative heuristic, some things may have to be adapted
         * here... */
        if (is_on_a_line(current_note->u.note.pitch)) {
            if (kind_of_pes && current_note->u.note.pitch -
                    current_note->next->u.note.pitch == -1) {
                write_low_choral_sign(f, current_note, 1);
                return;
            }
            if (current_note->previous
                    && (current_note->previous->signs == _PUNCTUM_MORA
                            || current_note->previous->signs ==
                            _V_EPISEMA_PUNCTUM_MORA)) {
                write_low_choral_sign(f, current_note, 1);
                return;
            }
        }

        write_low_choral_sign(f, current_note, 0);
    } else {
        /* let's cheat a little */
        if (is_on_a_line(current_note->u.note.pitch)) {
            write_high_choral_sign(f, current_note, 0);
        } else {
            write_high_choral_sign(f, current_note, 2);
        }
    }
}

/*
 *
 * A function that write the signs of a glyph, which has the type type (T_*,
 * not G_*, which is in the glyph->glyph_type), and (important), we start only
 * at the note current_note. It is due to the way we call it : if type is
 * T_ONE_NOTE, we just do the signs on current_note, not all. This is the case
 * for example for the first note of the torculus resupinus, or the
 * G_*_PUNCTA_INCLINATA.
 *
 */

/* small helper */
static __inline bool _found(FILE *const f, const bool found)
{
    if (!found) {
        fprintf (f, "%%\n");\
        return true;
    }
    return found;
}

static void compute_height_extrema(const gregorio_glyph *const glyph,
        const gregorio_note *note, signed char *const top_height,
        signed char *const bottom_height)
{
    char height;
    /* get the minima/maxima pitches */
    for (; note; note = note->next) {
        if (note->h_episema_above) {
            height = note->h_episema_above;
        } else {
            if (note->v_episema_height && note->v_episema_height
                    > note->u.note.pitch) {
                height = note->v_episema_height;
            } else {
                height = note->u.note.pitch;
            }

            if (note->choral_sign
                    && !choral_sign_here_is_low(glyph, note, NULL)) {
                ++height;
            }
        }
        if (*top_height == UNDETERMINED_HEIGHT || height > *top_height) {
            *top_height = height;
        }

        if (note->h_episema_below) {
            height = note->h_episema_below;
        } else if (note->v_episema_height && note->v_episema_height
                < note->u.note.pitch) {
            height = note->v_episema_height;
        } else {
            height = note->u.note.pitch;

            /* no vertical episema, so we need to check for a stem */
            gregorio_assert(glyph->type == GRE_GLYPH, compute_height_extrema,
                    "glyph->type must be GRE_GLYPH here", return);
            switch (glyph->u.notes.glyph_type) {
            case G_BIVIRGA:
            case G_TRIVIRGA:
            case G_VIRGA:
            case G_VIRGA_REVERSA:
                --height;
                break;

            case G_PUNCTUM:
                switch (note->u.note.shape) {
                case S_ORISCUS_SCAPUS_ASCENDENS:
                case S_ORISCUS_SCAPUS_DESCENDENS:
                    --height;
                    break;

                default:
                    /* default to avoid the warning */
                    break;
                }
                break;

            default:
                /* default to avoid the warning */

                /* other shapes like the porrectus have stems, but the height
                 * computation will cover other notes in the shape, negating
                 * the need to account for the stem on the shape */
                break;
            }
        }

        if (*bottom_height == UNDETERMINED_HEIGHT || height < *bottom_height) {
            *bottom_height = height;
        }
    }
}

static void compute_element_height_extrema(
        const gregorio_element *const element, signed char *const top_height,
        signed char *const bottom_height)
{
    const gregorio_glyph *glyph;
    gregorio_not_null(element, compute_element_height_extrema, return);
    if (element->type != GRE_ELEMENT) {
        return;
    }
    /* get the minima/maxima pitches */
    for (glyph = element->u.first_glyph; glyph; glyph = glyph->next) {
        if (glyph->type == GRE_GLYPH) {
            compute_height_extrema(glyph, glyph->u.notes.first_note,
                    top_height, bottom_height);
        }
    }
}

static __inline void fixup_height_extrema(signed char *const top_height,
        signed char *const bottom_height)
{
    if (*top_height == UNDETERMINED_HEIGHT) {
        *top_height = 9; /* 'g' */
    }
    if (*bottom_height == UNDETERMINED_HEIGHT) {
        *bottom_height = 9; /* 'g' */
    }
}

static __inline bool is_ledger_drawn(const bool setting,
        const gregorio_ledger_specificity specificity)
{
    return ((specificity & LEDGER_DRAWN) && setting);
}

static void write_signs(FILE *f, gtex_type type,
        const gregorio_glyph *glyph, const gregorio_note *note,
        int fuse_to_next_note, gregoriotex_status *const status,
        const gregorio_score *const score)
{
    /* i is the number of the note for which we are typesetting the sign. */
    int i;
    const gregorio_note *current_note;
    signed char high_pitch = UNDETERMINED_HEIGHT;
    signed char low_pitch = UNDETERMINED_HEIGHT;
    bool found = false;
    compute_height_extrema(glyph, note, &high_pitch, &low_pitch);
    fixup_height_extrema(&high_pitch, &low_pitch);
    fprintf(f, "%%\n{%%\n\\GreGlyphHeights{%d}{%d}%%\n",
            pitch_value(high_pitch), pitch_value(low_pitch));
    for (current_note = note, i = 1; current_note;
            current_note = current_note->next, ++i) {
        /* we start by the additional lines */
        if (is_ledger_drawn(current_note->low_ledger_line,
                current_note->low_ledger_specificity)) {
            write_additional_line(f, i, type, true, current_note, score);
            status->bottom_line = 1;
        }
        if (is_ledger_drawn(current_note->high_ledger_line,
                current_note->high_ledger_specificity)) {
            write_additional_line(f, i, type, false, current_note, score);
        }
        if (current_note->texverb) {
            fprintf(f, "%% verbatim text at note level:\n%s%%\n"
                    "%% end of verbatim text\n",
                    gregorio_texverb(current_note->texverb));
        }
        if (type == T_ONE_NOTE) {
            break;
        }
    }
    fprintf(f, "}{");
    /* now a first loop for the choral signs, because high signs must be taken
     * into account before any hepisema */
    for (current_note = note, i = 1; current_note;
            current_note = current_note->next, ++i) {
        if (current_note->choral_sign) {
            found = _found(f, found);
            write_choral_sign(f, glyph, current_note, false);
        }
        if (type == T_ONE_NOTE) {
            break;
        }
    }
    /* a loop for rare signs, vertical episema, and horizontal episema */
    for (current_note = note, i = 1; current_note;
            current_note = current_note->next, ++i) {
        /* we continue with the hepisema */
        if (current_note->h_episema_above || current_note->h_episema_below) {
            found = _found(f, found);
            gregoriotex_write_hepisema(f, current_note, i, type, glyph);
        }
        /* write_rare_sign also writes the vepisema */
        if (current_note->special_sign) {
            found = _found(f, found);
            write_rare_sign(f, current_note, current_note->special_sign);
        }
        if (current_note->signs != _NO_SIGN) {
            found = _found(f, found);
        }
        switch (current_note->signs) {
        case _V_EPISEMA:
        case _V_EPISEMA_PUNCTUM_MORA:
        case _V_EPISEMA_AUCTUM_DUPLEX:
            write_vepisema(f, current_note);
            break;
        default:
            /* do nothing */
            break;
        }
        if (type == T_ONE_NOTE) {
            break;
        }
    }
    /* final loop for choral signs and punctum mora */
    for (current_note = note, i = 1; current_note;
            current_note = current_note->next, ++i) {
        switch (current_note->signs) {
        case _PUNCTUM_MORA:
        case _V_EPISEMA_PUNCTUM_MORA:
            write_punctum_mora(f, glyph, current_note, fuse_to_next_note);
            break;
        case _AUCTUM_DUPLEX:
        case _V_EPISEMA_AUCTUM_DUPLEX:
            write_auctum_duplex(f, current_note);
            break;
        default:
            break;
        }
        if (current_note->choral_sign) {
            found = _found(f, found);
            write_choral_sign(f, glyph, current_note, true);
        }
        if (type == T_ONE_NOTE) {
            break;
        }
    }
    fprintf(f, "}{");
    if (status->point_and_click) {
        fprintf(f, "%u:%u:%u", note->src_line, note->src_offset,
                note->src_column + 1);
    }
    fprintf(f, "}%%\n");
}

static char *determine_leading_shape(const gregorio_glyph *const glyph)
{
    static char buf[BUFSIZE];
    int ambitus = compute_ambitus(glyph->u.notes.first_note);
    const char *head, *head_liquescence;

    switch (glyph->u.notes.first_note->u.note.shape) {
    case S_QUILISMA:
        head = "Quilisma";
        break;
    case S_ORISCUS_ASCENDENS:
    case S_ORISCUS_DESCENDENS:
        head = "Oriscus";
        break;
    default:
        head = "Punctum";
        break;
    }

    if (glyph->u.notes.liquescentia & L_INITIO_DEBILIS) {
        head_liquescence = "InitioDebilis";
    } else {
        head_liquescence = "";
    }

    gregorio_snprintf(buf, BUFSIZE, "Leading%s%s%s", head, tex_ambitus[ambitus],
            head_liquescence);
    return buf;
}

static __inline void write_composed_multinote_glyph(FILE *const f,
        gregorio_note *current_note, const gregorio_glyph *glyph,
        const signed char next_note_pitch,
        const gregorio_shape next_note_alteration,
        const int fuse_to_next_note, gregoriotex_status *const status,
        const gregorio_score *const score, const char *glyph_end_between_notes)
{
    while (current_note) {
        const bool has_next = current_note->next != NULL;
        if (has_next) {
            write_note(f, current_note, glyph, current_note->next->u.note.pitch,
                    S_UNDETERMINED, status);
        } else {
            write_note(f, current_note, glyph, next_note_pitch,
                    next_note_alteration, status);
        }
        write_signs(f, T_ONE_NOTE, glyph, current_note,
                has_next ? 0 : fuse_to_next_note, status, score);
        current_note = current_note->next;
        if (current_note && glyph_end_between_notes) {
            fputs(glyph_end_between_notes, f);
        }
    }
}

static void write_glyph(FILE *const f, const gregorio_syllable *const syllable,
        const gregorio_element *const element,
        gregorio_glyph *const glyph,
        gregoriotex_status *const status, const gregorio_score *const score)
{
    static char cpbuf[96], cpbuf2[96];
    /* type is the type of the glyph. Understand the type of the glyph for
     * gregoriotex, for the alignement between text and notes. (AT_ONE_NOTE,
     * etc.) */
    gtex_alignment type = 0;
    /* the type of the glyph, in the sense of the shape (T_PES, etc.) */
    gtex_type gtype = 0;
    signed char next_note_pitch = 0;
    gregorio_note *current_note;
    const char *leading_shape, *shape;
    const gregorio_glyph *prev_glyph =
            gregorio_previous_non_texverb_glyph(glyph);
    int fuse_to_next_note, fuse_from_previous_note =
            (prev_glyph && prev_glyph->type == GRE_GLYPH)
            ? prev_glyph->u.notes.fuse_to_next_glyph : 0;
    gregorio_shape next_note_alteration;
    gregorio_assert(glyph, write_glyph, "called with NULL pointer", return);
    gregorio_assert(glyph->type == GRE_GLYPH && glyph->u.notes.first_note,
            write_glyph, "called with glyph without note", return);
    fuse_to_next_note = glyph->u.notes.fuse_to_next_glyph;
    if (fuse_from_previous_note) {
        fprintf(f, "\\GreFuse");
    }
    next_note_pitch = gregorio_determine_next_pitch(syllable, element, glyph,
            &next_note_alteration);
    current_note = glyph->u.notes.first_note;
    /* first we check if it is really a unique glyph in gregoriotex... the
     * glyphs that are not a unique glyph are : trigonus and pucta inclinata
     * in general, and torculus resupinus and torculus resupinus flexus, so
     * we first divide the glyph into real gregoriotex glyphs */
    switch (glyph->u.notes.glyph_type) {
    case G_PUNCTA_INCLINATA:
    case G_2_PUNCTA_INCLINATA_DESCENDENS:
    case G_3_PUNCTA_INCLINATA_DESCENDENS:
    case G_4_PUNCTA_INCLINATA_DESCENDENS:
    case G_5_PUNCTA_INCLINATA_DESCENDENS:
    case G_2_PUNCTA_INCLINATA_ASCENDENS:
    case G_3_PUNCTA_INCLINATA_ASCENDENS:
    case G_4_PUNCTA_INCLINATA_ASCENDENS:
    case G_5_PUNCTA_INCLINATA_ASCENDENS:
        write_composed_multinote_glyph(f, current_note, glyph,
                next_note_pitch, next_note_alteration, fuse_to_next_note,
                status, score, NULL);
        break;
    case G_SCANDICUS:
        if ((glyph->u.notes.liquescentia & L_DEMINUTUS)
                || glyph->u.notes.liquescentia == L_NO_LIQUESCENTIA
                || glyph->u.notes.liquescentia == L_FUSED) {
            shape = gregoriotex_determine_glyph_name(glyph, &type, &gtype);
            fprintf(f, "\\GreGlyph{%s}{%d}{%s}{%d}", code_point(shape,
                        glyph->u.notes.is_cavum, cpbuf, sizeof cpbuf),
                    pitch_value(glyph->u.notes.first_note->u.note.pitch),
                    next_custos(next_note_pitch, next_note_alteration, status),
                    type);
            write_signs(f, gtype, glyph, glyph->u.notes.first_note,
                    fuse_to_next_note, status, score);
        } else {
            write_composed_multinote_glyph(f, current_note, glyph,
                    next_note_pitch, next_note_alteration, fuse_to_next_note,
                    status, score, NULL);
        }
        break;
    case G_ANCUS:
        gregorio_assert(glyph->u.notes.liquescentia & L_DEMINUTUS,
                write_glyph, "encountered a non-deminutus ancus", break);
        shape = gregoriotex_determine_glyph_name(glyph, &type, &gtype);
        fprintf(f, "\\GreGlyph{%s}{%d}{%s}{%d}", code_point(shape,
                        glyph->u.notes.is_cavum, cpbuf, sizeof cpbuf),
                pitch_value(glyph->u.notes.first_note->u.note.pitch),
                next_custos(next_note_pitch, next_note_alteration, status),
                type);
        write_signs(f, gtype, glyph, glyph->u.notes.first_note,
                fuse_to_next_note, status, score);
        break;
    case G_TORCULUS_RESUPINUS_FLEXUS:
        /* we retain this "old-style" fusion as it does look marginally better
         * on screen */
        leading_shape = determine_leading_shape(glyph);
        /* trick to have the good position for these glyphs */
        glyph->u.notes.glyph_type = G_PORRECTUS_FLEXUS_NO_BAR;
        glyph->u.notes.first_note = current_note->next;
        shape = gregoriotex_determine_glyph_name(glyph, &type, &gtype);
        fprintf(f, "\\GreGlyph{\\GreFuseTwo{%s}{%s}}{%d}{%s}{%d}",
                code_point(leading_shape, glyph->u.notes.is_cavum, cpbuf,
                    sizeof cpbuf),
                code_point(shape, glyph->u.notes.is_cavum, cpbuf2,
                    sizeof cpbuf2),
                pitch_value(glyph->u.notes.first_note->u.note.pitch),
                next_custos(next_note_pitch, next_note_alteration, status),
                type);
        glyph->u.notes.first_note = current_note;
        glyph->u.notes.glyph_type = G_TORCULUS_RESUPINUS_FLEXUS;
        write_signs(f, gtype, glyph, glyph->u.notes.first_note,
                fuse_to_next_note, status, score);
        break;
    case G_BIVIRGA:
    case G_TRIVIRGA:
        write_composed_multinote_glyph(f, current_note, glyph,
                next_note_pitch, next_note_alteration, fuse_to_next_note,
                status, score, "\\GreEndOfGlyph{4}%%\n");
        break;
    case G_TRISTROPHA:
    case G_TRISTROPHA_AUCTA:
    case G_DISTROPHA:
    case G_DISTROPHA_AUCTA:
        write_composed_multinote_glyph(f, current_note, glyph,
                next_note_pitch, next_note_alteration, fuse_to_next_note,
                status, score, "\\GreEndOfGlyph{5}%%\n");
        break;

    case G_PUNCTUM:
        switch (glyph->u.notes.first_note->u.note.shape) {
        case S_ORISCUS_ASCENDENS:
        case S_ORISCUS_DESCENDENS:
        case S_ORISCUS_DEMINUTUS:
        case S_ORISCUS_SCAPUS_ASCENDENS:
        case S_ORISCUS_SCAPUS_DESCENDENS:
            /* don't change the oriscus */
            break;

        default:
            switch (glyph->u.notes.liquescentia) {
            case L_AUCTUS_ASCENDENS:
                glyph->u.notes.first_note->u.note.shape =
                        S_PUNCTUM_AUCTUS_ASCENDENS;
                break;
            case L_AUCTUS_DESCENDENS:
                glyph->u.notes.first_note->u.note.shape =
                        S_PUNCTUM_AUCTUS_DESCENDENS;
                break;
            case L_INITIO_DEBILIS:
            if (glyph->u.notes.fuse_to_next_glyph > 0) {
                break;
            }
            /* else fall through to next case */
            case L_DEMINUTUS:
                glyph->u.notes.first_note->u.note.shape = S_PUNCTUM_DEMINUTUS;
            default:
                break;
            }
            break;
        }

        /* fall into the next case */
    case G_PUNCTUM_INCLINATUM:
    case G_VIRGA:
    case G_VIRGA_REVERSA:
    case G_STROPHA:
    case G_STROPHA_AUCTA:
    case G_ALTERATION:
        write_note(f, glyph->u.notes.first_note, glyph, next_note_pitch,
                next_note_alteration, status);
        write_signs(f, T_ONE_NOTE, glyph, current_note, fuse_to_next_note,
                status, score);
        break;
    default:
        /* we retain this "old-style" fusion as it does look marginally better
         * on screen */
        /* special case of the torculus resupinus which first note is not a
         * punctum */
        if (glyph->u.notes.glyph_type == G_TORCULUS_RESUPINUS
                && current_note->u.note.shape != S_PUNCTUM
                && current_note->u.note.shape != S_QUILISMA) {
            leading_shape = determine_leading_shape(glyph);
            /* trick to have the good position for these glyphs */
            glyph->u.notes.glyph_type = G_PORRECTUS_NO_BAR;
            glyph->u.notes.first_note = current_note->next;
            shape = gregoriotex_determine_glyph_name(glyph, &type, &gtype);
            fprintf(f, "\\GreGlyph{\\GreFuseTwo{%s}{%s}}{%d}{%s}{%d}",
                    code_point(leading_shape, glyph->u.notes.is_cavum, cpbuf,
                        sizeof cpbuf),
                    code_point(shape, glyph->u.notes.is_cavum, cpbuf2,
                        sizeof cpbuf2),
                    pitch_value(glyph->u.notes.first_note->u.note.pitch),
                    next_custos(next_note_pitch, next_note_alteration, status),
                    type);
            glyph->u.notes.first_note = current_note;
            glyph->u.notes.glyph_type = G_TORCULUS_RESUPINUS;
            write_signs(f, gtype, glyph, glyph->u.notes.first_note,
                    fuse_to_next_note, status, score);
            break;
        } else {
            shape = gregoriotex_determine_glyph_name(glyph, &type, &gtype);
            fprintf(f, "\\GreGlyph{%s}{%d}{%s}{%d}", code_point(shape,
                        glyph->u.notes.is_cavum, cpbuf, sizeof cpbuf),
                    pitch_value(glyph->u.notes.first_note->u.note.pitch),
                    next_custos(next_note_pitch, next_note_alteration, status),
                    type);
            write_signs(f, gtype, glyph, glyph->u.notes.first_note,
                    fuse_to_next_note, status, score);
            break;
        }
    }
}

static __inline unsigned int glyph_note_units(const gregorio_glyph *glyph)
{
    unsigned int count = 0;
    const gregorio_note *note;
    if (glyph->u.notes.glyph_type != G_ALTERATION) {
        for (note = glyph->u.notes.first_note; note; note = note->next) {
            ++count;
        }
    }
    return count;
}

/* here we absolutely need to pass the syllable as an argument, because we
 * will need the next note, that may be contained in the next syllable */

static unsigned int write_element(FILE *const f,
        const gregorio_syllable *const syllable,
        const gregorio_element *const element,
        gregoriotex_status *const status,
        const gregorio_score *const score)
{
    unsigned int note_unit_count = 0;
    if (element->type == GRE_ELEMENT) {
        gregorio_glyph *glyph;
        for (glyph = element->u.first_glyph; glyph; glyph = glyph->next) {
            switch (glyph->type) {
            case GRE_SPACE:
                switch (glyph->u.misc.unpitched.info.space) {
                case SP_ZERO_WIDTH:
                    fprintf(f, "\\GreEndOfGlyph{1}%%\n");
                    break;
                case SP_HALF_SPACE:
                    fprintf(f, "\\GreEndOfGlyph{22}%%\n");
                    break;
                case SP_INTERGLYPH_SPACE:
                    fprintf(f, "\\GreEndOfGlyph{0}%%\n");
                    break;
                default:
                    /* not reachable unless there's a programming error */
                    /* LCOV_EXCL_START */
                    gregorio_fail(write_element,
                            "encountered an unexpected glyph-level space");
                    break;
                    /* LCOV_EXCL_STOP */
                }
                break;

            case GRE_TEXVERB_GLYPH:
                if (glyph->texverb) {
                    fprintf(f, "%% verbatim text at glyph level:\n%s%%\n"
                            "%% end of verbatim text\n",
                            gregorio_texverb(glyph->texverb));
                }
                break;

            default:
                /* at this point glyph->type is GRE_GLYPH */
                assert(glyph->type == GRE_GLYPH);
                write_glyph(f, syllable, element, glyph, status, score);
                note_unit_count += glyph_note_units(glyph);
                if (glyph->next && glyph->next->type == GRE_GLYPH &&
                        glyph->next->u.notes.glyph_type != G_ALTERATION) {
                    if (is_fused(glyph->next->u.notes.liquescentia)) {
                        int space_case =
                            get_punctum_inclinatum_to_nobar_space_case(glyph);
                        if (space_case >= 0) {
                            /* fuse from punctum inclinatum to nobar glyph */
                            fprintf(f, "\\GreEndOfGlyph{%d}%%\n", space_case);
                        } else {
                            fprintf(f, "\\GreEndOfGlyph{1}%%\n");
                        }
                    } else if (is_puncta_inclinata(
                                glyph->next->u.notes.glyph_type)
                            || glyph->next->u.notes.glyph_type
                            == G_PUNCTA_INCLINATA) {
                        if ((is_puncta_inclinata(glyph->u.notes.glyph_type)
                                    || glyph->u.notes.glyph_type
                                    == G_PUNCTA_INCLINATA)
                                && glyph->next->u.notes.first_note->u.note.pitch
                                == gregorio_glyph_last_note(glyph)->u.note.pitch) {
                            /* special case for unison puncta inclinata */
                            fprintf(f, "\\GreEndOfGlyph{23}%%\n");
                        } else {
                            fprintf(f, "\\GreEndOfGlyph{9}%%\n");
                        }
                    } else if (glyph->u.notes.glyph_type != G_ALTERATION
                            || !glyph->next) {
                        fprintf(f, "\\GreEndOfGlyph{0}%%\n");
                    }
                }
                break;
            }
        }
    }
    return note_unit_count;
}

static void write_fixed_text_styles(FILE *f, gregorio_character *syllable_text,
        gregorio_character *next_syllable_text)
{
    if (syllable_text) {
        gregoriotex_ignore_style = gregoriotex_fix_style(syllable_text);
        if (gregoriotex_ignore_style) {
            fprintf(f, "\\GreSetFixedTextFormat{%d}",
                    gregoriotex_internal_style_to_gregoriotex
                    (gregoriotex_ignore_style));
        }
    }
    if (next_syllable_text) {
        gregoriotex_next_ignore_style = gregoriotex_fix_style(
                next_syllable_text);
        if (gregoriotex_next_ignore_style) {
            fprintf(f, "\\GreSetFixedNextTextFormat{%d}",
                    gregoriotex_internal_style_to_gregoriotex
                    (gregoriotex_next_ignore_style));
        }
    }
}

static void write_text(FILE *const f, const gregorio_character *const text)
{
    if (text == NULL) {
        fprintf(f, "{}{}{}{}{}");
        return;
    }
    fprintf(f, "{");
    gregorio_write_text(WTP_NORMAL, text, f, &gtex_write_verb,
            &gtex_print_char, &gtex_write_begin, &gtex_write_end,
            &gtex_write_special_char);
    fprintf(f, "}{");
    gregorio_write_first_letter_alignment_text(WTP_NORMAL, text,
            f, &gtex_write_verb, &gtex_print_char, &gtex_write_begin,
            &gtex_write_end, &gtex_write_special_char);
    gregoriotex_ignore_style = gregoriotex_next_ignore_style;
    gregoriotex_next_ignore_style = ST_NO_STYLE;
    fprintf(f, "}");
}

/*
 * Function printing the line clef change (only updating \localleftbox, not
 * printing the key). Useful for \GreDiscretionary.
 * TODO: I'm not sure about the third argument, but that's how it's called in
 * \GreChangeClef.
 */
static void gregoriotex_print_change_line_clef(FILE *f,
        const gregorio_element *const current_element)
{
    if (current_element->type == GRE_CLEF) {
        /* the third argument is 0 or 1 according to the need for a space
         * before the clef */
        fprintf(f, "\\GreSetLinesClef{%c}{%d}{1}{%d}{%c}{%d}{%d}%%\n",
                gregorio_clef_to_char(current_element->u.misc.clef.clef),
                current_element->u.misc.clef.line,
                clef_flat_height(current_element->u.misc.clef.clef,
                        current_element->u.misc.clef.line,
                        current_element->u.misc.clef.flatted),
                gregorio_clef_to_char(
                        current_element->u.misc.clef.secondary_clef),
                current_element->u.misc.clef.secondary_line,
                clef_flat_height(current_element->u.misc.clef.secondary_clef,
                        current_element->u.misc.clef.secondary_line,
                        current_element->u.misc.clef.secondary_flatted));
    }
}

static __inline bool next_is_bar(const gregorio_syllable *syllable,
        const gregorio_element *element)
{
    bool got_custos = false;
    if (element) {
        element = element->next;
    }

    while (syllable) {
        if (element) {
            if (element->type == GRE_BAR) {
                return true;
            }
            /* allow no more than one manual custos before a bar */
            if (got_custos || !is_manual_custos(element)) {
                return false;
            }
            got_custos = true;
            if (element->next) {
                /* look at the next element */
                element = element->next;
                continue;
            }
        }

        syllable = syllable->next_syllable;
        if (!syllable) {
            return false;
        }

        element = syllable->elements[0];
    }

    /* not reachable unless there's a programming error */
    /* LCOV_EXCL_START */
    gregorio_fail(next_is_bar, "unexpected end of syllables/elements");
    return false;
    /* LCOV_EXCL_STOP */
}

static void finish_syllable(FILE *f, gregorio_syllable *syllable) {
    /* Very last, if the syllable is the end of a no-linebreak area: */
    if (syllable->no_linebreak_area == NLBA_END) {
        fprintf(f, "\\GreEndNLBArea{%d}{0}%%\n",
                next_is_bar(syllable, NULL)? 3 : 1);
    }
    if (syllable->euouae == EUOUAE_END) {
        fprintf(f, "\\GreEndEUOUAE{%d}%%\n",
                next_is_bar(syllable, NULL)? 3 : 1);
    }
}

static void handle_final_bar(FILE *f, const char *type, gregorio_syllable *syllable)
{
    gregorio_element *element;
    fprintf(f, "\\GreFinal%s{%%\n", type);
    /* first element will be the bar, which we just handled, so skip it */
    for (element = (*syllable->elements)->next; element;
            element = element->next) {
        switch (element->type) {
        case GRE_TEXVERB_ELEMENT:
            if (element->texverb) {
                fprintf(f, "%% verbatim text at element level:\n%s%%\n"
                        "%% end of verbatim text\n",
                        gregorio_texverb(element->texverb));
            }
            break;

        case GRE_CUSTOS:
            assert(element->u.misc.pitched.force_pitch);
            fprintf(f, "\\GreFinalCustos{%d}{}%%\n",
                    pitch_value(element->u.misc.pitched.pitch));
            break;

        case GRE_END_OF_LINE:
            fprintf(f, "\\GreFinalNewLine%%\n");
            break;

        default:
            /* do nothing */
            break;
        }
    }
    fprintf(f, "}%%\n");
    finish_syllable(f, syllable);
}

static __inline void write_syllable_point_and_click(FILE *const f,
        const gregorio_syllable *const syllable,
        const gregoriotex_status *const status)
{
    if (status->point_and_click && syllable->src_line) {
        fprintf(f, "%u:%u:%u", syllable->src_line, syllable->src_offset,
                syllable->src_column + 1);
    }
}

static void write_syllable_text(FILE *f, const char *const syllable_type,
        const gregorio_syllable *const syllable,
        const bool ignored __attribute__((unused)))
{
    if (syllable_type != NULL) {
        fprintf(f, "%s{%s%s\\GreSetThisSyllable", syllable_type,
                syllable->clear? "\\GreClearSyllableText" : "",
                syllable->forced_center? "\\GreGABCForceCenters" : "");
        write_text(f, syllable->text);
        fprintf(f, "}");
    }
}

static void write_first_syllable_text(FILE *f, const char *const syllable_type,
        const gregorio_syllable *const syllable,
        const bool end_of_word)
{
    const gregorio_character *const text = syllable->text;
    gregorio_not_null(syllable_type, write_first_syllable_text, return);
    if (text == NULL) {
        fprintf(f, "}{%s}{\\GreSetNoFirstSyllableText}", syllable_type);
    } else {
        gregorio_character *text_with_initial = gregorio_clone_characters(text),
                *text_without_initial = gregorio_clone_characters(text);
        const gregorio_character *t;

        /* find out if there is a forced center -> has_forced_center */
        gregorio_center_determination center = syllable->forced_center?
                CENTER_FULLY_DETERMINED : CENTER_NOT_DETERMINED;

        gregorio_rebuild_first_syllable(&text_with_initial, false);
        gregorio_rebuild_characters(&text_with_initial, center, false);
        gregorio_set_first_word(&text_with_initial);

        gregorio_rebuild_first_syllable(&text_without_initial, true);
        gregorio_rebuild_characters(&text_without_initial, center, true);
        gregorio_set_first_word(&text_without_initial);

        fprintf(f, "}{%s}{%s%s\\GreSetFirstSyllableText", syllable_type,
                syllable->clear? "\\GreClearSyllableText" : "",
                syllable->forced_center? "\\GreGABCForceCenters" : "");

        fprintf(f, "{");
        gregorio_write_first_letter_alignment_text(WTP_FIRST_SYLLABLE,
                text_with_initial, f, &gtex_write_verb, &gtex_print_char,
                &gtex_write_begin, &gtex_write_end, &gtex_write_special_char);
        fprintf(f, "}{{");
        gregorio_write_text(WTP_FIRST_SYLLABLE, text_without_initial, f,
                &gtex_write_verb, &gtex_print_char, &gtex_write_begin,
                &gtex_write_end, &gtex_write_special_char);
        fprintf(f, "}}{{");
        gregorio_write_text(WTP_NORMAL, text_with_initial, f, &gtex_write_verb,
                &gtex_print_char, &gtex_write_begin, &gtex_write_end,
                &gtex_write_special_char);
        gregoriotex_ignore_style = gregoriotex_next_ignore_style;
        gregoriotex_next_ignore_style = ST_NO_STYLE;
        fprintf(f, "}}");

        /* Check to see if we need to force a hyphen (empty first syllable) */
        for (t = text_without_initial; t; t = t->next_character) {
            if (t->is_character) {
                break;
            } else if (t->cos.s.type == ST_T_BEGIN) {
                if (t->cos.s.style == ST_VERBATIM ||
                        t->cos.s.style == ST_SPECIAL_CHAR) {
                    break;
                } else if (t->cos.s.style == ST_INITIAL) {
                    for (; t; t = t->next_character) {
                        if (!t->is_character && t->cos.s.type == ST_T_END
                                && t->cos.s.style == ST_INITIAL) {
                            break;
                        }
                    }
                    if (!t) break;
                }
            }
        }
        if (t || end_of_word) {
            fprintf(f, "{}");
        } else {
            fprintf(f, "{\\GreEmptyFirstSyllableHyphen}");
        }

        fprintf(f, "}");

        gregorio_free_characters(text_with_initial);
        gregorio_free_characters(text_without_initial);
    }
}

static __inline void scan_syllable_for_eol(
        const gregorio_syllable *const syllable, char *const eol_forces_custos)
{
    const gregorio_element *element;
    if (syllable->elements) {
        for (element = *(syllable->elements); element; element = element->next) {
            if (element->type == GRE_END_OF_LINE) {
                if (element->u.misc.unpitched.info.eol_forces_custos) {
                    *eol_forces_custos = element->u.misc.unpitched.info
                            .eol_forces_custos_on?  '1' : '0';
                }
            }
        }
    }
}

/*
 * euouae_follows will be
 *  - '\0' if no euouae follows
 *  - '0' if euouae follows with no intervening linebreak
 *  - '1' if euouae follows with an intervening linebreak
 *
 * eol_forces_custos will be
 *  - '\0' if no linebreak follows or doesn't force a custos
 *  - '0' if a linebreak follows and forces custos off
 *  - '1' if a linebreak follows and forces custos on
 */
static __inline void anticipate_event(gregorio_syllable *syllable,
        char *const euouae_follows, char *const eol_forces_custos,
        unsigned short *const next_euouae_id)
{
    bool has_intervening_linebreak = false;

    *euouae_follows = '\0';
    *eol_forces_custos = '\0';
    *next_euouae_id = 0;

    if (syllable->next_syllable) {
        for (syllable = syllable->next_syllable;
                syllable && syllable->elements && *(syllable->elements)
                && (*(syllable->elements))->type == GRE_END_OF_LINE;
                syllable = syllable->next_syllable) {
            has_intervening_linebreak = true;
            /* we are at an end-of-line, so check if custos is forced */
            scan_syllable_for_eol(syllable, eol_forces_custos);
        }
        if (syllable) {
            scan_syllable_for_eol(syllable, eol_forces_custos);

            if (syllable->euouae == EUOUAE_BEGINNING) {
                *next_euouae_id = syllable->euouae_id = ++tex_position_id;
                *euouae_follows = has_intervening_linebreak? '1' : '0';
            }
        }
    }
}

static __inline void write_anticipated_event(FILE *f, const char euouae_follows,
        const char eol_forces_custos, const short next_euouae_id)
{
    if (euouae_follows) {
        fprintf(f, "%%\n\\GreNextSyllableBeginsEUOUAE{%hu}{%c}%%\n",
                next_euouae_id, euouae_follows);
    }
    if (eol_forces_custos) {
        fprintf(f, "%%\n\\GreUpcomingNewLineForcesCustos{%c}%%\n",
                eol_forces_custos);
    }
}

static __inline unsigned int count_note_units(const gregorio_element *element)
{
    unsigned int count = 0;
    for (; element; element = element->next) {
        if (element->type == GRE_ELEMENT) {
            const gregorio_glyph *glyph;
            for (glyph = element->u.first_glyph; glyph; glyph = glyph->next) {
                if (glyph->type == GRE_GLYPH) {
                    count += glyph_note_units(glyph);
                }
            }
        }
    }
    return count;
}

static __inline void handle_last_of_voice(FILE *const f,
        const gregorio_syllable *syllable,
        const gregorio_element *const element,
        const gregorio_element *const last_of_voice)
{
    if (element == last_of_voice) {
        if (syllable->next_syllable) {
            /* check for no-element syllables that follow */
            for (syllable = syllable->next_syllable;
                    syllable && (!syllable->elements || !*(syllable->elements));
                    syllable = syllable->next_syllable) {
                /* just loop */
            }
            /* if syllable is NULL here, then all syllables that follow
             * have no elements */
        }
        /* emit GreLastOfScore if we are either on the last syllable (and
         * thus the loop above is not executed, leaving syllable at the
         * current syllable) or if a syllable which follows the current
         * syllable has an element of some sort (and thus the loop above
         * stopped before running out of syllables); in any case, the check
         * is that syllable, at this point, is not NULL */
        if (syllable) {
            fprintf(f, "\\GreLastOfScore");
        }
    }
}

static bool is_before_linebreak(const gregorio_syllable *syllable,
        const gregorio_element *element)
{
    if (element) {
        element = element->next;
    }

    while (syllable) {
        if (element) {
            if (element->type == GRE_END_OF_LINE) {
                return true;
            }
            break;
        }

        syllable = syllable->next_syllable;
        if (syllable) {
            element = syllable->elements[0];
        }
    }

    return false;
}

/*
 * Arguments are relatively obvious. The most obscure is certainly first_of_disc
 * which is 0 all the time, except in the case of a "clef change syllable". In
 * this case we make a \GreDiscretionary with two arguments:
 *   1.what should be printed if the syllable is the last of its line (which
 *   basically means everything but clefs and custos), and
 *   2. what should be printed if it's in a middle of a line (which means
 *   everything)
 * So the first_of_disc argument is:
 *   0 if we don't know (general case)
 *   1 in case of the first argument of a \GreDiscretionary
 *   2 if we are in the second argument (necessary in order to avoid infinite loops)
 */
static void write_syllable(FILE *f, gregorio_syllable *syllable,
        unsigned char first_of_disc, gregoriotex_status *const status,
        const gregorio_score *const score,
        const gregorio_element *const *const last_of_voice,
        void (*const write_this_syllable_text)
        (FILE *, const char *, const gregorio_syllable *, bool))
{
    const gregorio_element *clef_change_element = NULL, *element;
    const char *syllable_type = NULL;
    bool anticipated_event_written = false;
    bool end_of_word;
    bool end_of_line;
    char euouae_follows;
    char eol_forces_custos;
    unsigned short next_euouae_id;
    unsigned int note_unit_count;

    gregorio_not_null(syllable, write_syllable, return);
    end_of_word = syllable->position == WORD_END
            || syllable->position == WORD_ONE_SYLLABLE || !syllable->text
            || !syllable->next_syllable;
    /* Very first: before anything, if the syllable is the beginning of a
     * no-linebreak area: */
    if (syllable->no_linebreak_area == NLBA_BEGINNING) {
        fprintf(f, "\\GreBeginNLBArea{1}{0}%%\n");
    }
    if (syllable->euouae == EUOUAE_BEGINNING) {
        fprintf(f, "\\GreBeginEUOUAE{%hu}%%\n", syllable->euouae_id);
    }
    if (syllable->elements && *(syllable->elements)) {
        /*
         * first we check if the syllable is only a end of line. If it is the
         * case, we don't print anything but a comment (to be able to read it
         * if we read GregorioTeX). The end of lines are treated separately in
         * GregorioTeX, it is buit inside the TeX structure.
         */
        if ((syllable->elements)[0]->type == GRE_END_OF_LINE) {
            gregorio_assert(syllable != score->first_syllable, write_syllable,
                    "line break is not supported on the first syllable",
                    return);
            if ((syllable->elements)[0]->u.misc.unpitched.info.eol_ragged) {
                fprintf(f, "%%\n%%\n\\GreNewParLine %%\n%%\n%%\n");
            } else {
                fprintf(f, "%%\n%%\n\\GreNewLine %%\n%%\n%%\n");
            }
            write_this_syllable_text(f, NULL, syllable, end_of_word);
            return;
        }
        /*
         * This case is not simple: if the syllable contains a clef change,
         * whether it is (c4) or (::c4) or (z0::c4), we put it in a
         * discretionary. Warning: only these three cases will have the expected
         * effect. So first we detect it:
         */
        if (first_of_disc == 0) {   /* to avoid infinite loops */
            clef_change_element = gregorio_get_clef_change(syllable);
            if (clef_change_element) {
                gregorio_assert(syllable != score->first_syllable,
                        write_syllable,
                        "clef change is not supported on the first syllable",
                        return);
                /*
                 * In this case, the first thing to do is to change the line clef
                 */
                gregoriotex_print_change_line_clef(f, clef_change_element);
                fprintf(f, "\\GreDiscretionary{0}{%%\n");
                write_syllable(f, syllable, 1, status, score, last_of_voice,
                        write_syllable_text);
                fprintf(f, "}{%%\n");
                write_syllable(f, syllable, 2, status, score, last_of_voice,
                        write_syllable_text);
                fprintf(f, "}%%\n");
                write_this_syllable_text(f, NULL, syllable, end_of_word);
                return;
            }
        }
        write_fixed_text_styles(f, syllable->text,
                syllable->next_syllable? syllable->next_syllable->text : NULL);
        if ((syllable->elements)[0]->type == GRE_BAR) {
            if (syllable != score->first_syllable) {
                if (!syllable->next_syllable && !syllable->text
                        && (syllable->elements)[0]->u.misc.unpitched.info.bar
                        == B_DIVISIO_FINALIS) {
                    handle_final_bar(f, "DivisioFinalis", syllable);
                    write_this_syllable_text(f, NULL, syllable, end_of_word);
                    return;
                }
                if (!syllable->next_syllable && !syllable->text
                        && (syllable->elements)[0]->u.misc.unpitched.info.bar
                        == B_DIVISIO_MAIOR) {
                    handle_final_bar(f, "DivisioMaior", syllable);
                    write_this_syllable_text(f, NULL, syllable, end_of_word);
                    return;
                }
            }
            /* the special case of first syllable will be treated as a normal
             * bar syllable */
            syllable_type = "\\GreBarSyllable";
        } else {
            if (first_of_disc != 0) {
                syllable_type = "\\GreBarSyllable";
            } else {
                syllable_type = "\\GreSyllable";
            }
        }
    } else {
        write_fixed_text_styles(f, syllable->text,
                syllable->next_syllable? syllable->next_syllable->text : NULL);
        syllable_type = "\\GreNoNoteSyllable";
    }
    write_this_syllable_text(f, syllable_type, syllable, end_of_word);
    fprintf(f, "{}{\\Gre%s}", syllable->first_word ? "FirstWord" : "Unstyled");
    if (end_of_word) {
        fprintf(f, "{1}");
    } else {
        fprintf(f, "{0}");
    }
    end_of_line = is_last_of_line(syllable);
    anticipate_event(syllable, &euouae_follows, &eol_forces_custos,
            &next_euouae_id);
    if (syllable->next_syllable) {
        fprintf(f, "{%s\\GreSetNextSyllable",
                syllable->next_syllable->forced_center?
                "\\GreGABCNextForceCenters" : "");
        write_text(f, syllable->next_syllable->text);
        if (end_of_line || first_of_disc == 1) {
            fprintf(f, "\\GreLastOfLine");
        } else if (euouae_follows) {
            fprintf(f, "\\GreLastSyllableBeforeEUOUAE{%hu}{%c}",
                    next_euouae_id, euouae_follows);
        }
        fprintf(f, "}{");
        write_syllable_point_and_click(f, syllable, status);
        fprintf(f, "}{%d}{",
                gregoriotex_syllable_first_type(syllable->next_syllable));
    } else {
        fprintf(f, "{\\GreSetNextSyllable{}{}{}{}{}");
        if (end_of_line || first_of_disc == 1) {
            fprintf(f, "\\GreLastOfLine");
        }
        fprintf(f, "}{");
        write_syllable_point_and_click(f, syllable, status);
        fprintf(f, "}{16}{");
    }
    if (syllable->translation) {
        if (syllable->translation_type == TR_WITH_CENTER_BEGINNING) {
            fprintf(f, "%%\n\\GreWriteTranslationWithCenterBeginning{");
        } else {
            fprintf(f, "%%\n\\GreWriteTranslation{");
        }
        write_translation(f, syllable->translation);
        fprintf(f, "}%%\n");
    }
    if (syllable->translation_type) {
        if (syllable->translation_type == TR_WITH_CENTER_END)
            fprintf(f, "%%\n\\GreTranslationCenterEnd %%\n");
    }
    if (syllable->abovelinestext) {
        fprintf(f, "%%\n\\GreSetTextAboveLines{%s}%%\n",
                syllable->abovelinestext);
    }
    fprintf(f, "}{%%\n");

    fprintf(f, "\\GreSyllableNoteCount{%u}%%\n", syllable->elements?
            count_note_units(*syllable->elements) : 0);

    note_unit_count = 0;
    if (syllable->elements) {
        for (element = *syllable->elements; element; element = element->next) {
            if (element->nabc_lines && element->nabc) {
                signed char high_pitch = UNDETERMINED_HEIGHT;
                signed char low_pitch = UNDETERMINED_HEIGHT;
                size_t i;
                compute_element_height_extrema(element, &high_pitch,
                        &low_pitch);
                fixup_height_extrema(&high_pitch, &low_pitch);
                for (i = 0; i < element->nabc_lines; i++) {
                    if (element->nabc[i]) {
                        fprintf(f, "\\GreNABCNeumes{%d}{", (int)(i+1));
                        tex_escape_text(f, element->nabc[i]);
                        fprintf(f, "}{%d}{%d}%%\n", pitch_value(high_pitch),
                                pitch_value(low_pitch));
                    }
                }
            }
            switch (element->type) {
            case GRE_SPACE:
                switch (element->u.misc.unpitched.info.space) {
                case SP_LARGER_SPACE:
                    fprintf(f, "\\GreEndOfElement{1}{0}{%u}%%\n",
                            note_unit_count);
                    break;
                case SP_GLYPH_SPACE:
                    fprintf(f, "\\GreEndOfElement{2}{0}{%u}%%\n",
                            note_unit_count);
                    break;
                case SP_NEUMATIC_CUT:
                    fprintf(f, "\\GreEndOfElement{0}{0}{%u}%%\n",
                            note_unit_count);
                    break;
                case SP_AD_HOC_SPACE:
                    fprintf(f, "\\GreAdHocSpaceEndOfElement{%s}{0}{%u}%%\n",
                            element->u.misc.unpitched.info.ad_hoc_space_factor,
                            note_unit_count);
                    break;
                case SP_GLYPH_SPACE_NB:
                    fprintf(f, "\\GreEndOfElement{2}{1}{%u}%%\n",
                            note_unit_count);
                    break;
                case SP_LARGER_SPACE_NB:
                    fprintf(f, "\\GreEndOfElement{1}{1}{%u}%%\n",
                            note_unit_count);
                    break;
                case SP_NEUMATIC_CUT_NB:
                    fprintf(f, "\\GreEndOfElement{0}{1}{%u}%%\n",
                            note_unit_count);
                    break;
                case SP_AD_HOC_SPACE_NB:
                    fprintf(f, "\\GreAdHocSpaceEndOfElement{%s}{1}{%u}%%\n",
                            element->u.misc.unpitched.info.ad_hoc_space_factor,
                            note_unit_count);
                    break;
                default:
                    /* not reachable unless there's a programming error */
                    /* LCOV_EXCL_START */
                    gregorio_fail(write_syllable,
                            "encountered an unexpected element-level space");
                    break;
                    /* LCOV_EXCL_STOP */
                }
                break;

            case GRE_TEXVERB_ELEMENT:
                if (element->texverb) {
                    fprintf(f, "%% verbatim text at element level:\n%s%%\n"
                            "%% end of verbatim text\n",
                            gregorio_texverb(element->texverb));
                }
                break;

            case GRE_NLBA:
                if (element->u.misc.unpitched.info.nlba == NLBA_BEGINNING) {
                    fprintf(f, "\\GreBeginNLBArea{0}{0}%%\n");
                } else {
                    fprintf(f, "\\GreEndNLBArea{%d}{0}%%\n",
                            next_is_bar(syllable, element)? 3 : 0);
                }
                break;

            case GRE_ALT:
                if (element->texverb) {
                    fprintf(f, "\\GreSetTextAboveLines{%s}%%\n",
                            gregorio_texverb(element->texverb));
                }
                break;

            case GRE_CLEF:
                /* We don't print clef changes at the end of a line */
                if (first_of_disc != 1) {
                    if (is_before_linebreak(syllable, element)) {
                        signed char next_note_pitch;
                        gregorio_shape next_note_alteration;

                        next_note_pitch = gregorio_adjust_pitch_into_staff(
                                score, gregorio_determine_next_pitch(
                                syllable, element, NULL, &next_note_alteration)
                                - element->u.misc.clef.pitch_difference);

                        fputs(next_custos(next_note_pitch, next_note_alteration,
                                status), f);
                        gregoriotex_print_change_line_clef(f, element);
                    } else {
                        /* the third argument is 0 or 1 according to the need
                         * for a space before the clef */
                        fprintf(f, "\\GreChangeClef{%c}{%d}{%c}{%d}{%c}{%d}{%d}%%\n",
                                gregorio_clef_to_char(element->u.misc.clef.clef),
                                element->u.misc.clef.line,
                                (!element->previous || element->previous->type
                                 == GRE_BAR)? '0' : '1',
                                clef_flat_height(element->u.misc.clef.clef,
                                        element->u.misc.clef.line,
                                        element->u.misc.clef.flatted),
                                gregorio_clef_to_char(
                                        element->u.misc.clef.secondary_clef),
                                element->u.misc.clef.secondary_line,
                                clef_flat_height(
                                        element->u.misc.clef.secondary_clef,
                                        element->u.misc.clef.secondary_line,
                                        element->u.misc.clef.secondary_flatted));
                    }
                }
                break;

            case GRE_CUSTOS:
                if (first_of_disc != 1) {
                    signed char next_note_pitch;
                    gregorio_shape next_note_alteration;
                    const char *alteration = "";
                    /* We don't print custos before a bar at the end of a line.
                     * We also print an unbreakable larger space before the
                     * custos */
                    handle_last_of_voice(f, syllable, element, *last_of_voice);
                    next_note_pitch = gregorio_determine_next_pitch(syllable,
                            element, NULL, &next_note_alteration);
                    if (!element->u.misc.pitched.force_pitch) {
                        alteration = alteration_name(next_note_alteration);
                    }
                    fprintf(f, "\\GreCustos{%d}{%s}%s%%\n",
                            pitch_value(element->u.misc.pitched.pitch), alteration,
                            next_custos(next_note_pitch, next_note_alteration,
                            status));
                    ++note_unit_count;
                }
                break;

            case GRE_SUPPRESS_CUSTOS:
                handle_last_of_voice(f, syllable, element, *last_of_voice);
                fprintf(f, "\\GreSuppressEolCustos %%\n");
                status->suppressed_custos = true;
                break;

            case GRE_BAR:
                handle_last_of_voice(f, syllable, element, *last_of_voice);
                write_bar(f, score, syllable, element, first_of_disc);
                break;

            case GRE_END_OF_LINE:
                if (!element->next) {
                    write_anticipated_event(f, euouae_follows,
                            eol_forces_custos, next_euouae_id);
                    anticipated_event_written = true;
                }
                /* here we suppose we don't have two linebreaks in the same
                 * syllable */
                if (element->u.misc.unpitched.info.eol_ragged) {
                    fprintf(f, "%%\n%%\n\\GreNewParLine %%\n%%\n%%\n");
                } else {
                    fprintf(f, "%%\n%%\n\\GreNewLine %%\n%%\n%%\n");
                }
                break;

            default:
                /* here current_element->type is GRE_ELEMENT */
                assert(element->type == GRE_ELEMENT);
                handle_last_of_voice(f, syllable, element, *last_of_voice);
                note_unit_count += write_element(f, syllable, element, status,
                        score);
                if (element->next && (element->next->type == GRE_ELEMENT
                                || (element->next->next
                                        && element->next->type == GRE_ALT
                                        && element->next->next->type ==
                                        GRE_ELEMENT))) {
                    fprintf(f, "\\GreEndOfElement{0}{0}{%u}%%\n",
                            note_unit_count);
                }
                break;
            }
        }
    }
    if (!anticipated_event_written) {
        write_anticipated_event(f, euouae_follows, eol_forces_custos,
                next_euouae_id);
    }
    fprintf(f, "}%%\n");
    if (syllable->position == WORD_END
            || syllable->position == WORD_ONE_SYLLABLE || !syllable->text) {
        fprintf(f, "%%\n");
    }
    finish_syllable(f, syllable);
}

static char *digest_to_hex(const unsigned char digest[SHA1_DIGEST_SIZE])
{
    static const char *const hex = "0123456789abcdef";
    static char result[41];

    char *p = result;
    unsigned char byte;

    int i;
    for (i = 0; i < SHA1_DIGEST_SIZE; ++i) {
        byte = digest[i];

        *(p++) = hex[(byte >> 4) & 0x0FU];
        *(p++) = hex[byte & 0x0FU];
    }

    *p = '\0';

    return result;
}

static void initialize_score(gregoriotex_status *const status,
        gregorio_score *score, const bool point_and_click,
        const gregorio_element **const last_of_voice)
{
    gregorio_syllable *syllable;

    status->bottom_line = false;
    status->top_height = status->bottom_height = UNDETERMINED_HEIGHT;
    status->abovelinestext = status->translation = false;
    status->suppressed_custos = false;

    /* first pass to compute positioning */
    for (syllable = score->first_syllable; syllable;
            syllable = syllable->next_syllable) {
        int voice;

        for (voice = 0; voice < score->number_of_voices; ++voice) {
            gregoriotex_compute_positioning(syllable->elements[voice], score);
        }
    }

    gregoriotex_compute_cross_syllable_positioning(score);

    for (syllable = score->first_syllable; syllable;
            syllable = syllable->next_syllable) {
        int voice;

        if (syllable->translation) {
            status->translation = true;
        }

        if (syllable->abovelinestext) {
            status->abovelinestext = true;
        }

        /* simultaneously compute height extrema and determine the last "real"
         * element in each voice */
        for (voice = 0; voice < score->number_of_voices; ++voice) {
            gregorio_element *element;

            for (element = syllable->elements[voice]; element;
                    element = element->next) {
                gregorio_glyph *glyph;

                switch (element->type) {
                case GRE_ALT:
                    status->abovelinestext = true;
                    break;

                case GRE_CUSTOS:
                    last_of_voice[voice] = element;
                    break;

                case GRE_ELEMENT:
                    for (glyph = element->u.first_glyph; glyph;
                            glyph = glyph->next) {
                        if (glyph->type == GRE_GLYPH) {
                            last_of_voice[voice] = element;
                            compute_height_extrema(glyph,
                                    glyph->u.notes.first_note,
                                    &(status->top_height),
                                    &(status->bottom_height));
                        }
                    }
                    break;

                case GRE_BAR:
                    last_of_voice[voice] = element;
                    break;

                default:
                    /* to eliminate the warning */
                    break;
                }
            }
        }
    }

    fixup_height_extrema(&(status->top_height), &(status->bottom_height));

    status->point_and_click = point_and_click;
}

static void write_header(FILE *const f, const char *const name,
        const char *const value)
{
    if (value) {
        fprintf(f, "\\GreHeader{");
        tex_escape_text(f, name);
        fprintf(f, "}{");
        tex_escape_text(f, value);
        fprintf(f, "}%%\n");
    }
}

static void write_headers(FILE *const f, gregorio_score *const score)
{
    gregorio_header *header;

    fprintf(f, "\\GreBeginHeaders %%\n");
    for (header = score->headers; header; header = header->next) {
        write_header(f, header->name, header->value);
    }
    fprintf(f, "\\GreEndHeaders %%\n");
}

static void suppress_expansion(FILE *const f, const char *text)
{
    if (!text) {
        return;
    }

    for (; *text; ++text) {
        if (*text == '\\') {
            fprintf(f, "\\noexpand");
        }
        fputc(*text, f);
    }
}

static int first_note_near_clef(const gregorio_score *const score) {
    gregorio_clef_info clef = gregorio_default_clef;
    if (score->first_voice_info) {
        clef = score->first_voice_info->initial_clef;
        if (!clef.secondary_line && !clef.flatted && score->first_syllable
                && score->first_syllable->elements) {
            const gregorio_element *element = score->first_syllable->elements[0];
            if (element && element->type == GRE_ELEMENT) {
                const gregorio_glyph *glyph = element->u.first_glyph;
                if (glyph && glyph->type == GRE_GLYPH) {
                    const signed char clef_pitch = LOW_LINE_PITCH
                        + ((clef.line - 1) * 2);
                    const gregorio_note *low_note = glyph->u.notes.first_note;
                    const gregorio_note *high_note = low_note;
                    switch (glyph->u.notes.glyph_type) {
                    case G_PODATUS:
                        /* next note is above the previous */
                        if (low_note->next) {
                            high_note = low_note->next;
                        }
                        break;
                    case G_FLEXA:
                    case G_PORRECTUS:
                    case G_PORRECTUS_FLEXUS:
                        /* there is a stem the size of the ambitus */
                        if (high_note->next) {
                            low_note = high_note->next;
                        }
                        break;
                    default:
                        /* to prevent the enum warning */
                        break;
                    }
                    if (high_note->u.note.pitch < clef_pitch - 3
                            || low_note->u.note.pitch > clef_pitch + 3) {
                        return 0;
                    }
                }
            }
        }
    }
    return 1;
}

static int clef_size(const gregorio_clef_info *const clef)
{
    /* this is a heuristic clef size approximation:
     * C-clef = 1
     * F-clef = 2
     * flatted = +2
     * unstacked double-clef = +1 (plus the clef size)
     */
    /* purposely making line signed */
    int line_diff = clef->line - clef->secondary_line;
    int size;
    if (clef->clef == CLEF_F) {
        size = 2;
    } else {
        size = 1;
    }
    if (clef->flatted) {
        size += 2;
    }
    if (clef->secondary_line) {
        if (line_diff < -1 || line_diff > 1) { /* stacked */
            int secondary_size;
            if (clef->secondary_clef == CLEF_F) {
                secondary_size = 2;
            } else {
                secondary_size = 1;
            }
            if (clef->secondary_flatted) {
                secondary_size += 2;
            }
            if (secondary_size > size) {
                size = secondary_size;
            }
        } else { /* unstacked */
            size += 1;
            if (clef->secondary_clef == CLEF_F) {
                size += 2;
            } else {
                size += 1;
            }
            if (clef->secondary_flatted) {
                size += 2;
            }
        }
    }
    return size;
}

static void write_largest_clef(FILE *const f, gregorio_score *const score)
{
    const gregorio_syllable *syllable;
    const gregorio_element *element;
    gregorio_clef_info clef = gregorio_default_clef;
    int size;

    if (score->first_voice_info) {
        clef = score->first_voice_info->initial_clef;
    }
    size = clef_size(&clef);

    for (syllable = score->first_syllable; syllable;
            syllable = syllable->next_syllable) {
        if (syllable->elements) {
            for (element = *syllable->elements; element;
                    element = element->next) {
                if (element->type == GRE_CLEF) {
                    const int this_size = clef_size(&element->u.misc.clef);
                    if (this_size > size) {
                        clef = element->u.misc.clef;
                        size = this_size;
                    }
                }
            }
        }
    }

    fprintf(f, "\\GreSetLargestClef{%c}{%d}{%d}{%c}{%d}{%d}%%\n",
            gregorio_clef_to_char(clef.clef), clef.line,
            clef_flat_height(clef.clef, clef.line, clef.flatted),
            gregorio_clef_to_char(clef.secondary_clef), clef.secondary_line,
            clef_flat_height(clef.secondary_clef, clef.secondary_line,
                    clef.secondary_flatted));
}

void gregoriotex_write_score(FILE *const f, gregorio_score *const score,
        const char *const point_and_click_filename)
{
    gregorio_clef_info clef = gregorio_default_clef;
    gregorio_syllable *current_syllable;
    int annotation_num;
    gregoriotex_status status;
    const gregorio_element *last_of_voice[MAX_NUMBER_OF_VOICES];

    memset(last_of_voice, 0, sizeof last_of_voice);
    initialize_score(&status, score, point_and_click_filename != NULL,
            last_of_voice);

    gregorio_assert(f, gregoriotex_write_score, "call with NULL file", return);

    gregorio_assert_only(score->number_of_voices == 1, gregoriotex_write_score,
            "gregoriotex only works in monophony (for the moment)");

    fprintf(f, "%% File generated by gregorio %s\n", GREGORIO_VERSION);
    fprintf(f, "\\GregorioTeXAPIVersion{%s}%%\n", VERSION);

    if (score->name) {
        fprintf(f, "%% Name: %s\n", score->name);
    }
    if (score->author) {
        fprintf(f, "%% Author: %s\n", score->author);
    }
    if (score->gabc_copyright) {
        fprintf(f, "%% The copyright of this gabc is: %s\n",
                score->gabc_copyright);
    }
    if (score->score_copyright) {
        fprintf(f, "%% The copyright of the score is: %s\n",
                score->score_copyright);
    }

    write_headers(f, score);

    if (score->first_voice_info) {
        clef = score->first_voice_info->initial_clef;
    }
    fprintf(f, "\\GreBeginScore{%s}{%d}{%d}{%d}{%d}{%s}{%u}"
            "{\\GreInitialClefPosition{%d}{%d}}%%\n",
            digest_to_hex(score->digest), status.top_height,
            status.bottom_height, bool_to_int(status.translation),
            bool_to_int(status.abovelinestext),
            point_and_click_filename? point_and_click_filename : "",
            score->staff_lines, clef.line, clef.secondary_line);
    if (score->nabc_lines) {
        fprintf(f, "\\GreScoreNABCLines{%d}", (int)score->nabc_lines);
    }
    if (score->annotation[0]) {
        fprintf(f, "\\GreAnnotationLines");
        for (annotation_num = 0; annotation_num < MAX_ANNOTATIONS;
             ++annotation_num) {
            if (score->annotation[annotation_num]) {
                fprintf(f, "{%s}",
                        score->annotation[annotation_num]);
            }
            else {
                fprintf(f, "{}");
            }
        }
        fprintf(f, "%%\n");
    }
    if (score->mode) {
        fprintf(f, "\\GreMode{");
        if (*(score->mode) >= '1' && *(score->mode) <= '8') {
            fprintf(f, "\\GreModeNumber{%c}%s", *(score->mode), score->mode + 1);
        } else {
            fprintf(f, "%s", score->mode);
        }
        fprintf(f, "}{");
        suppress_expansion(f, score->mode_modifier);
        fprintf(f, "}{");
        suppress_expansion(f, score->mode_differentia);
        fprintf(f, "}%%\n");
    }

    write_largest_clef(f, score);
    fprintf(f, "\\GreScoreOpening{%%\n"); /* GreScoreOpening#1 */
    if (score->first_voice_info) {
        gregoriotex_write_voice_info(f, score->first_voice_info);
    }
    fprintf(f, "}{%%\n"); /* GreScoreOpening#2 */
    fprintf(f, "\\GreSetInitialClef{%c}{%d}{%d}{%c}{%d}{%d}{%d}%%\n",
            gregorio_clef_to_char(clef.clef), clef.line,
            clef_flat_height(clef.clef, clef.line, clef.flatted),
            gregorio_clef_to_char(clef.secondary_clef), clef.secondary_line,
            clef_flat_height(clef.secondary_clef, clef.secondary_line,
                    clef.secondary_flatted),
            first_note_near_clef(score));
    fprintf(f, "}{%%\n"); /* GreScoreOpening#3 */
    current_syllable = score->first_syllable;
    if (current_syllable) {
        write_syllable(f, current_syllable, 0, &status, score, last_of_voice,
                write_first_syllable_text);
        current_syllable = current_syllable->next_syllable;
    } else {
        /* edge case: a score with no syllables */
        fprintf(f, "}{}{\\GreSetNoFirstSyllableText}%%\n");
    }
    while (current_syllable) {
        write_syllable(f, current_syllable, 0, &status, score, last_of_voice,
                write_syllable_text);
        current_syllable = current_syllable->next_syllable;
    }
    fprintf(f, "\\GreEndScore %%\n\\endinput %%\n");
}
