/* pdfgen.h

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

#ifndef PDFGEN_H
#  define PDFGEN_H

#  define PROCSET_PDF     (1 << 0)
#  define PROCSET_TEXT    (1 << 1)
#  define PROCSET_IMAGE_B (1 << 2)
#  define PROCSET_IMAGE_C (1 << 3)
#  define PROCSET_IMAGE_I (1 << 4)

#  define inf_pdf_mem_size    10000 /* min size of the |mem| array */
#  define sup_pdf_mem_size 10000000 /* max size of the |mem| array */

extern PDF static_pdf;

extern int pdf_get_mem(PDF pdf, int s);

/*

We use the similiar subroutines to handle the output buffer for PDF output. When
compress is used, the state of writing to buffer is held in |zip_write_state|. We
must write the header of PDF output file in initialization to ensure that it will
be the first written bytes.

*/

#  define inf_pdfout_buf_size   16384  /* initial value of |pdf->buf| size */
#  define sup_pdfout_buf_size   16384  /* arbitrary upper hard limit of |pdf->buf| size */
#  define inf_objstm_buf_size       1  /* initial value of |os->buf[OBJSTM_BUF]| size */
#  define sup_objstm_buf_size 5000000  /* arbitrary upper hard limit of |os->buf[OBJSTM_BUF]| size */

#  define PDF_OS_MAX_OBJS         100  /* maximum number of objects in object stream */

#  define inf_obj_tab_size       1000  /* min size of the cross-reference table for PDF output */
#  define sup_obj_tab_size    8388607  /* max size of the cross-reference table for PDF output */

/* The following macros are similar as for \.{DVI} buffer handling */

#  define pdf_offset(pdf) (pdf->gone + (off_t)(pdf->buf->p - pdf->buf->data))
#  define pdf_save_offset(pdf) pdf->save_offset = (pdf->gone + (off_t)(pdf->buf->p - pdf->buf->data))

#  define set_ff(A) do { \
    if (pdf_font_num(A) < 0) \
        ff = -pdf_font_num(A); \
    else \
        ff = A; \
} while (0)

typedef enum {  /* needs pdf_prefix */
    NOT_SHIPPING,
    SHIPPING_PAGE,
    SHIPPING_FORM
} shipping_mode_e;

extern scaled one_hundred_inch;
extern scaled one_inch;
extern scaled one_true_inch;
extern scaled one_hundred_bp;
extern scaled one_bp;
extern int ten_pow[10];

extern void pdf_flush(PDF);
extern void pdf_room(PDF, int);

extern void fix_pdf_minorversion(PDF);

/* output a byte to PDF buffer without checking of overflow */

#  define pdf_quick_out(pdf,A) * (pdf->buf->p++) = (unsigned char) (A)

/* do the same as |pdf_quick_out| and flush the PDF buffer if necessary */

#  define pdf_out(pdf,A) do { \
    pdf_room(pdf, 1); \
    pdf_quick_out(pdf, A); \
} while (0)

/*

Basic printing procedures for PDF output are very similiar to \TeX\ basic
printing ones but the output is going to PDF buffer. Subroutines with suffix
|_ln| append a new-line character to the PDF output.

*/

#  define pdf_print_ln(pdf,A) do { \
    pdf_print(pdf,A); \
    pdf_out(pdf, '\n'); \
} while (0)

/* print out an integer to PDF buffer followed by a new-line character */

#  define pdf_print_int_ln(pdf,A) do { \
    pdf_print_int(pdf,A); \
    pdf_out(pdf, '\n'); \
} while (0)

extern __attribute__ ((format(printf, 2, 3)))
void pdf_printf(PDF, const char *, ...);

extern void pdf_print(PDF, str_number);
extern void pdf_print_int(PDF, longinteger);
extern void print_pdffloat(PDF pdf, pdffloat f);
extern void pdf_print_str(PDF, const char *);

extern void pdf_add_null(PDF);
extern void pdf_add_bool(PDF, int i);
extern void pdf_add_int(PDF, int i);
extern void pdf_add_longint(PDF, longinteger n);
extern void pdf_add_ref(PDF, int num);
extern void pdf_add_string(PDF, const char *s);
extern void pdf_add_name(PDF, const char *name);

extern void pdf_dict_add_bool(PDF, const char *key, int i);
extern void pdf_dict_add_int(PDF, const char *key, int i);
extern void pdf_dict_add_ref(PDF, const char *key, int num);
extern void pdf_dict_add_name(PDF, const char *key, const char *val);
extern void pdf_dict_add_string(PDF pdf, const char *key, const char *val);
extern void pdf_dict_add_streaminfo(PDF);

extern void pdf_begin_stream(PDF);
extern void pdf_end_stream(PDF);

extern void pdf_add_bp(PDF, scaled);

extern strbuf_s *new_strbuf(size_t size, size_t limit);
extern void strbuf_seek(strbuf_s * b, off_t offset);
extern size_t strbuf_offset(strbuf_s * b);
extern void strbuf_putchar(strbuf_s * b, unsigned char c);
extern void strbuf_flush(PDF pdf, strbuf_s * b);
extern void strbuf_free(strbuf_s * b);

/* This is for the resource lists */

extern void addto_page_resources(PDF pdf, pdf_obj_type t, int k);
extern pdf_object_list *get_page_resources_list(PDF pdf, pdf_obj_type t);

extern void pdf_out_block(PDF pdf, const char *s, size_t n);

#  define pdf_puts(pdf, s) pdf_out_block((pdf), (s), strlen(s))

#  define pdf_print_resname_prefix(pdf) do { \
    if (pdf->resname_prefix != NULL) \
        pdf_puts(pdf, pdf->resname_prefix); \
} while (0)

extern void pdf_print_str_ln(PDF, const char *);

extern void pdf_print_toks(PDF, halfword);

extern void pdf_add_rect_spec(PDF, halfword);
extern void pdf_rectangle(PDF, halfword);

extern void pdf_begin_obj(PDF, int, int);
extern void pdf_end_obj(PDF);

extern void pdf_begin_dict(PDF);
extern void pdf_end_dict(PDF);
extern void pdf_begin_array(PDF);
extern void pdf_end_array(PDF);

extern void remove_pdffile(PDF);

extern void zip_free(PDF);

/* functions that do not output stuff */

extern scaled round_xn_over_d(scaled x, int n, unsigned int d);

extern char *convertStringToPDFString(const char *in, int len);

extern void init_start_time(PDF);
extern char *getcreationdate(PDF);

extern void check_o_mode(PDF pdf, const char *s, int o_mode, boolean errorflag);

extern void set_job_id(PDF, int, int, int, int);
extern char *get_resname_prefix(PDF);
extern void pdf_begin_page(PDF pdf);
extern void pdf_end_page(PDF pdf);
extern void print_pdf_table_string(PDF pdf, const char *s);
extern const char *get_pdf_table_string(const char *s);
extern int get_pdf_table_bool(PDF, const char *, int);

extern void ensure_output_state(PDF pdf, output_state s);
extern PDF init_pdf_struct(PDF pdf);

extern halfword pdf_info_toks;          /* additional keys of Info dictionary */
extern halfword pdf_catalog_toks;       /* additional keys of Catalog dictionary */
extern halfword pdf_catalog_openaction;
extern halfword pdf_names_toks;         /* additional keys of Names dictionary */
extern halfword pdf_trailer_toks;       /* additional keys of Trailer dictionary */
extern void scan_pdfcatalog(PDF pdf);
extern void finish_pdf_file(PDF pdf, int luatex_version, str_number luatex_revision);

extern shipping_mode_e global_shipping_mode;

#endif
