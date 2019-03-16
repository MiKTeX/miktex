/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.3.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         gabc_score_determination_parse
#define yylex           gabc_score_determination_lex
#define yyerror         gabc_score_determination_error
#define yydebug         gabc_score_determination_debug
#define yynerrs         gabc_score_determination_nerrs

#define yylval          gabc_score_determination_lval
#define yychar          gabc_score_determination_char
#define yylloc          gabc_score_determination_lloc

/* First part of user prologue.  */
#line 1 "gabc/gabc-score-determination.y" /* yacc.c:337  */

/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file implements the score parser.
 *
 * Gregorio score determination from gabc.
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

/*
 * 
 * This file is certainly not the most easy to understand, it is a bison file.
 * See the bison manual on gnu.org for further details.
 * 
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "bool.h"
#include "struct.h"
#include "unicode.h"
#include "messages.h"
#include "characters.h"
#include "support.h"
#include "sha1.h"
#include "plugins.h"
#include "gabc.h"

#define YYLLOC_DEFAULT(Current, Rhs, N) \
    if (N) { \
        (Current).first_line   = YYRHSLOC(Rhs, 1).first_line; \
        (Current).first_column = YYRHSLOC (Rhs, 1).first_column; \
        (Current).first_offset = YYRHSLOC (Rhs, 1).first_offset; \
        (Current).last_line    = YYRHSLOC (Rhs, N).last_line; \
        (Current).last_column  = YYRHSLOC (Rhs, N).last_column; \
        (Current).last_offset  = YYRHSLOC (Rhs, N).last_offset; \
        (Current).generate_point_and_click = YYRHSLOC (Rhs, 1).generate_point_and_click; \
    } else { \
        (Current).first_line   = (Current).last_line   = YYRHSLOC (Rhs, 0).last_line; \
        (Current).first_column = (Current).last_column = YYRHSLOC (Rhs, 0).last_column; \
        (Current).first_offset = (Current).last_offset = YYRHSLOC (Rhs, 0).last_offset; \
        (Current).first_offset = (Current).last_offset = YYRHSLOC (Rhs, 0).last_offset; \
        (Current).generate_point_and_click = YYRHSLOC (Rhs, 0).generate_point_and_click; \
    }

#include "gabc-score-determination.h"
#include "gabc-score-determination-l.h"

/* workaround for bison issue passing pointer to a "local" variable */
#define STYLE_BITS &styles

/* forward declaration of the flex/bison process function */
static int gabc_score_determination_parse(void);

/* uncomment it if you want to have an interactive shell to understand the
 * details on how bison works for a certain input */
/* int gabc_score_determination_debug=1; */

/*
 * 
 * We will need some variables and functions through the entire file, we
 * declare them there:
 * 
 */

/* the score that we will determine and return */
static gregorio_score *score;
/* an array of elements that we will use for each syllable */
static gregorio_element **elements;
gregorio_element *current_element;
/* a table containing the macros to use in gabc file */
static char *macros[10];
/* other variables that we will have to use */
static gregorio_character *current_character;
static gregorio_character *suspended_character;
static gregorio_character *first_text_character;
static gregorio_character *first_translation_character;
static gregorio_tr_centering translation_type;
static gregorio_nlba no_linebreak_area;
static gregorio_euouae euouae;
static gregorio_voice_info *current_voice_info;
static int number_of_voices;
static int voice;
/* see comments on text to understand this */
static gregorio_center_determination center_is_determined;
/* current_key is... the current key... updated by each notes determination
 * (for key changes) */
static int current_key;
static bool got_language;
static bool got_staff_lines;
static bool started_first_word;
static struct sha1_ctx digester;
static gabc_style_bits styles;
static bool generate_point_and_click;
static bool clear_syllable_text;
static bool has_protrusion;

/* punctum_inclinatum_orientation maintains the running punctum inclinatum
 * orientation in order to decide if the glyph needs to be cut when a punctum
 * inclinatum with forced orientation is encountered.  This should be set to
 * the shape of a non-liquescent punctum inclinatum with forced orientation
 * when one is encountered, be left alone when a non-liquescent punctum
 * inclinatum with undetermined orientation is encountered, or be reset to
 * S_PUNCTUM_INCLINATUM_UNDETERMINED otherwise (because such ends any previous
 * run of punctum inclinatum notes).  Based on the assumption that a punctum
 * inclinatum with forced orientation changes all the punctum inclinatum shapes
 * with undetermined orientation in the same run of notes before and after it
 * unless influenced by an earlier punctum inclinatum with forced orientation,
 * the value of punctum_inclinatum_orientation can be used to determine if a
 * punctum inclinatum with a forced orientation will have a different
 * orientation than the punctum inclinatum immediately before it, which would
 * require a cut of the glyph. */
static gregorio_shape punctum_inclinatum_orientation;

static __inline void check_multiple(const char *name, bool exists) {
    if (exists) {
        gregorio_messagef("det_score", VERBOSITY_WARNING, 0,
                _("several %s definitions found, only the last will be taken "
                "into consideration"), name);
    }
}

static void gabc_score_determination_error(const char *error_str)
{
    gregorio_message(error_str, (const char *) "gabc_score_determination_parse",
            VERBOSITY_ERROR, 0);
}

/*
 * The function that will initialize the variables. 
 */

