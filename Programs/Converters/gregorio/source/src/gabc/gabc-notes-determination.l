%{
/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file implements the note parser.
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
#include <ctype.h> /* for tolower */
#include "bool.h"
#include "struct.h"
#include "messages.h"
#include "support.h"

#include "gabc.h"

#define YY_NO_INPUT

#define YY_USER_ACTION gabc_update_location(&notes_lloc, \
        gabc_notes_determination_text, gabc_notes_determination_leng);

static gregorio_scanner_location notes_lloc;
static gregorio_note *current_note;
static char char_for_brace;
static unsigned int nbof_isolated_episema;
static char *notesmacros[10];
static char tempstr[256];
static unsigned short overbrace_var = 0, underbrace_var = 0;
static const char *overbrace_var_kind;
static int before_ledger_type;
static char *before_ledger_length = NULL;
static unsigned short ledger_var[2] = { 0, 0 };
static unsigned char staff_lines;
static signed char highest_pitch;
static signed char high_ledger_line_pitch;
static bool legacy_oriscus_orientation;
static unsigned short he_adjustment_index[2] = { 0, 0 };
static signed char bracket_low_pitch, bracket_high_pitch;
static unsigned short left_bracket_texverb = 0;

#define LEDGER(WHICH, SPECIFICITY, VALUE) \
    if (LEDGER_##SPECIFICITY > current_note->WHICH##_ledger_specificity) { \
        current_note->WHICH##_ledger_line = VALUE; \
        current_note->WHICH##_ledger_specificity = LEDGER_##SPECIFICITY; \
    }

typedef struct slur_info {
    unsigned short var;
    char shift;
    gregorio_note *start;
} slur_info;

static slur_info slur[2] = { { 0, '\0', NULL }, { 0, '\0', NULL } };

static __inline gregorio_sign_orientation letter_to_sign_orientation(
        const char letter) {
    switch (letter) {
    case 'u': /* "u"under */
        return SO_UNDER;
    case 'o': /* "o"ver */
        return SO_OVER;
    }
    /* not reachable unless there's a programming error */
    /* LCOV_EXCL_START */
    gregorio_fail2(letter_to_sign_orientation,
            "invalid sign orientation letter: %c", letter);
    return SO_OVER;
    /* LCOV_EXCL_STOP */
}

static __inline int letter_to_pitch_adjustment(const char letter) {
    switch (letter_to_sign_orientation(letter)) {
    case SO_OVER:
        return 1;
    case SO_UNDER:
        return -1;
    }
    /* not reachable unless there's a programming error */
    /* LCOV_EXCL_START */
    gregorio_fail2(letter_to_pitch_adjustment,
            "invalid sign orientation letter: %c", letter);
    return 0;
    /* LCOV_EXCL_STOP */
}

static __inline signed char pitch_letter_to_height(const char pitch) {
    char result = pitch - 'a' + LOWEST_PITCH;
    if (pitch == 'p') {
        --result;
    }
    if (result > highest_pitch) {
        gregorio_messagef("pitch_letter_to_height", VERBOSITY_ERROR, 0,
                _("invalid pitch for %u lines: %c"), (unsigned int)staff_lines,
                pitch);
    }
    if (left_bracket_texverb) {
        if (result < bracket_low_pitch) {
            bracket_low_pitch = result;
        }
        if (result > bracket_high_pitch) {
            bracket_high_pitch = result;
        }
    }
    return result;
}

static gregorio_shape punctum_inclinatum(const char orientation)
{
    switch (orientation) {
    case '0':
        return S_PUNCTUM_INCLINATUM_DESCENDENS;

    case '2':
        return S_PUNCTUM_INCLINATUM_STANS;

    case '1':
        return S_PUNCTUM_INCLINATUM_ASCENDENS;
    }

    return S_PUNCTUM_INCLINATUM_UNDETERMINED;
}

static __inline void lex_add_note(int i, gregorio_shape shape, char signs,
        char liquescentia)
{
    signed char height = pitch_letter_to_height(tolower(
            (unsigned char)gabc_notes_determination_text[i]));

    nbof_isolated_episema = 0;
    gregorio_add_note(&current_note, height, shape, signs, liquescentia, NULL,
            &notes_lloc);
    current_note->he_adjustment_index[SO_OVER] = he_adjustment_index[SO_OVER];
    current_note->he_adjustment_index[SO_UNDER] = he_adjustment_index[SO_UNDER];

    if (height >= high_ledger_line_pitch) {
        LEDGER(high, DRAWN, true);
    } else if (ledger_var[SO_OVER]) {
        LEDGER(high, EXPLICIT, true);
    }

    if (height <= LOW_LEDGER_LINE_PITCH) {
        LEDGER(low, DRAWN, true);
    } else if (ledger_var[SO_UNDER]) {
        LEDGER(low, EXPLICIT, true);
    }
}

static __inline void add_bar_as_note(gregorio_bar bar)
{
    nbof_isolated_episema = 0;
    gregorio_add_bar_as_note(&current_note, bar, &notes_lloc);
}

static __inline void error(void)
{
    gregorio_messagef("gabc_notes_determination", VERBOSITY_ERROR, 0,
            _("undefined macro used: m%d"),
            gabc_notes_determination_text[3] - '0');
}

static void add_h_episema(void)
{
    grehepisema_size size = H_NORMAL;
    gregorio_vposition vposition = VPOS_AUTO;
    bool disable_bridge = false;

    char *ptr = gabc_notes_determination_text;
    char current;
    /* first character is the underscore */
    while ((current = *(++ptr))) {
        switch(current) {
        case '0':
            vposition = VPOS_BELOW;
            break;
        case '1':
            vposition = VPOS_ABOVE;
            break;
        case '2':
            disable_bridge = true;
            break;
        case '3':
            size = H_SMALL_LEFT;
            break;
        case '4':
            size = H_SMALL_CENTRE;
            break;
        case '5':
            size = H_SMALL_RIGHT;
            break;
        default:
            /* not reachable unless there's a programming error */
            /* LCOV_EXCL_START */
            gregorio_fail2(gabc_notes_determination,
                    "unrecognized horizontal episema modifier: %c", current);
            break;
            /* LCOV_EXCL_STOP */
        };
    }

    gregorio_add_h_episema(current_note, size, vposition, disable_bridge,
                &nbof_isolated_episema);
}

static void add_sign(gregorio_sign sign)
{
    gregorio_vposition vposition = VPOS_AUTO;
    switch(gabc_notes_determination_text[1]) {
    case '0':
        vposition = VPOS_BELOW;
        break;
    case '1':
        vposition = VPOS_ABOVE;
        break;
    }
    gregorio_add_sign(current_note, sign, vposition);
}

