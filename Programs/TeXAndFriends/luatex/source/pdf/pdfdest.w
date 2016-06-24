% pdfdest.w
%
% Copyright 2009-2011 Taco Hoekwater <taco@@luatex.org>
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

@ Here we implement subroutines for work with objects and related things. Some of
them are used in former parts too, so we need to declare them forward.

@c
void init_dest_names(PDF pdf)
{
    pdf->dest_names_size = inf_dest_names_size;
    pdf->dest_names = xmallocarray(dest_name_entry, inf_dest_names_size); /* will grow dynamically */
}

@ @c
void append_dest_name(PDF pdf, char *s, int n)
{
    int a;
    if (pdf->dest_names_ptr == sup_dest_names_size)
        overflow("number of destination names (dest_names_size)",(unsigned) pdf->dest_names_size);
    if (pdf->dest_names_ptr == pdf->dest_names_size) {
        a = pdf->dest_names_size / 5;
        if (pdf->dest_names_size < sup_dest_names_size - a)
            pdf->dest_names_size = pdf->dest_names_size + a;
        else
            pdf->dest_names_size = sup_dest_names_size;
        pdf->dest_names = xreallocarray(pdf->dest_names, dest_name_entry, (unsigned) pdf->dest_names_size);
    }
    pdf->dest_names[pdf->dest_names_ptr].objname = xstrdup(s);
    pdf->dest_names[pdf->dest_names_ptr].objnum = n;
    pdf->dest_names_ptr++;
}

@ When a destination is created we need to check whether another destination
with the same identifier already exists and give a warning if needed.

@c
static void warn_dest_dup(int id, small_number byname)
{
    if (byname > 0) {
        char *ss = tokenlist_to_cstring(id, true, NULL);
        formatted_warning("pdf backend", "ignoring duplicate destination with the name '%s'",ss);
    } else {
        formatted_warning("pdf backend", "ignoring duplicate destination with the num '%d'",id);
    }
    /* no longer the annoying context */
}

@ @c
void do_dest(PDF pdf, halfword p, halfword parent_box, scaledpos cur)
{
    scaledpos pos = pdf->posstruct->pos;
    scaled_whd alt_rule;
    int k;
    if (global_shipping_mode == SHIPPING_FORM)
        normal_error("pdf backend", "destinations cannot be inside an xform");
    if (doing_leaders)
        return;
    k = pdf_get_obj(pdf, obj_type_dest, pdf_dest_id(p), pdf_dest_named_id(p));
    if (obj_dest_ptr(pdf, k) != null) {
        warn_dest_dup(pdf_dest_id(p), (small_number) pdf_dest_named_id(p));
        return;
    }
    obj_dest_ptr(pdf, k) = p;
    addto_page_resources(pdf, obj_type_dest, k);
    alt_rule.wd = width(p);
    alt_rule.ht = height(p);
    alt_rule.dp = depth(p);
    /* the different branches for matrixused is somewhat strange and should always be used  */
    switch (pdf_dest_type(p)) {
    case pdf_dest_xyz:
        if (matrixused())
            set_rect_dimens(pdf, p, parent_box, cur, alt_rule, pdf_dest_margin);
        else {
            pdf_ann_left(p) = pos.h;
            pdf_ann_top(p) = pos.v;
        }
        break;
    case pdf_dest_fith:
    case pdf_dest_fitbh:
        if (matrixused())
            set_rect_dimens(pdf, p, parent_box, cur, alt_rule, pdf_dest_margin);
        else
            pdf_ann_top(p) = pos.v;
        break;
    case pdf_dest_fitv:
    case pdf_dest_fitbv:
        if (matrixused())
            set_rect_dimens(pdf, p, parent_box, cur, alt_rule, pdf_dest_margin);
        else
            pdf_ann_left(p) = pos.h;
        break;
    case pdf_dest_fit:
    case pdf_dest_fitb:
        break;
    case pdf_dest_fitr:
        set_rect_dimens(pdf, p, parent_box, cur, alt_rule, pdf_dest_margin);
    }
}

