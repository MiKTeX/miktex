/* A Bison parser, made by GNU Bison 3.7.5.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30705

/* Bison version string.  */
#define YYBISON_VERSION "3.7.5"

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
#line 1 "gabc/gabc-score-determination.y"

/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file implements the score parser.
 *
 * Gregorio score determination from gabc.
 * Copyright (C) 2006-2021 The Gregorio Project (see CONTRIBUTORS.md)
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

#line 733 "gabc/gabc-score-determination-y.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
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

#include "gabc-score-determination-y.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_NAME = 3,                       /* NAME  */
  YYSYMBOL_AUTHOR = 4,                     /* AUTHOR  */
  YYSYMBOL_GABC_COPYRIGHT = 5,             /* GABC_COPYRIGHT  */
  YYSYMBOL_SCORE_COPYRIGHT = 6,            /* SCORE_COPYRIGHT  */
  YYSYMBOL_LANGUAGE = 7,                   /* LANGUAGE  */
  YYSYMBOL_STAFF_LINES = 8,                /* STAFF_LINES  */
  YYSYMBOL_ORISCUS_ORIENTATION = 9,        /* ORISCUS_ORIENTATION  */
  YYSYMBOL_DEF_MACRO = 10,                 /* DEF_MACRO  */
  YYSYMBOL_OTHER_HEADER = 11,              /* OTHER_HEADER  */
  YYSYMBOL_ANNOTATION = 12,                /* ANNOTATION  */
  YYSYMBOL_MODE = 13,                      /* MODE  */
  YYSYMBOL_MODE_MODIFIER = 14,             /* MODE_MODIFIER  */
  YYSYMBOL_MODE_DIFFERENTIA = 15,          /* MODE_DIFFERENTIA  */
  YYSYMBOL_END_OF_DEFINITIONS = 16,        /* END_OF_DEFINITIONS  */
  YYSYMBOL_END_OF_FILE = 17,               /* END_OF_FILE  */
  YYSYMBOL_COLON = 18,                     /* COLON  */
  YYSYMBOL_SEMICOLON = 19,                 /* SEMICOLON  */
  YYSYMBOL_CHARACTERS = 20,                /* CHARACTERS  */
  YYSYMBOL_NOTES = 21,                     /* NOTES  */
  YYSYMBOL_HYPHEN = 22,                    /* HYPHEN  */
  YYSYMBOL_ATTRIBUTE = 23,                 /* ATTRIBUTE  */
  YYSYMBOL_OPENING_BRACKET = 24,           /* OPENING_BRACKET  */
  YYSYMBOL_CLOSING_BRACKET = 25,           /* CLOSING_BRACKET  */
  YYSYMBOL_CLOSING_BRACKET_WITH_SPACE = 26, /* CLOSING_BRACKET_WITH_SPACE  */
  YYSYMBOL_I_BEGIN = 27,                   /* I_BEGIN  */
  YYSYMBOL_I_END = 28,                     /* I_END  */
  YYSYMBOL_TT_BEGIN = 29,                  /* TT_BEGIN  */
  YYSYMBOL_TT_END = 30,                    /* TT_END  */
  YYSYMBOL_UL_BEGIN = 31,                  /* UL_BEGIN  */
  YYSYMBOL_UL_END = 32,                    /* UL_END  */
  YYSYMBOL_C_BEGIN = 33,                   /* C_BEGIN  */
  YYSYMBOL_C_END = 34,                     /* C_END  */
  YYSYMBOL_B_BEGIN = 35,                   /* B_BEGIN  */
  YYSYMBOL_B_END = 36,                     /* B_END  */
  YYSYMBOL_SC_BEGIN = 37,                  /* SC_BEGIN  */
  YYSYMBOL_SC_END = 38,                    /* SC_END  */
  YYSYMBOL_SP_BEGIN = 39,                  /* SP_BEGIN  */
  YYSYMBOL_SP_END = 40,                    /* SP_END  */
  YYSYMBOL_VERB_BEGIN = 41,                /* VERB_BEGIN  */
  YYSYMBOL_VERB_END = 42,                  /* VERB_END  */
  YYSYMBOL_CENTER_BEGIN = 43,              /* CENTER_BEGIN  */
  YYSYMBOL_CENTER_END = 44,                /* CENTER_END  */
  YYSYMBOL_ELISION_BEGIN = 45,             /* ELISION_BEGIN  */
  YYSYMBOL_ELISION_END = 46,               /* ELISION_END  */
  YYSYMBOL_TRANSLATION_BEGIN = 47,         /* TRANSLATION_BEGIN  */
  YYSYMBOL_TRANSLATION_END = 48,           /* TRANSLATION_END  */
  YYSYMBOL_TRANSLATION_CENTER_END = 49,    /* TRANSLATION_CENTER_END  */
  YYSYMBOL_ALT_BEGIN = 50,                 /* ALT_BEGIN  */
  YYSYMBOL_ALT_END = 51,                   /* ALT_END  */
  YYSYMBOL_NLBA_B = 52,                    /* NLBA_B  */
  YYSYMBOL_NLBA_E = 53,                    /* NLBA_E  */
  YYSYMBOL_EUOUAE_B = 54,                  /* EUOUAE_B  */
  YYSYMBOL_EUOUAE_E = 55,                  /* EUOUAE_E  */
  YYSYMBOL_NABC_CUT = 56,                  /* NABC_CUT  */
  YYSYMBOL_NABC_LINES = 57,                /* NABC_LINES  */
  YYSYMBOL_CLEAR = 58,                     /* CLEAR  */
  YYSYMBOL_PROTRUSION = 59,                /* PROTRUSION  */
  YYSYMBOL_PROTRUSION_VALUE = 60,          /* PROTRUSION_VALUE  */
  YYSYMBOL_PROTRUSION_END = 61,            /* PROTRUSION_END  */
  YYSYMBOL_PROTRUDING_PUNCTUATION = 62,    /* PROTRUDING_PUNCTUATION  */
  YYSYMBOL_YYACCEPT = 63,                  /* $accept  */
  YYSYMBOL_score = 64,                     /* score  */
  YYSYMBOL_all_definitions = 65,           /* all_definitions  */
  YYSYMBOL_definitions = 66,               /* definitions  */
  YYSYMBOL_attribute_value = 67,           /* attribute_value  */
  YYSYMBOL_attribute = 68,                 /* attribute  */
  YYSYMBOL_definition = 69,                /* definition  */
  YYSYMBOL_notes = 70,                     /* notes  */
  YYSYMBOL_note = 71,                      /* note  */
  YYSYMBOL_closing_bracket_with_space = 72, /* closing_bracket_with_space  */
  YYSYMBOL_style_beginning = 73,           /* style_beginning  */
  YYSYMBOL_style_end = 74,                 /* style_end  */
  YYSYMBOL_special_style_beginning = 75,   /* special_style_beginning  */
  YYSYMBOL_special_style_end = 76,         /* special_style_end  */
  YYSYMBOL_euouae = 77,                    /* euouae  */
  YYSYMBOL_linebreak_area = 78,            /* linebreak_area  */
  YYSYMBOL_protrusion = 79,                /* protrusion  */
  YYSYMBOL_character = 80,                 /* character  */
  YYSYMBOL_text = 81,                      /* text  */
  YYSYMBOL_translation_character = 82,     /* translation_character  */
  YYSYMBOL_translation_text = 83,          /* translation_text  */
  YYSYMBOL_translation_beginning = 84,     /* translation_beginning  */
  YYSYMBOL_translation = 85,               /* translation  */
  YYSYMBOL_above_line_text = 86,           /* above_line_text  */
  YYSYMBOL_syllable_with_notes = 87,       /* syllable_with_notes  */
  YYSYMBOL_notes_without_word = 88,        /* notes_without_word  */
  YYSYMBOL_syllable = 89,                  /* syllable  */
  YYSYMBOL_syllables = 90                  /* syllables  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
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

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
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
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
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

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   317


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
static const yytype_int16 yyrline[] =
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

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "NAME", "AUTHOR",
  "GABC_COPYRIGHT", "SCORE_COPYRIGHT", "LANGUAGE", "STAFF_LINES",
  "ORISCUS_ORIENTATION", "DEF_MACRO", "OTHER_HEADER", "ANNOTATION", "MODE",
  "MODE_MODIFIER", "MODE_DIFFERENTIA", "END_OF_DEFINITIONS", "END_OF_FILE",
  "COLON", "SEMICOLON", "CHARACTERS", "NOTES", "HYPHEN", "ATTRIBUTE",
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

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317
};
#endif

#define YYPACT_NINF (-85)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
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
static const yytype_int8 yydefact[] =
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
       0,     1,     2,     3,   108,    71,    21,    87,   114,   115,
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
static const yytype_int8 yystos[] =
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
static const yytype_int8 yyr1[] =
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
static const yytype_int8 yyr2[] =
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


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

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

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

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

# ifndef YY_LOCATION_PRINT
#  if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

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

#   define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

#  else
#   define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#  endif
# endif /* !defined YY_LOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YY_LOCATION_PRINT (yyo, *yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
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
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
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
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

/* User initialization code.  */
#line 655 "gabc/gabc-score-determination.y"
{
    yylloc.first_line = 1;
    yylloc.first_column = 0;
    yylloc.first_offset = 0;
    yylloc.last_line = 1;
    yylloc.last_column = 0;
    yylloc.last_offset = 0;
    yylloc.generate_point_and_click = generate_point_and_click;
}

