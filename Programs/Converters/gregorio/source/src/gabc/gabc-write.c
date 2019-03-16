/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file provides functions for writing gabc from Gregorio structures.
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
 * 
 * This is a simple and easyly understandable output module. If you want to
 * write a module, you can consider it as a model.
 */

#include "config.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h> /* for strchr */
#include "bool.h"
#include "characters.h"
#include "struct.h"
#include "unicode.h"
#include "messages.h"
#include "plugins.h"

#include "gabc.h"

typedef enum {
    GABC_NORMAL,
    GABC_AT_PROTRUSION_FACTOR,
    GABC_IN_PROTRUSION_FACTOR
} gabc_write_state;

static gabc_write_state write_state;

static __inline char pitch_letter(const char height) {
    char result = height + 'a' - LOWEST_PITCH;
    if (result == 'o') {
        return 'p';
    }
    return result;
}

/* not reachable unless there's a programming error */
/* LCOV_EXCL_START */
static __inline void unsupported(const char *fn, const int line,
        const char *type, const char *value)
{
    gregorio_messagef(fn, VERBOSITY_ASSERTION, line, _("unsupported %s %s"),
            type, value);
}
/* LCOV_EXCL_STOP */

/*
 * Output one attribute, allowing for multi-line values 
 */
static void gabc_write_str_attribute(FILE *f, const char *name,
        const char *attr)
{
    if (attr) {
        fprintf(f, "%s: %s%s;\n", name, attr, strchr(attr, '\n') ? ";" : "");
    }
}

/*
 * 
 * Then we start the functions made to write the text of the syllable. See
 * comments on struct.h and struct-utils.c to understand more deeply.
 * 
 * This first function will be called each time we will encounter a
 * gregorio_character which is the beginning of a style.
 * 
 */

static void gabc_write_begin(FILE *f, grestyle_style style)
{
    switch (style) {
    case ST_ITALIC:
        fprintf(f, "<i>");
        break;
    case ST_COLORED:
        fprintf(f, "<c>");
        break;
    case ST_SMALL_CAPS:
        fprintf(f, "<sc>");
        break;
    case ST_BOLD:
        fprintf(f, "<b>");
        break;
    case ST_FORCED_CENTER:
        fprintf(f, "{");
        break;
    case ST_TT:
        fprintf(f, "<tt>");
        break;
    case ST_UNDERLINED:
        fprintf(f, "<ul>");
        break;
    case ST_ELISION:
        fprintf(f, "<e>");
        break;
    case ST_PROTRUSION_FACTOR:
        write_state = GABC_AT_PROTRUSION_FACTOR;
        break;
    case ST_INITIAL:
    case ST_CENTER:
    case ST_FIRST_WORD:
    case ST_FIRST_SYLLABLE:
    case ST_FIRST_SYLLABLE_INITIAL:
    case ST_PROTRUSION:
        /* nothing should be emitted for these */
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        unsupported("gabc_write_begin", __LINE__, "style",
                grestyle_style_to_string(style));
        break;
        /* LCOV_EXCL_STOP */
    }
}

/*
 * 
 * This function is about the same but for ends of styles.
 * 
 */

static void gabc_write_end(FILE *f, grestyle_style style)
{
    switch (style) {
    case ST_ITALIC:
        fprintf(f, "</i>");
        break;
    case ST_COLORED:
        fprintf(f, "</c>");
        break;
    case ST_SMALL_CAPS:
        fprintf(f, "</sc>");
        break;
    case ST_BOLD:
        fprintf(f, "</b>");
        break;
    case ST_FORCED_CENTER:
        fprintf(f, "}");
        break;
    case ST_TT:
        fprintf(f, "</tt>");
        break;
    case ST_UNDERLINED:
        fprintf(f, "</ul>");
        break;
    case ST_ELISION:
        fprintf(f, "</e>");
        break;
    case ST_PROTRUSION_FACTOR:
        if (write_state == GABC_IN_PROTRUSION_FACTOR) {
            fprintf(f, ">");
        }
        break;
    case ST_INITIAL:
    case ST_CENTER:
    case ST_FIRST_WORD:
    case ST_FIRST_SYLLABLE:
    case ST_FIRST_SYLLABLE_INITIAL:
    case ST_PROTRUSION:
        /* nothing should be emitted for these */
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        unsupported("gabc_write_end", __LINE__, "style",
                grestyle_style_to_string(style));
        break;
        /* LCOV_EXCL_STOP */
    }
}

