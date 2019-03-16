/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This header defines the Gregorio data structures and functions.
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
 * The file starts by the definition of all the structures used in
 * gregorio. As it is certainly the most important file for
 * understanding, read it carefully.
 */

#include "unicode.h"

#ifndef STRUCT_H
#define STRUCT_H

#include <assert.h>
#include "enum_generator.h"
#include "bool.h"
#include "sha1.h"
#include "messages.h"

#ifdef __cplusplus
#define ENUM_BITFIELD(TYPE) enum TYPE
#elif defined(__GNUC__) && __GNUC__ >= 2
#define ENUM_BITFIELD(TYPE) __extension__ enum TYPE
#else
#define ENUM_BITFIELD(TYPE) unsigned int
#endif

/* Lilypond-compatible location counters */
typedef struct gregorio_scanner_location {
    /* - line is the 1-based line number
     * - column is the 0-based* column number, tabs expanded to 8-column stops
     * - offset is the 0-based number of characters from the start of the line
     *
     * * = column is calculated and stored as a 0-based number so the tabstop
     * math is simpler; it should be printed 1-based */

    unsigned short first_line;
    unsigned short first_column;
    unsigned short first_offset;
    unsigned short last_line;
    unsigned short last_column;
    unsigned short last_offset;
    bool generate_point_and_click:1;
} gregorio_scanner_location;

/* all the different types of things a gregorio_* can be */

#define GREGORIO_TYPE(A,E,X,L) \
    A(GRE_NOTE, 1) \
    E(GRE_GLYPH) \
    E(GRE_ELEMENT) \
    E(GRE_CLEF) \
    E(GRE_END_OF_LINE) \
    E(GRE_SPACE) \
    E(GRE_BAR) \
    E(GRE_CUSTOS) \
    E(GRE_MANUAL_CUSTOS) \
    E(GRE_SUPPRESS_CUSTOS) \
    /* I don't really know how I could use the a TEXVERB_NOTE in gregoriotex,
     * as we don't write note by note... */ \
    /* GRE_TEXVERB_NOTE, */ \
    E(GRE_TEXVERB_GLYPH) \
    E(GRE_TEXVERB_ELEMENT) \
    /* above lines text, quite the same as GRE_TEXVERB_ELEMENT, but counted
     * differently for the spaces above the lines */ \
    E(GRE_ALT) \
    E(GRE_NLBA) \
    E(GRE_AUTOFUSE_START) \
    L(GRE_AUTOFUSE_END)
ENUM(gregorio_type, GREGORIO_TYPE);

#define GREGORIO_CLEF(A,E,X,L) \
    A(CLEF_C, 0) \
    X(CLEF_F, 1)
ENUM(gregorio_clef, GREGORIO_CLEF);

/* the different shapes, only for notes */

#define GREGORIO_SHAPE(A,E,X,L) \
    A(S_UNDETERMINED, 0) \
    E(S_PUNCTUM) \
    E(S_PUNCTUM_END_OF_GLYPH) \
    E(S_PUNCTUM_INCLINATUM_ASCENDENS) \
    E(S_PUNCTUM_INCLINATUM_STANS) \
    E(S_PUNCTUM_INCLINATUM_DESCENDENS) \
    E(S_PUNCTUM_INCLINATUM_DEMINUTUS) \
    E(S_PUNCTUM_INCLINATUM_AUCTUS) \
    E(S_VIRGA) \
    E(S_VIRGA_REVERSA) \
    E(S_BIVIRGA) \
    E(S_TRIVIRGA) \
    E(S_ORISCUS_ASCENDENS) \
    E(S_ORISCUS_DESCENDENS) \
    E(S_ORISCUS_DEMINUTUS) \
    E(S_ORISCUS_SCAPUS_ASCENDENS) \
    E(S_ORISCUS_SCAPUS_DESCENDENS) \
    E(S_QUILISMA) \
    E(S_STROPHA) \
    E(S_STROPHA_AUCTA) \
    E(S_DISTROPHA) \
    E(S_TRISTROPHA) \
    E(S_LINEA_PUNCTUM) \
    E(S_FLAT) \
    E(S_SHARP) \
    E(S_NATURAL) \
    /* special shapes that must not appear in the final form of the score :
     * quadratum is the shape of the first note of a punctum quadratum
     * and quilisma quadratum is the shape of the first note of a pes
     * quislisma quadratum */ \
    E(S_ORISCUS_UNDETERMINED) \
    E(S_ORISCUS_SCAPUS_UNDETERMINED) \
    E(S_QUADRATUM) \
    E(S_PUNCTUM_INCLINATUM_UNDETERMINED) \
    /* those shapes are for now used only in gregoriotex */ \
    E(S_QUILISMA_QUADRATUM) \
    E(S_PUNCTUM_AUCTUS_ASCENDENS) \
    E(S_PUNCTUM_AUCTUS_DESCENDENS) \
    E(S_PUNCTUM_DEMINUTUS) \
    L(S_LINEA)
ENUM(gregorio_shape, GREGORIO_SHAPE);

/* the different kind of bars */

