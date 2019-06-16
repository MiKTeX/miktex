/* writecff.h

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


#ifndef _CFF_LIMITS_H_
#  define _CFF_LIMITS_H_

#  include <limits.h>

#  define CFF_INT_MAX 0x7fffffff
#  define CFF_INT_MIN (-0x7fffffff - 1)

#  if (LONG_MAX < CFF_INT_MAX || LONG_MIN > CFF_INT_MIN)
#    error "CFF support won't work on this system."
#  endif

#  define CFF_SID_MAX    64999
#  define CFF_STDSTR_MAX 391

/* Type 2 Charstring */
#  define CS_NUM_SUBR_MAX    65536
#  define CS_STR_LEN_MAX     65536
#  define CS_STEM_ZONE_MAX   96
#  define CS_ARG_STACK_MAX   48
#  define CS_TRANS_ARRAY_MAX 32
#  define CS_SUBR_NEST_MAX   10

#endif                          /* _CFF_LIMITS_H_ */

#ifndef _CFF_TYPES_H_
#  define _CFF_TYPES_H_

/* CFF Data Types */

#  define CFF_TYPE_UNKNOWN 0
#  define CFF_TYPE_INTEGER (1 << 0)
#  define CFF_TYPE_REAL    (1 << 1)
#  define CFF_TYPE_NUMBER  (CFF_TYPE_INTEGER|CFF_TYPE_REAL)
#  define CFF_TYPE_BOOLEAN (1 << 2)
#  define CFF_TYPE_SID     (1 << 3)
#  define CFF_TYPE_ARRAY   (1 << 4)
#  define CFF_TYPE_DELTA   (1 << 5)

/* SID SID number */
#  define CFF_TYPE_ROS     (1 << 6)
/* offset(0) */
#  define CFF_TYPE_OFFSET  (1 << 7)
/* size offset(0) */
#  define CFF_TYPE_SZOFF   (1 << 8)

typedef unsigned char card8;    /* 1-byte unsigned number */
typedef unsigned short card16;  /* 2-byte unsigned number */
typedef unsigned char c_offsize;        /* 1-byte unsigned number specifies the size
                                           of an Offset field or fields, range 1-4 */
typedef unsigned long l_offset; /* 1, 2, 3, or 4-byte offset */
typedef unsigned short s_SID;   /* 2-byte string identifier  */

typedef unsigned long l_size;

typedef struct {
    card16 count;               /* number of objects stored in INDEX */
    c_offsize offsize;          /* Offset array element size, 1-4    */
    l_offset *offset;           /* Offset array, count + 1 offsets   */
    card8 *data;                /* Object data                       */
} cff_index;


/* Dictionary */
typedef struct {
    int id;                     /* encoded data value (as card8 or card16) */
    const char *key;            /* opname                                  */
    int count;                  /* number of values                        */
    double *values;             /* values                                  */
} cff_dict_entry;

typedef struct {
    int max;
    int count;
    cff_dict_entry *entries;
} cff_dict;

/* Encoding, Charset and FDSelect */
typedef struct {
    s_SID first;                /* SID or CID, or card8 for Encoding  */
    card8 n_left;               /* no. of remaining gids/codes in this range */
} cff_range1;

typedef struct {
    s_SID first;                /* SID or CID (card16)      */
    card16 n_left;              /* card16-version of range1 */
} cff_range2;

typedef struct {
    card8 code;
    s_SID glyph;
} cff_map;

typedef struct {
    card8 format;               /* if (format & 0x80) then have supplement */
    card8 num_entries;          /* number of entries */
    union {
        card8 *codes;           /* format 0 */
        cff_range1 *range1;     /* format 1 */
    } data;
    card8 num_supps;            /* number of supplementary data */
    cff_map *supp;              /* supplement */
} cff_encoding;

typedef struct {
    card8 format;
    card16 num_entries;
    union {
        s_SID *glyphs;          /* format 0 */
        cff_range1 *range1;     /* format 1 */
        cff_range2 *range2;     /* format 2 */
    } data;
} cff_charsets;

/* CID-Keyed font specific */
typedef struct {
    card16 first;
    card8 fd;
} cff_range3;

typedef struct {
    card8 format;
    card16 num_entries;         /* number of glyphs/ranges */
    union {
        card8 *fds;             /* format 0 */
        cff_range3 *ranges;     /* format 3 */
    } data;
    /* card16 sentinel; *//* format 3 only, must be equals to num_glyphs */
} cff_fdselect;

#endif                          /* _CFF_TYPES_H_ */

#ifndef _CFF_STDSTR_H_
#  define _CFF_STDSTR_H_

#  define CFF_STDSTR_MAX 391

extern const char *const cff_stdstr[];
#endif                          /* _CFF_STDSTR_H_ */

#ifndef _CFF_H_
#  define _CFF_H_

/*#include "mfileio.h"*/

/* Flag */
#  define FONTTYPE_CIDFONT  (1 << 0)
#  define FONTTYPE_FONT     (1 << 1)
#  define FONTTYPE_MMASTER  (1 << 2)

#  define ENCODING_STANDARD (1 << 3)
#  define ENCODING_EXPERT   (1 << 4)

#  define CHARSETS_ISOADOBE (1 << 5)
#  define CHARSETS_EXPERT   (1 << 6)
#  define CHARSETS_EXPSUB   (1 << 7)

#  define HAVE_STANDARD_ENCODING (ENCODING_STANDARD|ENCODING_EXPERT)
#  define HAVE_STANDARD_CHARSETS \
  (CHARSETS_ISOADOBE|CHARSETS_EXPERT|CHARSETS_EXPSUB)

