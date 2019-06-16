/* pdflink.h

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

#ifndef PDFLINK_H
#  define PDFLINK_H

#  define set_pdf_link_attr(A,B)   pdf_link_attr(A)=B
#  define set_pdf_link_action(A,B) pdf_link_action(A)=B
#  define set_pdf_link_objnum(A,B) pdf_link_objnum(A)=B

extern void push_link_level(PDF pdf, halfword p);
extern void pop_link_level(PDF pdf);
extern void do_link(PDF pdf, halfword p, halfword parent_box, scaledpos cur);
extern void end_link(PDF pdf, halfword p);
extern void append_link(PDF pdf, halfword parent_box, scaledpos cur, small_number i);
extern void scan_startlink(PDF pdf);


#endif
