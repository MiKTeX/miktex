/* pdftables.h

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

#ifndef PDFTABLES_H
#  define PDFTABLES_H

extern const char *pdf_obj_typenames[PDF_OBJ_TYPE_MAX + 1];

typedef enum {
    union_type_int,
    union_type_cstring,
} union_type;

typedef struct {
    union {
        int int0;
        char *str0;
    } u;
    union_type u_type; /* integer or char * in union above */
    int objptr;
} oentry;

/*

The cross-reference table |obj_tab| is an array of |obj_tab_size| of |obj_entry|.
Each entry contains five integer fields and represents an object in PDF file
whose object number is the index of this entry in |obj_tab|. Objects in |obj_tab|
maybe linked into list; objects in such a linked list have the same type.

The first field contains information representing identifier of this object. It
is usally a number for most of object types, but it may be a string number for
named destination or named thread.

The second field of |obj_entry| contains link to the next object in |obj_tab| if
this object is linked in a list.

The third field holds the byte offset of the object in the output PDF file, or
its byte offset within an object stream. As long as the object is not written,
this field is used for flags about the write status of the object; then it has a
negative value.

The fourth field holds the object number of the object stream, into which the
object is included.

The last field usually represents the pointer to some auxiliary data structure
depending on the object type; however it may be used as a counter as well.

*/

#  define obj_info(pdf,A)            pdf->obj_tab[(A)].u.int0   /* information representing identifier of this object */
#  define obj_start(pdf,A)           pdf->obj_tab[(A)].u.str0
#  define obj_link(pdf,A)            pdf->obj_tab[(A)].int1     /* link to the next entry in linked list */

#  define obj_offset(pdf,A)          pdf->obj_tab[(A)].int2     /* negative (flags), or byte offset for this object in PDF output file, or ... */
#  define obj_os_objnum(pdf,A)       pdf->obj_tab[(A)].int2     /* ... object stream number for this object */
#  define obj_os_idx(pdf,A)          pdf->obj_tab[(A)].int3     /* index of this object in object stream */
#  define obj_aux(pdf,A)             pdf->obj_tab[(A)].v.int4   /* auxiliary pointer */
#  define obj_stop(pdf,A)            pdf->obj_tab[(A)].v.str4
#  define obj_type(pdf,A)            pdf->obj_tab[(A)].objtype

#  define obj_data_ptr               obj_aux                    /* pointer to |pdf->mem| */

#  define set_obj_link(pdf,A,B)      obj_link(pdf,A)=(B)
#  define set_obj_start(pdf,A,B)     obj_start(pdf,A)=(B)
#  define set_obj_info(pdf,A,B)      obj_info(pdf,A)=(B)
#  define set_obj_offset(pdf,A,B)    obj_offset(pdf,A)=(B)
#  define set_obj_os_objnum(pdf,A,B) obj_offset(pdf,A)=(B)
#  define set_obj_aux(pdf,A,B)       obj_aux(pdf,A)=(B)
#  define set_obj_stop(pdf,A,B)      obj_stop(pdf,A)=(B)
#  define set_obj_data_ptr(pdf,A,B)  obj_data_ptr(pdf,A)=(B)

#  define set_obj_fresh(pdf,A)       obj_offset(pdf,(A))=(off_t)-2
#  define set_obj_scheduled(pdf,A)   if (obj_offset(pdf,A)==(off_t)-2) obj_offset(pdf,A)=(off_t)-1
#  define is_obj_scheduled(pdf,A)    ((obj_offset(pdf,A))>(off_t)-2)
#  define is_obj_written(pdf,A)      ((obj_offset(pdf,A))>(off_t)-1)

/*
    NOTE: The data structure definitions for the nodes on the typesetting side are
    inside |nodes.h|
*/

#  define inf_pk_dpi   72 /* min PK pixel density value from \.{texmf.cnf} */
#  define sup_pk_dpi 8000 /* max PK pixel density value from \.{texmf.cnf} */

#  define pdf2double(a) ((double) (a).m / ten_pow[(a).e])

#  define by_one_bp ((double) 65536 * (double) 72.27 / 72)  /* number of sp per 1bp */