#define GREGORIO_BAR(A,E,X,L) \
    A(B_NO_BAR, 0) \
    E(B_VIRGULA) \
    E(B_DIVISIO_MINIMA) \
    E(B_DIVISIO_MINOR) \
    E(B_DIVISIO_MAIOR) \
    E(B_DIVISIO_FINALIS) \
    E(B_DIVISIO_MINOR_D1) \
    E(B_DIVISIO_MINOR_D2) \
    E(B_DIVISIO_MINOR_D3) \
    E(B_DIVISIO_MINOR_D4) \
    E(B_DIVISIO_MINOR_D5) \
    E(B_DIVISIO_MINOR_D6) \
    E(B_DIVISIO_MINOR_D7) \
    E(B_DIVISIO_MINOR_D8) \
    E(B_VIRGULA_HIGH) \
    E(B_DIVISIO_MINIMA_HIGH) \
    E(B_DIVISIO_MAIOR_DOTTED) \
    E(B_DIVISIO_MINIMIS) \
    L(B_DIVISIO_MINIMIS_HIGH)
ENUM(gregorio_bar, GREGORIO_BAR);

/* definition of the signs. You can notice that the values are made so
 * that if you wan to add a vertical episema to a note, you juste
 * make note->signs+=_V_EPISEMA, so please don't change the value as
 * this trick is used. */

#define GREGORIO_SIGN(A,E,X,L) \
    A(_NO_SIGN, 0x00) \
    A(_PUNCTUM_MORA, 0x01) \
    A(_AUCTUM_DUPLEX, 0x02) \
    A(_V_EPISEMA, 0x10) \
    A(_V_EPISEMA_PUNCTUM_MORA, 0x11) \
    A(_V_EPISEMA_AUCTUM_DUPLEX, 0x12) \
    /* more rare signs, for now they can't be used with the others */ \
    A(_ACCENTUS, 0x03) \
    A(_ACCENTUS_REVERSUS, 0x04) \
    A(_CIRCULUS, 0x05) \
    A(_SEMI_CIRCULUS, 0x06) \
    A(_SEMI_CIRCULUS_REVERSUS, 0x07) \
    /* signs of a bar */ \
    A(_BAR_H_EPISEMA, 0x08) \
    A(_V_EPISEMA_BAR_H_EPISEMA, 0x18) \
    /* musica ficta */ \
    A(_MUSICA_FICTA_FLAT, 0x09) \
    A(_MUSICA_FICTA_NATURAL, 0x0a) \
    X(_MUSICA_FICTA_SHARP, 0x0b)
ENUM(gregorio_sign, GREGORIO_SIGN);

/* the different spaces */

#define GREGORIO_SPACE(A,E,X,L) \
    A(SP_DEFAULT, 1) \
    E(SP_NO_SPACE) \
    E(SP_ZERO_WIDTH) \
    E(SP_HALF_SPACE) \
    E(SP_INTERGLYPH_SPACE) \
    E(SP_NEUMATIC_CUT) \
    E(SP_LARGER_SPACE) \
    E(SP_GLYPH_SPACE) \
    E(SP_AD_HOC_SPACE) \
    E(SP_NEUMATIC_CUT_NB) \
    E(SP_LARGER_SPACE_NB) \
    E(SP_GLYPH_SPACE_NB) \
    L(SP_AD_HOC_SPACE_NB)
ENUM(gregorio_space, GREGORIO_SPACE);

/* the different liquescences, like for the signs, have special
 * values: to say that something is initio_debilis, just do
 * glyph->liquescentia+=L_INITIO_DEBILIS. So don't change the value,
 * the trick is much used */

#define TAIL_LIQUESCENTIA_MASK 0x07

#define GREGORIO_LIQUESCENTIA(A,E,X,L) \
    A(L_NO_LIQUESCENTIA, 0x00) \
    A(L_DEMINUTUS, 0x01) \
    A(L_AUCTUS_ASCENDENS, 0x02) \
    A(L_AUCTUS_DESCENDENS, 0x04) \
    A(L_INITIO_DEBILIS, 0x10) \
    A(L_DEMINUTUS_INITIO_DEBILIS, 0x11) \
    A(L_AUCTUS_ASCENDENS_INITIO_DEBILIS, 0x12) \
    A(L_AUCTUS_DESCENDENS_INITIO_DEBILIS, 0x14) \
    A(L_FUSED, 0x20) \
    A(L_FUSED_DEMINUTUS, 0x21) \
    A(L_FUSED_AUCTUS_ASCENDENS, 0x22) \
    X(L_FUSED_AUCTUS_DESCENDENS, 0x24)
ENUM(gregorio_liquescentia, GREGORIO_LIQUESCENTIA);

#define GREHEPISEMA_SIZE(A,E,X,L) \
    A(H_NORMAL, 0) \
    E(H_SMALL_LEFT) \
    E(H_SMALL_CENTRE) \
    L(H_SMALL_RIGHT)
ENUM(grehepisema_size, GREHEPISEMA_SIZE);

/* values are chosen so BELOW/ABOVE can be added to a pitch */
#define GREGORIO_VPOSITION(A,E,X,L) \
    A(VPOS_AUTO, 0) \
    A(VPOS_BELOW, -1) \
    X(VPOS_ABOVE, 1)
