% pdfthread.w
%
% Copyright 2009-2012 Taco Hoekwater <taco@@luatex.org>
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

@ Threads are handled in similar way as link annotations
@c
void append_bead(PDF pdf, halfword p)
{
    int a, b, c, t;
    if (global_shipping_mode == SHIPPING_FORM)
        normal_error("pdf backend", "threads cannot be inside an xform");
    t = pdf_get_obj(pdf, obj_type_thread, pdf_thread_id(p), pdf_thread_named_id(p));
    b = pdf_create_obj(pdf, obj_type_others, 0);
    obj_bead_ptr(pdf, b) = pdf_get_mem(pdf, pdfmem_bead_size);
    set_obj_bead_page(pdf, b, pdf->last_page);
    set_obj_bead_data(pdf, b, p);
    if (pdf_thread_attr(p) != null)
        set_obj_bead_attr(pdf, b, tokens_to_string(pdf_thread_attr(p)));
    else
        set_obj_bead_attr(pdf, b, 0);
    if (obj_thread_first(pdf, t) == 0) {
        obj_thread_first(pdf, t) = b;
        set_obj_bead_next(pdf, b, b);
        set_obj_bead_prev(pdf, b, b);
    } else {
        a = obj_thread_first(pdf, t);
        c = obj_bead_prev(pdf, a);
        set_obj_bead_prev(pdf, b, c);
        set_obj_bead_next(pdf, b, a);
        set_obj_bead_prev(pdf, a, b);
        set_obj_bead_next(pdf, c, b);
    }
    addto_page_resources(pdf, obj_type_bead, b);
}

@ @c
void do_thread(PDF pdf, halfword p, halfword parent_box, scaledpos cur)
{
    scaled_whd alt_rule;
    if ((type(p) == hlist_node) && (subtype(p) == pdf_start_thread_node))
        normal_error("pdf backend", "'startthread' ended up in hlist");
    if (doing_leaders)
        return;
    if (subtype(p) == pdf_start_thread_node) {
        pdf->thread.wd = width(p);
        pdf->thread.ht = height(p);
        pdf->thread.dp = depth(p);
        pdf->last_thread_id = pdf_thread_id(p);
        pdf->last_thread_named_id = (pdf_thread_named_id(p) > 0);
        if (pdf->last_thread_named_id)
            add_token_ref(pdf_thread_id(p));
        pdf->thread_level = cur_s;
    }
    alt_rule.wd = width(p);
    alt_rule.ht = height(p);
    alt_rule.dp = depth(p);
    set_rect_dimens(pdf, p, parent_box, cur, alt_rule, pdf_thread_margin);
    append_bead(pdf, p);
    pdf->last_thread = p;
}

@ @c
void append_thread(PDF pdf, halfword parent_box, scaledpos cur)
{
    scaled_whd alt_rule;
    halfword p = new_node(whatsit_node, pdf_thread_data_node);
    width(p) = pdf->thread.wd;
    height(p) = pdf->thread.ht;
    depth(p) = pdf->thread.dp;
    pdf_thread_attr(p) = null;
    pdf_thread_id(p) = pdf->last_thread_id;
    if (pdf->last_thread_named_id) {
        add_token_ref(pdf_thread_id(p));
        pdf_thread_named_id(p) = 1;
    } else {
        pdf_thread_named_id(p) = 0;
    }
    alt_rule.wd = width(p);
    alt_rule.ht = height(p);
    alt_rule.dp = depth(p);
    set_rect_dimens(pdf, p, parent_box, cur, alt_rule, pdf_thread_margin);
    append_bead(pdf, p);
    pdf->last_thread = p;
}

@ @c
void end_thread(PDF pdf, halfword p)
{
    scaledpos pos = pdf->posstruct->pos;
    if (type(p) == hlist_node)
        normal_error("pdf backend", "'endthread' ended up in hlist");
    if (pdf->thread_level != cur_s)
        normal_error("pdf backend", "'endthread' ended up in different nesting level than 'startthread'");
    if (is_running(pdf->thread.dp) && (pdf->last_thread != null)) {
        switch (pdf->posstruct->dir) {
            case dir_TLT:
            case dir_TRT:
                pdf_ann_bottom(pdf->last_thread) = pos.v - pdf_thread_margin;
                break;
            case dir_LTL:
                pdf_ann_right(pdf->last_thread) = pos.h + pdf_thread_margin;
                break;
            case dir_RTT:
                pdf_ann_left(pdf->last_thread) = pos.h - pdf_thread_margin;
                break;
            default:
                formatted_warning("pdf backend","forcing bad dir %i to TLT in end tread",pdf->posstruct->dir);
        }
    }
    if (pdf->last_thread_named_id)
        delete_token_ref(pdf->last_thread_id);
    pdf->last_thread = null;
}

@ The following function are needed for outputing article thread.
@c
void thread_title(PDF pdf, int t)
{
    pdf_add_name(pdf, "Title");
    pdf_out(pdf, '(');
    if (obj_info(pdf, t) < 0)
        pdf_print(pdf, -obj_info(pdf, t));
    else
        pdf_print_int(pdf, obj_info(pdf, t));
    pdf_out(pdf, ')');
}

