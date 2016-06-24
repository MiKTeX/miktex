% pdflistout.w
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

\def\pdfTeX{pdf\TeX}

@ @c
#include "ptexlib.h"

@ @c
#define kern_width(q) width(q) + ex_kern(q)

/*
    ext_xn_over_d(width(q), 1000000+ex_kern(q), 1000000);
*/

@ @c
pos_info_structure pos_info; /* to be accessed from Lua */

static backend_struct *backend = NULL;
backend_function *backend_out, *backend_out_whatsit;

@ @c
static void missing_backend_function(PDF pdf, halfword p)
{
    const char *n = get_node_name(type(p), subtype(p));
    if (type(p) == whatsit_node)
        formatted_error("pdf backend","no output function for whatsit %s",n);
    else
        formatted_error("pdf backend","no output function for node %s",n);
}

@ @c
static void init_none_backend_functions(void)
{
    backend_struct *p = &backend[OMODE_NONE];
    p->name = strdup("(None)");
}

@ @c
static void init_pdf_backend_functions(void)
{
    backend_struct *p = &backend[OMODE_PDF];
    p->name = strdup("PDF");
    p->node_fu[rule_node] = &pdf_place_rule;
    p->node_fu[glyph_node] = &pdf_place_glyph;
    p->whatsit_fu[special_node] = &pdf_special;
    p->whatsit_fu[pdf_literal_node] = &pdf_out_literal;
    p->whatsit_fu[pdf_refobj_node] = &pdf_ref_obj;
    p->whatsit_fu[pdf_annot_node] = &do_annot;
    p->whatsit_fu[pdf_start_link_node] = &do_link;
    p->whatsit_fu[pdf_end_link_node] = &end_link;
    p->whatsit_fu[pdf_dest_node] = &do_dest;
    p->whatsit_fu[pdf_thread_node] = &do_thread;
    p->whatsit_fu[pdf_end_thread_node] = &end_thread;
    p->whatsit_fu[late_lua_node] = &late_lua;
    p->whatsit_fu[pdf_colorstack_node] = &pdf_out_colorstack;
    p->whatsit_fu[pdf_setmatrix_node] = &pdf_out_setmatrix;
    p->whatsit_fu[pdf_save_node] = &pdf_out_save;
    p->whatsit_fu[pdf_restore_node] = &pdf_out_restore;
}

@ @c
static void init_dvi_backend_functions(void)
{
    backend_struct *p = &backend[OMODE_DVI];
    p->name = strdup("DVI");
    p->node_fu[rule_node] = &dvi_place_rule;
    p->node_fu[glyph_node] = &dvi_place_glyph;
    p->whatsit_fu[special_node] = &dvi_special;
    p->whatsit_fu[late_lua_node] = &late_lua;
}

@ @c
void init_backend_functionpointers(output_mode o_mode)
{
    int i, j;
    if (backend == NULL) {
        backend = xmalloc((MAX_OMODE + 1) * sizeof(backend_struct));
        for (i = 0; i <= MAX_OMODE; i++) {
            backend[i].node_fu = xmalloc((MAX_NODE_TYPE + 1) * sizeof(backend_function));
            backend[i].whatsit_fu = xmalloc((MAX_WHATSIT_TYPE + 1) * sizeof(backend_function));
            for (j = 0; j < MAX_NODE_TYPE + 1; j++)
                backend[i].node_fu[j] = &missing_backend_function;
            for (j = 0; j < MAX_WHATSIT_TYPE + 1; j++)
                backend[i].whatsit_fu[j] = &missing_backend_function;
        }
        init_none_backend_functions();
        init_dvi_backend_functions();
        init_pdf_backend_functions();
    }
    backend_out = backend[o_mode].node_fu;
    backend_out_whatsit = backend[o_mode].whatsit_fu;
}

@ This code scans forward to the ending |dir_node| while keeping
track of the needed width in |w|. When it finds the node that will end
this segment, it stores the accumulated with in the |dir_dvi_h| field
of that end node, so that when that node is found later in the
processing, the correct glue correction can be applied (obsolete).

@c
static scaled simple_advance_width(halfword p)
{
    halfword q = p;
    scaled w = 0;
    while ((q != null) && (vlink(q) != null)) {
        q = vlink(q);
        switch (type(q)) {
            case glyph_node:
                w += glyph_width(q);
/* experimental */
w += x_advance(q);
                break;
            case hlist_node:
            case vlist_node:
            case rule_node:
            case margin_kern_node:
                w += width(q);
                break;
            case kern_node:
                /* officially we should check the subtype */
                w += kern_width(q);
                break;
            case disc_node:
                /* hh: the frontend should append already */
                if (vlink(no_break(q)) != null)
                    w += simple_advance_width(no_break(q));
            default:
                break;
        }
    }
    return w;
}