/*
 * 
 * This function writes the special chars. As the specials chars are
 * represented simply in gabc, this function is very simple, but for TeX output 
 * modules, this may be.. a little more difficult.
 * 
 */
static void gabc_write_special_char(FILE *f, const grewchar *first_char)
{
    fprintf(f, "<sp>");
    gregorio_print_unistring(f, first_char);
    fprintf(f, "</sp>");
}

/*
 * 
 * This functions writes verbatim output... but as the previous one it is very
 * simple.
 * 
 */
static void gabc_write_verb(FILE *f, const grewchar *first_char)
{
    if (write_state == GABC_AT_PROTRUSION_FACTOR) {
        /* this is an auto protrusion, so ignore it */
        write_state = GABC_NORMAL;
    } else {
        fprintf(f, "<v>");
        gregorio_print_unistring(f, first_char);
        fprintf(f, "</v>");
    }
}

/*
 * 
 * The function called when we will encounter a character. There may be other
 * representations of the character (for example for Omega), so it is necessary 
 * to have such a function defined in each module.
 * 
 */

static void gabc_print_char(FILE *f, const grewchar to_print)
{
    if (write_state == GABC_AT_PROTRUSION_FACTOR) {
        write_state = GABC_IN_PROTRUSION_FACTOR;
        if (to_print == 'd') {
            fprintf(f, "<pr");
        } else {
            fprintf(f, "<pr:");
            gregorio_print_unichar(f, to_print);
        }
    } else {
        gregorio_print_unichar(f, to_print);
    }
}

/*
 * 
 * Quite important: the function that writes the liquescentia. It is called at
 * the end of the function that writes one glyph.
 * 
 */

static void gabc_write_end_liquescentia(FILE *f, char liquescentia)
{
    switch (liquescentia & TAIL_LIQUESCENTIA_MASK) {
    case L_DEMINUTUS:
        fprintf(f, "~");
        break;
    case L_AUCTUS_ASCENDENS:
        fprintf(f, "<");
        break;
    case L_AUCTUS_DESCENDENS:
        fprintf(f, ">");
        break;
    }
}

/*
 * 
 * The function that writes a key change... quite simple.
 * 
 */

static void gabc_write_clef(FILE *f, gregorio_clef_info clef)
{
    fprintf(f, "%c%s%d", clef.clef == CLEF_C? 'c' : 'f', clef.flatted? "b" : "",
            clef.line);
    if (clef.secondary_line) {
        fprintf(f, "@%c%s%d", clef.secondary_clef == CLEF_C? 'c' : 'f',
                clef.secondary_flatted? "b" : "", clef.secondary_line);
    }
}

/*
 * 
 * The function that writes spaces, called when we encounter one.
 * 
 */

static void gabc_write_space(FILE *f, gregorio_space type, char *factor,
        bool next_is_space)
{
    switch (type) {
    case SP_NEUMATIC_CUT:
        if (next_is_space) {
            /* if the following is not a space, we omit this because the
             * code always puts a "/" between elements unless there is some
             * other space there */
            fprintf (f, "/");
        }
        break;
    case SP_LARGER_SPACE:
        fprintf(f, "//");
        break;
    case SP_GLYPH_SPACE:
        fprintf(f, " ");
        break;
    case SP_AD_HOC_SPACE:
        fprintf(f, "/[%s]", factor);
        break;
    case SP_NEUMATIC_CUT_NB:
        fprintf(f, "!/");
        break;
    case SP_LARGER_SPACE_NB:
        fprintf(f, "!//");
        break;
    case SP_GLYPH_SPACE_NB:
        fprintf(f, "! ");
        break;
    case SP_AD_HOC_SPACE_NB:
        fprintf(f, "!/[%s]", factor);
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        unsupported("gabc_write_space", __LINE__, "space type",
                gregorio_space_to_string(type));
        break;
        /* LCOV_EXCL_STOP */
    }
}

/*
 * 
 * A function to write a bar.
 * 
 */