@ @c
void write_out_pdf_mark_destinations(PDF pdf)
{
    pdf_object_list *k;
    if ((k = get_page_resources_list(pdf, obj_type_dest)) != NULL) {
        while (k != NULL) {
            if (is_obj_written(pdf, k->info)) {
                normal_error("pdf backend","destination has been already written (this shouldn't happen)");
            } else {
                int i;
                i = obj_dest_ptr(pdf, k->info);
                pdf_begin_obj(pdf, k->info, OBJSTM_ALWAYS);
                if (pdf_dest_named_id(i) > 0) {
                    pdf_begin_dict(pdf);
                    pdf_add_name(pdf, "D");
                }
                pdf_begin_array(pdf);
                pdf_add_ref(pdf, pdf->last_page);
                switch (pdf_dest_type(i)) {
                    case pdf_dest_xyz:
                        pdf_add_name(pdf, "XYZ");
                        pdf_add_bp(pdf, pdf_ann_left(i));
                        pdf_add_bp(pdf, pdf_ann_top(i));
                        if (pdf_dest_xyz_zoom(i) == null) {
                            pdf_add_null(pdf);
                        } else {
                            if (pdf->cave == 1)
                                pdf_out(pdf, ' ');
                            pdf_print_int(pdf, pdf_dest_xyz_zoom(i) / 1000);
                            pdf_out(pdf, '.');
                            pdf_print_int(pdf, (pdf_dest_xyz_zoom(i) % 1000));
                            pdf->cave = 1;
                        }
                        break;
                    case pdf_dest_fit:
                        pdf_add_name(pdf, "Fit");
                        break;
                    case pdf_dest_fith:
                        pdf_add_name(pdf, "FitH");
                        pdf_add_bp(pdf, pdf_ann_top(i));
                        break;
                    case pdf_dest_fitv:
                        pdf_add_name(pdf, "FitV");
                        pdf_add_bp(pdf, pdf_ann_left(i));
                        break;
                    case pdf_dest_fitb:
                        pdf_add_name(pdf, "FitB");
                        break;
                    case pdf_dest_fitbh:
                        pdf_add_name(pdf, "FitBH");
                        pdf_add_bp(pdf, pdf_ann_top(i));
                        break;
                    case pdf_dest_fitbv:
                        pdf_add_name(pdf, "FitBV");
                        pdf_add_bp(pdf, pdf_ann_left(i));
                        break;
                    case pdf_dest_fitr:
                        pdf_add_name(pdf, "FitR");
                        pdf_add_rect_spec(pdf, i);
                        break;
                    default:
                        normal_error("pdf backend", "unknown dest type");
                        break;
                }
                pdf_end_array(pdf);
                if (pdf_dest_named_id(i) > 0)
                    pdf_end_dict(pdf);
                pdf_end_obj(pdf);
            }
            k = k->link;
        }
    }
}

@ @c
void scan_pdfdest(PDF pdf)
{
    halfword q;
    int k;
    str_number i;
    scaled_whd alt_rule;
    q = cur_list.tail_field;
    new_whatsit(pdf_dest_node);
    if (scan_keyword("num")) {
        scan_int();
        if (cur_val <= 0)
            normal_error("pdf backend", "num identifier must be positive");
        if (cur_val > max_halfword)
            normal_error("pdf backend", "number too big");
        set_pdf_dest_id(cur_list.tail_field, cur_val);
        set_pdf_dest_named_id(cur_list.tail_field, 0);
    } else if (scan_keyword("name")) {
        scan_toks(false, true);
        set_pdf_dest_id(cur_list.tail_field, def_ref);
        set_pdf_dest_named_id(cur_list.tail_field, 1);
    } else {
        normal_error("pdf backend", "identifier type missing");
    }
    if (scan_keyword("xyz")) {
        set_pdf_dest_type(cur_list.tail_field, pdf_dest_xyz);
        if (scan_keyword("zoom")) {
            scan_int();
            if (cur_val > max_halfword)
                normal_error("pdf backend", "number too big");
            set_pdf_dest_xyz_zoom(cur_list.tail_field, cur_val);
        } else {
            set_pdf_dest_xyz_zoom(cur_list.tail_field, null);
        }
    } else if (scan_keyword("fitbh")) {
        set_pdf_dest_type(cur_list.tail_field, pdf_dest_fitbh);
    } else if (scan_keyword("fitbv")) {
        set_pdf_dest_type(cur_list.tail_field, pdf_dest_fitbv);
    } else if (scan_keyword("fitb")) {
        set_pdf_dest_type(cur_list.tail_field, pdf_dest_fitb);
    } else if (scan_keyword("fith")) {
        set_pdf_dest_type(cur_list.tail_field, pdf_dest_fith);
    } else if (scan_keyword("fitv")) {
        set_pdf_dest_type(cur_list.tail_field, pdf_dest_fitv);
    } else if (scan_keyword("fitr")) {
        set_pdf_dest_type(cur_list.tail_field, pdf_dest_fitr);
    } else if (scan_keyword("fit")) {
        set_pdf_dest_type(cur_list.tail_field, pdf_dest_fit);
    } else {
        normal_error("pdf backend", "destination type missing");
    }
    /* Scan an optional space */
    get_x_token();
    if (cur_cmd != spacer_cmd)
        back_input();

    if (pdf_dest_type(cur_list.tail_field) == pdf_dest_fitr) {
        alt_rule = scan_alt_rule();     /* scans |<rule spec>| to |alt_rule| */
        set_width(cur_list.tail_field, alt_rule.wd);
        set_height(cur_list.tail_field, alt_rule.ht);
        set_depth(cur_list.tail_field, alt_rule.dp);
    }
    if (pdf_dest_named_id(cur_list.tail_field) != 0) {
        i = tokens_to_string(pdf_dest_id(cur_list.tail_field));
        k = find_obj(pdf, obj_type_dest, i, true);
        flush_str(i);
    } else {
        k = find_obj(pdf, obj_type_dest, pdf_dest_id(cur_list.tail_field), false);
    }
    if ((k != 0) && (obj_dest_ptr(pdf, k) != null)) {
        warn_dest_dup(pdf_dest_id(cur_list.tail_field),(small_number) pdf_dest_named_id(cur_list.tail_field));
        flush_node_list(cur_list.tail_field);
        cur_list.tail_field = q;
        vlink(q) = null;
    }
}