#line 1858 "gabc/gabc-score-determination-y.c"

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
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
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
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (STYLE_BITS);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
  case 3: /* all_definitions: definitions END_OF_DEFINITIONS  */
#line 704 "gabc/gabc-score-determination.y"
                                   {
        end_definitions();
    }
#line 2072 "gabc/gabc-score-determination-y.c"
    break;

  case 6: /* attribute_value: ATTRIBUTE  */
#line 714 "gabc/gabc-score-determination.y"
              {
        yyval.text = yyvsp[0].text;
    }
#line 2080 "gabc/gabc-score-determination-y.c"
    break;

  case 7: /* attribute_value: attribute_value ATTRIBUTE  */
#line 717 "gabc/gabc-score-determination.y"
                                {
        yyval.text = concatenate(yyvsp[-1].text, yyvsp[0].text);
    }
#line 2088 "gabc/gabc-score-determination-y.c"
    break;

  case 8: /* attribute: COLON attribute_value SEMICOLON  */
#line 723 "gabc/gabc-score-determination.y"
                                    {
        yyval.text = yyvsp[-1].text;
    }
#line 2096 "gabc/gabc-score-determination-y.c"
    break;

  case 9: /* attribute: COLON SEMICOLON  */
#line 726 "gabc/gabc-score-determination.y"
                      {
        yyval.text = NULL;
    }