static void initialize_variables(bool point_and_click)
{
    int i;
    /* build a brand new empty score */
    score = gregorio_new_score();
    /* initialization of the first voice info to an empty voice info */
    current_voice_info = NULL;
    gregorio_add_voice_info(&current_voice_info);
    score->first_voice_info = current_voice_info;
    /* other initializations */
    number_of_voices = 1;
    voice = 0; /* first (and only) voice */
    current_character = NULL;
    suspended_character = NULL;
    first_translation_character = NULL;
    first_text_character = NULL;
    translation_type = TR_NORMAL;
    no_linebreak_area = NLBA_NORMAL;
    euouae = EUOUAE_NORMAL;
    center_is_determined = CENTER_NOT_DETERMINED;
    current_key = gregorio_calculate_new_key(gregorio_default_clef);
    for (i = 0; i < 10; i++) {
        macros[i] = NULL;
    }
    got_language = false;
    got_staff_lines = false;
    started_first_word = false;
    styles = 0;
    punctum_inclinatum_orientation = S_PUNCTUM_INCLINATUM_UNDETERMINED;
    generate_point_and_click = point_and_click;
    clear_syllable_text = false;
    has_protrusion = false;
}

/*
 * function that frees the variables that need it, for when we have finished to 
 * determine the score 
 */

static void free_variables(void)
{
    int i;
    free(elements);
    for (i = 0; i < 10; i++) {
        free(macros[i]);
    }
}

/*
 * Function called when we have reached the end of the definitions, it tries to 
 * make the voice_infos coherent. 
 */
static void end_definitions(void)
{
    int i;

    gregorio_assert_only(gabc_check_infos_integrity(score), end_definitions,
            "can't determine valid infos on the score");

    elements = (gregorio_element **) gregorio_malloc(number_of_voices *
            sizeof(gregorio_element *));
    for (i = 0; i < number_of_voices; i++) {
        elements[i] = NULL;
    }

    if (!got_language) {
        static char latin[] = "Latin";
        gregorio_set_centering_language(latin);
    }
}

/*
 * Here starts the code for the determinations of the notes. The notes are not
 * precisely determined here, we separate the text describing the notes of each 
 * voice, and we call determine_elements_from_string to really determine them. 
 */
static char position = WORD_BEGINNING;
static gregorio_syllable *current_syllable = NULL;
static char *abovelinestext = NULL;

/*
 * Function called each time we find a space, it updates the current position. 
 */
static void update_position_with_space(void)
{
    if (position == WORD_MIDDLE) {
        position = WORD_END;
    }
    if (position == WORD_BEGINNING) {
        position = WORD_ONE_SYLLABLE;
    }
}

/*
 * When we encounter a translation center ending, we call this function that
 * sets translation_type = TR_WITH_CENTER_BEGINNING on previous syllable with
 * translation 
 */
static void gregorio_set_translation_center_beginning(
        gregorio_syllable *current_syllable)
{
    gregorio_syllable *syllable = current_syllable->previous_syllable;
    while (syllable) {
        if (syllable->translation_type == TR_WITH_CENTER_END) {
            gregorio_message("encountering translation centering end but "
                    "cannot find translation centering beginning...",
                    "set_translation_center_beginning", VERBOSITY_ERROR, 0);
            current_syllable->translation_type = TR_NORMAL;
            return;
        }
        if (syllable->translation) {
            syllable->translation_type = TR_WITH_CENTER_BEGINNING;
            return;
        }
        syllable = syllable->previous_syllable;
    }
    /* we didn't find any beginning... */
    gregorio_message("encountering translation centering end but cannot find "
            "translation centering beginning...",
            "set_translation_center_beginning", VERBOSITY_ERROR, 0);
    current_syllable->translation_type = TR_NORMAL;
}

static void ready_characters(void)
{
    if (current_character) {
        gregorio_go_to_first_character_c(&current_character);
        if (!score->first_syllable || (current_syllable
                && !current_syllable->previous_syllable
                && !current_syllable->text)) {
            started_first_word = true;
        }
    }
}

static void rebuild_score_characters(void)
{
    if (score->first_syllable) {
        gregorio_syllable *syllable;
        for (syllable = score->first_syllable; syllable;
                syllable = syllable->next_syllable) {
            const gregorio_character *t;

            /* find out if there is a forced center */
            gregorio_center_determination center = CENTER_NOT_DETERMINED;
            for (t = syllable->text; t; t = t->next_character) {
                if (!t->is_character && t->cos.s.style == ST_FORCED_CENTER) {
                    syllable->forced_center = true;
                    center = CENTER_FULLY_DETERMINED;
                    break;
                }
            }

            if (syllable == score->first_syllable) {
                /* leave the first syllable text untouched at this time */
                continue;
            }

            gregorio_rebuild_characters(&(syllable->text), center, false);

            if (syllable->first_word) {
                gregorio_set_first_word(&(syllable->text));
            }
        }
    }
}

/*
 * 
 * The two functions called when lex returns a style, we simply add it. All the 
 * complex things will be done by the function after...
 * 
 */

static void add_style(unsigned char style)
{
    gregorio_begin_style(&current_character, style);
}

static void end_style(unsigned char style)
{
    gregorio_end_style(&current_character, style);
}

static __inline void save_text(void)
{
    if (has_protrusion) {
        end_style(ST_PROTRUSION);
    }
    ready_characters();
    first_text_character = current_character;
}

/* a function called when we see a [, basically, all characters are added to
 * the translation pointer instead of the text pointer */
static void start_translation(unsigned char asked_translation_type)
{
    suspended_character = current_character;
    /* the middle letters of the translation have no sense */
    /*center_is_determined = CENTER_FULLY_DETERMINED;*/
    current_character = NULL;
    translation_type = asked_translation_type;
}

static void end_translation(void)
{
    ready_characters();
    first_translation_character = current_character;
    current_character = suspended_character;
}

/*
 * add_text is the function called when lex returns a char *. In
 * this function we convert it into grewchar, and then we add the corresponding 
 * gregorio_characters in the list of gregorio_characters. 
 */

