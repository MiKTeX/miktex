/* pdftypes.h

   Copyright 2009-2013 Taco Hoekwater <taco@luatex.org>

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

#ifndef PDFTYPES_H
#  define PDFTYPES_H

#ifdef HAVE_CONFIG_H
#include <w2c/config.h>
#endif
#  include <zlib.h>
#  include "lua/luatex-api.h"

/*
    The prefix "PTEX" for the PDF keys is special to pdfTeX and has been registered with
    Adobe by Hans Hagen.
*/

#  define pdfkeyprefix "PTEX"

#  define i32round(a) (int) floor((a) + 0.5)
/*#  define i64round(a) (int64_t) floor((a) + 0.5)*/
/*#  define i64round(a) (int64_t) ( (double)(a+0.5) - ((int64_t)(a+0.5))%1  ) */
#ifdef _WIN32
#if defined(MIKTEX_WINDOWS)
#  define i64round(a) (int64_t) lround(a)
#else
#  define i64round(a) (int64_t) win32_floor((a) + 0.5)
#endif
#else
#  define i64round(a) (int64_t) lround((a))
#endif


#  define MAX_OBJ_COMPRESS_LEVEL 3                  /* maximum/clipping value for \pdfobjcompresslevel */
#  define OBJSTM_UNSET -1                           /* initial value */
#  define OBJSTM_ALWAYS 1                           /* \pdfobjcompresslevel >= OBJSTM_ALWAYS: put object into object stream */
#  define OBJSTM_NEVER (MAX_OBJ_COMPRESS_LEVEL + 1) /* above maximum/clipping value for \pdfobjcompresslevel */

typedef int internal_font_number;                  /* |font| in a |char_node| */

typedef enum {
    NO_ZIP,       /* no \.{ZIP} compression */
    ZIP_WRITING,  /* \.{ZIP} compression being used */
    ZIP_FINISH    /* finish \.{ZIP} compression */
} zip_write_state_e;

typedef enum {
    PDFOUT_BUF,
    OBJSTM_BUF,
} buffer_e;

/*
    This stucture holds everything that is needed for the actual pdf generation.

    Because this structure interfaces with C++, it is not wise to use |boolean|
    here (C++ has a boolean type built-in that is not compatible). Also, I have
    plans to convert the backend code into a C library for use with e.g. standalone
    lua. Together, this means that it is best only to use the standard C types and
    the types explicitly defined in this header, and stay away from types like
    |integer| and |eight_bits| that are used elsewhere in the \LUATEX\ sources.

*/

typedef struct {
    int64_t m; /* mantissa (significand) */
    int e;     /* exponent * -1 */
} pdffloat;

typedef struct {
    pdffloat h;
    pdffloat v;
} pdfpos;

#  define scaled int

typedef struct scaledpos_ {
    int64_t h;
    int64_t v;
 } scaledpos;




typedef struct scaled_whd_ {
    scaled wd; /* TeX width */
    scaled ht; /* TeX height */
    scaled dp; /* TeX depth */
} scaled_whd;

typedef struct posstructure_ {
    scaledpos pos; /* position on the page */
    int dir;       /* direction of stuff to be put onto the page */
} posstructure;

typedef struct {
    scaledpos curpos;    /* \pdflastpos position */
    posstructure boxpos; /* box dir and position of the box origin on the page */
    scaled_whd boxdim;   /* box dimensions (in hlist/vlist coordinate system) */
} pos_info_structure;

typedef enum {
    PMODE_NONE,
    PMODE_PAGE,
    PMODE_TEXT,
    PMODE_CHARARRAY,
    PMODE_CHAR
} pos_mode;

typedef enum {
    ST_INITIAL,
    ST_OMODE_FIX,
    ST_FILE_OPEN,
    ST_HEADER_WRITTEN,
    ST_FILE_CLOSED
} output_state;

typedef struct pdf_object_list_ {
    int info;
    struct pdf_object_list_ *link;
} pdf_object_list;

typedef enum {  /* []TJ runs horizontal or vertical */
    WMODE_H,
    WMODE_V
} writing_mode;

