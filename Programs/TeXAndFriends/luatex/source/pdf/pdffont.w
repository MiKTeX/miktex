% pdffont.w
%
% Copyright 2009-2014 Taco Hoekwater <taco@@luatex.org>
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

@ As \pdfTeX{} should also act as a back-end driver, it needs to support virtual
fonts too. Information about virtual fonts can be found in the source of some
\.{DVI}-related programs.

Whenever we want to write out a character in a font to PDF output, we
should check whether the used character is a virtual or real character.
The |has_packet()| C macro checks for this condition.

@ The following code typesets a character to PDF output

@c
scaled_whd output_one_char(PDF pdf, halfword p)
{
    internal_font_number f = font(p);
    int c = character(p);
    int ex_glyph = ex_glyph(p)/1000;
    scaled_whd ci = get_charinfo_whd(f, c); /* the real width, height and depth of the character */
    if (!(char_exists(f,c))) {
        char_warning(f,c);
        return ci;
    }
    ci.wd = ext_xn_over_d(ci.wd, 1000000 + ex_glyph(p), 1000000);
    switch (pdf->posstruct->dir) {
        case dir_TLT:
            break;
        case dir_TRT:
            pos_left(ci.wd);
            break;
        case dir_LTL:
            pos_down(ci.ht);
            pos_left(ci.wd);
            break;
        case dir_RTT:
            pos_down(ci.ht);
            pos_left(ci.wd / 2);
            break;
        default:
            formatted_warning("pdf backend","ignoring bad dir %i when outputting a character",pdf->posstruct->dir);
    }
    if (has_packet(f, c)) {
        do_vf_packet(pdf, f, c, ex_glyph);
    } else {
        /* |pdf_place_glyph(pdf, f, c, ex_glyph);| */
        backend_out[glyph_node] (pdf, f, c, ex_glyph);
    }
    return ci;
}

@ Mark |f| as a used font; set |font_used(f)|, |font_size(f)| and |pdf_font_num(f)|
@c
static void pdf_use_font(internal_font_number f, int fontnum)
{
    set_font_used(f, true);
    if ((fontnum > 0) || ((fontnum < 0) && (pdf_font_num(-fontnum) > 0))) {
        set_pdf_font_num(f, fontnum);
    } else {
        normal_error("pdf backend","bad font id");
    }
}

@ To set PDF font we need to find out fonts with the same name, because \TeX\ can
load the same font several times for various sizes. For such fonts we define only
one font resource. The array |pdf_font_num| holds the object number of font
resource. A negative value of an entry of |pdf_font_num| indicates that the
corresponding font shares the font resource with the font

@c
#define same(n,f,k) (n(f) != NULL && n(k) != NULL && strcmp(n(f), n(k)) == 0)

/*
    For some lua-loaded (for instance AFM) fonts, it is normal to have
    a zero cidregistry,  and such fonts do not have a fontmap entry yet
    at this point, so the test should use the other branch
*/

static boolean font_shareable(internal_font_number f, internal_font_number k)
{
    if (font_cidregistry(f) == NULL && font_cidregistry(k) == NULL && font_encodingbytes(f) != 2 && font_encodingbytes(k) != 2) {
        if (font_map(k) != NULL && font_map(f) != NULL && (same(font_name, k, f))) {
            return 1;
        }
    } else if ((same(font_filename, k, f) && same(font_fullname, k, f))) {
        return 1;
    }
    return 0;
}

@ create a font object
@c
void pdf_init_font(PDF pdf, internal_font_number f)
{
    internal_font_number k;
    fm_entry *fm;
    int i, l;
    if (font_used(f)) {
        formatted_error("pdf backend","font %i gets initialized twice",(int) f);
    }
    /*
        check whether |f| can share the font object with some |k|: we have 2 cases
        here: 1) |f| and |k| have the same tfm name (so they have been loaded at
        different sizes, eg 'cmr10' and 'cmr10 at 11pt'); 2) |f| has been auto
        expanded from |k|

        take over slant and extend from map entry, if not already set;
        this should also be the only place where getfontmap() may be called.
    */
    fm = getfontmap(font_name(f));
    if (font_map(f) == NULL && fm != NULL) {
        font_map(f) = fm;
        if (is_slantset(fm))
            font_slant(f) = fm->slant;
        if (is_extendset(fm))
            font_extend(f) = fm->extend;
    }
    i = pdf->head_tab[obj_type_font];
    while (i != 0) {
        k = obj_info(pdf, i);
        if (font_shareable(f, k)) {
            if (pdf_font_num(k) < 0)
                pdf_use_font(f, pdf_font_num(k));
            else
                pdf_use_font(f, -k);
            return;
        }
        i = obj_link(pdf, i);
    }
    /* create a new font object for |f| */
    l = pdf_create_obj(pdf, obj_type_font, f);
    pdf_use_font(f, l);
}

@ set the actual font on PDF page; sets |ff| to the tfm number of the base font
sharing the font object with |f|; |ff| is either |f| itself (then it is its own
base font), or some font with the same tfm name at different size and/or
expansion.

@c
internal_font_number pdf_set_font(PDF pdf, internal_font_number f)
{
    int ff; /* for use with |set_ff| */
    if (!font_used(f))
        pdf_init_font(pdf, f);
    ff = pdf_font_num(f) < 0 ? -pdf_font_num(f) : f; /* aka |set_ff(f)| */
    addto_page_resources(pdf, obj_type_font, pdf_font_num(ff));
    return ff;
}

@ @c
void pdf_include_chars(PDF pdf)
{
    str_number s;
    unsigned char *k, *j; /* running index */
    internal_font_number f;
    scan_font_ident();
    f = cur_val;
    if (f == null_font)
        normal_error("pdf backend", "invalid font identifier for 'includechars'");
    pdf_check_vf(cur_val);
    if (!font_used(f))
        pdf_init_font(pdf, f);
    scan_toks(false, true);
    s = tokens_to_string(def_ref);
    delete_token_ref(def_ref);
    j = str_string(s) + str_length(s);
    for (k = str_string(s); k < j; k++) {
        pdf_mark_char(f, *k);
    }
    flush_str(s);
}