ENUM(gregorio_vposition, GREGORIO_VPOSITION);

/* The different types of glyph */

#define GREGORIO_GLYPH_TYPE(A,E,X,L) \
    A(G_PUNCTUM_INCLINATUM, 1) \
    E(G_2_PUNCTA_INCLINATA_DESCENDENS) \
    E(G_3_PUNCTA_INCLINATA_DESCENDENS) \
    E(G_4_PUNCTA_INCLINATA_DESCENDENS) \
    E(G_5_PUNCTA_INCLINATA_DESCENDENS) \
    E(G_2_PUNCTA_INCLINATA_ASCENDENS) \
    E(G_3_PUNCTA_INCLINATA_ASCENDENS) \
    E(G_4_PUNCTA_INCLINATA_ASCENDENS) \
    E(G_5_PUNCTA_INCLINATA_ASCENDENS) \
    E(G_PUNCTA_INCLINATA) \
    /* !!! DO NOT CHANGE THE ENUM ORDERING BEFORE THIS LINE !!! */ \
    E(G_UNDETERMINED) \
    E(G_VIRGA) \
    E(G_STROPHA) \
    E(G_STROPHA_AUCTA) \
    E(G_PUNCTUM) \
    E(G_ALTERATION) \
    E(G_PODATUS) \
    E(G_PES_QUADRATUM) \
    E(G_FLEXA) \
    E(G_TORCULUS) \
    E(G_TORCULUS_RESUPINUS) \
    E(G_TORCULUS_RESUPINUS_FLEXUS) \
    E(G_PORRECTUS) \
    E(G_PORRECTUS_FLEXUS) \
    E(G_BIVIRGA) \
    E(G_TRIVIRGA) \
    E(G_DISTROPHA) \
    E(G_DISTROPHA_AUCTA) \
    E(G_TRISTROPHA) \
    E(G_TRISTROPHA_AUCTA) \
    E(G_PES_QUADRATUM_FIRST_PART) \
    E(G_SCANDICUS) \
    E(G_PES_QUILISMA_QUADRATUM_FIRST_PART) \
    E(G_ANCUS) \
    E(G_PUNCTA_ASCENDENS) \
    E(G_PUNCTA_DESCENDENS) \
    E(G_VIRGA_REVERSA) \
    E(G_SALICUS) \
    E(G_SALICUS_FLEXUS) \
    E(G_PES_ASCENDENS_ORISCUS) \
    E(G_PES_DESCENDENS_ORISCUS) \
    E(G_TORCULUS_LIQUESCENS) \
    /* additional glyph types, necessary for determination */ \
    E(G_PORRECTUS_NO_BAR) \
    E(G_PORRECTUS_FLEXUS_NO_BAR) \
    L(G_FUSED)
ENUM(gregorio_glyph_type, GREGORIO_GLYPH_TYPE);

/*
 *
 * The we define the characters. First we define the different styles. You
 * can notice that center is just a style like the others
 *
 */

#define GRESTYLE_STYLE(A,E,X,L) \
    A(ST_NO_STYLE, 0) \
    E(ST_ITALIC) \
    E(ST_CENTER) \
    /* when the user types a {}, basically the same behaviour, except for
     * the initial */ \
    E(ST_FORCED_CENTER) \
    E(ST_BOLD) \
    E(ST_TT) \
    E(ST_SMALL_CAPS) \
    E(ST_SPECIAL_CHAR) \
    E(ST_VERBATIM) \
    E(ST_INITIAL) /* a style used to determine the initial */ \
    E(ST_UNDERLINED) \
    E(ST_COLORED) \
    E(ST_ELISION) \
    E(ST_FIRST_WORD) \
    E(ST_FIRST_SYLLABLE) \
    E(ST_FIRST_SYLLABLE_INITIAL) \
    E(ST_SYLLABLE_INITIAL) \
    /* protrusion is implemented a PROTRUSION_FACTOR followed by a PROTRUSION;
     * an automatic protrusion will have a verbatim TeX macro as factor */ \
    E(ST_PROTRUSION_FACTOR) \
    E(ST_PROTRUSION) \
    L(ST_SENTINEL) /* a temporary style to signify the end of a syllable */
ENUM(grestyle_style, GRESTYLE_STYLE);

/*
 * Then the different types of styles. See the next comments for further
 * readings.
 */

#define GRESTYLE_TYPE(A,E,X,L) \
    A(ST_T_NOTHING, 0) \
    E(ST_T_BEGIN) \
    L(ST_T_END)
ENUM(grestyle_type, GRESTYLE_TYPE);

/*
 * The different types of translation centerings
 */

#define GREGORIO_TR_CENTERING(A,E,X,L) \
    A(TR_NORMAL, 0) \
    E(TR_WITH_CENTER_BEGINNING) \
    L(TR_WITH_CENTER_END)
ENUM(gregorio_tr_centering, GREGORIO_TR_CENTERING);

/*
 * Nothing, beginning or end of area without linebreak
 */

#define GREGORIO_NLBA(A,E,X,L) \
    A(NLBA_NORMAL, 0) \
    E(NLBA_BEGINNING) \
    L(NLBA_END)
