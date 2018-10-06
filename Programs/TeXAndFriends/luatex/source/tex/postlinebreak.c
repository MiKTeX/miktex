/*

postlinebreak.w

Copyright 2006-2010 Taco Hoekwater <taco@@luatex.org>

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

#include "ptexlib.h"

/*tex

So far we have gotten a little way into the |line_break| routine, having covered
its important |try_break| subroutine. Now let's consider the rest of the process.

The main loop of |line_break| traverses the given hlist, starting at
|vlink(temp_head)|, and calls |try_break| at each legal breakpoint. A variable
called |auto_breaking| is set to true except within math formulas, since glue
nodes are not legal breakpoints when they appear in formulas.

The current node of interest in the hlist is pointed to by |cur_p|. Another
variable, |prev_p|, is usually one step behind |cur_p|, but the real meaning of
|prev_p| is this: If |type(cur_p)=glue_node| then |cur_p| is a legal breakpoint
if and only if |auto_breaking| is true and |prev_p| does not point to a glue
node, penalty node, explicit kern node, or math node.

The total number of lines that will be set by |post_line_break| is
|best_line-prev_graf-1|. The last breakpoint is specified by
|break_node(best_bet)|, and this passive node points to the other breakpoints via
the |prev_break| links. The finishing-up phase starts by linking the relevant
passive nodes in forward order, changing |prev_break| to |next_break|. (The
|next_break| fields actually reside in the same memory space as the |prev_break|
fields did, but we give them a new name because of their new significance.) Then
the lines are justified, one by one.

The |post_line_break| must also keep an dir stack, so that it can output end
direction instructions at the ends of lines and begin direction instructions at
the beginnings of lines.

*/

/*tex The new name for |prev_break| after links are reversed: */

#define next_break prev_break

