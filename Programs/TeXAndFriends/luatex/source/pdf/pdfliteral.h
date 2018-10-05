/* pdfliteral.h

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


#ifndef PDFLITERAL_H
#  define PDFLITERAL_H

#  define set_pdf_literal_mode(A,B) pdf_literal_mode(A)=B
#  define set_pdf_literal_type(A,B) pdf_literal_type(A)=B
#  define set_pdf_literal_data(A,B) pdf_literal_data(A)=B

extern void pdf_literal(PDF pdf, str_number s, int literal_mode, boolean warn);
extern void pdf_literal_set_mode(PDF pdf, int literal_mode);
extern void pdf_special(PDF pdf, halfword p);
extern void pdf_out_literal(PDF pdf, halfword p);

#endif