static void save_before_ledger(const char *const before_ledger)
{
    if (strcmp(before_ledger, "0") == 0) {
        before_ledger_type = 0;
        before_ledger_length = "";
    } else if (strcmp(before_ledger, "1") == 0) {
        before_ledger_type = 1;
        before_ledger_length = "";
    } else {
        before_ledger_type = 2;
        before_ledger_length = gregorio_strdup(before_ledger);
    }
}

static void add_static_ledger(const gregorio_sign_orientation type,
        const char *length) {
    gregorio_snprintf(tempstr, sizeof tempstr,
            "\\GreDrawAdditionalLine{%d}{%s}{%d}{%s}{0}{}",
            type, length + 1, before_ledger_type, before_ledger_length);

    if (before_ledger_type == 2) {
        free(before_ledger_length);
        before_ledger_length = NULL;
    }

    gregorio_add_texverb_as_note(&current_note, gregorio_strdup(tempstr),
            GRE_TEXVERB_GLYPH, &notes_lloc);
}

static __inline const char *over_or_under(
        const gregorio_sign_orientation type) {
    switch (type) {
    case SO_OVER:
        return "over";
    case SO_UNDER:
        return "under";
    }
    /* not reachable unless there's a programming error */
    /* LCOV_EXCL_START */
    gregorio_fail2(over_or_under, "invalid ledger type %d", type);
    return "";
    /* LCOV_EXCL_STOP */
}

static void add_variable_ledger(const gregorio_sign_orientation type,
        const char *after_ledger)
{
    if (ledger_var[type]) {
        const char *const typename = over_or_under(type);
        gregorio_messagef("add_variable_ledger", VERBOSITY_ERROR, 0,
                _("variable %s-staff ledger line without termination of "
                "previous %s-staff ledger line"), typename, typename);
    } else {
        int after_ledger_type;
        const char *after_ledger_length;

        ++after_ledger;

        if (strcmp(after_ledger, "0") == 0) {
            after_ledger_type = 0;
            after_ledger_length = "";
        } else if (strcmp(after_ledger, "1") == 0) {
            after_ledger_type = 1;
            after_ledger_length = "";
        } else {
            after_ledger_type = 2;
            after_ledger_length = after_ledger;
        }

        ledger_var[type] = ++tex_position_id;
        gregorio_snprintf(tempstr, sizeof tempstr,
                "\\GreVarBraceSavePos{%hu}{0}{1}"
                "\\GreDrawAdditionalLine{%d}{\\GreVarBraceLength{%hu}}"
                "{%d}{%s}{%d}{%s}",
                ledger_var[type], type, ledger_var[type], before_ledger_type,
                before_ledger_length, after_ledger_type, after_ledger_length);

        if (before_ledger_type == 2) {
            free(before_ledger_length);
            before_ledger_length = NULL;
        }

        gregorio_add_texverb_as_note(&current_note, gregorio_strdup(tempstr),
                GRE_TEXVERB_GLYPH, &notes_lloc);
    }
}

static void end_variable_ledger(const gregorio_sign_orientation type)
{
    if (!ledger_var[type]) {
        const char *const typename = over_or_under(type);
        gregorio_messagef("end_variable_ledger", VERBOSITY_ERROR, 0,
                _("variable %s-staff ledger line termination without variable "
                "%s-staff ledger line start"), typename, typename);
    } else {
        gregorio_snprintf(tempstr, sizeof tempstr,
                "\\GreVarBraceSavePos{%hu}{0}{2}", ledger_var[type]);
        ledger_var[type] = 0;
        gregorio_add_texverb_as_note(&current_note, gregorio_strdup(tempstr),
                GRE_TEXVERB_GLYPH, &notes_lloc);
    }
}

static __inline int parse_clef_line(char line)
{
    line -= '0';
    if (line < 0 || line > staff_lines) {
        gregorio_messagef("parse_clef_line", VERBOSITY_ERROR, 0,
                _("invalid clef line for %u lines: %d"),
                (unsigned int)staff_lines, (int)line);
        return 1;
    }
    return line;
}

static __inline gregorio_bar parse_dominican_bar(char bar)
{
    bar -= '0';
    if (bar < 1 || bar > (2 * (staff_lines - 1))) {
        gregorio_messagef("parse_dominican_line", VERBOSITY_ERROR, 0,
                _("invalid Dominican bar for %u lines: ;%d"),
                (unsigned int)staff_lines, (int)bar);
    }

    switch (bar) {
    case 1:
        return B_DIVISIO_MINOR_D1;
    case 2:
        return B_DIVISIO_MINOR_D2;
    case 3:
        return B_DIVISIO_MINOR_D3;
    case 4:
        return B_DIVISIO_MINOR_D4;
    case 5:
        return B_DIVISIO_MINOR_D5;
    case 6:
        return B_DIVISIO_MINOR_D6;
    case 7:
        return B_DIVISIO_MINOR_D7;
    case 8:
        return B_DIVISIO_MINOR_D8;
    }
    /* not reachable unless there's a programming error */
    /* LCOV_EXCL_START */
    gregorio_fail2(check_dominican_line, "invalid dominican bar: %d", (int)bar);
    return B_NO_BAR;
    /* LCOV_EXCL_STOP */
}

static __inline gregorio_clef letter_to_clef(char letter)
{
    switch (letter) {
    case 'c':
        return CLEF_C;
    case 'f':
        return CLEF_F;
    }
    /* not reachable unless there's a programming error */
    /* LCOV_EXCL_START */
    gregorio_fail2(letter_to_clef, "invalid clef: %c", letter);
    return CLEF_C;
    /* LCOV_EXCL_STOP */
}

/* this assertion should only fail if the lex rules are incorrect */
static __inline void slur_assert(char *fn, bool test) {
    if (!test) {
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        gregorio_message(_("invalid slur text"), fn, VERBOSITY_FATAL, 0);
        exit(1);
        /* LCOV_EXCL_STOP */
    }
}

static char *parse_slur_shift(char *shift)
{
    char *c;

    c = strchr(gabc_notes_determination_text, ':');
    slur_assert("parse_slur_shift", c != NULL);
    slur_assert("parse_slur_shift", *(++c) != '\0');
    *shift = *c;
    return c;
}