#  define CFF_STRING_NOTDEF 65535

typedef struct {
    char *fontname;             /* FontName */

    /* - CFF structure - */

    card8 header_major;         /* major version                  */
    card8 header_minor;         /* minor version                  */
    card8 header_hdr_size;      /* Header size (bytes)                  */
    c_offsize header_offsize;   /* Absolute offset (0) size             */

    cff_index *name;            /* Name INDEX */
    cff_dict *topdict;          /* Top DICT (single) */
    cff_index *string;          /* String INDEX */
    cff_index *gsubr;           /* Global Subr INDEX */
    cff_encoding *encoding;     /* Encodings */
    cff_charsets *charsets;     /* Charsets  */
    cff_fdselect *fdselect;     /* FDSelect, CIDFont only */
    cff_index *cstrings;        /* CharStrings */
    cff_dict **fdarray;         /* CIDFont only */
    cff_dict **private;         /* per-Font DICT */
    cff_index **subrs;          /* Local Subr INDEX, per-Private DICT */

    /* -- extra data -- */
    l_offset gsubr_offset;
    card16 num_glyphs;          /* number of glyphs (CharString INDEX count) */
    card8 num_fds;              /* number of Font DICT */

    /* Updated String INDEX.
     * Please fix this. We should separate input and output.
     */
    cff_index *_string;

    unsigned char *stream;
    l_offset offset;
    l_offset stream_size;

    int index;                  /* CFF fontset index */
    int flag;                   /* Flag: see above */
} cff_font;

extern cff_font *cff_open(unsigned char *stream, long stream_size, int n);
extern void cff_close(cff_font * cff);

#  define cff_seek_set(c, p) seek_absolute (((c)->stream), ((c)->offset) + (p));

/* CFF Header */
extern long cff_put_header(cff_font * cff, card8 * dest, long destlen);

/* CFF INDEX */
extern cff_index *cff_get_index(cff_font * cff);
extern cff_index *cff_get_index_header(cff_font * cff);
extern void cff_release_index(cff_index * idx);
extern cff_index *cff_new_index(card16 count);
extern long cff_index_size(cff_index * idx);
extern long cff_pack_index(cff_index * idx, card8 * dest, long destlen);

/* Name INDEX */
extern char *cff_get_name(cff_font * cff);
extern long cff_set_name(cff_font * cff, char *name);

/* Global and Local Subrs INDEX */
extern long cff_read_subrs(cff_font * cff);

/* Encoding */
extern void cff_release_encoding(cff_encoding * encoding);

/* Charsets */
extern long cff_read_charsets(cff_font * cff);
extern long cff_pack_charsets(cff_font * cff, card8 * dest, long destlen);

extern void cff_release_charsets(cff_charsets * charset);
/* Returns SID or CID */

/* FDSelect */
extern long cff_read_fdselect(cff_font * cff);
extern long cff_pack_fdselect(cff_font * cff, card8 * dest, long destlen);
extern card8 cff_fdselect_lookup(cff_font * cff, card16 gid);
extern void cff_release_fdselect(cff_fdselect * fdselect);

/* Font DICT(s) */
extern long cff_read_fdarray(cff_font * cff);

/* Private DICT(s) */
extern long cff_read_private(cff_font * cff);

/* String */
extern int cff_match_string(cff_font * cff, const char *str, s_SID sid);
extern char *cff_get_string(cff_font * cff, s_SID id);
extern long cff_get_sid(cff_font * cff, const char *str);
extern s_SID cff_add_string(cff_font * cff, const char *str);
extern void cff_update_string(cff_font * cff);

#  define cff_is_stdstr(s) (cff_get_sid(NULL, (s)) >= 0)

#endif                          /* _CFF_H_ */

#ifndef _CFF_DICT_H_
#  define _CFF_DICT_H_

#  define CFF_CIDCOUNT_DEFAULT      8720
#  define CFF_NOMINALWIDTHX_DEFAULT 0.0
#  define CFF_DEFAULTWIDTHX_DEFAULT 0.0

extern cff_dict *cff_new_dict(void);
extern void cff_release_dict(cff_dict * dict);

extern void cff_dict_set(cff_dict * dict, const char *key, int idx,
                         double value);
extern double cff_dict_get(cff_dict * dict, const char *key, int idx);
extern void cff_dict_add(cff_dict * dict, const char *key, int count);
extern void cff_dict_remove(cff_dict * dict, const char *key);
extern int cff_dict_known(cff_dict * dict, const char *key);

/* decode/encode DICT */
extern cff_dict *cff_dict_unpack(card8 * data, card8 * endptr);
extern long cff_dict_pack(cff_dict * dict, card8 * dest, long destlen);

extern void cff_dict_update(cff_dict * dict, cff_font * cff);

#endif                          /* _CFF_DICT_H_ */

#ifndef _CS_TYPE2_H_
#  define _CS_TYPE2_H_

typedef struct {
    int flags;                  /* unused in Type 2 charstring */
    double wx, wy;
    struct {
        double llx, lly, urx, ury;
    } bbox;
    struct {
        double asb, adx, ady;
        card8 bchar, achar;
    } seac;                     /* unused in Type 2 charstring */
} cs_ginfo;

#endif                          /* _CS_TYPE2_H_ */

#define cff_is_cidfont(a) (a->flag & FONTTYPE_CIDFONT)
#define cff_understandable(a) (a->header_major==1)

extern cff_font *read_cff(unsigned char *buf, long buflength, int subf);

extern void write_cff(PDF pdf, cff_font * cff, fd_entry * fd);
extern void write_cid_cff(PDF pdf, cff_font * cffont, fd_entry * fd);
