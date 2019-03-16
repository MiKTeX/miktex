/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file implements vowel rule handling (aside from parsing).
 *
 * Copyright (C) 2015-2019 The Gregorio Project (see CONTRIBUTORS.md)
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
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "bool.h"
#include "vowel.h"
#include "unicode.h"
#include "messages.h"
#include "support.h"

typedef struct character_set {
    grewchar *table;
    struct character_set **next;
    grewchar mask;
    unsigned int bins;
    unsigned int size;
    bool is_final;
} character_set;

static character_set *character_set_new(const bool alloc_next)
{
    character_set *set =
        (character_set *)gregorio_calloc(1, sizeof(character_set));
    set->mask = 0x0f;
    set->bins = 16;
    set->size = 0;
    set->table = (grewchar *)gregorio_calloc(set->bins, sizeof(grewchar));
    if (alloc_next) {
        set->next = (character_set **)gregorio_calloc(set->bins,
                sizeof(character_set *));
    }
    return set;
}

static void character_set_free(character_set *const set);

static __inline void character_set_next_elements_free(character_set *const set)
{
    unsigned int i;
    assert(set && set->next);
    for (i = 0; i < set->bins; ++i) {
        if (set->next[i]) {
            character_set_free(set->next[i]);
        }
    }
}

static void character_set_free(character_set *const set)
{
    if (set) {
        if (set->next) {
            character_set_next_elements_free(set);
            free(set->next);
        }
        free(set->table);
        free(set);
    }
}

static bool character_set_contains(character_set *const set,
        const grewchar character, character_set **const next)
{
    unsigned int index;
    assert(set);

    if (next) {
        *next = NULL;
    }
    for (index = ((unsigned long)character) & set->mask; set->table[index];
            index = (index + 1) & set->mask) {
        if (set->table[index] == character) {
            if (next && set->next) {
                *next = set->next[index];
            }
            return true;
        }
    }
    return false;
}

static __inline void character_set_put(character_set *const set,
        const grewchar character, character_set *next)
{
    unsigned int index;

    assert(set);

    index = ((unsigned long)character) & set->mask;
    while (set->table[index]) {
        index = (index + 1) & set->mask;
    }

    set->table[index] = character;
    if (set->next) {
        set->next[index] = next;
    }
}

static __inline void character_set_grow(character_set *const set) {
    static grewchar *old_table;
    static character_set **old_next;
    unsigned int old_bins, i;

    assert(set);

    if (set->bins >= 0x4000L) {
        /* this should not realistically be reached */
        /* LCOV_EXCL_START */
        gregorio_message(_("character set too large"), "character_set_grow",
                VERBOSITY_FATAL, 0);
        return;
        /* LCOV_EXCL_STOP */
    }

    old_table = set->table;
    old_next = set->next;
    old_bins = set->bins;

    set->bins <<= 1;
    set->mask = (set->mask << 1) | 0x01;
    set->table = gregorio_calloc(set->bins, sizeof(grewchar));
    if (old_next) {
        set->next = gregorio_calloc(set->bins, sizeof(character_set *));
    }
    for (i = 0; i < old_bins; ++i) {
        if (old_table[i]) {
            character_set_put(set, old_table[i], old_next ? old_next[i] : NULL);
        } else {
            assert(!old_next || !old_next[i]);
        }
    }

    free(old_table);
    if (old_next) {
        free(old_next);
    }
}

static character_set *character_set_add(character_set *const set,
        const grewchar character)
{
    character_set *next = NULL;

    if (!character_set_contains(set, character, &next)) {
        if (((++(set->size) * 10) / set->bins) >= 7) {
            character_set_grow(set);
        }

        if (set->next) {
            next = character_set_new(true);
        }

        character_set_put(set, character, next);
    }

    return next;
}

static __inline void character_set_clear(character_set *const set)
{
    if (set) {
        memset(set->table, 0, set->bins * sizeof(grewchar));

        if (set->next) {
            character_set_next_elements_free(set);
            memset(set->next, 0, set->bins * sizeof(character_set *));
        }

        set->size = 0;
    }
}

static character_set *vowel_table = NULL;
static character_set *prefix_table = NULL;
static character_set *suffix_table = NULL;
static character_set *secondary_table = NULL;
static grewchar *prefix_buffer = NULL;
static size_t prefix_buffer_size;
static size_t prefix_buffer_mask;

extern FILE *gregorio_vowel_rulefile_in;

static void prefix_buffer_grow(size_t required_size)
{
    while (required_size > prefix_buffer_size) {
        prefix_buffer_size <<= 1;
        prefix_buffer_mask = (prefix_buffer_mask << 1) | 0x01;
    }
    prefix_buffer = gregorio_realloc(prefix_buffer,
            prefix_buffer_size * sizeof(grewchar));
}

void gregorio_vowel_tables_init(void)
{
    if (vowel_table) {
        character_set_clear(vowel_table);
        character_set_clear(prefix_table);
        character_set_clear(suffix_table);
        character_set_clear(secondary_table);
    } else {
        vowel_table = character_set_new(false);
        prefix_table = character_set_new(true);
        suffix_table = character_set_new(true);
        secondary_table = character_set_new(true);

        prefix_buffer_size = 16;
        prefix_buffer_mask = 0x0f;
        prefix_buffer = (grewchar *)gregorio_malloc(
                prefix_buffer_size * sizeof(grewchar));
    }
}

