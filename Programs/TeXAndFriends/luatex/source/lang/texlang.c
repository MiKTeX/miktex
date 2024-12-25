/*

texlang.w

Copyright 2006-2012 Taco Hoekwater <taco@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU General Public License along
with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

#include "ptexlib.h"
#include <string.h>
#include "lua/luatex-api.h"

/*tex Low-level helpers */

#define unVERBOSE

#define MAX_TEX_LANGUAGES  16384

static struct tex_language *tex_languages[MAX_TEX_LANGUAGES] = { NULL };

static int next_lang_id = 0;

struct tex_language *new_language(int n)
{
    struct tex_language *lang;
    unsigned l;
    if (n >= 0) {
        l = (unsigned) n;
        if (l != (MAX_TEX_LANGUAGES - 1))
            if (next_lang_id <= n)
                next_lang_id = n + 1;
    } else {
        while (tex_languages[next_lang_id] != NULL)
            next_lang_id++;
        l = (unsigned) next_lang_id++;
    }
    if (l < (MAX_TEX_LANGUAGES - 1) && tex_languages[l] == NULL) {
        lang = xmalloc(sizeof(struct tex_language));
        tex_languages[l] = lang;
        lang->id = (int) l;
        lang->exceptions = 0;
        lang->patterns = NULL;
        lang->pre_hyphen_char = '-';
        lang->post_hyphen_char = 0;
        lang->pre_exhyphen_char = 0;
        lang->post_exhyphen_char = 0;
        lang->hyphenation_min = -1;
        if (saving_hyph_codes_par) {
            /*tex
                For now, we might just use specific value for whatever task.
            */
            hj_codes_from_lc_codes(l);
        }
        return lang;
    } else {
        return NULL;
    }
}

struct tex_language *get_language(int n)
{
    if (n >= 0 && n < MAX_TEX_LANGUAGES) {
        if (tex_languages[n] != NULL) {
            return tex_languages[n];
        } else {
            return new_language(n);
        }
    } else {
        return NULL;
    }
}

void set_pre_hyphen_char(int n, int v)
{
    struct tex_language *l = get_language((int) n);
    if (l != NULL)
        l->pre_hyphen_char = (int) v;
}

void set_post_hyphen_char(int n, int v)
{
    struct tex_language *l = get_language((int) n);
    if (l != NULL)
        l->post_hyphen_char = (int) v;
}

void set_pre_exhyphen_char(int n, int v)
{
    struct tex_language *l = get_language((int) n);
    if (l != NULL)
        l->pre_exhyphen_char = (int) v;
}

void set_post_exhyphen_char(int n, int v)
{
    struct tex_language *l = get_language((int) n);
    if (l != NULL)
        l->post_exhyphen_char = (int) v;
}

int get_pre_hyphen_char(int n)
{
    struct tex_language *l = get_language((int) n);
    if (l == NULL)
        return -1;
    return (int) l->pre_hyphen_char;
}

int get_post_hyphen_char(int n)
{
    struct tex_language *l = get_language((int) n);
    if (l == NULL)
        return -1;
    return (int) l->post_hyphen_char;
}

int get_pre_exhyphen_char(int n)
{
    struct tex_language *l = get_language((int) n);
    if (l == NULL)
        return -1;
    return (int) l->pre_exhyphen_char;
}

int get_post_exhyphen_char(int n)
{
    struct tex_language *l = get_language((int) n);
    if (l == NULL)
        return -1;
    return (int) l->post_exhyphen_char;
}

void set_hyphenation_min(int n, int v)
{
    struct tex_language *l = get_language((int) n);
    if (l != NULL)
        l->hyphenation_min = (int) v;
}

int get_hyphenation_min(int n)
{
    struct tex_language *l = get_language((int) n);
    if (l == NULL)
        return -1;
    return (int) l->hyphenation_min;
}

void load_patterns(struct tex_language *lang, const unsigned char *buff)
{
    if (lang == NULL || buff == NULL || strlen((const char *) buff) == 0)
        return;
    if (lang->patterns == NULL) {
        lang->patterns = hnj_hyphen_new();
    }
    hnj_hyphen_load(lang->patterns, buff);
}

void clear_patterns(struct tex_language *lang)
{
    if (lang == NULL)
        return;
    if (lang->patterns != NULL) {
        hnj_hyphen_clear(lang->patterns);
    }
}

void load_tex_patterns(int curlang, halfword head)
{
    char *s = tokenlist_to_cstring(head, 1, NULL);
    load_patterns(get_language(curlang), (unsigned char *) s);
}

#define STORE_CHAR(l,x) do { \
    unsigned xx = get_hj_code(l,x); \
    if (!xx || xx <= 32) { \
        xx = x; \
    } \
    uindex = uni2string(uindex, xx); \
} while (0)

/*

    This cleans one word which is returned in |cleaned|, returns the new offset
    into |buffer|.

*/

