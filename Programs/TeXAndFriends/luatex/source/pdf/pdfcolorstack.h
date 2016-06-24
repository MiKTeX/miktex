/* pdfcolorstack.h

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


#ifndef PDFCOLORSTACK_H
#  define PDFCOLORSTACK_H

#  define set_pdf_colorstack_stack(A,B) pdf_colorstack_stack(A)=B
#  define set_pdf_colorstack_cmd(A,B)   pdf_colorstack_cmd(A)=B
#  define set_pdf_colorstack_data(A,B)  pdf_colorstack_data(A)=B

#  define STACK_INCREMENT 8

int newcolorstack(const char *s, int literal_mode, boolean pagestart);
int colorstackused(void);
int colorstackpop(int colstack_no);
int colorstackcurrent(int colstack_no);
int colorstackskippagestart(int colstack_no);
void colorstackpagestart(void);

extern void pdf_out_colorstack(PDF pdf, halfword p);
extern void pdf_out_colorstack_startpage(PDF pdf);

#endif
