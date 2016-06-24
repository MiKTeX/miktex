% writet3.w
%
% Copyright 1996-2006 Han The Thanh <thanh@@pdftex.org>
% Copyright 2006-2011 Taco Hoekwater <taco@@luatex.org>
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
#include <kpathsea/tex-glyph.h>
#include <kpathsea/magstep.h>
#include <string.h>

#define T3_BUF_SIZE   1024

typedef char t3_line_entry;
define_array(t3_line);

FILE *t3_file;
static boolean t3_image_used;

static int t3_char_procs[256];
static float t3_char_widths[256];
static int t3_glyph_num;
static float t3_font_scale;
static int t3_b0, t3_b1, t3_b2, t3_b3;
static boolean is_pk_font;

/* not static because used by pkin.c  */
unsigned char *t3_buffer = NULL;
int t3_size = 0;
int t3_curbyte = 0;

#define t3_check_eof() \
    if (t3_eof()) \
        normal_error("type 3","unexpected end of file");

@
@c
static void update_bbox(int llx, int lly, int urx, int ury, boolean is_first_glyph)
{
    if (is_first_glyph) {
        t3_b0 = llx;
        t3_b1 = lly;
        t3_b2 = urx;
        t3_b3 = ury;
    } else {
        if (llx < t3_b0)
            t3_b0 = llx;
        if (lly < t3_b1)
            t3_b1 = lly;
        if (urx > t3_b2)
            t3_b2 = urx;
        if (ury > t3_b3)
            t3_b3 = ury;
    }
}

/* fixed precision 3 (+5 in pdfgen.w)*/

#define get_pk_font_scale(pdf,f,scale_factor) \
    divide_scaled(scale_factor, divide_scaled(font_size(f),one_hundred_bp,pk_decimal_digits(pdf,2)), 0)

#define pk_char_width(pdf,f,w,scale_factor) \
    divide_scaled(divide_scaled(w,font_size(f),pk_decimal_digits(pdf,4)), get_pk_font_scale(pdf,f,scale_factor), 0)