static void parse_slur(void)
{
    const int direction = letter_to_pitch_adjustment(
            gabc_notes_determination_text[1]);
    char shift, *width, *height, *end;

    if (!current_note || current_note->type != GRE_NOTE) {
        gregorio_message(
                _("cannot add a slur to something that is not a note"),
                "parse_slur", VERBOSITY_ERROR, 0);
        return;
    }

    end = parse_slur_shift(&shift);
    width = strchr(end, ';');
    slur_assert("parse_slur", width != NULL);
    height = strchr(++width, ',');
    slur_assert("parse_slur", height != NULL);
    *height = '\0';
    end = strchr(++height, ']');
    slur_assert("parse_slur", end != NULL);
    *end = '\0';

    gregorio_snprintf(tempstr, sizeof tempstr,
            "\\GreSlur{%d}{%d}{%c}{%s}{%s}{}",
            current_note->u.note.pitch + direction, direction, shift, width,
            height);
    gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));
}

static void start_var_slur(void)
{
    const gregorio_sign_orientation index = letter_to_sign_orientation(
            gabc_notes_determination_text[1]);

    if (!current_note || current_note->type != GRE_NOTE) {
        gregorio_message(
                _("cannot add a slur to something that is not a note"),
                "start_var_slur", VERBOSITY_ERROR, 0);
        return;
    }

    if (slur[index].var) {
        gregorio_messagef("start_var_slur", VERBOSITY_ERROR, 0,
                _("variable %s-note slur without termination of previous slur"),
                over_or_under(index));
        return;
    }

    slur[index].var = ++tex_position_id;
    parse_slur_shift(&(slur[index].shift));
    slur[index].start = current_note;
}

static void end_var_slur(void)
{
    const int direction = letter_to_pitch_adjustment(
            gabc_notes_determination_text[1]);
    const gregorio_sign_orientation index = letter_to_sign_orientation(
            gabc_notes_determination_text[1]);
    char shift;

    if (!current_note || current_note->type != GRE_NOTE) {
        gregorio_message(
                _("cannot add a slur to something that is not a note"),
                "end_var_slur", VERBOSITY_ERROR, 0);
        return;
    }

    if (!slur[index].var || !slur[index].shift || !slur[index].start) {
        gregorio_messagef("end_var_slur", VERBOSITY_ERROR, 0,
                _("variable %s-note slur end without start"),
                over_or_under(index));
        return;
    }

    parse_slur_shift(&shift);

    gregorio_snprintf(tempstr, sizeof tempstr,
            "\\GreVarBraceSavePos{%hu}{%c}{1}"
            "\\GreSlur{%d}{%d}{%c}{\\GreVarBraceLength{%hu}}{}{%d}",
            slur[index].var, slur[index].shift,
            slur[index].start->u.note.pitch + direction, direction,
            slur[index].shift, slur[index].var,
            current_note->u.note.pitch + direction);
    gregorio_add_texverb_to_note(slur[index].start, gregorio_strdup(tempstr));

    gregorio_snprintf(tempstr, sizeof tempstr,
            "\\GreVarBraceSavePos{%hu}{%c}{2}", slur[index].var, shift);
    gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));


    slur[index].var = 0;
    slur[index].shift = '\0';
    slur[index].start = NULL;
}

static void left_bracket(void)
{
    if (left_bracket_texverb) {
        gregorio_message(
                _("cannot add a left bracket before closing the previous one"),
                "left_bracket", VERBOSITY_ERROR, 0);
        return;
    }

    /* when setting the left bracket, temporarily store the point-and-click
     * information in the texverb */
    if (notes_lloc.generate_point_and_click) {
        gregorio_snprintf(tempstr, sizeof tempstr, "%u:%u:%u",
                notes_lloc.first_line, notes_lloc.first_offset,
                notes_lloc.first_column + 1);
    } else {
        tempstr[0] = '\0';
    }
    left_bracket_texverb = gregorio_add_texverb_as_note(&current_note,
            gregorio_strdup(tempstr), GRE_TEXVERB_GLYPH, &notes_lloc);
    bracket_low_pitch = MAX_PITCH;
    bracket_high_pitch = LOWEST_PITCH;
}

static void right_bracket(void)
{
    if (!left_bracket_texverb) {
        gregorio_message(
                _("cannot add a right bracket without a matching left bracket"),
                "right_bracket", VERBOSITY_ERROR, 0);
        return;
    }

    if (bracket_high_pitch < bracket_low_pitch) {
        gregorio_message(
                _("cannot add brackets without notes between them"),
                "right_bracket", VERBOSITY_ERROR, 0);
        return;
    }

    gregorio_snprintf(tempstr, sizeof tempstr, "\\GreBracket{0}{%d}{%d}{%s}",
            bracket_low_pitch, bracket_high_pitch,
            gregorio_texverb(left_bracket_texverb));
    gregorio_change_texverb(left_bracket_texverb, gregorio_strdup(tempstr));

    if (notes_lloc.generate_point_and_click) {
        gregorio_snprintf(tempstr, sizeof tempstr,
                "\\GreBracket{1}{%d}{%d}{%u:%u:%u}", bracket_low_pitch,
                bracket_high_pitch, notes_lloc.first_line,
                notes_lloc.first_offset, notes_lloc.first_column + 1);
    } else {
        gregorio_snprintf(tempstr, sizeof tempstr,
                "\\GreBracket{1}{%d}{%d}{}", bracket_low_pitch,
                bracket_high_pitch);
    }
    gregorio_add_texverb_as_note(&current_note, gregorio_strdup(tempstr),
            GRE_TEXVERB_GLYPH, &notes_lloc);

    left_bracket_texverb = 0;
}

