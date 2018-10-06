/* stringpool.h

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


#ifndef STRINGPOOL_H
#  define STRINGPOOL_H

/* Both lua and tex strings can contains null, but C strings cannot, so: */

typedef struct {
    unsigned char *s;
    size_t l;
} lstring;

typedef struct {
    const char *s;
    size_t l;
} const_lstring;

extern lstring *string_pool;

extern str_number str_ptr;
extern str_number init_str_ptr;

#  define STRING_OFFSET 0x200000
#  define STRING_OFFSET_BITS 21

#  define get_nullstr() STRING_OFFSET

#  define biggest_char           1114111  /* 0x10FFFF, the largest allowed character number; must be |< max_halfword| */
#  define too_big_char   (biggest_char+1) /* 1114112, |biggest_char+1| */
#  define special_char   (biggest_char+2) /* 1114113, |biggest_char+2| */
#  define number_chars   (biggest_char+3) /* 1114112, |biggest_char+1| */

/*
  Several of the elementary string operations are performed using
  macros instead of procedures, because many of the
  operations are done quite frequently and we want to avoid the
  overhead of procedure calls. For example, here is
  a simple macro that computes the length of a string.
*/

#  define str_length(a) string_pool[(a)-STRING_OFFSET].l
#  define str_string(a) string_pool[(a)-STRING_OFFSET].s
#  define str_lstring(a) string_pool[(a)-STRING_OFFSET]

/* Strings are created by appending character codes to |str_pool|.
   The |append_char| macro, defined here, does not check to see if the
   value of |pool_ptr| has gotten too high; this test is supposed to be
   made before |append_char| is used. There is also a |flush_char|
   macro, which erases the last character appended.

   To test if there is room to append |l| more characters to |str_pool|,
   we shall write |str_room(l)|, which aborts \TeX\ and gives an
   apologetic error message if there isn't enough room.
*/

/* The length of the current string is called |cur_length|: */

extern unsigned char *cur_string;
extern unsigned cur_length;
extern unsigned cur_string_size;
extern unsigned pool_size;

#  define EXTRA_STRING 500

/* put |ASCII_code| \# at the end of |str_pool| */
#  define append_char(A) do {                                           \
        if (cur_string==NULL) reset_cur_string();                       \
        else str_room(1);                                               \
        cur_string[cur_length++]=(unsigned char)(A);                    \
    } while (0)

#  define str_room(wsize) do {                                          \
        unsigned nsize;                                                 \
        if ((cur_length+wsize) > cur_string_size) {                     \
            nsize = cur_string_size + cur_string_size / 5 + EXTRA_STRING; \
            if (nsize < (unsigned)(wsize)) {                            \
                nsize = wsize + EXTRA_STRING;                           \
            }                                                           \
            cur_string = (unsigned char *) xreallocarray(cur_string, unsigned char, nsize); \
            memset (cur_string+cur_length,0,nsize-cur_length); \
            cur_string_size = nsize;                                    \
        }                                                               \
    } while (0)

#  define flush_char() --cur_length     /* forget the last character in the pool */

extern str_number make_string(void);
extern boolean str_eq_buf(str_number s, int k);
extern boolean str_eq_str(str_number s, str_number t);
extern boolean str_eq_cstr(str_number, const char *, size_t);
extern boolean get_strings_started(void);
extern void reset_cur_string(void);

#  define save_cur_string() (cur_length>0 ? make_string() : 0)

#  define restore_cur_string(u) if (u!=0) {                   \
        unsigned l = (unsigned)str_length(u);		      \
	xfree(cur_string);                                  \
        reset_cur_string();                                 \
        str_room(l);                                        \
        memcpy(cur_string, str_string(u),l);                \
        cur_length = l;                                     \
        flush_str(u);                                       \
        u=0;                                                \
    }


extern str_number search_string(str_number search);
extern int pool_to_unichar(unsigned char *t);

extern str_number maketexstring(const char *);
extern str_number maketexlstring(const char *, size_t);
extern void append_string(const unsigned char *s, unsigned l);

extern char *makecstring(int);
extern char *makeclstring(int, size_t *);

extern int dump_string_pool(void);
extern int undump_string_pool(void);

extern void init_string_pool_array(unsigned s);
extern void flush_str(str_number s);

#endif