const char *clean_hyphenation(int id, const char *buff, char **cleaned)
{
    int items = 0;
    /*tex Work buffer for bytes: */
    unsigned char word[MAX_WORD_LEN + 1];
    /*tex Work buffer for \UNICODE: */
    unsigned uword[MAX_WORD_LEN + 1] = { 0 };
    /*tex The \UNICODE\ buffer value: */
    int u = 0;
    /*tex The index into buffer: */
    int i = 0;
    char *uindex = (char *)word;
    const char *s = buff;

    while (*s && !isspace((unsigned char)*s)) {
        word[i++] = (unsigned)*s;
        s++;
        if ((s-buff)>MAX_WORD_LEN) {
            /*tex Todo: this is too strict, should count \UNICODE, not bytes. */
            *cleaned = NULL;
            tex_error("exception too long", NULL);
            return s;
        }
    }
    /*tex Now convert the input to \UNICODE. */
    word[i] = '\0';
    utf2uni_strcpy(uword, (const char *)word);
    /*tex Build the new word string. */
    i = 0;
    while (uword[i]>0) {
        u = uword[i++];
        if (u == '-') {
            /*tex Skip. */
        } else if (u == '=') {
            STORE_CHAR(id,'-');
        } else if (u == '{') {
            u = uword[i++];
            items = 0;
            while (u && u != '}') {
                u = uword[i++];
            }
            if (u == '}') {
                items++;
                u = uword[i++];
            }
            while (u && u != '}') {
                u = uword[i++];
            }
            if (u == '}') {
                items++;
                u = uword[i++];
            }
            if (u == '{') {
                u = uword[i++];
            }
            while (u && u != '}') {
                STORE_CHAR(id,u);
                u = uword[i++];
            }
            if (u == '}') {
                items++;
            }
            if (items != 3) {
                *cleaned = NULL;
                tex_error("exception syntax error", NULL);
                return s;
            }
            if (uword[i] == '[' && uword[i+1] >= '0' && uword[i+1] <= '9' && uword[i+2] == ']') {
                i += 3;
            }
        } else {
            STORE_CHAR(id,u);
        }
    }
    *uindex = '\0';
    *cleaned = xstrdup((char *) word);
    return s;
}

void load_hyphenation(struct tex_language *lang, const unsigned char *buff)
{
    const char *s;
    const char *value;
    char *cleaned;
    int id ;
    if (lang == NULL)
        return;
    lua_checkstack(Luas, 3);
    if (lang->exceptions == 0) {
        lua_newtable(Luas);
        lang->exceptions = luaL_ref(Luas, LUA_REGISTRYINDEX);
    }
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, lang->exceptions);
    s = (const char *) buff;
    id = lang->id;
    while (*s) {
        while (isspace((unsigned char)*s))
            s++;
        if (*s) {
            value = s;
            s = clean_hyphenation(id, s, &cleaned);
            if (cleaned != NULL) {
                if ((s - value) > 0) {
                    lua_pushstring(Luas, cleaned);
                    lua_pushlstring(Luas, value, (size_t) (s - value));
                    lua_rawset(Luas, -3);
                }
                free(cleaned);
            } else {
#ifdef VERBOSE
                formatted_warning("hyphenation","skipping invalid hyphenation exception: %s", value);
#endif
            }
        }
    }
    lua_pop(Luas, 1);
}

void clear_hyphenation(struct tex_language *lang)
{
    if (lang == NULL)
        return;
    if (lang->exceptions != 0) {
        luaL_unref(Luas, LUA_REGISTRYINDEX, lang->exceptions);
        lang->exceptions = 0;
    }
}

void load_tex_hyphenation(int curlang, halfword head)
{
    char *s = tokenlist_to_cstring(head, 1, NULL);
    load_hyphenation(get_language(curlang), (unsigned char *) s);
}

static halfword insert_discretionary(halfword t, halfword pre, halfword post, halfword replace, int penalty)
{
    halfword g;
    int f;
    halfword d = new_node(disc_node, syllable_disc);
    int attr = node_attr(t) ;
    disc_penalty(d) = penalty;
    if (t == replace) {
        /*tex We have |prev disc next-next|. */
        try_couple_nodes(d, vlink(t));
        try_couple_nodes(alink(t), d);
        alink(t) = null;
        vlink(t) = null;
        replace = t ;
    } else {
        /*tex We have |prev disc next|. */
        try_couple_nodes(d, vlink(t));
        couple_nodes(t, d);
    }
    if (replace != null) {
        f = font(replace);
    } else {
        /*tex For compound words following explicit hyphens. */
        f = get_cur_font();
    }
    for (g = pre; g != null; g = vlink(g)) {
        if (! font(g)) {
            font(g) = f;
        }
        if (attr != null) {
            delete_attribute_ref(node_attr(g));
            node_attr(g) = attr;
            attr_list_ref(attr) += 1;
        }
    }
    for (g = post; g != null; g = vlink(g)) {
        if (! font(g)) {
            font(g) = f;
        }
        if (attr != null) {
            delete_attribute_ref(node_attr(g));
            node_attr(g) = attr;
            attr_list_ref(attr) += 1;
        }
    }
    if (attr != null) {
        for (g = replace; g != null; g = vlink(g)) {
            delete_attribute_ref(node_attr(g));
            node_attr(g) = attr;
            attr_list_ref(attr) += 1;
        }
        delete_attribute_ref(node_attr(d));
        node_attr(d) = attr;
        attr_list_ref(attr) += 1;
    }
    set_disc_field(pre_break(d), pre);
    set_disc_field(post_break(d), post);
    set_disc_field(no_break(d), replace);
    return d;
}

