/*

directions.w

Copyright 2009-2014 Taco Hoekwater <taco@@luatex.org>

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

void scan_direction(void)
{
    int save_cur_cmd = cur_cmd;
    int save_cur_chr = cur_chr;
    get_x_token();
    if (cur_cmd == assign_dir_cmd) {
        cur_val = eqtb[cur_chr].cint;
        goto EXIT;
    } else {
        back_input();
    }
    if (scan_keyword("tlt")) {
        cur_val = dir_TLT;
    } else if (scan_keyword("trt")) {
        cur_val = dir_TRT;
    } else if (scan_keyword("ltl")) {
        cur_val = dir_LTL;
    } else if (scan_keyword("rtt")) {
        cur_val = dir_RTT;
    } else {
        tex_error("Bad direction", NULL);
        cur_val = 0;
    }
    get_x_token();
    if (cur_cmd != spacer_cmd)
        back_input();
  EXIT:
    cur_cmd = save_cur_cmd;
    cur_chr = save_cur_chr;
}

/*tex

    The next two are used by |postlinebreak.c|:

*/

halfword do_push_dir_node(halfword p, halfword a)
{
    halfword n = copy_node(a);
    vlink(n) = p;
    return n;
}

halfword do_pop_dir_node(halfword p)
{
    halfword n = vlink(p);
    flush_node(p);
    return n;
}

halfword dir_ptr;
halfword text_dir_ptr;

void initialize_directions(void)
{
    /*tex There is no need to do anything here at the moment. */
}

halfword new_dir(int s)
{
    halfword p = new_node(dir_node, 0);
    dir_dir(p) = s;
    dir_level(p) = cur_level;
    return p;
}

const char *dir_strings_par[4] = { [0] =
  "TLT","TRT","LTL","RTT",
};

const char *dir_strings_text_normal[4] = { [0] =
  "+TLT","+TRT","+LTL","+RTT",
};

const char *dir_strings_text_cancel[4] = { [0] =
  "-TLT","-TRT","-LTL","-RTT",
};

void print_dir_par(int d)
{
    tprint(dir_strings_par[d]);
}

void print_dir_text(halfword d)
{
    if (subtype(d) == cancel_dir) {
        tprint(dir_strings_text_cancel[dir_dir(d)]);
    } else {
        tprint(dir_strings_text_normal[dir_dir(d)]);
    }
}

scaled pack_width(int curdir, int pdir, halfword p, boolean isglyph)
{
    scaled wd = 0;
    if (isglyph) {
        if (textdir_parallel(curdir, pdir) == textglyphdir_orthogonal(pdir)) {
            wd = glyph_width(p);
            if (ex_glyph(p) != 0) {
             /* wd = round_xn_over_d(wd, 1000 + ex_glyph(p)/1000, 1000); */
                wd = ext_xn_over_d(wd, 1000000+ex_glyph(p), 1000000);

            }
        } else {
            wd = glyph_depth(p) + glyph_height(p);
        }
    } else {
        if (textdir_parallel(pdir, curdir))
            wd = width(p);
        else
            wd = depth(p) + height(p);
    }
    return wd;
}

scaled_whd pack_width_height_depth(int curdir, int pdir, halfword p, boolean isglyph)
{
    scaled_whd whd = { 0, 0, 0 };
    whd.wd = pack_width(curdir, pdir, p, isglyph);
    if (isglyph) {
        if (is_rotated(curdir)) {
            if (textdir_parallel(curdir, pdir))
                whd.ht = whd.dp = (glyph_height(p) + glyph_depth(p)) / 2;
            else
                whd.ht = whd.dp = glyph_width(p) / 2;
        } else if (is_rotated(pdir)) {
            if (textdir_parallel(curdir, pdir))
                whd.ht = whd.dp = (glyph_height(p) + glyph_depth(p)) / 2;
            else
                whd.ht = glyph_width(p);
        } else {
            if (glyphdir_eq(curdir, pdir)) {
                whd.ht = glyph_height(p);
                whd.dp = glyph_depth(p);
            } else if (glyphdir_opposite(curdir, pdir)) {
                whd.ht = glyph_depth(p);
                whd.dp = glyph_height(p);
            } else
                whd.ht = glyph_width(p);
        }
    } else {
        if (is_rotated(curdir)) {
            if (textdir_parallel(curdir, pdir))
                whd.ht = whd.dp = (height(p) + depth(p)) / 2;
            else
                whd.ht = whd.dp = width(p) / 2;
        } else if (pardir_eq(curdir, pdir)) {
            whd.ht = height(p);
            whd.dp = depth(p);
        } else if (pardir_opposite(curdir, pdir)) {
            whd.ht = depth(p);
            whd.dp = height(p);
        } else
            whd.ht = width(p);
    }
    return whd;
}

void update_text_dir_ptr(int val)
{
    if (dir_level(text_dir_ptr) == cur_level) {
        /* update */
        dir_dir(text_dir_ptr) = val;
    } else {
        /* addition */
        halfword text_dir_tmp = new_dir(val);
        vlink(text_dir_tmp) = text_dir_ptr;
        text_dir_ptr = text_dir_tmp;
    }
}
