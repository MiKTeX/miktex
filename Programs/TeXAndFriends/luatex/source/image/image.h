/* image.h

   Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
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

#ifndef IMAGE_H
#  define IMAGE_H

#  include <png.h>
#  include "pdf/pdftypes.h"     /* for scaled_whd */

#  define JPG_UINT16      unsigned int
#  define JPG_UINT32      unsigned long
#  define JPG_UINT8       unsigned char

extern int do_zround(double r); /* from utils.w */
extern scaled one_hundred_bp;   /* from pdfgen.w */

#  define bp2sp(p)        do_zround(p * (one_hundred_bp / 100.0))
#  define sp2bp(i)        (i * 100.0 / one_hundred_bp)

#  define TYPE_IMG        "image.meta"
#  define TYPE_IMG_DICT   "image.dict"

typedef struct {
    char *stream;
} pdf_stream_struct;

typedef struct {
    png_structp png_ptr;
    png_infop info_ptr;
} png_img_struct;

typedef struct {
    int color_space;   /* used color space. See JPG_ constants */
    JPG_UINT32 length; /* length of file/data */
} jpg_img_struct;

typedef struct {
    int length;        /* length of file/data */
} jp2_img_struct;

#  if 0
typedef struct {       /* currently unused */
} jb2_img_struct;
#  endif

typedef enum {
    DICT_NEW,          /* fresh dictionary */
    DICT_FILESCANNED,  /* image file scanned */
    DICT_REFERED,      /* pdf_refximage_node in node list --> read-only dict */
    DICT_OUTIMG,       /* /Im* appears in pagestream */
    DICT_SCHEDULED,    /* image dict scheduled for writing (PONR) */
    DICT_WRITTEN       /* image dict written to file */
} dict_state;

typedef enum {
    IMG_TYPE_NONE,
    IMG_TYPE_PDF,
    IMG_TYPE_PNG,
    IMG_TYPE_JPG,
    IMG_TYPE_JP2,
    IMG_TYPE_JBIG2,
    IMG_TYPE_PDFSTREAM,
    IMG_TYPE_PDFMEMSTREAM,
    IMG_TYPE_SENTINEL
} imgtype_e;

typedef enum { IMG_KEEPOPEN, IMG_CLOSEINBETWEEN } img_readtype_e;

typedef enum {
    PDF_BOX_SPEC_NONE,
    PDF_BOX_SPEC_MEDIA,
    PDF_BOX_SPEC_CROP,
    PDF_BOX_SPEC_BLEED,
    PDF_BOX_SPEC_TRIM,
    PDF_BOX_SPEC_ART,
    PDF_BOX_SPEC_SENTINEL
} pdfboxspec_e;

typedef struct {
    int objnum;
    int index;                  /* /Im1, /Im2, ... */
    scaled_whd dimen;           /* TeX dimensions given to \pdfximage */
    int transform;              /* transform given to \pdfximage */
    int x_size;                 /* dimensions in pixel counts as in JPG/PNG/JBIG2 file */
    int y_size;
    int x_orig;                 /* origin in sp for PDF files */
    int y_orig;
    int x_res;                  /* pixel resolution as in JPG/PNG/JBIG2 file */
    int y_res;
    int rotation;               /* rotation (multiples of 90 deg.) for PDF files */
    int colorspace;             /* number of /ColorSpace object */
    int group_ref;              /* if it's <=0, the page has no group */
    int total_pages;
    int page_num;               /* requested page (by number) */
    char *pagename;             /* requested page (by name) */
    char *filename;             /* requested raw file name */
    char *visiblefilename;      /* blocks or overwrites filename as it appears in PDF output */
    char *filepath;             /* full file path after kpathsea */
    char *attr;                 /* additional image dict entries */
    FILE *file;
    imgtype_e image_type;
    int procset;                /* /ProcSet flags */
    int color_depth;            /* color depth */
    pdfboxspec_e page_box_spec; /* PDF page box spec.: media/crop/bleed/trim/art */
    int bbox[4];
    dict_state state;
    int flags;
    int luaref ;
    boolean keepopen;
    int errorlevel;
    int pdfminorversion;
    union {
        pdf_stream_struct *pdfstream;
        png_img_struct *png;
        jpg_img_struct *jpg;
        jp2_img_struct *jp2;
     /* jb2_img_struct *jb2; */
    } img_struct;
} image_dict;