halfword insert_syllable_discretionary(halfword t, lang_variables * lan)
{
    halfword g, n;
    n = new_node(disc_node, syllable_disc);
    disc_penalty(n) = hyphen_penalty_par;
    couple_nodes(n, vlink(t));
    couple_nodes(t, n);
    delete_attribute_ref(node_attr(n));
    if (node_attr(t) != null) {
        node_attr(n) = node_attr(t);
        attr_list_ref(node_attr(t))++;
    } else {
        node_attr(n) = null;
    }
    if (lan->pre_hyphen_char > 0) {
        g = raw_glyph_node();
        set_to_character(g);
        character(g) = lan->pre_hyphen_char;
        font(g) = font(t);
        lang_data(g) = lang_data(t);
        if (node_attr(t) != null) {
            node_attr(g) = node_attr(t);
            attr_list_ref(node_attr(t))++;
        }
        set_disc_field(pre_break(n), g);
    }
    if (lan->post_hyphen_char > 0) {
        t = vlink(n);
        g = raw_glyph_node();
        set_to_character(g);
        character(g) = lan->post_hyphen_char;
        font(g) = font(t);
        lang_data(g) = lang_data(t);
        if (node_attr(t) != null) {
            node_attr(g) = node_attr(t);
            attr_list_ref(node_attr(t)) += 1;
        }
        set_disc_field(post_break(n), g);
    }
    return n;
}

static halfword insert_character(halfword t, int c)
{
    halfword p;
    p = new_node(glyph_node, 0);
    set_to_character(p);
    character(p) = c;
    if (t != null) {
        couple_nodes(t, p);
    }
    return p;
}

static halfword compound_word_break(halfword t, int clang)
{
    halfword disc = null;
    halfword pre = null;
    halfword pos = null;
    halfword pre_exhyphen_char = get_pre_exhyphen_char(clang);
    halfword post_exhyphen_char = get_post_exhyphen_char(clang);
    if (pre_exhyphen_char > 0) {
        pre = insert_character(null,pre_exhyphen_char);
    } else {
        pre = insert_character(null,ex_hyphen_char_par);
    }
    if (post_exhyphen_char > 0)
        pos = insert_character(null,post_exhyphen_char);
    disc = insert_discretionary(t,pre,pos,t,ex_hyphen_penalty_par);
    subtype(disc) = automatic_disc;
    set_automatic_disc_penalty(disc);
    return disc;
}

void set_disc_field(halfword f, halfword t)
{
    if (t != null) {
        couple_nodes(f, t);
        tlink(f) = tail_of_list(t);
    } else {
        vlink(f) = null;
        tlink(f) = null;
    }
}

static char *hyphenation_exception(int exceptions, char *w)
{
    char *ret = NULL;
    lua_checkstack(Luas, 2);
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, exceptions);
    if (lua_istable(Luas, -1)) {
        /*tex Word table: */
        lua_pushstring(Luas, w);
        lua_rawget(Luas, -2);
        if (lua_type(Luas, -1) == LUA_TSTRING) {
            ret = xstrdup(lua_tostring(Luas, -1));
        }
        lua_pop(Luas, 2);
    } else {
        lua_pop(Luas, 1);
    }
    return ret;
}

char *exception_strings(struct tex_language *lang)
{
    const char *value;
    size_t size = 0, current = 0;
    size_t l = 0;
    char *ret = NULL;
    if (lang->exceptions == 0)
        return NULL;
    lua_checkstack(Luas, 2);
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, lang->exceptions);
    if (lua_istable(Luas, -1)) {
        /*tex Iterate and join. */
        lua_pushnil(Luas);
        while (lua_next(Luas, -2) != 0) {
            value = lua_tolstring(Luas, -1, &l);
            if (current + 2 + l > size) {
                ret = xrealloc(ret, (unsigned) ((size + size / 5) + current + l + 1024));
                size = (size + size / 5) + current + l + 1024;
            }
            *(ret + current) = ' ';
            strcpy(ret + current + 1, value);
            current += l + 1;
            lua_pop(Luas, 1);
        }
    }
    return ret;
}

/*tex

    The sequence from |wordstart| to |r| can contain only normal characters it
    could be faster to modify a halfword pointer and return an integer

    We now take the font from the wordstart (as in \LUAMETATEX) but leave the
    rest as it is, because we don't want to break compatibility (end June 2022).
    We make a copy now of the parent and hope for the best. Backporting would be
    too intrusive so this has to do. It went unnoticed for ages anyway.

*/

static halfword find_exception_part(unsigned int *j, unsigned int *uword, int len, halfword parent)
{
    halfword g = null, gg = null;
    register unsigned i = *j;
    /*tex This puts uword[i] on the |{|. */
    i++;
    while (i < (unsigned) len && uword[i + 1] != '}') {
        if (g == null) {
         /* gg = new_char(font(parent), (int) uword[i + 1]); */
            gg = copy_node(parent);
            g = gg;
        } else {
         /* halfword s = new_char(font(parent), (int) uword[i + 1]); */
            halfword s = copy_node(parent);
            couple_nodes(g, s);
            g = s;
        }
        character(g) = (int) uword[i + 1];
        i++;
    }
    *j = ++i;
    return gg;
}

static int count_exception_part(unsigned int *j, unsigned int *uword, int len)
{
    int ret = 0;
    register unsigned i = *j;
    /*tex This puts uword[i] on the |{|. */
    i++;
    while (i < (unsigned) len && uword[i + 1] != '}') {
        ret++;
        i++;
    }
    *j = ++i;
    return ret;
}

