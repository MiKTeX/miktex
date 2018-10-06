/*

Copyright 2006-2008 Taco Hoekwater <taco@luatex.org>

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
#include "font/writettf.h"
#include "font/writecff.h"

/*tex

    Here we also support the newer CFF2 specification,

*/

extern unsigned char *ttf_buffer;

void writetype0(PDF pdf, fd_entry * fd)
{
    int callback_id;
    int file_opened = 0;
    long i = 0;
    dirtab_entry *tab;
    cff_font *cff;
    sfnt *sfont;
    dir_tab = NULL;
    glyph_tab = NULL;
    /*tex |fd_cur| is global inside |writettf.c| */
    fd_cur = fd;
    assert(fd_cur->fm != NULL);
    assert(is_opentype(fd_cur->fm) || is_truetype(fd_cur->fm));
    assert(is_included(fd_cur->fm));
    ttf_curbyte = 0;
    ttf_size = 0;
    cur_file_name =
        luatex_find_file(fd_cur->fm->ff_name, find_opentype_file_callback);
    if (cur_file_name == NULL) {
        cur_file_name =
            luatex_find_file(fd_cur->fm->ff_name, find_truetype_file_callback);
        if (cur_file_name == NULL) {
            formatted_error("type 0","cannot find file '%s'", fd_cur->fm->ff_name);
        }
    }
    callback_id = callback_defined(read_opentype_file_callback);
    if (callback_id > 0) {
        if (run_callback(callback_id, "S->bSd", cur_file_name,
                         &file_opened, &ttf_buffer, &ttf_size) &&
            file_opened && ttf_size > 0) {
        } else {
            formatted_error("type 0","cannot find file '%s'", cur_file_name);
        }
    } else {
        if (!otf_open(cur_file_name)) {
            formatted_error("type 0","cannot find file '%s'", cur_file_name);
        }
        ttf_read_file();
        ttf_close();
    }
    fd_cur->ff_found = true;
    sfont = sfnt_open(ttf_buffer, ttf_size);
    if (sfont->type == SFNT_TYPE_TTC)
        i = ff_get_ttc_index(fd->fm->ff_name, fd->fm->ps_name);

    if (is_subsetted(fd_cur->fm)) {
        report_start_file(filetype_subset, cur_file_name);
    } else {
        report_start_file(filetype_font, cur_file_name);
    }

    if (sfont->type == SFNT_TYPE_TTC) otc_read_tabdir(i);
    else ttf_read_tabdir();
    sfnt_close(sfont);

    /*tex Read font parameters: */
    if (ttf_name_lookup("head", false) != NULL)
        ttf_read_head();
    if (ttf_name_lookup("hhea", false) != NULL)
        ttf_read_hhea();
    if (ttf_name_lookup("PCLT", false) != NULL)
        ttf_read_pclt();
    if (ttf_name_lookup("post", false) != NULL)
        ttf_read_post();
    /*tex Copy font file, including the newer variant: */
    if (ttf_name_lookup("CFF2", false) != NULL)
        tab = ttf_seek_tab("CFF2", 0);
    else
        tab = ttf_seek_tab("CFF ", 0);
    cff = read_cff(ttf_buffer + ttf_curbyte, (long) tab->length, 0);
    if (!is_subsetted(fd_cur->fm)) {
        /*tex not subsetted, copy: */
        for (i = (long) tab->length; i > 0; i--)
            strbuf_putchar(pdf->fb, (unsigned char) ttf_getnum(1));
    } else {
        if (cff != NULL) {
            if (cff_is_cidfont(cff)) {
                write_cid_cff(pdf, cff, fd_cur);
            } else {
                write_cff(pdf, cff, fd_cur);
            }
        } else {
            /*tex Just copy: */
            for (i = (long) tab->length; i > 0; i--)
                strbuf_putchar(pdf->fb, (unsigned char) ttf_getnum(1));
        }
    }
    xfree(dir_tab);
    xfree(ttf_buffer);
    if (is_subsetted(fd_cur->fm)) {
        report_stop_file(filetype_subset);
    } else {
        report_stop_file(filetype_font);
    }
    cur_file_name = NULL;
}
