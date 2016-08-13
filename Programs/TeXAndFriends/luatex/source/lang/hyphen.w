% hyphen.w
%
% Libhnj is dual licensed under LGPL and MPL. Boilerplate for both
% licenses follows.
%
%
% LibHnj - a library for high quality hyphenation and justification
% Copyright (C) 1998 Raph Levien,
% 	     (C) 2001 ALTLinux, Moscow (http://www.alt-linux.org),
%           (C) 2001 Peter Novodvorsky (nidd@@cs.msu.su)
%
% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% version 2 of the License, or (at your option) any later version.
%
% This library is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% Library General Public License for more details.
%
% You should have received a copy of the GNU Library General Public
% License along with this library; if not, write to the
% Free Software Foundation, Inc., 59 Temple Place - Suite 330,
% Boston, MA  02111-1307  USA.
%
%
%
% The contents of this file are subject to the Mozilla Public License
% Version 1.0 (the "MPL"); you may not use this file except in
% compliance with the MPL.  You may obtain a copy of the MPL at
% http://www.mozilla.org/MPL/
%
% Software distributed under the MPL is distributed on an "AS IS" basis,
% WITHOUT WARRANTY OF ANY KIND, either express or implied. See the MPL
% for the specific language governing rights and limitations under the
% MPL.


@ @c


#include "ptexlib.h"
#include "lua/luatex-api.h"

#include <stdlib.h>             /* for NULL, malloc */
#include <stdio.h>              /* for fprintf */
#include <string.h>             /* for strdup */
#include <stdlib.h>             /* for malloc used by substring inclusion */

#define MAXPATHS 40960

#ifdef UNX
#  include <unistd.h>           /* for exit */
#endif

#include <kpathsea/c-ctype.h>

#define noVERBOSE

#include "lang/hnjalloc.h"

@ TODO: should be moved to separate library

@c
static unsigned char *hnj_strdup(const unsigned char *s)
{
    unsigned char *new;
    size_t l;

    l = strlen((const char *) s);
    new = hnj_malloc((int) l + 1);
    memcpy(new, s, l);
    new[l] = 0;
    return new;
}

@* Type definitions.

@ a little bit of a hash table implementation. This simply maps strings
   to state numbers

@c
typedef struct _HashTab HashTab;
typedef struct _HashEntry HashEntry;
typedef struct _HashIter HashIter;
typedef union _HashVal HashVal;

/* A cheap, but effective, hack. */
#define HASH_SIZE 31627

struct _HashTab {
    HashEntry *entries[HASH_SIZE];
};

union _HashVal {
    int state;
    char *hyppat;
};

struct _HashEntry {
    HashEntry *next;
    unsigned char *key;
    HashVal u;
};

struct _HashIter {
    HashEntry **e;
    HashEntry *cur;
    int ndx;
};

@ State machine

@c
typedef struct _HyphenState HyphenState;
typedef struct _HyphenTrans HyphenTrans;
#define MAX_CHARS 256
#define MAX_NAME 20

struct _HyphenDict {
    int num_states;
    int pat_length;
    char cset[MAX_NAME];
    HyphenState *states;
    HashTab *patterns;
    HashTab *merged;
    HashTab *state_num;
};

struct _HyphenState {
    char *match;
    /*char *repl; */
    /*signed char replindex; */
    /*signed char replcut; */
    int fallback_state;
    int num_trans;
    HyphenTrans *trans;
};

struct _HyphenTrans {
    int uni_ch;
    int new_state;
};


@ Combine two right-aligned number patterns, 04000 + 020 becomes 04020

@c
static char *combine(char *expr, const char *subexpr)
{
    size_t l1 = strlen(expr);
    size_t l2 = strlen(subexpr);
    size_t off = l1 - l2;
    unsigned j;
    /* this works also for utf8 sequences because the substring is identical
     to the last substring-length bytes of expr except for the (single byte)
     hyphenation encoders
     */
    for (j = 0; j < l2; j++) {
        if (expr[off + j] < subexpr[j])
            expr[off + j] = subexpr[j];
    }
    return expr;
}


@ ORIGINAL CODE
@c
static HashIter *new_HashIter(HashTab * h)
{
    HashIter *i = hnj_malloc(sizeof(HashIter));
    i->e = h->entries;
    i->cur = NULL;
    i->ndx = -1;
    return i;
}


static int nextHashStealPattern(HashIter * i, unsigned char **word, char **pattern)
{
    while (i->cur == NULL) {
        if (i->ndx >= HASH_SIZE - 1)
            return 0;
        i->cur = i->e[++i->ndx];
    }
    *word = i->cur->key;
    *pattern = i->cur->u.hyppat;
    i->cur->u.hyppat = NULL;
    i->cur = i->cur->next;
    return 1;
}


static int nextHash(HashIter * i, unsigned char **word)
{
    while (i->cur == NULL) {
        if (i->ndx >= HASH_SIZE - 1)
            return 0;
        i->cur = i->e[++i->ndx];
    }
    *word = i->cur->key;
    i->cur = i->cur->next;
    return 1;
}


static int eachHash(HashIter * i, unsigned char **word, char **pattern)
{
    while (i->cur == NULL) {
        if (i->ndx >= HASH_SIZE - 1)
            return 0;
        i->cur = i->e[++i->ndx];
    }
    *word = i->cur->key;
    *pattern = i->cur->u.hyppat;
    i->cur = i->cur->next;
    return 1;
}


static void delete_HashIter(HashIter * i)
{
    hnj_free(i);
}


@ a |char*| hash function from ASU - adapted from Gtk+

@c
static unsigned int hnj_string_hash(const unsigned char *s)
{
    const unsigned char *p;
    unsigned int h = 0, g;

    for (p = s; *p != '\0'; p += 1) {
        h = (h << 4) + *p;
        if ((g = (h & 0xf0000000))) {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }
    return h /* \% M */ ;
}


@ assumes that key is not already present!

@c
static void state_insert(HashTab * hashtab, unsigned char *key, int state)
{
    int i;
    HashEntry *e;

    i = (int) (hnj_string_hash(key) % HASH_SIZE);
    e = hnj_malloc(sizeof(HashEntry));
    e->next = hashtab->entries[i];
    e->key = key;
    e->u.state = state;
    hashtab->entries[i] = e;
}


@ assumes that key is not already present!

@c
static void hyppat_insert(HashTab * hashtab, unsigned char *key, char *hyppat)
{
    int i;
    HashEntry *e;

    i = (int) (hnj_string_hash(key) % HASH_SIZE);
    for (e = hashtab->entries[i]; e; e = e->next) {
        if (strcmp((char *) e->key, (char *) key) == 0) {
            if (e->u.hyppat) {
                if (hyppat
                    && strcmp((char *) e->u.hyppat, (char *) hyppat) != 0) {
                    print_err("Conflicting pattern ignored");
                    error();
                }
                hnj_free(e->u.hyppat);
            }
            e->u.hyppat = hyppat;
            hnj_free(key);
            return;
        }
    }
    e = hnj_malloc(sizeof(HashEntry));
    e->next = hashtab->entries[i];
    e->key = key;
    e->u.hyppat = hyppat;
    hashtab->entries[i] = e;
}


@ return state if found, otherwise $-1$

@c
static int state_lookup(HashTab * hashtab, const unsigned char *key)
{
    int i;
    HashEntry *e;

    i = (int) (hnj_string_hash(key) % HASH_SIZE);
    for (e = hashtab->entries[i]; e; e = e->next) {
        if (!strcmp((const char *) key, (const char *) e->key)) {
            return e->u.state;
        }
    }
    return -1;
}


@ return state if found, otherwise $-1$

@c
static char *hyppat_lookup(HashTab * hashtab, const unsigned char *chars, int l)
{
    int i;
    HashEntry *e;
    unsigned char key[256];     /* should be ample */
    strncpy((char *) key, (const char *) chars, (size_t) l);
    key[l] = 0;
    i = (int) (hnj_string_hash(key) % HASH_SIZE);
    for (e = hashtab->entries[i]; e; e = e->next) {
        if (!strcmp((char *) key, (char *) e->key)) {
            return e->u.hyppat;
        }
    }
    return NULL;
}


@ Get the state number, allocating a new state if necessary.

@c
static int hnj_get_state(HyphenDict * dict,
                         const unsigned char *str, int *state_num)
{
    *state_num = state_lookup(dict->state_num, str);

    if (*state_num >= 0)
        return *state_num;

    state_insert(dict->state_num, hnj_strdup(str), dict->num_states);
    /* predicate is true if |dict->num_states| is a power of two */
    if (!(dict->num_states & (dict->num_states - 1))) {
        dict->states = hnj_realloc(dict->states,
                                   (int) ((dict->num_states << 1) *
                                          (int) sizeof(HyphenState)));
    }
    dict->states[dict->num_states].match = NULL;
    dict->states[dict->num_states].fallback_state = -1;
    dict->states[dict->num_states].num_trans = 0;
    dict->states[dict->num_states].trans = NULL;
    return dict->num_states++;
}


@ Add a transition from state1 to state2 through ch - assumes that the
   transition does not already exist

@c
static void hnj_add_trans(HyphenDict * dict, int state1, int state2, int uni_ch)
{
    int num_trans;
    /* TH: this test was a bit too strict, it is quite normal for old
       patterns to have chars in the range 0-31 or 127-159 (inclusive).
       To ease the transition, let's only disallow NUL for now
       (this is probably a requirement of the code anyway).
     */
    if (uni_ch == 0) {
        char errmsg[256]; /* temp hack ... we will have a formatted error */
        snprintf(errmsg, 255, "character out of bounds: u%04x", uni_ch);
        errmsg[255] = '\0';
        normal_error("hyphenation",errmsg); /* todo */
    }
    num_trans = dict->states[state1].num_trans;
    if (num_trans == 0) {
        dict->states[state1].trans = hnj_malloc(sizeof(HyphenTrans));
    } else {
        /* TH: The old version did
           } else if (!(num_trans & (num_trans - 1))) {
             ... hnj_realloc(dict->states[state1].trans,
                                                 (int) ((num_trans << 1) *
                                                        sizeof(HyphenTrans)));
           but that is incredibly nasty when adding patters one-at-a-time.
           Controlled growth would be nicer than the current +1, but if
           noone complains, this is good enough ;)
         */
        dict->states[state1].trans = hnj_realloc(dict->states[state1].trans,
                                                 (int) ((num_trans + 1) *
                                                        sizeof(HyphenTrans)));
    }
    dict->states[state1].trans[num_trans].uni_ch = uni_ch;
    dict->states[state1].trans[num_trans].new_state = state2;
    dict->states[state1].num_trans++;
}


#ifdef VERBOSE

static unsigned char *get_state_str(int state)
{
    int i;
    HashEntry *e;

    for (i = 0; i < HASH_SIZE; i++)
        for (e = global->entries[i]; e; e = e->next)
            if (e->u.state == state)
                return e->key;
    return NULL;
}
#endif


@ I've changed the semantics a bit here: |hnj_hyphen_load| used to
   operate on a file, but now the argument is a string buffer.

@c
static const unsigned char *next_pattern(size_t * length,
                                         const unsigned char **buf)
{
    const unsigned char *here, *rover = *buf;
    while (*rover && isspace(*rover))
        rover++;
    here = rover;
    while (*rover) {
        if (isspace(*rover)) {
            *length = (size_t) (rover - here);
            *buf = rover;
            return here;
        }
        rover++;
    }
    *length = (size_t) (rover - here);
    *buf = rover;
    return *length ? here : NULL;       /* zero sensed */
}

static void init_hash(HashTab ** h)
{
    int i;
    if (*h)
        return;
    *h = hnj_malloc(sizeof(HashTab));
    for (i = 0; i < HASH_SIZE; i++)
        (*h)->entries[i] = NULL;
}


static void clear_state_hash(HashTab ** h)
{
    int i;
    if (*h == NULL)
        return;
    for (i = 0; i < HASH_SIZE; i++) {
        HashEntry *e, *next;
        for (e = (*h)->entries[i]; e; e = next) {
            next = e->next;
            hnj_free(e->key);
            hnj_free(e);
        }
    }
    hnj_free(*h);
    *h = NULL;
}


static void clear_hyppat_hash(HashTab ** h)
{
    int i;
    if (*h == NULL)
        return;
    for (i = 0; i < HASH_SIZE; i++) {
        HashEntry *e, *next;
        for (e = (*h)->entries[i]; e; e = next) {
            next = e->next;
            hnj_free(e->key);
            if (e->u.hyppat)
                hnj_free(e->u.hyppat);
            hnj_free(e);
        }
    }
    hnj_free(*h);
    *h = NULL;
}


static void init_dict(HyphenDict * dict)
{
    dict->num_states = 1;
    dict->pat_length = 0;
    dict->states = hnj_malloc(sizeof(HyphenState));
    dict->states[0].match = NULL;
    dict->states[0].fallback_state = -1;
    dict->states[0].num_trans = 0;
    dict->states[0].trans = NULL;
    dict->patterns = NULL;
    dict->merged = NULL;
    dict->state_num = NULL;
    init_hash(&dict->patterns);
}


static void clear_dict(HyphenDict * dict)
{
    int state_num;
    for (state_num = 0; state_num < dict->num_states; state_num++) {
        HyphenState *hstate = &dict->states[state_num];
        if (hstate->match)
            hnj_free(hstate->match);
        if (hstate->trans)
            hnj_free(hstate->trans);
    }
    hnj_free(dict->states);
    clear_hyppat_hash(&dict->patterns);
    clear_hyppat_hash(&dict->merged);
    clear_state_hash(&dict->state_num);
}



HyphenDict *hnj_hyphen_new(void)
{
    HyphenDict *dict = hnj_malloc(sizeof(HyphenDict));
    init_dict(dict);
    return dict;
}


void hnj_hyphen_clear(HyphenDict * dict)
{
    clear_dict(dict);
    init_dict(dict);
}


void hnj_hyphen_free(HyphenDict * dict)
{
    clear_dict(dict);
    hnj_free(dict);
}

unsigned char *hnj_serialize(HyphenDict * dict)
{
    HashIter *v;
    unsigned char *word;
    char *pattern;
    unsigned char *buf = hnj_malloc(dict->pat_length);
    unsigned char *cur = buf;
    v = new_HashIter(dict->patterns);
    while (eachHash(v, &word, &pattern)) {
        int i = 0, e = 0;
        while (word[e + i]) {
            if (pattern[i] != '0')
                *cur++ = (unsigned char) pattern[i];
            *cur++ = word[e + i++];
            while (is_utf8_follow(word[e + i]))
                *cur++ = word[i + e++];
        }
        if (pattern[i] != '0')
            *cur++ = (unsigned char) pattern[i];
        *cur++ = ' ';
    }
    delete_HashIter(v);
    *cur = 0;
    return buf;
}


void hnj_free_serialize(unsigned char *c)
{
    hnj_free(c);
}


@ hyphenation pattern:

signed bytes

0 indicates end (actually any negative number)

: prio(1+),startpos,length,len1,[replace],len2,[replace]

most basic example is:

p n 0 0 0

for a hyphenation point between characters


@c
void hnj_hyphen_load(HyphenDict * dict, const unsigned char *f)
{
    int state_num, last_state;
    int ch;
    int found;
    HashEntry *e;
    HashIter *v;
    unsigned char *word;
    char *pattern;
    size_t l = 0;

    const unsigned char *format;
    const unsigned char *begin = f;
    unsigned char *pat;
    char *org;
    while ((format = next_pattern(&l, &f)) != NULL) {
        int i, j, e1;
        if (l>=255) {
           help1("Individual patterns should not be longer than 254 bytes total.");
           print_err("Pattern of enormous length ignored");
           error();
           continue;
        }
#if 0
           printf("%s\n",format);
           char* repl = strnchr(format, '/',l);
           int replindex = 0;
           int replcut = 0;
           if (repl) {
           int clen = l-(repl-format);
           l = repl-format;
           char * index = strnchr(repl + 1, ',',clen);
           if (index) {
           char * index2 = strnchr(index + 1, ',',clen-(index-repl));
           if (index2) {
           replindex = (signed char) atoi(index + 1) - 1;
           replcut = (signed char) atoi(index2 + 1);
           }
           } else {
           hnj_strchomp(repl + 1);
           replindex = 0;
           replcut = strlen(buf);
           }
           repl = hnj_strdup(repl + 1);
           }
#endif
        for (i = 0, j = 0, e1 = 0; (unsigned) i < l; i++) {
            if (format[i] >= '0' && format[i] <= '9')
                j++;
            if (is_utf8_follow(format[i]))
                e1++;
        }
        /* |l-e1|   => number of {\it characters} not {\it bytes} */
        /* |l-j|   => number of pattern bytes */
        /* |l-e1-j| => number of pattern characters */
        pat = (unsigned char *) malloc((1 + l - (size_t) j));
        org = (char *) malloc((size_t) (2 + l - (size_t) e1 - (size_t) j));
        /* remove hyphenation encoders (digits) from pat */
        org[0] = '0';
        for (i = 0, j = 0, e1 = 0; (unsigned) i < l; i++) {
            unsigned char c = format[i];
            if (is_utf8_follow(c)) {
                pat[j + e1++] = c;
            } else if (c < '0' || c > '9') {
                pat[e1 + j++] = c;
                org[j] = '0';
            } else {
                org[j] = (char) c;
            }
        }
        pat[e1 + j] = 0;
        org[j + 1] = 0;
        hyppat_insert(dict->patterns, pat, org);
    }
    dict->pat_length += (int) ((f - begin) + 2);        /* 2 for spurious spaces */
    init_hash(&dict->merged);
    v = new_HashIter(dict->patterns);
    while (nextHash(v, &word)) {
        int wordsize = (int) strlen((char *) word);
        int j1, l1;
        for (l1 = 1; l1 <= wordsize; l1++) {
            if (is_utf8_follow(word[l1]))
                continue;       /* Do not clip an utf8 sequence */
            for (j1 = 1; j1 <= l1; j1++) {
                char *subpat_pat;
                int i1 = l1 - j1;
                if (is_utf8_follow(word[i1]))
                    continue;   /* Do not start halfway an utf8 sequence */
                if ((subpat_pat =
                     hyppat_lookup(dict->patterns, word + i1, j1)) != NULL) {
                    char *newpat_pat;
                    if ((newpat_pat =
                         hyppat_lookup(dict->merged, word, l1)) == NULL) {
                        char *neworg;
                        unsigned char *newword =
                            (unsigned char *) malloc((size_t) (l1 + 1));
                        int e1 = 0;
                        strncpy((char *) newword, (char *) word, (size_t) l1);
                        newword[l1] = 0;
                        for (i1 = 0; i1 < l1; i1++)
                            if (is_utf8_follow(newword[i1]))
                                e1++;
                        neworg = malloc((size_t) (l1 + 2 - e1));
                        sprintf(neworg, "%0*d", l1 + 1 - e1, 0);  /* fill with right amount of '0' */
                        hyppat_insert(dict->merged, newword, combine(neworg, subpat_pat));
                    } else {
                        combine(newpat_pat, subpat_pat);
                    }
                }
            }
        }
    }
    delete_HashIter(v);

    init_hash(&dict->state_num);
    state_insert(dict->state_num, hnj_strdup((const unsigned char *) ""), 0);
    v = new_HashIter(dict->merged);
    while (nextHashStealPattern(v, &word, &pattern)) {
        static unsigned char mask[] = { 0x3F, 0x1F, 0xF, 0x7 };
        int j1 = (int) strlen((char *) word);
#ifdef VERBOSE
        printf("word %s pattern %s, j = %d\n", word, pattern, j1);
#endif
        state_num = hnj_get_state(dict, word, &found);
        dict->states[state_num].match = pattern;

        /* now, put in the prefix transitions */
        while (found < 0) {
            j1--;
            last_state = state_num;
            ch = word[j1];
            if (ch >= 0x80) {
                int m;
                int i1 = 1;
                while (is_utf8_follow(word[j1 - i1]))
                    i1++;
                ch = word[j1 - i1] & mask[i1];
                m = j1 - i1;
                while (i1--) {
                    ch = (ch << 6) + (0x3F & word[j1 - i1]);
                }
                j1 = m;
            }
            word[j1] = '\0';
            state_num = hnj_get_state(dict, word, &found);
            hnj_add_trans(dict, state_num, last_state, ch);
        }
    }
    delete_HashIter(v);
    clear_hyppat_hash(&dict->merged);

    /* put in the fallback states */
    {
    int i, j = 0;
    for (i = 0; i < HASH_SIZE; i++) {
        for (e = dict->state_num->entries[i]; e; e = e->next) {
            /* do not do state==0 otherwise things get confused */
            if (e->u.state) {
                for (j = 1; 1; j++) {
                    state_num = state_lookup(dict->state_num, e->key + j);
                    if (state_num >= 0)
                        break;
                }
                dict->states[e->u.state].fallback_state = state_num;
            }
        }
    }
#ifdef VERBOSE
    for (i = 0; i < HASH_SIZE; i++) {
        for (e = dict->state_num->entries[i]; e; e = e->next) {
            printf("%d string %s state %d, fallback=%d\n",
                i, e->key, e->u.state, dict->states[e->u.state].fallback_state);
            for (j = 0; j < dict->states[e->u.state].num_trans; j++) {
                printf(" u%4x->%d\n",
                   (int) dict->states[e->u.state].trans[j].uni_ch,
                   dict->states[e->u.state].trans[j].new_state);
            }
        }
    }
#endif
    }
    clear_state_hash(&dict->state_num);
}

@ @c
void hnj_hyphen_hyphenate(HyphenDict * dict,
                          halfword first1,
                          halfword last1,
                          int length,
                          halfword left, halfword right, lang_variables * lan)
{
    int char_num;
    halfword here;
    int state = 0;
    /* +2 for dots at each end, +1 for points /outside/ characters */
    int ext_word_len = length + 2;
    int hyphen_len = ext_word_len + 1;
    char *hyphens = hnj_malloc(hyphen_len + 1);

    /* Add a '.' to beginning and end to facilitate matching */
    set_vlink(begin_point, first1);
    set_vlink(end_point, get_vlink(last1));
    set_vlink(last1, end_point);

    for (char_num = 0; char_num < hyphen_len; char_num++) {
        hyphens[char_num] = '0';
    }
    hyphens[hyphen_len] = 0;

    /* now, run the finite state machine */
    for (char_num = 0, here = begin_point; here != get_vlink(end_point);
         here = get_vlink(here)) {

        int ch;
        if (here == begin_point || here == end_point) {
            ch = '.';
        } else {
            ch = get_hj_code(char_lang(here),get_character(here));
            if (ch <= 32) {
                ch = get_character(here);
            }
        }
        while (state != -1) {
#if 0
            printf("%*s%s%c",char_num-strlen(get_state_str(state)),"",get_state_str(state),(char)ch);
#endif
            HyphenState *hstate = &dict->states[state];
            int k;
            for (k = 0; k < hstate->num_trans; k++) {
                if (hstate->trans[k].uni_ch == ch) {
                    char *match;
                    state = hstate->trans[k].new_state;
#if 0
                    printf(" state %d\n",state);
#endif
                    match = dict->states[state].match;
                    if (match) {
                        /* +2 because:
                         1 string length is one bigger than offset
                         1 hyphenation starts before first character
                         */
                        int offset = (int) (char_num + 2 - (int) strlen(match));
#if 0
                        printf("%*s%s\n", offset,"", match);
#endif
                        int m;
                        for (m = 0; match[m]; m++) {
                            if (hyphens[offset + m] < match[m])
                                hyphens[offset + m] = match[m];
                        }
                    }
                    goto try_next_letter;
                }
            }
            state = hstate->fallback_state;
#if 0
            printf(" back to %d\n", state);
#endif
        }
        /* nothing worked, let's go to the next character */
        state = 0;
      try_next_letter:;
        char_num++;
    }

    /* restore the correct pointers */
    set_vlink(last1, get_vlink(end_point));

    /* pattern is \.{\^.\^w\^o\^r\^d\^.\^}   |word_len|=4, |ext_word_len|=6, |hyphens|=7
     * check      \.{    \^ \^ \^    }   so drop first two and stop after |word_len-1|
     */
    for (here = first1, char_num = 2; here != left; here = get_vlink(here))
        char_num++;
    for (; here != right; here = get_vlink(here)) {
        if (hyphens[char_num] & 1)
            here = insert_syllable_discretionary(here, lan);
        char_num++;
    }
    hnj_free(hyphens);
}