static const char *PAT_ERROR[] = {
    "Exception discretionaries should contain three pairs of braced items.",
    "No intervening spaces are allowed.",
    NULL
};

/*tex

    The exceptions are taken as-is: no min values are taken into account. One can
    add normal patterns on-the-fly if needed.

*/

static void do_exception(halfword wordstart, halfword r, char *replacement)
{
    unsigned i;
    halfword t, pen;
    unsigned len;
    int clang;
    lang_variables langdata;
    unsigned uword[MAX_WORD_LEN + 1] = { 0 };
    utf2uni_strcpy(uword, replacement);
    len = u_length(uword);
    i = 0;
    t = wordstart;
    clang = char_lang(wordstart);
    langdata.pre_hyphen_char = get_pre_hyphen_char(clang);
    langdata.post_hyphen_char = get_post_hyphen_char(clang);
    for (i = 0; i < len; i++) {
        if (uword[i + 1] == 0 ) {
            /*tex We ran out of the exception pattern. */
            break;
        } else if (uword[i + 1] == '-') {
            /*tex A hyphen follows. */
            if (vlink(t) == r)
                break;
            insert_syllable_discretionary(t, &langdata);
            t = vlink(t);       /* skip the new disc */
        } else if (uword[i + 1] == '=') {
            /*tex We skip a disc. */
            t = vlink(t);
        } else if (uword[i + 1] == '{') {
            /*tex We ran into an exception |{}{}{}| or |{}{}{}[]|. */
            halfword gg, hh, replace = null;
            int repl;
            /*tex |pre| */
            gg = find_exception_part(&i, uword, (int) len, wordstart);
            if (i == len || uword[i + 1] != '{') {
                tex_error("broken pattern 1", PAT_ERROR);
            }
            /*tex |post| */
            hh = find_exception_part(&i, uword, (int) len, wordstart);
            if (i == len || uword[i + 1] != '{') {
                tex_error("broken pattern 2", PAT_ERROR);
            }
            /*tex |replace| */
            repl = count_exception_part(&i, uword, (int) len);
            if (i == len) {
                tex_error("broken pattern 3", PAT_ERROR);
            }
            /*tex Play safe. */
            if (vlink(t) == r)
                break;
            /*tex Let's deal with an (optional) replacement. */
            if (repl > 0) {
                /*tex Assemble the replace stream. */
                halfword q = t;
                replace = vlink(q);
                while (repl > 0 && q != null) {
                    q = vlink(q);
                    if (type(q) == glyph_node || type(q) == disc_node) {
                        repl--;
                    } else {
                        break ;
                    }
                }
                /*tex Remove it from the main stream */
                try_couple_nodes(t, vlink(q));
                /*tex and finish it in the replace. */
                vlink(q) = null;
                /*tex Sanitize the replace stream (we could use the flattener instead). */
                q = replace ;
                while (q != null) {
                    halfword n = vlink(q);
                    if (type(q) == disc_node) {
                        /*tex Beware: the replacement starts after the no_break pointer. */
                        halfword r = vlink(no_break(q));
                        vlink(no_break(q)) = null;
                        alink(r) = null ;
                        /*tex Insert the replacement glyph. */
                        if (q == replace) {
                            replace = r;
                        } else {
                            try_couple_nodes(alink(q),r);
                        }
                        /*tex Append the glyph (one). */
                        try_couple_nodes(r,n);
                        /*tex Flush the disc. */
                        flush_node(q);
                    }
                    q = n ;
                }
            }
            /*tex Let's check if we have a penalty spec. */
            if (((i+3) < len) && uword[i+1] == '[' && uword[i+2] >= '0' && uword[i+2] <= '9' && uword[i+3] == ']') {
                if (exception_penalty_par > 0) {
                    if (exception_penalty_par > 10000) {
                        pen = exception_penalty_par;
                    } else {
                        pen = (uword[i+2] - '0') * exception_penalty_par ;
                    }
                } else {
                    pen = hyphen_penalty_par;
                }
                i += 3;
            } else {
                pen = hyphen_penalty_par;
            }
            /*tex And now we insert a disc node. */
            t = insert_discretionary(t, gg, hh, replace, pen);
            /*tex We skip the new disc node. */
            t = vlink(t);
            /*tex check if we have two exceptions in a row */
            if (uword[i + 1] == '{') {
                i--;
t = alink(t);
            }
        } else {
            t = vlink(t);
        }
        /*tex Again we play safe. */
        if (t == null || vlink(t) == r) {
            break;
        }
    }
}

