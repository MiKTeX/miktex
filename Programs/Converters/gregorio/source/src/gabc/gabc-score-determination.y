%{
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
%}

%initial-action {
    @$.first_line = 1;
    @$.first_column = 0;
    @$.first_offset = 0;
    @$.last_line = 1;
    @$.last_column = 0;
    @$.last_offset = 0;
    @$.generate_point_and_click = generate_point_and_click;
}

%lex-param { gabc_style_bits *STYLE_BITS }

%token NAME AUTHOR GABC_COPYRIGHT SCORE_COPYRIGHT
%token LANGUAGE STAFF_LINES ORISCUS_ORIENTATION
%token DEF_MACRO OTHER_HEADER
%token ANNOTATION MODE MODE_MODIFIER MODE_DIFFERENTIA
%token END_OF_DEFINITIONS END_OF_FILE
%token COLON SEMICOLON CHARACTERS NOTES HYPHEN ATTRIBUTE
%token OPENING_BRACKET CLOSING_BRACKET CLOSING_BRACKET_WITH_SPACE
%token I_BEGIN I_END
%token TT_BEGIN TT_END
%token UL_BEGIN UL_END
%token C_BEGIN C_END
%token B_BEGIN B_END
%token SC_BEGIN SC_END
%token SP_BEGIN SP_END
%token VERB_BEGIN VERB_END
%token CENTER_BEGIN CENTER_END
%token ELISION_BEGIN ELISION_END
%token TRANSLATION_BEGIN TRANSLATION_END TRANSLATION_CENTER_END
%token ALT_BEGIN ALT_END
%token NLBA_B NLBA_E
%token EUOUAE_B EUOUAE_E
%token NABC_CUT NABC_LINES
%token CLEAR
%token PROTRUSION PROTRUSION_VALUE PROTRUSION_END PROTRUDING_PUNCTUATION

%precedence HYPHEN PROTRUDING_PUNCTUATION
%precedence TRANSLATION_BEGIN TRANSLATION_CENTER_END
%precedence TRANSLATION_END
%precedence OPENING_BRACKET

%%

score:
    all_definitions syllables
    ;

all_definitions:
    definitions END_OF_DEFINITIONS {
        end_definitions();
    }
    ;

definitions:
    | definitions definition
    ;

attribute_value:
    ATTRIBUTE {
        $$.text = $1.text;
    }
    | attribute_value ATTRIBUTE {
        $$.text = concatenate($1.text, $2.text);
    }
    ;

attribute:
    COLON attribute_value SEMICOLON {
        $$.text = $2.text;
    }
    | COLON SEMICOLON {
        $$.text = NULL;
    }
    ;

definition:
    DEF_MACRO attribute {
        /* these definitions are not passed through */
        free(macros[$1.character - '0']);
        macros[$1.character - '0'] = $2.text;
    }
    | NAME attribute {
        if ($2.text == NULL) {
            gregorio_message("name can't be empty","det_score",
                    VERBOSITY_WARNING, 0);
        }
        check_multiple("name", score->name != NULL);
        gregorio_add_score_header(score, $1.text, $2.text);
        score->name = $2.text;
    }
    | LANGUAGE attribute {
        check_multiple("language", got_language);
        gregorio_add_score_header(score, $1.text, $2.text);
        gregorio_set_centering_language($2.text);
        got_language = true;
    }
    | GABC_COPYRIGHT attribute {
        check_multiple("gabc-copyright", score->gabc_copyright != NULL);
        gregorio_add_score_header(score, $1.text, $2.text);
        score->gabc_copyright = $2.text;
    }
    | SCORE_COPYRIGHT attribute {
        check_multiple("score_copyright", score->score_copyright != NULL);
        gregorio_add_score_header(score, $1.text, $2.text);
        score->score_copyright = $2.text;
    }
    | MODE attribute {
        check_multiple("mode", score->mode != 0);
        gregorio_add_score_header(score, $1.text, $2.text);
        score->mode = $2.text;
    }
    | MODE_MODIFIER attribute {
        check_multiple("mode-modifier", score->mode_modifier != NULL);
        gregorio_add_score_header(score, $1.text, $2.text);
        score->mode_modifier = $2.text;
    }
    | MODE_DIFFERENTIA attribute {
        check_multiple("mode-differentia", score->mode_differentia != NULL);
        gregorio_add_score_header(score, $1.text, $2.text);
        score->mode_differentia = $2.text;
    }
    | STAFF_LINES attribute {
        check_multiple("staff-lines", got_staff_lines);
        if ($2.text) {
            gregorio_add_score_header(score, $1.text, $2.text);
            gregorio_set_score_staff_lines(score, atoi($2.text));
            got_staff_lines = true;
        }
    }
    | NABC_LINES attribute {
        check_multiple("nabc lines", score->nabc_lines != 0);
        if ($2.text) {
            gregorio_add_score_header(score, $1.text, $2.text);
            nabc_lines=atoi($2.text);
            score->nabc_lines=nabc_lines;
        }
    }
    | ANNOTATION attribute {
        if (score->annotation [MAX_ANNOTATIONS - 1]) {
            gregorio_messagef("det_score", VERBOSITY_WARNING, 0,
                    _("too many definitions of annotation found, only the "
                    "first %d will be taken"), MAX_ANNOTATIONS);
        }
        gregorio_add_score_header(score, $1.text, $2.text);
        gregorio_set_score_annotation(score, $2.text);
    }
    | AUTHOR attribute {
        check_multiple("author", score->author != NULL);
        gregorio_add_score_header(score, $1.text, $2.text);
        score->author = $2.text;
    }
    | ORISCUS_ORIENTATION attribute {
        gregorio_add_score_header(score, $1.text, $2.text);
        score->legacy_oriscus_orientation = (strcmp($2.text, "legacy") == 0);
    }
    | OTHER_HEADER attribute {
        gregorio_add_score_header(score, $1.text, $2.text);
    }
    ;

