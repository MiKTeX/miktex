/* texfont.h Main font API implementation for the pascal parts

   Copyright 2006-2010 Taco Hoekwater <taco@luatex.org>

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


/* Here we have the interface to LuaTeX's font system, as seen from the
   main pascal program. There is a companion list in luatex.defines to
   keep web2c happy */

/*
    This file is read at the end of ptexlib.h, which is called for at
    the end of luatexcoerce.h, as well as from the C sources.
*/

#ifndef TEXFONT_H
#  define TEXFONT_H

#  define pointer halfword

#  define FONT_SLANT_MIN   -2000
#  define FONT_SLANT_MAX    2000
#  define FONT_EXTEND_MIN  -5000
#  define FONT_EXTEND_MAX   5000
#  define FONT_SQUEEZE_MIN -5000
#  define FONT_SQUEEZE_MAX  5000
#  define FONT_MODE_MIN        0
#  define FONT_MODE_MAX        3 /* pdf values */
#  define FONT_WIDTH_MIN       0
#  define FONT_WIDTH_MAX    5000

/* these are dumped en block, so they need endianness tests */

typedef struct liginfo {
#  ifdef WORDS_BIGENDIAN
    int adj;
    int lig;
    int type;
#  else
    int type;
    int lig;
    int adj;
#  endif
} liginfo;

/* these are dumped en block, so they need endianness tests */

typedef struct kerninfo {
#  ifdef WORDS_BIGENDIAN
    int adj;
    scaled sc;
#  else
    scaled sc;
    int adj;
#  endif
} kerninfo;

typedef struct extinfo {
    struct extinfo *next;
    int glyph;
    int start_overlap;
    int end_overlap;
    int advance;
    int extender;
} extinfo;

/* todo: maybe create a 'math info structure' */

typedef struct charinfo {
    char *name;                 /* postscript character name */
    liginfo *ligatures;         /* ligature items */
    kerninfo *kerns;            /* kern items */
    eight_bits *packets;        /* virtual commands.  */
    unsigned short index;       /* CID index */
    int remainder;              /* spare value for odd items, could be union-ed with extensible */
    scaled width;               /* width */
    scaled height;              /* height */
    scaled depth;               /* depth */
    scaled italic;              /* italic correction */
    scaled vert_italic;         /* italic correction */
    scaled top_accent;          /* top accent alignment */
    scaled bot_accent;          /* bot accent alignment */
    int ef;                     /* font expansion factor */
    int lp;                     /* left protruding factor */
    int rp;                     /* right protruding factor */
    char tag;                   /* list / ext taginfo */
    char used;                  /* char is typeset ? */
    char *tounicode;            /* unicode equivalent */
    extinfo *hor_variants;      /* horizontal variants */
    extinfo *vert_variants;     /* vertical variants */
    int top_left_math_kerns;
    int top_right_math_kerns;
    int bottom_right_math_kerns;
    int bottom_left_math_kerns;
    scaled *top_left_math_kern_array;
    scaled *top_right_math_kern_array;
    scaled *bottom_right_math_kern_array;
    scaled *bottom_left_math_kern_array;
} charinfo;

#  define EXT_NORMAL 0
#  define EXT_REPEAT 1

extern extinfo *get_charinfo_vert_variants(charinfo * ci);
extern extinfo *get_charinfo_hor_variants(charinfo * ci);
extern void set_charinfo_hor_variants(charinfo * ci, extinfo * ext);
extern void set_charinfo_vert_variants(charinfo * ci, extinfo * ext);
extern void add_charinfo_vert_variant(charinfo * ci, extinfo * ext);
extern void add_charinfo_hor_variant(charinfo * ci, extinfo * ext);

extern extinfo *copy_variants(extinfo * o);

extern extinfo *new_variant(int glyph, int startconnect, int endconnect, int advance, int repeater);

extern scaled_whd get_charinfo_whd(internal_font_number f, int c);