/*tex

    This is a documentation section from the pascal web file. It is not true any
    more, but I (Taco) do not have time right now to rewrite it.

    When the line-breaking routine is unable to find a feasible sequence of
    breakpoints, it makes a second pass over the paragraph, attempting to
    hyphenate the hyphenatable words. The goal of hyphenation is to insert
    discretionary material into the paragraph so that there are more potential
    places to break.

    The general rules for hyphenation are somewhat complex and technical, because
    we want to be able to hyphenate words that are preceded or followed by
    punctuation marks, and because we want the rules to work for languages other
    than English. We also must contend with the fact that hyphens might radically
    alter the ligature and kerning structure of a word.

    A sequence of characters will be considered for hyphenation only if it
    belongs to a ``potentially hyphenatable part'' of the current paragraph. This
    is a sequence of nodes $p_0p_1\ldots p_m$ where $p_0$ is a glue node,
    $p_1\ldots p_{m-1}$ are either character or ligature or whatsit or implicit
    kern nodes, and $p_m$ is a glue or penalty or insertion or adjust or mark or
    whatsit or explicit kern node. (Therefore hyphenation is disabled by boxes,
    math formulas, and discretionary nodes already inserted by the user.) The
    ligature nodes among $p_1\ldots p_{m-1}$ are effectively expanded into the
    original non-ligature characters; the kern nodes and whatsits are ignored.
    Each character |c| is now classified as either a nonletter (if
    |lc_code(c)=0|), a lowercase letter (if |lc_code(c)=c|), or an uppercase
    letter (otherwise); an uppercase letter is treated as if it were |lc_code(c)|
    for purposes of hyphenation. The characters generated by $p_1\ldots p_{m-1}$
    may begin with nonletters; let $c_1$ be the first letter that is not in the
    middle of a ligature. Whatsit nodes preceding $c_1$ are ignored; a whatsit
    found after $c_1$ will be the terminating node $p_m$. All characters that do
    not have the same font as $c_1$ will be treated as nonletters. The
    |hyphen_char| for that font must be between 0 and 255, otherwise hyphenation
    will not be attempted. \TeX\ looks ahead for as many consecutive letters
    $c_1\ldots c_n$ as possible; however, |n| must be less than 64, so a
    character that would otherwise be $c_{64}$ is effectively not a letter.
    Furthermore $c_n$ must not be in the middle of a ligature. In this way we
    obtain a string of letters $c_1\ldots c_n$ that are generated by nodes
    $p_a\ldots p_b$, where |1<=a<=b+1<=m|. If |n>=l_hyf+r_hyf|, this string
    qualifies for hyphenation; however, |uc_hyph| must be positive, if $c_1$ is
    uppercase.

    The hyphenation process takes place in three stages. First, the candidate
    sequence $c_1\ldots c_n$ is found; then potential positions for hyphens are
    determined by referring to hyphenation tables; and finally, the nodes
    $p_a\ldots p_b$ are replaced by a new sequence of nodes that includes the
    discretionary breaks found.

    Fortunately, we do not have to do all this calculation very often, because of
    the way it has been taken out of \TeX's inner loop. For example, when the
    second edition of the author's 700-page book {\sl Seminumerical Algorithms}
    was typeset by \TeX, only about 1.2 hyphenations needed to be tried per
    paragraph, since the line breaking algorithm needed to use two passes on only
    about 5 per cent of the paragraphs.

    When a word been set up to contain a candidate for hyphenation, \TeX\ first
    looks to see if it is in the user's exception dictionary. If not, hyphens are
    inserted based on patterns that appear within the given word, using an
    algorithm due to Frank~M. Liang.

    This is incompatible with \TEX\ because the first word of a paragraph can be
    hyphenated, but most European users seem to agree that prohibiting
    hyphenation there was not the best idea ever.

    We have some variants available that are controlled by the paremeter
    \type {\hyphenationbounds}:

    \starttabulate
    \NC \type {0} \NC not strict \NC \NR
    \NC \type {1} \NC strict start \NC \NR
    \NC \type {2} \NC strict end \NC \NR
    \NC \type {3} \NC strict start and strict end \NC \NR
    \stoptabulate

    \startbuffer
    \parindent0pt \hsize=1.1cm
    12-34-56 \par
    12-34-\hbox{56} \par
    12-34-\vrule width 1em height 1.5ex \par
    12-\hbox{34}-56 \par
    12-\vrule width 1em height 1.5ex-56 \par
    \hjcode`\1=`\1 \hjcode`\2=`\2 \hjcode`\3=`\3 \hjcode`\4=`\4 \vskip.5cm
    12-34-56 \par
    12-34-\hbox{56} \par
    12-34-\vrule width 1em height 1.5ex \par
    12-\hbox{34}-56 \par
    12-\vrule width 1em height 1.5ex-56 \par
    \stopbuffer

    \typebuffer

    \startpacked \getbuffer \stopbuffer

    We only accept an explicit hyphen when there is a preceding glyph and we skip
    a sequence of explicit hyphens as that normally indicates a \type {--} or
    \type {---} ligature in which case we can in a worse case usage get bad node
    lists later on due to messed up ligature building as these dashes are
    ligatures in base fonts. This is a side effect of the separating the
    hyphenation, ligaturing and kerning steps. A test is cmr with \type {------}.

    A font handler can collapse successive hyphens but it's not nice to put the
    burden there. A somewhat messy border case is \type {----} but in \LUATEX\ we
    don't treat \type {--} and \type {---} special. Also, traditional \TEX\ will
    break a line at \type {-foo} but this can be disabled by setting the
    automatic mode to \type {1}.

*/

