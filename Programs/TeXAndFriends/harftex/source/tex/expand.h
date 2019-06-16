/* expand.h

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


#ifndef EXPAND_H
#  define EXPAND_H

extern boolean is_in_csname;
extern void expand(void);
extern void complain_missing_csname(void);
extern void manufacture_csname(boolean use);
extern void inject_last_tested_cs(void);
extern void insert_relax(void);
extern void get_x_token(void);
extern void x_token(void);

#  define top_mark_code 0       /* the mark in effect at the previous page break */
#  define first_mark_code 1     /* the first mark between |top_mark| and |bot_mark| */
#  define bot_mark_code 2       /* the mark in effect at the current page break */
#  define split_first_mark_code 3
                                /* the first mark found by \.{\\vsplit} */
#  define split_bot_mark_code 4 /* the last mark found by \.{\\vsplit} */
#  define marks_code 5

#  define top_mark(A) top_marks_array[(A)]
#  define first_mark(A) first_marks_array[(A)]
#  define bot_mark(A) bot_marks_array[(A)]
#  define split_first_mark(A) split_first_marks_array[(A)]
#  define split_bot_mark(A) split_bot_marks_array[(A)]

#  define set_top_mark(A,B) top_mark(A)=(B)
#  define set_first_mark(A,B) first_mark(A)=(B)
#  define set_bot_mark(A,B) bot_mark(A)=(B)
#  define set_split_first_mark(A,B) split_first_mark(A)=(B)
#  define set_split_bot_mark(A,B) split_bot_mark(A)=(B)

#  define delete_top_mark(A) do {			\
     if (top_mark(A)!=null)			\
       delete_token_ref(top_mark(A));		\
     top_mark(A)=null;				\
   } while (0)

#  define delete_bot_mark(A) do {			\
     if (bot_mark(A)!=null)			\
       delete_token_ref(bot_mark(A));		\
     bot_mark(A)=null;				\
   } while (0)

#  define delete_first_mark(A) do {		\
     if (first_mark(A)!=null)			\
       delete_token_ref(first_mark(A));		\
     first_mark(A)=null;			\
   } while (0)

#  define delete_split_first_mark(A) do {		\
     if (split_first_mark(A)!=null)		\
       delete_token_ref(split_first_mark(A));	\
     split_first_mark(A)=null;			\
   } while (0)

#  define delete_split_bot_mark(A) do {		\
     if (split_bot_mark(A)!=null)		\
       delete_token_ref(split_bot_mark(A));	\
     split_bot_mark(A)=null;			\
} while (0)

#  define clear_marks_code 1

#  define biggest_mark 65535

extern halfword top_marks_array[(biggest_mark + 1)];
extern halfword first_marks_array[(biggest_mark + 1)];
extern halfword bot_marks_array[(biggest_mark + 1)];
extern halfword split_first_marks_array[(biggest_mark + 1)];
extern halfword split_bot_marks_array[(biggest_mark + 1)];
extern halfword biggest_used_mark;


extern void initialize_marks(void);

extern int long_state;
extern halfword pstack[9];
extern void macro_call(void);


#endif