void gregorio_vowel_tables_load(const char *const filename,
        char **const language, rulefile_parse_status *status)
{
    gregorio_check_file_access(read, filename, WARNING, return);
    gregorio_vowel_rulefile_in = fopen(filename, "r");
    if (gregorio_vowel_rulefile_in) {
        gregorio_vowel_rulefile_parse(filename, language, status);
        fclose(gregorio_vowel_rulefile_in);
        gregorio_vowel_rulefile_in = NULL;
    } else {
        /* tests cannot be expected to simulate the system error that would
         * cause gregorio_vowel_rulefile_in to be NULL */
        /* LCOV_EXCL_START */
        gregorio_messagef("gregorio_vowel_tables_load", VERBOSITY_WARNING, 0,
                _("unable to open %s: %s"), filename, strerror(errno));
        /* LCOV_EXCL_STOP */
    }
}

void gregorio_vowel_tables_free(void)
{
    if (vowel_table) {
        character_set_free(vowel_table);
    }
    if (prefix_table) {
        character_set_free(prefix_table);
    }
    if (suffix_table) {
        character_set_free(suffix_table);
    }
    if (secondary_table) {
        character_set_free(secondary_table);
    }
    if (prefix_buffer) {
        free(prefix_buffer);
    }
}

void gregorio_vowel_table_add(const char *vowels)
{
    if (vowels) {
        grewchar *str = gregorio_build_grewchar_string_from_buf(vowels), *p;
        for (p = str; *p; ++p) {
            character_set_add(vowel_table, *p);
        }
        free(str);
    }
}

void gregorio_prefix_table_add(const char *prefix)
{
    character_set *set = prefix_table;
    grewchar *str, *p;

    /* store prefixes backwards */
    if (prefix && *prefix) {
        str = gregorio_build_grewchar_string_from_buf(prefix);
        p = str;

        while (*(++p)) ;

        while (p > str) {
            set = character_set_add(set, *(--p));
        }

        set->is_final = true;
        prefix_buffer_grow(gregorio_wcstrlen(str));
        free(str);
    }
}

void gregorio_suffix_table_add(const char *suffix)
{
    character_set *set = suffix_table;
    grewchar *str, *p;

    if (suffix && *suffix) {
        str = gregorio_build_grewchar_string_from_buf(suffix);
        p = str;

        while (*p) {
            set = character_set_add(set, *(p++));
        }

        set->is_final = true;
        free(str);
    }
}

void gregorio_secondary_table_add(const char *secondary)
{
    character_set *set = secondary_table;
    grewchar *str, *p;

    if (secondary && *secondary) {
        str = gregorio_build_grewchar_string_from_buf(secondary);
        p = str;

        while (*p) {
            set = character_set_add(set, *(p++));
        }

        set->is_final = true;
        free(str);
    }
}

typedef enum {
    VWL_BEFORE = 0,
    VWL_WITHIN,
    VWL_SUFFIX
} vowel_group_state;

static __inline bool is_in_prefix(size_t bufpos) {
    character_set *previous = prefix_table, *prefix;

    while (character_set_contains(previous, prefix_buffer[bufpos], &prefix)) {
        bufpos = (bufpos + prefix_buffer_size - 1) & prefix_buffer_mask;
        previous = prefix;
    }

    return previous->is_final;
}

bool gregorio_find_vowel_group(const grewchar *const string, int *const start,
        int *const end)
{
    size_t bufpos = 0;
    vowel_group_state state = VWL_BEFORE;
    character_set *cset = NULL;
    const grewchar *subject;
    int i;

    /* stick the 0 in here to avoid overruns from is_in_prefix */
    prefix_buffer[0] = 0;

    for (i = 0, subject = string; true; ++i, ++subject) {
        switch (state) {
        case VWL_BEFORE:
            bufpos = (bufpos + 1) & prefix_buffer_mask;
            prefix_buffer[bufpos] = *subject;
            if (character_set_contains(vowel_table, *subject, NULL)) {
                /* we found a vowel */
                if ((!character_set_contains(vowel_table, *(subject + 1), NULL)
                            && *(subject + 1) != GREVOWEL_ELISION_MARK)
                        || !is_in_prefix(bufpos)) {
                    /* no vowel/elision after, or not in prefix, so this is the
                     * start */
                    *start = i;
                    state = VWL_WITHIN;
                }
            }
            break;

        case VWL_WITHIN:
            if (!character_set_contains(vowel_table, *subject, NULL)) {
                /* not a vowel; is it a suffix? */
                *end = i;
                if (character_set_contains(suffix_table, *subject, &cset)) {
                    /* it's the start of a suffix */
                    state = VWL_SUFFIX;
                } else {
                    /* neither a vowel nor a suffix, so this is the end */
                    return true;
                }
            }
            break;

        case VWL_SUFFIX:
            if (cset->is_final) {
                /* remember the last final position */
                *end = i;
            }
            if (!character_set_contains(cset, *subject, &cset)) {
                /* no longer in valid suffix */
                return true;
            }
            break;

        default:
            /* not reachable unless there's a programming error */
            /* LCOV_EXCL_START */
            assert(false);
            break;
            /* LCOV_EXCL_STOP */
        }

        if (!*subject) {
            break;
        }
    }

    /* no vowel found; look for a secondary */
    *end = -1;
    for (*start = 0; *(subject = string + *start); ++ *start) {
        for (cset = secondary_table, i = *start;
                character_set_contains(cset, *subject, &cset); ++i, ++subject) {
            if (cset->is_final) {
                /* remember the last final position */
                *end = i + 1;
            }
        }
        if (*end >= 0) {
            return true;
        }
    }

    /* no center found */
    *start = -1;
    return false;
}
