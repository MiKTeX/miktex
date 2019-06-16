/* postlinebreak.h
   
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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


#ifndef POSTLINEBREAK_H
#  define POSTLINEBREAK_H

/* todo: check this macro, especially  values of alink() */

#  define append_list(A,B)  do {                                          \
     vlink(cur_list.tail_field) = vlink((A));                           \
     cur_list.tail_field = (B);                                         \
   } while (0)


void ext_post_line_break(int paragraph_dir,
                         int right_skip,
                         int left_skip,
                         int protrude_chars,
                         halfword par_shape_ptr,
                         int adjust_spacing,
                         halfword inter_line_penalties_ptr,
                         int inter_line_penalty,
                         int club_penalty,
                         halfword club_penalties_ptr,
                         halfword widow_penalties_ptr,
                         int widow_penalty,
                         int broken_penalty,
                         halfword final_par_glue,
                         halfword best_bet,
                         halfword last_special_line,
                         scaled second_width,
                         scaled second_indent,
                         scaled first_width,
                         scaled first_indent, halfword best_line);

#endif
