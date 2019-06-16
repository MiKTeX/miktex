/* buildpage.h
   
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


#ifndef BUILDPAGE_H
#  define BUILDPAGE_H

/* empty = 0 */
#  define inserts_only 1        /* |page_contents| when an insert node has been contributed, but no boxes */
#  define box_there 2           /* |page_contents| when a box or rule has been contributed */

extern halfword page_tail;      /* the final node on the current page */
extern int page_contents;       /* what is on the current page so far? */
extern scaled page_max_depth;   /* maximum box depth on page being built */
extern halfword best_page_break;        /* break here to get the best page known so far */
extern int least_page_cost;     /* the score for this currently best page */
extern scaled best_size;        /* its |page_goal| */

/*
The data structure definitions here use the fact that the |@!height| field
appears in the fourth word of a box node.
@^data structure assumptions@>
*/

#  define broken_ptr(A) vlink((A)+2)    /* an insertion for this class will break here if anywhere */
#  define broken_ins(A) vinfo((A)+2)    /* this insertion might break at |broken_ptr| */
#  define last_ins_ptr(A) vlink((A)+3)  /* the most recent insertion for this |subtype| */
#  define best_ins_ptr(A) vinfo((A)+3)  /* the optimum most recent insertion */

extern void initialize_buildpage(void);

#  define page_goal page_so_far[0]      /* desired height of information on page being built */
#  define page_total page_so_far[1]     /* height of the current page */
#  define page_shrink page_so_far[6]    /* shrinkability of the current page */
#  define page_depth page_so_far[7]     /* depth of the current page */

extern scaled page_so_far[8];   /* height and glue of the current page */
extern halfword last_glue;      /* used to implement \.{\\lastskip */
extern int last_penalty;        /* used to implement \.{\\lastpenalty} */
extern scaled last_kern;        /* used to implement \.{\\lastkern} */
extern int last_node_type;      /* used to implement \.{\\lastnodetype} */
extern int insert_penalties;    /* sum of the penalties for held-over insertions */

extern void print_totals(void);
extern void freeze_page_specs(int s);

extern boolean output_active;

extern void start_new_page(void);

#  define contrib_tail nest[0].tail_field
                                        /* tail of the contribution list */

extern void build_page(void);   /* append contributions to the current page */
extern void fire_up(halfword c);
extern void resume_after_output(void);

#endif