static void gabc_write_bar(FILE *f, gregorio_bar type)
{
    switch (type) {
    case B_VIRGULA:
        fprintf(f, "`");
        break;
    case B_DIVISIO_MINIMA:
        fprintf(f, ",");
        break;
    case B_DIVISIO_MINOR:
        fprintf(f, ";");
        break;
    case B_DIVISIO_MAIOR:
        fprintf(f, ":");
        break;
    case B_DIVISIO_FINALIS:
        fprintf(f, "::");
        break;
    case B_DIVISIO_MINOR_D1:
        fprintf(f, ";1");
        break;
    case B_DIVISIO_MINOR_D2:
        fprintf(f, ";2");
        break;
    case B_DIVISIO_MINOR_D3:
        fprintf(f, ";3");
        break;
    case B_DIVISIO_MINOR_D4:
        fprintf(f, ";4");
        break;
    case B_DIVISIO_MINOR_D5:
        fprintf(f, ";5");
        break;
    case B_DIVISIO_MINOR_D6:
        fprintf(f, ";6");
        break;
    case B_DIVISIO_MINOR_D7:
        fprintf(f, ";7");
        break;
    case B_DIVISIO_MINOR_D8:
        fprintf(f, ";8");
        break;
    case B_VIRGULA_HIGH:
        fprintf(f, "`0");
        break;
    case B_DIVISIO_MINIMA_HIGH:
        fprintf(f, ",0");
        break;
    case B_DIVISIO_MAIOR_DOTTED:
        fprintf(f, ":?");
        break;
    case B_DIVISIO_MINIMIS:
        fprintf(f, "^");
        break;
    case B_DIVISIO_MINIMIS_HIGH:
        fprintf(f, "^0");
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        unsupported("gabc_write_bar", __LINE__, "bar type",
                gregorio_bar_to_string(type));
        break;
        /* LCOV_EXCL_STOP */
    }
}

/* writing the signs of a bar */

static void gabc_write_bar_signs(FILE *f, gregorio_sign type)
{
    switch (type) {
    case _V_EPISEMA:
        fprintf(f, "'");
        break;
    case _V_EPISEMA_BAR_H_EPISEMA:
        fprintf(f, "'_");
        break;
    case _BAR_H_EPISEMA:
        fprintf(f, "_");
        break;
    case _NO_SIGN:
        /* if there's no sign, don't emit anything */
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        unsupported("gabc_write_bar_signs", __LINE__, "bar signs",
                gregorio_sign_to_string(type));
        break;
        /* LCOV_EXCL_STOP */
    }
}

static void gabc_hepisema(FILE *f, const char *prefix, bool connect,
        grehepisema_size size)
{
    fprintf(f, "_%s", prefix);
    if (!connect) {
        fprintf(f, "2");
    }
    switch (size) {
    case H_SMALL_LEFT:
        fprintf(f, "3");
        break;
    case H_SMALL_CENTRE:
        fprintf(f, "4");
        break;
    case H_SMALL_RIGHT:
        fprintf(f, "5");
        break;
    case H_NORMAL:
        /* nothing to print */
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        unsupported("gabc_hepisema", __LINE__, "hepisema size",
                grehepisema_size_to_string(size));
        break;
        /* LCOV_EXCL_STOP */
    }
}

static const char *vepisema_position(gregorio_note *note)
{
    if (!note->v_episema_height) {
        return "";
    }
    if (note->v_episema_height < note->u.note.pitch) {
        return "0";
    }
    return "1";
}

static const char *mora_vposition(gregorio_note *note)
{
    switch (note->mora_vposition) {
    case VPOS_AUTO:
        return "";
    case VPOS_ABOVE:
        return "1";
    case VPOS_BELOW:
        return "0";
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        unsupported("mora_vposition", __LINE__, "vposition",
                gregorio_vposition_to_string(note->mora_vposition));
        return "";
        /* LCOV_EXCL_STOP */
    }
}

static void write_note_heuristics(FILE *f, gregorio_note *note) {
    switch (note->high_ledger_specificity) {
    case LEDGER_EXPLICIT:
        fprintf(f, "[hl:%c]", note->high_ledger_line? '1' : '0');
        break;
    case LEDGER_EXPLICITLY_DRAWN:
        fprintf(f, "[oll:%c]", note->high_ledger_line? '1' : '0');
        break;
    default:
        break;
    }
    switch (note->low_ledger_specificity) {
    case LEDGER_EXPLICIT:
        fprintf(f, "[ll:%c]", note->low_ledger_line? '1' : '0');
        break;
    case LEDGER_EXPLICITLY_DRAWN:
        fprintf(f, "[ull:%c]", note->low_ledger_line? '1' : '0');
        break;
    default:
        break;
    }
}

typedef struct glyph_context {
    gregorio_syllable *syllable;
    gregorio_element *element;
    unsigned short he_adjustment_index[2];
} glyph_context;