#line 2104 "gabc/gabc-score-determination-y.c"
    break;

  case 10: /* definition: DEF_MACRO attribute  */
#line 732 "gabc/gabc-score-determination.y"
                        {
        /* these definitions are not passed through */
        free(macros[yyvsp[-1].character - '0']);
        macros[yyvsp[-1].character - '0'] = yyvsp[0].text;
    }
#line 2114 "gabc/gabc-score-determination-y.c"
    break;

  case 11: /* definition: NAME attribute  */
#line 737 "gabc/gabc-score-determination.y"
                     {
        if (yyvsp[0].text == NULL) {
            gregorio_message("name can't be empty","det_score",
                    VERBOSITY_WARNING, 0);
        }
        check_multiple("name", score->name != NULL);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->name = yyvsp[0].text;
    }
#line 2128 "gabc/gabc-score-determination-y.c"
    break;

  case 12: /* definition: LANGUAGE attribute  */
#line 746 "gabc/gabc-score-determination.y"
                         {
        check_multiple("language", got_language);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        gregorio_set_centering_language(yyvsp[0].text);
        got_language = true;
    }
#line 2139 "gabc/gabc-score-determination-y.c"
    break;

  case 13: /* definition: GABC_COPYRIGHT attribute  */
#line 752 "gabc/gabc-score-determination.y"
                               {
        check_multiple("gabc-copyright", score->gabc_copyright != NULL);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->gabc_copyright = yyvsp[0].text;
    }