typedef struct {
    pdfpos pdf;                 /* pos. on page (PDF page raster) */
    pdfpos pdf_bt_pos;          /* pos. at begin of BT-ET group (PDF page raster) */
    pdfpos pdf_tj_pos;          /* pos. at begin of TJ array (PDF page raster) */
    pdffloat cw;                /* pos. within [(..)..]TJ array (glyph raster); cw.e = fractional digits in /Widths array */
    pdffloat tj_delta;          /* rel. movement in [(..)..]TJ array (glyph raster) */
    pdffloat fs;                /* font size in PDF units */
    pdffloat fs_cur;            /* to check if fs.m has changed and Tf needed */
    pdffloat tm0_cur;           /* to check if tm[0] has changed and Tm needed */
    pdffloat cm[6];             /* cm array */
    pdffloat tm[6];             /* Tm array */
    double k1;                  /* conv. factor from TeX sp to PDF page raster */
    double k2;                  /* conv. factor from PDF page raster to TJ array raster */
    int f_pdf;                  /* /F* font number, of unexpanded base font! */
    int f_pdf_cur;              /* to check if f_pdf has changed and Tf needed */
    writing_mode wmode;         /* PDF writing mode WMode (horizontal/vertical) */
    pos_mode mode;              /* current positioning mode */
    int ishex;                  /* Whether the current char string is <> or () */
    int need_tf;                /* flag whether Tf needs to be set */
    int need_tm;                /* flag whether Tm needs to be set */
    int cur_ex;                 /* the current glyph ex factor */
} pdfstructure;

typedef struct obj_entry_ {
    union {
        int int0;
        char *str0;
    } u;
    int int1;
    off_t int2;
    int int3;
    union {
        int int4;
        char *str4;
    } v;
    int objtype;                /* integer int5 */
} obj_entry;

typedef struct dest_name_entry_ {
    char *objname;              /* destination name */
    int objnum;                 /* destination object number */
} dest_name_entry;

# define pdf_max_link_level 10  /* maximum depth of link nesting */

typedef struct pdf_link_stack_record {
    int nesting_level;
    int link_node;              /* holds a copy of the corresponding |pdf_start_link_node| */
    int ref_link_node;          /* points to original |pdf_start_link_node|, or a
                                   copy of |link_node| created by |append_link| in
                                   case of multi-line link */
} pdf_link_stack_record;

/* types of objects */

/*
    NB: |obj_type_thread| is the highest entry in |head_tab|, but there are a few
    more linked lists that are handy.
*/

typedef enum {
    obj_type_font = 0,          /* index of linked list of Fonts objects */
    obj_type_outline = 1,       /* index of linked list of outline objects */
    obj_type_dest = 2,          /* index of linked list of destination objects */
    obj_type_obj = 3,           /* index of linked list of raw objects */
    obj_type_xform = 4,         /* index of linked list of XObject forms */
    obj_type_ximage = 5,        /* index of linked list of XObject images */
    obj_type_thread = 6,        /* index of linked list of num article threads */
    obj_type_pagestream = 7,    /* Page stream objects */
    obj_type_page = 8,          /* /Page objects */
    obj_type_pages = 9,         /* /Pages objects */
    obj_type_catalog = 10,      /* /Catalog object */
    obj_type_info = 11,         /* /Info object */
    obj_type_link = 12,         /* link objects */
    obj_type_annot = 13,        /* annotation objects */
    obj_type_annots = 14,       /* /Annots objects */
    obj_type_bead = 15,         /* thread bead objects */
    obj_type_beads = 16,        /* /B objects (array of bead objects) */
    obj_type_objstm = 17,       /* /ObjStm objects */
    obj_type_others = 18        /* any other objects (also not linked in any list) */
} pdf_obj_type;

#  define HEAD_TAB_MAX      6   /* obj_type_thread */
#  define PDF_OBJ_TYPE_MAX 18   /* obj_type_others */

typedef struct pdf_resource_struct_ {
    struct avl_table *resources_tree;
    int last_resources;         /* halfword to most recently generated Resources object. */
} pdf_resource_struct;


typedef struct os_obj_data_ {
    int num;
    int off;
} os_obj_data;

typedef struct strbuf_s_ {
    unsigned char *data;        /* a PDF stream buffer */
    unsigned char *p;           /* pointer to the next character in the PDF stream buffer */
    size_t size;                /* currently allocated size of the PDF stream buffer, grows dynamically */
    size_t limit;               /* maximum allowed PDF stream buffer size */
} strbuf_s;

typedef struct os_struct_ {
    os_obj_data *obj;           /* array of object stream objects */
    strbuf_s *buf[3];
    buffer_e curbuf;            /* select into which buffer to output */
    unsigned int cur_objstm;    /* number of current object stream object */
    unsigned int idx;           /* index of object within object stream [1...PDF_OS_MAX_OBJS - 1] */
    unsigned int ostm_ctr;      /* statistics: counter for object stream objects */
    unsigned int o_ctr;         /* statistics: counter for objects within object streams */
} os_struct;


#  define packet_max_recursion 100 /* see |packet_cur_s| */
#  define packet_stack_size    100

typedef struct packet_stack_record_ {
    float c0;
    float c1;
    float c2;
    float c3;
    scaledpos pos;              /* c4, c5 */
} packet_stack_record;

