% packaging.w
%
% Copyright 2009-2010 Taco Hoekwater <taco@@luatex.org>
%
% This file is part of LuaTeX.
%
% LuaTeX is free software; you can redistribute it and/or modify it under
% the terms of the GNU General Public License as published by the Free
% Software Foundation; either version 2 of the License, or (at your
% option) any later version.
%
% LuaTeX is distributed in the hope that it will be useful, but WITHOUT
% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
% FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
% License for more details.
%
% You should have received a copy of the GNU General Public License along
% with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

@ @c

#include "ptexlib.h"

@ We're essentially done with the parts of \TeX\ that are concerned with the
input (|get_next|) and the output (|ship_out|). So it's time to get heavily into
the remaining part, which does the real work of typesetting.

After lists are constructed, \TeX\ wraps them up and puts them into boxes. Two
major subroutines are given the responsibility for this task: |hpack| applies to
horizontal lists (hlists) and |vpack| applies to vertical lists (vlists). The
main duty of |hpack| and |vpack| is to compute the dimensions of the resulting
boxes, and to adjust the glue if one of those dimensions is pre-specified. The
computed sizes normally enclose all of the material inside the new box; but some
items may stick out if negative glue is used, if the box is overfull, or if a
\.{\\vbox} includes other boxes that have been shifted left.

The subroutine call |hpack(p,w,m)| returns a pointer to an |hlist_node| for a box
containing the hlist that starts at |p|. Parameter |w| specifies a width; and
parameter |m| is either `|exactly|' or `|additional|'. Thus, |hpack(p,w,exactly)|
produces a box whose width is exactly |w|, while |hpack(p,w,additional)| yields a
box whose width is the natural width plus |w|. It is convenient to define a macro
called `|natural|' to cover the most common case, so that we can say
|hpack(p,natural)| to get a box that has the natural width of list |p|.

Similarly, |vpack(p,w,m)| returns a pointer to a |vlist_node| for a box
containing the vlist that starts at |p|. In this case |w| represents a height
instead of a width; the parameter |m| is interpreted as in |hpack|.

@ The parameters to |hpack| and |vpack| correspond to \TeX's primitives like
`\.{\\hbox} \.{to} \.{300pt}', `\.{\\hbox} \.{spread} \.{10pt}'; note that
`\.{\\hbox}' with no dimension following it is equivalent to `\.{\\hbox}
\.{spread} \.{0pt}'. The |scan_spec| subroutine scans such constructions in the
user's input, including the mandatory left brace that follows them, and it puts
the specification onto |save_stack| so that the desired box can later be obtained
by executing the following code: $$\vbox{\halign{#\hfil\cr
|save_ptr:=save_ptr-1;|\cr |hpack(p,saved_value(0),saved_level(0)).|\cr}}$$

@c
/*
    void scan_spec(group_code c)
    {
        int spec_code;
        if (scan_keyword("to")) {
            spec_code = exactly;
            scan_normal_dimen();
        } else if (scan_keyword("spread")) {
            spec_code = additional;
            scan_normal_dimen();
        } else {
            spec_code = additional;
            cur_val = 0;
        }
        set_saved_record(0, saved_boxspec, spec_code, cur_val);
        save_ptr++;
        new_save_level(c);
        scan_left_brace();
    }
*/

void scan_spec(group_code c)
{                               /* scans a box specification and left brace */
    int spec_code;
    boolean done = false ;
    do {
        get_x_token();
    } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));
    if (cur_cmd == left_brace_cmd) {
        spec_code = additional;
        cur_val = 0;
        done = true;
    } else {
        /* todo: attr */
        back_input();
        if (scan_keyword("to")) {
            spec_code = exactly;
            scan_normal_dimen();
        } else if (scan_keyword("spread")) {
            spec_code = additional;
            scan_normal_dimen();
        } else {
            spec_code = additional;
            cur_val = 0;
        }
    }
    set_saved_record(0, saved_boxspec, spec_code, cur_val);
    save_ptr++;
    new_save_level(c);
    if (!done) {
        scan_left_brace();
    }
}

@ When scanning, special care is necessary to ensure that the special
|save_stack| codes are placed just below the new group code, because scanning can
change |save_stack| when \.{\\csname} appears.

This coincides with the text on |dir| and |attr| keywords, as these are exaclty
the uses of \.{\\hbox}, \.{\\vbox}, and \.{\\vtop} in the input stream (the
others are \.{\\vcenter}, \.{\\valign}, and \.{\\halign}).

@c
/*
    void scan_full_spec(group_code c, int spec_direction)
    {
        int s;
        int i;
        int v;
        int spec_code;
        halfword attr_list;
        if (attr_list_cache == cache_disabled)
            update_attribute_cache();
        attr_list = attr_list_cache;
        s = saved_value(0);
      CONTINUE:
        while (cur_cmd == relax_cmd || cur_cmd == spacer_cmd) {
            get_x_token();
            if (cur_cmd != relax_cmd && cur_cmd != spacer_cmd)
                back_input();
        }
        if (scan_keyword("attr")) {
            scan_register_num();
            i = cur_val;
            scan_optional_equals();
            scan_int();
            v = cur_val;
            if ((attr_list != null) && (attr_list == attr_list_cache)) {
                attr_list = copy_attribute_list(attr_list_cache);
                add_node_attr_ref(attr_list);
            }
            attr_list = do_set_attribute(attr_list, i, v);
            goto CONTINUE;
        }
        if (scan_keyword("dir")) {
            scan_direction();
            spec_direction = cur_val;
            goto CONTINUE;
        }
        if (attr_list == attr_list_cache) {
            add_node_attr_ref(attr_list);
        }
        if (scan_keyword("to")) {
            spec_code = exactly;
        } else if (scan_keyword("spread")) {
            spec_code = additional;
        } else {
            spec_code = additional;
            cur_val = 0;
            goto FOUND;
        }
        scan_normal_dimen();
      FOUND:
        set_saved_record(0, saved_boxcontext, 0, s);
        set_saved_record(1, saved_boxspec, spec_code, cur_val);
        if (spec_direction != -1) {
            set_saved_record(2, saved_boxdir, spec_direction, text_dir_ptr);
            text_dir_ptr = new_dir(spec_direction);
        } else {
            set_saved_record(2, saved_boxdir, spec_direction, null);
        }
        set_saved_record(3, saved_boxattr, 0, attr_list);
        save_ptr += 4;
        new_save_level(c);
        scan_left_brace();
        eq_word_define(int_base + body_direction_code, spec_direction);
        eq_word_define(int_base + par_direction_code, spec_direction);
        eq_word_define(int_base + text_direction_code, spec_direction);
    }
*/

/* scans a box specification and left brace */

void scan_full_spec(group_code c, int spec_direction, int just_pack)
{
    int s, i, v, spec_code;
    boolean done = false ;
    halfword attr_list;
    boolean attr_done = false ;
    if (attr_list_cache == cache_disabled)
        update_attribute_cache();
    attr_list = attr_list_cache;
    s = saved_value(0); /* the box context */
    do {
        get_x_token();
    } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));
    if (cur_cmd == left_brace_cmd) {
        goto QUICK;
    } else {
        back_input();
        goto KEYWORDS;
    }
  CONTINUE:
    while (cur_cmd == relax_cmd || cur_cmd == spacer_cmd) {
        get_x_token();
        if (cur_cmd == left_brace_cmd) {
            goto QUICK;
        } else if (cur_cmd != relax_cmd && cur_cmd != spacer_cmd) {
            back_input();
            break;
        }
    }
  KEYWORDS:
    if (scan_keyword("attr")) {
        scan_register_num();
        i = cur_val;
        scan_optional_equals();
        scan_int();
        v = cur_val;
        if (! attr_done) {
            attr_list = copy_attribute_list(attr_list_cache);
            attr_done = true;
        }
        attr_list = do_set_attribute(attr_list, i, v);
        goto CONTINUE;
    }
    if (scan_keyword("dir")) {
        scan_direction();
        spec_direction = cur_val;
        goto CONTINUE;
    }
    if (scan_keyword("to")) {
        spec_code = exactly;
    } else if (scan_keyword("spread")) {
        spec_code = additional;
    } else {
        spec_code = additional;
        cur_val = 0;
        goto FOUND;
    }
    scan_normal_dimen();
    goto FOUND;
  QUICK:
    spec_code = additional;
    cur_val = 0;
    done = true;
  FOUND:
    add_node_attr_ref(attr_list);
    set_saved_record(0, saved_boxcontext, 0, s);
    set_saved_record(1, saved_boxspec, spec_code, cur_val);
    /* DIR: Adjust |text_dir_ptr| for |scan_spec| */
    if (spec_direction != -1) {
        set_saved_record(2, saved_boxdir, spec_direction, text_dir_ptr);
        text_dir_ptr = new_dir(spec_direction);
    } else {
        set_saved_record(2, saved_boxdir, spec_direction, null);
    }
    set_saved_record(3, saved_boxattr, 0, attr_list);
    set_saved_record(4, saved_boxpack, 0, just_pack);
    save_ptr += 5;
    new_save_level(c);
    if (! done) {
        scan_left_brace();
    }
    /* no gain: if (body_direction_par != spec_direction) etc */
    eq_word_define(int_base + body_direction_code, spec_direction);
    eq_word_define(int_base + par_direction_code, spec_direction);
    eq_word_define(int_base + text_direction_code, spec_direction);
}


