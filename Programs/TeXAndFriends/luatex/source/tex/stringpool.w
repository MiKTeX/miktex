% stringpool.w
%
% Copyright 2009-2010 Taco Hoekwater <taco@@luatex.org>
%
% This file is part of LuaTeX.
%
% LuaTeX is free software; you can redistribute it and/or modify it under
% the terms of the GNU General Public License as published by the Free
% Software Foundation; either version 2 of the License, or (at your
% option) any later version.
%
% LuaTeX is distributed in the hope that it will be useful, but WITHOUT
% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
% FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
% License for more details.
%
% You should have received a copy of the GNU General Public License along
% with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

@ @c


#include "ptexlib.h"

@ Control sequence names and diagnostic messages are variable-length strings
of eight-bit characters. Since PASCAL did not have a well-developed string
mechanism, \TeX\ did all of its string processing by homegrown methods.

Elaborate facilities for dynamic strings are not needed, so all of the
necessary operations can be handled with a simple data structure.
The array |str_pool| contains all of the (eight-bit) bytes off all
of the strings, and the array |str_start| contains indices of the starting
points of each string. Strings are referred to by integer numbers, so that
string number |s| comprises the characters |str_pool[j]| for
|str_start_macro(s)<=j<str_start_macro(s+1)|. Additional integer variables
|pool_ptr| and |str_ptr| indicate the number of entries used so far
in |str_pool| and |str_start|, respectively; locations
|str_pool[pool_ptr]| and |str_start_macro(str_ptr)| are
ready for the next string to be allocated.

String numbers 0 to |biggest_char| are reserved for strings that correspond to 
single UNICODE characters. This is in accordance with the conventions of \.{WEB}
which converts single-character strings into the ASCII code number of the
single character involved.

@c
lstring *string_pool;           /* the array of strings */
lstring *_string_pool;          /* this variable lives |STRING_OFFSET| below |string_pool| 
                                   (handy for debugging: 
                                   |_string_pool[str_ptr] == str_string(str_ptr)| */

str_number str_ptr = (STRING_OFFSET + 1);       /* number of the current string being created */
str_number init_str_ptr;        /* the starting value of |str_ptr| */

unsigned char *cur_string;      /*  current string buffer */
unsigned cur_length;            /* current index in that buffer */
unsigned cur_string_size;       /*  malloced size of |cur_string| */
unsigned pool_size;             /* occupied byte count */


@ Once a sequence of characters has been appended to |cur_string|, it
officially becomes a string when the function |make_string| is called.
This function returns the identification number of the new string as its
value.

@c
void reset_cur_string(void)
{
    cur_length = 0;
    cur_string_size = 255;
    cur_string = (unsigned char *) xmalloc(256);
    memset(cur_string, 0, 256);
}

@  current string enters the pool 
@c
str_number make_string(void)
{
    if (str_ptr == (max_strings + STRING_OFFSET))
        overflow("number of strings",
                 (unsigned) (max_strings - init_str_ptr + STRING_OFFSET));
    str_room(1);
    cur_string[cur_length] = '\0';      /* now |lstring.s| is always a valid C string */
    str_string(str_ptr) = (unsigned char *) cur_string;
    str_length(str_ptr) = cur_length;
    pool_size += cur_length;
    reset_cur_string();
#if 0
    printf("Made a string: %s (s=%d)\n", (char *)str_string(str_ptr), (int)str_ptr);
#endif
    str_ptr++;
    return (str_ptr - 1);
}

@ @c
int pool_to_unichar(unsigned char *t)
{
    return (int) str2uni(t);
}



@ The following subroutine compares string |s| with another string of the
same length that appears in |buffer| starting at position |k|;
the result is |true| if and only if the strings are equal.
Empirical tests indicate that |str_eq_buf| is used in such a way that
it tends to return |true| about 80 percent of the time.

@c
boolean str_eq_buf(str_number s, int k)
{                               /* test equality of strings */
    int a;                      /* a unicode character */
    if (s < STRING_OFFSET) {
        a = buffer_to_unichar(k);
        if (a != s)
            return false;
    } else {
        unsigned char *j = str_string(s);
        unsigned char *l = j + str_length(s);
        while (j < l) {
            if (*j++ != buffer[k++])
                return false;
        }
    }
    return true;
}


@ Here is a similar routine, but it compares two strings in the string pool,
and it does not assume that they have the same length.

@c
boolean str_eq_str(str_number s, str_number t)
{                               /* test equality of strings */
    int a = 0;                  /* a utf char */
    unsigned char *j, *k, *l;   /* running indices */
    if (s < STRING_OFFSET) {
        if (t >= STRING_OFFSET) {
            k = str_string(t);
            if (s <= 0x7F && (str_length(t) == 1) && *k == s)
                return true;
            a = pool_to_unichar(k);
            if (a != s)
                return false;
        } else {
            if (t != s)
                return false;
        }
    } else if (t < STRING_OFFSET) {
        j = str_string(s);
        if (t <= 0x7F && (str_length(s) == 1) && *j == t)
            return true;
        a = pool_to_unichar(j);
        if (a != t)
            return false;
    } else {
        if (str_length(s) != str_length(t))
            return false;
        k = str_string(t);
        j = str_string(s);
        l = j + str_length(s);
        while (j < l) {
            if (*j++ != *k++)
                return false;
        }
    }
    return true;
}

