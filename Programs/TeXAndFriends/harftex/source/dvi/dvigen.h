/* dvigen.h

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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


#ifndef DVIGEN_H
#  define DVIGEN_H

/* todo: move initialization from mainbody to ensure_open */

extern int dvi_buf_size;
extern eight_bits *dvi_buf;     /* 0 is unused */

/*tex Housekeeping.  */

extern void dvi_open_file(PDF pdf);
extern void dvi_write_header(PDF pdf);
extern void dvi_finish_file(PDF pdf, int fatal_error);
extern void dvi_begin_page(PDF pdf);
extern void dvi_end_page(PDF pdf);

/*tex Specific injections. */

extern void dvi_place_glyph(PDF pdf, internal_font_number f, int c, int ex);
extern void dvi_place_rule(PDF pdf, halfword q, scaledpos size);
extern void dvi_special(PDF pdf, halfword p);

/*tex List handling (and nesting). */

extern void dvi_push_list(PDF pdf, scaledpos *saved_pos, int *saved_loc);
extern void dvi_pop_list(PDF pdf, scaledpos *saved_pos, int *saved_loc);
extern void dvi_set_reference_point(PDF pdf, posstructure *refpoint);

/*tex Status information used in |lstatslib|. Not that useful. */

extern int dvi_get_status_ptr(PDF pdf);
extern int dvi_get_status_gone(PDF pdf);

#endif
