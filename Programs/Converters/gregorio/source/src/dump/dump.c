/*
 * Gregorio is a program that translates gabc files to GregorioTeX.
 * This file provides functions to dump out Gregorio structures.
 *
 * Copyright (C) 2007-2019 The Gregorio Project (see CONTRIBUTORS.md)
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
#include "bool.h"
#include "struct.h"
#include "unicode.h"
#include "messages.h"
#include "plugins.h"
#include "support.h"

static const char *dump_style_to_string(grestyle_style style)
{
    static char buf[50];

    gregorio_snprintf(buf, sizeof buf, "%16s", grestyle_style_to_string(style));
    return buf;
}

void dump_write_characters(FILE *const f,
        const gregorio_character * current_character)
{
    while (current_character) {
        fprintf(f,
                "---------------------------------------------------------------------\n");
        if (current_character->is_character) {
            fprintf(f, "     character                 ");
            gregorio_print_unichar(f, current_character->cos.character);
            fprintf(f, "\n");
        } else {
            switch (current_character->cos.s.type) {
            case ST_T_BEGIN:
                fprintf(f, "     beginning of style   %s\n",
                        dump_style_to_string(current_character->cos.s.style));
                break;
            case ST_T_END:
                fprintf(f, "     end of style         %s\n",
                        dump_style_to_string(current_character->cos.s.style));
                break;
            default:
                /* not reachable unless there's a programming error */
                /* LCOV_EXCL_START */
                fprintf(f, "     !! IMPROPER STYLE !! %s\n",
                        dump_style_to_string(current_character->cos.s.style));
                break;
                /* LCOV_EXCL_STOP */
            }
        }
        current_character = current_character->next_character;
    }
}

static const char *dump_bool(bool value) {
    return value? "true" : "false";
}

static const char *dump_pitch(const char height, const char highest_pitch) {
    static char buf[20];
    if (height >= LOWEST_PITCH && height <= highest_pitch) {
        char pitch = height + 'a' - LOWEST_PITCH;
        if (pitch == 'o') {
            pitch = 'p';
        }
        gregorio_snprintf(buf, 20, "%c", pitch);
    } else {
        /* not reachable unless there's a programming error */
        gregorio_snprintf(buf, 20, "?%d", height); /* LCOV_EXCL_LINE */
    }
    return buf;
}

static void dump_hepisema_adjustment(FILE *const f, const gregorio_note *note,
        const gregorio_sign_orientation orientation, const char *const which) {
    unsigned short index = note->he_adjustment_index[orientation];
    if (index) {
        gregorio_hepisema_adjustment *adj = gregorio_get_hepisema_adjustment(
                index);
        fprintf(f, "         %s hepisema group   %d\n", which, index);
        fprintf(f, "         %s hep. vbasepos    %d (%s)\n", which,
                adj->vbasepos,
                gregorio_hepisema_vbasepos_to_string(adj->vbasepos));
        if (adj->nudge) {
            fprintf(f, "         %s hepisema nudge   %s\n", which, adj->nudge);
        }
    }
}