static void add_text(char *mbcharacters)
{
    if (!current_character) {
        /* insert open styles, leaving out ELISION on purpose */
        if (styles & SB_I) {
            add_style(ST_ITALIC);
        }
        if (styles & SB_B) {
            add_style(ST_BOLD);
        }
        if (styles & SB_TT) {
            add_style(ST_TT);
        }
        if (styles & SB_SC) {
            add_style(ST_SMALL_CAPS);
        }
        if (styles & SB_UL) {
            add_style(ST_UNDERLINED);
        }
        if (styles & SB_C) {
            add_style(ST_COLORED);
        }
    }
    if (current_character) {
        current_character->next_character = gregorio_build_char_list_from_buf(
                mbcharacters);
        current_character->next_character->previous_character =
                current_character;
    } else {
        current_character = gregorio_build_char_list_from_buf(mbcharacters);
    }
    while (current_character && current_character->next_character) {
        current_character = current_character->next_character;
    }
    free(mbcharacters);
}

static void add_protrusion(char *factor)
{
    if (has_protrusion) {
        gregorio_message("syllable already has protrusion; pr tag ignored",
                "det_score", VERBOSITY_WARNING, 0);
        free(factor);
    } else {
        if (center_is_determined == CENTER_HALF_DETERMINED) {
            gregorio_message("closing open syllable center before protrusion",
                    "det_score", VERBOSITY_WARNING, 0);
            end_style(ST_FORCED_CENTER);
            center_is_determined = CENTER_FULLY_DETERMINED;
        }

        add_style(ST_PROTRUSION_FACTOR);
        add_text(factor);
        end_style(ST_PROTRUSION_FACTOR);
        add_style(ST_PROTRUSION);
        has_protrusion = true;
    }
}

static void add_auto_protrusion(char *protrusion)
{
    if (has_protrusion) {
        add_text(protrusion);
    } else {
        add_style(ST_PROTRUSION_FACTOR);
        add_style(ST_VERBATIM);
        add_text(gregorio_strdup("\\GreProtrusionFactor{"));

        switch (*protrusion) {
        case ',':
            add_text(gregorio_strdup("comma"));
            break;
        case ';':
            add_text(gregorio_strdup("semicolon"));
            break;
        case ':':
            add_text(gregorio_strdup("colon"));
            break;
        case '.':
            add_text(gregorio_strdup("period"));
            break;
        default:
            /* not reachable unless there's a programming error */
            /* LCOV_EXCL_START */
            gregorio_fail2(add_auto_protrusion,
                    "unsupported protruding punctuation: %c", *protrusion);
            break;
            /* LCOV_EXCL_STOP */
        }

        add_text(gregorio_strdup("}"));
        end_style(ST_VERBATIM);
        end_style(ST_PROTRUSION_FACTOR);

        add_style(ST_PROTRUSION);
        add_text(protrusion);
        end_style(ST_PROTRUSION);

        has_protrusion = true;
    }
}

/*
 * Function to close a syllable and update the position.
 */

static void close_syllable(YYLTYPE *loc)
{
    int i = 0;
    gregorio_character *ch;

    /* make sure any elisions that are opened are closed within the syllable */
    for (ch = first_text_character; ch; ch = ch->next_character) {
        if (!ch->is_character) {
            switch (ch->cos.s.style) {
            case ST_ELISION:
                switch (ch->cos.s.type) {
                case ST_T_BEGIN:
                    ++i;
                    /* the parser precludes this from falling here */
                    gregorio_assert_only(i <= 1, close_syllable,
                            "elisions may not be nested");
                    break;

                case ST_T_END:
                    --i;
                    /* the parser precludes this from failing here */
                    gregorio_assert_only(i >= 0, close_syllable,
                            "encountered elision end with no beginning");
                    break;

                case ST_T_NOTHING:
                    /* not reachable unless there's a programming error */
                    /* LCOV_EXCL_START */
                    gregorio_fail(close_syllable, "encountered ST_T_NOTHING");
                    break;
                    /* LCOV_EXCL_STOP */
                }
                break;

            case ST_FORCED_CENTER:
                if (i > 0) {
                    gregorio_message(
                            _("forced center may not be within an elision"),
                            "close_syllable", VERBOSITY_ERROR, 0);
                }
                break;

            default:
                break;
            }
        }
    }
    /* the parser precludes this from failing here */
    gregorio_assert_only(i == 0, close_syllable,
            "encountered elision beginning with no end");

    gregorio_add_syllable(&current_syllable, number_of_voices, elements,
            first_text_character, first_translation_character, position,
            abovelinestext, translation_type, no_linebreak_area, euouae, loc,
            started_first_word, clear_syllable_text);
    if (!score->first_syllable) {
        /* we rebuild the first syllable if we have to */
        score->first_syllable = current_syllable;
    }
    if (translation_type == TR_WITH_CENTER_END) {
        gregorio_set_translation_center_beginning(current_syllable);
    }
    /* we update the position */
    if (position == WORD_BEGINNING) {
        position = WORD_MIDDLE;
    }
    if (position == WORD_ONE_SYLLABLE || position == WORD_END) {
        position = WORD_BEGINNING;

        if (started_first_word) {
            started_first_word = false;
        }
    }
    center_is_determined = CENTER_NOT_DETERMINED;
    current_character = NULL;
    suspended_character = NULL;
    first_text_character = NULL;
    first_translation_character = NULL;
    translation_type = TR_NORMAL;
    no_linebreak_area = NLBA_NORMAL;
    euouae = EUOUAE_NORMAL;
    abovelinestext = NULL;
    for (i = 0; i < number_of_voices; i++) {
        elements[i] = NULL;
    }
    current_element = NULL;
    clear_syllable_text = false;
    has_protrusion = false;
}

void gabc_digest(const void *const buf, const size_t size)
{
    sha1_process_bytes(buf, size, &digester);
}

/*
 * The "main" function. It is the function that is called when we have to read
 * a gabc file. It takes a file descriptor, that is to say a file that is
 * aleady open. It returns a valid gregorio_score 
 */