@ To figure out the glue setting, |hpack| and |vpack| determine how much
stretchability and shrinkability are present, considering all four orders of
infinity. The highest order of infinity that has a nonzero coefficient is then
used as if no other orders were present.

For example, suppose that the given list contains six glue nodes with the
respective stretchabilities 3pt, 8fill, 5fil, 6pt, $-3$fil, $-8$fill. Then the
total is essentially 2fil; and if a total additional space of 6pt is to be
achieved by stretching, the actual amounts of stretch will be 0pt, 0pt, 15pt,
0pt, $-9$pt, and 0pt, since only `fil' glue will be considered. (The `fill' glue
is therefore not really stretching infinitely with respect to `fil'; nobody would
actually want that to happen.)

The arrays |total_stretch| and |total_shrink| are used to determine how much glue
of each kind is present. A global variable |last_badness| is used to implement
\.{\\badness}.

@c
scaled total_stretch[5];
scaled total_shrink[5];         /* glue found by |hpack| or |vpack| */
int last_badness;               /* badness of the most recently packaged box */

@ If the global variable |adjust_tail| is non-null, the |hpack| routine also
removes all occurrences of |ins_node|, |mark_node|, and |adjust_node| items and
appends the resulting material onto the list that ends at location |adjust_tail|.

@c
halfword adjust_tail;           /* tail of adjustment list */


@ Materials in \.{\\vadjust} used with \.{pre} keyword will be appended to
|pre_adjust_tail| instead of |adjust_tail|.

@c
halfword pre_adjust_tail;

halfword last_leftmost_char;
halfword last_rightmost_char;

halfword next_char_p;           /* pointer to the next char of an implicit kern */
halfword prev_char_p;           /* pointer to the previous char of an implicit kern */

@ This procedure is called repeatedly from inside the line break algorithm.
@c
void set_prev_char_p(halfword p)
{
    prev_char_p = p;
}

/*
    the kern stretch / shrink code was (or had become) rather weird ... the width field
    is set, and then used in a second calculation, repeatedly, so why is that ... maybe some
    some weird left-over ... anyway, the values are so small that in practice they are not
    significant at all when the backend sees them because a few hundred sp positive or
    negative are just noise there (so adjustlevel 3 has hardly any consequence for the
    result but is more efficient)
*/


@ @c
scaled char_stretch(halfword p)
{
    internal_font_number f = font(p);
    int m = font_max_stretch(f);
    if (m > 0) {
        int c = character(p);
        int ef = get_ef_code(f, c);
        if (ef > 0) {
            scaled dw = calc_char_width(f, c, m) - char_width(f, c) - x_advance(p);
            if (dw > 0) {
                return round_xn_over_d(dw, ef, 1000);
            }
        }
    }
    return 0;
}

@ @c
scaled char_shrink(halfword p)
{
    internal_font_number f = font(p);
    int m = font_max_shrink(f);
    if (m > 0) {
        int c = character(p);
        int ef = get_ef_code(f, c);
        if (ef > 0) {
            scaled dw = char_width(f, c) + x_advance(p) - calc_char_width(f, c, -m);
            if (dw > 0) {
                return round_xn_over_d(dw, ef, 1000);
            }
        }
    }
    return 0;
}

@ @c
/*
scaled kern_stretch(halfword p)
{
    halfword l, r;
    scaled d;
    int m;
    if ((prev_char_p == null) || (vlink(prev_char_p) != p) || (vlink(p) == null))
        return 0;
    l = prev_char_p;
    // we need a left char
    if (!is_char_node(l))
        return 0;
    r = vlink(p);
    // and a right char
    if (!is_char_node(r))
        return 0;
    // and a reason to kern
    if ((font(l) != font(r)) || (font_max_stretch(font(l)) == 0))
        return 0;
    m = font_max_stretch(font(l));
    d = get_kern(font(l), character(l), character(r)); // real kern, so what is width(p) then; the messed up one
    d = round_xn_over_d(d, 1000 + m, 1000);
    return round_xn_over_d(d - width(p), get_ef_code(font(l), character(l)), 1000);
}
*/

scaled kern_stretch(halfword p)
{
    int m;
    scaled d, e, x;
    scaled w = width(p) ;
    halfword l;
    halfword r;
    if (w == 0)  {
        /* why bother about zero kerns */
        return 0;
    }
    l = prev_char_p ;
    if ((l == null) || (vlink(l) != p)) {
        /* we only care about kerns following a char*/
        return 0;
    }
    r = vlink(p);
    if (r == null) {
        /* we only care about kerns between a char and something else */
    }
    if (!(is_char_node(l) && is_char_node(r))) {
        /* we want two chars (but but don't care about the fonts) */
        return 0;
    }
    /* we use the old logic, kind of, but average the ef as we might depend on proper overlap */
    m = (font_max_stretch(font(l)) + font_max_stretch(font(r)))/2;
    if (m == 0) {
        /* nothing to kern */
        return 0;
    }
    d = round_xn_over_d(w, 1000 + m, 1000);
    /* we use the old logic, kind of, but average the ef as we might depend on proper overlap */
    e = (get_ef_code(font(l), character(l)) + get_ef_code(font(r), character(r)))/2 ;
    if (e == 1000) {
        x = d - w;
    } else {
        x = round_xn_over_d(d - w, e, 1000);
    }
    /*
        printf("STRETCH w=%i s=%i x=%i\n",w,e+m,x);
    */
    return x;
}

@ @c
/*
scaled kern_shrink(halfword p)
{
    halfword l, r;
    scaled d;
    int m;
    if ((prev_char_p == null) || (vlink(prev_char_p) != p) || (vlink(p) == null))
        return 0;
    l = prev_char_p;
    // we need a left char
    if (!is_char_node(l))
        return 0;
    r = vlink(p);
    // and a right char
    if (!is_char_node(r))
        return 0;
    // and a reason to kern
    if ((font(l) != font(r)) || (font_max_shrink(font(l)) == 0))
        return 0;
    m = font_max_shrink(font(l));
    d = get_kern(font(l), character(l), character(r)); // real kern, so what is width(p) then; the messed up one
    d = round_xn_over_d(d, 1000 - m, 1000);
    return round_xn_over_d(width(p) - d, get_ef_code(font(l), character(l)), 1000);
}
*/

scaled kern_shrink(halfword p)
{
    int m;
    scaled d, e, x;
    scaled w = width(p) ;
    halfword l;
    halfword r;
    if (w == 0)  {
        /* why bother about zero kerns */
        return 0;
    }
    l = prev_char_p ;
    if ((l == null) || (vlink(l) != p)) {
        /* we only care about kerns following a char*/
        return 0;
    }
    r = vlink(p);
    if (r == null) {
        /* we only care about kerns between a char and something else */
    }
    if (!(is_char_node(l) && is_char_node(r))) {
        /* we want two chars (but but don't care about the fonts) */
        return 0;
    }
    /* we use the old logic, kind of, but average the ef as we might depend on proper overlap */
    m = (font_max_shrink(font(l)) + font_max_shrink(font(r)))/2;
    if (m == 0) {
        /* nothing to kern */
        return 0;
    }
    d = round_xn_over_d(w, 1000 - m, 1000);
    e = (get_ef_code(font(l), character(l)) + get_ef_code(font(r), character(r)))/2 ;
    if (e == 1000) {
         x = w - d ;
    } else {
        x = round_xn_over_d(w - d, e, 1000);
    }
    /*
    printf("SHRINK w=%i s=%i x=%i\n",w,e+m,x);
    */
    return x;
}

@ @c
void do_subst_font(halfword p, int ex_ratio)
{
    if (type(p) == disc_node) {
        halfword r = vlink(pre_break(p));
        while (r != null) {
            if (is_char_node(r))
                do_subst_font(r, ex_ratio);
            r = vlink(r);
        }
        r = vlink(post_break(p));
        while (r != null) {
            if (is_char_node(r))
                do_subst_font(r, ex_ratio);
            r = vlink(r);
        }
        r = vlink(no_break(p));
        while (r != null) {
            if (is_char_node(r))
                do_subst_font(r, ex_ratio);
            r = vlink(r);
        }
        return;
    }
    if (! is_char_node(p)) {
        normal_error("font expansion", "invalid node type");
        return;
    } else {
        internal_font_number f = font(p);
        int ef = get_ef_code(f, character(p));
        if (ef == 0)
            return;
        if ((font_max_stretch(f) > 0) && (ex_ratio > 0)) {
            int ex_stretch = ext_xn_over_d(ex_ratio * ef, font_max_stretch(f), 1000000);
            ex_glyph(p) = fix_expand_value(f, ex_stretch)*1000;
        } else if ((font_max_shrink(f) > 0) && (ex_ratio < 0)) {
            int ex_shrink = ext_xn_over_d(ex_ratio * ef, font_max_shrink(f), 1000000);
            ex_glyph(p) = fix_expand_value(f, ex_shrink)*1000;
        }
    }
}

@ @c
scaled char_pw(halfword p, int side)
{
    internal_font_number f;
    int c, w;
    if (side == left_side)
        last_leftmost_char = null;
    else
        last_rightmost_char = null;
    if (p == null)
        return 0;
    if (!is_char_node(p))
        return 0;
    f = font(p);
    if (side == left_side) {
        c = get_lp_code(f, character(p));
        last_leftmost_char = p;
    } else {
        c = get_rp_code(f, character(p));
        last_rightmost_char = p;
    }
    if (c == 0)
        return 0;
    w = quad(f);
    return round_xn_over_d(w, c, 1000);
}

@ @c
halfword new_margin_kern(scaled w, halfword p, int side)
{
    halfword k, q;
    k = new_node(margin_kern_node, side);
    width(k) = w;
    if (p == null)
        normal_error("margin kerning", "invalid pointer to marginal char node");
    q = new_char(font(p), character(p));
    margin_char(k) = q;
    return k;
}

@ Here is |hpack|, which is place where we do font substituting when font
expansion is being used.

@c
int font_expand_ratio = 0;  /* current expansion ratio, needed for recursive call */

int ignore_math_skip(halfword p)
{
    if (math_skip_mode == 6) {
        if (subtype(p) == after) {
            if (math_skip_boundary(vlink(p))) {
                return 0;
            }
        } else {
            if (math_skip_boundary(alink(p))) {
                return 0;
            }
        }
    } else if (math_skip_mode == 7) {
        if (subtype(p) == after) {
            if (! math_skip_boundary(vlink(p))) {
                return 0;
            }
        } else {
            if (! math_skip_boundary(alink(p))) {
                return 0;
            }
        }
    } else {
        return 0;
    }
    reset_glue_to_zero(p);
    return 1;
}

halfword hpack(halfword p, scaled w, int m, int pack_direction)
{
    halfword r;                 /* the box node that will be returned */
    halfword q;                 /* trails behind |p| */
    scaled h = 0;               /* height */
    scaled d = 0;               /* depth */
    scaled x = 0;               /* natural width */
    scaled_whd whd;
    scaled s;                   /* shift amount */
    int o;                      /* order of infinity */
    halfword dir_ptr1 = null;   /* for managing the direction stack */
    int hpack_dir;              /* the current direction */
    int disc_level = 0;
    halfword pack_interrupt[8];
    scaled font_stretch = 0;
    scaled font_shrink = 0;
    int adjust_spacing = adjust_spacing_par;

/*
    int font_expand_ratio = 0;
*/
    last_badness = 0;
    r = new_node(hlist_node, min_quarterword); /* the box node that will be returned */
    if (pack_direction == -1) {
        hpack_dir = text_direction_par;
    } else {
        hpack_dir = pack_direction;
    }
    box_dir(r) = hpack_dir;
    /*
        potential optimimization, save a little but neglectable in practice (not so
        many empty boxes are used)

        if (p == null) {
            width(r) = w;
            return r;
        }
    */
    push_dir(dir_ptr1,hpack_dir); /* push null */
    q = r + list_offset; /* hm, adding something to a node address? */
    vlink(q) = p;
    if (m == cal_expand_ratio) {
        prev_char_p = null; /* why not always */
    }
    if (adjust_spacing > 2) {
        adjust_spacing = 0;
    }
    total_stretch[normal] = 0;
    total_shrink[normal] = 0;
    total_stretch[sfi] = 0;
    total_shrink[sfi] = 0;
    total_stretch[fil] = 0;
    total_shrink[fil] = 0;
    total_stretch[fill] = 0;
    total_shrink[fill] = 0;
    total_stretch[filll] = 0;
    total_shrink[filll] = 0;

  RESWITCH:
    while ((p != null) || (disc_level > 0)) {
        if (p == null) {
            decr(disc_level);
            p = pack_interrupt[disc_level];
            goto RESWITCH;
        }
        /*
            Examine node |p| in the hlist, taking account of its effect
            on the dimensions of the new box, or moving it to the adjustment list;
            then advance |p| to the next node
        */
        while (is_char_node(p)) {
            /*
                Incorporate character dimensions into the dimensions of the hbox
                that will contain~it, then move to the next node.

                The following code is part of \TeX's inner loop; i.e., adding
                another character of text to the user's input will cause each of
                these instructions to be exercised one more time.
             */
            if (m >= cal_expand_ratio) {
                prev_char_p = p;
                if (m == cal_expand_ratio) {
                    font_stretch += char_stretch(p);
                    font_shrink += char_shrink(p);
                } else if (m == subst_ex_font) {
                    do_subst_font(p, font_expand_ratio);
                }
            }
            whd = pack_width_height_depth(hpack_dir, dir_TRT, p, true);
            x += whd.wd;
            if (whd.ht > h)
                h = whd.ht;
            if (whd.dp > d)
                d = whd.dp;
            p = vlink(p);
        }
        if (p != null) {
            switch (type(p)) {
                case hlist_node:
                case vlist_node:
                    /*
                        Incorporate box dimensions into the dimensions of the hbox
                        that will contain~it

                        The code here implicitly uses the fact that running dimensions are
                        indicated by |null_flag|, which will be ignored in the calculations
                        because it is a highly negative number.
                    */
                    s = shift_amount(p);
                    whd = pack_width_height_depth(hpack_dir, box_dir(p), p, false);
                    x += whd.wd;
                    if (whd.ht - s > h)
                        h = whd.ht - s;
                    if (whd.dp + s > d)
                        d = whd.dp + s;
                    break;
                /*
                case rule_node:
                case unset_node:
                    x += width(p);
                    if (type(p) >= rule_node) // always
                        s = 0;
                    else
                        s = shift_amount(p);
                    if (height(p) - s > h)
                        h = height(p) - s;
                    if (depth(p) + s > d)
                        d = depth(p) + s;
                    break;
                */
                case rule_node:
                case unset_node:
                    x += width(p);
                    if (height(p) > h)
                        h = height(p);
                    if (depth(p) > d)
                        d = depth(p);
                    break;
                /* */
                case math_node:
                    /* begin mathskip code */
                    if (glue_is_zero(p) || ignore_math_skip(p)) {
                        x += surround(p);
                        break;
                    } else {
                        /* fall through */
                    }
                    /* end mathskip code */
                case glue_node:
                    /* Incorporate glue into the horizontal totals */
                    x += width(p);
                    o = stretch_order(p);
                    total_stretch[o] = total_stretch[o] + stretch(p);
                    o = shrink_order(p);
                    total_shrink[o] = total_shrink[o] + shrink(p);
                    if (subtype(p) >= a_leaders) {
                        halfword g = leader_ptr(p);
                        if (height(g) > h)
                            h = height(g);
                        if (depth(g) > d)
                            d = depth(g);
                    }
                    break;
                case kern_node:
                    x += width(p);
                    if (subtype(p) == font_kern && adjust_spacing) {
                        /* so only when 1 or 2 */
                        if (m == cal_expand_ratio) {
                            font_stretch = font_stretch + kern_stretch(p);
                            font_shrink = font_shrink + kern_shrink(p);
                        } else if (m == subst_ex_font) {
                            /* this is the finalizer */
                            int k = 0;
                            if (font_expand_ratio > 0) {
                                k = kern_stretch(p);
                            } else if (font_expand_ratio < 0) {
                                k = kern_shrink(p);
                            }
                            ex_kern(p) = k;
                            x += k;
                            /*
                                if (x!=0) printf("SET %i %i %i\n",font_expand_ratio,k,x);
                            */
                        }
                    }
                    break;
                case disc_node:
                    if (m == subst_ex_font)
                        do_subst_font(p, font_expand_ratio);
                    if ((subtype(p) != select_disc) && (vlink(no_break(p)) != null)) {
                        pack_interrupt[disc_level] = vlink(p);
                        incr(disc_level);
                        p = no_break(p);
                    }
                    break;
                case dir_node:
                    /* Adjust the dir stack for the |hpack| routine */
                    if (dir_dir(p) >= 0) {
                        hpack_dir = dir_dir(p);
                        push_dir_node(dir_ptr1,p);
                    } else {
                        pop_dir_node(dir_ptr1);
                        if (dir_ptr1 != null)
                            hpack_dir = dir_dir(dir_ptr1);
                    }
                    break;
                case margin_kern_node:
                    if (m == cal_expand_ratio) {
                        int f = font(margin_char(p));
                        do_subst_font(margin_char(p), 1000);
                        if (f != font(margin_char(p)))
                            font_stretch = font_stretch - width(p) - char_pw(margin_char(p), subtype(p));
                        font(margin_char(p)) = f;
                        do_subst_font(margin_char(p), -1000);
                        if (f != font(margin_char(p)))
                            font_shrink = font_shrink - width(p) - char_pw(margin_char(p), subtype(p));
                        font(margin_char(p)) = f;
                    } else if (m == subst_ex_font) {
                        do_subst_font(margin_char(p), font_expand_ratio);
                        width(p) = -char_pw(margin_char(p), subtype(p));
                    }
                    x += width(p);
                    break;
                case ins_node:
                case mark_node:
                case adjust_node:
                    /*
                        Transfer node |p| to the adjustment list.

                        Although node |q| is not necessarily the immediate predecessor of node |p|,
                        it always points to some node in the list preceding |p|. Thus, we can delete
                        nodes by moving |q| when necessary. The algorithm takes linear time, and the
                        extra computation does not intrude on the inner loop unless it is necessary
                        to make a deletion.
                     */
                    if (adjust_tail != null || pre_adjust_tail != null) {
                        while (vlink(q) != p)
                            q = vlink(q);
                        if (type(p) == adjust_node) {
                            if (adjust_pre(p) != 0)
                                update_adjust_list(pre_adjust_tail);
                            else
                                update_adjust_list(adjust_tail);
                            p = vlink(p);
                            adjust_ptr(vlink(q)) = null;
                            flush_node(vlink(q));
                        } else {
                            vlink(adjust_tail) = p;
                            adjust_tail = p;
                            p = vlink(p);
                        }
                        vlink(q) = p;
                        p = q;
                    }
                    break;
                /* */
                default:
                    break;
            }
            p = vlink(p);
        }

    }

    if (adjust_tail != null)
        vlink(adjust_tail) = null;
    if (pre_adjust_tail != null)
        vlink(pre_adjust_tail) = null;
    height(r) = h;
    depth(r) = d;
    /*
        Determine the value of |width(r)| and the appropriate glue setting; then
        |return| or |goto common_ending|.

        When we get to the present part of the program, |x| is the natural width
        of the box being packaged.
    */
    if (m == additional)
        w = x + w;
    width(r) = w;
    x = w - x;
    /* now |x| is the excess to be made up */
    if (x == 0) {
        glue_sign(r) = normal;
        glue_order(r) = normal;
        set_glue_ratio_zero(glue_set(r));
        goto EXIT;
    } else if (x > 0) {
        /*
            Determine horizontal glue stretch setting, then |return|
            or \hbox{|goto common_ending|}.

            If |hpack| is called with |m=cal_expand_ratio| we calculate
            |font_expand_ratio| and return without checking for overfull or
            underfull box.
        */
        if (total_stretch[filll] != 0)
            o = filll;
        else if (total_stretch[fill] != 0)
            o = fill;
        else if (total_stretch[fil] != 0)
            o = fil;
        else if (total_stretch[sfi] != 0)
            o = sfi;
        else
            o = normal;

        if ((m == cal_expand_ratio) && (o == normal) && (font_stretch > 0)) {
            font_expand_ratio = divide_scaled_n(x, font_stretch, 1000.0);
            goto EXIT;
        }
        glue_order(r) = (quarterword) o;
        glue_sign(r) = stretching;
        if (total_stretch[o] != 0) {
            glue_set(r) = unfloat((double) x / total_stretch[o]);
        } else {
            /* there's nothing to stretch */
            glue_sign(r) = normal;
            set_glue_ratio_zero(glue_set(r));
        }
        if (o == normal) {
            if (list_ptr(r) != null) {
                /*
                    Report an underfull hbox and |goto common_ending|, if this box
                    is sufficiently bad.
                */
                last_badness = badness(x, total_stretch[normal]);
                if (last_badness > hbadness_par) {
                    int callback_id = callback_defined(hpack_quality_callback);
                    if (callback_id > 0) {
                        halfword rule = null;
                        if (last_badness > 100) {
                            run_callback(callback_id, "SdNdd->N","underfull",last_badness,r,abs(pack_begin_line),line,&rule);
                        } else {
                            run_callback(callback_id, "SdNdd->N","loose",last_badness,r,abs(pack_begin_line),line,&rule);
                        }
                        if (rule != null) {
                            while (vlink(q) != null) {
                                q = vlink(q);
                            }
                            couple_nodes(q,rule);
                        }
                    } else {
                        print_ln();
                        if (last_badness > 100) {
                            tprint_nl("Underfull \\hbox (badness ");
                        } else {
                            tprint_nl("Loose \\hbox (badness ");
                        }
                        print_int(last_badness);
                        goto COMMON_ENDING;
                    }
                }
            }
        }
        goto EXIT;
    } else {
        /*
            Determine horizontal glue shrink setting, then |return|
            or \hbox{|goto common_ending|},
        */
        if (total_shrink[filll] != 0)
            o = filll;
        else if (total_shrink[fill] != 0)
            o = fill;
        else if (total_shrink[fil] != 0)
            o = fil;
        else if (total_shrink[sfi] != 0)
            o = sfi;
        else
            o = normal;

        if ((m == cal_expand_ratio) && (o == normal) && (font_shrink > 0)) {
            font_expand_ratio = divide_scaled_n(x, font_shrink, 1000.0);
            goto EXIT;
        }
        glue_order(r) = (quarterword) o;
        glue_sign(r) = shrinking;
        if (total_shrink[o] != 0) {
            glue_set(r) = unfloat((double) (-x) / (double) total_shrink[o]);
        } else {
            /* there's nothing to shrink */
            glue_sign(r) = normal;
            set_glue_ratio_zero(glue_set(r));
        }
        if ((total_shrink[o] < -x) && (o == normal) && (list_ptr(r) != null)) {
            int overshoot = -x - total_shrink[normal] ;
            last_badness = 1000000;
            /* use the maximum shrinkage */
            set_glue_ratio_one(glue_set(r));
            /*
                Report an overfull hbox and |goto common_ending|, if this box
                is sufficiently bad.
            */
            if ((overshoot > hfuzz_par) || (hbadness_par < 100)) {
                int callback_id = callback_defined(hpack_quality_callback);
                halfword rule = null;
                if (callback_id > 0) {
                    run_callback(callback_id, "SdNdd->N","overfull",overshoot,r,abs(pack_begin_line),line,&rule);
                } else if (overfull_rule_par > 0) {
                    rule = new_rule(normal_rule);
                    rule_dir(rule) = box_dir(r);
                    width(rule) = overfull_rule_par;
                }
                if (rule != null) {
                    while (vlink(q) != null) {
                        q = vlink(q);
                    }
                    couple_nodes(q,rule);
                }
                if (callback_id == 0) {
                    print_ln();
                    tprint_nl("Overfull \\hbox (");
                    print_scaled(overshoot);
                    tprint("pt too wide");
                    goto COMMON_ENDING;
                }
            }
        } else if (o == normal) {
            if (list_ptr(r) != null) {
                /*
                    Report a tight hbox and |goto common_ending|, if this box is
                    sufficiently bad.
                */
                last_badness = badness(-x, total_shrink[normal]);
                if (last_badness > hbadness_par) {
                    int callback_id = callback_defined(hpack_quality_callback);
                    if (callback_id > 0) {
                        halfword rule = null;
                        run_callback(callback_id, "SdNdd->N","tight",last_badness,r,abs(pack_begin_line),line,&rule);
                        if (rule != null) {
                            while (vlink(q) != null) {
                                q = vlink(q);
                            }
                            couple_nodes(q,rule);
                        }
                    } else {
                        print_ln();
                        tprint_nl("Tight \\hbox (badness ");
                        print_int(last_badness);
                        goto COMMON_ENDING;
                    }
                }
            }
        }
        goto EXIT;
    }

  COMMON_ENDING:
    /*
        Finish issuing a diagnostic message for an overfull or underfull
        hbox.
    */
    if (output_active) {
        tprint(") has occurred while \\output is active");
    } else {
        if (pack_begin_line != 0) {
            if (pack_begin_line > 0) {
                tprint(") in paragraph at lines ");
            } else {
                tprint(") in alignment at lines ");
            }
            print_int(abs(pack_begin_line));
            tprint("--");
        } else {
            tprint(") detected at line ");
        }
        print_int(line);
    }

    print_ln();
    font_in_short_display = null_font;
    short_display(list_ptr(r));
    print_ln();
    begin_diagnostic();
    show_box(r);
    end_diagnostic(true);
  EXIT:
    if ((m == cal_expand_ratio) && (font_expand_ratio != 0)) {
        font_expand_ratio = fix_int(font_expand_ratio, -1000, 1000);
        q = list_ptr(r);
        list_ptr(r) = null;
        flush_node(r);
        /* this nested call uses the more or less global font_expand_ratio */
        r = hpack(q, w, subst_ex_font, hpack_dir);
    }
    while (dir_ptr1 != null)
        pop_dir_node(dir_ptr1);
    /* here we reset the font_expan_ratio */
    font_expand_ratio = 0;
    return r;
}

@ @c
halfword filtered_hpack(halfword p, halfword qt, scaled w, int m, int grp, int pac, int just_pack, halfword attr)
{
    halfword q;
    if (just_pack) {
        q = vlink(p);
    } else if (type(p) == temp_node && vlink(p) == null) {
        q = vlink(p);
        /*
            q = new_node(hlist_node, min_quarterword);
            box_dir(q) = (pac == -1) ? text_direction_par : pac;
            width(q) = w;
            return q;
        */
    } else {
        new_hyphenation(p, qt);
        (void) new_ligkern(p, qt);  /* we don't care about the tail in this case */
        q = vlink(p);
        /* maybe here: alink(p) = null */
        q = lua_hpack_filter(q, w, m, grp, pac, attr); /* ignores empty anyway */ /* maybe also pass tail */
    }
    return hpack(q, w, m, pac);
}

@ here is a function to calculate the natural whd of a (horizontal) node list

@c
scaled_whd natural_sizes(halfword p, halfword pp, glue_ratio g_mult,
                         int g_sign, int g_order, int pack_direction)
{
    scaled s;      /* shift amount */
    halfword g;    /* points to a glue specification */
    int hpack_dir;
    scaled_whd xx; /* for recursion */
    scaled_whd whd, siz = { 0, 0, 0 };
    scaled gp = 0;
    scaled gm = 0;
    if (pack_direction == -1) {
        hpack_dir = text_direction_par;
    } else {
        hpack_dir = pack_direction;
    }
    while (p != pp && p != null) {
        while (is_char_node(p) && p != pp) {
            whd = pack_width_height_depth(hpack_dir, dir_TRT, p, true);
            siz.wd += whd.wd;
            if (whd.ht > siz.ht)
                siz.ht = whd.ht;
            if (whd.dp > siz.dp)
                siz.dp = whd.dp;
            p = vlink(p);
        }
        if (p != pp && p != null) {
            switch (type(p)) {
                case hlist_node:
                case vlist_node:
                    s = shift_amount(p);
                    whd = pack_width_height_depth(hpack_dir, box_dir(p), p, false);
                    siz.wd += whd.wd;
                    if (whd.ht - s > siz.ht)
                        siz.ht = whd.ht - s;
                    if (whd.dp + s > siz.dp)
                        siz.dp = whd.dp + s;
                    break;
                /*
                case rule_node:
                case unset_node:
                    siz.wd += width(p);
                    if (type(p) >= rule_node) // always true
                        s = 0;
                    else
                        s = shift_amount(p);
                    if (height(p) - s > siz.ht)
                        siz.ht = height(p) - s;
                    if (depth(p) + s > siz.dp)
                        siz.dp = depth(p) + s;
                    break;
                */
                case rule_node:
                case unset_node:
                    siz.wd += width(p);
                    if (height(p) > siz.ht)
                        siz.ht = height(p);
                    if (depth(p) > siz.dp)
                        siz.dp = depth(p);
                    break;
                /* */
                case math_node:
                    /* begin mathskip code */
                    if (glue_is_zero(p) || ignore_math_skip(p)) {
                        siz.wd += surround(p);
                        break;
                    } else {
                        /* fall through */
                    }
                    /* end mathskip code */
                case glue_node:
                    siz.wd += width(p);
                    if (g_sign != normal) {
                        if (g_sign == stretching) {
                            if (stretch_order(p) == g_order) {
                                /*
                                    siz.wd += float_round(float_cast(g_mult) * float_cast(stretch(p)));
                                */
                                gp += stretch(p);
                            }
                        } else if (shrink_order(p) == g_order) {
                            /*
                                siz.wd -= float_round(float_cast(g_mult) * float_cast(shrink(p)));
                            */
                            gm += shrink(p);
                        }
                    }
                    if (subtype(p) >= a_leaders) {
                        g = leader_ptr(p);
                        if (height(g) > siz.ht)
                            siz.ht = height(g);
                        if (depth(g) > siz.dp)
                            siz.dp = depth(g);
                    }
                    break;
                case margin_kern_node:
                    siz.wd += width(p);
                    break;
                case kern_node:
                    siz.wd += width(p) + ex_kern(p);
                    break;
                case disc_node:
                    xx = natural_sizes(no_break(p), null, g_mult, g_sign, g_order, hpack_dir);
                    siz.wd += xx.wd;
                    if (xx.ht > siz.ht)
                        siz.ht = xx.ht;
                    if (xx.dp > siz.dp)
                        siz.dp = xx.dp;
                    break;
                default:
                    break;
            }
            p = vlink(p);
        }

    }
    if (g_sign != normal) {
        if (g_sign == stretching) {
            siz.wd += float_round(float_cast(g_mult) * float_cast(gp));
        } else {
            siz.wd -= float_round(float_cast(g_mult) * float_cast(gm));
        }
    }
    return siz;
}

@ In order to provide a decent indication of where an overfull or underfull box
originated, we use a global variable |pack_begin_line| that is set nonzero only
when |hpack| is being called by the paragraph builder or the alignment finishing
routine.

@ The source file line where the current paragraph or alignment began; a negative
value denotes alignment:

@c
int pack_begin_line;

@ The |vpack| subroutine is actually a special case of a slightly more general
routine called |vpackage|, which has four parameters. The fourth parameter, which
is |max_dimen| in the case of |vpack|, specifies the maximum depth of the page
box that is constructed. The depth is first computed by the normal rules; if it
exceeds this limit, the reference point is simply moved down until the limiting
depth is attained.

@c
halfword vpackage(halfword p, scaled h, int m, scaled l, int pack_direction)
{
    halfword r;                 /* the box node that will be returned */
    scaled w = 0;               /* width */
    scaled d = 0;               /* depth */
    scaled x = 0;               /* natural height */
    scaled_whd whd;
    scaled s;                   /* shift amount */
    int o;                      /* order of infinity */
    last_badness = 0;
    r = new_node(vlist_node, 0);
    if (pack_direction == -1) {
        box_dir(r) = body_direction_par;
    } else {
        box_dir(r) = pack_direction;
    }
    subtype(r) = min_quarterword;
    shift_amount(r) = 0;
    list_ptr(r) = p;
    total_stretch[normal] = 0;
    total_shrink[normal] = 0;
    total_stretch[sfi] = 0;
    total_shrink[sfi] = 0;
    total_stretch[fil] = 0;
    total_shrink[fil] = 0;
    total_stretch[fill] = 0;
    total_shrink[fill] = 0;
    total_stretch[filll] = 0;
    total_shrink[filll] = 0;

    while (p != null) {
        /*
            Examine node |p| in the vlist, taking account of its effect
            on the dimensions of the new box; then advance |p| to the next
            node.
        */
        if (is_char_node(p)) {
            confusion("vpack");
        } else {
            switch (type(p)) {
            case hlist_node:
            case vlist_node:
                /*
                    Incorporate box dimensions into the dimensions of
                    the vbox that will contain it.
                */
                s = shift_amount(p);
                whd = pack_width_height_depth(box_dir(r), box_dir(p), p, false);
                if (whd.wd + s > w)
                    w = whd.wd + s;
                x += d + whd.ht;
                d = whd.dp;
                break;
            /*
            case rule_node:
            case unset_node:
                x += d + height(p);
                d = depth(p);
                if (type(p) >= rule_node) // always
                    s = 0;
                else
                    s = shift_amount(p);
                if (width(p) + s > w)
                    w = width(p) + s;
                break;
            */
            case rule_node:
            case unset_node:
                x += d + height(p);
                d = depth(p);
                if (width(p) > w)
                    w = width(p);
                break;
            /* */
            case glue_node:
                /* Incorporate glue into the vertical totals */
                x += d;
                d = 0;
                x += width(p);
                o = stretch_order(p);
                total_stretch[o] = total_stretch[o] + stretch(p);
                o = shrink_order(p);
                total_shrink[o] = total_shrink[o] + shrink(p);
                if (subtype(p) >= a_leaders) {
                    halfword g = leader_ptr(p);
                    if (width(g) > w)
                        w = width(g);
                }
                break;
            case kern_node:
                x += d + width(p);
                d = 0;
                break;
            default:
                break;
            }
        }
        p = vlink(p);
    }
    width(r) = w;
    if (d > l) {
        x += d - l;
        depth(r) = l;
    } else {
        depth(r) = d;
    }
    /*
        Determine the value of |height(r)| and the appropriate glue setting;
        then |return| or |goto common_ending|.

        When we get to the present part of the program, |x| is the natural
        height of the box being packaged.
    */
    if (m == additional)
        h = x + h;
    height(r) = h;
    x = h - x;
    /* now |x| is the excess to be made up */
    if (x == 0) {
        glue_sign(r) = normal;
        glue_order(r) = normal;
        set_glue_ratio_zero(glue_set(r));
        return r;
    } else if (x > 0) {
        /*
            Determine vertical glue stretch setting, then |return|
            or \hbox{|goto common_ending|}.
        */
        if (total_stretch[filll] != 0)
            o = filll;
        else if (total_stretch[fill] != 0)
            o = fill;
        else if (total_stretch[fil] != 0)
            o = fil;
        else if (total_stretch[sfi] != 0)
            o = sfi;
        else
            o = normal;

        glue_order(r) = (quarterword) o;
        glue_sign(r) = stretching;
        if (total_stretch[o] != 0) {
            glue_set(r) = unfloat((double) x / total_stretch[o]);
        } else {
            glue_sign(r) = normal;
            set_glue_ratio_zero(glue_set(r));   /* there's nothing to stretch */
        }
        if (o == normal) {
            if (list_ptr(r) != null) {
                /*
                    Report an underfull vbox and |goto common_ending|, if this box
                    is sufficiently bad.
                */
                last_badness = badness(x, total_stretch[normal]);
                if (last_badness > vbadness_par) {
                    int callback_id = callback_defined(vpack_quality_callback);
                    if (callback_id > 0) {
                        if (last_badness > 100) {
                            run_callback(callback_id, "SdNdd->","underfull",last_badness,r,abs(pack_begin_line),line);
                        } else {
                            run_callback(callback_id, "SdNdd->","loose",last_badness,r,abs(pack_begin_line),line);
                        }
                        goto EXIT;
                    } else {
                        print_ln();
                        if (last_badness > 100) {
                            tprint_nl("Underfull \\vbox (badness ");
                        } else {
                            tprint_nl("Loose \\vbox (badness ");
                        }
                        print_int(last_badness);
                        goto COMMON_ENDING;
                    }
                }
            }
        }
        return r;

    } else {
        /*
            Determine vertical glue shrink setting, then |return|
            or \hbox{|goto common_ending|}.
        */
        if (total_shrink[filll] != 0)
            o = filll;
        else if (total_shrink[fill] != 0)
            o = fill;
        else if (total_shrink[fil] != 0)
            o = fil;
        else if (total_shrink[sfi] != 0)
            o = sfi;
        else
            o = normal;

        glue_order(r) = (quarterword) o;
        glue_sign(r) = shrinking;
        if (total_shrink[o] != 0) {
            glue_set(r) = unfloat((double) (-x) / total_shrink[o]);
        } else {
            /* there's nothing to shrink */
            glue_sign(r) = normal;
            set_glue_ratio_zero(glue_set(r));
        }
        if ((total_shrink[o] < -x) && (o == normal) && (list_ptr(r) != null)) {
            int overshoot = -x - total_shrink[normal];
            last_badness = 1000000;
            /* use the maximum shrinkage */
            set_glue_ratio_one(glue_set(r));
            /*
                Report an overfull vbox and |goto common_ending|, if this box
                is sufficiently bad.
            */
            if ((overshoot > vfuzz_par) || (vbadness_par < 100)) {
                int callback_id = callback_defined(vpack_quality_callback);
                if (callback_id > 0) {
                    run_callback(callback_id, "SdNdd->","overfull",overshoot,r,abs(pack_begin_line),line);
                    goto EXIT;
                } else {
                    print_ln();
                    tprint_nl("Overfull \\vbox (");
                    print_scaled(-x - total_shrink[normal]);
                    tprint("pt too high");
                    goto COMMON_ENDING;
                }
            }
        } else if (o == normal) {
            if (list_ptr(r) != null) {
                /*
                    Report a tight vbox and |goto common_ending|, if this box is
                    sufficiently bad.
                */
                last_badness = badness(-x, total_shrink[normal]);
                if (last_badness > vbadness_par) {
                    int callback_id = callback_defined(vpack_quality_callback);
                    if (callback_id > 0) {
                        run_callback(callback_id, "SdNdd->","tight",last_badness,r,abs(pack_begin_line),line);
                        goto EXIT;
                    } else {
                        print_ln();
                        tprint_nl("Tight \\vbox (badness ");
                        print_int(last_badness);
                        goto COMMON_ENDING;
                    }
                }
            }
        }
        return r;
    }

  COMMON_ENDING:
    /* Finish issuing a diagnostic message or an overfull or underfull vbox */
    if (output_active) {
        tprint(") has occurred while \\output is active");
    } else {
        if (pack_begin_line != 0) {
            /* it's actually negative */
            tprint(") in alignment at lines ");
            print_int(abs(pack_begin_line));
            tprint("--");
        } else {
            tprint(") detected at line ");
        }
        print_int(line);
        print_ln();
    }
    begin_diagnostic();
    show_box(r);
    end_diagnostic(true);
  EXIT:
    return r;
}

@ @c
halfword filtered_vpackage(halfword p, scaled h, int m, scaled l, int grp, int pack_direction, int just_pack, halfword attr)
{
    halfword q = p;
    if (!just_pack)
 /* if (q != null) */
        q = lua_vpack_filter(q, h, m, l, grp, pack_direction, attr);
    return vpackage(q, h, m, l, pack_direction);
}

@ @c
void finish_vcenter(void)
{
    halfword p;
    unsave();
    save_ptr--;
    p = vpack(vlink(cur_list.head_field), saved_value(0), saved_level(0), -1);
    pop_nest();
    p = math_vcenter_group(p);
    tail_append(p);
}

@ @c
void package(int c)
{
    halfword saved0, saved2, saved3, saved4;
    int grp = cur_group;
    scaled d = box_max_depth_par; /* max depth */
    unsave();
    save_ptr -= 5;
    saved0 = saved_value(0);
    saved2 = saved_value(2);
    saved3 = saved_value(3);
    saved4 = saved_value(4);
    if (cur_list.mode_field == -hmode) {
        cur_box = filtered_hpack(cur_list.head_field, cur_list.tail_field,
            saved_value(1), saved_level(1), grp, saved_level(2), saved4, saved3);
        subtype(cur_box) = hbox_list;
    } else {
        cur_box = filtered_vpackage(vlink(cur_list.head_field),
            saved_value(1), saved_level(1), d, grp, saved_level(2), saved4, saved3);
        if (c == vtop_code) {
            /*
                Read just the height and depth of |cur_box|, for \.{\\vtop}. The
                height of a `\.{\\vtop}' box is inherited from the first item on
                its list, if that item is an |hlist_node|, |vlist_node|, or
                |rule_node|; otherwise the \.{\\vtop} height is zero.
            */
            scaled h = 0;
            halfword p = list_ptr(cur_box);
            if ((p != null) && (type(p) <= rule_node)) {
                /* hlist, vlist, rule */
                h = height(p);
            }
            depth(cur_box) = depth(cur_box) - h + height(cur_box);
            height(cur_box) = h;
        }
    }
    if (saved2 != null) {
        /* DIR: Adjust back |text_dir_ptr| for |scan_spec| */
        flush_node_list(text_dir_ptr);
        text_dir_ptr = saved2;

    }
    replace_attribute_list(cur_box, saved3);
    pop_nest();
    box_end(saved0);
}