@ @c
static halfword calculate_width_to_enddir(halfword p, real cur_glue, scaled cur_g, halfword this_box)
{
    int dir_nest = 1;
    halfword q = p, enddir_ptr = p;
    scaled w = 0;
    real glue_temp; /* glue value before rounding */
    int g_sign = glue_sign(this_box);
    int g_order = glue_order(this_box);
    while ((q != null) && (vlink(q) != null)) {
        q = vlink(q);
        if (is_char_node(q))
            w += pack_width(box_dir(this_box), dir_TRT, q, true);
        else {
            switch (type(q)) {
                case hlist_node:
                case vlist_node:
                    w += pack_width(box_dir(this_box), box_dir(q), q, false);
                    break;
                case rule_node:
                case margin_kern_node:
                    w += width(q);
                    break;
                case kern_node:
                    /* officially we should check the subtype */
                    w += kern_width(q);
                    break;
                case math_node:
                    /* begin mathskip code */
                    if (glue_is_zero(q)) {
                        w += surround(q);
                        break;
                    } else {
                        /* fall through: mathskip */
                    }
                    /* end mathskip code */
                case glue_node:
                    w += width(q) - cur_g;
                    if (g_sign != normal) {
                        if (g_sign == stretching) {
                            if (stretch_order(q) == g_order) {
                                cur_glue = cur_glue + stretch(q);
                                vet_glue(float_cast(glue_set(this_box)) * cur_glue);
                                cur_g = float_round(glue_temp);
                            }
                        } else if (shrink_order(q) == g_order) {
                            cur_glue = cur_glue - shrink(q);
                            vet_glue(float_cast(glue_set(this_box)) * cur_glue);
                            cur_g = float_round(glue_temp);
                        }
                    }
                    w += cur_g;
                    break;
                case disc_node:
                    /* hh: the frontend should append already */
                    if (vlink(no_break(q)) != null)
                        w += simple_advance_width(no_break(q));
                    break;
                case dir_node:
                    if (dir_dir(q) >= 0)
                        dir_nest++;
                    else
                        dir_nest--;
                    if (dir_nest == 0) {
                        enddir_ptr = q;
                        dir_cur_h(enddir_ptr) = w;
                        q = null;
                    }
                    break;
                default:
                    break;
            }
        }
    }
    if (enddir_ptr == p)
        /* no enddir found, just transport w by begindir */
        dir_cur_h(enddir_ptr) = w;
    return enddir_ptr;
}

@ The |out_what| procedure takes care of outputting the whatsit nodes for
|vlist_out| and |hlist_out|, which are similar for either case.

We don't implement \.{\\write} inside of leaders. (The reason is that
the number of times a leader box appears might be different in different
implementations, due to machine-dependent rounding in the glue calculations.)
@^leaders@>

@c
void out_what(PDF pdf, halfword p)
{
    switch (subtype(p)) {
        case special_node:         /* |pdf_special(pdf, p)|; */
        case late_lua_node:        /* |late_lua(pdf, p)|; */
        case pdf_save_node:        /* |pdf_out_save(pdf, p)|; */
        case pdf_restore_node:     /* |pdf_out_restore(pdf, p)|; */
        case pdf_end_link_node:    /* |end_link(pdf, p)|; */
        case pdf_end_thread_node:  /* |end_thread(pdf, p)|; */
        case pdf_literal_node:     /* |pdf_out_literal(pdf, p)|; */
        case pdf_colorstack_node:  /* |pdf_out_colorstack(pdf, p)|; */
        case pdf_setmatrix_node:   /* |pdf_out_setmatrix(pdf, p)|; */
        case pdf_refobj_node:      /* |pdf_ref_obj(pdf, p)| */
            backend_out_whatsit[subtype(p)] (pdf, p);
            break;
        case open_node:
        case write_node:
        case close_node:
            /* do some work that has been queued up for \.{\\write} */
            if (!doing_leaders) {
                int j = write_stream(p);
                if (subtype(p) == write_node) {
                    write_out(p);
                } else if (subtype(p) == close_node) {
                    close_write_file(j);
                } else if (valid_write_file(j)) {
                    char *fn;
                    close_write_file(j);
                    cur_name = open_name(p);
                    cur_area = open_area(p);
                    cur_ext = open_ext(p);
                    if (cur_ext == get_nullstr())
                        cur_ext = maketexstring(".tex");
                    fn = pack_file_name(cur_name, cur_area, cur_ext);
                    while (! open_write_file(j,fn)) {
                        fn = prompt_file_name("output file name", ".tex");
                    }
                }
            }
            break;
        case user_defined_node:
            break;
        default:
            /* this should give an error about missing whatsit backend function */
            backend_out_whatsit[subtype(p)] (pdf, p);
    }
}