gregorio_score *gabc_read_score(FILE *f_in, bool point_and_click)
{
    /* compute the SHA-1 digest while parsing, for I/O efficiency */
    sha1_init_ctx(&digester);
    /* digest GREGORIO_VERSION to get a different value when the version
    changes */
    sha1_process_bytes(GREGORIO_VERSION, strlen(GREGORIO_VERSION), &digester);
    /* the input file that flex will parse */
    gabc_score_determination_in = f_in;
    gregorio_assert(f_in, gabc_read_score, "can't read stream from NULL",
            return NULL);
    initialize_variables(point_and_click);
    /* the flex/bison main call, it will build the score (that we have
     * initialized) */
    gabc_score_determination_parse();
    if (!score->legacy_oriscus_orientation) {
        gabc_determine_oriscus_orientation(score);
    }
    gabc_determine_punctum_inclinatum_orientation(score);
    gabc_determine_ledger_lines(score);
    gregorio_fix_initial_keys(score, gregorio_default_clef);
    rebuild_score_characters();
    gabc_suppress_extra_custos_at_linebreak(score);
    gabc_fix_custos_pitches(score);
    gabc_det_notes_finish();
    free_variables();
    /* then we check the validity and integrity of the score we have built. */
    if (!gabc_check_score_integrity(score)) {
        gregorio_message(_("unable to determine a valid score from file"),
                "gabc_read_score", VERBOSITY_ERROR, 0);
    }
    sha1_finish_ctx(&digester, score->digest);
    return score;
}

unsigned char nabc_state = 0;
size_t nabc_lines = 0;

static void gabc_y_add_notes(char *notes, YYLTYPE loc) {
    if (nabc_state == 0) {
        if (!elements[voice]) {
            elements[voice] = gabc_det_elements_from_string(notes, &current_key,
                    macros, &loc, &punctum_inclinatum_orientation, score);
            current_element = elements[voice];
        } else {
            gregorio_element *new_elements = gabc_det_elements_from_string(
                    notes, &current_key, macros, &loc,
                    &punctum_inclinatum_orientation, score);
            gregorio_element *last_element = elements[voice];
            while (last_element->next) {
                last_element = last_element->next;
            }
            last_element->next = new_elements;
            new_elements->previous = last_element;
            current_element = new_elements;
        }
    } else {
        if (!elements[voice]) {
            gregorio_add_element(&elements[voice], NULL);
            current_element = elements[voice];
        }
        gregorio_assert(current_element, gabc_y_add_notes,
                "current_element is null, this shouldn't happen!",
                return);
        if (!current_element->nabc) {
            current_element->nabc = (char **) gregorio_calloc (nabc_lines,
                    sizeof (char *));
        }
        current_element->nabc[nabc_state-1] = gregorio_strdup(notes);
        current_element->nabc_lines = nabc_state;
    }
}

static char *concatenate(char *first, char *const second) {
    first = (char *)gregorio_realloc(first, strlen(first) + strlen(second) + 1);
    strcat(first, second);
    free(second);
    return first;
}

#line 733 "gabc/gabc-score-determination-y.c" /* yacc.c:337  */
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "gabc-score-determination-y.h".  */
#ifndef YY_GABC_SCORE_DETERMINATION_GABC_GABC_SCORE_DETERMINATION_Y_H_INCLUDED
# define YY_GABC_SCORE_DETERMINATION_GABC_GABC_SCORE_DETERMINATION_Y_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int gabc_score_determination_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    NAME = 258,
    AUTHOR = 259,
    GABC_COPYRIGHT = 260,
    SCORE_COPYRIGHT = 261,
    LANGUAGE = 262,
    STAFF_LINES = 263,
    ORISCUS_ORIENTATION = 264,
    DEF_MACRO = 265,
    OTHER_HEADER = 266,
    ANNOTATION = 267,
    MODE = 268,
    MODE_MODIFIER = 269,
    MODE_DIFFERENTIA = 270,
    END_OF_DEFINITIONS = 271,
    END_OF_FILE = 272,
    COLON = 273,
    SEMICOLON = 274,
    CHARACTERS = 275,
    NOTES = 276,
    HYPHEN = 277,
    ATTRIBUTE = 278,
    OPENING_BRACKET = 279,
    CLOSING_BRACKET = 280,
    CLOSING_BRACKET_WITH_SPACE = 281,
    I_BEGIN = 282,
    I_END = 283,
    TT_BEGIN = 284,
    TT_END = 285,
    UL_BEGIN = 286,
    UL_END = 287,
    C_BEGIN = 288,
    C_END = 289,
    B_BEGIN = 290,
    B_END = 291,
    SC_BEGIN = 292,
    SC_END = 293,
    SP_BEGIN = 294,
    SP_END = 295,
    VERB_BEGIN = 296,
    VERB_END = 297,
    CENTER_BEGIN = 298,
    CENTER_END = 299,
    ELISION_BEGIN = 300,
    ELISION_END = 301,
    TRANSLATION_BEGIN = 302,
    TRANSLATION_END = 303,
    TRANSLATION_CENTER_END = 304,
    ALT_BEGIN = 305,
    ALT_END = 306,
    NLBA_B = 307,
    NLBA_E = 308,
    EUOUAE_B = 309,
    EUOUAE_E = 310,
    NABC_CUT = 311,
    NABC_LINES = 312,
    CLEAR = 313,
    PROTRUSION = 314,
    PROTRUSION_VALUE = 315,
    PROTRUSION_END = 316,
    PROTRUDING_PUNCTUATION = 317
  };
