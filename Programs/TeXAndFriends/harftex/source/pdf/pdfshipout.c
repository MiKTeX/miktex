/*

Copyright 2010-2012 Taco Hoekwater <taco@luatex.org>

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

scaledpos shipbox_refpos;

/*tex

    |ship_out| is used to shipout a box to PDF or DVI mode. If |shipping_mode| is
    set to |SHIPPING_FORM| then the output will be a |Form| object (only PDF), and
    if it is set to |SHIPPING_PAGE| it will be a |Page| object.

*/

void ship_out(PDF pdf, halfword p, shipping_mode_e shipping_mode)
{
    /*tex Indices to first ten count registers: */
    int j, k;
    int post_callback_id;
    int pre_callback_id;
    /*tex The origin position on the page: */
    posstructure refpoint;
    int rule_callback_id = 0;
    scaledpos cur = { 0, 0 };
    refpoint.pos.h = 0;
    refpoint.pos.v = 0;
    ensure_output_state(pdf, ST_HEADER_WRITTEN);
    /*tex This is only for complaining if \.{\\outputmode} has changed: */
    fix_o_mode();
    pdf->f_cur = null_font;
    /*tex
        Start sheet {\sl Sync\TeX} information record. We assume that
        |pdf_output| is properly set up.
    */
    if (synctex_par) {
        if (output_mode_used == OMODE_DVI) {
            synctexsheet(mag_par);
        } else {
            synctexsheet(1000);
        }
    }
    pre_callback_id = callback_defined(start_page_number_callback);
    post_callback_id = callback_defined(stop_page_number_callback);
    if ((tracing_output_par > 0) && (pre_callback_id == 0)) {
        tprint_nl("");
        print_ln();
        tprint("Completed box being shipped out");
    }
    global_shipping_mode = shipping_mode;
    if (shipping_mode == SHIPPING_PAGE) {
        if (pre_callback_id > 0)
            (void) run_callback(pre_callback_id, "->");
        else if (pre_callback_id == 0) {
            if (term_offset > max_print_line - 9)
                print_ln();
            else if ((term_offset > 0) || (file_offset > 0))
                print_char(' ');
            print_char('[');
            j = 9;
            while ((count(j) == 0) && (j > 0))
                j--;
            for (k = 0; k <= j; k++) {
                print_int(count(k));
                if (k < j)
                    print_char('.');
            }
        }
    }
    if ((tracing_output_par > 0) && shipping_mode == SHIPPING_PAGE) {
        if (pre_callback_id == 0) {
            print_char(']');
        }
        update_terminal();
        begin_diagnostic();
        show_box(p);
        end_diagnostic(true);
    }
    /*tex Ship out box |p|:*/
    if (shipping_mode == SHIPPING_PAGE && box_dir(p) != page_direction_par)
        normal_warning("backend","pagedir differs from bodydir, the output may be placed wrongly on the page");
    /*tex
        Update the values of |max_h| and |max_v|; but if the page is too large,
        |goto done|. Sometimes the user will generate a huge page because other
        error messages are being ignored. Such pages are not output to the
        \.{dvi} file, since they may confuse the printing software.
    */
    if ((height(p) > max_dimen) || (depth(p) > max_dimen) || (height(p) + depth(p) + v_offset_par > max_dimen) || (width(p) + h_offset_par > max_dimen)) {
        const char *hlp[] = {
            "The page just created is more than 18 feet tall or",
            "more than 18 feet wide, so I suspect something went wrong.",
            NULL
        };
        tex_error("Huge page cannot be shipped out", hlp);
        if (tracing_output_par <= 0) {
            begin_diagnostic();
            tprint_nl("The following box has been deleted:");
            show_box(p);
            end_diagnostic(true);
        }
        goto DONE;
    }
    if (height(p) + depth(p) + v_offset_par > max_v)
        max_v = height(p) + depth(p) + v_offset_par;
    if (width(p) + h_offset_par > max_h)
        max_h = width(p) + h_offset_par;
    /*tex Calculate page dimensions and margins. */
    if (global_shipping_mode == SHIPPING_PAGE) {
        if (page_width_par > 0)
            pdf->page_size.h = page_width_par;
        else {
            switch (page_direction_par) {
                case dir_TLT:
                    pdf->page_size.h = width(p) + 2 * page_left_offset_par;
                    break;
                case dir_TRT:
                    pdf->page_size.h = width(p) + 2 * page_right_offset_par;
                    break;
                case dir_LTL:
                    pdf->page_size.h = height(p) + depth(p) + 2 * page_left_offset_par;
                    break;
                case dir_RTT:
                    pdf->page_size.h = height(p) + depth(p) + 2 * page_right_offset_par;
                    break;
                default:
                    pdf->page_size.h = width(p) + 2 * page_left_offset_par;
                    normal_warning("pdf backend","bad page direction, assuming TLT, case 1");
            }
        }
        if (page_height_par > 0)
            pdf->page_size.v = page_height_par;
        else {
            switch (page_direction_par) {
                case dir_TLT:
                case dir_TRT:
                    pdf->page_size.v = height(p) + depth(p) + 2 * page_top_offset_par;
                    break;
                case dir_LTL:
                case dir_RTT:
                    pdf->page_size.v = width(p) + 2 * page_top_offset_par;
                    break;
                default:
                    pdf->page_size.v = height(p) + depth(p) + 2 * page_top_offset_par;
                    normal_warning("pdf backend","bad page direction, assuming TLT, case 2");
                }
        }
        /*tex
            Think in upright page/paper coordinates (page origin = lower left edge).
            First preset |refpoint.pos| to the DVI origin (near upper left page edge).
        */
        backend_out_control[backend_control_set_reference_point](pdf,&refpoint);
        /*tex
            Then shift |refpoint.pos| of the DVI origin depending on the
            |page_direction| within the upright (TLT) page coordinate system.
        */
        switch (page_direction_par) {
            case dir_TLT:
            case dir_LTL:
                refpoint.pos.h += h_offset_par;
                refpoint.pos.v -= v_offset_par;
                break;
            case dir_TRT:
            case dir_RTT:
                refpoint.pos.h += pdf->page_size.h - page_right_offset_par - one_true_inch;
                refpoint.pos.v -= v_offset_par;
                break;
            default:
                refpoint.pos.h += h_offset_par;
                refpoint.pos.v -= v_offset_par;
                normal_warning("pdf backend","bad page direction, assuming TLT, case 3");
        }
        /*tex
            Then switch to page box coordinate system; do |height(p)| movement,
            to get the location of the box origin.
        */
        pdf->posstruct->dir = page_direction_par;
        cur.h = 0;
        cur.v = height(p);
        synch_pos_with_cur(pdf->posstruct, &refpoint, cur);
    } else {
        /*tex We're shipping out a |/Form|. */
        pdf->posstruct->dir = box_dir(p);
        switch (pdf->posstruct->dir) {
            case dir_TLT:
            case dir_TRT:
                pdf->page_size.h = width(p);
                pdf->page_size.v = height(p) + depth(p);
                break;
            case dir_LTL:
            case dir_RTT:
                pdf->page_size.h = height(p) + depth(p);
                pdf->page_size.v = width(p);
                break;
            default:
                pdf->page_size.h = width(p);
                pdf->page_size.v = height(p) + depth(p);
                normal_warning("pdf backend","bad page direction, assuming TLT, case 4");
            }
        switch (pdf->posstruct->dir) {
            case dir_TLT:
                pdf->posstruct->pos.h = 0;
                pdf->posstruct->pos.v = depth(p);
                break;
            case dir_TRT:
                pdf->posstruct->pos.h = width(p);
                pdf->posstruct->pos.v = depth(p);
                break;
            case dir_LTL:
                pdf->posstruct->pos.h = height(p);
                pdf->posstruct->pos.v = width(p);
                break;
            case dir_RTT:
                pdf->posstruct->pos.h = depth(p);
                pdf->posstruct->pos.v = width(p);
                break;
            default:
                pdf->posstruct->pos.h = 0;
                pdf->posstruct->pos.v = depth(p);
                normal_warning("pdf backend","bad page direction, assuming TLT, case 5");
        }
    }
    /*tex
        Now we are at the point on the page where the origin of the page box
        should go. First we register the poisition for \.{\\gleaders}.
    */
    shipbox_refpos = pdf->posstruct->pos;
    backend_out_control[backend_control_begin_page](pdf);
    rule_callback_id = callback_defined(process_rule_callback);
    switch (type(p)) {
        case vlist_node:
            vlist_out(pdf, p, rule_callback_id);
            break;
        case hlist_node:
            hlist_out(pdf, p, rule_callback_id);
            break;
        default:
            normal_error("pdf backend", "no vlist or hlist in (xform) shipout");
    }
    if (shipping_mode == SHIPPING_PAGE)
        total_pages++;
    cur_s = -1;
    /*tex Finish shipping */
    backend_out_control[backend_control_end_page](pdf);
  DONE:
    if ((tracing_output_par <= 0) && (post_callback_id == 0) && shipping_mode == SHIPPING_PAGE) {
        print_char(']');
        update_terminal();
    }
    dead_cycles = 0;
    /*tex Flush the box from memory, showing statistics if requested. */
    if ((tracing_stats_par > 1) && (pre_callback_id == 0)) {
        tprint_nl("Memory usage before: ");
        print_int(var_used);
        print_char('&');
        print_int(dyn_used);
        print_char(';');
    }
    flush_node_list(p);
    if ((tracing_stats_par > 1) && (post_callback_id == 0)) {
        tprint(" after: ");
        print_int(var_used);
        print_char('&');
        print_int(dyn_used);
        print_ln();
    }
    if (shipping_mode == SHIPPING_PAGE && (post_callback_id > 0))
        (void) run_callback(post_callback_id, "->");
    /*tex Finish sheet {\sl Sync\TeX} information record. */
    if (synctex_par)
        synctexteehs();
    global_shipping_mode = NOT_SHIPPING;
}