/*
 * 
 * The function that writes one gregorio_note.
 * 
 */

static void gabc_write_gregorio_note(FILE *f, gregorio_note *note,
        const bool is_quadratum)
{
    char shape;
    gregorio_assert(note, gabc_write_gregorio_note, "call with NULL argument",
            return);
    gregorio_assert(note->type == GRE_NOTE, gabc_write_gregorio_note,
            "call with argument which type is not GRE_NOTE", return);
    shape = note->u.note.shape;
    switch (shape) {
        /* first we write the letters that determine the shapes */
    case S_PUNCTUM:
        if (is_quadratum) {
            fprintf(f, "%cq", pitch_letter(note->u.note.pitch));
        } else {
            fprintf(f, "%c", pitch_letter(note->u.note.pitch));
        }
        break;
    case S_PUNCTUM_INCLINATUM_ASCENDENS:
        fprintf(f, "%c1", toupper((unsigned char)pitch_letter(note->u.note.pitch)));
        break;
    case S_PUNCTUM_INCLINATUM_DESCENDENS:
        fprintf(f, "%c0", toupper((unsigned char)pitch_letter(note->u.note.pitch)));
        break;
    case S_PUNCTUM_INCLINATUM_STANS:
        fprintf(f, "%c2", toupper((unsigned char)pitch_letter(note->u.note.pitch)));
        break;
    case S_PUNCTUM_INCLINATUM_DEMINUTUS:
        if (note->next) {
            fprintf(f, "%c~", toupper((unsigned char)pitch_letter(note->u.note.pitch)));
        } else {
            fprintf(f, "%c", toupper((unsigned char)pitch_letter(note->u.note.pitch)));
        }
        break;
    case S_PUNCTUM_INCLINATUM_AUCTUS:
        fprintf(f, "%c", toupper((unsigned char)pitch_letter(note->u.note.pitch)));
        break;
    case S_FLAT:
        fprintf(f, "%cx", pitch_letter(note->u.note.pitch));
        break;
    case S_NATURAL:
        fprintf(f, "%cy", pitch_letter(note->u.note.pitch));
        break;
    case S_SHARP:
        fprintf(f, "%c#", pitch_letter(note->u.note.pitch));
        break;
    case S_VIRGA:
        fprintf(f, "%cv", pitch_letter(note->u.note.pitch));
        break;
    case S_VIRGA_REVERSA:
        fprintf(f, "%cV", pitch_letter(note->u.note.pitch));
        break;
    case S_ORISCUS_ASCENDENS:
        fprintf(f, "%co1", pitch_letter(note->u.note.pitch));
        break;
    case S_ORISCUS_DESCENDENS:
        fprintf(f, "%co0", pitch_letter(note->u.note.pitch));
        break;
    case S_ORISCUS_DEMINUTUS:
        fprintf(f, "%co", pitch_letter(note->u.note.pitch));
        /* Note: the DEMINUTUS is also in the liquescentia */
        break;
    case S_QUILISMA:
        if (is_quadratum) {
            fprintf(f, "%cW", pitch_letter(note->u.note.pitch));
        } else {
            fprintf(f, "%cw", pitch_letter(note->u.note.pitch));
        }
        break;
    case S_LINEA:
        fprintf(f, "%c=", pitch_letter(note->u.note.pitch));
        break;
    case S_LINEA_PUNCTUM:
        fprintf(f, "%cR", pitch_letter(note->u.note.pitch));
        break;
    case S_ORISCUS_SCAPUS_ASCENDENS:
        fprintf(f, "%cO1", pitch_letter(note->u.note.pitch));
        break;
    case S_ORISCUS_SCAPUS_DESCENDENS:
        fprintf(f, "%cO0", pitch_letter(note->u.note.pitch));
        break;
    case S_STROPHA:
    case S_STROPHA_AUCTA:
        fprintf(f, "%cs", pitch_letter(note->u.note.pitch));
        break;
    default:
        /* includes S_BIVIRGA, S_TRIVIRGA, S_DISTROPHA, and S_TRISTROPHA */
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        unsupported("gabc_write_gregorio_note", __LINE__, "shape",
                gregorio_shape_to_string(shape));
        fprintf(f, "%c", pitch_letter(note->u.note.pitch));
        break;
        /* LCOV_EXCL_STOP */
    }
    if (note->u.note.is_cavum) {
        fprintf(f, "r");
    }
    switch (note->signs) {
    case _PUNCTUM_MORA:
        fprintf(f, ".%s", mora_vposition(note));
        break;
    case _AUCTUM_DUPLEX:
        fprintf(f, "..");
        break;
    case _V_EPISEMA:
        fprintf(f, "'%s", vepisema_position(note));
        break;
    case _V_EPISEMA_PUNCTUM_MORA:
        fprintf(f, "'%s.%s", vepisema_position(note), mora_vposition(note));
        break;
    case _V_EPISEMA_AUCTUM_DUPLEX:
        fprintf(f, "'%s..", vepisema_position(note));
        break;
    case _NO_SIGN:
        /* if there's no sign, don't emit anything */
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        unsupported("gabc_write_gregorio_note", __LINE__, "shape signs",
                gregorio_sign_to_string(note->signs));
        break;
        /* LCOV_EXCL_STOP */
    }
    switch (note->special_sign) {
    case _ACCENTUS:
        fprintf(f, "r1");
        break;
    case _ACCENTUS_REVERSUS:
        fprintf(f, "r2");
        break;
    case _CIRCULUS:
        fprintf(f, "r3");
        break;
    case _SEMI_CIRCULUS:
        fprintf(f, "r4");
        break;
    case _SEMI_CIRCULUS_REVERSUS:
        fprintf(f, "r5");
        break;
    case _MUSICA_FICTA_FLAT:
        fprintf(f, "r6");
        break;
    case _MUSICA_FICTA_NATURAL:
        fprintf(f, "r7");
        break;
    case _MUSICA_FICTA_SHARP:
        fprintf(f, "r8");
        break;
    case _NO_SIGN:
        /* if there's no sign, don't emit anything */
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        unsupported("gabc_write_gregorio_note", __LINE__, "special sign",
                gregorio_sign_to_string(note->special_sign));
        break;
        /* LCOV_EXCL_STOP */
    }
    if (note->h_episema_above == HEPISEMA_AUTO
            && note->h_episema_below == HEPISEMA_AUTO) {
        gabc_hepisema(f, "", note->h_episema_above_connect,
                note->h_episema_above_size);
    } else {
        if (note->h_episema_below == HEPISEMA_FORCED) {
            gabc_hepisema(f, "0", note->h_episema_below_connect,
                    note->h_episema_below_size);
        }
        if (note->h_episema_above == HEPISEMA_FORCED) {
            gabc_hepisema(f, "1", note->h_episema_above_connect,
                    note->h_episema_above_size);
        }
    }
    write_note_heuristics(f, note);
    if (note->texverb) {
        fprintf(f, "[nv:%s]", gregorio_texverb(note->texverb));
    }
}