#endif
/* Tokens.  */
#define NAME 258
#define AUTHOR 259
#define GABC_COPYRIGHT 260
#define SCORE_COPYRIGHT 261
#define LANGUAGE 262
#define STAFF_LINES 263
#define ORISCUS_ORIENTATION 264
#define DEF_MACRO 265
#define OTHER_HEADER 266
#define ANNOTATION 267
#define MODE 268
#define MODE_MODIFIER 269
#define MODE_DIFFERENTIA 270
#define END_OF_DEFINITIONS 271
#define END_OF_FILE 272
#define COLON 273
#define SEMICOLON 274
#define CHARACTERS 275
#define NOTES 276
#define HYPHEN 277
#define ATTRIBUTE 278
#define OPENING_BRACKET 279
#define CLOSING_BRACKET 280
#define CLOSING_BRACKET_WITH_SPACE 281
#define I_BEGIN 282
#define I_END 283
#define TT_BEGIN 284
#define TT_END 285
#define UL_BEGIN 286
#define UL_END 287
#define C_BEGIN 288
#define C_END 289
#define B_BEGIN 290
#define B_END 291
#define SC_BEGIN 292
#define SC_END 293
#define SP_BEGIN 294
#define SP_END 295
#define VERB_BEGIN 296
#define VERB_END 297
#define CENTER_BEGIN 298
#define CENTER_END 299
#define ELISION_BEGIN 300
#define ELISION_END 301
#define TRANSLATION_BEGIN 302
#define TRANSLATION_END 303
#define TRANSLATION_CENTER_END 304
#define ALT_BEGIN 305
#define ALT_END 306
#define NLBA_B 307
#define NLBA_E 308
#define EUOUAE_B 309
#define EUOUAE_E 310
#define NABC_CUT 311
#define NABC_LINES 312
#define CLEAR 313
#define PROTRUSION 314
#define PROTRUSION_VALUE 315
#define PROTRUSION_END 316
#define PROTRUDING_PUNCTUATION 317

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE gabc_score_determination_lval;
extern YYLTYPE gabc_score_determination_lloc;
int gabc_score_determination_parse (void);

#endif /* !YY_GABC_SCORE_DETERMINATION_GABC_GABC_SCORE_DETERMINATION_Y_H_INCLUDED  */



#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   247

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  63
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  28
/* YYNRULES -- Number of rules.  */
#define YYNRULES  101
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  145

#define YYUNDEFTOK  2
#define YYMAXUTOK   317

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   700,   700,   704,   709,   710,   714,   717,   723,   726,
     732,   737,   746,   752,   757,   762,   767,   772,   777,   785,
     793,   802,   807,   811,   816,   817,   821,   826,   832,   843,
     847,   855,   856,   857,   861,   864,   867,   870,   873,   876,
     879,   882,   888,   891,   894,   897,   900,   903,   906,   909,
     915,   918,   935,   938,   951,   954,   960,   963,   969,   972,
     978,   979,   982,   983,   984,   985,   986,   987,   988,   991,
     992,   995,  1001,  1002,  1006,  1009,  1010,  1011,  1014,  1020,
    1021,  1025,  1031,  1034,  1037,  1044,  1050,  1054,  1061,  1068,
    1073,  1078,  1082,  1089,  1096,  1101,  1109,  1112,  1118,  1119,
    1122,  1123
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NAME", "AUTHOR", "GABC_COPYRIGHT",
  "SCORE_COPYRIGHT", "LANGUAGE", "STAFF_LINES", "ORISCUS_ORIENTATION",
  "DEF_MACRO", "OTHER_HEADER", "ANNOTATION", "MODE", "MODE_MODIFIER",
  "MODE_DIFFERENTIA", "END_OF_DEFINITIONS", "END_OF_FILE", "COLON",
  "SEMICOLON", "CHARACTERS", "NOTES", "HYPHEN", "ATTRIBUTE",
  "OPENING_BRACKET", "CLOSING_BRACKET", "CLOSING_BRACKET_WITH_SPACE",
  "I_BEGIN", "I_END", "TT_BEGIN", "TT_END", "UL_BEGIN", "UL_END",
  "C_BEGIN", "C_END", "B_BEGIN", "B_END", "SC_BEGIN", "SC_END", "SP_BEGIN",
  "SP_END", "VERB_BEGIN", "VERB_END", "CENTER_BEGIN", "CENTER_END",
  "ELISION_BEGIN", "ELISION_END", "TRANSLATION_BEGIN", "TRANSLATION_END",
  "TRANSLATION_CENTER_END", "ALT_BEGIN", "ALT_END", "NLBA_B", "NLBA_E",
  "EUOUAE_B", "EUOUAE_E", "NABC_CUT", "NABC_LINES", "CLEAR", "PROTRUSION",
  "PROTRUSION_VALUE", "PROTRUSION_END", "PROTRUDING_PUNCTUATION",
  "$accept", "score", "all_definitions", "definitions", "attribute_value",
  "attribute", "definition", "notes", "note", "closing_bracket_with_space",
  "style_beginning", "style_end", "special_style_beginning",
  "special_style_end", "euouae", "linebreak_area", "protrusion",
  "character", "text", "translation_character", "translation_text",
  "translation_beginning", "translation", "above_line_text",
  "syllable_with_notes", "notes_without_word", "syllable", "syllables", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317
};
# endif

