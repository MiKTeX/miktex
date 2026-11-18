/*

Copyright 2009-2012 Taco Hoekwater <taco@@luatex.org>

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

    To implement nested link annotations, we need a stack to hold copy of
    |pdf_start_link_node|'s that are being written out, together with their box
    nesting level.

*/

void pdf_out_link_state(PDF pdf, halfword p)
{
    if (pdf_link_state(p) >= 0 && pdf_link_state(p) <= 1) {
        pdf->link_state = pdf_link_state(p);
    } else {
        /* ignores so one can use them for whatever purpose */
    }
}


void push_link_level(PDF pdf, halfword p)
{
    if (pdf->link_stack_ptr >= pdf_max_link_level)
        overflow("pdf link stack size", pdf_max_link_level);
    pdf->link_stack_ptr++;
    pdf->link_stack[pdf->link_stack_ptr].nesting_level = cur_s;
    pdf->link_stack[pdf->link_stack_ptr].link_node = copy_node_list(p);
    pdf->link_stack[pdf->link_stack_ptr].ref_link_node = p;
    pdf->link_stack[pdf->link_stack_ptr].direction = pdf->posstruct->dir;
    pdf->link_stack[pdf->link_stack_ptr].count =  0;
    pdf->link_stack[pdf->link_stack_ptr].measuring = pdf_linking == 2 && is_running(width(p));
    pdf->link_stack[pdf->link_stack_ptr].split = 0;
    pdf->link_stack[pdf->link_stack_ptr].untracked = 0;
    pdf->link_stack[pdf->link_stack_ptr].anchor = pdf->posstruct->pos.h;
    pdf->link_stack[pdf->link_stack_ptr].nseg = 0;
}

/*tex

    The rectangle of a link is taken from the positions of its start and end.
    When a direction node comes in between, these are in different runs, so
    the rectangle is wrong, and the linked text need not even be one stretch
    of the line (a link from the middle of a left-to-right run into the
    surrounding right-to-left text). With |\pdfvariable linking 2|, the part
    of an open link before each direction node is kept as a stretch (from
    its start or the previous direction node to the node), and each visually
    contiguous stretch becomes one annotation, as each line does for a link
    that breaks over lines. The \.{\\leftskip} and \.{\\rightskip} of a line
    are left out the same way (the old rectangle of a link that breaks over
    lines runs to the edges of the line box). Links without direction nodes
    or these skips, with a width of their own or in vertical text keep the
    old rectangle. Under a matrix, each stretch is transformed as the old
    rectangle is (its bounding box).

*/

# define link_extent_slack 100 /* touching stretches, allowing for rounding */

static void add_link_extent(pdf_link_stack_record *r, scaled a, scaled b)
{
    if (a == b)
        return;
    if (r->nseg == r->segmax) {
        r->segmax = r->segmax ? 2 * r->segmax : 8;
        r->seg = xrealloc(r->seg, (unsigned) (2 * r->segmax * sizeof(scaled)));
    }
    r->seg[2 * r->nseg] = a < b ? a : b;
    r->seg[2 * r->nseg + 1] = a < b ? b : a;
    r->nseg++;
}

/*tex
    Called by |hlist_out| at a direction node with the position before it
    (|after| false) and after it (|after| true).
*/

void link_dir_switch(PDF pdf, int after)
{
    int i;
    for (i = 1; i <= pdf->link_stack_ptr; i++) {
        pdf_link_stack_record *r = &pdf->link_stack[i];
        if (r->measuring && r->nesting_level == cur_s) {
            if (pdf->posstruct->dir != dir_TLT && pdf->posstruct->dir != dir_TRT)
                r->untracked = 1;
            if (after) {
                r->anchor = pdf->posstruct->pos.h;
            } else {
                add_link_extent(r, r->anchor, pdf->posstruct->pos.h);
                r->split = 1;
            }
        }
    }
}

/*tex
    Called by |hlist_out| after a \.{\\leftskip} or \.{\\rightskip} glue that
    started at page position |before|: the skip is not part of the link.
*/

void link_past_skip(PDF pdf, scaled before)
{
    int i;
    if (pdf->posstruct->pos.h == before)
        return;
    for (i = 1; i <= pdf->link_stack_ptr; i++) {
        pdf_link_stack_record *r = &pdf->link_stack[i];
        if (r->measuring && r->nesting_level == cur_s) {
            add_link_extent(r, r->anchor, before);
            r->anchor = pdf->posstruct->pos.h;
            r->split = 1;
        }
    }
}

/*tex
    Set the rectangles of the current piece of link |i|, which ends at the
    current position, from its stretches, one annotation per contiguous
    stretch: the first is |ref_link_node|, the others are new
    |pdf_link_data_node|'s like those of |append_link| (flushed when the page
    is written). Returns false when the old rectangle has to be used.
*/