@ When a box is being appended to the current vertical list, the baselineskip
calculation is handled by the |append_to_vlist| routine.

@c
void append_to_vlist(halfword b, int location)
{
    scaled d;   /* deficiency of space between baselines */
    halfword p; /* a new glue node */
    boolean mirrored = (type(b) == hlist_node) && is_mirrored(box_dir(b)) ;
    halfword result = null;
    halfword next_depth = ignore_depth;
    boolean prev_set = false ;
    if (lua_appendtovlist_callback(b,location,prev_depth_par,mirrored,&result,&next_depth,&prev_set)) {
        while (result != null) {
            couple_nodes(cur_list.tail_field, result);
            cur_list.tail_field = result;
            result = vlink(result);
        }
        if (prev_set) {
            prev_depth_par = next_depth;
        }
    } else {
        if (prev_depth_par > ignore_depth) {
            if (mirrored) {
                d = width(baseline_skip_par) - prev_depth_par - depth(b);
            } else {
                d = width(baseline_skip_par) - prev_depth_par - height(b);
            }
            if (d < line_skip_limit_par) {
                p = new_param_glue(line_skip_code);
            } else {
                p = new_skip_param(baseline_skip_code);
                width(p) = d;
            }
            couple_nodes(cur_list.tail_field, p);
            cur_list.tail_field = p;
        }
        couple_nodes(cur_list.tail_field, b);
        cur_list.tail_field = b;
        if (mirrored) {
            prev_depth_par = height(b);
        } else {
            prev_depth_par = depth(b);
        }
    }
}

