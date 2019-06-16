/* nesting.h
   
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


#ifndef NESTING_H
#  define NESTING_H

#  define vmode 1               /* vertical mode */
#  define hmode (vmode+max_command_cmd+1)
                                        /* horizontal mode */
#  define mmode (hmode+max_command_cmd+1)
                                        /* math mode */

extern void print_mode(int m);
extern void print_in_mode(int m);
extern int get_mode_id(void);

#  define ignore_depth -65536000        /* magic dimension value to mean `ignore me' */

typedef struct list_state_record_ {
    int mode_field;
    halfword head_field;
    halfword tail_field;
    halfword eTeX_aux_field;
    int pg_field;
    int ml_field;
    halfword prev_depth_field;
    halfword space_factor_field;
    halfword incompleat_noad_field;
    halfword dirs_field;
    int math_field;
    int math_style_field;
} list_state_record;

extern list_state_record *nest;
extern int nest_ptr;
extern int max_nest_stack;
#  define cur_list nest[nest_ptr]
                                /* the ``top'' semantic state */
extern int shown_mode;
extern halfword save_tail;

extern void push_nest(void);
extern void pop_nest(void);
extern void initialize_nesting(void);

extern void tail_append(halfword p);
extern halfword pop_tail(void);


extern void show_activities(void);


#endif
