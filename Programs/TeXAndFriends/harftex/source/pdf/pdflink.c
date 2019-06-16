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

void push_link_level(PDF pdf, halfword p)
{
    if (pdf->link_stack_ptr >= pdf_max_link_level)
        overflow("pdf link stack size", pdf_max_link_level);
    pdf->link_stack_ptr++;
    pdf->link_stack[pdf->link_stack_ptr].nesting_level = cur_s;
    pdf->link_stack[pdf->link_stack_ptr].link_node = copy_node_list(p);
    pdf->link_stack[pdf->link_stack_ptr].ref_link_node = p;
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
    set_rect_dimens(pdf, p, parent_box, cur, alt_rule, pdf_link_margin);
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
    /*tex
        The test for running link must be done on |link_node| and not
        |ref_link_node|, as |ref_link_node| can be set by |do_link| or
        |append_link| already.
    */
    if (is_running(width(pdf->link_stack[pdf->link_stack_ptr].link_node))) {
        q = pdf->link_stack[pdf->link_stack_ptr].ref_link_node;
        if (global_shipping_mode == SHIPPING_PAGE && matrixused()) {
            matrixrecalculate(pos.h + pdf_link_margin);
            pdf_ann_left(q) = getllx() - pdf_link_margin;
            pdf_ann_top(q) = getlly() - pdf_link_margin;
            pdf_ann_right(q) = geturx() + pdf_link_margin;
            pdf_ann_bottom(q) = getury() + pdf_link_margin;
        } else {
            switch (pdf->posstruct->dir) {
                case dir_TLT:
                    pdf_ann_right(q) = pos.h + pdf_link_margin;
                    break;
                case dir_TRT:
                    pdf_ann_left(q) = pos.h - pdf_link_margin;
                    break;
                case dir_LTL:
                case dir_RTT:
                    pdf_ann_bottom(q) = pos.v - pdf_link_margin;
                    break;
                default:
                    pdf_ann_right(q) = pos.h + pdf_link_margin;
                    formatted_warning("pdf backend","forcing bad dir %i to TLT in link",pdf->posstruct->dir);
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
    /*tex This node is not a normal link node. */
    subtype(p) = pdf_link_data_node;
    alt_rule.wd = width(p);
    alt_rule.ht = height(p);
    alt_rule.dp = depth(p);
    set_rect_dimens(pdf, p, parent_box, cur, alt_rule, pdf_link_margin);
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