void dump_write_score(FILE *f, gregorio_score *score)
{
    gregorio_voice_info *voice_info = score->first_voice_info;
    int i;
    int annotation_num;
    gregorio_syllable *syllable;
    gregorio_header *header;

    gregorio_assert(f, dump_write_score, "call with NULL file", return);

    fprintf(f,
            "=====================================================================\n"
            " SCORE INFOS\n"
            "=====================================================================\n");
    if (score->number_of_voices) {
        fprintf(f, "   number_of_voices          %d\n",
                score->number_of_voices);
    }
    if (score->name) {
        fprintf(f, "   name                      %s\n", score->name);
    }
    if (score->gabc_copyright) {
        fprintf(f, "   gabc_copyright            %s\n", score->gabc_copyright);
    }
    if (score->score_copyright) {
        fprintf(f, "   score_copyright           %s\n", score->score_copyright);
    }
    if (score->author) {
        fprintf(f, "   author                    %s\n", score->author);
    }
    if (score->mode) {
        fprintf(f, "   mode                      %s\n", score->mode);
    }
    if (score->mode_modifier) {
        fprintf(f, "   mode_modifier             %s\n", score->mode_modifier);
    }
    if (score->mode_differentia) {
        fprintf(f, "   mode_differentia          %s\n", score->mode_differentia);
    }
    if (score->staff_lines != 4) {
        fprintf(f, "   staff_lines               %d\n", (int)score->staff_lines);
    }
    if (score->nabc_lines) {
        fprintf(f, "   nabc_lines                %d\n", (int)score->nabc_lines);
    }
    if (score->legacy_oriscus_orientation) {
        fprintf(f, "   oriscus-orientation       legacy\n");
    }
    fprintf(f, "\n\n"
            "=====================================================================\n"
            " HEADERS\n"
            "=====================================================================\n");
    for (header = score->headers; header; header = header->next) {
        fprintf(f, "   %-25s %s\n", header->name, header->value);
    }
    fprintf(f, "\n\n"
            "=====================================================================\n"
            " VOICES INFOS\n"
            "=====================================================================\n");
    for (i = 0; i < score->number_of_voices; i++) {
        fprintf(f, "  Voice %d\n", i + 1);
        if (voice_info->initial_clef.line) {
            fprintf(f, "   initial_key               %d (%c%d)\n",
                    gregorio_calculate_new_key(voice_info->initial_clef),
                    gregorio_clef_to_char(voice_info->initial_clef.clef),
                    voice_info->initial_clef.line);
            if (voice_info->initial_clef.flatted) {
                fprintf(f, "   flatted_key               true\n");
            }
            if (voice_info->initial_clef.secondary_line) {
                fprintf(f, "     secondary_clef          %c%d\n",
                        gregorio_clef_to_char(
                            voice_info->initial_clef.secondary_clef),
                        voice_info->initial_clef.secondary_line);
                if (voice_info->initial_clef.secondary_flatted) {
                    fprintf(f, "     secondary_flatted_key   true\n");
                }
            }
        }
        for (annotation_num = 0; annotation_num < MAX_ANNOTATIONS;
             ++annotation_num) {
            if (score->annotation[annotation_num]) {
                fprintf(f, "   annotation                %s\n",
                        score->annotation[annotation_num]);
            }
        }
        voice_info = voice_info->next_voice_info;
    }
    fprintf(f, "\n\n"
            "=====================================================================\n"
            " SCORE\n"
            "=====================================================================\n");
    for (syllable = score->first_syllable; syllable;
            syllable = syllable->next_syllable) {
        gregorio_element *element;
        fprintf(f, "   type                      0 (GRE_SYLLABLE)\n");
        if (syllable->position) {
            fprintf(f, "   position                  %d (%s)\n",
                    syllable->position,
                    gregorio_word_position_to_string(syllable->position));
        }
        if (syllable->no_linebreak_area != NLBA_NORMAL) {
            fprintf(f, "   no line break area        %s\n",
                    gregorio_nlba_to_string(syllable->no_linebreak_area));
        }
        if (syllable->euouae != EUOUAE_NORMAL) {
            fprintf(f, "   euouae                    %s\n",
                    gregorio_euouae_to_string(syllable->euouae));
        }
        if (syllable->clear) {
            fprintf(f, "   clear                     true\n");
        }
        if (syllable->text) {
            if (syllable->translation) {
                fprintf(f, "\n  Text\n");
            }
            dump_write_characters(f, syllable->text);
        }
        if ((syllable->translation
             && syllable->translation_type != TR_WITH_CENTER_END)
            || syllable->translation_type == TR_WITH_CENTER_END) {
            fprintf(f, "\n  Translation type             %s",
                    gregorio_tr_centering_to_string
                    (syllable->translation_type));
            if (syllable->translation_type == TR_WITH_CENTER_END) {
                fprintf(f, "\n");
            }
        }
        if (syllable->translation) {
            fprintf(f, "\n  Translation\n");
            dump_write_characters(f, syllable->translation);
        }
        if (syllable->abovelinestext) {
            fprintf(f, "\n  Abovelinestext\n    %s", syllable->abovelinestext);
        }
        for (element = *syllable->elements; element; element = element->next) {
            gregorio_glyph *glyph;
            fprintf(f, "---------------------------------------------------------------------\n");
            if (element->type) {
                fprintf(f, "     type                    %d (%s)\n",
                        element->type, gregorio_type_to_string(element->type));
            }
            switch (element->type) {
            case GRE_CUSTOS:
                if (element->u.misc.pitched.pitch) {
                    fprintf(f, "     pitch                   %s\n",
                            dump_pitch(element->u.misc.pitched.pitch,
                                score->highest_pitch));
                }
                if (element->u.misc.pitched.force_pitch) {
                    fprintf(f, "     force_pitch             true\n");
                }
                break;
            case GRE_SUPPRESS_CUSTOS:
                /* not handling this would generate an error below */
                break;
            case GRE_SPACE:
                if (element->u.misc.unpitched.info.space) {
                    char *factor = element->u.misc.unpitched.info.
                            ad_hoc_space_factor;
                    fprintf(f, "     space                   %d (%s)%s%s\n",
                            element->u.misc.unpitched.info.space,
                            gregorio_space_to_string(
                                element->u.misc.unpitched.info.space),
                            factor? " x " : "", factor? factor : "");
                }
                break;
            case GRE_TEXVERB_ELEMENT:
                fprintf(f, "     TeX string              \"%s\"\n",
                        gregorio_texverb(element->texverb));
                break;
            case GRE_NLBA:
                fprintf(f, "     nlba                    %d (%s)\n",
                        element->u.misc.unpitched.info.nlba,
                        gregorio_nlba_to_string(element->u.misc.unpitched.info.
                            nlba));
                break;
            case GRE_ALT:
                fprintf(f, "     Above lines text        \"%s\"\n",
                        gregorio_texverb(element->texverb));
                break;
            case GRE_BAR:
                if (element->u.misc.unpitched.info.bar) {
                    fprintf(f, "     bar                     %d (%s)\n",
                            element->u.misc.unpitched.info.bar,
                            gregorio_bar_to_string(element->u.misc.unpitched.
                                                   info.bar));
                    if (element->u.misc.unpitched.special_sign) {
                        fprintf(f, "     special sign            %d (%s)\n",
                                element->u.misc.unpitched.special_sign,
                                gregorio_sign_to_string(element->
                                        u.misc.unpitched.special_sign));
                    }
                }
                break;
            case GRE_CLEF:
                if (element->u.misc.clef.line) {
                    fprintf(f, "     clef                    %d (%c%d)\n",
                            gregorio_calculate_new_key(element->u.misc.clef),
                            gregorio_clef_to_char(element->u.misc.clef.clef),
                            element->u.misc.clef.line);
                    if (element->u.misc.clef.flatted) {
                        fprintf(f, "     flatted_key             true\n");
                    }
                    if (element->u.misc.clef.secondary_line) {
                        fprintf(f, "     secondary_clef          %c%d\n",
                                gregorio_clef_to_char(
                                    element->u.misc.clef.secondary_clef),
                                element->u.misc.clef.secondary_line);
                        if (element->u.misc.clef.secondary_flatted) {
                            fprintf(f, "     secondary_flatted_key   true\n");
                        }
                    }
                }
                break;
            case GRE_END_OF_LINE:
                if (element->u.misc.unpitched.info.eol_ragged) {
                    fprintf(f, "         ragged                 true\n");
                }
                if (element->u.misc.unpitched.info.eol_forces_custos) {
                    fprintf(f, "         forces custos          %s\n",
                            dump_bool(element
                                ->u.misc.unpitched.info.eol_forces_custos_on));
                }
                break;
            case GRE_ELEMENT:
                for (glyph = element->u.first_glyph; glyph;
                        glyph = glyph->next) {
                    gregorio_note *note;
                    fprintf(f, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
                    if (glyph->type) {
                        fprintf(f, "       type                  %d (%s)\n",
                                glyph->type, gregorio_type_to_string(glyph->
                                                                     type));
                    }
                    switch (glyph->type) {
                    case GRE_TEXVERB_GLYPH:
                        fprintf(f, "       TeX string            \"%s\"\n",
                                gregorio_texverb(glyph->texverb));
                        break;

                    case GRE_SPACE:
                        fprintf(f, "       space                 %d (%s)\n",
                                glyph->u.misc.unpitched.info.space,
                                gregorio_space_to_string(glyph->u.misc.
                                                         unpitched.info.space));
                        break;

                    case GRE_GLYPH:
                        fprintf(f, "       glyph_type            %d (%s)\n",
                                glyph->u.notes.glyph_type,
                                gregorio_glyph_type_to_string(glyph->u.notes.
                                                              glyph_type));
                        if (glyph->u.notes.is_cavum) {
                            fprintf(f, "       is_cavum              true\n");
                        }
                        if (glyph->u.notes.liquescentia) {
                            fprintf(f, "       liquescentia          %d (%s)\n",
                                    glyph->u.notes.liquescentia,
                                    gregorio_liquescentia_to_string(
                                            glyph->u.notes.liquescentia));
                        }
                        break;

                    default:
                        /* not reachable unless there's a programming error */
                        /* LCOV_EXCL_START */
                        fprintf(f, "         !!! NOT ALLOWED !!!    %s\n",
                                gregorio_type_to_string(glyph->type));
                        break;
                        /* LCOV_EXCL_STOP */
                    }
                    if (glyph->type == GRE_GLYPH) {
                        for (note = glyph->u.notes.first_note; note;
                                note = note->next) {
                            fprintf(f, "-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  \n");
                            if (note->type) {
                                fprintf(f, "         type                   %d (%s)\n",
                                        note->type,
                                        gregorio_type_to_string(note->type));
                            }
                            switch (note->type) {
                            case GRE_NOTE:
                                if (note->u.note.pitch) {
                                    fprintf(f, "         pitch                  %s\n",
                                            dump_pitch(note->u.note.pitch,
                                                score->highest_pitch));
                                }
                                if (note->u.note.shape) {
                                    fprintf(f, "         shape                  %d (%s)\n",
                                            note->u.note.shape,
                                            gregorio_shape_to_string(
                                                    note->u.note.shape));
                                }
                                if (note->u.note.is_cavum) {
                                    fprintf(f, "         is_cavum               true\n");
                                }
                                if (note->u.note.liquescentia) {
                                    fprintf(f, "         liquescentia           %d (%s)\n",
                                            note->u.note.liquescentia,
                                            gregorio_liquescentia_to_string(
                                                    note->u.note.liquescentia));
                                }
                                break;

                            default:
                                /* not reachable unless there's a programming error */
                                /* LCOV_EXCL_START */
                                fprintf(f, "         !!! NOT ALLOWED !!!    %s\n",
                                        gregorio_type_to_string(note->type));
                                break;
                                /* LCOV_EXCL_STOP */
                            }
                            if (note->texverb) {
                                fprintf(f, "         TeX string             \"%s\"\n",
                                        gregorio_texverb(note->texverb));
                            }
                            if (note->choral_sign) {
                                fprintf(f, "         Choral Sign            \"%s\"\n",
                                        note->choral_sign);
                            }
                            if (note->signs) {
                                fprintf(f, "         signs                  %d (%s)\n",
                                        note->signs,
                                        gregorio_sign_to_string(note->signs));
                            }
                            if (note->signs & _V_EPISEMA && note->v_episema_height) {
                                if (note->v_episema_height < note->u.note.pitch) {
                                    fprintf(f, "         v episema forced       BELOW\n");
                                }
                                else {
                                    fprintf(f, "         v episema forced       ABOVE\n");
                                }
                            }
                            if ((note->signs == _PUNCTUM_MORA
                                        || note->signs == _V_EPISEMA_PUNCTUM_MORA)
                                    && note->mora_vposition) {
                                fprintf(f, "         mora vposition         %s\n",
                                        gregorio_vposition_to_string(note->
                                                mora_vposition));
                            }
                            if (note->special_sign) {
                                fprintf(f, "         special sign           %d (%s)\n",
                                        note->special_sign,
                                        gregorio_sign_to_string(
                                                note->special_sign));
                            }
                            if (note->h_episema_above == HEPISEMA_AUTO
                                    && note->h_episema_below == HEPISEMA_AUTO) {
                                fprintf(f, "         auto hepisema size     %d (%s)\n",
                                        note->h_episema_above_size,
                                        grehepisema_size_to_string(note->
                                                h_episema_above_size));
                                fprintf(f, "         auto hepisema bridge   %s\n",
                                        dump_bool(note->h_episema_above_connect));
                            }
                            else {
                                if (note->h_episema_above == HEPISEMA_FORCED) {
                                    fprintf(f, "         above hepisema size    %d (%s)\n",
                                            note->h_episema_above_size,
                                            grehepisema_size_to_string(note->
                                                    h_episema_above_size));
                                    fprintf(f, "         above hepisema bridge  %s\n",
                                            dump_bool(note->h_episema_above_connect));
                                }
                                if (note->h_episema_below == HEPISEMA_FORCED) {
                                    fprintf(f, "         below hepisema size    %d (%s)\n",
                                            note->h_episema_below_size,
                                            grehepisema_size_to_string(note->
                                                    h_episema_below_size));
                                    fprintf(f, "         below hepisema bridge  %s\n",
                                            dump_bool(note->h_episema_below_connect));
                                }
                            }
                            dump_hepisema_adjustment(f, note, SO_OVER, "above");
                            dump_hepisema_adjustment(f, note, SO_UNDER,
                                    "below");
                            switch (note->high_ledger_specificity) {
                            case LEDGER_SUPPOSED:
                                break;
                            case LEDGER_EXPLICIT:
                                fprintf(f, "         explicit high line     %s\n",
                                        dump_bool(note->high_ledger_line));
                                break;
                            case LEDGER_DRAWN:
                                fprintf(f, "         drawn high line        %s\n",
                                        dump_bool(note->high_ledger_line));
                                break;
                            case LEDGER_EXPLICITLY_DRAWN:
                                fprintf(f, "         forced drawn high line %s\n",
                                        dump_bool(note->high_ledger_line));
                                break;
                            }
                            switch (note->low_ledger_specificity) {
                            case LEDGER_SUPPOSED:
                                break;
                            case LEDGER_EXPLICIT:
                                fprintf(f, "         explicit low line      %s\n",
                                        dump_bool(note->low_ledger_line));
                                break;
                            case LEDGER_DRAWN:
                                fprintf(f, "         drawn low line         %s\n",
                                        dump_bool(note->low_ledger_line));
                                break;
                            case LEDGER_EXPLICITLY_DRAWN:
                                fprintf(f, "         forced drawn low line  %s\n",
                                        dump_bool(note->low_ledger_line));
                                break;
                            }
                        }
                    }
                }
                break;

            default:
                /* not reachable unless there's a programming error */
                /* LCOV_EXCL_START */
                fprintf(f, "         !!! NOT ALLOWED !!!    %s\n",
                        gregorio_type_to_string(element->type));
                break;
                /* LCOV_EXCL_STOP */
            }
            if (element->nabc_lines) {
                fprintf(f, "     nabc_lines              %d\n",
                        (int)element->nabc_lines);
            }
            if (element->nabc_lines && element->nabc) {
                for (i = 0; i < (int)element->nabc_lines; i++) {
                    if (element->nabc[i]) {
                        fprintf(f, "     nabc_line %d             \"%s\"\n",
                                (int)(i+1), element->nabc[i]);
                    }
                }
            }
        }
        fprintf(f, "=====================================================================\n");
    }
}