static void parse_hepisema_adjustment(void)
{
    /* See https://github.com/gregorio-project/gregorio/issues/872
     *
     * [xh:yzw]
     *
     * - x is o(ver) or u(under)
     * - y is optional and is
     *   - l for low in the space (chooses ol or ul depending on x)
     *   - m for middle of the space
     *   - h for high in the space (chooses oh or uh depending on x)
     *   - ol for low in the space as if the episema were over the note
     *   - oh for high in the space as if the episema were over the note
     *   - ul for low in the space as if the episema were under the note
     *   - uh for high in the space as if the episema were under the note
     * - z is an optional nudge and must start with + or -
     * - w is optional and may be { to start a range or } to end a range.
     *
     * at least one of y, z, or w must be provided
     * if y and z are omitted, the : may be omitted
     * y and z are not permitted when w is }
     */

    const gregorio_sign_orientation index = letter_to_sign_orientation(
            gabc_notes_determination_text[1]);
    gregorio_sign_orientation det_index = index;
    char *ch = gabc_notes_determination_text + 3;
    gregorio_hepisema_vbasepos vbasepos = HVB_AUTO;
    char *nudge = NULL;
    char save;
    short hepisema_adjustment_id;

    if (he_adjustment_index[index]) {
        gregorio_messagef("parse_hepisema_adustment", VERBOSITY_ERROR, 0,
                _("horizontal %s-episema adjustment start before ending the "
                "previous adjustment"), over_or_under(index));
        return;
    }

    if (*ch == ':') {
        ++ch;
        if (*ch == 'm') {
            vbasepos = HVB_MIDDLE;
        } else {
            switch (*ch) {
            case 'o':
                ++ch;
                det_index = SO_OVER;
                break;
            case 'u':
                ++ch;
                det_index = SO_UNDER;
                break;
            }

            switch (*ch) {
            case 'l':
                switch (det_index) {
                case SO_OVER:
                    vbasepos = HVB_O_LOW;
                    break;
                case SO_UNDER:
                    vbasepos = HVB_U_LOW;
                    break;
                }
                break;
            case 'h':
                switch (det_index) {
                case SO_OVER:
                    vbasepos = HVB_O_HIGH;
                    break;
                case SO_UNDER:
                    vbasepos = HVB_U_HIGH;
                    break;
                }
                break;
            }
        }
        if (vbasepos) {
            ++ch;
        }
        if (*ch == '+' || *ch == '-') {
            nudge = ch;
            do {
                ++ch;
            } while (*ch && *ch != '{' && *ch != ']');
            save = *ch;
            *ch = '\0';
            nudge = gregorio_strdup(nudge);
            *ch = save;
        }
    }

    hepisema_adjustment_id = gregorio_add_hepisema_adjustment(vbasepos, nudge);

    if (*ch == '{') {
        he_adjustment_index[index] = hepisema_adjustment_id;
    } else {
        if (!current_note || current_note->type != GRE_NOTE) {
            gregorio_message(_("cannot add a horizontal episema adjustment to "
                    "something that is not a note"),
                    "parse_hepisema_adjustment", VERBOSITY_ERROR, 0);
            return;
        }
        current_note->he_adjustment_index[index] = hepisema_adjustment_id;
    }
}

static void end_hepisema_adjustment(void)
{
    /* [xh:}]
     * - x indicates l(ow) or h(igh) episema
     * - : is optional
     */

    const gregorio_sign_orientation index = letter_to_sign_orientation(
            gabc_notes_determination_text[1]);

    if (!he_adjustment_index[index]) {
        gregorio_messagef("end_hepisema_adustment", VERBOSITY_ERROR, 0,
                _("horizontal %s-episema adjustment end with no matching "
                "start"), over_or_under(index));
        return;
    }

    he_adjustment_index[index] = 0;
}

void gabc_det_notes_finish(void)
{
    gregorio_sign_orientation orientation;
    if (overbrace_var) {
        gregorio_message(_("unclosed variable over-staff brace"),
                "gabc_det_notes_finish", VERBOSITY_ERROR, 0);
        overbrace_var = 0;
    }
    if (underbrace_var) {
        gregorio_message(_("unclosed variable under-staff brace"),
                "gabc_det_notes_finish", VERBOSITY_ERROR, 0);
        underbrace_var = 0;
    }
    for (orientation = SO_OVER; orientation <= SO_UNDER; ++orientation) {
        const char *name = over_or_under(orientation);
        if (ledger_var[orientation]) {
            gregorio_messagef("gabc_det_notes_finish", VERBOSITY_ERROR, 0,
                    _("unclosed variable %s-staff ledger line"), name);
            ledger_var[orientation] = 0;
        }
        if (slur[orientation].var) {
            gregorio_messagef("gabc_det_notes_finish", VERBOSITY_ERROR, 0,
                    _("unclosed variable %s-note slur"), name);
            slur[orientation].var = 0;
            slur[orientation].shift = '\0';
            slur[orientation].start = NULL;
        }
        if (he_adjustment_index[orientation]) {
            gregorio_messagef("gabc_det_notes_finish", VERBOSITY_ERROR, 0,
                    _("unclosed horizontal %s-episema adjustment"),
                    over_or_under(orientation));
            ledger_var[orientation] = 0;
        }
    }
    if (left_bracket_texverb) {
        gregorio_message(_("unclosed left bracket"),
                "gabc_det_notes_finish", VERBOSITY_ERROR, 0);
        left_bracket_texverb = 0;
    }
}

%}

%option stack
%option 8bit
%option pointer
%option nounput
%option noyy_push_state
%option noyy_pop_state
%option noyy_top_state
%option full
%option noread
%option nomain
%option align
%option noyylineno
%option prefix="gabc_notes_determination_"
%option noyywrap

%x texverbnote
%x texverbglyph
%x texverbelement
%x choralsign
%x choralnabc
%x alt
%x comments
%x overbrace
%x underbrace
%x overcurlybrace
%x overcurlyaccentusbrace
%x space
%x nbspace
%x overledger overledger2
%x underledger underledger2
%x endledger

%%
<INITIAL>\% {
        BEGIN(comments);
    }
<comments>(\n|\r)+ {
        BEGIN(INITIAL);
    }
<comments>[^\n\r]* {
        /* ignored */
    }