static halfword find_next_wordstart(halfword r, halfword first_language, halfword strict_bound)
{
    register int l;
    register int start_ok = 1;
    int mathlevel = 1;
    int chr ;
    halfword t ;
    while (r != null) {
        switch (type(r)) {
            case boundary_node:
                if (subtype(r) == word_boundary) {
                    start_ok = 1;
                }
                break;
            case hlist_node:
            case vlist_node:
            case rule_node:
            case dir_node:
            case whatsit_node:
                if (strict_bound == 1 || strict_bound == 3) {
                    start_ok = 0;
                }
                break;
            case glue_node:
                start_ok = 1;
                break;
            case math_node:
                while (mathlevel > 0) {
                    r = vlink(r);
                    if (r == null)
                        return r;
                    if (type(r) == math_node) {
                        if (subtype(r) == before) {
                            mathlevel++;
                        } else {
                            mathlevel--;
                        }
                    }
                }
                break;
            case glyph_node:
                if (is_simple_character(r)) {
                    chr = character(r) ;
                    if (chr == ex_hyphen_char_par) {
                        t = vlink(r) ;
                        if ((automatic_hyphen_mode_par == 0) && (t != null) && (type(t) == glyph_node) && (character(t) != ex_hyphen_char_par)) {
                            /*tex We have no word yet and the next character is a non hyphen. */
                            r = compound_word_break(r, char_lang(r));
                        } else {
                            /*tex We jump over the sequence of hyphens. */
                            while ((t != null) && (type(t) == glyph_node) && (character(t) == ex_hyphen_char_par)) {
                                r = t ;
                                t = vlink(r) ;
                            }
                            if (t == null) {
                                /*tex We reached the end of the list so we have no word start. */
                                return null;
                            }
                        }
                        /*tex We need a restart. */
                        start_ok = 0;
                    } else if (start_ok && (char_lang(r)>=first_language) && ((l = get_hj_code(char_lang(r),chr)) > 0)) {
                        if (char_uchyph(r) || l == chr || l <= 32) {
                            return r;
                        } else {
                            start_ok = 0;
                        }
                    } else {
                        /*tex We go on. */
                    }
                }
                break;
            default:
                start_ok = 0;
                break;
        }
        r = vlink(r);
    }
    return r;
}

static int valid_wordend(halfword s, halfword strict_bound)
{
    register halfword r = s;
    register int clang = char_lang(s);
    if (r == null)
        return 1;
    while ( (r != null) &&
           (    (type(r) == glyph_node && is_simple_character(r) && clang == char_lang(r))
             || (type(r) == kern_node && (subtype(r) == normal))
            )
           ) {
        r = vlink(r);
    }
    if (r == null || (type(r) == glyph_node && is_simple_character(r) && clang != char_lang(r))
                  ||  type(r) == glue_node
                  ||  type(r) == penalty_node
                  || (type(r) == kern_node && (subtype(r) == explicit_kern ||
                                               subtype(r) == italic_kern   ||
                                               subtype(r) == accent_kern   ))
                  ||  ((type(r) == hlist_node   ||
                        type(r) == vlist_node   ||
                        type(r) == rule_node    ||
                        type(r) == dir_node     ||
                        type(r) == whatsit_node ||
                        type(r) == ins_node     ||
                        type(r) == adjust_node
                       ) && ! (strict_bound == 2 || strict_bound == 3))
                  ||  type(r) == boundary_node
        )
        return 1;
    return 0;
}