#line 2149 "gabc/gabc-score-determination-y.c"
    break;

  case 14: /* definition: SCORE_COPYRIGHT attribute  */
#line 757 "gabc/gabc-score-determination.y"
                                {
        check_multiple("score_copyright", score->score_copyright != NULL);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->score_copyright = yyvsp[0].text;
    }
#line 2159 "gabc/gabc-score-determination-y.c"
    break;

  case 15: /* definition: MODE attribute  */
#line 762 "gabc/gabc-score-determination.y"
                     {
        check_multiple("mode", score->mode != 0);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->mode = yyvsp[0].text;
    }
#line 2169 "gabc/gabc-score-determination-y.c"
    break;

  case 16: /* definition: MODE_MODIFIER attribute  */
#line 767 "gabc/gabc-score-determination.y"
                              {
        check_multiple("mode-modifier", score->mode_modifier != NULL);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->mode_modifier = yyvsp[0].text;
    }
#line 2179 "gabc/gabc-score-determination-y.c"
    break;

  case 17: /* definition: MODE_DIFFERENTIA attribute  */
#line 772 "gabc/gabc-score-determination.y"
                                 {
        check_multiple("mode-differentia", score->mode_differentia != NULL);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->mode_differentia = yyvsp[0].text;
    }
#line 2189 "gabc/gabc-score-determination-y.c"
    break;

  case 18: /* definition: STAFF_LINES attribute  */
#line 777 "gabc/gabc-score-determination.y"
                            {
        check_multiple("staff-lines", got_staff_lines);
        if (yyvsp[0].text) {
            gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
            gregorio_set_score_staff_lines(score, atoi(yyvsp[0].text));
            got_staff_lines = true;
        }
    }
#line 2202 "gabc/gabc-score-determination-y.c"
    break;

  case 19: /* definition: NABC_LINES attribute  */
#line 785 "gabc/gabc-score-determination.y"
                           {
        check_multiple("nabc lines", score->nabc_lines != 0);
        if (yyvsp[0].text) {
            gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
            nabc_lines=atoi(yyvsp[0].text);
            score->nabc_lines=nabc_lines;
        }
    }
#line 2215 "gabc/gabc-score-determination-y.c"
    break;

  case 20: /* definition: ANNOTATION attribute  */
#line 793 "gabc/gabc-score-determination.y"
                           {
        if (score->annotation [MAX_ANNOTATIONS - 1]) {
            gregorio_messagef("det_score", VERBOSITY_WARNING, 0,
                    _("too many definitions of annotation found, only the "
                    "first %d will be taken"), MAX_ANNOTATIONS);
        }
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        gregorio_set_score_annotation(score, yyvsp[0].text);
    }
#line 2229 "gabc/gabc-score-determination-y.c"
    break;

  case 21: /* definition: AUTHOR attribute  */
#line 802 "gabc/gabc-score-determination.y"
                       {
        check_multiple("author", score->author != NULL);
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->author = yyvsp[0].text;
    }
#line 2239 "gabc/gabc-score-determination-y.c"
    break;

  case 22: /* definition: ORISCUS_ORIENTATION attribute  */
#line 807 "gabc/gabc-score-determination.y"
                                    {
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
        score->legacy_oriscus_orientation = (strcmp(yyvsp[0].text, "legacy") == 0);
    }