<INITIAL>\[cs: {
        BEGIN(choralsign);
    }
<INITIAL>\[cn: {
        BEGIN(choralnabc);
    }
<INITIAL>\[ob:[01]; {
        char_for_brace = gabc_notes_determination_text[4]-'0';
        BEGIN(overbrace);
    }
<INITIAL>\[ub:[01]; {
        char_for_brace = gabc_notes_determination_text[4]-'0';
        BEGIN(underbrace);
    }
<INITIAL>\[ocb:[01]; {
        char_for_brace = gabc_notes_determination_text[5]-'0';
        BEGIN(overcurlybrace);
    }
<INITIAL>\[ocba:[01]; {
        char_for_brace = gabc_notes_determination_text[6]-'0';
        BEGIN(overcurlyaccentusbrace);
    }
<INITIAL>\[ob:[01]\{\] {
        if (overbrace_var) {
            gregorio_messagef("gabc_notes_determination", VERBOSITY_ERROR, 0,
                              _("variable overbrace without termination of "
                                "previous overbrace"));
        } else {
            char_for_brace = gabc_notes_determination_text[4]-'0';
            overbrace_var = ++tex_position_id;
            overbrace_var_kind = "ob";
            gregorio_snprintf(tempstr, sizeof tempstr,
                    "\\GreVarBraceSavePos{%hu}{%d}{1}"
                    "\\GreOverBrace{\\GreVarBraceLength{%hu}}{0pt}{0pt}{%d}",
                    overbrace_var, char_for_brace, overbrace_var, char_for_brace);
            gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));
        }
    }
<INITIAL>\[ub:[01]\{\] {
        if (underbrace_var) {
            gregorio_messagef("gabc_notes_determination", VERBOSITY_ERROR, 0,
                              _("variable underbrace without termination of "
                                "previous underbrace"));
        } else {
            char_for_brace = gabc_notes_determination_text[4]-'0';
            underbrace_var = ++tex_position_id;
            gregorio_snprintf(tempstr, sizeof tempstr,
                    "\\GreVarBraceSavePos{%hu}{%d}{1}"
                    "\\GreUnderBrace{\\GreVarBraceLength{%hu}}{0pt}{0pt}{%d}",
                    underbrace_var, char_for_brace, underbrace_var, char_for_brace);
            gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));
        }
    }
<INITIAL>\[ocb:[01]\{\] {
        if (overbrace_var) {
            gregorio_messagef("gabc_notes_determination", VERBOSITY_ERROR, 0,
                              _("variable overbrace without termination of "
                                "previous overbrace"));
        } else {
            char_for_brace = gabc_notes_determination_text[5]-'0';
            overbrace_var = ++tex_position_id;
            overbrace_var_kind = "ocb";
            gregorio_snprintf(tempstr, sizeof tempstr,
                    "\\GreVarBraceSavePos{%hu}{%d}{1}"
                    "\\GreOverCurlyBrace{\\GreVarBraceLength{%hu}}{0pt}{0pt}{%d}{0}",
                    overbrace_var, char_for_brace, overbrace_var, char_for_brace);
            gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));
        }
    }
<INITIAL>\[ocba:[01]\{\] {
        if (overbrace_var) {
            gregorio_messagef("gabc_notes_determination", VERBOSITY_ERROR, 0,
                              _("variable overbrace without termination of "
                                "previous overbrace"));
        } else {
            char_for_brace = gabc_notes_determination_text[6]-'0';
            overbrace_var = ++tex_position_id;
            overbrace_var_kind = "ocba";
            gregorio_snprintf(tempstr, sizeof tempstr,
                    "\\GreVarBraceSavePos{%hu}{%d}{1}"
                    "\\GreOverCurlyBrace{\\GreVarBraceLength{%hu}}{0pt}{0pt}{%d}{1}",
                    overbrace_var, char_for_brace, overbrace_var, char_for_brace);
            gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));
        }
    }
<INITIAL>\[ob:[01]\}\] {
        if (!overbrace_var) {
            gregorio_messagef("gabc_notes_determination", VERBOSITY_ERROR, 0,
                              _("variable overbrace termination without "
                                "variable overbrace start"));
        } else if (strcmp (overbrace_var_kind, "ob")) {
            gregorio_messagef("gabc_notes_determination", VERBOSITY_ERROR, 0,
                              _("variable overbrace started with %s "
                                "and terminated with ob"),
                              overbrace_var_kind);
        } else {
            char_for_brace = gabc_notes_determination_text[4]-'0';
            gregorio_snprintf(tempstr, sizeof tempstr,
                    "\\GreVarBraceSavePos{%hu}{%d}{2}", overbrace_var,
                    char_for_brace);
            overbrace_var = 0;
            gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));
        }
    }
<INITIAL>\[ub:[01]\}\] {
        if (!underbrace_var) {
            gregorio_messagef("gabc_notes_determination", VERBOSITY_ERROR, 0,
                              _("variable underbrace termination without "
                                "variable underbrace start"));
        } else {
            char_for_brace = gabc_notes_determination_text[4]-'0';
            gregorio_snprintf(tempstr, sizeof tempstr,
                    "\\GreVarBraceSavePos{%hu}{%d}{2}", underbrace_var,
                    char_for_brace);
            underbrace_var = 0;
            gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));
        }
    }
<INITIAL>\[ocb:[01]\}\] {
        if (!overbrace_var) {
            gregorio_messagef("gabc_notes_determination", VERBOSITY_ERROR, 0,
                              _("variable overbrace termination without "
                                "variable overbrace start"));
        } else if (strcmp (overbrace_var_kind, "ocb")) {
            gregorio_messagef("gabc_notes_determination", VERBOSITY_ERROR, 0,
                              _("variable overbrace started with %s "
                                "and terminated with ocb"),
                              overbrace_var_kind);
        } else {
            char_for_brace = gabc_notes_determination_text[5]-'0';
            gregorio_snprintf(tempstr, sizeof tempstr,
                    "\\GreVarBraceSavePos{%hu}{%d}{2}", overbrace_var,
                    char_for_brace);
            overbrace_var = 0;
            gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));
        }
    }
<INITIAL>\[ocba:[01]\}\] {
        if (!overbrace_var) {
            gregorio_messagef("gabc_notes_determination", VERBOSITY_ERROR, 0,
                              _("variable overbrace termination without "
                                "variable overbrace start"));
        } else if (strcmp (overbrace_var_kind, "ocba")) {
            gregorio_messagef("gabc_notes_determination", VERBOSITY_ERROR, 0,
                              _("variable overbrace started with %s "
                                "and terminated with ocba"),
                              overbrace_var_kind);
        } else {
            char_for_brace = gabc_notes_determination_text[6]-'0';
            gregorio_snprintf(tempstr, sizeof tempstr,
                    "\\GreVarBraceSavePos{%hu}{%d}{2}", overbrace_var,
                    char_for_brace);
            overbrace_var = 0;
            gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));
        }
    }
<INITIAL>\[nm[1-9]\] {
        if (notesmacros[gabc_notes_determination_text[3]-'0']) {
            gregorio_add_texverb_to_note(current_note, gregorio_strdup(
                    notesmacros[gabc_notes_determination_text[3]-'0']));
        } else error();
    }
<INITIAL>\[gm[1-9]\] {
        if (notesmacros[gabc_notes_determination_text[3]-'0']) {
            gregorio_add_texverb_as_note(&current_note, gregorio_strdup(
                    notesmacros[gabc_notes_determination_text[3]-'0']),
                    GRE_TEXVERB_GLYPH, &notes_lloc);
        } else error();
    }
<INITIAL>\[em[1-9]\] {
        if (notesmacros[gabc_notes_determination_text[3]-'0']) {
            gregorio_add_texverb_as_note(&current_note, gregorio_strdup(
                    notesmacros[gabc_notes_determination_text[3]-'0']),
                    GRE_TEXVERB_ELEMENT, &notes_lloc);
        } else error();
    }
<INITIAL>\[altm[1-9]\] {
        if (notesmacros[gabc_notes_determination_text[5]-'0']) {
            gregorio_add_texverb_as_note(&current_note, gregorio_strdup(
                    notesmacros[gabc_notes_determination_text[5]-'0']),
                    GRE_TEXVERB_ELEMENT, &notes_lloc);
        } else error();
    }