#  define img_objnum(N)           ((N)->objnum)
#  define img_index(N)            ((N)->index)
#  define img_dimen(N)            ((N)->dimen)
#  define img_width(N)            ((N)->dimen.wd)
#  define img_height(N)           ((N)->dimen.ht)
#  define img_depth(N)            ((N)->dimen.dp)
#  define img_transform(N)        ((N)->transform)
#  define img_xsize(N)            ((N)->x_size)
#  define img_ysize(N)            ((N)->y_size)
#  define img_xorig(N)            ((N)->x_orig)
#  define img_yorig(N)            ((N)->y_orig)
#  define img_xres(N)             ((N)->x_res)
#  define img_yres(N)             ((N)->y_res)
#  define img_rotation(N)         ((N)->rotation)
#  define img_colorspace(N)       ((N)->colorspace)
#  define img_group_ref(N)        ((N)->group_ref)
#  define img_totalpages(N)       ((N)->total_pages)
#  define img_pagenum(N)          ((N)->page_num)
#  define img_pagename(N)         ((N)->pagename)
#  define img_filename(N)         ((N)->filename)
#  define img_visiblefilename(N)  ((N)->visiblefilename)
#  define img_filepath(N)         ((N)->filepath)
#  define img_attr(N)             ((N)->attr)
#  define img_file(N)             ((N)->file)
#  define img_type(N)             ((N)->image_type)
#  define img_procset(N)          ((N)->procset)
#  define img_colordepth(N)       ((N)->color_depth)
#  define img_pagebox(N)          ((N)->page_box_spec)
#  define img_bbox(N)             ((N)->bbox)
#  define img_state(N)            ((N)->state)
#  define img_flags(N)            ((N)->flags)
#  define img_luaref(N)           ((N)->luaref)
#  define img_keepopen(N)         ((N)->keepopen)
#  define img_errorlevel(N)       ((N)->errorlevel)
#  define img_pdfminorversion(N)  ((N)->pdfminorversion)

#  define img_pdfstream_ptr(N)    ((N)->img_struct.pdfstream)
#  define img_pdfstream_stream(N) ((N)->img_struct.pdfstream->stream)

#  define img_png_ptr(N)          ((N)->img_struct.png)
#  define img_png_png_ptr(N)      ((N)->img_struct.png->png_ptr)
#  define img_png_info_ptr(N)     ((N)->img_struct.png->info_ptr)

#  define img_jpg_ptr(N)          ((N)->img_struct.jpg)
#  define img_jpg_color(N)        ((N)->img_struct.jpg->color_space)

#  define img_jp2_ptr(N)          ((N)->img_struct.jp2)

#  define img_jb2_ptr(N)          ((N)->img_struct.jb2)

#  define F_FLAG_BBOX             (1 << 0)
#  define F_FLAG_GROUP            (1 << 1)

#  define img_set_bbox(N)         (img_flags(N) |= F_FLAG_BBOX)
#  define img_unset_bbox(N)       (img_flags(N) &= ~F_FLAG_BBOX)
#  define img_is_bbox(N)          ((img_flags(N) & F_FLAG_BBOX) != 0)

#  define img_set_group(N)        (img_flags(N) |= F_FLAG_GROUP)
#  define img_unset_group(N)      (img_flags(N) &= ~F_FLAG_GROUP)
#  define img_is_group(N)         ((img_flags(N) & F_FLAG_GROUP) != 0)

#  define epdf_xsize(a)           img_xsize(idict_array[a])
#  define epdf_ysize(a)           img_ysize(idict_array[a])
#  define epdf_orig_x(a)          img_xorig(idict_array[a])
#  define epdf_orig_y(a)          img_yorig(idict_array[a])

#  define is_pdf_image(a)         ((img_type(idict_array[a]) == IMG_TYPE_PDF) || (img_type(idict_array[a]) == IMG_TYPE_PDFMEMSTREAM))
#  define is_png_image(a)         (img_type(idict_array[a]) == IMG_TYPE_PNG)

#  define img_is_refered(N)       (img_index(N) != -1)

typedef struct {
    scaled_whd dimen; /* requested/actual TeX dimensions */
    int transform;
    image_dict *dict;
    int dict_ref;     /* luaL_ref() reference */
} image;

#  define img_dict(N)             ((N)->dict)
#  define img_dictref(N)          ((N)->dict_ref)

#  define set_wd_running(N)       (img_width(N) = null_flag)
#  define set_ht_running(N)       (img_height(N) = null_flag)
#  define set_dp_running(N)       (img_depth(N) = null_flag)
#  define is_wd_running(N)        (img_width(N) == null_flag)
#  define is_ht_running(N)        (img_height(N) == null_flag)
#  define is_dp_running(N)        (img_depth(N) == null_flag)

#endif