ENUM(gregorio_nlba, GREGORIO_NLBA);

#define GREGORIO_EUOUAE(A,E,X,L) \
    A(EUOUAE_NORMAL, 0) \
    E(EUOUAE_BEGINNING) \
    L(EUOUAE_END)
ENUM(gregorio_euouae, GREGORIO_EUOUAE);

#define GREGORIO_WORD_POSITION(A,E,X,L) \
    A(WORD_BEGINNING, 1) \
    E(WORD_MIDDLE) \
    E(WORD_END) \
    L(WORD_ONE_SYLLABLE)
ENUM(gregorio_word_position, GREGORIO_WORD_POSITION);

#define GREGORIO_SIGN_ORIENTATION(A,E,X,L) \
    A(SO_OVER, 0) \
    X(SO_UNDER, 1)
ENUM(gregorio_sign_orientation, GREGORIO_SIGN_ORIENTATION);

/* the numeric values will be directly passed to TeX */
#define GREGORIO_HEPISEMA_VBASEPOS(A,E,X,L) \
    A(HVB_AUTO, 0) \
    A(HVB_MIDDLE, 1) \
    A(HVB_O_LOW, 2) \
    A(HVB_O_HIGH, 3) \
    A(HVB_U_LOW, 4) \
    X(HVB_U_HIGH, 5)
ENUM(gregorio_hepisema_vbasepos, GREGORIO_HEPISEMA_VBASEPOS);

/* these may be used as bit fields, with LEDGER_EXPLICIT and LEDGER_DRAWN being
 * the bits */
#define GREGORIO_LEDGER_SPECIFICITY(A,E,X,L) \
    A(LEDGER_SUPPOSED, 0) \
    A(LEDGER_EXPLICIT, 1) \
    A(LEDGER_DRAWN, 2) \
    X(LEDGER_EXPLICITLY_DRAWN, 3)
ENUM(gregorio_ledger_specificity, GREGORIO_LEDGER_SPECIFICITY);

typedef struct gregorio_extra_info {
    char *ad_hoc_space_factor;
    ENUM_BITFIELD(gregorio_bar) bar:5;
    ENUM_BITFIELD(gregorio_space) space:4;
    ENUM_BITFIELD(gregorio_nlba) nlba:2;
    bool eol_ragged:1;
    bool eol_forces_custos:1;
    bool eol_forces_custos_on:1;
} gregorio_extra_info;

typedef struct gregorio_clef_info {
    signed char line;
    signed char secondary_line;
    signed char pitch_difference;
    ENUM_BITFIELD(gregorio_clef) clef:1;
    bool flatted:1;
    ENUM_BITFIELD(gregorio_clef) secondary_clef:1;
    bool secondary_flatted:1;
} gregorio_clef_info;

typedef union gregorio_misc_element_info {
    /* pitched is used for GRE_CUSTOS */
    struct {
        /* The pitch of the glyph. */
        signed char pitch;
        /* boolean indicating whether the pitch is forced */
        bool force_pitch:1;
    } pitched;
    /* clef is used for GRE_CLEF */
    struct gregorio_clef_info clef;
    /* unpitched is used for everything else */
    struct {
        struct gregorio_extra_info info;
        /* an element might carry a sign. */
        ENUM_BITFIELD(gregorio_sign) special_sign:8;
    } unpitched;
} gregorio_misc_element_info;

/*
 * ! We start with the most precise structure, the note structure. The
 * note is always a real note (we'll see that glyphs and elements can be
 * other things).
 */