/*tex The |int|s are actually halfwords. */

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
                         scaled first_indent, halfword best_line)
{

    boolean have_directional = true;
    /*tex temporary registers for list manipulation */
    halfword q, r;
    halfword k;
    scaled w;
     /*tex was a break at glue? */
    boolean glue_break;
    /*tex was the current break at a discretionary node? */
    boolean disc_break;
    /*tex and did it have a nonempty post-break part? */
    boolean post_disc_break;
    /*tex width of line number |cur_line| */
    scaled cur_width;
    /*tex left margin of line number |cur_line| */
    scaled cur_indent;
    /*tex use when calculating penalties between lines */
    int pen;
    /*tex |cur_p|, but localized */
    halfword cur_p;
    /*tex the current line number being justified */
    halfword cur_line;
    /*tex the current direction: */
    dir_ptr = cur_list.dirs_field;
    /*tex
        Reverse the links of the relevant passive nodes, setting |cur_p| to the
        first breakpoint. The job of reversing links in a list is conveniently
        regarded as the job of taking items off one stack and putting them on
        another. In this case we take them off a stack pointed to by |q| and
        having |prev_break| fields; we put them on a stack pointed to by |cur_p|
        and having |next_break| fields. Node |r| is the passive node being moved
        from stack to stack.
    */
    q = break_node(best_bet);
    /*tex |cur_p| will become the first breakpoint; */
    cur_p = null;
    do {
        r = q;
        q = prev_break(q);
        next_break(r) = cur_p;
        cur_p = r;
    } while (q != null);
    /*tex prevgraf + 1 */
    cur_line = cur_list.pg_field + 1;
    do {
        /*tex
            Justify the line ending at breakpoint |cur_p|, and append it to the
            current vertical list, together with associated penalties and other
            insertions.

            The current line to be justified appears in a horizontal list
            starting at |vlink(temp_head)| and ending at |cur_break(cur_p)|. If
            |cur_break(cur_p)| is a glue node, we reset the glue to equal the
            |right_skip| glue; otherwise we append the |right_skip| glue at the
            right. If |cur_break(cur_p)| is a discretionary node, we modify the
            list so that the discretionary break is compulsory, and we set
            |disc_break| to |true|. We also append the |left_skip| glue at the
            left of the line, unless it is zero.
        */
        if (dir_ptr != null) {
            /*tex Insert dir nodes at the beginning of the current line. */
            for (q = dir_ptr; q != null; q = vlink(q)) {
                halfword tmp = new_dir(dir_dir(q));
                halfword nxt = vlink(temp_head);
                delete_attribute_ref(node_attr(tmp));
                node_attr(tmp) = node_attr(temp_head);
                add_node_attr_ref(node_attr(tmp));
                couple_nodes(temp_head, tmp);
                /*tex \.{\\break}\.{\\par} */
                try_couple_nodes(tmp, nxt);
            }
            flush_node_list(dir_ptr);
            dir_ptr = null;
        }
        /*tex
            Modify the end of the line to reflect the nature of the break and to
            include \.{\\rightskip}; also set the proper value of |disc_break|.
            At the end of the following code, |q| will point to the final node on
            the list about to be justified. In the meanwhile |r| will point to
            the node we will use to insert end-of-line stuff after. |q==null|
            means we use the final position of |r|.
        */

        if (temp_head != null) {
            /*tex begin mathskip code */
            q = temp_head;
            while(q != null) {
                if (type(q) == math_node) {
                    surround(q) = 0 ;
                    reset_glue_to_zero(q);
                    break;
                } else if ((type(q) == hlist_node) && (subtype(q) == indent_list)) {
                    /* go on */
                } else if (is_char_node(q)) {
                    break;
                } else if (non_discardable(q)) {
                    break;
                } else if (type(q) == kern_node && subtype(q) != explicit_kern && subtype(q) != italic_kern) {
                    break;
                }
                q = vlink(q);
            }
            /*tex end mathskip code */
        }

        r = cur_break(cur_p);
        q = null;
        disc_break = false;
        post_disc_break = false;
        glue_break = false;


        if (r == null) {
            for (r = temp_head; vlink(r) != null; r = vlink(r));
            if (r == final_par_glue) {
                /*tex This should almost always be true... */
                q = r;
                /*tex |q| refers to the last node of the line (and paragraph) */
                r = alink(r);
            }
            /*tex |r| refers to the node after which the dir nodes should be closed */
        } else if (type(r) == math_node) {
            surround(r) = 0;
            /*tex begin mathskip code */
            reset_glue_to_zero(r);
            /*tex end mathskip code */
        } else if (type(r) == glue_node) {
            copy_glue_values(r,right_skip);
            subtype(r) = right_skip_code + 1;
            glue_break = true;
            /*tex |q| refers to the last node of the line */
            q = r;
            r = alink(r);
            assert(vlink(r) == q);
            /*tex |r| refers to the node after which the dir nodes should be closed */
        } else if (type(r) == disc_node) {
            halfword a = alink(r);
            halfword v = vlink(r);
            assert(a != null);
            assert(v != null);
            switch (subtype(r)) {
            case select_disc:
                if (vlink_pre_break(r) != null) {
                    flush_node_list(vlink_pre_break(r));
                    vlink_pre_break(r) = null;
                    tlink_pre_break(r) = null;
                }
                if (vlink_no_break(r) != null) {
                    couple_nodes(a, vlink_no_break(r));
                    couple_nodes(tlink_no_break(r), r);
                    vlink_no_break(r) = null;
                    tlink_no_break(r) = null;
                }
                assert(type(a) == disc_node && subtype(a) == init_disc);
                flush_node_list(vlink_no_break(a));
                vlink_no_break(a) = null;
                tlink_no_break(a) = null;
                flush_node_list(vlink_pre_break(a));
                vlink_pre_break(a) = null;
                tlink_pre_break(a) = null;
                flush_node_list(vlink_post_break(a));
                vlink_post_break(a) = null;
                tlink_post_break(a) = null;
                break;
            case init_disc:
                assert(type(v) == disc_node && subtype(v) == select_disc);
                subtype(v) = syllable_disc;
                flush_node_list(vlink_no_break(v));
                vlink_no_break(v) = vlink_post_break(r);
                tlink_no_break(v) = tlink_post_break(r);
                vlink_post_break(r) = null;
                tlink_post_break(r) = null;
            default:
                if (vlink_no_break(r) != null) {
                    flush_node_list(vlink_no_break(r));
                    vlink_no_break(r) = null;
                    tlink_no_break(r) = null;
                }
                if (vlink_pre_break(r) != null) {
                    couple_nodes(a, vlink_pre_break(r));
                    couple_nodes(tlink_pre_break(r), r);
                    vlink_pre_break(r) = null;
                    tlink_pre_break(r) = null;
                }
            }
            if (vlink_post_break(r) != null) {
                couple_nodes(r, vlink_post_break(r));
                couple_nodes(tlink_post_break(r), v);
                vlink_post_break(r) = null;
                tlink_post_break(r) = null;
                post_disc_break = true;
            }
            disc_break = true;
        } else if (type(r) == kern_node) {
            width(r) = 0;
        }
        /*tex Adjust the dir stack based on dir nodes in this line. */
        if (have_directional) {
            halfword e, p;
            for (e = vlink(temp_head); e != null && e != cur_break(cur_p); e = vlink(e)) {
                if (type(e) == dir_node) {
                    if (subtype(e) == normal_dir) {
                        dir_ptr = do_push_dir_node(dir_ptr, e);
                    } else if (dir_ptr != null && dir_dir(dir_ptr) == dir_dir(e)) {
                        dir_ptr = do_pop_dir_node(dir_ptr);
                    }
                }
            }
            assert(e == cur_break(cur_p));
            /*tex Insert dir nodes at the end of the current line. */
            e = vlink(r);
            for (p = dir_ptr; p != null; p = vlink(p)) {
                halfword s = new_dir(dir_dir(p));
                subtype(s) = cancel_dir;
                delete_attribute_ref(node_attr(s));
                node_attr(s) = node_attr(r);
                add_node_attr_ref(node_attr(s));
                couple_nodes(r, s);
                try_couple_nodes(s, e);
                r = s;
            }
        }
        if (passive_right_box(cur_p) != null) {
            halfword s = copy_node_list(passive_right_box(cur_p));
            halfword e = vlink(r);
            couple_nodes(r, s);
            try_couple_nodes(s, e);
            r = s;
        }
        if (q == null) {
            q = r;
        }
        /*tex
            Now [q] refers to the last node on the line and therefore the
            rightmost breakpoint. The only exception is the case of a
            discretionary break with non-empty |pre_break|, then |q| has been
            changed to the last node of the |pre_break| list. If the par ends
            with a \break command, the last line is utterly empty. That is the
            case of |q==temp_head|.
        */
        if (q != temp_head && protrude_chars > 0) {
            halfword p, ptmp;
            if (disc_break && (is_char_node(q) || (type(q) != disc_node))) {
                /*tex |q| is reset to the last node of |pre_break| */
                p = q;
                ptmp = p;
            } else {
                /*tex get |vlink(p) = q| */
                p = alink(q);
                assert(vlink(p) == q);
                ptmp = p;
            }
            p = find_protchar_right(vlink(temp_head), p);
            w = char_pw(p, right_side);
            if (w != 0) {
                /*tex we have found a marginal kern, append it after |ptmp| */
                k = new_margin_kern(-w, last_rightmost_char, right_side);
                delete_attribute_ref(node_attr(k));
                node_attr(k) = node_attr(p);
                add_node_attr_ref(node_attr(k));
                try_couple_nodes(k, vlink(ptmp));
                couple_nodes(ptmp,k);
                if (ptmp == q)
                    q = vlink(q);
            }
        }
        /*tex
            If |q| was not a breakpoint at glue and has been reset to |rightskip|
            then we append |rightskip| after |q| now.
        */
        if (!glue_break) {
            /*tex Put the \.{\\rightskip} glue after node |q|. */
            halfword r1 = new_glue((right_skip == null ? zero_glue : right_skip));
            subtype(r1) = right_skip_code+1;
            try_couple_nodes(r1,vlink(q));
            delete_attribute_ref(node_attr(r1));
            node_attr(r1) = node_attr(q);
            add_node_attr_ref(node_attr(r1));
            couple_nodes(q,r1);
            q = r1;
        }
        /*tex
            Modify the end of the line to reflect the nature of the break and to
            include \.{\\rightskip}; also set the proper value of |disc_break|;
            Also put the \.{\\leftskip} glue at the left and detach this line.

            The following code begins with |q| at the end of the list to be
            justified. It ends with |q| at the beginning of that list, and with
            |vlink(temp_head)| pointing to the remainder of the paragraph, if
            any.
        */
        r = vlink(q);
        vlink(q) = null;

        q = vlink(temp_head);
        try_couple_nodes(temp_head, r);
        if (passive_left_box(cur_p) != null && passive_left_box(cur_p) != 0) {
            halfword s;
            r = copy_node_list(passive_left_box(cur_p));
            s = vlink(q);
            couple_nodes(r,q);
            q = r;
            if ((cur_line == cur_list.pg_field + 1) && (s != null)) {
                if (type(s) == hlist_node) {
                    if (list_ptr(s) == null) {
                        q = vlink(q);
                        try_couple_nodes(r,vlink(s));
                        try_couple_nodes(s, r);
                    }
                }
            }
        }
        /*tex
            At this point |q| is the leftmost node; all discardable nodes have
            been discarded
        */
        if (protrude_chars > 0) {
            halfword p;
            p = q;
            p = find_protchar_left(p, false);
            w = char_pw(p, left_side);
            if (w != 0) {
                k = new_margin_kern(-w, last_leftmost_char, left_side);
                delete_attribute_ref(node_attr(k));
                node_attr(k) = node_attr(q);
                add_node_attr_ref(node_attr(k));
                couple_nodes(k,q);
                q = k;
            }
        }
        if (! glue_is_zero(left_skip)) {
            r = new_glue(left_skip);
            subtype(r) = left_skip_code+1;
            delete_attribute_ref(node_attr(r));
            node_attr(r) = node_attr(q);
            add_node_attr_ref(node_attr(r));
            couple_nodes(r,q);
            q = r;
        }
        /*tex
            Put the \.{\\leftskip} glue at the left and detach this line. Call
            the packaging subroutine, setting |just_box| to the justified box.
            Now |q| points to the hlist that represents the current line of the
            paragraph. We need to compute the appropriate line width, pack the
            line into a box of this size, and shift the box by the appropriate
            amount of indentation.
        */
        if (cur_line > last_special_line) {
            cur_width = second_width;
            cur_indent = second_indent;
        } else if (par_shape_ptr == null) {
            cur_width = first_width;
            cur_indent = first_indent;
        } else {
            cur_indent = varmem[(par_shape_ptr + 2 * cur_line)].cint;
            cur_width = varmem[(par_shape_ptr + 2 * cur_line + 1)].cint;
        }
        adjust_tail = adjust_head;
        pre_adjust_tail = pre_adjust_head;
        if (adjust_spacing > 0) {
            just_box = hpack(q, cur_width, cal_expand_ratio, paragraph_dir);
        } else {
            just_box = hpack(q, cur_width, exactly, paragraph_dir);
        }
        shift_amount(just_box) = cur_indent;
        subtype(just_box) = line_list;
        /*tex
            Call the packaging subroutine, setting |just_box| to the justified
            box.
        */
        if ((vlink(contrib_head) != null))
            checked_break_filter(pre_box);
        if (pre_adjust_head != pre_adjust_tail) {
            append_list(pre_adjust_head, pre_adjust_tail);
            checked_break_filter(pre_adjust);
        }
        pre_adjust_tail = null;
        append_to_vlist(just_box,lua_key_index(post_linebreak));
        checked_break_filter(box);
        if (adjust_head != adjust_tail) {
            append_list(adjust_head, adjust_tail);
            checked_break_filter(adjust);
        }
        adjust_tail = null;
        /*tex
            Append the new box to the current vertical list, followed by the list
            of special nodes taken out of the box by the packager. Append a
            penalty node, if a nonzero penalty is appropriate. Penalties between
            the lines of a paragraph come from club and widow lines, from the
            |inter_line_penalty| parameter, and from lines that end at
            discretionary breaks. Breaking between lines of a two-line paragraph
            gets both club-line and widow-line penalties. The local variable
            |pen| will be set to the sum of all relevant penalties for the
            current line, except that the final line is never penalized.
        */
        if (cur_line + 1 != best_line) {
            q = inter_line_penalties_ptr;
            if (q != null) {
                r = cur_line;
                if (r > penalty(q))
                    r = penalty(q);
                pen = penalty(q + r);
            } else {
                if (passive_pen_inter(cur_p) != 0) {
                    pen = passive_pen_inter(cur_p);
                } else {
                    pen = inter_line_penalty;
                }
            }
            q = club_penalties_ptr;
            if (q != null) {
                /*tex prevgraf */
                r = cur_line - cur_list.pg_field;
                if (r > penalty(q))
                    r = penalty(q);
                pen += penalty(q + r);
            } else if (cur_line == cur_list.pg_field + 1) {
                /*tex prevgraf */
                pen += club_penalty;
            }
            q = widow_penalties_ptr;
            if (q != null) {
                r = best_line - cur_line - 1;
                if (r > penalty(q))
                    r = penalty(q);
                pen += penalty(q + r);
            } else if (cur_line + 2 == best_line) {
                pen += widow_penalty;
            }
            if (disc_break) {
                if (passive_pen_broken(cur_p) != 0) {
                    pen += passive_pen_broken(cur_p);
                } else {
                    pen += broken_penalty;
                }
            }
            if (pen != 0) {
                r = new_penalty(pen,linebreak_penalty);
                couple_nodes(cur_list.tail_field, r);
                cur_list.tail_field = r;
            }
        }
        /*tex
            Append a penalty node, if a nonzero penalty is appropriate. Justify
            the line ending at breakpoint |cur_p|, and append it to the current
            vertical list, together with associated penalties and other
            insertions.
        */
        incr(cur_line);
        cur_p = next_break(cur_p);
        if (cur_p != null && !post_disc_break) {
            /*tex
                Prune unwanted nodes at the beginning of the next line. Glue and
                penalty and kern and math nodes are deleted at the beginning of a
                line, except in the anomalous case that the node to be deleted is
                actually one of the chosen breakpoints. Otherwise the pruning
                done here is designed to match the lookahead computation in
                |try_break|, where the |break_width| values are computed for
                non-discretionary breakpoints.
            */
            r = temp_head;
            /*tex
                Normally we have a matching math open and math close node but
                when we cross a line the open node is removed, including any glue
                or penalties following it. This is however not that nice for
                callbacks that rely on symmetry. Of course this only counts for
                one liners, as we can still have only a begin or end node on a
                line. The end_of_math lua helper is made robust against this
                although there you should be aware of the fact that one can end
                up in the middle of math in callbacks that don't work on whole
                paragraphs, but at least this branch makes sure that some proper
                analysis is possible. (todo: check if math glyphs have the
                subtype marked done).
            */
            while (1) {
                q = vlink(r);
                /*
                if (q == cur_break(cur_p) || is_char_node(q))
                    break;
                if (!((type(q) == local_par_node))) {
                    if (non_discardable(q) || (type(q) == kern_node && subtype(q) != explicit_kern && subtype(q) != italic_kern))
                        break;
                }
                */
                if (type(q) == math_node) {
                    /*tex begin mathskip code */
                    surround(q) = 0 ;
                    reset_glue_to_zero(q);
                    /*tex end mathskip code */
                }
                if (q == cur_break(cur_p)) {
                    break;
                } else if (is_char_node(q)) {
                    break;
                } else if (type(q) == local_par_node) {
                    /*tex weird, in the middle somewhere */
                } else if (non_discardable(q)) {
                    break;
                } else if (type(q) == kern_node && subtype(q) != explicit_kern && subtype(q) != italic_kern) {
                    break;
                }
                r = q;
            }
            if (r != temp_head) {
                vlink(r) = null;
                flush_node_list(vlink(temp_head));
                try_couple_nodes(temp_head, q);
            }
        }
    } while (cur_p != null);
    if ((cur_line != best_line) || (vlink(temp_head) != null))
        confusion("line breaking");
    /*tex prevgraf */
    cur_list.pg_field = best_line - 1;
    /*tex |dir_save| */
    cur_list.dirs_field = dir_ptr;
    dir_ptr = null;
}