void pdf_fix_thread(PDF pdf, int t)
{
    halfword a;
    if (obj_info(pdf, t) < 0) {
        char *ss = makecstring(-obj_info(pdf, t));
        formatted_warning("pdf backend", "unknown thread destination name '%s'",ss);
    } else {
        formatted_warning("pdf backend", "unknown thread destination num '%d'",obj_info(pdf, t));
    }
    a = pdf_create_obj(pdf, obj_type_others, 0);
    pdf_begin_obj(pdf, a, OBJSTM_ALWAYS);
    pdf_begin_dict(pdf);
    pdf_dict_add_ref(pdf, "T", t);
    pdf_dict_add_ref(pdf, "V", a);
    pdf_dict_add_ref(pdf, "N", a);
    pdf_dict_add_ref(pdf, "P", pdf->last_page);
    pdf_add_name(pdf, "R");
    pdf_begin_array(pdf);
    pdf_add_int(pdf, 0);
    pdf_add_int(pdf, 0);
    pdf_add_bp(pdf, page_width_par);
    pdf_add_bp(pdf, page_height_par);
    pdf_end_array(pdf);
    pdf_end_dict(pdf);
    pdf_end_obj(pdf);
    pdf_begin_obj(pdf, t, OBJSTM_ALWAYS);
    pdf_begin_dict(pdf);
    pdf_add_name(pdf, "I");
    pdf_begin_dict(pdf);
    thread_title(pdf, t);
    pdf_end_dict(pdf);
    pdf_dict_add_ref(pdf, "F", a);
    pdf_end_dict(pdf);
    pdf_end_obj(pdf);
}

void out_thread(PDF pdf, int t)
{
    halfword a, b;
    int last_attr;
    if (obj_thread_first(pdf, t) == 0) {
        pdf_fix_thread(pdf, t);
        return;
    }
    pdf_begin_obj(pdf, t, OBJSTM_ALWAYS);
    pdf_begin_dict(pdf);
    a = obj_thread_first(pdf, t);
    b = a;
    last_attr = 0;
    do {
        if (obj_bead_attr(pdf, a) != 0)
            last_attr = obj_bead_attr(pdf, a);
        a = obj_bead_next(pdf, a);
    } while (a != b);
    if (last_attr != 0) {
        pdf_print_ln(pdf, last_attr);
    } else {
        pdf_add_name(pdf, "I");
        pdf_begin_dict(pdf);
        thread_title(pdf, t);
        pdf_end_dict(pdf);
    }
    pdf_dict_add_ref(pdf, "F", a);
    pdf_end_dict(pdf);
    pdf_end_obj(pdf);
    do {
        pdf_begin_obj(pdf, a, OBJSTM_ALWAYS);
        pdf_begin_dict(pdf);
        if (a == b)
            pdf_dict_add_ref(pdf, "T", t);
        pdf_dict_add_ref(pdf, "V", obj_bead_prev(pdf, a));
        pdf_dict_add_ref(pdf, "N", obj_bead_next(pdf, a));
        pdf_dict_add_ref(pdf, "P", obj_bead_page(pdf, a));
        pdf_dict_add_ref(pdf, "R", obj_bead_rect(pdf, a));
        pdf_end_dict(pdf);
        pdf_end_obj(pdf);
        a = obj_bead_next(pdf, a);
    } while (a != b);
}

@ @c
void scan_thread_id(void)
{
    if (scan_keyword("num")) {
        scan_int();
        if (cur_val <= 0)
            normal_error("pdf backend", "num identifier must be positive");
        if (cur_val > max_halfword)
            normal_error("pdf backend", "number too big");
        set_pdf_thread_id(cur_list.tail_field, cur_val);
        set_pdf_thread_named_id(cur_list.tail_field, 0);
    } else if (scan_keyword("name")) {
        scan_toks(false, true);
        set_pdf_thread_id(cur_list.tail_field, def_ref);
        set_pdf_thread_named_id(cur_list.tail_field, 1);
    } else {
        normal_error("pdf backend", "identifier type missing");
    }
}

void check_running_thread(PDF pdf, halfword this_box, scaledpos cur)
{
    if ((pdf->last_thread != null) && is_running(pdf->thread.dp)
        && (pdf->thread_level == cur_s))
        append_thread(pdf, this_box, cur);
}

@ @c
void print_bead_rectangles(PDF pdf)
{
    halfword i;
    pdf_object_list *k;
    int l;
    if ((k = get_page_resources_list(pdf, obj_type_bead)) != NULL) {
        while (k != NULL) {
            l = pdf_create_obj(pdf, obj_type_others, 0);
            pdf_begin_obj(pdf, l, OBJSTM_ALWAYS);
            pdf_begin_array(pdf);
            i = obj_bead_data(pdf, k->info); /* pointer to a whatsit or whatsit-like node */
            pdf_add_rect_spec(pdf, i);
            if (subtype(i) == pdf_thread_data_node)
                flush_node(i);
            pdf_end_array(pdf);
            pdf_end_obj(pdf);
            set_obj_bead_rect(pdf, k->info, l); /* rewrite |obj_bead_data| */
            k = k->link;
        }
    }
}