@ @c
void hlist_out(PDF pdf, halfword this_box, int rule_callback_id)
{
    posstructure localpos;      /* the position structure local within this function */
    posstructure *refpos;       /* the list origin pos. on the page, provided by the caller */
    scaledpos cur = { 0, 0 }, tmpcur, basepoint;
    scaledpos size = { 0, 0 };  /* rule dimensions */
    scaled effective_horizontal;
    scaled save_h;              /* what |cur.h| should pop to */
    scaled edge;                /* right edge of sub-box or leader space */
    halfword enddir_ptr;        /* temporary pointer to enddir node */
    int g_order;                /* applicable order of infinity for glue */
    int g_sign;                 /* selects type of glue */
    halfword p, q;              /* current position in the hlist */
    halfword leader_box;        /* the leader box being replicated */
    scaled leader_wd;           /* width of leader box being replicated */
    scaled lx;                  /* extra space between leader boxes */
    boolean outer_doing_leaders;        /* were we doing leaders? */
    real glue_temp;             /* glue value before rounding */
    real cur_glue = 0.0;        /* glue seen so far */
    scaled cur_g = 0;           /* rounded equivalent of |cur_glue| times the glue ratio */
    scaled_whd rule, ci;
    int i;                      /* index to scan |pdf_link_stack| */
    int save_loc = 0;           /* DVI! \.{DVI} byte location upon entry */
    scaledpos save_dvi = { 0, 0 };      /* DVI! what |dvi| should pop to */
    int synctex = int_par(synctex_code) ;

    g_order = glue_order(this_box);
    g_sign = glue_sign(this_box);
    p = list_ptr(this_box);

    refpos = pdf->posstruct;
    pdf->posstruct = &localpos; /* use local structure for recursion */
    localpos.pos = refpos->pos;
    localpos.dir = box_dir(this_box);

    cur_s++;
    if (cur_s > max_push)
        max_push = cur_s;

    if (output_mode_used == OMODE_DVI) {
        if (cur_s > 0) {
            dvi_push();
            save_dvi = dvi;
        }
        save_loc = dvi_offset + dvi_ptr;
    }

    for (i = 1; i <= pdf->link_stack_ptr; i++) {
        if (pdf->link_stack[i].nesting_level == cur_s)
            append_link(pdf, this_box, cur, (small_number) i);
    }

    if (synctex)
        synctexhlist(this_box);
    while (p != null) {
        if (is_char_node(p)) {
            do {
                if (x_displace(p) != 0 || y_displace(p) != 0) {
                    tmpcur.h = cur.h + x_displace(p);
                    tmpcur.v = cur.v - y_displace(p);
                    synch_pos_with_cur(pdf->posstruct, refpos, tmpcur);
                }
                ci = output_one_char(pdf, p);
                if (textdir_parallel(localpos.dir, dir_TLT)) {
                    cur.h += ci.wd;
/* experimental */
cur.h += x_advance(p);
                } else {
                    cur.h += ci.ht + ci.dp;
                }
                synch_pos_with_cur(pdf->posstruct, refpos, cur);
                p = vlink(p);
            } while (is_char_node(p));
            if (synctex)
                synctexcurrent();
        } else {
            /* output the non-|char_node| |p| for |hlist_out| and move to the next node */
            switch (type(p)) {
                case hlist_node:
                case vlist_node:
                    if (textdir_parallel(box_dir(p), localpos.dir)) {
                        effective_horizontal = width(p);
                        basepoint.v = 0;
                        if (textdir_opposite(box_dir(p), localpos.dir))
                            basepoint.h = width(p);
                        else
                            basepoint.h = 0;
                    } else {
                        effective_horizontal = height(p) + depth(p);
                        if (!is_mirrored(box_dir(p))) {
                            if (partextdir_eq(box_dir(p), localpos.dir))
                                basepoint.h = height(p);
                            else
                                basepoint.h = depth(p);
                        } else {
                            if (partextdir_eq(box_dir(p), localpos.dir))
                                basepoint.h = depth(p);
                            else
                                basepoint.h = height(p);
                        }
                        if (is_rotated(localpos.dir)) {
                            if (partextdir_eq(localpos.dir, box_dir(p)))
                                basepoint.v = -width(p) / 2; /* up */
                            else
                                basepoint.v = width(p) / 2;  /* down */
                        } else if (is_mirrored(localpos.dir)) {
                            if (partextdir_eq(localpos.dir, box_dir(p)))
                                basepoint.v = 0;
                            else
                                basepoint.v = width(p);      /* down */
                        } else {
                            if (partextdir_eq(localpos.dir, box_dir(p)))
                                basepoint.v = -width(p);     /* up */
                            else
                                basepoint.v = 0;
                        }
                    }
                    if (!is_mirrored(localpos.dir))
                        basepoint.v = basepoint.v + shift_amount(p); /* shift the box down */
                    else
                        basepoint.v = basepoint.v - shift_amount(p); /* shift the box up */
                    if (list_ptr(p) == null) {
                        if (synctex) {
                            if (type(p) == vlist_node)
                                synctexvoidvlist(p, this_box);
                            else
                                synctexvoidhlist(p, this_box);
                        }
                    } else {
                        tmpcur.h = cur.h + basepoint.h;
                        tmpcur.v = basepoint.v;
                        synch_pos_with_cur(pdf->posstruct, refpos, tmpcur);
                        if (type(p) == vlist_node)
                            vlist_out(pdf, p, rule_callback_id);
                        else
                            hlist_out(pdf, p, rule_callback_id);
                    }
                    cur.h += effective_horizontal;
                    break;
                case disc_node:
                    /* hh: the frontend should append already */
                    if (vlink(no_break(p)) != null) {
                        if (subtype(p) != select_disc) {
                            q = tail_of_list(vlink(no_break(p))); /* this could be a tlink */
                            vlink(q) = vlink(p);
                            q = vlink(no_break(p));
                            vlink(no_break(p)) = null;
                            vlink(p) = q;
                        }
                    }
                    break;
                case glue_node:
                    {
                        /* move right or output leaders, we use real multiplication */
                        rule.wd = width(p) - cur_g;
                        if (g_sign != normal) {
                            if (g_sign == stretching) {
                                if (stretch_order(p) == g_order) {
                                    cur_glue = cur_glue + stretch(p);
                                    vet_glue(float_cast(glue_set(this_box)) * cur_glue);
                                    cur_g = float_round(glue_temp);
                                }
                            } else if (shrink_order(p) == g_order) {
                                cur_glue = cur_glue - shrink(p);
                                vet_glue(float_cast(glue_set(this_box)) * cur_glue);
                                cur_g = float_round(glue_temp);
                            }
                    }
                    rule.wd = rule.wd + cur_g;
                    if (subtype(p) >= a_leaders) {
                        /* output leaders in an hlist, |goto fin_rule| if a rule or to |next_p| if done */
                        leader_box = leader_ptr(p);
                        if (type(leader_box) == rule_node) {
                            rule.ht = height(leader_box);
                            rule.dp = depth(leader_box);
                            goto FIN_RULE;
                        }
                        if (textdir_parallel(box_dir(leader_box), localpos.dir))
                            leader_wd = width(leader_box);
                        else
                            leader_wd = height(leader_box) + depth(leader_box);
                        if ((leader_wd > 0) && (rule.wd > 0)) {
                            rule.wd = rule.wd + 10; /* compensate for floating-point rounding */
                            edge = cur.h + rule.wd;
                            lx = 0;
                            /*
                                let |cur.h| be the position of the first box, and set |leader_wd+lx|
                                to the spacing between corresponding parts of boxes
                            */
                            if (subtype(p) == g_leaders) {
                                save_h = cur.h;
                                switch (localpos.dir) {
                                    case dir_TLT:
                                        cur.h += refpos->pos.h - shipbox_refpos.h;
                                        cur.h = leader_wd * (cur.h / leader_wd);
                                        cur.h -= refpos->pos.h - shipbox_refpos.h;
                                        break;
                                    case dir_TRT:
                                        cur.h = refpos->pos.h - shipbox_refpos.h - cur.h;
                                        cur.h = leader_wd * (cur.h / leader_wd);
                                        cur.h = refpos->pos.h - shipbox_refpos.h - cur.h;
                                        break;
                                    case dir_LTL:
                                    case dir_RTT:
                                        cur.h = refpos->pos.v - shipbox_refpos.v - cur.h;
                                        cur.h = leader_wd * (cur.h / leader_wd);
                                        cur.h = refpos->pos.v - shipbox_refpos.v - cur.h;
                                        break;
                                    default:
                                        formatted_warning("pdf backend","forcing bad dir %i to TLT in hlist case 1",localpos.dir);
                                        localpos.dir = dir_TLT;
                                        cur.h += refpos->pos.h - shipbox_refpos.h;
                                        cur.h = leader_wd * (cur.h / leader_wd);
                                        cur.h -= refpos->pos.h - shipbox_refpos.h;
                                        break;
                                }
                                if (cur.h < save_h)
                                    cur.h += leader_wd;
                            } else if (subtype(p) == a_leaders) {
                                save_h = cur.h;
                                cur.h = leader_wd * (cur.h / leader_wd);
                                if (cur.h < save_h)
                                    cur.h += leader_wd;
                            } else {
                                lq = rule.wd / leader_wd; /* the number of box copies */
                                lr = rule.wd % leader_wd; /* the remaining space */
                                if (subtype(p) == c_leaders) {
                                    cur.h += lr / 2;
                                } else {
                                    lx = lr / (lq + 1);
                                    cur.h += (lr - (lq - 1) * lx) / 2;
                                }
                            }
                            while (cur.h + leader_wd <= edge) {
                                /* output a leader box at |cur.h|, then advance |cur.h| by |leader_wd+lx| */
                                if (pardir_parallel(box_dir(leader_box), localpos.dir)) {
                                    basepoint.v = 0;
                                    if (textdir_opposite(box_dir(leader_box), localpos.dir))
                                        basepoint.h = width(leader_box);
                                    else
                                        basepoint.h = 0;
                                } else {
                                    if (!is_mirrored(box_dir(leader_box))) {
                                        if (partextdir_eq(box_dir(leader_box), localpos.dir))
                                            basepoint.h = height(leader_box);
                                        else
                                            basepoint.h = depth(leader_box);
                                    } else {
                                        if (partextdir_eq(box_dir(leader_box), localpos.dir))
                                            basepoint.h = depth(leader_box);
                                        else
                                            basepoint.h = height(leader_box);
                                    }
                                    if (partextdir_eq(localpos.dir, box_dir(leader_box)))
                                        basepoint.v = -(width(leader_box) / 2);
                                    else
                                        basepoint.v = (width(leader_box) / 2);
                                }
                                if (!is_mirrored(localpos.dir))
                                    basepoint.v = basepoint.v + shift_amount(leader_box); /* shift the box down */
                                else
                                    basepoint.v = basepoint.v - shift_amount(leader_box); /* shift the box up */
                                tmpcur.h = cur.h + basepoint.h;
                                tmpcur.v = basepoint.v;
                                synch_pos_with_cur(pdf->posstruct, refpos, tmpcur);
                                outer_doing_leaders = doing_leaders;
                                doing_leaders = true;
                                if (type(leader_box) == vlist_node)
                                    vlist_out(pdf, leader_box, rule_callback_id);
                                else
                                    hlist_out(pdf, leader_box, rule_callback_id);
                                doing_leaders = outer_doing_leaders;
                                cur.h += leader_wd + lx;
                            }
                            cur.h = edge - 10;
                            goto NEXTP;
                        }
                    }
                    }
                    goto MOVE_PAST;
                    break;
                case kern_node:
                    if (synctex)
                        synctexkern(p, this_box);
                    /* officially we should check the subtype */
                    cur.h += kern_width(p);
                    break;
                case rule_node:
                    if (rule_dir(p) < 0)
                        rule_dir(p) = localpos.dir;
                    if (pardir_parallel(rule_dir(p), localpos.dir)) {
                        rule.ht = height(p);
                        rule.dp = depth(p);
                        rule.wd = width(p);
                    } else {
                        rule.ht = width(p) / 2;
                        rule.dp = width(p) / 2;
                        rule.wd = height(p) + depth(p);
                    }
                    goto FIN_RULE;
                    break;
                case dir_node:
                    /* output a reflection instruction if the direction has changed */
                    if (dir_dir(p) >= 0) {
                        /*
                            Calculate the needed width to the matching |enddir|, return the
                            |enddir| node, with width info
                        */
                        enddir_ptr = calculate_width_to_enddir(p, cur_glue, cur_g, this_box);
                        if (textdir_parallel(dir_dir(p), localpos.dir)) {
                            dir_cur_h(enddir_ptr) += cur.h;
                            if (textdir_opposite(dir_dir(p), localpos.dir))
                                cur.h = dir_cur_h(enddir_ptr);
                        } else
                            dir_cur_h(enddir_ptr) = cur.h;
                        if (enddir_ptr != p) {
                            /* only if it is an enddir */
                            dir_cur_v(enddir_ptr) = cur.v;
                            dir_refpos_h(enddir_ptr) = refpos->pos.h;
                            dir_refpos_v(enddir_ptr) = refpos->pos.v;
                            /* negative: mark it as |enddir| */
                            dir_dir(enddir_ptr) = localpos.dir - dir_swap;
                        }
                        /* fake a nested |hlist_out| */
                        synch_pos_with_cur(pdf->posstruct, refpos, cur);
                        refpos->pos = pdf->posstruct->pos;
                        localpos.dir = dir_dir(p);
                        cur.h = 0;
                        cur.v = 0;
                    } else {
                        refpos->pos.h = dir_refpos_h(p);
                        refpos->pos.v = dir_refpos_v(p);
                        localpos.dir = dir_dir(p) + dir_swap;
                        cur.h = dir_cur_h(p);
                        cur.v = dir_cur_v(p);
                    }
                    break;
                case whatsit_node:
                    /* output the whatsit node |p| in |hlist_out| */
                    switch (subtype(p)) {
                        case save_pos_node:
                            last_position = pdf->posstruct->pos;
                            pos_info.curpos = pdf->posstruct->pos;
                            pos_info.boxpos.pos = refpos->pos;
                            pos_info.boxpos.dir = localpos.dir;
                            pos_info.boxdim.wd = width(this_box);
                            pos_info.boxdim.ht = height(this_box);
                            pos_info.boxdim.dp = depth(this_box);
                            break;
                        case pdf_annot_node:
                        case pdf_start_link_node:
                        case pdf_dest_node:
                        case pdf_start_thread_node:
                        case pdf_thread_node:
                            backend_out_whatsit[subtype(p)] (pdf, p, this_box, cur);
                            break;
                        default:
                            out_what(pdf, p);
                    }
                    break;
                case math_node:
                    if (synctex) {
                        synctexmath(p, this_box);
                    }
                    /* begin mathskip code */
                    if (glue_is_zero(p)) {
                        cur.h += surround(p);
                        break;
                    } else {
                        /* fall through: mathskip */
                    }
                    /* end mathskip code */
                case margin_kern_node:
                    cur.h += width(p);
                    break;
                default:
                    break;
            }
            goto NEXTP;
          FIN_RULE:
            /* output a rule in an hlist */
            if (is_running(rule.ht))
                rule.ht = height(this_box);
            if (is_running(rule.dp))
                rule.dp = depth(this_box);
            /* we don't output empty rules */
            if ((rule.ht + rule.dp) > 0 && rule.wd > 0) {
                switch (localpos.dir) {
                    case dir_TLT:
                        size.h = rule.wd;
                        size.v = rule.ht + rule.dp;
                        pos_down(rule.dp);
                        break;
                    case dir_TRT:
                        size.h = rule.wd;
                        size.v = rule.ht + rule.dp;
                        pos_left(size.h);
                        pos_down(rule.dp);
                        break;
                    case dir_LTL:
                        size.h = rule.ht + rule.dp;
                        size.v = rule.wd;
                        pos_left(rule.ht);
                        pos_down(size.v);
                        break;
                    case dir_RTT:
                        size.h = rule.ht + rule.dp;
                        size.v = rule.wd;
                        pos_left(rule.dp);
                        pos_down(size.v);
                        break;
                    default:
                        formatted_warning("pdf backend","forcing bad dir %i to TLT in hlist case 2",localpos.dir);
                        localpos.dir = dir_TLT;
                        size.h = rule.wd;
                        size.v = rule.ht + rule.dp;
                        pos_down(rule.dp);
                        break;
                }
                if (type(p) == glue_node) {
                    q = leader_ptr(p);
                    if ((q) && (type(q) == rule_node)) {
                        backend_out[rule_node] (pdf, q, size, rule_callback_id);
                    }
                } else {
                    backend_out[rule_node] (pdf, p, size, rule_callback_id);
                }
            }
          MOVE_PAST:
            cur.h += rule.wd;
            if (synctex) {
                synch_pos_with_cur(pdf->posstruct, refpos, cur);
                synctexhorizontalruleorglue(p, this_box);
            }
          NEXTP:
            p = vlink(p);
            synch_pos_with_cur(pdf->posstruct, refpos, cur);
        }
    }
    if (synctex)
        synctextsilh(this_box);
    if (output_mode_used == OMODE_DVI) {
        prune_movements(save_loc);
        if (cur_s > 0) {
            dvi_pop(save_loc);
            dvi = save_dvi;
        }
    }
    cur_s--;
    pdf->posstruct = refpos;
}