static void get_next_hepisema_adjustments(unsigned short *adjustment_index,
        const gregorio_syllable *syllable, const gregorio_element *element,
        const gregorio_glyph *glyph, const gregorio_note *note)
{
    while (note) {
        note = note->next;
        if (!note) {
            while (glyph) {
                glyph = glyph->next;
                if (!glyph) {
                    while (element) {
                        element = element->next;
                        if (!element) {
                            syllable = syllable->next_syllable;
                            if (syllable) {
                                element = syllable->elements[0];
                            }
                        }
                        if (element && element->type == GRE_ELEMENT) {
                            glyph = element->u.first_glyph;
                            break;
                        }
                    }
                }
                if (glyph && glyph->type == GRE_GLYPH) {
                    note = glyph->u.notes.first_note;
                    break;
                }
            }
        }
        if (note && note->type == GRE_NOTE) {
            break;
        }
    }

    if (note) {
        adjustment_index[SO_OVER] = note->he_adjustment_index[SO_OVER];
        adjustment_index[SO_UNDER] = note->he_adjustment_index[SO_UNDER];
    } else {
        adjustment_index[SO_OVER] = 0;
        adjustment_index[SO_UNDER] = 0;
    }
}

static __inline void emit_hepisema_adjustment(FILE *const f,
        const gregorio_note *const note, const gregorio_sign_orientation index,
        const char which, const bool open_brace)
{
    gregorio_hepisema_adjustment *adj = gregorio_get_hepisema_adjustment(
            note->he_adjustment_index[index]);

    fprintf(f, "[%ch", which);
    if (adj->vbasepos || adj->nudge) {
        fputc(':', f);
        switch (adj->vbasepos) {
        case HVB_AUTO:
            break;
        case HVB_MIDDLE:
            fputc('m', f);
            break;
        case HVB_O_LOW:
            if (index == SO_OVER) {
                fputc('l', f);
            } else {
                fprintf(f, "ol");
            }
            break;
        case HVB_O_HIGH:
            if (index == SO_OVER) {
                fputc('h', f);
            } else {
                fprintf(f, "oh");
            }
            break;
        case HVB_U_LOW:
            if (index == SO_UNDER) {
                fputc('l', f);
            } else {
                fprintf(f, "ul");
            }
            break;
        case HVB_U_HIGH:
            if (index == SO_UNDER) {
                fputc('h', f);
            } else {
                fprintf(f, "uh");
            }
            break;
        }
        if (adj->nudge) {
            fprintf(f, "%s", adj->nudge);
        }
    }
    if (open_brace) {
        fputc('{', f);
    }
    fputc(']', f);
}