typedef struct vf_struct_ {
    packet_stack_record *packet_stack;  /* for "push" and "pop" */
    int packet_stack_level;
    int packet_stack_minlevel;  /* to check stack underflow */
    internal_font_number lf;    /* local font number, resolved */
    int fs_f;                   /* local font size */
    int ex_glyph;               /* expansion value; ex_glyph = 0 means unexpanded */
    int packet_cur_s;           /* do_vf_packet() nesting level */
    posstructure *refpos;
    int vflua;                  /* flag, whether vf.*() functions are allowed */
} vf_struct;

typedef struct pdf_output_file_ {
    FILE *file;                 /* the PDF output file handle */
    char *file_name;            /* the PDF output file name */
    char *job_name;
    output_mode o_mode;         /* output mode (DVI/PDF/...) */
    output_state o_state;
    /* generation parameters */
    int gamma;
    int image_gamma;
    int image_hicolor;          /* boolean */
    int image_apply_gamma;
    int draftmode;
    int pk_resolution;
    int pk_fixed_dpi;
    int decimal_digits;
    int gen_tounicode;
    int omit_cidset;
    int inclusion_copy_font;
    int minor_version;          /* fixed minor part of the PDF version */
    int compress_level;         /* level for zlib object stream compression */
    int objcompresslevel;       /* fixed level for activating PDF object streams */
    char *job_id_string;        /* the full job string */

    int os_enable;              /* true if object streams are globally enabled */
    os_struct *os;              /* object stream structure pointer */

    strbuf_s *buf;              /* pointer to the current stream buffer (PDF stream, ObjStm, or Lua) */

    off_t save_offset;          /* to save |pdf_offset| */
    off_t gone;                 /* number of bytes that were flushed to output */

    char *printf_buf;           /* a scratch buffer for |pdf_printf| */

    time_t start_time;          /* when this job started */
    char *start_time_str;       /* minimum size for time_str is 24: "D:YYYYmmddHHMMSS+HH'MM'" */

    strbuf_s *fb;               /* pointer to auxiliary buffer for font stuff */

    char *zipbuf;
    z_stream *c_stream;         /* compression stream pointer */
    zip_write_state_e zip_write_state;  /* which state of compression we are in */
    int stream_deflate;         /* true, if stream dict has /Filter/FlateDecode */
    int stream_writing;         /* true while writing stream */

    int pk_scale_factor;        /* this is just a preprocessed value that depends on |pk_resolution| and |decimal_digits| */

    int img_page_group_val;     /* page group information pointer from included pdf or png images */
    char *resname_prefix;       /* global prefix of resources name */

    int mem_size;               /* allocated size of |mem| array */
    int *mem;
    int mem_ptr;

    pdfstructure *pstruct;      /* utity structure keeping position status in PDF page stream */
    posstructure *posstruct;    /* structure for positioning within page */

    int obj_tab_size;           /* allocated size of |obj_tab| array */
    obj_entry *obj_tab;
    int head_tab[HEAD_TAB_MAX + 1];     /* heads of the object lists in |obj_tab| */
    struct avl_table *obj_tree[PDF_OBJ_TYPE_MAX + 1];   /* this is useful for finding the objects back */

    int pages_tail;
    int obj_ptr;                /* objects counter */
    int last_pages;             /* pointer to most recently generated pages object */
    int last_page;              /* pointer to most recently generated page object */
    int last_stream;            /* pointer to most recently generated stream */
    off_t stream_length;        /* length of most recently generated stream */
    off_t stream_length_offset; /* file offset of the last stream length */
    int seek_write_length;      /* flag whether to seek back and write \.{/Length} */
    int last_byte;              /* byte most recently written to PDF file; for \.{endstream} in new line */

    int obj_count;
    int xform_count;
    int ximage_count;

    pdf_resource_struct *page_resources;

    scaledpos page_size;        /* width and height of page being shipped */

    /* the variables from pdfdest */
    int dest_names_size;
    int dest_names_ptr;
    dest_name_entry *dest_names;
    /* the (static) variables from pdfoutline */
    int first_outline;
    int last_outline;
    int parent_outline;
    /* the pdf link stack */
    pdf_link_stack_record link_stack[(pdf_max_link_level + 1)];
    int link_stack_ptr;
    /* the thread data */
    int last_thread;            /* pointer to the last thread */
    scaled_whd thread;
    int last_thread_id;         /* identifier of the last thread */
    int last_thread_named_id;   /* is identifier of the last thread named */
    int thread_level;           /* depth of nesting of box containing the last thread */

    int f_cur;                  /* TeX font number */
    int cave;                   /* stay away from previous PDF object */

    vf_struct *vfstruct;
} pdf_output_file;

typedef pdf_output_file *PDF;

#endif