typedef struct gregorio_note {
    /* then two pointer to other notes, to make a chained list. */
    struct gregorio_note *previous;
    struct gregorio_note *next;
    /* choral sign is a letter that appears next to a note in some choral
     * scores we put it as char* because sometimes two letters appear */
    char *choral_sign;
    union {
        /* note is used for GRE_NOTE */
        struct {
            /* the pitch is the height of the note on the score, that is to
             * say the letter it is represented by in gabc.  If a clef
             * change, pitch will be a number indicating the line of the
             * clef. */
            signed char pitch;
            /* shape is the shape of the note... if you want to know the
             * different possible shapes, see above. */
            ENUM_BITFIELD(gregorio_shape) shape:8;
            /* liquescentia is the liquescence on the note, it is not really
             * used in the final score, but it is, like type, used in the
             * determination of glyphs. */
            ENUM_BITFIELD(gregorio_liquescentia) liquescentia:7;
            bool is_cavum:1;
        } note;
        /* clef is used for GRE_CLEF */
        struct gregorio_clef_info clef;
        /* other is used for everything else */
        struct gregorio_extra_info other;
    } u;

    /* these go to the end for structure alignment */
    /* index to a string containing a possible TeX verbatim; necessary during
     * structure generation. */
    unsigned short texverb;
    unsigned short src_line, src_column, src_offset;
    unsigned short he_adjustment_index[2];

    /* we have seen that notes are always real notes, that is to say
     * GRE_NOTE. the type is always that in the final structure. But there
     * is however this field in the structure because of the temporary
     * states that can appear in the determination, where notes can be
     * other things. This is the case for example in gabc reading. */
    ENUM_BITFIELD(gregorio_type) type:8;
    /* signs is the signs on the notes, see above for all possible values */
    ENUM_BITFIELD(gregorio_sign) signs:8;
    /* special_sign is the sign we sometimes encounter on punctum cavum, like
     * accentus, semi-circulus, etc. */
    ENUM_BITFIELD(gregorio_sign) special_sign:8;
    /* h_episema_type is the type of horizontal episema, possible values
     * are H_ALONE for an isolated horizontal episema, H_MULTI_BEGINNING
     * if the note is the first note of an episema on several notes,
     * H_MULTI_MIDDLE if it is inside an episema on several notes. I let
     * you guess what could be the use of H_MULTI_END. Other values are
     * temporary values used in determination, they must not appear in the
     * final structure. */

    const char *gtex_offset_case;
    signed char v_episema_height;
    signed char h_episema_above;
    signed char h_episema_below;
    ENUM_BITFIELD(grehepisema_size) h_episema_above_size:2;
    ENUM_BITFIELD(grehepisema_size) h_episema_below_size:2;
    bool h_episema_above_connect:1;
    bool h_episema_below_connect:1;
    /* the "specificity" field indicates how to interpret the line flag */
    bool high_ledger_line:1;
    ENUM_BITFIELD(gregorio_ledger_specificity) high_ledger_specificity:2;
    bool low_ledger_line:1;
    ENUM_BITFIELD(gregorio_ledger_specificity) low_ledger_specificity:2;
    bool is_lower_note:1;
    bool is_upper_note:1;
    ENUM_BITFIELD(gregorio_vposition) mora_vposition:2;
    bool choral_sign_is_nabc:1;
} gregorio_note;

/*
 * ! @brief The gregorio glyph structure Unlike gregorio_note, gregorio_glyph
 * can be other things besides GRE_GLYPH: it can also be GRE_SPACE
 */
typedef struct gregorio_glyph {
    /* two pointer to make a chained list */
    struct gregorio_glyph *previous;
    struct gregorio_glyph *next;
    union {
        /* glyph is used for GRE_GLYPH */
        struct {
            /* a pointer to a (chained list of) gregorio_notes, the first of
             * the glyph. */
            struct gregorio_note *first_note;
            signed char fuse_to_next_glyph;
            /* The glyph type for a GRE_GLYPH (porrectus, pes, etc.).  They
             * are all listed above. */
            ENUM_BITFIELD(gregorio_glyph_type) glyph_type:8;
            /* liquescentia is really used, because that will determine the
             * shape we will have to use. */
            ENUM_BITFIELD(gregorio_liquescentia) liquescentia:7;
            bool is_cavum:1;
        } notes;
        union gregorio_misc_element_info misc;
    } u;

    /* index to a string containing a possible TeX verbatim; necessary during
     * structure generation. */
    unsigned short texverb;

    /* type can have the values explained in the comment just above. */
    ENUM_BITFIELD(gregorio_type) type:8;

    /* There is no additional parameter for GRE_SPACE in a gregorio_glyph
     * because gregorio_space in this case can have only one value:
     * SP_ZERO_WIDTH, as other spaces would break the glyphs into
     * different elements. */
} gregorio_glyph;

typedef struct gregorio_element {
    /* pointers to the next and previous elements. */
    struct gregorio_element *previous;
    struct gregorio_element *next;
    /* The nabc string */
    char **nabc;
    /* we put it here to get the length of the array here too */
    size_t nabc_lines;
    union {
        /* first_glyph is used for GRE_ELEMENT */
        /* a pointer to the first glyph of the element. */
        struct gregorio_glyph *first_glyph;
        union gregorio_misc_element_info misc;
    } u;

    /* index to a string containing a possible TeX verbatim; necessary during
     * structure generation. */
    unsigned short texverb;
    /* type can have the values GRE_ELEMENT, GRE_BAR, GRE_CLEF, GRE_CUSTOS,
     * GRE_END_OF_LINE, GRE_SPACE, GRE_TEXVERB_ELEMENT or GRE_NLBA */
    ENUM_BITFIELD(gregorio_type) type:8;
} gregorio_element;

/*
 *
 * gregorio_characters are a bit specials. As there can be styles in the
 * text, I had to find a structure mode adapted that just grewchar *. So
 * basically a gregorio_character is a double-chained list of things that
 * can be grewchar or gregorio_styles. For example if you type (in gabc)
 * p<i>o</i>t, the corresponding gregorio_character list will be
 * P->style(type: beginning, style italic) -> o -> style(type:end, style:
 * italic). But for this list to be coherent, it is mandatory that it is
 * xml-compliant, that is to say that a<b>c<i>d</b>e</i> will be
 * interpreted as a<b>c<i>d</i></b><i>e</i>. This MUST be done when reading
 * a file, so that the structure in memory is coherent. It makes input
 * modules more comple, but output modules muche more simpler. The last
 * particularity is that center must also be determined in the input
 * modules, so that it is already defined in memory. But it is a bit more
 * complex, because for TeX-like output modules, we need to close all
 * styles before the center style: if the user types <i>pot</i> it must be
 * represented as <i>p</i>{<i>o</i>}<i>t</i>.
 *
 * Here is the declaration of the gregorio_style struct. It is simply two
 * chars, one telling the type of style character it is (beginning for a
 * character that marks the beginning of a style, and end for a character
 * marking the end of a style). The other char simply is the style
 * represented by the character (italic, etc.)
 *
 */