static int finish_link_piece(PDF pdf, int i)
{
    pdf_link_stack_record *r = &pdf->link_stack[i];
    halfword q, a;
    int j, k, n;
    if (! r->measuring)
        return 0;
    r->measuring = 0;
    add_link_extent(r, r->anchor, pdf->posstruct->pos.h);
    if (! r->split || r->untracked || r->nseg == 0) {
        r->nseg = 0;
        return 0;
    }
    /*tex Sort the stretches and join those that touch. */
    for (j = 2; j < 2 * r->nseg; j += 2) {
        scaled lo = r->seg[j], hi = r->seg[j + 1];
        for (k = j - 2; k >= 0 && r->seg[k] > lo; k -= 2) {
            r->seg[k + 2] = r->seg[k];
            r->seg[k + 3] = r->seg[k + 1];
        }
        r->seg[k + 2] = lo;
        r->seg[k + 3] = hi;
    }
    n = 0;
    for (j = 2; j < 2 * r->nseg; j += 2) {
        if (r->seg[j] <= r->seg[2 * n + 1] + link_extent_slack) {
            if (r->seg[j + 1] > r->seg[2 * n + 1])
                r->seg[2 * n + 1] = r->seg[j + 1];
        } else {
            n++;
            r->seg[2 * n] = r->seg[j];
            r->seg[2 * n + 1] = r->seg[j + 1];
        }
    }
    n++;
    q = r->ref_link_node;
    for (j = 0; j < n; j++) {
        if (j == 0) {
            a = q;
        } else {
            a = copy_node(r->link_node);
            subtype(a) = pdf_link_data_node;
            pdf_ann_margin(a) = pdf_ann_margin(q);
        }
        /*tex Without a matrix this gives the stretch as it is. */
        matrixtransformrect(r->seg[2 * j], r->ll.v, r->seg[2 * j + 1], r->ur.v);
        pdf_ann_left(a) = getllx();
        pdf_ann_bottom(a) = getlly();
        pdf_ann_right(a) = geturx();
        pdf_ann_top(a) = getury();
        if (j > 0) {
            k = pdf_create_obj(pdf, obj_type_others, 0);
            obj_annot_ptr(pdf, k) = a;
            set_obj_scheduled(pdf, k);
            addto_page_resources(pdf, obj_type_link, k);
        }
    }
    r->nseg = 0;
    return 1;
}

/*tex At the end of a box: links that go on in a later box. */

void finish_link_pieces(PDF pdf)
{
    int i;
    for (i = 1; i <= pdf->link_stack_ptr; i++) {
        if (pdf->link_stack[i].nesting_level == cur_s)
            (void) finish_link_piece(pdf, i);
    }
}

void pop_link_level(PDF pdf)
{
    flush_node_list(pdf->link_stack[pdf->link_stack_ptr].link_node);
    pdf->link_stack_ptr--;
}

void do_link(PDF pdf, halfword p, halfword parent_box, scaledpos cur)
{
    scaled_whd alt_rule;
    int k;
    if (type(p) == vlist_node)
        normal_error("pdf backend", "'startlink' ended up in vlist");
    if (global_shipping_mode == SHIPPING_FORM)
        normal_error("pdf backend", "link annotations cannot be inside an xform");
    if (is_obj_scheduled(pdf, pdf_link_objnum(p)))
        pdf_link_objnum(p) = pdf_create_obj(pdf, obj_type_others, 0);
    push_link_level(pdf, p);
    alt_rule.wd = width(p);
    alt_rule.ht = height(p);
    alt_rule.dp = depth(p);
    pdf->link_stack[pdf->link_stack_ptr].count++;
    set_rect_dimens(pdf, p, parent_box, cur, alt_rule);
    pdf->link_stack[pdf->link_stack_ptr].ll = rect_dimens_ll;
    pdf->link_stack[pdf->link_stack_ptr].ur = rect_dimens_ur;
    pdf_ann_margin(p) = pdf_link_margin;
    /*tex The reference for the annot object must be set here. */
    obj_annot_ptr(pdf, pdf_link_objnum(p)) = p;
    k = pdf_link_objnum(p);
    set_obj_scheduled(pdf, pdf_link_objnum(p));
    addto_page_resources(pdf, obj_type_link, k);
}