<INITIAL>\[nv: {
        BEGIN(texverbnote);
    }
<INITIAL>\[gv: {
        BEGIN(texverbglyph);
    }
<INITIAL>\[ev: {
        BEGIN(texverbelement);
    }
<INITIAL>\[alt: {
        BEGIN(alt);
    }
<INITIAL><nlba> {
        gregorio_add_nlba_as_note(&current_note, NLBA_BEGINNING, &notes_lloc);
    }
<INITIAL><\/nlba> {
        gregorio_add_nlba_as_note(&current_note, NLBA_END, &notes_lloc);
    }
<INITIAL>@\[ {
        gregorio_start_autofuse(&current_note, &notes_lloc);
    }
<INITIAL>\] {
        gregorio_end_autofuse(&current_note, &notes_lloc);
    }
<overbrace>[^\]]+ {
        gregorio_snprintf(tempstr, sizeof tempstr,
                "\\GreOverBrace{%s}{0pt}{0pt}{%d}",
                gabc_notes_determination_text, char_for_brace);
        gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));
    }
<underbrace>[^\]]+ {
        gregorio_snprintf(tempstr, sizeof tempstr,
                "\\GreUnderBrace{%s}{0pt}{0pt}{%d}",
                gabc_notes_determination_text, char_for_brace);
        gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));
    }
<overcurlybrace>[^\]]+ {
        gregorio_snprintf(tempstr, sizeof tempstr,
                "\\GreOverCurlyBrace{%s}{0pt}{0pt}{%d}{0}",
                gabc_notes_determination_text, char_for_brace);
        gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));
    }
<overcurlyaccentusbrace>[^\]]+ {
        gregorio_snprintf(tempstr, sizeof tempstr,
                "\\GreOverCurlyBrace{%s}{0pt}{0pt}{%d}{1}",
                gabc_notes_determination_text, char_for_brace);
        gregorio_add_texverb_to_note(current_note, gregorio_strdup(tempstr));
    }
<choralsign>[^\]]+ {
        gregorio_add_cs_to_note(&current_note,
                gregorio_strdup(gabc_notes_determination_text), false);
    }
<choralnabc>[^\]]+ {
        gregorio_add_cs_to_note(&current_note,
                gregorio_strdup(gabc_notes_determination_text), true);
    }
<texverbnote>[^\]]+ {
        gregorio_add_texverb_to_note(current_note,
                gregorio_strdup(gabc_notes_determination_text));
    }
<texverbglyph>[^\]]+ {
        gregorio_add_texverb_as_note(&current_note,
                gregorio_strdup(gabc_notes_determination_text),
                GRE_TEXVERB_GLYPH, &notes_lloc);
    }
<texverbelement>[^\]]+ {
        gregorio_add_texverb_as_note(&current_note,
                gregorio_strdup(gabc_notes_determination_text),
                GRE_TEXVERB_ELEMENT, &notes_lloc);
    }
<alt>[^\]]+ {
        gregorio_add_texverb_as_note(&current_note,
                gregorio_strdup(gabc_notes_determination_text), GRE_ALT,
                &notes_lloc);
    }
<INITIAL>\/\[ {
        BEGIN(space);
    }
<space>-?(\.[0-9]+|[0-9]+(\.[0-9]*)?)/\] {
        gregorio_add_space_as_note(&current_note, SP_AD_HOC_SPACE,
                gregorio_strdup(gabc_notes_determination_text), &notes_lloc);
    }
<INITIAL>!\/\[ {
        BEGIN(nbspace);
    }
<nbspace>-?(\.[0-9]+|[0-9]+(\.[0-9]*)?)/\] {
        gregorio_add_space_as_note(&current_note, SP_AD_HOC_SPACE_NB,
                gregorio_strdup(gabc_notes_determination_text), &notes_lloc);
    }
<INITIAL>\[oll:1\] {
        LEDGER(high, EXPLICITLY_DRAWN, true);
    }
<INITIAL>\[oll:0\] {
        LEDGER(high, EXPLICITLY_DRAWN, false);
    }
<INITIAL>\[oll:\}\] {
        end_variable_ledger(SO_OVER);
    }
<INITIAL>\[oll: {
        BEGIN(overledger);
    }
<overledger>[^;{]+ {
        save_before_ledger(gabc_notes_determination_text);
        BEGIN(overledger2);
    }
<overledger2>;[^\]]+ {
        add_static_ledger(SO_OVER, gabc_notes_determination_text);
        BEGIN(endledger);
    }
<overledger2>\{[^\]]+ {
        add_variable_ledger(SO_OVER, gabc_notes_determination_text);
        BEGIN(endledger);
    }
<INITIAL>\[ull:1\] {
        LEDGER(low, EXPLICITLY_DRAWN, true);
    }
<INITIAL>\[ull:0\] {
        LEDGER(low, EXPLICITLY_DRAWN, false);
    }
<INITIAL>\[ull:\}\] {
        end_variable_ledger(SO_UNDER);
    }
<INITIAL>\[ull: {
        BEGIN(underledger);
    }
<underledger>[^;{]+ {
        save_before_ledger(gabc_notes_determination_text);
        BEGIN(underledger2);
    }
<underledger2>;[^\]]+ {
        add_static_ledger(SO_UNDER, gabc_notes_determination_text);
        BEGIN(endledger);
    }
<underledger2>\{[^\]]+ {
        add_variable_ledger(SO_UNDER, gabc_notes_determination_text);
        BEGIN(endledger);
    }
<texverbnote,texverbglyph,texverbelement,choralsign,choralnabc,alt,overcurlyaccentusbrace,overcurlybrace,overbrace,underbrace,space,nbspace,endledger>\] {
        BEGIN(INITIAL);
    }
<INITIAL>\[[ou]slur:[012];[^,]+,[^\]]+\] {
        parse_slur();
    }
<INITIAL>\[[ou]slur:[012]\{\] {
        start_var_slur();
    }
<INITIAL>\[[ou]slur:[012]\}\] {
        end_var_slur();
    }