#line 2248 "gabc/gabc-score-determination-y.c"
    break;

  case 23: /* definition: OTHER_HEADER attribute  */
#line 811 "gabc/gabc-score-determination.y"
                             {
        gregorio_add_score_header(score, yyvsp[-1].text, yyvsp[0].text);
    }
#line 2256 "gabc/gabc-score-determination-y.c"
    break;

  case 26: /* note: NOTES CLOSING_BRACKET  */
#line 821 "gabc/gabc-score-determination.y"
                          {
        gabc_y_add_notes(yyvsp[-1].text, (yylsp[-1]));
        free(yyvsp[-1].text);
        nabc_state=0;
    }
#line 2266 "gabc/gabc-score-determination-y.c"
    break;

  case 27: /* note: NOTES closing_bracket_with_space  */
#line 826 "gabc/gabc-score-determination.y"
                                       {
        gabc_y_add_notes(yyvsp[-1].text, (yylsp[-1]));
        free(yyvsp[-1].text);
        nabc_state=0;
        update_position_with_space();
    }
#line 2277 "gabc/gabc-score-determination-y.c"
    break;

  case 28: /* note: NOTES NABC_CUT  */
#line 832 "gabc/gabc-score-determination.y"
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
#line 2293 "gabc/gabc-score-determination-y.c"
    break;

  case 29: /* note: CLOSING_BRACKET  */
#line 843 "gabc/gabc-score-determination.y"
                      {
        elements[voice]=NULL;
        nabc_state=0;
    }
#line 2302 "gabc/gabc-score-determination-y.c"
    break;

  case 30: /* note: closing_bracket_with_space  */
#line 847 "gabc/gabc-score-determination.y"
                                 {
        elements[voice]=NULL;
        nabc_state=0;
        update_position_with_space();
    }
#line 2312 "gabc/gabc-score-determination-y.c"
    break;

  case 34: /* style_beginning: I_BEGIN  */
#line 861 "gabc/gabc-score-determination.y"
            {
        add_style(ST_ITALIC);
    }
#line 2320 "gabc/gabc-score-determination-y.c"
    break;

  case 35: /* style_beginning: TT_BEGIN  */
#line 864 "gabc/gabc-score-determination.y"
               {
        add_style(ST_TT);
    }
#line 2328 "gabc/gabc-score-determination-y.c"
    break;

  case 36: /* style_beginning: UL_BEGIN  */
#line 867 "gabc/gabc-score-determination.y"
               {
        add_style(ST_UNDERLINED);
    }
#line 2336 "gabc/gabc-score-determination-y.c"
    break;

  case 37: /* style_beginning: C_BEGIN  */
#line 870 "gabc/gabc-score-determination.y"
              {
        add_style(ST_COLORED);
    }
#line 2344 "gabc/gabc-score-determination-y.c"
    break;

  case 38: /* style_beginning: B_BEGIN  */
#line 873 "gabc/gabc-score-determination.y"
              {
        add_style(ST_BOLD);
    }
#line 2352 "gabc/gabc-score-determination-y.c"
    break;

  case 39: /* style_beginning: SC_BEGIN  */
#line 876 "gabc/gabc-score-determination.y"
               {
        add_style(ST_SMALL_CAPS);
    }
#line 2360 "gabc/gabc-score-determination-y.c"
    break;

  case 40: /* style_beginning: VERB_BEGIN  */
#line 879 "gabc/gabc-score-determination.y"
                 {
        add_style(ST_VERBATIM);
    }
#line 2368 "gabc/gabc-score-determination-y.c"
    break;

  case 41: /* style_beginning: SP_BEGIN  */
#line 882 "gabc/gabc-score-determination.y"
               {
        add_style(ST_SPECIAL_CHAR);
    }
#line 2376 "gabc/gabc-score-determination-y.c"
    break;

  case 42: /* style_end: I_END  */
#line 888 "gabc/gabc-score-determination.y"
          {
        end_style(ST_ITALIC);
    }