@ When |saving_vdiscards| is positive then the glue, kern, and penalty nodes
removed by the page builder or by \.{\\vsplit} from the top of a vertical list
are saved in special lists instead of being discarded.

@c
#define tail_page_disc disc_ptr[copy_code]  /* last item removed by page builder */
#define page_disc disc_ptr[last_box_code]   /* first item removed by page builder */
#define split_disc disc_ptr[vsplit_code]    /* first item removed by \.{\\vsplit} */

halfword disc_ptr[(vsplit_code + 1)];       /* list pointers */

@ The |vsplit| procedure, which implements \TeX's \.{\\vsplit} operation, is
considerably simpler than |line_break| because it doesn't have to worry about
hyphenation, and because its mission is to discover a single break instead of an
optimum sequence of breakpoints. But before we get into the details of |vsplit|,
we need to consider a few more basic things.

A subroutine called |prune_page_top| takes a pointer to a vlist and returns a
pointer to a modified vlist in which all glue, kern, and penalty nodes have been
deleted before the first box or rule node. However, the first box or rule is
actually preceded by a newly created glue node designed so that the topmost
baseline will be at distance |split_top_skip| from the top, whenever this is
possible without backspacing.

When the second argument |s| is |false| the deleted nodes are destroyed,
otherwise they are collected in a list starting at |split_disc|.