typedef struct texfont {
    int _font_size;
    int _font_dsize;
    int _font_units_per_em;
    char *_font_name;
    char *_font_area;
    char *_font_filename;
    char *_font_fullname;
    char *_font_psname;
    char *_font_encodingname;
    char *_font_cidregistry;
    char *_font_cidordering;
    int _font_cidversion;
    int _font_cidsupplement;

    int _font_ec;
    unsigned _font_checksum;    /* internal information */
    char _font_used;            /* internal information */
    char _font_touched;         /* internal information */
    int _font_cache_id;         /* internal information */
    char _font_encodingbytes;   /* 1 or 2 bytes */
    boolean _font_oldmath;      /* default to false when MathConstants seen */
    int _font_slant;            /* a slant in ppt */
    int _font_extend;           /* an extension in ppt, or 1000 */
    int _font_squeeze;          /* an extension in ppt, or 1000 */
    int _font_width;
    int _font_mode;
    int font_max_shrink;
    int font_max_stretch;
    int _font_step;             /* amount of one step of expansion */

    char _font_tounicode;       /* 1 if info is present */
    fm_entry *_font_map;
    int _font_type;
    int _font_format;
    int _font_writingmode;
    int _font_identity;
    int _font_embedding;
    int _font_streamprovider;
    int _font_bc;
    int _hyphen_char;
    int _skew_char;
    int _font_natural_dir;

    charinfo *_left_boundary;
    charinfo *_right_boundary;

    int _font_params;
    scaled *_param_base;

    int _font_math_params;
    scaled *_math_param_base;

    sa_tree characters;
    int charinfo_count;
    int charinfo_size;
    charinfo *charinfo;
    int *charinfo_cache;
    int ligatures_disabled;

    int _pdf_font_num;          /* maps to a PDF resource ID */
    str_number _pdf_font_attr;  /* pointer to additional attributes */
} texfont;

typedef enum {
    unknown_font_type = 0,      /* new font (has not been used yet) */
    virtual_font_type,          /* virtual font */
    real_font_type,             /* real font */
} font_types;

typedef enum {
    unknown_format = 0,
    type1_format,
    type3_format,
    truetype_format,
    opentype_format,
} font_formats;

typedef enum {
    unknown_writingmode = 0,
    horizontal_writingmode,
    vertical_writingmode,
} writingmode_types;

typedef enum {
    unknown_identity = 0,
    horizontal_identity,
    vertical_identity,
} identity_types;

typedef enum {
    unknown_embedding = 0,
    no_embedding,
    subset_embedding,
    full_embedding,
} font_embedding_option;

extern const char *font_type_strings[];
extern const char *font_format_strings[];
extern const char *font_writingmodes_strings[];
extern const char *font_identity_strings[];
extern const char *font_embedding_strings[];

#  define font_checksum(a)           font_tables[a]->_font_checksum
#  define set_font_checksum(a,b)     font_checksum(a) = b

#  define font_check_0(a)            ((font_tables[a]->_font_checksum&0xFF000000)>>24)
#  define font_check_1(a)            ((font_tables[a]->_font_checksum&0x00FF0000)>>16)
#  define font_check_2(a)            ((font_tables[a]->_font_checksum&0x0000FF00)>>8)
#  define font_check_3(a)             (font_tables[a]->_font_checksum&0x000000FF)

#  define font_size(a)               font_tables[a]->_font_size
#  define set_font_size(a,b)         font_size(a) = b
#  define font_dsize(a)              font_tables[a]->_font_dsize
#  define set_font_dsize(a,b)        font_dsize(a) = b

#  define font_units_per_em(a)       font_tables[a]->_font_units_per_em
#  define set_font_units_per_em(a,b) font_units_per_em(a) = b

#  define font_name(a)               font_tables[a]->_font_name
#  define get_font_name(a)           (unsigned char *)font_name(a)
#  define set_font_name(f,b)         font_name(f) = b
#  define tex_font_name(a)           maketexstring(font_name(a))

#  define font_area(a)               font_tables[a]->_font_area
#  define get_font_area(a)           (unsigned char *)font_area(a)
#  define set_font_area(f,b)         font_area(f) = b
#  define tex_font_area(a)           maketexstring(font_area(a))

boolean cmp_font_area(int, str_number);

#  define font_reassign(a,b)             { if (a!=NULL) free(a); a = b; }

#  define font_filename(a)               font_tables[a]->_font_filename
#  define set_font_filename(f,b)         font_reassign(font_filename(f),b)

#  define font_fullname(a)               font_tables[a]->_font_fullname
#  define set_font_fullname(f,b)         font_reassign(font_fullname(f),b)

#  define font_psname(a)                 font_tables[a]->_font_psname
#  define set_font_psname(f,b)           font_reassign(font_psname(f),b)

#  define font_encodingname(a)           font_tables[a]->_font_encodingname
#  define set_font_encodingname(f,b)     font_reassign(font_encodingname(f),b)

#  define cmp_font_filename(a,b)         (!(font_filename(a)!=NULL || font_filename(b)!=NULL || \
                                          strcmp(font_filename(a),font_filename(b))))