notes:
    | notes note
    ;

note:
    NOTES CLOSING_BRACKET {
        gabc_y_add_notes($1.text, @1);
        free($1.text);
        nabc_state=0;
    }
    | NOTES closing_bracket_with_space {
        gabc_y_add_notes($1.text, @1);
        free($1.text);
        nabc_state=0;
        update_position_with_space();
    }
    | NOTES NABC_CUT {
        if (!nabc_lines) {
            gregorio_message(_("You used character \"|\" in gabc without "
                               "setting \"nabc-lines\" parameter. Please "
                               "set it in your gabc header."),
                             "det_score", VERBOSITY_FATAL, 0);
        }
        gabc_y_add_notes($1.text, @1);
        free($1.text);
        nabc_state = (nabc_state + 1) % (nabc_lines+1);
    }
    | CLOSING_BRACKET {
        elements[voice]=NULL;
        nabc_state=0;
    }
    | closing_bracket_with_space {
        elements[voice]=NULL;
        nabc_state=0;
        update_position_with_space();
    }
    ;

closing_bracket_with_space:
    CLOSING_BRACKET_WITH_SPACE
    | CLOSING_BRACKET_WITH_SPACE END_OF_FILE
    | CLOSING_BRACKET END_OF_FILE
    ;

style_beginning:
    I_BEGIN {
        add_style(ST_ITALIC);
    }
    | TT_BEGIN {
        add_style(ST_TT);
    }
    | UL_BEGIN {
        add_style(ST_UNDERLINED);
    }
    | C_BEGIN {
        add_style(ST_COLORED);
    }
    | B_BEGIN {
        add_style(ST_BOLD);
    }
    | SC_BEGIN {
        add_style(ST_SMALL_CAPS);
    }
    | VERB_BEGIN {
        add_style(ST_VERBATIM);
    }
    | SP_BEGIN {
        add_style(ST_SPECIAL_CHAR);
    }
    ;

style_end:
    I_END {
        end_style(ST_ITALIC);
    }
    | TT_END {
        end_style(ST_TT);
    }
    | UL_END {
        end_style(ST_UNDERLINED);
    }
    | C_END {
        end_style(ST_COLORED);
    }
    | B_END {
        end_style(ST_BOLD);
    }
    | SC_END {
        end_style(ST_SMALL_CAPS);
    }
    | VERB_END {
        end_style(ST_VERBATIM);
    }
    | SP_END {
        end_style(ST_SPECIAL_CHAR);
    }
    ;