@c
halfword prune_page_top(halfword p, boolean s)
{
    halfword q;
    halfword prev_p = temp_head; /* lags one step behind |p| */
    halfword r = null;
    vlink(temp_head) = p;
    while (p != null) {
        switch (type(p)) {
        case hlist_node:
        case vlist_node:
        case rule_node:
            /* Insert glue for |split_top_skip| and set~|p:=null| */
            q = new_skip_param(split_top_skip_code);
            vlink(prev_p) = q;
            vlink(q) = p;
            if (width(q) > height(p))
                width(q) = width(q) - height(p);
            else
                width(q) = 0;
            p = null;
            break;
        case boundary_node:
        case whatsit_node:
        case mark_node:
        case ins_node:
            prev_p = p;
            p = vlink(prev_p);
            break;
        case glue_node:
        case kern_node:
        case penalty_node:
            q = p;
            p = vlink(q);
            vlink(q) = null;
            vlink(prev_p) = p;
            if (s) {
                if (split_disc == null)
                    split_disc = q;
                else
                    vlink(r) = q;
                r = q;
            } else {
                flush_node_list(q);
            }
            break;
        default:
            confusion("pruning");
            break;
        }
    }
    return vlink(temp_head);
}

@ The next subroutine finds the best place to break a given vertical list so as
to obtain a box of height~|h|, with maximum depth~|d|. A pointer to the beginning
of the vertical list is given, and a pointer to the optimum breakpoint is
returned. The list is effectively followed by a forced break, i.e., a penalty
node with the |eject_penalty|; if the best break occurs at this artificial node,
the value |null| is returned.