@ sorts |dest_names| by names
@c
static int dest_cmp(const void *a, const void *b)
{
    dest_name_entry aa = *(const dest_name_entry *) a;
    dest_name_entry bb = *(const dest_name_entry *) b;
    return strcmp(aa.objname, bb.objname);
}

@ @c
void sort_dest_names(PDF pdf)
{
    qsort(pdf->dest_names, (size_t) pdf->dest_names_ptr, sizeof(dest_name_entry), dest_cmp);
}

@ Output the name tree. The tree nature of the destination list forces the
storing of intermediate data in |obj_info| and |obj_aux| fields, which
is further uglified by the fact that |obj_tab| entries do not accept char
pointers.

@c
int output_name_tree(PDF pdf)
{
    boolean is_names = true;    /* flag for name tree output: is it Names or Kids? */
    int k = 0;                  /* index of current child of |l|; if |k < pdf_dest_names_ptr|
                                   then this is pointer to |dest_names| array;
                                   otherwise it is the pointer to |obj_tab| (object number) */
    int b = 0;
    int m, j, l;
    int dests = 0;
    int names_head = 0;
    int names_tail = 0;
    if (pdf->dest_names_ptr == 0) {
        goto DONE;
    }
    sort_dest_names(pdf);
    while (true) {
        do {
            l = pdf_create_obj(pdf, obj_type_others, 0);        /* create a new node */
            if (b == 0)
                b = l;          /* first in this level */
            if (names_head == 0) {
                names_head = l;
                names_tail = l;
            } else {
                set_obj_link(pdf, names_tail, l);
                names_tail = l;
            }
            set_obj_link(pdf, names_tail, 0);
            /* Output the current node in this level */
            pdf_begin_obj(pdf, l, OBJSTM_ALWAYS);
            pdf_begin_dict(pdf);
            j = 0;
            if (is_names) {
                set_obj_start(pdf, l, pdf->dest_names[k].objname);
                pdf_add_name(pdf, "Names");
                pdf_begin_array(pdf);
                do {
                    pdf_add_string(pdf, pdf->dest_names[k].objname);
                    pdf_add_ref(pdf, pdf->dest_names[k].objnum);
                    j++;
                    k++;
                } while (j != name_tree_kids_max && k != pdf->dest_names_ptr);
                pdf_end_array(pdf);
                set_obj_stop(pdf, l, pdf->dest_names[k - 1].objname);   /* for later */
                if (k == pdf->dest_names_ptr) {
                    is_names = false;
                    k = names_head;
                    b = 0;
                }

            } else {
                set_obj_start(pdf, l, obj_start(pdf, k));
                pdf_add_name(pdf, "Kids");
                pdf_begin_array(pdf);
                do {
                    pdf_add_ref(pdf, k);
                    set_obj_stop(pdf, l, obj_stop(pdf, k));
                    k = obj_link(pdf, k);
                    j++;
                } while (j != name_tree_kids_max && k != b
                         && obj_link(pdf, k) != 0);
                pdf_end_array(pdf);
                if (k == b)
                    b = 0;
            }
            pdf_add_name(pdf, "Limits");
            pdf_begin_array(pdf);
            pdf_add_string(pdf, obj_start(pdf, l));
            pdf_add_string(pdf, obj_stop(pdf, l));
            pdf_end_array(pdf);
            pdf_end_dict(pdf);
            pdf_end_obj(pdf);
        } while (b != 0);

        if (k == l) {
            dests = l;
            goto DONE;
        }
    }
  DONE:
    if ((dests != 0) || (pdf_names_toks != null)) {
        m = pdf_create_obj(pdf, obj_type_others, 0);
        pdf_begin_obj(pdf, m, OBJSTM_ALWAYS);
        pdf_begin_dict(pdf);
        if (dests != 0)
            pdf_dict_add_ref(pdf, "Dests", dests);
        if (pdf_names_toks != null) {
            pdf_print_toks(pdf, pdf_names_toks);
            delete_token_ref(pdf_names_toks);
            pdf_names_toks = null;
        }
        print_pdf_table_string(pdf, "names");
        pdf_end_dict(pdf);
        pdf_end_obj(pdf);
        return m;
    } else {
        return 0;
    }
}