<INITIAL>\[[ou]h(?x:
  :(m|[ou]?[lh])\{?
| :(m|[ou]?[lh])?[+-][^\]\{]+\{?
| :?\{
)\] {
        parse_hepisema_adjustment();
    }
<INITIAL>\[[ou]h:?\}\] {
        end_hepisema_adjustment();
    }
<INITIAL>\[\[ {
        left_bracket();
    }
<INITIAL>\]\] {
        right_bracket();
    }
\{  {
        gregorio_add_texverb_as_note(&current_note,
                gregorio_strdup("\\hbox to 0pt{"), GRE_TEXVERB_ELEMENT,
                &notes_lloc);
    }
\}  {
        gregorio_add_texverb_as_note(&current_note,
                gregorio_strdup("\\hss%\n}%\n\\GreNoBreak\\relax "),
                GRE_TEXVERB_ELEMENT, &notes_lloc);
    }
[a-np]\+ {
        gregorio_add_manual_custos_as_note(&current_note,
                pitch_letter_to_height(gabc_notes_determination_text[0]),
                &notes_lloc);
    }
[\t\r\n]+ /* ignore ends of line and tabs */;
<INITIAL>\[nocustos\] {
        gregorio_add_suppress_custos_as_note(&current_note, &notes_lloc);
    }
z0  {
        gregorio_add_custos_as_note(&current_note, &notes_lloc);
    }
z   {
        gregorio_add_end_of_line_as_note(&current_note, false, false, false,
                &notes_lloc);
    }
z\+ {
        gregorio_add_end_of_line_as_note(&current_note, false, true, true,
                &notes_lloc);
    }
z-  {
        gregorio_add_end_of_line_as_note(&current_note, false, true, false,
                &notes_lloc);
    }
Z   {
        gregorio_add_end_of_line_as_note(&current_note, true, false, false,
                &notes_lloc);
    }
Z\+ {
        gregorio_add_end_of_line_as_note(&current_note, true, true, true,
                &notes_lloc);
    }
Z-  {
        gregorio_add_end_of_line_as_note(&current_note, true, true, false,
                &notes_lloc);
    }
[cf][1-5] {
        gregorio_add_clef_as_note(&current_note,
                letter_to_clef(gabc_notes_determination_text[0]),
                parse_clef_line(gabc_notes_determination_text[1]), false,
                &notes_lloc);
    }
[cf]b[1-5] {
        gregorio_add_clef_as_note(&current_note,
                letter_to_clef(gabc_notes_determination_text[0]),
                parse_clef_line(gabc_notes_determination_text[2]), true,
                &notes_lloc);
    }
@[cf][1-5] {
        gregorio_add_secondary_clef_to_note(current_note,
                letter_to_clef(gabc_notes_determination_text[1]),
                parse_clef_line(gabc_notes_determination_text[2]), false);
    }
@[cf]b[1-5] {
        gregorio_add_secondary_clef_to_note(current_note,
                letter_to_clef(gabc_notes_determination_text[1]),
                parse_clef_line(gabc_notes_determination_text[3]), true);
    }
`   {
        add_bar_as_note(B_VIRGULA);
    }
`0  {
        add_bar_as_note(B_VIRGULA_HIGH);
    }
\^  {
        add_bar_as_note(B_DIVISIO_MINIMIS);
    }
\^0 {
        add_bar_as_note(B_DIVISIO_MINIMIS_HIGH);
    }
,   {
        add_bar_as_note(B_DIVISIO_MINIMA);
    }
,0  {
        add_bar_as_note(B_DIVISIO_MINIMA_HIGH);
    }
[,;][1-8] {
        add_bar_as_note(parse_dominican_bar(gabc_notes_determination_text[1]));
    }
;   {
        add_bar_as_note(B_DIVISIO_MINOR);
    }
:   {
        add_bar_as_note(B_DIVISIO_MAIOR);
    }
::  {
        add_bar_as_note(B_DIVISIO_FINALIS);
    }
:\? {
        add_bar_as_note(B_DIVISIO_MAIOR_DOTTED);
    }
r   {
        gregorio_add_cavum(current_note);
    }
R   {
        gregorio_change_shape(current_note, S_LINEA_PUNCTUM,
                legacy_oriscus_orientation);
    }
r0  {
        gregorio_change_shape(current_note, S_LINEA_PUNCTUM,
                legacy_oriscus_orientation);
        gregorio_add_cavum(current_note);
    }
r1  {
        gregorio_add_special_sign(current_note, _ACCENTUS);
    }
r2  {
        gregorio_add_special_sign(current_note, _ACCENTUS_REVERSUS);
    }
r3  {
        gregorio_add_special_sign(current_note, _CIRCULUS);
    }
r4  {
        gregorio_add_special_sign(current_note, _SEMI_CIRCULUS);
    }
r5  {
        gregorio_add_special_sign(current_note, _SEMI_CIRCULUS_REVERSUS);
    }
r6  {
        gregorio_add_special_sign(current_note, _MUSICA_FICTA_FLAT);
    }
r7  {
        gregorio_add_special_sign(current_note, _MUSICA_FICTA_NATURAL);
    }
r8  {
        gregorio_add_special_sign(current_note, _MUSICA_FICTA_SHARP);
    }
x   {
        gregorio_change_shape(current_note, S_FLAT, legacy_oriscus_orientation);
    }
#   {
        gregorio_change_shape(current_note, S_SHARP,
                legacy_oriscus_orientation);
    }
y   {
        gregorio_change_shape(current_note, S_NATURAL,
                legacy_oriscus_orientation);
    }
!?\/0 {
        gregorio_add_space_as_note(&current_note, SP_HALF_SPACE, NULL,
                &notes_lloc);
    }
!?\/! {
        gregorio_add_space_as_note(&current_note, SP_INTERGLYPH_SPACE, NULL,
                &notes_lloc);
    }
\/  {
        gregorio_add_space_as_note(&current_note, SP_NEUMATIC_CUT, NULL,
                &notes_lloc);
    }
\//\/\[ {
        gregorio_add_space_as_note(&current_note, SP_NEUMATIC_CUT, NULL,
                &notes_lloc);
    }
\/\/ {
        gregorio_add_space_as_note(&current_note, SP_LARGER_SPACE, NULL,
                &notes_lloc);
    }
\   {
        gregorio_add_space_as_note(&current_note, SP_GLYPH_SPACE, NULL,
                &notes_lloc);
    }
!\/ {
        gregorio_add_space_as_note(&current_note, SP_NEUMATIC_CUT_NB, NULL,
                &notes_lloc);
    }
!\//\/\[ {
        gregorio_add_space_as_note(&current_note, SP_NEUMATIC_CUT_NB, NULL,
                &notes_lloc);
    }
!\/\/ {
        gregorio_add_space_as_note(&current_note, SP_LARGER_SPACE_NB, NULL,
                &notes_lloc);
    }
!\  {
        gregorio_add_space_as_note(&current_note, SP_GLYPH_SPACE_NB, NULL,
                &notes_lloc);
    }