extern int find_obj(PDF pdf, int t, int i, boolean byname);
extern void check_obj_exists(PDF pdf, int objnum);
extern void check_obj_type(PDF pdf, int t, int objnum);
extern int pdf_get_obj(PDF pdf, int t, int i, boolean byname);
extern int pdf_create_obj(PDF pdf, int t, int i);
extern void set_rect_dimens(PDF pdf, halfword p, halfword parent_box, scaledpos cur, scaled_whd alt_rule);
extern void libpdffinish(PDF);

#  define set_width(A,B)  width(A)=(B)
#  define set_height(A,B) height(A)=(B)
#  define set_depth(A,B)  depth(A)=(B)

/* pdf backend definitions */

typedef enum {
    c_pdf_compress_level = 0,
    c_pdf_decimal_digits,
    c_pdf_image_resolution,
    c_pdf_pk_resolution,
    c_pdf_unique_resname,
    c_pdf_major_version,
    c_pdf_minor_version,
    c_pdf_pagebox,
    c_pdf_inclusion_errorlevel,
    c_pdf_ignore_unknown_images,
    c_pdf_gamma,
    c_pdf_image_apply_gamma,
    c_pdf_image_gamma,
    c_pdf_image_hicolor,
    c_pdf_image_addfilename,
    c_pdf_obj_compress_level,
    c_pdf_inclusion_copy_font,
    c_pdf_gen_tounicode,
    c_pdf_pk_fixed_dpi,
    c_pdf_suppress_optional_info,
    c_pdf_omit_cidset,
    c_pdf_recompress,
    c_pdf_omit_charset,
    c_pdf_omit_infodict,
    c_pdf_omit_mediabox,
    c_pdf_linking,
    c_pdf_omit_procset,
    c_pdf_ptex_prefix,
} pdf_backend_counters ;

typedef enum {
    d_pdf_h_origin = 0,
    d_pdf_v_origin,
    d_pdf_thread_margin,
    d_pdf_dest_margin,
    d_pdf_link_margin,
    d_pdf_xform_margin,
} pdf_backend_dimensions ;

typedef enum {
    t_pdf_page_attr = 0,
    t_pdf_page_resources,
    t_pdf_pages_attr,
    t_pdf_xform_attr,
    t_pdf_xform_resources,
    t_pdf_pk_mode,
    t_pdf_trailer_id,
} pdf_backend_tokenlists ;

extern int pdf_last_annot;
extern int pdf_last_link;
extern int pdf_last_obj;
extern int pdf_retval;
extern int pdf_cur_form;

#  define pdf_compress_level            get_tex_extension_count_register(c_pdf_compress_level)
#  define pdf_obj_compress_level        get_tex_extension_count_register(c_pdf_obj_compress_level)
#  define pdf_decimal_digits            get_tex_extension_count_register(c_pdf_decimal_digits)
#  define pdf_image_resolution          get_tex_extension_count_register(c_pdf_image_resolution)
#  define pdf_pk_resolution             get_tex_extension_count_register(c_pdf_pk_resolution)
#  define pdf_unique_resname            get_tex_extension_count_register(c_pdf_unique_resname)
#  define pdf_major_version             get_tex_extension_count_register(c_pdf_major_version)
#  define pdf_minor_version             get_tex_extension_count_register(c_pdf_minor_version)
#  define pdf_pagebox                   get_tex_extension_count_register(c_pdf_pagebox)
#  define pdf_inclusion_errorlevel      get_tex_extension_count_register(c_pdf_inclusion_errorlevel)
#  define pdf_ignore_unknown_images     get_tex_extension_count_register(c_pdf_ignore_unknown_images)
#  define pdf_gamma                     get_tex_extension_count_register(c_pdf_gamma)
#  define pdf_image_apply_gamma         get_tex_extension_count_register(c_pdf_image_apply_gamma)
#  define pdf_image_gamma               get_tex_extension_count_register(c_pdf_image_gamma)
#  define pdf_image_hicolor             get_tex_extension_count_register(c_pdf_image_hicolor)
#  define pdf_image_addfilename         get_tex_extension_count_register(c_pdf_image_addfilename)
#  define pdf_inclusion_copy_font       get_tex_extension_count_register(c_pdf_inclusion_copy_font)
#  define pdf_gen_tounicode             get_tex_extension_count_register(c_pdf_gen_tounicode)
#  define pdf_pk_fixed_dpi              get_tex_extension_count_register(c_pdf_pk_fixed_dpi)
#  define pdf_suppress_optional_info    get_tex_extension_count_register(c_pdf_suppress_optional_info)
#  define pdf_omit_cidset               get_tex_extension_count_register(c_pdf_omit_cidset)
#  define pdf_omit_charset              get_tex_extension_count_register(c_pdf_omit_charset)
#  define pdf_omit_infodict             get_tex_extension_count_register(c_pdf_omit_infodict)
#  define pdf_omit_mediabox             get_tex_extension_count_register(c_pdf_omit_mediabox)
#  define pdf_recompress                get_tex_extension_count_register(c_pdf_recompress)
#  define pdf_linking                   get_tex_extension_count_register(c_pdf_linking)
#  define pdf_omit_procset              get_tex_extension_count_register(c_pdf_omit_procset)
#  define pdf_ptex_prefix               get_tex_extension_count_register(c_pdf_ptex_prefix)