static __inline void open_hepisema_adjustment(FILE *const f,
        const gregorio_note *const note,
        const unsigned short *const prev_adjustment_index,
        const unsigned short *const next_adjustment_index,
        const gregorio_sign_orientation index, const char which)
{
    const unsigned short adjustment_index =
            note->he_adjustment_index[index];

    if (adjustment_index
            && adjustment_index != prev_adjustment_index[index]
            && adjustment_index == next_adjustment_index[index]) {
        emit_hepisema_adjustment(f, note, index, which, true);
    }
}

static __inline void close_hepisema_adjustment(FILE *const f,
        const gregorio_note *const note,
        const unsigned short *const prev_adjustment_index,
        const unsigned short *const next_adjustment_index,
        const gregorio_sign_orientation index, const char which)
{
    const unsigned short adjustment_index =
            note->he_adjustment_index[index];

    if (adjustment_index) {
        if (adjustment_index != next_adjustment_index[index]) {
            if (adjustment_index == prev_adjustment_index[index]) {
                fprintf(f, "[%ch}]", which);
            } else {
                emit_hepisema_adjustment(f, note, index, which, false);
            }
        }
    }
}

/*
 * 
 * The function that writes one glyph. If it is really a glyph (meaning not a
 * space or an alteration), we just do like always, a loop on the notes and a
 * call to the function that writes one note on each of them.
 * 
 */

static void gabc_write_gregorio_glyph(FILE *f, gregorio_glyph *glyph,
        glyph_context *context)
{
    unsigned short next_adjustment_index[2] = { 0, 0 };
    gregorio_note *current_note;

    gregorio_assert(glyph, gabc_write_gregorio_glyph, "call with NULL argument",
            return);
    switch (glyph->type) {
    case GRE_TEXVERB_GLYPH:
        if (glyph->texverb) {
            fprintf(f, "[gv:%s]", gregorio_texverb(glyph->texverb));
        }
        break;
    case GRE_SPACE:
        if (glyph->next) {
            switch (glyph->u.misc.unpitched.info.space) {
            case SP_ZERO_WIDTH:
                fprintf(f, "!");
                break;
            case SP_HALF_SPACE:
                fprintf(f, "/0");
                break;
            case SP_INTERGLYPH_SPACE:
                fprintf(f, "/!");
                break;
            default:
                /* not reachable unless there's a programming error */
                /* LCOV_EXCL_START */
                unsupported("gabc_write_gregorio_glyph", __LINE__, "space type",
                        gregorio_space_to_string(
                            glyph->u.misc.unpitched.info.space));
                break;
                /* LCOV_EXCL_STOP */
            }
        } else {
            /* not reachable unless there's a programming error */
            /* LCOV_EXCL_START */
            gregorio_fail(gabc_write_gregorio_glyph, "bad space");
            /* LCOV_EXCL_STOP */
        }
        break;
    case GRE_GLYPH:
        if (is_initio_debilis(glyph->u.notes.liquescentia)) {
            fprintf(f, "-");
        } else if (is_fused(glyph->u.notes.liquescentia)) {
            fprintf(f, "@");
        }

        current_note = glyph->u.notes.first_note;
        while (current_note) {
            get_next_hepisema_adjustments(next_adjustment_index,
                    context->syllable, context->element, glyph, current_note);

            open_hepisema_adjustment(f, current_note,
                    context->he_adjustment_index, next_adjustment_index,
                    SO_OVER, 'o');
            open_hepisema_adjustment(f, current_note,
                    context->he_adjustment_index, next_adjustment_index,
                    SO_UNDER, 'u');

            /* third argument necessary for the special shape pes quadratum */
            gabc_write_gregorio_note(f, current_note,
                    glyph->u.notes.glyph_type == G_PES_QUADRATUM
                    && current_note == glyph->u.notes.first_note);

            close_hepisema_adjustment(f, current_note,
                    context->he_adjustment_index, next_adjustment_index,
                    SO_OVER, 'o');
            close_hepisema_adjustment(f, current_note,
                    context->he_adjustment_index, next_adjustment_index,
                    SO_UNDER, 'u');

            context->he_adjustment_index[SO_OVER] =
                    current_note->he_adjustment_index[SO_OVER];
            context->he_adjustment_index[SO_UNDER] =
                    current_note->he_adjustment_index[SO_UNDER];

            current_note = current_note->next;
        }
        gabc_write_end_liquescentia(f, glyph->u.notes.liquescentia);
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        unsupported("gabc_write_gregorio_glyph", __LINE__, "glyph type",
                gregorio_type_to_string(glyph->type));
        break;
        /* LCOV_EXCL_STOP */
    }
}

