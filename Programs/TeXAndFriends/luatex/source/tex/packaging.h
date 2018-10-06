/* packaging.h

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

#ifndef PACKAGING_H
#  define PACKAGING_H

/* We define some constants used when calling |hpack| to deal with font expansion. */

typedef enum {
    exactly = 0,      /*a box dimension is pre-specified */
    additional,       /*a box dimension is increased from the natural one */
    cal_expand_ratio, /* calculate amount for font expansion after breaking paragraph into lines */
    subst_ex_font     /* substitute fonts */
} hpack_subtypes;

#  define substituted 3 /* |subtype| of kern nodes that should be substituted */

extern void scan_spec(group_code c);
extern void scan_full_spec(group_code c, int spec_direction, int justpack);

extern scaled total_stretch[5];
extern scaled total_shrink[5];        /* glue found by |hpack| or |vpack| */
extern int last_badness;              /* badness of the most recently packaged box */
extern halfword adjust_tail;          /* tail of adjustment list */
extern halfword pre_adjust_tail;
extern int font_expand_ratio;         /* current expansion ratio */
extern halfword last_leftmost_char;
extern halfword last_rightmost_char;
extern halfword next_char_p;          /* pointer to the next char of an implicit kern */
extern halfword prev_char_p;          /* pointer to the previous char of an implicit kern */

extern void set_prev_char_p(halfword p);
extern scaled char_stretch(halfword p);
extern scaled char_shrink(halfword p);
extern scaled kern_stretch(halfword p);
extern scaled kern_shrink(halfword p);
extern void do_subst_font(halfword p, int ex_ratio);
extern scaled char_pw(halfword p, int side);
extern halfword new_margin_kern(scaled w, halfword p, int side);

#  define update_adjust_list(A) do { \
	if (A == null) \
	    normal_error("pre vadjust", "adjust_tail or pre_adjust_tail is null"); \
	vlink(A) = adjust_ptr(p); \
	while (vlink(A) != null) \
	    A = vlink(A); \
    } while (0)

extern halfword filtered_hpack(halfword p, halfword qt, scaled w, int m, int grp, int d, int just_pack, halfword attr);
extern scaled_whd natural_sizes(halfword p, halfword pp, glue_ratio g_mult, int g_sign, int g_order, int d);
extern halfword hpack(halfword p, scaled w, int m, int d);

extern int pack_begin_line;

extern halfword vpackage(halfword p, scaled h, int m, scaled l, int d);
extern halfword filtered_vpackage(halfword p, scaled h, int m, scaled l, int grp, int d, int just_pack, halfword attr);
extern void finish_vcenter(void);
extern void package(int c);
extern void append_to_vlist(halfword b, int location);

# define vpack(A,B,C,D) vpackage(A,B,C,max_dimen,D) /* special case of unconstrained depth */

extern halfword prune_page_top(halfword p, boolean s);
extern scaled active_height[10];        /* distance from first active node to~|cur_p| */

#  define cur_height active_height[1]   /* the natural height */
#  define awful_bad 07777777777         /* more than a billion demerits */
#  define deplorable 100000             /* more than |inf_bad|, but less than |awful_bad| */

extern scaled best_height_plus_depth;   /* height of the best box, without stretching or shrinking */

extern halfword vert_break(halfword p, scaled h, scaled d);
extern halfword vsplit(halfword n, scaled h, int m); /* extracts a page of height |h| from box |n| */

#  define box_code      0 /* |chr_code| for `\.{\\box}' */
#  define copy_code     1 /* |chr_code| for `\.{\\copy}' */
#  define last_box_code 2 /* |chr_code| for `\.{\\lastbox}' */
#  define vsplit_code   3 /* |chr_code| for `\.{\\vsplit}' */
#  define tpack_code    4
#  define vpack_code    5
#  define hpack_code    6
#  define vtop_code     7 /* |chr_code| for `\.{\\vtop}' */

#  define tail_page_disc disc_ptr[copy_code] /* last item removed by page builder */
#  define page_disc disc_ptr[last_box_code]  /* first item removed by page builder */
#  define split_disc disc_ptr[vsplit_code]   /* first item removed by \.{\\vsplit} */

extern halfword disc_ptr[(vsplit_code + 1)]; /* list pointers */

/*

Now let's turn to the question of how \.{\\hbox} is treated. We actually need to
consider also a slightly larger context, since constructions like

`\.{\\setbox3=}\penalty0\.{\\hbox...}' and
`\.{\\leaders}\penalty0\.{\\hbox...}' and
`\.{\\lower3.8pt\\hbox...}'

are supposed to invoke quite different actions after the box has been packaged.
Conversely, constructions like `\.{\\setbox3=}' can be followed by a variety of
different kinds of boxes, and we would like to encode such things in an efficient
way.

In other words, there are two problems: To represent the context of a box, and to
represent its type. The first problem is solved by putting a ``context code'' on
the |save_stack|, just below the two entries that give the dimensions produced by
|scan_spec|. The context code is either a (signed) shift amount, or it is a large
integer |>=box_flag|, where |box_flag=@t$2^{30}$@>|. Codes |box_flag| through
|box_flag+biggest_reg| represent `\.{\\setbox0}' through
`\.{\\setbox}|biggest_reg|'; codes |box_flag+biggest_reg+1| through
|box_flag+2*biggest_reg| represent `\.{\\global\\setbox0}' through
`\.{\\global\\setbox}|biggest_reg|'; code |box_flag+2*number_regs| represents
`\.{\\shipout}'; and codes |box_flag+2*number_regs+1| through
|box_flag+2*number_regs+3| represent `\.{\\leaders}', `\.{\\cleaders}', and
`\.{\\xleaders}'.

The second problem is solved by giving the command code |make_box| to all control
sequences that produce a box, and by using the following |chr_code| values to
distinguish between them: |box_code|, |copy_code|, |last_box_code|,
|vsplit_code|, |vtop_code|, |vtop_code+vmode|, and |vtop_code+hmode|, where the
latter two are used denote \.{\\vbox} and \.{\\hbox}, respectively.

*/

#  define box_flag            010000000000                  /* context code for `\.{\\setbox0}' */
#  define global_box_flag     (box_flag+number_regs)        /* context code for `\.{\\global\\setbox0}' */
#  define max_global_box_flag (global_box_flag+number_regs)
#  define ship_out_flag       (max_global_box_flag+1)       /* context code for `\.{\\shipout}' */
#  define lua_scan_flag       (max_global_box_flag+2)       /* context code for |scan_list| */
#  define leader_flag         (max_global_box_flag+3)       /* context code for `\.{\\leaders}' */

extern void begin_box(int box_context);

#define math_skip_boundary(n) \
(n && type(n) == glue_node && (subtype(n) == space_skip_subtype || subtype(n) == xspace_skip_subtype))

extern int ignore_math_skip(halfword p) ;

#endif