#  define cmp_font_fullname(a,b)         (!(font_fullname(a)!=NULL || font_fullname(b)!=NULL || \
                                          strcmp(font_fullname(a),font_fullname(b))))
#  define cmp_font_encodingname(a,b)     (!(font_encoding(a)!=NULL || font_encodingname(b)!=NULL || \
                                          strcmp(font_encodingname(a),font_encodingname(b))))
#  define font_bc(a)                     font_tables[a]->_font_bc
#  define set_font_bc(f,b)               font_bc(f) = b

#  define font_ec(a)                     font_tables[a]->_font_ec
#  define set_font_ec(f,b)               font_ec(f) = b

#  define font_used(a)                   (font_tables[a]!=NULL && font_tables[a]->_font_used)
#  define set_font_used(a,b)             font_tables[a]->_font_used = b

#  define font_touched(a)                font_tables[a]->_font_touched
#  define set_font_touched(a,b)          font_touched(a) = b

#  define font_type(a)                   font_tables[a]->_font_type
#  define set_font_type(a,b)             font_type(a) = b;

#  define font_format(a)                 font_tables[a]->_font_format
#  define font_format_name(a)            font_format_strings[font_tables[a]->_font_format]
#  define set_font_format(a,b)           font_format(a) = b

#  define font_writingmode(a)            font_tables[a]->_font_writingmode
#  define font_writingmode_name(a)       font_writingmode_strings[font_tables[a]->_font_writingmode]
#  define set_font_writingmode(a,b)      font_writingmode(a) = b

#  define font_identity(a)               font_tables[a]->_font_identity
#  define font_identity_name(a)          font_identity_strings[font_tables[a]->_font_identity]
#  define set_font_identity(a,b)         font_identity(a) = b

#  define font_embedding(a)              font_tables[a]->_font_embedding
#  define set_font_embedding(a,b)        font_embedding(a) = b

#  define font_cidversion(a)             font_tables[a]->_font_cidversion
#  define set_font_cidversion(a,b)       font_cidversion(a) = b

#  define font_cidsupplement(a)          font_tables[a]->_font_cidsupplement
#  define set_font_cidsupplement(a,b)    font_cidsupplement(a) = b

#  define font_cidordering(a)            font_tables[a]->_font_cidordering
#  define set_font_cidordering(f,b)      font_reassign(font_cidordering(f),b)

#  define font_cidregistry(a)            font_tables[a]->_font_cidregistry
#  define set_font_cidregistry(f,b)      font_reassign(font_cidregistry(f),b)

#  define font_map(a)                    font_tables[a]->_font_map
#  define set_font_map(a,b)              font_map(a) = b

#  define font_cache_id(a)               font_tables[a]->_font_cache_id
#  define set_font_cache_id(a,b)         font_cache_id(a) = b

#  define font_encodingbytes(a)          font_tables[a]->_font_encodingbytes
#  define set_font_encodingbytes(a,b)    font_encodingbytes(a) = b

#  define font_streamprovider(a)         font_tables[a]->_font_streamprovider
#  define set_font_streamprovider(a,b)   font_streamprovider(a) = b


#  define font_oldmath(a)                font_tables[a]->_font_oldmath
#  define set_font_oldmath(a,b)          font_oldmath(a) = b

#  define font_slant(a)                  font_tables[a]->_font_slant
#  define set_font_slant(a,b)            font_slant(a) = b

#  define font_extend(a)                 font_tables[a]->_font_extend
#  define set_font_extend(a,b)           font_extend(a) = b

#  define font_squeeze(a)                font_tables[a]->_font_squeeze
#  define set_font_squeeze(a,b)          font_squeeze(a) = b

#  define font_width(a)                  font_tables[a]->_font_width
#  define set_font_width(a,b)            font_width(a) = b

#  define font_mode(a)                   font_tables[a]->_font_mode
#  define set_font_mode(a,b)             font_mode(a) = b

#  define font_shrink(a)                 font_tables[a]->_font_shrink
#  define set_font_shrink(a,b)           font_shrink(a) = b

#  define font_stretch(a)                font_tables[a]->_font_stretch
#  define set_font_stretch(a,b)          font_stretch(a) = b

#  define font_max_shrink(a)             font_tables[a]->font_max_shrink
#  define set_font_max_shrink(a,b)       font_max_shrink(a) = b

#  define font_max_stretch(a)            font_tables[a]->font_max_stretch
#  define set_font_max_stretch(a,b)      font_max_stretch(a) = b