/*
 * 
 * To write an element, first we check the type of the element (if it is a bar, 
 * etc.), and if it is really an element, we make a loop on the list of glyphs
 * inside the neume, and for each of them we call the function that will write
 * one glyph.
 * 
 */

static void gabc_write_gregorio_element(FILE *f, gregorio_element *element,
        glyph_context *context)
{
    gregorio_glyph *current_glyph;
    gregorio_assert(element, gabc_write_gregorio_element,
            "call with NULL argument", return);
    current_glyph = element->u.first_glyph;
    switch (element->type) {
    case GRE_ELEMENT:
        while (current_glyph) {
            gabc_write_gregorio_glyph(f, current_glyph, context);
            current_glyph = current_glyph->next;
        }
        break;
    case GRE_TEXVERB_ELEMENT:
        if (element->texverb) {
            fprintf(f, "[ev:%s]", gregorio_texverb(element->texverb));
        }
        break;
    case GRE_ALT:
        if (element->texverb) {
            fprintf(f, "[alt:%s]", gregorio_texverb(element->texverb));
        }
        break;
    case GRE_SPACE:
        gabc_write_space(f, element->u.misc.unpitched.info.space,
                element->u.misc.unpitched.info.ad_hoc_space_factor,
                element->next && element->next->type == GRE_SPACE);
        break;
    case GRE_BAR:
        gabc_write_bar(f, element->u.misc.unpitched.info.bar);
        gabc_write_bar_signs(f, element->u.misc.unpitched.special_sign);
        break;
    case GRE_CLEF:
        gabc_write_clef(f, element->u.misc.clef);
        break;
    case GRE_END_OF_LINE:
        if (element->u.misc.unpitched.info.eol_ragged) {
            fprintf(f, "Z");
        } else {
            fprintf(f, "z");
        }
        if (element->u.misc.unpitched.info.eol_forces_custos) {
            fprintf(f, element->u.misc.unpitched.info.eol_forces_custos_on? "+"
                    : "-");
        }
        break;
    case GRE_CUSTOS:
        if (element->u.misc.pitched.force_pitch) {
            fprintf(f, "%c+", pitch_letter(element->u.misc.pitched.pitch));
        } else {
            fprintf(f, "z0");
        }
        break;
    case GRE_SUPPRESS_CUSTOS:
        fprintf(f, "[nocustos]");
        break;
    case GRE_NLBA:
        switch (element->u.misc.unpitched.info.nlba) {
        case NLBA_BEGINNING:
            fprintf(f, "<nlba>");
            break;
        case NLBA_END:
            fprintf(f, "</nlba>");
            break;
        default:
            /* not reachable unless there's a programming error */
            /* LCOV_EXCL_START */
            unsupported("gabc_write_gregorio_element", __LINE__, "nlba type",
                    gregorio_nlba_to_string(element->u.misc.unpitched.info.nlba));
            break;
            /* LCOV_EXCL_STOP */
        }
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        unsupported("gabc_write_gregorio_element", __LINE__, "element type",
                gregorio_type_to_string(element->type));
        break;
        /* LCOV_EXCL_STOP */
    }
}

/*
 * 
 * Here is defined the function that will write the list of gregorio_elements.
 * It is very simple: it makes a loop in which it calls a function that writes
 * one element.
 * 
 */

