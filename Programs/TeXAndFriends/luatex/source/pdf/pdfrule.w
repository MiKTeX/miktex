% pdfrule.w
%
% Copyright 2010-2011 Taco Hoekwater <taco@@luatex.org>
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
#include "pdf/pdfpage.h"

@ @c

/* maybe we should have an extra callback on normal rules or on any rule in 2.0+ */

void pdf_place_rule(PDF pdf, halfword q, scaledpos size, int callback_id)
{
    pdfpos dim;
    pdfstructure *p = pdf->pstruct;
    scaledpos pos = pdf->posstruct->pos;
    halfword s = subtype(q);
    /*  (void) q; */
    if (s >= math_over_rule && s <= math_radical_rule) {
        if (callback_id == 0) {
            s = normal_rule;
        } else {
            s = user_rule;
        }
    }
    if (s == box_rule) {
        pdf_place_form(pdf,q);
    } else if (s == image_rule) {
        pdf_place_image(pdf,q);
    } else if (s == empty_rule) {
        /* place nothing, only take space */
    } else if (s == user_rule) {
        if (callback_id != 0) {
            pdf_goto_pagemode(pdf);
            pdf_puts(pdf, "q\n");
            pdf_set_pos_temp(pdf, pos);
            run_callback(callback_id, "Ndd->",q,size.h,size.v);
            pdf_puts(pdf, "\nQ\n");
        }
    } else {
        /* normal_rule or >= 100 being a leader rule */
        pdf_goto_pagemode(pdf);
        dim.h.m = i64round(size.h * p->k1);
        dim.h.e = p->pdf.h.e;
        dim.v.m = i64round(size.v * p->k1);
        dim.v.e = p->pdf.v.e;
        pdf_puts(pdf, "q\n");
        if (size.v <= one_bp) {
            pos.v += i64round(0.5 * size.v);
            pdf_set_pos_temp(pdf, pos);
            pdf_puts(pdf, "[]0 d 0 J ");
            print_pdffloat(pdf, dim.v);
            pdf_puts(pdf, " w 0 0 m ");
            print_pdffloat(pdf, dim.h);
            pdf_puts(pdf, " 0 l S\n");
        } else if (size.h <= one_bp) {
            pos.h += i64round(0.5 * size.h);
            pdf_set_pos_temp(pdf, pos);
            pdf_puts(pdf, "[]0 d 0 J ");
            print_pdffloat(pdf, dim.h);
            pdf_puts(pdf, " w 0 0 m 0 ");
            print_pdffloat(pdf, dim.v);
            pdf_puts(pdf, " l S\n");
        } else {
            pdf_set_pos_temp(pdf, pos);
            pdf_puts(pdf, "0 0 ");
            print_pdffloat(pdf, dim.h);
            pdf_out(pdf, ' ');
            print_pdffloat(pdf, dim.v);
            pdf_puts(pdf, " re f\n");
        }
        pdf_puts(pdf, "Q\n");
    }
}