#  define font_step(a)                   font_tables[a]->_font_step
#  define set_font_step(a,b)             font_step(a) = b

#  define font_tounicode(a)              font_tables[a]->_font_tounicode
#  define set_font_tounicode(a,b)        font_tounicode(a) = b

#  define hyphen_char(a)                 font_tables[a]->_hyphen_char
#  define set_hyphen_char(a,b)           hyphen_char(a) = b

#  define skew_char(a)                   font_tables[a]->_skew_char
#  define set_skew_char(a,b)             skew_char(a) = b

#  define font_natural_dir(a)            font_tables[a]->_font_natural_dir
#  define set_font_natural_dir(a,b)      font_natural_dir(a) = b

#  define pdf_font_num(a)                font_tables[a]->_pdf_font_num
#  define set_pdf_font_num(a,b)          pdf_font_num(a) = b

#  define pdf_font_attr(a)               font_tables[a]->_pdf_font_attr
#  define set_pdf_font_attr(a,b)         pdf_font_attr(a) = b

#  define left_boundarychar  -1
#  define right_boundarychar -2
#  define non_boundarychar   -3

#  define left_boundary(a)        font_tables[a]->_left_boundary
#  define has_left_boundary(a)    (left_boundary(a)!=NULL)
#  define set_left_boundary(a,b)  font_reassign(left_boundary(a),b)

#  define right_boundary(a)       font_tables[a]->_right_boundary
#  define has_right_boundary(a)   (right_boundary(a)!=NULL)
#  define set_right_boundary(a,b) font_reassign(right_boundary(a),b)

#  define font_bchar(a) (right_boundary(a)!=NULL ? right_boundarychar : non_boundarychar)

/* font parameters */

#  define font_params(a)  font_tables[a]->_font_params
#  define param_base(a)   font_tables[a]->_param_base
#  define font_param(a,b) font_tables[a]->_param_base[b]

extern void set_font_params(internal_font_number f, int b);

#  define set_font_param(f,n,b) { \
    if (font_params(f)<n) \
        set_font_params(f,n); \
    font_param(f,n) = b; \
}
#  define font_math_params(a)  font_tables[a]->_font_math_params
#  define math_param_base(a)   font_tables[a]->_math_param_base
#  define font_math_param(a,b) font_tables[a]->_math_param_base[b]

extern void set_font_math_params(internal_font_number f, int b);

#  define set_font_math_param(f,n,b) { \
    if (font_math_params(f)<n) \
        set_font_math_params(f,n); \
    font_math_param(f,n) = b; \
}

/* Font parameters are sometimes referred to as |slant(f)|, |space(f)|, etc.*/

typedef enum {
    slant_code = 1,
    space_code = 2,
    space_stretch_code = 3,
    space_shrink_code = 4,
    x_height_code = 5,
    quad_code = 6,
    extra_space_code = 7
} font_parameter_codes;

#  define slant(f)         font_param(f,slant_code)
#  define space(f)         font_param(f,space_code)
#  define space_stretch(f) font_param(f,space_stretch_code)
#  define space_shrink(f)  font_param(f,space_shrink_code)
#  define x_height(f)      font_param(f,x_height_code)
#  ifdef quad
#    undef quad
#  endif
#  define quad(f)          font_param(f,quad_code)
#  define extra_space(f)   font_param(f,extra_space_code)

/* now for characters  */

typedef enum {
    top_right_kern = 1,
    bottom_right_kern = 2,
    bottom_left_kern = 3,
    top_left_kern = 4
} font_math_kern_codes;

extern charinfo *get_charinfo(internal_font_number f, int c);
extern int char_exists(internal_font_number f, int c);
extern int lua_glyph_not_found_callback(internal_font_number f, int c);
extern charinfo *char_info(internal_font_number f, int c);

/*
    Here is a quick way to test if a glyph exists, when you are
    already certain the font |f| exists, and that the |c| is a regular
    glyph id, not one of the two special boundary objects.
*/

#  define quick_char_exists(f,c) (get_sa_item(font_tables[f]->characters,c).int_value)