#line 2384 "gabc/gabc-score-determination-y.c"
    break;

  case 43: /* style_end: TT_END  */
#line 891 "gabc/gabc-score-determination.y"
             {
        end_style(ST_TT);
    }
#line 2392 "gabc/gabc-score-determination-y.c"
    break;

  case 44: /* style_end: UL_END  */
#line 894 "gabc/gabc-score-determination.y"
             {
        end_style(ST_UNDERLINED);
    }
#line 2400 "gabc/gabc-score-determination-y.c"
    break;

  case 45: /* style_end: C_END  */
#line 897 "gabc/gabc-score-determination.y"
            {
        end_style(ST_COLORED);
    }
#line 2408 "gabc/gabc-score-determination-y.c"
    break;

  case 46: /* style_end: B_END  */
#line 900 "gabc/gabc-score-determination.y"
            {
        end_style(ST_BOLD);
    }
#line 2416 "gabc/gabc-score-determination-y.c"
    break;

  case 47: /* style_end: SC_END  */
#line 903 "gabc/gabc-score-determination.y"
             {
        end_style(ST_SMALL_CAPS);
    }
#line 2424 "gabc/gabc-score-determination-y.c"
    break;

  case 48: /* style_end: VERB_END  */
#line 906 "gabc/gabc-score-determination.y"
               {
        end_style(ST_VERBATIM);
    }
#line 2432 "gabc/gabc-score-determination-y.c"
    break;

  case 49: /* style_end: SP_END  */
#line 909 "gabc/gabc-score-determination.y"
             {
        end_style(ST_SPECIAL_CHAR);
    }
#line 2440 "gabc/gabc-score-determination-y.c"
    break;

  case 50: /* special_style_beginning: ELISION_BEGIN  */
#line 915 "gabc/gabc-score-determination.y"
                  {
        add_style(ST_ELISION);
    }
#line 2448 "gabc/gabc-score-determination-y.c"
    break;

  case 51: /* special_style_beginning: CENTER_BEGIN  */
#line 918 "gabc/gabc-score-determination.y"
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
#line 2467 "gabc/gabc-score-determination-y.c"
    break;

  case 52: /* special_style_end: ELISION_END  */
#line 935 "gabc/gabc-score-determination.y"
                {
        end_style(ST_ELISION);
    }
#line 2475 "gabc/gabc-score-determination-y.c"
    break;

  case 53: /* special_style_end: CENTER_END  */
#line 938 "gabc/gabc-score-determination.y"
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
#line 2490 "gabc/gabc-score-determination-y.c"
    break;

  case 54: /* euouae: EUOUAE_B  */
#line 951 "gabc/gabc-score-determination.y"
             {
        euouae = EUOUAE_BEGINNING;
    }
#line 2498 "gabc/gabc-score-determination-y.c"
    break;

  case 55: /* euouae: EUOUAE_E  */
#line 954 "gabc/gabc-score-determination.y"
               {
        euouae = EUOUAE_END;
    }
#line 2506 "gabc/gabc-score-determination-y.c"
    break;

  case 56: /* linebreak_area: NLBA_B  */
#line 960 "gabc/gabc-score-determination.y"
           {
        no_linebreak_area = NLBA_BEGINNING;
    }
#line 2514 "gabc/gabc-score-determination-y.c"
    break;

  case 57: /* linebreak_area: NLBA_E  */
#line 963 "gabc/gabc-score-determination.y"
             {
        no_linebreak_area = NLBA_END;
    }
#line 2522 "gabc/gabc-score-determination-y.c"
    break;

  case 58: /* protrusion: PROTRUSION PROTRUSION_VALUE PROTRUSION_END  */
#line 969 "gabc/gabc-score-determination.y"
                                               {
        add_protrusion(yyvsp[-1].text);
    }
#line 2530 "gabc/gabc-score-determination-y.c"
    break;

  case 59: /* protrusion: PROTRUSION  */