@
@c
static boolean writepk(PDF pdf, internal_font_number f)
{
    kpse_glyph_file_type font_ret;
    int llx, lly, urx, ury;
    int cw, rw, i, j;
    pdffloat pf;
    halfword *row;
    char *name;
    chardesc cd;
    boolean is_null_glyph, check_preamble;
    int dpi = 0;
    int newdpi = 0;
    int callback_id = 0;
    int file_opened = 0;
    xfree(t3_buffer);
    t3_curbyte = 0;
    t3_size = 0;

    callback_id = callback_defined(find_pk_file_callback);

    if (pdf->pk_fixed_dpi) {
        newdpi = pdf->pk_resolution;
    } else {
        newdpi = dpi;
    }

    if (callback_id > 0) {
        /* <base>.dpi/<fontname>.<tdpi>pk */
        dpi = round((float) pdf->pk_resolution * (((float) font_size(f)) / (float) font_dsize(f)));
        cur_file_name = font_name(f);
        run_callback(callback_id, "Sd->S", cur_file_name, (int) newdpi, &name);
        if (name == NULL || strlen(name) == 0) {
            formatted_warning("type 3","font %s at %i not found", cur_file_name, (int) dpi);
            return false;
        }
    } else {
        dpi = (int) kpse_magstep_fix(
            (unsigned) round ((float)pdf->pk_resolution * ((float)font_size(f)/(float)font_dsize(f))),
            (unsigned) pdf->pk_resolution, NULL
        );
        cur_file_name = font_name(f);
        name = kpse_find_pk(cur_file_name, (unsigned) dpi, &font_ret);
        if (name == NULL || !FILESTRCASEEQ(cur_file_name, font_ret.name)
            || !kpse_bitmap_tolerance((float) font_ret.dpi, (float) dpi)) {
                formatted_error("type 3","font %s at %i not found", cur_file_name, (int) dpi);
                return false;
        }
    }
    callback_id = callback_defined(read_pk_file_callback);
    if (callback_id > 0) {
        if (!(run_callback(callback_id, "S->bSd", name, &file_opened, &t3_buffer, &t3_size) && file_opened && t3_size > 0)) {
            formatted_warning("type 3","font %s at %i not found", cur_file_name, (int) dpi);
            cur_file_name = NULL;
            return false;
        }
    } else {
        t3_file = xfopen(name, FOPEN_RBIN_MODE);
        recorder_record_input(name);
        t3_read_file();
        t3_close();
    }
    t3_image_used = true;
    is_pk_font = true;
    report_start_file(filetype_font,(char *) name);
    cd.rastersize = 256;
    cd.raster = xtalloc((unsigned long) cd.rastersize, halfword);
    check_preamble = true;
    if (dpi==0) {
        normal_error("type 3","invalid dpi value 0");
    }  else if (newdpi==0) {
        newdpi = dpi;
    }
    while (readchar(check_preamble, &cd) != 0) {
        check_preamble = false;
        if (!pdf_char_marked(f, cd.charcode))
            continue;
        t3_char_widths[cd.charcode] = (float) pk_char_width(pdf, f, get_charwidth(f, cd.charcode), pdf->pk_scale_factor);
        if (cd.cwidth < 1 || cd.cheight < 1) {
            cd.xescape = cd.cwidth = round(t3_char_widths[cd.charcode] / 100.0);
            cd.cheight = 1;
            cd.xoff = 0;
            cd.yoff = 0;
            is_null_glyph = true;
        } else
            is_null_glyph = false;
        llx = -cd.xoff;
        lly = cd.yoff - cd.cheight + 1;
        urx = cd.cwidth + llx + 1;
        ury = cd.cheight + lly;
        update_bbox(llx, lly, urx, ury, t3_glyph_num == 0);
        t3_glyph_num++;
        t3_char_procs[cd.charcode] = pdf_create_obj(pdf, obj_type_others, 0);
        pdf_begin_obj(pdf, t3_char_procs[cd.charcode], OBJSTM_NEVER);
        pdf_begin_dict(pdf);
        pdf_dict_add_streaminfo(pdf);
        pdf_end_dict(pdf);
        pdf_begin_stream(pdf);
        setpdffloat(pf, (int64_t) t3_char_widths[cd.charcode], 2);
        print_pdffloat(pdf, pf);
        pdf_printf(pdf, " 0 %i %i %i %i d1\n", (int) llx, (int) lly, (int) urx, (int) ury);
        if (is_null_glyph)
            goto end_stream;
        pdf_printf(pdf, "q\n%i 0 0 %i %i %i cm\nBI\n",
            (int) (dpi * cd.cwidth  / newdpi),
            (int) (dpi * cd.cheight / newdpi),
            (int) llx, (int) lly);
        pdf_printf(pdf, "/W %i\n/H %i\n", (int) cd.cwidth, (int) cd.cheight);
        pdf_puts(pdf, "/IM true\n/BPC 1\n/D [1 0]\nID ");
        cw = (cd.cwidth + 7) / 8;
        rw = (cd.cwidth + 15) / 16;
        row = cd.raster;
        for (i = 0; i < cd.cheight; i++) {
            for (j = 0; j < rw - 1; j++) {
                pdf_out(pdf, (unsigned char) (*row / 256));
                pdf_out(pdf, (unsigned char) (*row % 256));
                row++;
            }
            pdf_out(pdf, (unsigned char) (*row / 256));
            if (2 * rw == cw)
                pdf_out(pdf, (unsigned char) (*row % 256));
            row++;
        }
        pdf_puts(pdf, "\nEI\nQ\n");
      end_stream:
        pdf_end_stream(pdf);
        pdf_end_obj(pdf);
    }
    xfree(cd.raster);
    cur_file_name = NULL;
    return true;
}

