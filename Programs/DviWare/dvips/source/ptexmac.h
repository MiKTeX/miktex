/*
Copyright (c) 1996-2012 Han The Thanh, <thanh@pdftex.org>

This file is part of pdfTeX.

pdfTeX is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

pdfTeX is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with pdfTeX; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

$Id: //depot/Build/source.development/TeX/texk/web2c/pdftexdir/ptexmac.h#18 $
*/

#ifndef PDFTEXMAC
#define PDFTEXMAC

#ifdef WIN32
// Why relying on gmalloc() ???
#define gmalloc(n) xmalloc(n)
#define gfree(p) free(p)
#define inline __inline
#define srandom(n) srand(n)
#define random() rand()
#endif

/* Pascal WEB macros */
#define maxinteger 0x7FFFFFFF
#define maxdimen   0x3FFFFFFF

#define objinfo(n) objtab[n].int0

#define pdfroom(n) do {                                 \
    if (pdfbufsize < n)                                 \
        pdftex_fail("PDF output buffer overflowed");    \
    if (pdfptr + n > pdfbufsize)                        \
        pdfflush();                                     \
} while (0)

#define pdfout(c)  do {             \
    if (pdfptr > pdfbufsize)        \
        pdfflush();                 \
    pdfbuf[pdfptr++] = c;           \
} while (0)

#define pdfoffset()     (pdfgone + pdfptr)
#define pdfinitfont(f)  {tmpf = f; pdfcreatefontobj();}

#define MAX_CHAR_CODE       255

#define PRINTF_BUF_SIZE     1024
#define MAX_CSTRING_LEN     1024
#define MAX_PSTRING_LEN     1024
#define SMALL_BUF_SIZE      256
#define SMALL_ARRAY_SIZE    256
#define FONTNAME_BUF_SIZE   128 /* a PDF name can be maximum 127 chars long */

#define check_buf(size, buf_size)                          \
    if ((size) > (buf_size))                               \
        pdftex_fail("buffer overflow", (buf_size))

#define append_char_to_buf(c, p, buf, buf_size) do {       \
    if (c == 9)                                            \
        c = 32;                                            \
    if (c == 13 || c == EOF)                               \
        c = 10;                                            \
    if (c != ' ' || (p > buf && p[-1] != 32)) {            \
        check_buf(p - buf + 1, (buf_size));                \
        *p++ = c;                                          \
    }                                                      \
} while (0)

#define append_eol(p, buf, buf_size) do {                  \
    check_buf(p - buf + 2, (buf_size));                    \
    if (p - buf > 1 && p[-1] != 10)                        \
        *p++ = 10;                                         \
    if (p - buf > 2 && p[-2] == 32) {                      \
        p[-2] = 10;                                        \
        p--;                                               \
    }                                                      \
    *p = 0;                                                \
} while (0)

#define remove_eol(p, buf) do {                            \
    p = strend(buf) - 1;                                   \
    if (*p == 10)                                          \
        *p = 0;                                            \
} while (0)

#define skip(p, c)   if (*p == c)  p++

#define alloc_array(T, n, s) do {                           \
    if (T##_array == NULL) {                                \
        T##_limit = (s);                                    \
        if ((n) > T##_limit)                                \
            T##_limit = (n);                                \
        T##_array = xtalloc(T##_limit, T##_entry);          \
        T##_ptr = T##_array;                                \
    }                                                       \
    else if (T##_ptr - T##_array + (n) > T##_limit) {       \
        last_ptr_index = T##_ptr - T##_array;               \
        T##_limit *= 2;                                     \
        if (T##_ptr - T##_array + (n) > T##_limit)          \
            T##_limit = T##_ptr - T##_array + (n);          \
        xretalloc(T##_array, T##_limit, T##_entry);         \
        T##_ptr = T##_array + last_ptr_index;               \
    }                                                       \
} while (0)

#define is_cfg_comment(c) \
    (c == 10 || c == '*' || c == '#' || c == ';' || c == '%')

#define define_array(T)                     \
T##_entry      *T##_ptr, *T##_array = NULL;    \
size_t          T##_limit

#define xfree(p)            do { if (p != NULL) free(p); p = NULL; } while (0)
#define strend(s)           strchr(s, 0)
#define xtalloc             XTALLOC
#define xretalloc           XRETALLOC

#define ASCENT_CODE         0
#define CAPHEIGHT_CODE      1
#define DESCENT_CODE        2
#define FONTNAME_CODE       3
#define ITALIC_ANGLE_CODE   4
#define STEMV_CODE          5
#define XHEIGHT_CODE        6
#define FONTBBOX1_CODE      7
#define FONTBBOX2_CODE      8
#define FONTBBOX3_CODE      9
#define FONTBBOX4_CODE      10
#define MAX_KEY_CODE        (FONTBBOX1_CODE + 1)
#define FONT_KEYS_NUM       (FONTBBOX4_CODE + 1)

#define F_INCLUDED          0x01
#define F_SUBSETTED         0x02
#define F_TRUETYPE          0x04
#define F_BASEFONT          0x08

#define set_included(fm)    ((fm)->type |= F_INCLUDED)
#define set_subsetted(fm)   ((fm)->type |= F_SUBSETTED)
#define set_truetype(fm)    ((fm)->type |= F_TRUETYPE)
#define set_basefont(fm)    ((fm)->type |= F_BASEFONT)

#define unset_included(fm)  ((fm)->type &= ~F_INCLUDED)
#define unset_subsetted(fm) ((fm)->type &= ~F_SUBSETTED)
#define unset_truetype(fm)  ((fm)->type &= ~F_TRUETYPE)
#define unset_basefont(fm)  ((fm)->type &= ~F_BASEFONT)

#define unset_fontfile(fm)  xfree((fm)->ff_name)

#define is_included(fm)     ((fm)->type & F_INCLUDED)
#define is_subsetted(fm)    ((fm)->type & F_SUBSETTED)
#define is_truetype(fm)     ((fm)->type & F_TRUETYPE)
#define is_basefont(fm)     ((fm)->type & F_BASEFONT)
#define no_font_desc(fm)    (is_basefont(fm) && !is_included(fm))

#define fm_slant(fm)        (fm)->slant
#define fm_extend(fm)       (fm)->extend
#define fm_fontfile(fm)     (fm)->ff_name

#define is_reencoded(fm)    ((fm)->encoding != NULL)
#define is_fontfile(fm)     (fm_fontfile(fm) != NULL)
#define is_t1fontfile(fm)   (is_fontfile(fm) && !is_truetype(fm))

#define LINK_TFM            0x01
#define LINK_PS             0x02
#define set_tfmlink(fm)     ((fm)->links |= LINK_TFM)
#define set_pslink(fm)      ((fm)->links |= LINK_PS)
#define unset_tfmlink(fm)   ((fm)->links &= ~LINK_TFM)
#define unset_pslink(fm)    ((fm)->links &= ~LINK_PS)
#define has_tfmlink(fm)     ((fm)->links & LINK_TFM)
#define has_pslink(fm)      ((fm)->links & LINK_PS)


#define set_cur_file_name(s)      \
    cur_file_name = s;      \
    packfilename(maketexstring(cur_file_name), getnullstr(), getnullstr())

#endif  /* PDFTEXMAC */
