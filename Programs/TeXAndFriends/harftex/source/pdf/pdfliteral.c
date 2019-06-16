/*

Copyright 2009-2010 Taco Hoekwater <taco@luatex.org>

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

void pdf_special(PDF pdf, halfword p)
{
    int old_setting = selector;
    str_number s;
    selector = new_string;
    show_token_list(token_link(write_tokens(p)), null, -1);
    selector = old_setting;
    s = make_string();
    pdf_literal(pdf, s, scan_special, true);
    flush_str(s);
}

/*tex

    To ship out a \TeX\ box to PDF page description we need to implement
    |hlist_out|, |vlist_out| and |ship_out|, which are equivalent to the \TeX'
    original |hlist_out|, |vlist_out| and |ship_out| resp. But first we need to
    declare some procedures needed in |hlist_out| and |vlist_out|.

*/

void pdf_out_literal(PDF pdf, halfword p)
{
    int old_setting;
    str_number s;
    int t = pdf_literal_type(p);
    pdfstructure *ps = pdf->pstruct;
    if (t == normal) {
        old_setting = selector;
        selector = new_string;
        show_token_list(token_link(pdf_literal_data(p)), null, -1);
        selector = old_setting;
        s = make_string();
        pdf_literal(pdf, s, pdf_literal_mode(p), false);
        flush_str(s);
    } else if (t == lua_refid_literal) {
        switch (pdf_literal_mode(p)) {
            case set_origin:
                pdf_goto_pagemode(pdf);
                pdf_set_pos(pdf, pdf->posstruct->pos);
                break;
            case direct_page:
                pdf_goto_pagemode(pdf);
                break;
            case direct_text:
                pdf_goto_textmode(pdf);
                break;
            case direct_font:
                pdf_goto_fontmode(pdf);
                break;
            case direct_always:
                pdf_end_string_nl(pdf);
                ps->need_tm = true;
                break;
            case direct_raw:
                pdf_end_string_nl(pdf);
                break;
            default:
                normal_error("pdf backend","bad literal mode");
                break;
        }
        lua_pdf_literal(pdf, pdf_literal_data(p), 0);
    }
}

/*tex Test equality of start of strings: */

static boolean str_in_cstr(str_number s, const char *r, unsigned i)
{
    const unsigned char *k, *l;
    if ((unsigned) str_length(s) < i + strlen(r))
        return false;
    k = (const unsigned char *) r;
    l = str_string(s) + i;
    while ((*l) && (*k)) {
        if (*l++ != *k++)
            return false;
    }
    return true;
}

void pdf_literal(PDF pdf, str_number s, int literal_mode, boolean warn)
{
    unsigned char *ss;
    size_t l;
    /*tex
        The current character code position, initialized to make the compiler
        happy:
    */
    pool_pointer j = 0;
    pdfstructure *p = pdf->pstruct;
    if (s >= STRING_OFFSET) {
        /*tex Needed for |out_save|: */
        j = 0;
        /*tex
            Unfortunately we always go through this when we have vf specials (and
            also via temp strings):
        */
        if (literal_mode == scan_special) {
            if (!(str_in_cstr(s, "pdf:", 0) || str_in_cstr(s, "PDF:", 0))) {
                if (warn && ((!(str_in_cstr(s, "src:", 0) || str_in_cstr(s, "SRC:", 0))) || (str_length(s) == 0)))
                    tprint_nl("Non-PDF special ignored!");
                return;
            }
            /*tex |strlen("PDF:")| */
            j = j + (pool_pointer) 4;
            if (str_in_cstr(s, "direct:", 4)) {
                /*tex |strlen("direct:")| */
                j = j + (pool_pointer) 7;
                literal_mode = direct_always;
            } else if (str_in_cstr(s, "page:", 4)) {
                /*tex |strlen("page:")| */
                j = j + (pool_pointer) 5;
                literal_mode = direct_page;
            } else if (str_in_cstr(s, "text:", 4)) {
                /*tex |strlen("text:")| */
                j = j + (pool_pointer) 5;
                literal_mode = direct_text;
            } else if (str_in_cstr(s, "raw:", 4)) {
                /*tex |strlen("raw:")| */
                j = j + (pool_pointer) 4;
                literal_mode = direct_raw;
            } else if (str_in_cstr(s, "origin:", 4)) {
                /*tex |strlen("origin:")| */
                j = j + (pool_pointer) 7;
                literal_mode = set_origin;
            } else {
                literal_mode = set_origin;
            }
        }
    }
    switch (literal_mode) {
        case set_origin:
            pdf_goto_pagemode(pdf);
            pdf_set_pos(pdf, pdf->posstruct->pos);
            break;
        case direct_page:
            pdf_goto_pagemode(pdf);
            break;
        case direct_text:
            pdf_goto_fontmode(pdf);
            /*tex not: |pdf_goto_textmode(pdf);| */
            break;
        case direct_always:
            pdf_end_string_nl(pdf);
            p->need_tm = true;
            break;
        case direct_raw:
            pdf_end_string_nl(pdf);
            break;
        default:
            normal_error("pdf backend","bad literal mode");
            break;
    }
    if (s >= STRING_OFFSET) {
        ss = str_string(s);
        l = str_length(s) - (size_t) j;
        pdf_out_block(pdf, (const char *) (ss + j), l);
    } else {
        pdf_out(pdf, s);
    }
    pdf_out(pdf, '\n');
}

void pdf_literal_set_mode(PDF pdf, int literal_mode)
{
    pdfstructure *p = pdf->pstruct;
    switch (literal_mode) {
        case set_origin:
            pdf_goto_pagemode(pdf);
            pdf_set_pos(pdf, pdf->posstruct->pos);
            break;
        case direct_page:
            pdf_goto_pagemode(pdf);
            break;
        case direct_text:
            pdf_goto_textmode(pdf);
            break;
        case direct_font:
            pdf_goto_fontmode(pdf);
            break;
        case direct_always:
            pdf_end_string_nl(pdf);
            p->need_tm = true;
            break;
        case direct_raw:
            pdf_end_string_nl(pdf);
            break;
        default:
            normal_error("pdf backend","bad literal mode");
            break;
    }
}