#define YYPACT_NINF -85

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-85)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -85,    11,   -85,    69,   -85,    66,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   -85,
       0,   -85,   -85,   -20,   -85,   -85,   -85,   -85,   -85,   -85,
     -85,   -85,   -85,   -85,   -85,   -85,   -85,   -85,   -85,   -85,
     -85,   -85,   -85,   -85,   -85,   -85,   -85,     4,   -85,   -85,
     -85,   -85,   -85,   -41,   -19,   -85,   -85,   -85,   -85,   -85,
     -85,   -85,   -85,   107,   148,    -4,   -85,   -85,   -85,   -85,
      -7,   -85,   -85,   -85,   -85,   -85,   -85,   -85,   -85,   -85,
     -85,   -85,   -85,   -85,   -85,   -85,     1,   -11,   -17,   -29,
     -85,     9,    -2,   -85,    -1,   -85,    13,   -85,   -85,   -85,
     -85,   -85,   -85,   -85,   184,   -85,   -85,   -85,    -6,   -11,
     -85,   -18,    27,    32,   -85,   -85,   -85,   -85,   -11,   -85,
     -85,    26,   -11,   -85,    28,   -85,   -85,   -85,   -11,   -85,
     -85,   -11,    27,   -85,   -85,   -85,   -85,   -11,   -11,   -85,
     -11,   -85,   -11,   -11,   -11
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,   100,     0,     1,     2,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     3,
       0,     5,    61,    70,    24,    34,    42,    35,    43,    36,
      44,    37,    45,    38,    46,    39,    47,    41,    49,    40,
      48,    51,    53,    50,    52,    81,    84,     0,    56,    57,
      54,    55,    68,    59,    71,    62,    63,    64,    65,    67,
      66,    69,    72,     0,     0,     0,    60,    98,    99,   101,
       0,    11,    21,    13,    14,    12,    18,    22,    10,    23,
      20,    15,    16,    17,    19,    24,     0,    96,     0,     0,
      24,     0,    70,    24,    71,    73,     0,    74,    77,    82,
      78,    75,    76,    79,     0,    24,     9,     6,     0,    87,
      24,     0,    29,    31,    25,    30,    85,    58,    89,    24,
      24,     0,    86,    24,     0,    24,    83,    80,    97,     8,
       7,    92,    26,    28,    27,    33,    32,    94,    88,    24,
      90,    24,    91,    93,    95
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -85,   -85,   -85,   -85,   -85,   227,   -85,   -84,   -85,   -60,
     -62,   -61,   -85,   -85,   -85,   -85,   -85,   -10,   -85,   -50,
     -85,   -85,   -23,   -85,   -85,   -85,   -85,   -85
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     2,     3,   108,    71,    21,    87,   114,   115,
      55,    56,    57,    58,    59,    60,    61,    62,    63,   103,
     104,    64,    65,    66,    67,    68,    69,     5
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      86,   109,   101,   102,    85,    90,   118,   132,   113,   122,
     111,     4,   106,   129,   112,   113,   107,   130,    70,    89,
     105,   128,   120,   123,    88,   110,   131,    45,    45,    46,
      46,    91,   117,   119,   116,   137,   138,   125,   133,   140,
      96,   142,   101,   102,   135,    45,    45,    46,    46,   136,
     139,   134,   141,    95,   127,   143,     0,   144,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   121,
       0,   124,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    22,     0,    23,     0,
      24,     0,     0,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,     0,    46,    47,     0,    48,    49,
      50,    51,     0,     0,    52,    53,    20,    22,    54,    92,
       0,    93,     0,     0,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,     0,    46,    47,     0,    48,
      49,    50,    51,     0,     0,    52,    53,     0,    97,    94,
      98,     0,     0,     0,     0,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,     0,     0,     0,    97,     0,    98,     0,     0,     0,
     100,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,   126,     0,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,   100,    84
};

static const yytype_int16 yycheck[] =
{
      23,    85,    64,    64,    24,    24,    90,    25,    26,    93,
      21,     0,    19,    19,    25,    26,    23,    23,    18,    60,
      24,   105,    24,    24,    20,    24,   110,    47,    47,    49,
      49,    54,    61,    24,    51,   119,   120,    24,    56,   123,
      63,   125,   104,   104,    17,    47,    47,    49,    49,    17,
      24,   111,    24,    63,   104,   139,    -1,   141,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      -1,    94,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    20,    -1,    22,    -1,
      24,    -1,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    50,    -1,    52,    53,
      54,    55,    -1,    -1,    58,    59,    57,    20,    62,    22,
      -1,    24,    -1,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    50,    -1,    52,
      53,    54,    55,    -1,    -1,    58,    59,    -1,    20,    62,
      22,    -1,    -1,    -1,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    20,    -1,    22,    -1,    -1,    -1,
      62,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    -1,    -1,    -1,
      -1,    -1,    48,    -1,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    62,    20
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    64,    65,    66,     0,    90,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      57,    69,    20,    22,    24,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    49,    50,    52,    53,
      54,    55,    58,    59,    62,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    84,    85,    86,    87,    88,    89,
      18,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    24,    85,    70,    20,    60,
      24,    85,    22,    24,    62,    80,    85,    20,    22,    48,
      62,    73,    74,    82,    83,    24,    19,    23,    67,    70,
      24,    21,    25,    26,    71,    72,    51,    61,    70,    24,
      24,    85,    70,    24,    85,    24,    48,    82,    70,    19,
      23,    70,    25,    56,    72,    17,    17,    70,    70,    24,
      70,    24,    70,    70,    70
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    63,    64,    65,    66,    66,    67,    67,    68,    68,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    70,    70,    71,    71,    71,    71,
      71,    72,    72,    72,    73,    73,    73,    73,    73,    73,
      73,    73,    74,    74,    74,    74,    74,    74,    74,    74,
      75,    75,    76,    76,    77,    77,    78,    78,    79,    79,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    81,    81,    82,    82,    82,    82,    82,    83,
      83,    84,    85,    85,    85,    86,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    88,    88,    89,    89,
      90,    90
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     0,     2,     1,     2,     3,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     0,     2,     2,     2,     2,     1,
       1,     1,     2,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       2,     1,     2,     3,     1,     3,     3,     3,     4,     3,
       4,     4,     4,     5,     4,     5,     2,     3,     1,     1,
       0,     2
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (yylocationp);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyo, *yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
{
  unsigned long yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return (YYSIZE_T) (yystpcpy (yyres, yystr) - yyres);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

/* User initialization code.  */
#line 655 "gabc/gabc-score-determination.y" /* yacc.c:1431  */
{
    yylloc.first_line = 1;
    yylloc.first_column = 0;
    yylloc.first_offset = 0;
    yylloc.last_line = 1;
    yylloc.last_column = 0;
    yylloc.last_offset = 0;
    yylloc.generate_point_and_click = generate_point_and_click;
}

#line 2058 "gabc/gabc-score-determination-y.c" /* yacc.c:1431  */
  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yynewstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  *yyssp = (yytype_int16) yystate;

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = (YYSIZE_T) (yyssp - yyss + 1);

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (STYLE_BITS);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 704 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        end_definitions();
    }
#line 2254 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 6:
#line 714 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        yyval.text = yyvsp[0].text;
    }