@ string compare 
@c
boolean str_eq_cstr(str_number r, const char *s, size_t l)
{
    if (l != (size_t) str_length(r))
        return false;
    return (strncmp((const char *) (str_string(r)), s, l) == 0);
}


@ The initial values of |str_pool|, |str_start|, |pool_ptr|,
and |str_ptr| are computed by the \.{INITEX} program, based in part
on the information that \.{WEB} has output while processing \TeX.

The first |string_offset| strings are single-characters strings matching
Unicode. There is no point in generating all of these. But |str_ptr| has
initialized properly, otherwise |print_char| cannot see the difference
between characters and strings.


@ initializes the string pool, but returns |false| if something goes wrong 
@c
boolean get_strings_started(void)
{
    reset_cur_string();
    return true;
}

@ The string recycling routines.
   \TeX{} uses 2 upto 4 {\it new\/} strings when scanning a filename in an
   \.{\\input}, \.{\\openin}, or \.{\\openout} operation.  These strings are
   normally lost because the reference to them are not saved after finishing
   the operation.  |search_string| searches through the string pool for the
   given string and returns either 0 or the found string number.

@c
str_number search_string(str_number search)
{
    str_number s;               /* running index */
    size_t len;                 /* length of searched string */
    len = str_length(search);
    if (len == 0) {
        return get_nullstr();
    } else {
        s = search - 1;         /* start search with newest string below |s|; |search>1|! */
        while (s >= STRING_OFFSET) {
            /* first |string_offset| strings depend on implementation!! */
            if (str_length(s) == len)
                if (str_eq_str(s, search))
                    return s;
            s--;
        }
    }
    return 0;
}

@ @c
str_number maketexstring(const char *s)
{
    if (s == NULL || *s == 0)
        return get_nullstr();
    return maketexlstring(s, strlen(s));
}

@ @c
str_number maketexlstring(const char *s, size_t l)
{
    if (s == NULL || l == 0)
        return get_nullstr();
    str_string(str_ptr) = xmalloc((unsigned) (l + 1));
    memcpy(str_string(str_ptr), s, (l + 1));
    str_length(str_ptr) = (unsigned) l;
    str_ptr++;
    return (str_ptr - 1);
}

@ append a C string to a TeX string
@c
void append_string(const unsigned char *s, unsigned l)
{
    if (s == NULL || *s == 0)
        return;
    l = (unsigned) strlen((const char *) s);
    str_room(l);
    memcpy(cur_string + cur_length, s, l);
    cur_length += l;
    return;
}

@ @c
char *makecstring(int s)
{
    size_t l;
    return makeclstring(s, &l);
}

@ @c
char *makeclstring(int s, size_t * len)
{
    if (s < STRING_OFFSET) {
        *len = (size_t) utf8_size(s);
        return (char *) uni2str((unsigned) s);
    } else {
        unsigned l = (unsigned) str_length(s);
        char *cstrbuf = xmalloc(l + 1);
        memcpy(cstrbuf, str_string(s), l);
        cstrbuf[l] = '\0';
        *len = (size_t) l;
        return cstrbuf;
    }
}

@ @c
int dump_string_pool(void)
{
    int j;
    int l;
    int k = str_ptr;
    dump_int(k - STRING_OFFSET);
    for (j = STRING_OFFSET + 1; j < k; j++) {
        l = (int) str_length(j);
        if (str_string(j) == NULL)
            l = -1;
        dump_int(l);
        if (l > 0)
            dump_things(*str_string(j), str_length(j));
    }
    return (k - STRING_OFFSET);
}

@ @c
int undump_string_pool(void)
{
    int j;
    int x;
    undump_int(str_ptr);
    if (max_strings < str_ptr + strings_free)
        max_strings = str_ptr + strings_free;
    str_ptr += STRING_OFFSET;
    if (ini_version)
        libcfree(string_pool);
    init_string_pool_array((unsigned) max_strings);
    for (j = STRING_OFFSET + 1; j < str_ptr; j++) {
        undump_int(x);
        if (x >= 0) {
            str_length(j) = (unsigned) x;
            pool_size += (unsigned) x;
            str_string(j) = xmallocarray(unsigned char, (unsigned) (x + 1));
            undump_things(*str_string(j), (unsigned) x);
            *(str_string(j) + str_length(j)) = '\0';
        } else {
            str_length(j) = 0;
        }
    }
    init_str_ptr = str_ptr;
    return str_ptr;
}

@ @c
void init_string_pool_array(unsigned s)
{
    string_pool = xmallocarray(lstring, s);
    _string_pool = string_pool - STRING_OFFSET;
    memset(string_pool, 0, s * sizeof(lstring));
    /* seed the null string */
    string_pool[0].s = xmalloc(1);
    string_pool[0].s[0] = '\0';
}

@ To destroy an already made string, we say |flush_str|. 
@c
void flush_str(str_number s)
{
#if 0	
    printf("Flushing a string: %s (s=%d,str_ptr=%d)\n", (char *)str_string(s), (int)s, (int)str_ptr); 
#endif
    if (s > STRING_OFFSET) {    /* don't ever delete the null string */
        pool_size -= (unsigned) str_length(s);
        str_length(s) = 0;
        xfree(str_string(s));
    }
    while (str_string((str_ptr - 1)) == NULL)
        str_ptr--;
}