@
@c
void writet3(PDF pdf, internal_font_number f)
{
    int i;
    char s[32];
    int wptr, eptr, cptr;
    int first_char, last_char;
    int pk_font_scale;
    pdffloat pf;
    boolean is_notdef;

    t3_glyph_num = 0;
    t3_image_used = false;
    for (i = 0; i < 256; i++) {
        t3_char_procs[i] = 0;
        t3_char_widths[i] = 0;
    }
    is_pk_font = false;

    xfree(t3_buffer);
    t3_curbyte = 0;
    t3_size = 0;
    if (!writepk(pdf, f))
        return;
    for (i = font_bc(f); i <= font_ec(f); i++)
        if (pdf_char_marked(f, i))
            break;
    first_char = i;
    for (i = font_ec(f); i > first_char; i--)
        if (pdf_char_marked(f, i))
            break;
    last_char = i;

    /* Type 3 font dictionary */
    pdf_begin_obj(pdf, pdf_font_num(f), OBJSTM_ALWAYS);
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "Font");
    pdf_dict_add_name(pdf, "Subtype", "Type3");
    snprintf(s, 31, "F%i", (int) f);
    pdf_dict_add_name(pdf, "Name", s);
    if (pdf_font_attr(f) != get_nullstr() && pdf_font_attr(f) != 0) {
        pdf_out(pdf, '\n');
        pdf_print(pdf, pdf_font_attr(f));
        pdf_out(pdf, '\n');
    }
    if (is_pk_font) {
        pk_font_scale = get_pk_font_scale(pdf,f,pdf->pk_scale_factor);
        pdf_add_name(pdf, "FontMatrix");
        pdf_begin_array(pdf);
        setpdffloat(pf, (int64_t) pk_font_scale, 5);
        print_pdffloat(pdf, pf);
        pdf_puts(pdf, " 0 0 ");
        print_pdffloat(pdf, pf);
        pdf_puts(pdf, " 0 0");
        pdf_end_array(pdf);
    } else {
        pdf_add_name(pdf, "FontMatrix");
        pdf_begin_array(pdf);
        pdf_printf(pdf, "%g 0 0 %g 0 0", (double) t3_font_scale, (double) t3_font_scale);
        pdf_end_array(pdf);
    }
    pdf_add_name(pdf, font_key[FONTBBOX1_CODE].pdfname);
    pdf_begin_array(pdf);
    pdf_add_int(pdf, (int) t3_b0);
    pdf_add_int(pdf, (int) t3_b1);
    pdf_add_int(pdf, (int) t3_b2);
    pdf_add_int(pdf, (int) t3_b3);
    pdf_end_array(pdf);
    pdf_add_name(pdf, "Resources");
    pdf_begin_dict(pdf);
    pdf_add_name(pdf, "ProcSet");
    pdf_begin_array(pdf);
    pdf_add_name(pdf, "PDF");
    if (t3_image_used) {
        pdf_add_name(pdf, "ImageB");
    }
    pdf_end_array(pdf);
    pdf_end_dict(pdf);
    pdf_dict_add_int(pdf, "FirstChar", first_char);
    pdf_dict_add_int(pdf, "LastChar", last_char);
    wptr = pdf_create_obj(pdf, obj_type_others, 0);
    eptr = pdf_create_obj(pdf, obj_type_others, 0);
    cptr = pdf_create_obj(pdf, obj_type_others, 0);
    pdf_dict_add_ref(pdf, "Widths", (int) wptr);
    pdf_dict_add_ref(pdf, "Encoding", (int) eptr);
    pdf_dict_add_ref(pdf, "CharProcs", (int) cptr);
    pdf_end_dict(pdf);
    pdf_end_obj(pdf);

    /* chars width array */
    pdf_begin_obj(pdf, wptr, OBJSTM_ALWAYS);
    pdf_begin_array(pdf);
    if (is_pk_font) {
        for (i = first_char; i <= last_char; i++) {
            setpdffloat(pf, (int64_t) t3_char_widths[i], 2);
            print_pdffloat(pdf, pf);
            pdf_out(pdf, ' ');
        }
    } else {
        for (i = first_char; i <= last_char; i++) {
            pdf_add_int(pdf, (int) t3_char_widths[i]);
        }
    }
    pdf_end_array(pdf);
    pdf_end_obj(pdf);

    /* encoding dictionary */
    pdf_begin_obj(pdf, eptr, OBJSTM_ALWAYS);
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "Encoding");
    pdf_add_name(pdf, "Differences");
    pdf_begin_array(pdf);
    pdf_add_int(pdf, first_char);
    if (t3_char_procs[first_char] == 0) {
        pdf_add_name(pdf, notdef);
        is_notdef = true;
    } else {
        snprintf(s, 31, "a%i", first_char);
        pdf_add_name(pdf, s);
        is_notdef = false;
    }
    for (i = first_char + 1; i <= last_char; i++) {
        if (t3_char_procs[i] == 0) {
            if (!is_notdef) {
                pdf_add_int(pdf, i);
                pdf_add_name(pdf, notdef);
                is_notdef = true;
            }
        } else {
            if (is_notdef) {
                pdf_add_int(pdf, i);
                is_notdef = false;
            }
            snprintf(s, 31, "a%i", i);
            pdf_add_name(pdf, s);
        }
    }
    pdf_end_array(pdf);
    pdf_end_dict(pdf);
    pdf_end_obj(pdf);

    /* CharProcs dictionary */
    pdf_begin_obj(pdf, cptr, OBJSTM_ALWAYS);
    pdf_begin_dict(pdf);
    for (i = first_char; i <= last_char; i++) {
        if (t3_char_procs[i] != 0) {
            snprintf(s, 31, "a%i", (int) i);
            pdf_dict_add_ref(pdf, s, (int) t3_char_procs[i]);
        }
    }
    pdf_end_dict(pdf);
    pdf_end_obj(pdf);
    report_stop_file(filetype_font);
    cur_file_name = NULL;
}