#line 2262 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 7:
#line 717 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        yyval.text = concatenate(yyvsp[-1].text, yyvsp[0].text);
    }
#line 2270 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 8:
#line 723 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        yyval.text = yyvsp[-1].text;
    }
#line 2278 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 9:
#line 726 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        yyval.text = NULL;
    }
#line 2286 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 10:
#line 732 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        /* these definitions are not passed through */
        free(macros[yyvsp[-1].character - '0']);
        macros[yyvsp[-1].character - '0'] = yyvsp[0].text;
    }
#line 2296 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 11:
#line 737 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        if (yyvsp[0].text == NULL) {
            gregorio_message("name can't be empty","det_score",
                    VERBOSITY_WARNING, 0);
        }
        check_multiple("name", score->name != NULL);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->name = yyvsp[0].text;
    }
#line 2310 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 12:
#line 746 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        check_multiple("language", got_language);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        gregorio_set_centering_language(yyvsp[0].text);
        got_language = true;
    }
#line 2321 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 13:
#line 752 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        check_multiple("gabc-copyright", score->gabc_copyright != NULL);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->gabc_copyright = yyvsp[0].text;
    }
#line 2331 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 14:
#line 757 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        check_multiple("score_copyright", score->score_copyright != NULL);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->score_copyright = yyvsp[0].text;
    }
#line 2341 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 15:
#line 762 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        check_multiple("mode", score->mode != 0);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->mode = yyvsp[0].text;
    }
#line 2351 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 16:
#line 767 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        check_multiple("mode-modifier", score->mode_modifier != NULL);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->mode_modifier = yyvsp[0].text;
    }
#line 2361 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 17:
#line 772 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        check_multiple("mode-differentia", score->mode_differentia != NULL);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->mode_differentia = yyvsp[0].text;
    }
#line 2371 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 18:
#line 777 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        check_multiple("staff-lines", got_staff_lines);
        if (yyvsp[0].text) {
            gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
            gregorio_set_score_staff_lines(score, atoi(yyvsp[0].text));
            got_staff_lines = true;
        }
    }
#line 2384 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 19:
#line 785 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        check_multiple("nabc lines", score->nabc_lines != 0);
        if (yyvsp[0].text) {
            gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
            nabc_lines=atoi(yyvsp[0].text);
            score->nabc_lines=nabc_lines;
        }
    }
#line 2397 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 20:
#line 793 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        if (score->annotation [MAX_ANNOTATIONS - 1]) {
            gregorio_messagef("det_score", VERBOSITY_WARNING, 0,
                    _("too many definitions of annotation found, only the "
                    "first %d will be taken"), MAX_ANNOTATIONS);
        }
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        gregorio_set_score_annotation(score, yyvsp[0].text);
    }
#line 2411 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 21:
#line 802 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        check_multiple("author", score->author != NULL);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->author = yyvsp[0].text;
    }
#line 2421 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 22:
#line 807 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->legacy_oriscus_orientation = (strcmp(yyvsp[0].text, "legacy") == 0);
    }
#line 2430 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 23:
#line 811 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
    }
#line 2438 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 26:
#line 821 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        gabc_y_add_notes(yyvsp[-1].text, (yylsp[-1]));
        free(yyvsp[-1].text);
        nabc_state=0;
    }
#line 2448 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 27:
#line 826 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        gabc_y_add_notes(yyvsp[-1].text, (yylsp[-1]));
        free(yyvsp[-1].text);
        nabc_state=0;
        update_position_with_space();
    }
#line 2459 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 28:
#line 832 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        if (!nabc_lines) {
            gregorio_message(_("You used character \"|\" in gabc without "
                               "setting \"nabc-lines\" parameter. Please "
                               "set it in your gabc header."),
                             "det_score", VERBOSITY_FATAL, 0);
        }
        gabc_y_add_notes(yyvsp[-1].text, (yylsp[-1]));
        free(yyvsp[-1].text);
        nabc_state = (nabc_state + 1) % (nabc_lines+1);
    }
#line 2475 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 29:
#line 843 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        elements[voice]=NULL;
        nabc_state=0;
    }
#line 2484 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 30:
#line 847 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        elements[voice]=NULL;
        nabc_state=0;
        update_position_with_space();
    }
#line 2494 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 34:
#line 861 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_style(ST_ITALIC);
    }
#line 2502 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 35:
#line 864 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_style(ST_TT);
    }
#line 2510 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 36:
#line 867 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_style(ST_UNDERLINED);
    }
#line 2518 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 37:
#line 870 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_style(ST_COLORED);
    }
#line 2526 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 38:
#line 873 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_style(ST_BOLD);
    }
#line 2534 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 39:
#line 876 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_style(ST_SMALL_CAPS);
    }
#line 2542 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 40:
#line 879 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_style(ST_VERBATIM);
    }
#line 2550 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 41:
#line 882 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_style(ST_SPECIAL_CHAR);
    }
