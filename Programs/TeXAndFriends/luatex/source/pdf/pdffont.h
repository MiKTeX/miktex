/* pdffont.h

   Copyright 2010-2013 Taco Hoekwater <taco@luatex.org>

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


#ifndef PDFFONT_H
#  define PDFFONT_H

extern scaled_whd output_one_char(PDF pdf, halfword p);
extern void pdf_init_font(PDF pdf, internal_font_number f);
extern internal_font_number pdf_set_font(PDF pdf, internal_font_number f);
extern void pdf_include_chars(PDF);

#endif
