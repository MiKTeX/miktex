/*

   Copyright 2008-2013 Taco Hoekwater <taco@luatex.org>

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

#ifndef LUATEXFONT_H

#define LUATEXFONT_H

#include "ptexlib.h"
#include "sfnt.h"

#define ASCENT_CODE       0
#define CAPHEIGHT_CODE    1
#define DESCENT_CODE      2
#define ITALIC_ANGLE_CODE 3
#define STEMV_CODE        4
#define XHEIGHT_CODE      5
#define FONTBBOX1_CODE    6
#define FONTBBOX2_CODE    7
#define FONTBBOX3_CODE    8
#define FONTBBOX4_CODE    9
#define FONTNAME_CODE     10
#define GEN_KEY_NUM       (XHEIGHT_CODE + 1)
#define MAX_KEY_CODE      (FONTBBOX1_CODE + 1)
#define INT_KEYS_NUM      (FONTBBOX4_CODE + 1)
#define FONT_KEYS_NUM     (FONTNAME_CODE + 1)

#define FD_FLAGS_NOT_SET_IN_MAPLINE -1
#define FD_FLAGS_DEFAULT_EMBED  4          /* a symbol font */
#define FD_FLAGS_DEFAULT_NON_EMBED 0x22    /* a nonsymbolic serif font */

typedef struct {
    const char *pdfname;
    const char *t1name;
    boolean valid;
} key_entry;

extern const key_entry font_key[FONT_KEYS_NUM];

#include "mapfile.h"

typedef struct {
    int val;                         /* value */
    boolean set;                     /* true if parameter has been set */
} intparm;

typedef struct {
    int fe_objnum;                   /* object number */
    char *name;                      /* encoding file name */
    char **glyph_names;              /* array of glyph names */
    struct avl_table *tx_tree;       /* tree of encoding positions marked as used by TeX */
} fe_entry;

typedef struct fd_entry_ {
    int fd_objnum;                   /* object number of the font descriptor object */
    char *fontname;                  /* /FontName (without subset tag) */
    char *subset_tag;                /* 6-character subset tag */
    boolean ff_found;
    int ff_objnum;                   /* object number of the font program stream */
    boolean all_glyphs;              /* embed all glyphs? */
    boolean write_ttf_glyph_names;
    intparm font_dim[FONT_KEYS_NUM];
    fe_entry *fe;                    /* pointer to encoding structure */
    char **builtin_glyph_names;      /* builtin encoding as read from the Type1 font file */
    fm_entry *fm;                    /* pointer to font map structure */
    struct avl_table *tx_tree;       /* tree of non-reencoded TeX characters marked as used */
    struct avl_table *gl_tree;       /* tree of all marked glyphs */
    internal_font_number tex_font;   /* needed for variable */
} fd_entry;

typedef struct fo_entry_ {
    int fo_objnum;                   /* object number of the font dictionary */
    internal_font_number tex_font;   /* needed only for \pdffontattr{} */
    fm_entry *fm;                    /* pointer to font map structure for this font dictionary */
    fd_entry *fd;                    /* pointer to /FontDescriptor object structure */
    fe_entry *fe;                    /* pointer to encoding structure */
    int cw_objnum;                   /* object number of the font program object */
    int first_char;                  /* first character used in this font */
    int last_char;                   /* last character used in this font */
    struct avl_table *tx_tree;       /* tree of non-reencoded TeX characters marked as used */
    int tounicode_objnum;            /* object number of ToUnicode */
} fo_entry;

typedef struct {
    char *name;                      /* glyph name */
    long code;                       /* -1 = undefined; -2 = multiple codes, stored as string in unicode_seq; otherwise unicode value */
    char *unicode_seq;               /* multiple unicode sequence */
} glyph_unicode_entry;

typedef struct glw_entry_ {          /* subset glyphs for inclusion in CID-based fonts */
    unsigned int id;                 /* glyph CID */
    signed int wd;                   /* glyph width in 1/1000 em parts */
} glw_entry;

typedef struct {
    int charcode, cwidth, cheight, xoff, yoff, xescape, rastersize;
    halfword *raster;
} chardesc;

#include "texfont.h"

/* tounicode.c */

int write_cid_tounicode(PDF, fo_entry *, internal_font_number);
void glyph_unicode_free(void);
void def_tounicode(str_number, str_number);
int write_tounicode(PDF, char **, char *);

/* vfpacket.c */

void replace_packet_fonts(internal_font_number f, int *old_fontid, int *new_fontid, int count);
int *packet_local_fonts(internal_font_number f, int *num);

int packet_cur_s;               /* current |do_vf_packet()| recursion level */
int packet_stack_ptr;           /* pointer into |packet_stack| */
vf_struct *new_vfstruct(void);

/* writecff.c */

void writetype1w(PDF pdf, fd_entry * fd);

/* writetype0.c */

void writetype0(PDF pdf, fd_entry * fd);

/* writefont.c */

void do_pdf_font(PDF, internal_font_number);
fd_entry *lookup_fd_entry(char *);
fd_entry *new_fd_entry(internal_font_number);
void write_fontstuff(PDF);
void register_fd_entry(fd_entry * fd);

/* writet1.c */

boolean t1_subset(char *, char *, unsigned char *);
char **load_enc_file(char *);
void writet1(PDF, fd_entry *, int wide);
void t1_free(void);
extern int t1_length1, t1_length2, t1_length3;

extern int t1_wide_mode;

/* writetype2.c */

boolean writetype2(PDF, fd_entry *);
extern unsigned long cidtogid_obj;
unsigned long ttc_read_offset(sfnt * sfont, int ttc_idx, fd_entry *fd);

/* writeenc.c */

fe_entry *get_fe_entry(char *);
void enc_free(void);
void write_fontencodings(PDF pdf);

/* writettf.c */

void writettf(PDF, fd_entry *);
void writeotf(PDF, fd_entry *);
void ttf_free(void);
extern int ttf_length;

/* pkin.c */

int readchar(boolean, chardesc *);

/* macnames.c */

extern char notdef[];

/* vfovf.c */

internal_font_number letter_space_font(internal_font_number f, int e, boolean nolig);
void pdf_check_vf(internal_font_number f);
internal_font_number copy_font_info(internal_font_number f);

/* writet3.c */

extern FILE *t3_file;
void writet3(PDF, internal_font_number);

extern unsigned char *t3_buffer;
extern int t3_size;
extern int t3_curbyte;

#define t3_read_file() readbinfile(t3_file, &t3_buffer, &t3_size)
#define t3_close()     xfclose(t3_file, cur_file_name)
#define t3_getchar()   t3_buffer[t3_curbyte++]
#define t3_eof()       (t3_curbyte>t3_size)
#define t3_prefix(s)   (!strncmp(t3_line_array, s, strlen(s)))
#define t3_putchar(c)  pdfout(c)

#endif