static bool gabc_write_gregorio_elements(FILE *f, gregorio_element *element,
        glyph_context *context)
{
    bool linebreak_or_bar_in_element = false;
    while (element) {
        context->element = element;
        gabc_write_gregorio_element(f, element, context);
        /* we don't want a bar after an end of line */
        if (element->type != GRE_END_OF_LINE
            && (element->type != GRE_SPACE
                || element->u.misc.unpitched.info.space == SP_NEUMATIC_CUT)
            && element->next && element->next->type == GRE_ELEMENT) {
            fprintf(f, "/");
        }
        if (element->type == GRE_END_OF_LINE || element->type == GRE_BAR)
        {
            linebreak_or_bar_in_element = true;
        }
        element = element->next;
    }
    return linebreak_or_bar_in_element;
}

/*
 * 
 * Here it goes, we are writing a gregorio_syllable.
 * 
 */

static void gabc_write_gregorio_syllable(FILE *f, gregorio_syllable *syllable,
        glyph_context *context)
{
    bool linebreak_or_bar_in_element;
    gregorio_assert(syllable, gabc_write_gregorio_syllable,
            "call with NULL argument", return);
    write_state = GABC_NORMAL;
    if (syllable->no_linebreak_area == NLBA_BEGINNING) {
        fprintf(f, "<nlba>");
    }
    if (syllable->euouae == EUOUAE_BEGINNING) {
        fprintf(f, "<eu>");
    }
    if (syllable->clear) {
        fprintf(f, "<clear>");
    }
    if (syllable->text) {
        /* we call the magic function (defined in struct_utils.c), that will
         * write our text. */
        gregorio_write_text(WTP_NORMAL, syllable->text, f, &gabc_write_verb,
                &gabc_print_char, &gabc_write_begin, &gabc_write_end,
                &gabc_write_special_char);
    }
    if (syllable->translation) {
        fprintf(f, "[");
        gregorio_write_text(WTP_NORMAL, syllable->translation, f,
                &gabc_write_verb, &gabc_print_char, &gabc_write_begin,
                &gabc_write_end, &gabc_write_special_char);
        fprintf(f, "]");
    } else if (syllable->translation_type == TR_WITH_CENTER_END) {
        fprintf(f, "[/]");
    }
    if (syllable->euouae == EUOUAE_END) {
        fprintf(f, "</eu>");
    }
    if (syllable->no_linebreak_area == NLBA_END) {
        fprintf(f, "</nlba>");
    }
    fprintf(f, "(");
    /* we write all the elements of the syllable. */
    linebreak_or_bar_in_element = gabc_write_gregorio_elements(f, syllable->elements[0], context);
    if (linebreak_or_bar_in_element)
    {
        fprintf(f, ")\n");
    } else {
        if (syllable->position == WORD_END
            || syllable->position == WORD_ONE_SYLLABLE
            || gregorio_is_only_special(syllable->elements[0]))
        {
            fprintf(f, ") ");
        } else {
            fprintf(f, ")");
        }
    }
}

/*
 * 
 * This is the top function, the one called when we want to write a
 * gregorio_score in gabc.
 * 
 */

void gabc_write_score(FILE *f, gregorio_score *score)
{
    glyph_context context;
    gregorio_syllable *syllable;
    gregorio_header *header;

    gregorio_assert(f, gabc_write_score, "call with NULL file", return);

    context.he_adjustment_index[0] = 0;
    context.he_adjustment_index[1] = 0;

    for (header = score->headers; header; header = header->next) {
        gabc_write_str_attribute(f, header->name, header->value);
    }
    /* And since the gabc is generated by this program, note this. */
    fprintf(f, "generated-by: %s %s;\n", "gregorio", GREGORIO_VERSION);
    gregorio_assert(score->number_of_voices == 1, gabc_write_score,
            "gregorio_score seems to be empty", return);
    fprintf(f, "%%%%\n");
    /* at present we only allow for one clef at the start of the gabc */
    if (score->first_voice_info) {
        fprintf(f, "(");
        gabc_write_clef(f, score->first_voice_info->initial_clef);
        fprintf(f, ")");
    }
    syllable = score->first_syllable;
    /* the we write every syllable */
    while (syllable) {
        context.syllable = syllable;
        gabc_write_gregorio_syllable(f, syllable, &context);
        syllable = syllable->next_syllable;
    }
    fprintf(f, "\n");
}

/* And that's it... not really hard isn't it? */