#  define pdf_h_origin                  get_tex_extension_dimen_register(d_pdf_h_origin)
#  define pdf_v_origin                  get_tex_extension_dimen_register(d_pdf_v_origin)
#  define pdf_thread_margin             get_tex_extension_dimen_register(d_pdf_thread_margin)
#  define pdf_dest_margin               get_tex_extension_dimen_register(d_pdf_dest_margin)
#  define pdf_link_margin               get_tex_extension_dimen_register(d_pdf_link_margin)
#  define pdf_xform_margin              get_tex_extension_dimen_register(d_pdf_xform_margin)

#  define pdf_page_attr                 get_tex_extension_toks_register(t_pdf_page_attr)
#  define pdf_page_resources            get_tex_extension_toks_register(t_pdf_page_resources)
#  define pdf_pages_attr                get_tex_extension_toks_register(t_pdf_pages_attr)
#  define pdf_xform_attr                get_tex_extension_toks_register(t_pdf_xform_attr)
#  define pdf_xform_resources           get_tex_extension_toks_register(t_pdf_xform_resources)
#  define pdf_pk_mode                   get_tex_extension_toks_register(t_pdf_pk_mode)
#  define pdf_trailer_id                get_tex_extension_toks_register(t_pdf_trailer_id)

#  define set_pdf_major_version(i)      set_tex_extension_count_register(c_pdf_major_version,i)
#  define set_pdf_minor_version(i)      set_tex_extension_count_register(c_pdf_minor_version,i)
#  define set_pdf_compress_level(i)     set_tex_extension_count_register(c_pdf_compress_level,i)
#  define set_pdf_obj_compress_level(i) set_tex_extension_count_register(c_pdf_obj_compress_level,i)
#  define set_pdf_omit_cidset(i)        set_tex_extension_count_register(c_pdf_omit_cidset,i)
#  define set_pdf_omit_charset(i)       set_tex_extension_count_register(c_pdf_omit_charset,i)
#  define set_pdf_omit_infodict(i)      set_tex_extension_count_register(c_pdf_omit_infodict,i)
#  define set_pdf_omit_mediabox(i)      set_tex_extension_count_register(c_pdf_omit_mediabox,i)
#  define set_pdf_gen_tounicode(i)      set_tex_extension_count_register(c_pdf_gen_tounicode,i)
#  define set_pdf_recompress(i)         set_tex_extension_count_register(c_pdf_recompress,i)
#  define set_pdf_linking(i)            set_tex_extension_count_register(c_pdf_linking,i)
#  define set_pdf_omit_procset(i)       set_tex_extension_count_register(c_pdf_omit_procset,i)
#  define set_pdf_ptex_prefix(i)        set_tex_extension_count_register(c_pdf_ptex_prefix,i)

#  define set_pdf_decimal_digits(i)     set_tex_extension_count_register(c_pdf_decimal_digits,i)
#  define set_pdf_pk_resolution(i)      set_tex_extension_count_register(c_pdf_pk_resolution,i)
#  define set_pdf_pk_fixed_dpi(i)       set_tex_extension_count_register(c_pdf_pk_fixed_dpi,i)

#  define pk_decimal_digits(pdf,delta) ((pdf->decimal_digits > 4 ? 4 : 3) + delta)

#endif