typedef struct gregorio_style {
    ENUM_BITFIELD(grestyle_style) style:6;
    ENUM_BITFIELD(grestyle_type) type:2;
} gregorio_style;

/*
 *
 * This union is quite ugly... but necessary for a gregorio_character to be
 * able to be a grewchar or gregorio_style.
 *
 */
typedef union character_or_style {
    grewchar character;
    struct gregorio_style s;
} character_or_style;

/*
 *
 * Finally the gregorio_character structure in itself, It is first a char
 * determining the type (character or gregorio_style). This char is 0 when
 * it is a style and something else when it is a character. Then the two
 * pointers to build the double chained list, and finally the union. So
 * when you want to access to the style of a gregorio_character (when you
 * know it is a character of style), you must access to
 * mygregoriochar.cos.s.style, and for the character
 * mygregoriochar.cos.character .
 *
 */

typedef struct gregorio_character {
    bool is_character;
    struct gregorio_character *next_character;
    struct gregorio_character *previous_character;
    union character_or_style cos;
} gregorio_character;

typedef struct gregorio_syllable {
    /* pointer to a gregorio_text structure corresponding to the text. */
    struct gregorio_character *text;
    /* pointer to a gregorio_text structure corresponding to the
     * translation */
    struct gregorio_character *translation;
    /* a string representing the text above the lines (raw TeX) */
    char *abovelinestext;
    /* pointer to the next and previous syllable */
    struct gregorio_syllable *next_syllable;
    struct gregorio_syllable *previous_syllable;
    /* and finally a pointer to the elements of the structure. Here we see
     * that we point to an array of elements. In fact it is the array of
     * the first elements of the different voices of the syllable, for the
     * case of polyphonic score. In most scores (monophonic), the array
     * has only one element. */
    struct gregorio_element **elements;
    unsigned short euouae_id;
    unsigned short src_line, src_column, src_offset;
    /* type of translation (with center beginning or only center end) */
    ENUM_BITFIELD(gregorio_tr_centering) translation_type:2;
    /* beginning or end of area without linebreak? */
    ENUM_BITFIELD(gregorio_nlba) no_linebreak_area:2;
    /* beginning or end of euouae area */
    ENUM_BITFIELD(gregorio_euouae) euouae:2;
    /* position is WORD_BEGINNING for the beginning of a multi-syllable
     * word, WORD_ONE_SYLLABLE for syllable that are alone in their word,
     * and i let you gess what are WORD_MIDDLE and WORD_END. */
    ENUM_BITFIELD(gregorio_word_position) position:3;
    bool first_word:1;
    bool forced_center:1;
    bool clear:1;
} gregorio_syllable;

/* Stores a header in a singly-linked list */
typedef struct gregorio_header {
    char *name;
    char *value;
    struct gregorio_header *next;
} gregorio_header;

/*
 *
 * Score is the top structure, the structure in which we will convert
 * everything, and from which we will construct XML
 *
 */

#define MAX_ANNOTATIONS 2

typedef struct gregorio_score {
    unsigned char digest[SHA1_DIGEST_SIZE];
    /* the structure starts by a pointer to the first syllable of the
     * score. */
    struct gregorio_syllable *first_syllable;
    /* the number of voices is very important. In monophony it is one. If
     * there are more voices thant number_of_voices, the additional voices
     * won't be taken into consideration. */
    int number_of_voices;
    /* then start some metadata: */
    char *name;
    char *gabc_copyright;
    char *score_copyright;
    char *mode;
    char *mode_modifier;
    char *mode_differentia;
    char *author;
    /* There is one annotation for each line above the initial letter */
    char *annotation[MAX_ANNOTATIONS];
    /* field giving informations on the initial (no initial, normal initial
     * or two lines initial) */
    size_t nabc_lines;
    char *user_notes;
    /* the determination method (maximal ambitus, etc.) */
    unsigned char det_method;
    /* then, as there are some metadata that are voice-specific, we add a
     * pointer to the first voice_info. (see comments below) */
    struct gregorio_voice_info *first_voice_info;
    struct gregorio_header *headers;
    struct gregorio_header *last_header;
    unsigned char staff_lines;
    signed char highest_pitch;
    signed char high_ledger_line_pitch;
    signed char virgula_far_pitch;
    bool legacy_oriscus_orientation;
} gregorio_score;

/*
 *
 * gregorio_voice info contains everything that is voice_specific, for
 * example the key, etc. that can be different from one voice to another.
 * The order of the voice_info (it is a chained list) is the same as the
 * order of the voices (from top to bottom in their representation on the
 * score).
 *
 */

typedef struct gregorio_voice_info {
    gregorio_clef_info initial_clef;
    struct gregorio_voice_info *next_voice_info;
} gregorio_voice_info;