void hnj_hyphenation(halfword head, halfword tail)
{
    int lchar, i;
    struct tex_language *lang;
    lang_variables langdata;
    char utf8word[(4 * MAX_WORD_LEN) + 1];
    int wordlen = 0;
    char *hy = utf8word;
    char *replacement = NULL;
    boolean explicit_hyphen = false;
    boolean valid_word = false;
    halfword first_language = first_valid_language_par;
    halfword strict_bound = hyphenation_bounds_par;
    halfword s, r = head, wordstart = null, save_tail1 = null, left = null, right = null;
    halfword expstart = null;
    boolean compound_hyphen = compound_hyphen_mode_par;
    /*tex Let's play safe: */
    if (tail == null)
        return;
    /*tex
        This first movement assures two things:

        \startitemize
            \startitem
                That we won't waste lots of time on something that has been
                handled already (in that case, none of the glyphs match
                |simple_character|).
            \stopitem
            \startitem
                That the first word can be hyphenated. if the movement was not
                explicit, then the indentation at the start of a paragraph list
                would make |find_next_wordstart()| look too far ahead.
            \stopitem
        \stopitemize
    */
    while (r != null && (type(r) != glyph_node || !is_simple_character(r))) {
        r = vlink(r);
    }
    /*tex
        This will make |r| a glyph node with subtype character.
    */
    r = find_next_wordstart(r,first_language,strict_bound);
    if (r == null)
        return;
    if (tail == null)
        return;
    save_tail1 = vlink(tail);
    s = new_penalty(0,word_penalty);
    couple_nodes(tail, s);
    while (r != null) {
        /*tex This could be |while(1)|, but let's be paranoid: */
        int clang, lhmin, rhmin, hmin;
        halfword hyf_font;
        halfword end_word = r;
        wordstart = r;
        /*assert(is_simple_character(wordstart));*/
        if (!(is_simple_character(wordstart))){
         if (!(is_character(wordstart))) {
          tex_error("the word doesn't start with a character", NULL);
         } else if (is_ligature(wordstart)) {
           tex_error("the word starts with a ligature", NULL);
         } else if (is_ghost(wordstart)) {
           tex_error("the word starts with a ghost glyph", NULL);
         } else {
           tex_error("the word doesn't start with a simple character", NULL);
         }
        }
        hyf_font = font(wordstart);
        if (hyphen_char(hyf_font) < 0) {
            /*tex For backward compatibility we set: */
            hyf_font = 0;
        }
        clang = char_lang(wordstart);
        lhmin = char_lhmin(wordstart);
        rhmin = char_rhmin(wordstart);
        hmin = get_hyphenation_min(clang);
        langdata.pre_hyphen_char = get_pre_hyphen_char(clang);
        langdata.post_hyphen_char = get_post_hyphen_char(clang);
        while (    r != null
                && type(r) == glyph_node
                && is_simple_character(r)
                && clang == char_lang(r)
                && (    (     (clang >= first_language)
                           && (lchar = get_hj_code(clang,character(r))) > 0
                        )
                     || (     character(r) == ex_hyphen_char_par
                           && (lchar = ex_hyphen_char_par)
                        )
                   )
              ) {
            if (character(r) == ex_hyphen_char_par) {
                explicit_hyphen = true;
                break;
            }
            wordlen++;
            if (wordlen >= MAX_WORD_LEN) {
             while (r && type(r) == glyph_node) {
               r = vlink(r);
             }
             goto PICKUP;
            }
            if (lchar <= 32) {
                if (lchar == 32) {
                    lchar = 0 ;
                }
                if (wordlen <= lhmin) {
                    lhmin = lhmin - lchar + 1 ;
                    if (lhmin < 0)
                        lhmin = 1;
                }
                if (wordlen >= rhmin) {
                    rhmin = rhmin - lchar + 1 ;
                    if (rhmin < 0)
                        rhmin = 1;
                }
                hmin = hmin - lchar + 1 ;
                if (hmin < 0)
                    rhmin = 1;
                lchar = character(r) ;
            }
            hy = uni2string(hy, (unsigned) lchar);
            end_word = r;
            r = vlink(r);
        }
        if (explicit_hyphen == true) {
            /*tex we are not at the start, so we only need to look ahead */
            halfword t = vlink(r) ;
            if ((automatic_hyphen_mode_par == 0 || automatic_hyphen_mode_par == 1) && (t != null) && ((type(t) == glyph_node) && is_simple_character(t) && (character(t) != ex_hyphen_char_par))) {
                /*tex we have a word already but the next character may not be a hyphen too */
                r = compound_word_break(r, char_lang(r));
                if (compound_hyphen) {
                    if (expstart == null) {
                        expstart = wordstart;
                    }
                    explicit_hyphen = false;
                    hy = uni2string(hy, '-');
                    r = t;
                    continue;
                }
            } else {
                /*tex we jump over the sequence of hyphens */
               while ((t != null) && (type(t) == glyph_node) && is_simple_character(t) && (character(t) == ex_hyphen_char_par)) {
                    r = t ;
                    t = vlink(r) ;
                }
                if (t == null) {
                    /*tex we reached the end of the list and will quit the loop later */
                    r = null;
                }
            }
        } else if (     valid_wordend(r,strict_bound)
              && clang >= first_language
              && wordlen >= lhmin + rhmin
              && (hmin <= 0 || wordlen >= hmin)
              && (hyf_font != 0)
              && (lang = tex_languages[clang]) != NULL
           ) {
            *hy = 0;
            /*tex
                this is messy and nasty: we can have a word with a - in it which
                is why we have two branches
            */
            if (lang->exceptions != 0 && (replacement = hyphenation_exception(lang->exceptions, utf8word)) != NULL) {
                /*tex handle the exception and go on to the next word */
                if (expstart == null) {
                    do_exception(wordstart, r, replacement);
                } else {
                    do_exception(expstart,r,replacement);
                }
                free(replacement);
            } else if (expstart != null) {
                /*tex We're done already */
            } else if (lang->patterns != NULL) {
                /*tex A safeguard, not needed but doesn't hurt either: */
                valid_word = true;
                left = wordstart;
                for (i = lhmin; i > 1; i--) {
                    left = vlink(left);
                    if (left == null) {
                        valid_word = false;
                        break ;
                    }
                    while (!is_simple_character(left)) {
                        left = vlink(left);
                        if (left == null) {
                            valid_word = false;
                            break ;
                        }
                    }
                }
                if (valid_word) {
                    right = r;
                    if (right == left) {
                        valid_word = false;
                        break ;
                    }
                    for (i = rhmin; i > 0; i--) {
                        right = alink(right);
                        if (right == null || right == left) {
                            valid_word = false;
                            break ;
                        }
                        while (!is_simple_character(right)) {
                            right = alink(right);
                            if (right == null || right == left) {
                                valid_word = false;
                                break ;
                            }
                        }
                    }
                    if (valid_word && expstart == null) {
                        (void) hnj_hyphen_hyphenate(lang->patterns, wordstart, end_word, wordlen, left, right, &langdata);
                    } else {
                        /*tex nothing yet */
                    }
                }
            }
        }
PICKUP:
        expstart = null ;
        explicit_hyphen = false;
        wordlen = 0;
        hy = utf8word;
        if (r == null)
            break;
        r = find_next_wordstart(r,first_language,strict_bound);
    }
    flush_node(vlink(tail));
    vlink(tail) = save_tail1;
}

