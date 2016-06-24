/* pdfannot.h

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


#ifndef PDFANNOT_H
#  define PDFANNOT_H

/* data structure of annotations; words 1..4 represent the coordinates of the annotation */

#  define obj_annot_ptr obj_aux /* pointer to corresponding whatsit node */

#  define set_pdf_annot_objnum(A,B) pdf_annot_objnum(A)=B
#  define set_pdf_annot_data(A,B)   pdf_annot_data(A)=B

extern void do_annot(PDF pdf, halfword p, halfword parent_box, scaledpos cur);
extern void new_annot_whatsit(small_number w);

extern void scan_annot(PDF pdf);

#endif