typedef struct gregorio_hepisema_adjustment {
    gregorio_hepisema_vbasepos vbasepos;
    char *nudge;
    signed char pitch_extremum;
} gregorio_hepisema_adjustment;

/* the maximum number of voices, more than this is total nonsense in
 * gregorian chant. */
#define MAX_NUMBER_OF_VOICES 10

#define MAX_TEXT_LENGTH 200

#define C_KEY 'c'
#define F_KEY 'f'
#define NO_KEY -5

extern unsigned short tex_position_id;
extern gregorio_clef_info gregorio_default_clef;

#define MONOPHONY 0

#define USELESS_VALUE 0

static __inline bool is_puncta_inclinata(char glyph)
{
    return glyph <= G_5_PUNCTA_INCLINATA_ASCENDENS;
}

#define IS_INITIO_DEBILIS 5
#define NO_INITIO_DEBILIS 0

static __inline bool is_tail_liquescentia(char liquescentia)
{
    return liquescentia & TAIL_LIQUESCENTIA_MASK;
}

static __inline bool is_initio_debilis(char liquescentia)
{
    return liquescentia & L_INITIO_DEBILIS;
}

static __inline bool is_fused(char liquescentia)
{
    return liquescentia & L_FUSED;
}

#define HEPISEMA_NONE 0
#define HEPISEMA_AUTO -1
#define HEPISEMA_FORCED -2

/* The first pitch MUST be an odd number */
#define LOWEST_PITCH 3
#define DUMMY_PITCH (LOWEST_PITCH + 6)
#define LOW_LEDGER_LINE_PITCH (LOWEST_PITCH + 1)
#define LOW_LINE_PITCH (LOWEST_PITCH + 3)
#define MAX_PITCH (LOWEST_PITCH + 4 + (2 * 5))

#define NO_PITCH -128

/* defines the maximal interval between two notes of the same glyph */
#define MAX_AMBITUS 5

void gregorio_struct_init(void);
void gregorio_struct_destroy(void);
gregorio_score *gregorio_new_score(void);
void gregorio_add_note(gregorio_note **current_note, signed char pitch,
        gregorio_shape shape, gregorio_sign signs,
        gregorio_liquescentia liquescentia, gregorio_note* prototype,
        const gregorio_scanner_location *loc);
void gregorio_add_glyph(gregorio_glyph **current_glyph,
        gregorio_glyph_type type, gregorio_note *first_note,
        gregorio_liquescentia liquescentia, bool is_cavum);
void gregorio_add_element(gregorio_element **current_element,
        gregorio_glyph *first_glyph);
void gregorio_add_syllable(gregorio_syllable **current_syllable,
        int number_of_voices, gregorio_element *elements[],
        gregorio_character *first_character,
        gregorio_character *first_translation_character,
        gregorio_word_position position, char *abovelinestext,
        gregorio_tr_centering translation_type, gregorio_nlba no_linebreak_area,
        gregorio_euouae euouae, const gregorio_scanner_location *loc,
        bool first_word, bool clear);
void gregorio_add_special_sign(gregorio_note *current_note, gregorio_sign sign);
void gregorio_change_shape(gregorio_note *note, gregorio_shape shape,
        bool legacy_oriscus_orientation);
void gregorio_add_cavum(gregorio_note *note);
void gregorio_position_h_episema_above(gregorio_note *note, signed char height,
        bool connect);
void gregorio_position_h_episema_below(gregorio_note *note, signed char height,
        bool connect);
void gregorio_add_h_episema(gregorio_note *note, grehepisema_size size,
        gregorio_vposition vposition, bool disable_bridge,
        unsigned int *nbof_isolated_episema);
void gregorio_add_sign(gregorio_note *note, gregorio_sign sign,
        gregorio_vposition vposition);
void gregorio_add_tail_liquescentia(gregorio_note *note,
        gregorio_liquescentia liquescentia, bool legacy_oriscus_orientation);
void gregorio_add_voice_info(gregorio_voice_info **current_voice_info);
void gregorio_free_voice_infos(gregorio_voice_info *voice_info);
void gregorio_free_one_note(gregorio_note **note);
void gregorio_free_one_glyph(gregorio_glyph **glyph);
void gregorio_free_one_element(gregorio_element **element);
void gregorio_free_score(gregorio_score *score);
void gregorio_free_characters(gregorio_character *current_character);
void gregorio_go_to_first_character(const gregorio_character **character);
void gregorio_add_clef_as_glyph(gregorio_glyph **current_glyph,
        gregorio_clef_info clef, unsigned short texverb);
void gregorio_add_pitched_element_as_glyph(gregorio_glyph **current_glyph,
        gregorio_type type, signed char pitch, bool force_pitch,
        unsigned short texverb);
void gregorio_add_unpitched_element_as_glyph(gregorio_glyph **current_glyph,
        gregorio_type type, gregorio_extra_info *info, gregorio_sign sign,
        unsigned short texverb);
void gregorio_add_end_of_line_as_note(gregorio_note **current_note,
        bool eol_ragged, bool eol_forces_custos, bool eol_forces_custos_on,
        const gregorio_scanner_location *loc);
void gregorio_add_custos_as_note(gregorio_note **current_note,
        const gregorio_scanner_location *loc);
