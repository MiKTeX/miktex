/* pdfxform.h

   Copyright 2009 Taco Hoekwater <taco@luatex.org>

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

#ifndef PDFXFORM_H
#  define PDFXFORM_H

#  define pdfmem_xform_size 10 /* size of memory in |pdf->mem| which |obj_data_ptr| holds */

#  define obj_xform_width(pdf,A)          pdf->mem[obj_data_ptr(pdf,A) + 0]
#  define obj_xform_height(pdf,A)         pdf->mem[obj_data_ptr(pdf,A) + 1]
#  define obj_xform_depth(pdf,A)          pdf->mem[obj_data_ptr(pdf,A) + 2]
#  define obj_xform_box(pdf,A)            pdf->mem[obj_data_ptr(pdf,A) + 3] /* this field holds pointer to the corresponding box */
#  define obj_xform_attr(pdf,A)           pdf->mem[obj_data_ptr(pdf,A) + 4] /* additional xform attributes */
#  define obj_xform_resources(pdf,A)      pdf->mem[obj_data_ptr(pdf,A) + 5] /* additional xform Resources */
#  define obj_xform_attr_str(pdf,A)       pdf->mem[obj_data_ptr(pdf,A) + 6]
#  define obj_xform_resources_str(pdf,A)  pdf->mem[obj_data_ptr(pdf,A) + 7]
#  define obj_xform_type(pdf,A)           pdf->mem[obj_data_ptr(pdf,A) + 8]
#  define obj_xform_margin(pdf,A)         pdf->mem[obj_data_ptr(pdf,A) + 9]

#  define set_pdf_xform_objnum(A,B)            pdf_xform_objnum(A)=B
#  define set_obj_xform_width(pdf,A,B)         obj_xform_width(pdf,A)=B
#  define set_obj_xform_height(pdf,A,B)        obj_xform_height(pdf,A)=B
#  define set_obj_xform_depth(pdf,A,B)         obj_xform_depth(pdf,A)=B
#  define set_obj_xform_box(pdf,A,B)           obj_xform_box(pdf,A)=B
#  define set_obj_xform_attr(pdf,A,B)          obj_xform_attr(pdf,A)=B
#  define set_obj_xform_resources(pdf,A,B)     obj_xform_resources(pdf,A)=B
#  define set_obj_xform_attr_str(pdf,A,B)      obj_xform_attr_str(pdf,A)=B
#  define set_obj_xform_resources_str(pdf,A,B) obj_xform_resources_str(pdf,A)=B
#  define set_obj_xform_type(pdf,A,B)          obj_xform_type(pdf,A)=B
#  define set_obj_xform_margin(pdf,A,B)        obj_xform_margin(pdf,A)=B

extern int pdf_cur_form; /* the form being output */

void pdf_place_form(PDF pdf, halfword p);
void scan_pdfxform(PDF pdf);
void scan_pdfrefxform(PDF pdf);

#endif