#line 972 "gabc/gabc-score-determination.y"
                 {
        add_protrusion(gregorio_strdup("d")); /* d = default */
    }
#line 2538 "gabc/gabc-score-determination-y.c"
    break;

  case 61: /* character: CHARACTERS  */
#line 979 "gabc/gabc-score-determination.y"
                 {
        add_text(yyvsp[0].text);
    }
#line 2546 "gabc/gabc-score-determination-y.c"
    break;

  case 68: /* character: CLEAR  */
#line 988 "gabc/gabc-score-determination.y"
            {
        clear_syllable_text = true;
    }
#line 2554 "gabc/gabc-score-determination-y.c"
    break;

  case 70: /* character: HYPHEN  */
#line 992 "gabc/gabc-score-determination.y"
             {
        add_text(gregorio_strdup("-"));
    }
#line 2562 "gabc/gabc-score-determination-y.c"
    break;

  case 71: /* character: PROTRUDING_PUNCTUATION  */
#line 995 "gabc/gabc-score-determination.y"
                             {
        add_text(yyvsp[0].text);
    }
#line 2570 "gabc/gabc-score-determination-y.c"
    break;

  case 74: /* translation_character: CHARACTERS  */
#line 1006 "gabc/gabc-score-determination.y"
               {
        add_text(yyvsp[0].text);
    }
#line 2578 "gabc/gabc-score-determination-y.c"
    break;

  case 77: /* translation_character: HYPHEN  */
#line 1011 "gabc/gabc-score-determination.y"
             {
        add_text(gregorio_strdup("-"));
    }
#line 2586 "gabc/gabc-score-determination-y.c"
    break;

  case 78: /* translation_character: PROTRUDING_PUNCTUATION  */
#line 1014 "gabc/gabc-score-determination.y"
                             {
        add_text(yyvsp[0].text);
    }
#line 2594 "gabc/gabc-score-determination-y.c"
    break;

  case 81: /* translation_beginning: TRANSLATION_BEGIN  */
#line 1025 "gabc/gabc-score-determination.y"
                      {
        start_translation(TR_NORMAL);
    }
#line 2602 "gabc/gabc-score-determination-y.c"
    break;

  case 82: /* translation: translation_beginning TRANSLATION_END  */
#line 1031 "gabc/gabc-score-determination.y"
                                          {
        end_translation();
    }
#line 2610 "gabc/gabc-score-determination-y.c"
    break;

  case 83: /* translation: translation_beginning translation_text TRANSLATION_END  */
#line 1034 "gabc/gabc-score-determination.y"
                                                             {
        end_translation();
    }
#line 2618 "gabc/gabc-score-determination-y.c"
    break;

  case 84: /* translation: TRANSLATION_CENTER_END  */
#line 1037 "gabc/gabc-score-determination.y"
                             {
        start_translation(TR_WITH_CENTER_END);
        end_translation();
    }
#line 2627 "gabc/gabc-score-determination-y.c"
    break;

  case 85: /* above_line_text: ALT_BEGIN CHARACTERS ALT_END  */
#line 1044 "gabc/gabc-score-determination.y"
                                 {
        abovelinestext = yyvsp[-1].text;
    }
#line 2635 "gabc/gabc-score-determination-y.c"
    break;

  case 86: /* syllable_with_notes: text OPENING_BRACKET notes  */
#line 1050 "gabc/gabc-score-determination.y"
                               {
        save_text();
        close_syllable(&(yylsp[-2]));
    }
#line 2644 "gabc/gabc-score-determination-y.c"
    break;

  case 87: /* syllable_with_notes: HYPHEN OPENING_BRACKET notes  */
#line 1054 "gabc/gabc-score-determination.y"
                                   {
        add_style(ST_VERBATIM);
        add_text(gregorio_strdup("\\GreForceHyphen"));
        end_style(ST_VERBATIM);
        save_text();
        close_syllable(&(yylsp[-2]));
    }