extern void set_charinfo_width(charinfo * ci, scaled val);
extern void set_charinfo_height(charinfo * ci, scaled val);
extern void set_charinfo_depth(charinfo * ci, scaled val);
extern void set_charinfo_italic(charinfo * ci, scaled val);
extern void set_charinfo_vert_italic(charinfo * ci, scaled val);
extern void set_charinfo_top_accent(charinfo * ci, scaled val);
extern void set_charinfo_bot_accent(charinfo * ci, scaled val);
extern void set_charinfo_tag(charinfo * ci, scaled val);
extern void set_charinfo_remainder(charinfo * ci, scaled val);
extern void set_charinfo_used(charinfo * ci, scaled val);
extern void set_charinfo_index(charinfo * ci, scaled val);
extern void set_charinfo_name(charinfo * ci, char *val);
extern void set_charinfo_tounicode(charinfo * ci, char *val);
extern void set_charinfo_ligatures(charinfo * ci, liginfo * val);
extern void set_charinfo_kerns(charinfo * ci, kerninfo * val);
extern void set_charinfo_packets(charinfo * ci, eight_bits * val);
extern void set_charinfo_extensible(charinfo * ci, int a, int b, int c, int d);
extern void set_charinfo_ef(charinfo * ci, scaled val);
extern void set_charinfo_lp(charinfo * ci, scaled val);
extern void set_charinfo_rp(charinfo * ci, scaled val);

extern void add_charinfo_math_kern(charinfo * ci, int type, scaled ht, scaled krn);
extern int get_charinfo_math_kerns(charinfo * ci, int id);

#  define set_char_used(f,a,b) do { \
    if (char_exists(f,a)) \
        set_charinfo_used(char_info(f,a),b); \
} while (0)

extern scaled get_charinfo_width(charinfo * ci);
extern scaled get_charinfo_height(charinfo * ci);
extern scaled get_charinfo_depth(charinfo * ci);
extern scaled get_charinfo_italic(charinfo * ci);
extern scaled get_charinfo_vert_italic(charinfo * ci);
extern scaled get_charinfo_top_accent(charinfo * ci);
extern scaled get_charinfo_bot_accent(charinfo * ci);
extern char get_charinfo_tag(charinfo * ci);
extern int get_charinfo_remainder(charinfo * ci);
extern char get_charinfo_used(charinfo * ci);
extern int get_charinfo_index(charinfo * ci);
extern char *get_charinfo_name(charinfo * ci);
extern char *get_charinfo_tounicode(charinfo * ci);
extern liginfo *get_charinfo_ligatures(charinfo * ci);
extern kerninfo *get_charinfo_kerns(charinfo * ci);
extern eight_bits *get_charinfo_packets(charinfo * ci);
extern int get_charinfo_ef(charinfo * ci);
extern int get_charinfo_rp(charinfo * ci);
extern int get_charinfo_lp(charinfo * ci);
extern int get_charinfo_extensible(charinfo * ci, int which);

extern int ext_top(internal_font_number f, int c);
extern int ext_bot(internal_font_number f, int c);
extern int ext_rep(internal_font_number f, int c);
extern int ext_mid(internal_font_number f, int c);

#  define set_ligature_item(f,b,c,d) { f.type = b; f.adj = c;  f.lig = d; }
#  define set_kern_item(f,b,c) { f.adj = b;  f.sc = c; }

/* character information */

#  define non_char    65536 /* a code that can't match a real character */
#  define non_address     0 /* a spurious |bchar_label| */

/* character kerns and ligatures */

#  define end_kern     0x7FFFFF /* otherchar value meaning "stop" */
#  define ignored_kern 0x800000 /* otherchar value meaning "disabled" */

#  define charinfo_kern(b,c) b->kerns[c]

#  define kern_char(b)       (b).adj
#  define kern_kern(b)       (b).sc
#  define kern_end(b)        ((b).adj == end_kern)
#  define kern_disabled(b)   ((b).adj > end_kern)

/* character ligatures */

#  define end_ligature     0x7FFFFF /* otherchar value meaning "stop" */
#  define ignored_ligature 0x800000 /* otherchar value meaning "disabled" */

#  define charinfo_ligature(b,c)     b->ligatures[c]

#  define is_valid_ligature(a)   ((a).type!=0)
#  define lig_type(a)            ((a).type>>1)
#  define lig_char(a)            (a).adj
#  define lig_replacement(a)     (a).lig
#  define lig_end(a)             (lig_char(a) == end_ligature)
#  define lig_disabled(a)        (lig_char(a) > end_ligature)

#  define no_tag   0 /* vanilla character */
#  define lig_tag  1 /* character has a ligature/kerning program */
#  define list_tag 2 /* character has a successor in a charlist */
#  define ext_tag  3 /* character is extensible */