@ @c
void vlist_out(PDF pdf, halfword this_box, int rule_callback_id)
{
    posstructure localpos; /* the position structure local within this function */
    posstructure *refpos;  /* the list origin pos. on the page, provided by the caller */

    scaledpos cur, tmpcur, basepoint;
    scaledpos size = { 0, 0 };     /* rule dimensions */
    scaled effective_vertical;
    scaled save_v;                 /* what |cur.v| should pop to */
    scaled top_edge;               /* the top coordinate for this box */
    scaled edge;                   /* bottom boundary of leader space */
    glue_ord g_order;              /* applicable order of infinity for glue */
    int g_sign;                    /* selects type of glue */
    halfword p;                    /* current position in the vlist */
    halfword q;                    /* temp */
    halfword leader_box;           /* the leader box being replicated */
    scaled leader_ht;              /* height of leader box being replicated */
    scaled lx;                     /* extra space between leader boxes */
    boolean outer_doing_leaders;   /* were we doing leaders? */
    real glue_temp;                /* glue value before rounding */
    real cur_glue = 0.0;           /* glue seen so far */
    scaled cur_g = 0;              /* rounded equivalent of |cur_glue| times the glue ratio */
    scaled_whd rule;
    int save_loc = 0;              /* DVI byte location upon entry */
    scaledpos save_dvi = { 0, 0 }; /* DVI! what |dvi| should pop to */
    int synctex = int_par(synctex_code) ;

    g_order = (glue_ord) glue_order(this_box);
    g_sign = glue_sign(this_box);
    p = list_ptr(this_box);

    cur.h = 0;
    cur.v = -height(this_box);
    top_edge = cur.v;

    refpos = pdf->posstruct;
    pdf->posstruct = &localpos; /* use local structure for recursion */
    localpos.dir = box_dir(this_box);
    synch_pos_with_cur(pdf->posstruct, refpos, cur);

    cur_s++;
    if (cur_s > max_push)
        max_push = cur_s;

    if (output_mode_used == OMODE_DVI) {
        if (cur_s > 0) {
            dvi_push();
            save_dvi = dvi;
        }
        save_loc = dvi_offset + dvi_ptr;
    }

    if (synctex)
        synctexvlist(this_box);

    /* create thread for the current vbox if needed */
    check_running_thread(pdf, this_box, cur);

    while (p != null) {
        switch (type(p)) {
            case hlist_node:
            case vlist_node:
                /*
                    output a box in a vlist:

                    todo: the direct test to switch between |width(p)| and |-width(p)|
                    is definately wrong, because it does not nest properly. But at least
                    it fixes a very obvious problem that otherwise occured with
                    \.{\\pardir TLT} in a document with \.{\\bodydir TRT}, and so it
                    will have to do for now. (hh: is this still true?)

                */
                if (pardir_parallel(box_dir(p), localpos.dir)) {
                    effective_vertical = height(p) + depth(p);
                    if ((type(p) == hlist_node) && is_mirrored(box_dir(p)))
                        basepoint.v = depth(p);
                    else
                        basepoint.v = height(p);
                    if (textdir_opposite(box_dir(p), localpos.dir))
                        basepoint.h = width(p);
                    else
                        basepoint.h = 0;
                } else {
                    effective_vertical = width(p);
                    if (!is_mirrored(box_dir(p))) {
                        if (partextdir_eq(box_dir(p), localpos.dir))
                            basepoint.h = height(p);
                        else
                            basepoint.h = depth(p);
                    } else {
                        if (partextdir_eq(box_dir(p), localpos.dir))
                            basepoint.h = depth(p);
                        else
                            basepoint.h = height(p);
                    }
                    if (partextdir_eq(localpos.dir, box_dir(p)))
                        basepoint.v = 0;
                    else
                        basepoint.v = width(p);
                }
                basepoint.h = basepoint.h + shift_amount(p); /* shift the box right */
                if (list_ptr(p) == null) {
                    cur.v += effective_vertical;
                    if (synctex) {
                        synch_pos_with_cur(pdf->posstruct, refpos, cur);
                        if (type(p) == vlist_node)
                            synctexvoidvlist(p, this_box);
                        else
                            synctexvoidhlist(p, this_box);
                    }
                } else {
                    tmpcur.h = basepoint.h;
                    tmpcur.v = cur.v + basepoint.v;
                    synch_pos_with_cur(pdf->posstruct, refpos, tmpcur);
                    if (type(p) == vlist_node)
                        vlist_out(pdf, p, rule_callback_id);
                    else
                        hlist_out(pdf, p, rule_callback_id);
                    cur.v += effective_vertical;
                }
                break;
            case rule_node:
                if (rule_dir(p) < 0)
                    rule_dir(p) = localpos.dir;
                if (pardir_parallel(rule_dir(p), localpos.dir)) {
                    rule.ht = height(p);
                    rule.dp = depth(p);
                    rule.wd = width(p);
                } else {
                    rule.ht = width(p) / 2;
                    rule.dp = width(p) / 2;
                    rule.wd = height(p) + depth(p);
                }
                goto FIN_RULE;
                break;
            case glue_node:
                {
                    /* move down or output leaders, we use real multiplication */
                    rule.ht = width(p) - cur_g;
                    if (g_sign != normal) {
                        if (g_sign == stretching) {
                            if (stretch_order(p) == g_order) {
                                cur_glue = cur_glue + stretch(p);
                                vet_glue(float_cast(glue_set(this_box)) * cur_glue);
                                cur_g = float_round(glue_temp);
                            }
                        } else if (shrink_order(p) == g_order) {
                            cur_glue = cur_glue - shrink(p);
                            vet_glue(float_cast(glue_set(this_box)) * cur_glue);
                            cur_g = float_round(glue_temp);
                        }
                }
                rule.ht = rule.ht + cur_g;
                if (subtype(p) >= a_leaders) {
                    /* output leaders in a vlist, |goto fin_rulefin_rule| if a rule or to |next_p| if done */
                    leader_box = leader_ptr(p);
                    if (type(leader_box) == rule_node) {
                        rule.wd = width(leader_box);
                        rule.dp = 0;
                        goto FIN_RULE;
                    }
                    leader_ht = height(leader_box) + depth(leader_box);
                    if ((leader_ht > 0) && (rule.ht > 0)) {
                        rule.ht = rule.ht + 10; /* compensate for floating-point rounding */
                        edge = cur.v + rule.ht;
                        lx = 0;
                        /*
                            let |cur.v| be the position of the first box, and set |leader_ht+lx|
                            to the spacing between corresponding parts of boxes
                        */
                        if (subtype(p) == g_leaders) {
                            save_v = cur.v;
                            switch (localpos.dir) {
                                case dir_LTL:
                                    cur.v += refpos->pos.h - shipbox_refpos.h;
                                    cur.v = leader_ht * (cur.v / leader_ht);
                                    cur.v -= refpos->pos.h - shipbox_refpos.h;
                                    break;
                                case dir_RTT:
                                    cur.v = refpos->pos.h - shipbox_refpos.h - cur.v;
                                    cur.v = leader_ht * (cur.v / leader_ht);
                                    cur.v = refpos->pos.h - shipbox_refpos.h - cur.v;
                                    break;
                                case dir_TLT:
                                case dir_TRT:
                                    cur.v = refpos->pos.v - shipbox_refpos.v - cur.v;
                                    cur.v = leader_ht * (cur.v / leader_ht);
                                    cur.v = refpos->pos.v - shipbox_refpos.v - cur.v;
                                    break;
                                default:
                                    formatted_warning("pdf backend","forcing bad dir %i to TLT in vlist case 1",localpos.dir);
                                    localpos.dir = dir_TLT;
                                    cur.v += refpos->pos.h - shipbox_refpos.h;
                                    cur.v = leader_ht * (cur.v / leader_ht);
                                    cur.v -= refpos->pos.h - shipbox_refpos.h;
                                    break;
                            }
                            if (cur.v < save_v)
                                cur.v += leader_ht;
                        } else if (subtype(p) == a_leaders) {
                            save_v = cur.v;
                            cur.v = top_edge + leader_ht * ((cur.v - top_edge) / leader_ht);
                            if (cur.v < save_v)
                                cur.v += leader_ht;
                        } else {
                            lq = rule.ht / leader_ht; /* the number of box copies */
                            lr = rule.ht % leader_ht; /* the remaining space */
                            if (subtype(p) == c_leaders) {
                                cur.v += lr / 2;
                            } else {
                                lx = lr / (lq + 1);
                                cur.v += (lr - (lq - 1) * lx) / 2;
                            }
                        }

                        while (cur.v + leader_ht <= edge) {
                            /* output a leader box at |cur.v|, then advance |cur.v| by |leader_ht+lx| */
                            tmpcur.h = shift_amount(leader_box);
                            tmpcur.v = cur.v + height(leader_box);
                            synch_pos_with_cur(pdf->posstruct, refpos, tmpcur);
                            outer_doing_leaders = doing_leaders;
                            doing_leaders = true;
                            if (type(leader_box) == vlist_node)
                                vlist_out(pdf, leader_box, rule_callback_id);
                            else
                                hlist_out(pdf, leader_box, rule_callback_id);
                            doing_leaders = outer_doing_leaders;
                            cur.v += leader_ht + lx;
                        }
                        cur.v = edge - 10;
                        goto NEXTP;
                    }
                }
                }
                goto MOVE_PAST;
                break;
            case kern_node:
                cur.v += width(p);
                break;
            case whatsit_node:
                /* output the whatsit node |p| in |vlist_out| */
                switch (subtype(p)) {
                    case save_pos_node:
                        last_position = pdf->posstruct->pos;
                        pos_info.curpos = pdf->posstruct->pos;
                        pos_info.boxpos.pos = refpos->pos;
                        pos_info.boxpos.dir = localpos.dir;
                        pos_info.boxdim.wd = width(this_box);
                        pos_info.boxdim.ht = height(this_box);
                        pos_info.boxdim.dp = depth(this_box);
                        break;
                    case pdf_annot_node:
                    case pdf_start_link_node:
                    case pdf_dest_node:
                    case pdf_start_thread_node:
                    case pdf_thread_node:
                        backend_out_whatsit[subtype(p)] (pdf, p, this_box, cur);
                        break;
                    default:
                        out_what(pdf, p);
                }
                break;
            case glyph_node:
            case disc_node:
                confusion("vlistout"); /* this can't happen */
                break;
            default:
                break;
        }
        goto NEXTP;
      FIN_RULE:
        /* output a rule in a vlist, |goto next_p| */
        if (is_running(rule.wd))
            rule.wd = width(this_box);
        /* we don't output empty rules */
        if ((rule.ht + rule.dp) > 0 && rule.wd > 0) {
            switch (localpos.dir) {
                case dir_TLT:
                    size.h = rule.wd;
                    size.v = rule.ht + rule.dp;
                    pos_down(size.v);
                    break;
                case dir_TRT:
                    size.h = rule.wd;
                    size.v = rule.ht + rule.dp;
                    pos_left(size.h);
                    pos_down(size.v);
                    break;
                case dir_LTL:
                    size.h = rule.ht + rule.dp;
                    size.v = rule.wd;
                    pos_down(size.v);
                    break;
                case dir_RTT:
                    size.h = rule.ht + rule.dp;
                    size.v = rule.wd;
                    pos_left(size.h);
                    pos_down(size.v);
                    break;
                default:
                    formatted_warning("pdf backend","forcing bad dir %i to TLT in vlist case 2",localpos.dir);
                    localpos.dir = dir_TLT;
                    size.h = rule.wd;
                    size.v = rule.ht + rule.dp;
                    pos_down(size.v);
                    break;
            }
            if (type(p) == glue_node) {
                q = leader_ptr(p);
                if ((q) && (type(q) == rule_node)) {
                    backend_out[rule_node] (pdf, q, size, rule_callback_id);
                }
            } else {
                backend_out[rule_node] (pdf, p, size, rule_callback_id);
            }
        }
        cur.v += rule.ht + rule.dp;
        goto NEXTP;
      MOVE_PAST:
        cur.v += rule.ht;
      NEXTP:
        p = vlink(p);
        synch_pos_with_cur(pdf->posstruct, refpos, cur);
    }
    if (synctex)
        synctextsilv(this_box);

    if (output_mode_used == OMODE_DVI) {
        prune_movements(save_loc);
        if (cur_s > 0) {
            dvi_pop(save_loc);
            dvi = save_dvi;
        }
    }
    cur_s--;
    pdf->posstruct = refpos;
}