#line 2656 "gabc/gabc-score-determination-y.c"
    break;

  case 88: /* syllable_with_notes: text HYPHEN OPENING_BRACKET notes  */
#line 1061 "gabc/gabc-score-determination.y"
                                        {
        add_style(ST_VERBATIM);
        add_text(gregorio_strdup("\\GreForceHyphen"));
        end_style(ST_VERBATIM);
        save_text();
        close_syllable(&(yylsp[-3]));
    }
#line 2668 "gabc/gabc-score-determination-y.c"
    break;

  case 89: /* syllable_with_notes: PROTRUDING_PUNCTUATION OPENING_BRACKET notes  */
#line 1068 "gabc/gabc-score-determination.y"
                                                   {
        add_auto_protrusion(yyvsp[-2].text);
        save_text();
        close_syllable(&(yylsp[-2]));
    }
#line 2678 "gabc/gabc-score-determination-y.c"
    break;

  case 90: /* syllable_with_notes: text PROTRUDING_PUNCTUATION OPENING_BRACKET notes  */
#line 1073 "gabc/gabc-score-determination.y"
                                                        {
        add_auto_protrusion(yyvsp[-2].text);
        save_text();
        close_syllable(&(yylsp[-3]));
    }
#line 2688 "gabc/gabc-score-determination-y.c"
    break;

  case 91: /* syllable_with_notes: text translation OPENING_BRACKET notes  */
#line 1078 "gabc/gabc-score-determination.y"
                                             {
        save_text();
        close_syllable(&(yylsp[-3]));
    }
#line 2697 "gabc/gabc-score-determination-y.c"
    break;

  case 92: /* syllable_with_notes: HYPHEN translation OPENING_BRACKET notes  */
#line 1082 "gabc/gabc-score-determination.y"
                                               {
        add_style(ST_VERBATIM);
        add_text(gregorio_strdup("\\GreForceHyphen"));
        end_style(ST_VERBATIM);
        save_text();
        close_syllable(&(yylsp[-3]));
    }
#line 2709 "gabc/gabc-score-determination-y.c"
    break;

  case 93: /* syllable_with_notes: text HYPHEN translation OPENING_BRACKET notes  */
#line 1089 "gabc/gabc-score-determination.y"
                                                    {
        add_style(ST_VERBATIM);
        add_text(gregorio_strdup("\\GreForceHyphen"));
        end_style(ST_VERBATIM);
        save_text();
        close_syllable(&(yylsp[-4]));
    }
#line 2721 "gabc/gabc-score-determination-y.c"
    break;

  case 94: /* syllable_with_notes: PROTRUDING_PUNCTUATION translation OPENING_BRACKET notes  */
#line 1096 "gabc/gabc-score-determination.y"
                                                               {
        add_auto_protrusion(yyvsp[-3].text);
        save_text();
        close_syllable(&(yylsp[-3]));
    }
#line 2731 "gabc/gabc-score-determination-y.c"
    break;

  case 95: /* syllable_with_notes: text PROTRUDING_PUNCTUATION translation OPENING_BRACKET notes  */
#line 1101 "gabc/gabc-score-determination.y"
                                                                    {
        add_auto_protrusion(yyvsp[-3].text);
        save_text();
        close_syllable(&(yylsp[-4]));
    }
#line 2741 "gabc/gabc-score-determination-y.c"
    break;

  case 96: /* notes_without_word: OPENING_BRACKET notes  */
#line 1109 "gabc/gabc-score-determination.y"
                          {
        close_syllable(NULL);
    }
#line 2749 "gabc/gabc-score-determination-y.c"
    break;

  case 97: /* notes_without_word: translation OPENING_BRACKET notes  */
#line 1112 "gabc/gabc-score-determination.y"
                                        {
        close_syllable(NULL);
    }
#line 2757 "gabc/gabc-score-determination-y.c"
    break;


#line 2761 "gabc/gabc-score-determination-y.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

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
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

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


#if !defined yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;
#endif


/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