@c
scaled active_height[10]; /* distance from first active node to~|cur_p| */

@ An array of six |scaled| distances is used to keep track of the height from the
beginning of the list to the current place, just as in |line_break|. In fact, we
use one of the same arrays, only changing its name to reflect its new
significance.

@c
#define do_all_six(A) A(1);A(2);A(3);A(4);A(5);A(6);A(7)
#define set_height_zero(A) active_height[A]=0   /* initialize the height to zero */

@ A global variable |best_height_plus_depth| will be set to the natural size of
the box that corresponds to the optimum breakpoint found by |vert_break|. (This
value is used by the insertion-splitting algorithm of the page builder.)

@ height of the best box, without stretching or shrinking

@c
scaled best_height_plus_depth;

/* finds optimum page break */

halfword vert_break(halfword p, scaled h, scaled d)
{
    halfword prev_p = p;        /* if |p| is a glue node, |type(prev_p)| determines whether |p| is a
                                   legal breakpoint, an initial glue node is not a legal breakpoint */
    int pi = 0;                 /* penalty value */
    int b;                      /* badness at a trial breakpoint */
    int t;                      /* |type| of the node following a kern */
    int least_cost;             /* the smallest badness plus penalties found so far */
    halfword best_place = null; /* the most recent break that leads to |least_cost| */
    scaled prev_dp = 0;         /* depth of previous box in the list */
    least_cost = awful_bad;
    do_all_six(set_height_zero);
    while (1) {
        /* If node |p| is a legal breakpoint, check if this break is
           the best known, and |goto done| if |p| is null or
           if the page-so-far is already too full to accept more stuff */
        /* A subtle point to be noted here is that the maximum depth~|d| might be
           negative, so |cur_height| and |prev_dp| might need to be corrected even
           after a glue or kern node. */

        if (p == null) {
            pi = eject_penalty;
        } else {
            /* Use node |p| to update the current height and depth measurements;
               if this node is not a legal breakpoint, |goto not_found|
               or |update_heights|,
               otherwise set |pi| to the associated penalty at the break */
            switch (type(p)) {
            case hlist_node:
            case vlist_node:
            case rule_node:
                cur_height = cur_height + prev_dp + height(p);
                prev_dp = depth(p);
                goto NOT_FOUND;
                break;
            case boundary_node:
            case whatsit_node:
                goto NOT_FOUND;
                break;
            case glue_node:
                if (precedes_break(prev_p))
                    pi = 0;
                else
                    goto UPDATE_HEIGHTS;
                break;
            case kern_node:
                if (vlink(p) == null)
                    t = penalty_node;
                else
                    t = type(vlink(p));
                if (t == glue_node)
                    pi = 0;
                else
                    goto UPDATE_HEIGHTS;
                break;
            case penalty_node:
                pi = penalty(p);
                break;
            case mark_node:
            case ins_node:
                goto NOT_FOUND;
                break;
            default:
                confusion("vertbreak");
                break;
            }
        }
        /* Check if node |p| is a new champion breakpoint; then |goto done|
           if |p| is a forced break or if the page-so-far is already too full */
        if (pi < inf_penalty) {
            /* Compute the badness, |b|, using |awful_bad| if the box is too full */
            if (cur_height < h) {
                if ((active_height[3] != 0) || (active_height[4] != 0) ||
                    (active_height[5] != 0) || (active_height[6] != 0))
                    b = 0;
                else
                    b = badness(h - cur_height, active_height[2]);
            } else if (cur_height - h > active_height[7]) {
                b = awful_bad;
            } else {
                b = badness(cur_height - h, active_height[7]);
            }

            if (b < awful_bad) {
                if (pi <= eject_penalty)
                    b = pi;
                else if (b < inf_bad)
                    b = b + pi;
                else
                    b = deplorable;
            }
            if (b <= least_cost) {
                best_place = p;
                least_cost = b;
                best_height_plus_depth = cur_height + prev_dp;
            }
            if ((b == awful_bad) || (pi <= eject_penalty))
                goto DONE;
        }

        if ((type(p) < glue_node) || (type(p) > kern_node))
            goto NOT_FOUND;
      UPDATE_HEIGHTS:
        /* Update the current height and depth measurements with
           respect to a glue or kern node~|p| */
        /* Vertical lists that are subject to the |vert_break| procedure should not
           contain infinite shrinkability, since that would permit any amount of
           information to ``fit'' on one page. */

        if (type(p) != kern_node) {
            active_height[2 + stretch_order(p)] += stretch(p);
            active_height[7] += shrink(p);
            if ((shrink_order(p) != normal) && (shrink(p) != 0)) {
                print_err("Infinite glue shrinkage found in box being split");
                help4("The box you are \\vsplitting contains some infinitely",
                      "shrinkable glue, e.g., `\\vss' or `\\vskip 0pt minus 1fil'.",
                      "Such glue doesn't belong there; but you can safely proceed,",
                      "since the offensive shrinkability has been made finite.");
                error();
                shrink_order(p) = normal;
            }
        }
        cur_height = cur_height + prev_dp + width(p);
        prev_dp = 0;
      NOT_FOUND:
        if (prev_dp > d) {
            cur_height = cur_height + prev_dp - d;
            prev_dp = d;
        }
        prev_p = p;
        p = vlink(prev_p);
    }
  DONE:
    return best_place;
}