#line 2558 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 42:
#line 888 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        end_style(ST_ITALIC);
    }
#line 2566 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 43:
#line 891 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        end_style(ST_TT);
    }
#line 2574 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 44:
#line 894 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        end_style(ST_UNDERLINED);
    }
#line 2582 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 45:
#line 897 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        end_style(ST_COLORED);
    }
#line 2590 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 46:
#line 900 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        end_style(ST_BOLD);
    }
#line 2598 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 47:
#line 903 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        end_style(ST_SMALL_CAPS);
    }
#line 2606 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 48:
#line 906 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        end_style(ST_VERBATIM);
    }
#line 2614 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 49:
#line 909 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        end_style(ST_SPECIAL_CHAR);
    }
#line 2622 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 50:
#line 915 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_style(ST_ELISION);
    }
#line 2630 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 51:
#line 918 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        if (center_is_determined) {
            gregorio_message(
                    "syllable already has center; ignoring additional center",
                    "det_score", VERBOSITY_WARNING, 0);
        } else if (has_protrusion) {
            gregorio_message(
                    "center not allowed after protrusion; ignored",
                    "det_score", VERBOSITY_WARNING, 0);
        } else {
            add_style(ST_FORCED_CENTER);
            center_is_determined = CENTER_HALF_DETERMINED;
        }
    }
#line 2649 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 52:
#line 935 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        end_style(ST_ELISION);
    }
#line 2657 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 53:
#line 938 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        if (center_is_determined == CENTER_HALF_DETERMINED) {
            end_style(ST_FORCED_CENTER);
            center_is_determined = CENTER_FULLY_DETERMINED;
        } else {
            gregorio_message(
                    "not within a syllable center",
                    "det_score", VERBOSITY_WARNING, 0);
        }
    }
#line 2672 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 54:
#line 951 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        euouae = EUOUAE_BEGINNING;
    }
#line 2680 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 55:
#line 954 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        euouae = EUOUAE_END;
    }
#line 2688 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 56:
#line 960 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        no_linebreak_area = NLBA_BEGINNING;
    }
#line 2696 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 57:
#line 963 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        no_linebreak_area = NLBA_END;
    }
#line 2704 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 58:
#line 969 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_protrusion(yyvsp[-1].text);
    }
#line 2712 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 59:
#line 972 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_protrusion(gregorio_strdup("d")); /* d = default */
    }
#line 2720 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 61:
#line 979 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_text(yyvsp[0].text);
    }
#line 2728 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 68:
#line 988 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        clear_syllable_text = true;
    }
#line 2736 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 70:
#line 992 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_text(gregorio_strdup("-"));
    }
#line 2744 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 71:
#line 995 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_text(yyvsp[0].text);
    }
#line 2752 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 74:
#line 1006 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_text(yyvsp[0].text);
    }
#line 2760 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 77:
#line 1011 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_text(gregorio_strdup("-"));
    }
#line 2768 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 78:
#line 1014 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_text(yyvsp[0].text);
    }
#line 2776 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 81:
#line 1025 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        start_translation(TR_NORMAL);
    }
#line 2784 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 82:
#line 1031 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        end_translation();
    }
#line 2792 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 83:
#line 1034 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        end_translation();
    }
#line 2800 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 84:
#line 1037 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        start_translation(TR_WITH_CENTER_END);
        end_translation();
    }
#line 2809 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 85:
#line 1044 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        abovelinestext = yyvsp[-1].text;
    }
#line 2817 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 86:
#line 1050 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        save_text();
        close_syllable(&(yylsp[-2]));
    }
#line 2826 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 87:
#line 1054 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_style(ST_VERBATIM);
        add_text(gregorio_strdup("\\GreForceHyphen"));
        end_style(ST_VERBATIM);
        save_text();
        close_syllable(&(yylsp[-2]));
    }
#line 2838 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 88:
#line 1061 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_style(ST_VERBATIM);
        add_text(gregorio_strdup("\\GreForceHyphen"));
        end_style(ST_VERBATIM);
        save_text();
        close_syllable(&(yylsp[-3]));
    }
#line 2850 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 89:
#line 1068 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_auto_protrusion(yyvsp[-2].text);
        save_text();
        close_syllable(&(yylsp[-2]));
    }
#line 2860 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 90:
#line 1073 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_auto_protrusion(yyvsp[-2].text);
        save_text();
        close_syllable(&(yylsp[-3]));
    }
#line 2870 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 91:
#line 1078 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        save_text();
        close_syllable(&(yylsp[-3]));
    }
#line 2879 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 92:
#line 1082 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_style(ST_VERBATIM);
        add_text(gregorio_strdup("\\GreForceHyphen"));
        end_style(ST_VERBATIM);
        save_text();
        close_syllable(&(yylsp[-3]));
    }
#line 2891 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 93:
#line 1089 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_style(ST_VERBATIM);
        add_text(gregorio_strdup("\\GreForceHyphen"));
        end_style(ST_VERBATIM);
        save_text();
        close_syllable(&(yylsp[-4]));
    }
#line 2903 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 94:
#line 1096 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_auto_protrusion(yyvsp[-3].text);
        save_text();
        close_syllable(&(yylsp[-3]));
    }
#line 2913 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 95:
#line 1101 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        add_auto_protrusion(yyvsp[-3].text);
        save_text();
        close_syllable(&(yylsp[-4]));
    }
#line 2923 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 96:
#line 1109 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        close_syllable(NULL);
    }
#line 2931 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;

  case 97:
#line 1112 "gabc/gabc-score-determination.y" /* yacc.c:1652  */
    {
        close_syllable(NULL);
    }
#line 2939 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
    break;


#line 2943 "gabc/gabc-score-determination-y.c" /* yacc.c:1652  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
