/* sfnt.h

   Copyright 2002 by Jin-Hwan Cho and Shunsaku Hirata,
   the dvipdfmx project team <dvipdfmx@project.ktug.or.kr>
   Copyright 2006-2008 Taco Hoekwater <taco@luatex.org>

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


#ifndef _SFNT_H_
#  define _SFNT_H_

#  if  HAVE_CONFIG_H
#    include <w2c/config.h>
#  endif                        /* HAVE_CONFIG_H_ */

/* Data Types as described in Apple's TTRefMan */

/*typedef unsigned char BYTE;*/ /* defined in pdfgen.h */
typedef signed char ICHAR;
typedef unsigned short USHORT;
typedef signed short SHORT;
/*typedef unsigned long ULONG;*//* defined in pdfgen.h */
typedef signed long LONG;
typedef unsigned long Fixed;    /* 16.16-bit signed fixed-point number */
typedef short FWord;
typedef unsigned short uFWord;
typedef short F2Dot14;          /* 16-bit signed fixed number with the low 14 bits representing fraction. */

struct sfnt_table {
    /* table header */
    char tag[4];
    ULONG check_sum;
    ULONG offset;
    ULONG length;
    char *data;                 /* table data */
};

#  define SFNT_TABLE_REQUIRED (1 << 0)

struct sfnt_table_directory {
    ULONG version;              /* Fixed for Win */
    USHORT num_tables;
    USHORT search_range;
    USHORT entry_selector;
    USHORT range_shift;
    USHORT num_kept_tables;     /* number of kept tables */
    char *flags;                /* keep or omit */
    struct sfnt_table *tables;
};

/* sfnt resource */
#  define SFNT_TYPE_TRUETYPE   (1 << 0)
#  define SFNT_TYPE_OPENTYPE   (1 << 1)
#  define SFNT_TYPE_POSTSCRIPT (1 << 2)
#  define SFNT_TYPE_TTC        (1 << 4)

typedef struct {
    int type;
    struct sfnt_table_directory *directory;
    BYTE *buffer;
    long buflen;
    long loc;
} sfnt;

/* Convert sfnt "fixed" type to double */

#  define fixed(a) ((double)((a)%0x10000L)/(double)(0x10000L) + \
 (a)/0x10000L - (((a)/0x10000L > 0x7fffL) ? 0x10000L : 0))

BYTE get_unsigned_byte(sfnt * f);
ICHAR get_signed_byte(sfnt * f);
USHORT get_unsigned_pair(sfnt * f);
SHORT get_signed_pair(sfnt * f);
ULONG get_unsigned_quad(sfnt * f);
int do_sfnt_read(unsigned char *dest, int len, sfnt * f);

#define sfnt_get_byte(s)   ((BYTE)   get_unsigned_byte(s))
#define sfnt_get_char(s)   ((ICHAR)   get_signed_byte  (s))
#define sfnt_get_ushort(s) ((USHORT) get_unsigned_pair(s))
#define sfnt_get_short(s)  ((SHORT)  get_signed_pair  (s))
#define sfnt_get_ulong(s)  ((ULONG)  get_unsigned_quad(s))
#define sfnt_get_long(s)   ((LONG)   get_signed_quad  (s))

#define sfnt_seek_set(s,o) (s)->loc = (o)
#define sfnt_read(b,l,s)   do_sfnt_read((b), (l), (s))

extern int put_big_endian(void *s, LONG q, int n);

#define sfnt_put_ushort(s,v) put_big_endian((s), v, 2);
#define sfnt_put_short(s,v)  put_big_endian((s), v, 2);
#define sfnt_put_ulong(s,v)  put_big_endian((s), v, 4);
#define sfnt_put_long(s,v)   put_big_endian((s), v, 4);

extern sfnt *sfnt_open(unsigned char *buffer, int buflen);
extern void sfnt_close(sfnt * sfont);

/* table directory */

extern int sfnt_read_table_directory(sfnt * sfont, ULONG offset);
extern ULONG sfnt_find_table_len(sfnt * sfont, const char *tag);
extern ULONG sfnt_find_table_pos(sfnt * sfont, const char *tag);
extern ULONG sfnt_locate_table(sfnt * sfont, const char *tag);

extern void sfnt_set_table(sfnt * sfont,
                           const char *tag, void *data, ULONG length);
extern int sfnt_require_table(sfnt * sfont, const char *tag, int must_exist);

#define ASSERT(a) assert(a)
#define RELEASE(a) free(a)
#define NEW(a,b) xmalloc((unsigned)((unsigned)(a)*sizeof(b)))
#define RENEW(a,b,c) xrealloc(a, (unsigned)((unsigned)(b)*sizeof(c)))

extern pdf_obj *sfnt_create_FontFile_stream(sfnt * sfont);

#endif                          /* _SFNT_H_ */