extern scaled char_height(internal_font_number f, int c);
extern scaled calc_char_width(internal_font_number f, int c, int ex);
extern scaled char_width(internal_font_number f, int c);
extern scaled char_depth(internal_font_number f, int c);
extern scaled char_italic(internal_font_number f, int c);
extern scaled char_vert_italic(internal_font_number f, int c);
extern scaled char_top_accent(internal_font_number f, int c);
extern scaled char_bot_accent(internal_font_number f, int c);

extern liginfo *char_ligatures(internal_font_number f, int c);
extern kerninfo *char_kerns(internal_font_number f, int c);
extern eight_bits *char_packets(internal_font_number f, int c);

#  define has_lig(f,b)    (char_exists(f,b) &&( char_ligatures(f,b) != NULL))
#  define has_kern(f,b)   (char_exists(f,b) && (char_kerns(f,b) != NULL))
#  define has_packet(f,b) (char_exists(f,b) && (char_packets(f,b) != NULL))

extern int char_remainder(internal_font_number f, int c);
extern char char_tag(internal_font_number f, int c);
extern char char_used(internal_font_number f, int c);
extern char *char_name(internal_font_number f, int c);
extern int char_index(internal_font_number f, int c);

scaled raw_get_kern(internal_font_number f, int lc, int rc);
scaled get_kern(internal_font_number f, int lc, int rc);
liginfo get_ligature(internal_font_number f, int lc, int rc);

#  define EXT_TOP 0
#  define EXT_BOT 1
#  define EXT_MID 2
#  define EXT_REP 3

extern texfont **font_tables;

int new_font(void);
extern void font_malloc_charinfo(internal_font_number f, int num);
int copy_font(int id);
int scale_font(int id, int atsize);
int max_font_id(void);
void set_max_font_id(int id);
int new_font_id(void);
void create_null_font(void);
void delete_font(int id);
boolean is_valid_font(int id);

void dump_font(int font_number);
void undump_font(int font_number);

int test_no_ligatures(internal_font_number f);
void set_no_ligatures(internal_font_number f);

extern int get_tag_code(internal_font_number f, int c);
extern int get_lp_code(internal_font_number f, int c);
extern int get_rp_code(internal_font_number f, int c);
extern int get_ef_code(internal_font_number f, int c);

extern void set_tag_code(internal_font_number f, int c, int i);
extern void set_lp_code(internal_font_number f, int c, int i);
extern void set_rp_code(internal_font_number f, int c, int i);
extern void set_ef_code(internal_font_number f, int c, int i);

int read_tfm_info(internal_font_number f, const char *nom, scaled s);

/* from dofont.c */

extern int read_font_info(pointer u, char *cnom, scaled s, int ndir);
extern int find_font_id(const char *nom, scaled s);

/* for and from vfpacket.c */

typedef enum { packet_char_code,
    packet_font_code,
    packet_pop_code,
    packet_push_code,
    packet_special_code,
    packet_image_code,
    packet_right_code,
    packet_down_code,
    packet_rule_code,
    packet_node_code,
    packet_nop_code,
    packet_end_code,
    packet_scale_code,
    packet_lua_code,
    packet_pdf_code,
    packet_pdf_mode
} packet_command_codes;

extern scaled store_scaled_f(scaled sq, int fw);

extern void do_vf_packet(PDF pdf, internal_font_number vf_f, int c, int ex);
extern int vf_packet_bytes(charinfo * co);

extern charinfo *copy_charinfo(charinfo * ci);

/* this function is in vfovf.c for the moment */

extern int make_vf_table(lua_State * L, const char *name, scaled s);

/* some bits of the old interface, used by e.g. writet3.c */

#  define get_x_height(f) x_height(f)
#  define get_quad(f) quad(f)
#  define get_slant(f) slant(f)
#  define get_charwidth(f,c) (char_exists(f,c) ? char_width(f,c) : 0)
#  define get_charheight(f,c) (char_exists(f,c) ? char_height(f,c) : 0)
#  define get_chardepth(f,c) (char_exists(f,c) ? char_depth(f,c) : 0)

extern int pk_dpi; /* PK pixel density value from \.{texmf.cnf} */

extern internal_font_number tfm_lookup(char *s, scaled fs);

extern int fix_expand_value(internal_font_number f, int e);

extern void set_expand_params(internal_font_number f, int stretch_limit, int shrink_limit, int font_step);

extern void read_expand_font(void);
extern void new_letterspaced_font(small_number a);
extern void make_font_copy(small_number a);

extern void glyph_to_unicode(void);

#endif
