/* pdfpage.h

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

#ifndef PDFPAGE_H
#  define PDFPAGE_H

#  define is_pagemode(p)      ((p)->mode == PMODE_PAGE)
#  define is_textmode(p)      ((p)->mode == PMODE_TEXT)
#  define is_chararraymode(p) ((p)->mode == PMODE_CHARARRAY)
#  define is_charmode(p)      ((p)->mode == PMODE_CHAR)

#  define setpdffloat(a,b,c) do {(a).m = (b); (a).e = (c);} while (0)

#  ifdef hz
/* AIX 4.3 defines hz as 100 in system headers */
#    undef hz
#  endif

void synch_pos_with_cur(posstructure * pos, posstructure * refpos, scaledpos cur);
boolean calc_pdfpos(pdfstructure * p, scaledpos pos);
void pdf_end_string_nl(PDF pdf);
void pdf_goto_pagemode(PDF pdf);
void pdf_goto_textmode(PDF pdf);
void init_pdf_pagecalculations(PDF pdf);
void pdf_print_cm(PDF pdf, pdffloat * cm);
void pdf_set_pos(PDF pdf, scaledpos pos);
void pdf_set_pos_temp(PDF pdf, scaledpos pos);
void print_pdf_matrix(PDF pdf, pdffloat * tm);

#endif