!/[^\/ ] {
        gregorio_add_space_as_note(&current_note, SP_ZERO_WIDTH, NULL,
                &notes_lloc);
    }
=   {
        gregorio_change_shape(current_note, S_LINEA,
                legacy_oriscus_orientation);
    }
[a-npA-NP]vv {
        lex_add_note(0, S_BIVIRGA, _NO_SIGN, L_NO_LIQUESCENTIA);
    }
[a-npA-NP]vvv {
        lex_add_note(0, S_TRIVIRGA, _NO_SIGN, L_NO_LIQUESCENTIA);
    }
[a-npA-NP]VV {
        lex_add_note(0, S_BIVIRGA, _NO_SIGN, L_NO_LIQUESCENTIA);
    }
[a-npA-NP]VVV {
        lex_add_note(0, S_TRIVIRGA, _NO_SIGN, L_NO_LIQUESCENTIA);
    }
[a-npA-NP]ss {
        lex_add_note(0, S_DISTROPHA, _NO_SIGN, L_NO_LIQUESCENTIA);
    }
[a-npA-NP]ss(\<|\>) {
        lex_add_note(0, S_DISTROPHA, _NO_SIGN, L_AUCTUS_ASCENDENS);
    }
[a-npA-NP]sss {
        lex_add_note(0, S_TRISTROPHA, _NO_SIGN, L_NO_LIQUESCENTIA);
    }
[a-npA-NP]sss(\<|\>) {
        lex_add_note(0, S_TRISTROPHA, _NO_SIGN, L_AUCTUS_ASCENDENS);
    }
[a-np] {
        lex_add_note(0, S_PUNCTUM, _NO_SIGN, L_NO_LIQUESCENTIA);
    }
-[a-np] {
        lex_add_note(1, S_PUNCTUM, _NO_SIGN, L_INITIO_DEBILIS);
    }
@[a-np] {
        lex_add_note(1, S_PUNCTUM, _NO_SIGN, L_FUSED);
    }
[A-NP][012]? {
        lex_add_note(0, punctum_inclinatum(gabc_notes_determination_text[1]),
                _NO_SIGN, L_NO_LIQUESCENTIA);
    }
-[A-NP][012]? {
        lex_add_note(1, punctum_inclinatum(gabc_notes_determination_text[2]),
                _NO_SIGN, L_INITIO_DEBILIS);
    }
@[A-NP][012]? {
        lex_add_note(1, punctum_inclinatum(gabc_notes_determination_text[2]),
                _NO_SIGN, L_FUSED);
    }
\'[01]? {
        add_sign(_V_EPISEMA);
    }
_[0-5]* {
        add_h_episema();
    }
\.[01]? {
        add_sign(_PUNCTUM_MORA);
    }
~   {
        gregorio_add_tail_liquescentia(current_note, L_DEMINUTUS,
                legacy_oriscus_orientation);
    }
>   {
        gregorio_add_tail_liquescentia(current_note, L_AUCTUS_DESCENDENS,
                legacy_oriscus_orientation);
    }
\<  {
        gregorio_add_tail_liquescentia(current_note, L_AUCTUS_ASCENDENS,
                legacy_oriscus_orientation);
    }
q   {
        gregorio_change_shape(current_note, S_QUADRATUM,
                legacy_oriscus_orientation);
    }
o   {
        gregorio_change_shape(current_note, S_ORISCUS_UNDETERMINED,
                legacy_oriscus_orientation);
    }
o0  {
        gregorio_change_shape(current_note, S_ORISCUS_DESCENDENS,
                legacy_oriscus_orientation);
    }
o1  {
        gregorio_change_shape(current_note, S_ORISCUS_ASCENDENS,
                legacy_oriscus_orientation);
    }
O   {
        gregorio_change_shape(current_note, S_ORISCUS_SCAPUS_UNDETERMINED,
                legacy_oriscus_orientation);
    }
O0  {
        gregorio_change_shape(current_note, S_ORISCUS_SCAPUS_DESCENDENS,
                legacy_oriscus_orientation);
    }
O1  {
        gregorio_change_shape(current_note, S_ORISCUS_SCAPUS_ASCENDENS,
                legacy_oriscus_orientation);
    }
w   {
        gregorio_change_shape(current_note, S_QUILISMA,
                legacy_oriscus_orientation);
    }
W   {
        gregorio_change_shape(current_note, S_QUILISMA_QUADRATUM,
                legacy_oriscus_orientation);
    }
v   {
        gregorio_change_shape(current_note, S_VIRGA,
                legacy_oriscus_orientation);
    }
V   {
        gregorio_change_shape(current_note, S_VIRGA_REVERSA,
                legacy_oriscus_orientation);
    }
s   {
        gregorio_change_shape(current_note, S_STROPHA,
                legacy_oriscus_orientation);
    }
\[hl:1\] {
        LEDGER(high, EXPLICIT, true);
    }
\[hl:0\] {
        LEDGER(high, EXPLICIT, false);
    }
\[ll:1\] {
        LEDGER(low, EXPLICIT, true);
    }
\[ll:0\] {
        LEDGER(low, EXPLICIT, false);
    }
.|\n {
        gregorio_messagef("det_notes_from_string", VERBOSITY_ERROR, 0,
                _("unrecognized character: \"%c\""),
                gabc_notes_determination_text[0]);
    }

%%

gregorio_note *gabc_det_notes_from_string(char *str, char *newmacros[10],
        gregorio_scanner_location *loc, const gregorio_score *const score)
{
    int i;
    YY_BUFFER_STATE buf;

    notes_lloc.first_line = loc->first_line;
    notes_lloc.first_column = loc->first_column;
    notes_lloc.first_offset = loc->first_offset;
    /* yes... I do mean to set values from loc->first_* */
    notes_lloc.last_line = loc->first_line;
    notes_lloc.last_column = loc->first_column;
    notes_lloc.last_offset = loc->first_offset;
    notes_lloc.generate_point_and_click = loc->generate_point_and_click;

    staff_lines = score->staff_lines;
    highest_pitch = score->highest_pitch;
    high_ledger_line_pitch = score->high_ledger_line_pitch;
    legacy_oriscus_orientation = score->legacy_oriscus_orientation;

    /* a small optimization could uccur here: we could do it only once at the
     * beginning of the score, not at each syllable */
    for (i = 0; i < 10; i++) {
        notesmacros[i] = newmacros[i];
    }
    nbof_isolated_episema = 0;
    current_note = NULL;
    buf = yy_scan_string(str);
    yylex();
    yy_flush_buffer(buf);
    yy_delete_buffer(buf);
    gregorio_go_to_first_note(&current_note);
    return current_note;
}