void end_link(PDF pdf, halfword p)
{
    halfword q;
    scaledpos pos = pdf->posstruct->pos;
    if (type(p) == vlist_node)
        normal_error("pdf backend","'endlink' ended up in vlist");
    if (pdf->link_stack_ptr < 1)
        normal_error("pdf backend","pdf link_stack empty, 'endlink' used without 'startlink'");
    if (pdf->link_stack[pdf->link_stack_ptr].nesting_level != cur_s)
        normal_error("pdf backend","'endlink' ended up in different nesting level than 'startlink'");
    if (finish_link_piece(pdf, pdf->link_stack_ptr)) {
        pop_link_level(pdf);
        return;
    }
    /*tex
        The test for running link must be done on |link_node| and not
        |ref_link_node|, as |ref_link_node| can be set by |do_link| or
        |append_link| already.
    */
    if (is_running(width(pdf->link_stack[pdf->link_stack_ptr].link_node))) {
        q = pdf->link_stack[pdf->link_stack_ptr].ref_link_node;
        if (global_shipping_mode == SHIPPING_PAGE && matrixused()) {
            /*tex
                |matrixrecalculate| uses the rectangle transformed last,
                which need not be this link's (another link or annotation
                may come in between); under 2 the link keeps its own.
            */
            if (pdf_linking == 2) {
                pdf_link_stack_record *r = &pdf->link_stack[pdf->link_stack_ptr];
                matrixtransformrect(r->ll.h, r->ll.v, pos.h, r->ur.v);
            } else {
                matrixrecalculate(pos.h);
            }
            pdf_ann_left(q) = getllx();
            pdf_ann_top(q) = getlly();
            pdf_ann_right(q) = geturx();
            pdf_ann_bottom(q) = getury();
        } else {
            switch (pdf->posstruct->dir) {
                case dir_TLT:
                    pdf_ann_right(q) = pos.h;
                    break;
                case dir_TRT:
                    /*tex 
                       In version 1.13.0 the test for swapping was moved
                       to the moment we write the rectangle, but it did not
                       consider this case.
                    */ 
                    if (pdf_linking == 1) {
                        /* begin of experiment, can be simplified */
                        if (pdf->link_stack[pdf->link_stack_ptr].count > 1) { 
                            pdf_ann_right(q) = pdf_ann_right(q) - pdf_ann_left(q) + pos.h;
                            pdf_ann_left(q) = pos.h;
                            if (pdf_ann_left(q) > pdf_ann_right(q)) {
                                halfword r = pdf_ann_right(q);
                                /* halfword l = pdf_ann_left(q); */
                                pdf_ann_right(q) = pos.h - r;
                                pdf_ann_left(q) = pos.h;
                            }
                            break;
                        }
                        /* end of experiment */
                    }
                    if (pdf_ann_left(q)<pdf_ann_right(q)) {
                        pdf_ann_left(q) = pos.h;
                    } else {
                        pdf_ann_right(q) = pos.h;
                    } 
                    break;
                case dir_LTL:
                case dir_RTT:
                    pdf_ann_bottom(q) = pos.v;
                    break;
                default:
                    pdf_ann_right(q) = pos.h;
                    formatted_warning("pdf backend","forcing bad dir %i to TLT in link",pdf->posstruct->dir);
                    break;
            }
        }
    }
    pop_link_level(pdf);
}

/*tex

    For ``running'' annotations we must append a new node when the end of
    annotation is in other box than its start. The new created node is identical
    to corresponding whatsit node representing the start of annotation, but its
    |info| field is |max_halfword|. We set |info| field just before destroying
    the node, in order to use |flush_node_list| to do the job.

*/

void append_link(PDF pdf, halfword parent_box, scaledpos cur, small_number i)
{
    halfword p;
    int k;
    scaled_whd alt_rule;
    p = copy_node(pdf->link_stack[(int) i].link_node);
    pdf->link_stack[(int) i].ref_link_node = p;
    pdf->link_stack[(int) i].count++;
    pdf->link_stack[(int) i].measuring = pdf_linking == 2 && is_running(width(p));
    pdf->link_stack[(int) i].split = 0;
    pdf->link_stack[(int) i].untracked = 0;
    pdf->link_stack[(int) i].anchor = pdf->posstruct->pos.h;
    pdf->link_stack[(int) i].nseg = 0;
    /*tex This node is not a normal link node. */
    subtype(p) = pdf_link_data_node;
    alt_rule.wd = width(p);
    alt_rule.ht = height(p);
    alt_rule.dp = depth(p);
    set_rect_dimens(pdf, p, parent_box, cur, alt_rule);
    pdf->link_stack[(int) i].ll = rect_dimens_ll;
    pdf->link_stack[(int) i].ur = rect_dimens_ur;
    k = pdf_create_obj(pdf, obj_type_others, 0);
    obj_annot_ptr(pdf, k) = p;
    set_obj_scheduled(pdf, pdf_link_objnum(p));
    addto_page_resources(pdf, obj_type_link, k);
}

void scan_startlink(PDF pdf)
{
    int k;
    halfword r;
    if (abs(cur_list.mode_field) == vmode)
        normal_error("pdf backend", "startlink cannot be used in vertical mode");
    k = pdf_create_obj(pdf, obj_type_others, 0);
    new_annot_whatsit(pdf_start_link_node);
    set_pdf_link_attr(cur_list.tail_field, null);
    if (scan_keyword("attr")) {
        scan_toks(false, true);
        set_pdf_link_attr(cur_list.tail_field, def_ref);
    }
    r = scan_action(pdf);
    set_pdf_link_action(cur_list.tail_field, r);
    set_pdf_link_objnum(cur_list.tail_field, k);
    pdf_last_link = k;
    /*tex
        Although it is possible to set |obj_annot_ptr(k) := tail| here, it is not
        safe if nodes are later copied/destroyed/moved; a better place to do this
        is inside |do_link|, when the whatsit node is written out.
    */
}