@ Now we are ready to consider |vsplit| itself. Most of its work is accomplished
by the two subroutines that we have just considered.

Given the number of a vlist box |n|, and given a desired page height |h|, the
|vsplit| function finds the best initial segment of the vlist and returns a box
for a page of height~|h|. The remainder of the vlist, if any, replaces the
original box, after removing glue and penalties and adjusting for
|split_top_skip|. Mark nodes in the split-off box are used to set the values of
|split_first_mark| and |split_bot_mark|; we use the fact that
|split_first_mark(x)=null| if and only if |split_bot_mark(x)=null|.

The original box becomes ``void'' if and only if it has been entirely extracted.
The extracted box is ``void'' if and only if the original box was void (or if it
was, erroneously, an hlist box).

@c
/* extracts a page of height |h| from box |n| */

halfword vsplit(halfword n, scaled h, int m)
{
    halfword v;  /* the box to be split */
    int vdir;    /* the direction of the box to be split */
    halfword p;  /* runs through the vlist */
    halfword q;  /* points to where the break occurs */
    halfword i;  /* for traversing marks lists */
    v = box(n);
    vdir = box_dir(v);
    flush_node_list(split_disc);
    split_disc = null;
    for (i = 0; i <= biggest_used_mark; i++) {
        delete_split_first_mark(i);
        delete_split_bot_mark(i);
    }
    /* Dispense with trivial cases of void or bad boxes */
    if (v == null) {
        return null;
    }
    if (type(v) != vlist_node) {
        print_err("\\vsplit needs a \\vbox");
        help2("The box you are trying to split is an \\hbox.",
              "i can't split such a box, so I''ll leave it alone.");
        error();
        return null;
    }
    q = vert_break(list_ptr(v), h, split_max_depth_par);
    /*
        Look at all the marks in nodes before the break, and set the final
        link to |null| at the break. It's possible that the box begins with
        a penalty node that is the ``best'' break, so we must be careful to
        handle this special case correctly.
    */
    p = list_ptr(v);
    if (p == q) {
        list_ptr(v) = null;
    } else {
        while (1) {
            if (type(p) == mark_node) {
                if (split_first_mark(mark_class(p)) == null) {
                    set_split_first_mark(mark_class(p), mark_ptr(p));
                    set_split_bot_mark(mark_class(p), split_first_mark(mark_class(p)));
                    set_token_ref_count(split_first_mark(mark_class(p)),
                        token_ref_count(split_first_mark(mark_class(p))) + 2);
                } else {
                    delete_token_ref(split_bot_mark(mark_class(p)));
                    set_split_bot_mark(mark_class(p), mark_ptr(p));
                    add_token_ref(split_bot_mark(mark_class(p)));
                }
            }
            if (vlink(p) == q) {
                vlink(p) = null;
                break;
            }
            p = vlink(p);
        }
    }
    q = prune_page_top(q, saving_vdiscards_par > 0);
    p = list_ptr(v);
    list_ptr(v) = null;
    flush_node(v);
    if (q == null) {
        /* the |eq_level| of the box stays the same */
        box(n) = null;
    } else {
        box(n) = filtered_vpackage(q, 0, additional, max_depth_par, split_keep_group, vdir, 0, 0);
    }
    if (m == exactly) {
        return filtered_vpackage(p, h, exactly, split_max_depth_par, split_off_group, vdir, 0, 0);
    } else {
        return filtered_vpackage(p, 0, additional, max_depth_par, split_off_group, vdir, 0, 0);
    }
}