void gregorio_add_manual_custos_as_note(gregorio_note **current_note,
        signed char pitch, const gregorio_scanner_location *loc);
void gregorio_add_suppress_custos_as_note(gregorio_note **current_note,
        const gregorio_scanner_location *loc);
void gregorio_add_clef_as_note(gregorio_note **current_note,
        gregorio_clef clef, signed char clef_line, bool flatted,
        const gregorio_scanner_location *loc);
void gregorio_add_secondary_clef_to_note(gregorio_note *current_note,
        gregorio_clef clef, signed char clef_line, bool flatted);
void gregorio_add_bar_as_note(gregorio_note **current_note, gregorio_bar bar,
        const gregorio_scanner_location *loc);
void gregorio_add_space_as_note(gregorio_note **current_note,
        gregorio_space space, char *factor,
        const gregorio_scanner_location *loc);
unsigned short gregorio_add_texverb_as_note(gregorio_note **current_note,
        char *str, gregorio_type type, const gregorio_scanner_location *loc);
void gregorio_add_nlba_as_note(gregorio_note **current_note,
        gregorio_nlba type, const gregorio_scanner_location *loc);
void gregorio_start_autofuse(gregorio_note **current_note,
        const gregorio_scanner_location *loc);
void gregorio_end_autofuse(gregorio_note **current_note,
        const gregorio_scanner_location *loc);
void gregorio_add_texverb_to_note(gregorio_note *current_note, char *str);
void gregorio_add_cs_to_note(gregorio_note *const*current_note, char *str,
        bool nabc);
void gregorio_add_misc_element(gregorio_element **current_element,
        gregorio_type type, gregorio_misc_element_info *info,
        unsigned short texverb);
void gregorio_set_score_annotation(gregorio_score *score, char *annotation);
void gregorio_set_score_staff_lines(gregorio_score *score, char staff_lines);
void gregorio_add_score_header(gregorio_score *score, char *name,
        char *value);
void gregorio_fix_initial_keys(gregorio_score *score,
        gregorio_clef_info default_clef);
void gregorio_go_to_first_note(gregorio_note **note);
void gregorio_go_to_first_glyph(gregorio_glyph **glyph);
bool gregorio_is_only_special(gregorio_element *element);
int gregorio_calculate_new_key(gregorio_clef_info clef);
void gregorio_add_character(gregorio_character **current_character,
        grewchar wcharacter);
void gregorio_begin_style(gregorio_character **current_character,
        grestyle_style style);
void gregorio_end_style(gregorio_character **current_character,
        grestyle_style style);
gregorio_character *gregorio_clone_characters(const gregorio_character *source);
signed char gregorio_determine_next_pitch(const gregorio_syllable *syllable,
        const gregorio_element *element, const gregorio_glyph *glyph,
        gregorio_shape *next_pitch_alteration);
const char *gregorio_unknown(int value);
gregorio_element *gregorio_get_clef_change(gregorio_syllable *syllable);
unsigned short gregorio_add_hepisema_adjustment(
        gregorio_hepisema_vbasepos vbasepos, char *nudge);
gregorio_hepisema_adjustment *gregorio_get_hepisema_adjustment(
        unsigned short index);
const char *gregorio_texverb(unsigned short index);
void gregorio_change_texverb(unsigned short index, char *texverb);

static __inline void gregorio_go_to_first_character_c(gregorio_character **character)
{
    gregorio_go_to_first_character((const gregorio_character **)character);
}

static __inline gregorio_note *gregorio_glyph_last_note(
        const gregorio_glyph *const glyph)
{
    gregorio_note *note;
    gregorio_assert(glyph && glyph->type == GRE_GLYPH, gregorio_glyph_last_note,
            "trying to find the last note of something that is not a glyph",
            return NULL);
    for (note = glyph->u.notes.first_note; note->next; note = note->next) {
        /* iterate to find the last note */
    }
    return note;
}

static __inline const gregorio_glyph *gregorio_next_non_texverb_glyph(
        const gregorio_glyph *glyph)
{
    if (glyph) {
        for (glyph = glyph->next; glyph; glyph = glyph->next) {
            if (glyph->type != GRE_TEXVERB_GLYPH) {
                return glyph;
            }
        }
    }
    return NULL;
}

static __inline const gregorio_glyph *gregorio_previous_non_texverb_glyph(
        const gregorio_glyph *glyph)
{
    if (glyph) {
        for (glyph = glyph->previous; glyph; glyph = glyph->previous) {
            if (glyph->type != GRE_TEXVERB_GLYPH) {
                return glyph;
            }
        }
    }
    return NULL;
}

static __inline char gregorio_clef_to_char(gregorio_clef clef)
{
    return (clef == CLEF_C)? 'c' : 'f';
}

static __inline signed char gregorio_adjust_pitch_into_staff(
        const gregorio_score *score, signed char pitch)
{
    while (pitch < LOWEST_PITCH) {
        pitch += 7;
    }
    while (pitch > score->highest_pitch) {
        pitch -= 7;
    }
    assert(pitch >= LOWEST_PITCH && pitch <= score->highest_pitch);
    return pitch;
}

#endif