special_style_beginning:
    ELISION_BEGIN {
        add_style(ST_ELISION);
    }
    | CENTER_BEGIN {
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
    ;

special_style_end:
    ELISION_END {
        end_style(ST_ELISION);
    }
    | CENTER_END {
        if (center_is_determined == CENTER_HALF_DETERMINED) {
            end_style(ST_FORCED_CENTER);
            center_is_determined = CENTER_FULLY_DETERMINED;
        } else {
            gregorio_message(
                    "not within a syllable center",
                    "det_score", VERBOSITY_WARNING, 0);
        }
    }
    ;

euouae:
    EUOUAE_B {
        euouae = EUOUAE_BEGINNING;
    }
    | EUOUAE_E {
        euouae = EUOUAE_END;
    }
    ;

linebreak_area:
    NLBA_B {
        no_linebreak_area = NLBA_BEGINNING;
    }
    | NLBA_E {
        no_linebreak_area = NLBA_END;
    }
    ;

protrusion:
    PROTRUSION PROTRUSION_VALUE PROTRUSION_END {
        add_protrusion($2.text);
    }
    | PROTRUSION {
        add_protrusion(gregorio_strdup("d")); /* d = default */
    }
    ;

character:
    above_line_text
    | CHARACTERS {
        add_text($1.text);
    }
    | style_beginning
    | style_end
    | special_style_beginning
    | special_style_end
    | linebreak_area
    | euouae
    | CLEAR {
        clear_syllable_text = true;
    }
    | protrusion
    | HYPHEN {
        add_text(gregorio_strdup("-"));
    }
    | PROTRUDING_PUNCTUATION {
        add_text($1.text);
    }
    ;

text:
    character
    | text character
    ;

translation_character:
    CHARACTERS {
        add_text($1.text);
    }
    | style_beginning
    | style_end
    | HYPHEN {
        add_text(gregorio_strdup("-"));
    }
    | PROTRUDING_PUNCTUATION {
        add_text($1.text);
    }
    ;

translation_text:
    translation_character
    | translation_text translation_character
    ;

translation_beginning:
    TRANSLATION_BEGIN {
        start_translation(TR_NORMAL);
    }
    ;

translation:
    translation_beginning TRANSLATION_END {
        end_translation();
    }
    | translation_beginning translation_text TRANSLATION_END {
        end_translation();
    }
    | TRANSLATION_CENTER_END {
        start_translation(TR_WITH_CENTER_END);
        end_translation();
    }
    ;

above_line_text:
    ALT_BEGIN CHARACTERS ALT_END {
        abovelinestext = $2.text;
    }
    ;

syllable_with_notes:
    text OPENING_BRACKET notes {
        save_text();
        close_syllable(&@1);
    }
    | HYPHEN OPENING_BRACKET notes {
        add_style(ST_VERBATIM);
        add_text(gregorio_strdup("\\GreForceHyphen"));
        end_style(ST_VERBATIM);
        save_text();
        close_syllable(&@1);
    }
    | text HYPHEN OPENING_BRACKET notes {
        add_style(ST_VERBATIM);
        add_text(gregorio_strdup("\\GreForceHyphen"));
        end_style(ST_VERBATIM);
        save_text();
        close_syllable(&@1);
    }
    | PROTRUDING_PUNCTUATION OPENING_BRACKET notes {
        add_auto_protrusion($1.text);
        save_text();
        close_syllable(&@1);
    }
    | text PROTRUDING_PUNCTUATION OPENING_BRACKET notes {
        add_auto_protrusion($2.text);
        save_text();
        close_syllable(&@1);
    }
    | text translation OPENING_BRACKET notes {
        save_text();
        close_syllable(&@1);
    }
    | HYPHEN translation OPENING_BRACKET notes {
        add_style(ST_VERBATIM);
        add_text(gregorio_strdup("\\GreForceHyphen"));
        end_style(ST_VERBATIM);
        save_text();
        close_syllable(&@1);
    }
    | text HYPHEN translation OPENING_BRACKET notes {
        add_style(ST_VERBATIM);
        add_text(gregorio_strdup("\\GreForceHyphen"));
        end_style(ST_VERBATIM);
        save_text();
        close_syllable(&@1);
    }
    | PROTRUDING_PUNCTUATION translation OPENING_BRACKET notes {
        add_auto_protrusion($1.text);
        save_text();
        close_syllable(&@1);
    }
    | text PROTRUDING_PUNCTUATION translation OPENING_BRACKET notes {
        add_auto_protrusion($2.text);
        save_text();
        close_syllable(&@1);
    }
    ;

notes_without_word:
    OPENING_BRACKET notes {
        close_syllable(NULL);
    }
    | translation OPENING_BRACKET notes {
        close_syllable(NULL);
    }
    ;

syllable:
    syllable_with_notes
    | notes_without_word
    ;

syllables:
    | syllables syllable
    ;