@ Now that we can see what eventually happens to boxes, we can consider the first
steps in their creation. The |begin_box| routine is called when |box_context| is
a context specification, |cur_chr| specifies the type of box desired, and
|cur_cmd=make_box|.

@c
void begin_box(int box_context)
{
    halfword q; /* run through the current list */
    halfword k; /* 0 or |vmode| or |hmode| */
    int n;      /* a box number */
    int spec_direction = -1;
    int just_pack = 0;
    int split_mode = exactly ;
    switch (cur_chr) {
        case box_code:
            scan_register_num();
            cur_box = box(cur_val);
            /* the box becomes void, at the same level */
            box(cur_val) = null;
            break;
        case copy_code:
            scan_register_num();
            cur_box = copy_node_list(box(cur_val));
            break;
        case last_box_code:
            /*
                If the current list ends with a box node, delete it from
                the list and make |cur_box| point to it; otherwise set
                |cur_box:=null|.
            */
            cur_box = null;
            if (abs(cur_list.mode_field) == mmode) {
                you_cant();
                help1("Sorry; this \\lastbox will be void.");
                error();
            } else if ((cur_list.mode_field == vmode) && (cur_list.head_field == cur_list.tail_field)) {
                you_cant();
                help2("Sorry...I usually can't take things from the current page.",
                      "This \\lastbox will therefore be void.");
                error();
            } else {
                if (cur_list.head_field != cur_list.tail_field) {
                    /* todo: new code,  needs testing */

                    /* maybe: ((type(cur_list.tail_field) == hlist_node) < rule_node) */

                    if ((type(cur_list.tail_field) == hlist_node) || (type(cur_list.tail_field) == vlist_node)) {
                        /* Remove the last box ... */
                        q = alink(cur_list.tail_field);
                        if (q == null || vlink(q) != cur_list.tail_field) {
                            q = cur_list.head_field;
                            while (vlink(q) != cur_list.tail_field)
                                q = vlink(q);
                        }
                        uncouple_node(cur_list.tail_field);
                        cur_box = cur_list.tail_field;
                        shift_amount(cur_box) = 0;
                        cur_list.tail_field = q;
                        vlink(cur_list.tail_field) = null;
                    }
                }
            }
            break;
        case vsplit_code:
            /*
                Split off part of a vertical box, make |cur_box| point to it. Here we
                deal with things like `\.{\\vsplit 13 to 100pt}'.
            */
            scan_register_num();
            n = cur_val;
            if (scan_keyword("upto")) {
                split_mode = additional ;
            } else if (!scan_keyword("to")) {
                print_err("Missing `to' inserted");
                help2("I'm working on `\\vsplit<box number> to <dimen>';",
                      "will look for the <dimen> next.");
                error();
            }
            scan_normal_dimen();
            cur_box = vsplit(n, cur_val, split_mode);
         break;
        default:
            /*
                Initiate the construction of an hbox or vbox, then |return|. Here is
                where we enter restricted horizontal mode or internal vertical mode,
                in order to make a box.
            */
            switch (cur_chr) {
                case tpack_code:
                    cur_chr = vtop_code;
                    just_pack = 1;
                    break;
                case vpack_code:
                    cur_chr = vtop_code + vmode;
                    just_pack = 1;
                    break;
                case hpack_code:
                    cur_chr = vtop_code + hmode;
                    just_pack = 1;
                    break;
            }
            /* */
            k = cur_chr - vtop_code;
            set_saved_record(0, saved_boxcontext, 0, box_context);
            switch (abs(cur_list.mode_field)) {
                case vmode:
                    spec_direction = body_direction_par;
                    break;
                case hmode:
                    spec_direction = text_direction_par;
                    break;
                case mmode:
                    spec_direction = math_direction_par;
                    break;
            }
            if (k == hmode) {
                if ((box_context < box_flag) && (abs(cur_list.mode_field) == vmode))
                    scan_full_spec(adjusted_hbox_group, spec_direction,just_pack);
                else
                    scan_full_spec(hbox_group, spec_direction,just_pack);
            } else {
                if (k == vmode) {
                    scan_full_spec(vbox_group, spec_direction,just_pack);
                } else {
                    scan_full_spec(vtop_group, spec_direction,just_pack);
                    k = vmode;
                }
                normal_paragraph();
            }
            push_nest();
            cur_list.mode_field = -k;
            if (k == vmode) {
                prev_depth_par = ignore_depth;
                if (every_vbox_par != null)
                    begin_token_list(every_vbox_par, every_vbox_text);
            } else {
                space_factor_par = 1000;
                if (every_hbox_par != null)
                    begin_token_list(every_hbox_par, every_hbox_text);
            }
            return;
            break;
    }
    /* in simple cases, we use the box immediately */
    box_end(box_context);
}