void new_hyphenation(halfword head, halfword tail)
{
    int i, top;
    register int callback_id = 0;
    if (head == null || vlink(head) == null)
        return;
    fix_node_list(head);
    callback_id = callback_defined(hyphenate_callback);
    if (callback_id > 0) {
        top = lua_gettop(Luas);
        if (!get_callback(Luas, callback_id)) {
            lua_settop(Luas, top);
            return;
        }
        nodelist_to_lua(Luas, head);
        nodelist_to_lua(Luas, tail);
        if ((i=lua_pcall(Luas, 2, 0, 0)) != 0) {
            formatted_warning("hyphenation","bad specification: %s",lua_tostring(Luas, -1));
            lua_settop(Luas, top);
            luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
            return;
        }
        lua_settop(Luas, top);
    } else if (callback_id == 0) {
        hnj_hyphenation(head, tail);
    }
}

/*tex

    Dumping and undumping languages:

*/

#define dump_string(a)                \
  if (a!=NULL) {                      \
      x = (int)strlen(a)+1;           \
    dump_int(x);  dump_things(*a, x); \
  } else {                            \
    x = 0; dump_int(x);               \
  }

static void dump_one_language(int i)
{
    char *s = NULL;
    int x = 0;
    struct tex_language *lang;
    lang = tex_languages[i];
    dump_int(lang->id);
    dump_int(lang->pre_hyphen_char);
    dump_int(lang->post_hyphen_char);
    dump_int(lang->pre_exhyphen_char);
    dump_int(lang->post_exhyphen_char);
    dump_int(lang->hyphenation_min);
    if (lang->patterns != NULL) {
        s = (char *) hnj_serialize(lang->patterns);
    }
    dump_string(s);
    if (s != NULL) {
        free(s);
        s = NULL;
    }
    if (lang->exceptions != 0)
        s = exception_strings(lang);
    dump_string(s);
    if (s != NULL) {
        free(s);
    }
    free(lang);
}

void dump_language_data(void)
{
    int i;
    dump_int(next_lang_id);
    for (i = 0; i < next_lang_id; i++) {
        if (tex_languages[i]) {
            dump_int(1);
            dump_one_language(i);
        } else {
            dump_int(0);
        }
    }
}

static void undump_one_language(int i)
{
    char *s = NULL;
    int x = 0;
    struct tex_language *lang = get_language(i);
    undump_int(x);
    lang->id = x;
    undump_int(x);
    lang->pre_hyphen_char = x;
    undump_int(x);
    lang->post_hyphen_char = x;
    undump_int(x);
    lang->pre_exhyphen_char = x;
    undump_int(x);
    lang->post_exhyphen_char = x;
    undump_int(x);
    lang->hyphenation_min = x;
    /*tex patterns */
    undump_int(x);
    if (x > 0) {
        s = xmalloc((unsigned) x);
        undump_things(*s, x);
        load_patterns(lang, (unsigned char *) s);
        free(s);
    }
    /*tex exceptions */
    undump_int(x);
    if (x > 0) {
        s = xmalloc((unsigned) x);
        undump_things(*s, x);
        load_hyphenation(lang, (unsigned char *) s);
        free(s);
    }
}

void undump_language_data(void)
{
    int i, x, numlangs;
    undump_int(numlangs);
    next_lang_id = numlangs;
    for (i = 0; i < numlangs; i++) {
        undump_int(x);
        if (x == 1) {
            undump_one_language(i);
        }
    }
}

/*tex

    When \TeX\ has scanned `\.{\\hyphenation}', it calls on a procedure named
    |new_hyph_exceptions| to do the right thing.

*/

void new_hyph_exceptions(void)
{
    (void) scan_toks(false, true);
    load_tex_hyphenation(language_par, def_ref);
    flush_list(def_ref);
}

/*
    Similarly, when \TeX\ has scanned `\.{\\patterns}', it calls on a procedure
    named |new_patterns|.

*/

void new_patterns(void)
{
    (void) scan_toks(false, true);
    load_tex_patterns(language_par, def_ref);
    flush_list(def_ref);
}

/*tex

    `\.{\\prehyphenchar}', sets the |pre_break| character, and
    `\.{\\posthyphenchar}' the |post_break| character. Their respective defaults
    are ascii hyphen ("-") and zero (nul).

*/

void new_pre_hyphen_char(void)
{
    scan_optional_equals();
    scan_int();
    set_pre_hyphen_char(language_par, cur_val);
}

void new_post_hyphen_char(void)
{
    scan_optional_equals();
    scan_int();
    set_post_hyphen_char(language_par, cur_val);
}

/*tex

    `\.{\\preexhyphenchar}', sets the |pre_break| character, and
    `\.{\\postexhyphenchar}' the |post_break| character. Their defaults are both
    zero (nul).

*/

void new_pre_exhyphen_char(void)
{
    scan_optional_equals();
    scan_int();
    set_pre_exhyphen_char(language_par, cur_val);
}

void new_post_exhyphen_char(void)
{
    scan_optional_equals();
    scan_int();
    set_post_exhyphen_char(language_par, cur_val);
}

void new_hyphenation_min(void)
{
    scan_optional_equals();
    scan_int();
    set_hyphenation_min(language_par, cur_val);
}

void new_hj_code(void)
{
    int i ;
    scan_int();
    i = cur_val;
    scan_optional_equals();
    scan_int();
    set_hj_code(language_par, i, cur_val, -1);
}
