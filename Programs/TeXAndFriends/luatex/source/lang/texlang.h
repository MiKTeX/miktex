/* texlang.h

   Copyright 2009 Taco Hoekwater <taco@luatex.org>

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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


#ifndef TEXLANG_H
#  define TEXLANG_H

typedef struct _lang_variables {
    int pre_hyphen_char;
    int post_hyphen_char;
    int pre_exhyphen_char;
    int post_exhyphen_char;
} lang_variables;

#  include "lang/hyphen.h"

struct tex_language {
    HyphenDict *patterns;
    int exceptions;             /* lua registry pointer, should be replaced */
    int id;
    int pre_hyphen_char;
    int post_hyphen_char;
    int pre_exhyphen_char;
    int post_exhyphen_char;
    int hyphenation_min;
};

#  define MAX_WORD_LEN 65536      /* in chars */

extern struct tex_language *new_language(int n);
extern struct tex_language *get_language(int n);
extern void load_patterns(struct tex_language *lang, const unsigned char *buf);
extern void load_hyphenation(struct tex_language *lang,
                             const unsigned char *buf);
extern int hyphenate_string(struct tex_language *lang, char *w, char **ret);

extern void new_hyphenation(halfword h, halfword t);
extern void clear_patterns(struct tex_language *lang);
extern void clear_hyphenation(struct tex_language *lang);
extern const char *clean_hyphenation(int id, const char *buffer, char **cleaned);
extern void hnj_hyphenation(halfword head, halfword tail);

extern void set_pre_hyphen_char(int lan, int val);
extern void set_post_hyphen_char(int lan, int val);
extern int get_pre_hyphen_char(int lan);
extern int get_post_hyphen_char(int lan);

extern void set_pre_exhyphen_char(int lan, int val);
extern void set_post_exhyphen_char(int lan, int val);
extern int get_pre_exhyphen_char(int lan);
extern int get_post_exhyphen_char(int lan);

extern void set_hyphenation_min(int lan, int val);
extern int get_hyphenation_min(int lan);

extern halfword compound_word_break(halfword t, int clang);

extern void dump_language_data(void);
extern void undump_language_data(void);
extern char *exception_strings(struct tex_language *lang);

extern void new_hyph_exceptions(void);
extern void new_patterns(void);
extern void new_pre_hyphen_char(void);
extern void new_post_hyphen_char(void);
extern void new_pre_exhyphen_char(void);
extern void new_post_exhyphen_char(void);
extern void new_hyphenation_min(void);
extern void new_hj_code(void);


#endif
