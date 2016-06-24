% pdfobj.w
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
#include "lua/luatex-api.h"

@ write a raw PDF object

@c
void pdf_write_obj(PDF pdf, int k)
{
    lstring data;
    const_lstring st;
    size_t li;                  /* index into |data.s| */
    int saved_compress_level = pdf->compress_level;
    int os_threshold = OBJSTM_ALWAYS;   /* gives compressed objects for \.{\\pdfvariable objcompresslevel} >= |OBJSTM_ALWAYS| */
    int l = 0;                          /* possibly a lua registry reference */
    int ll = 0;
    data.s = NULL;
    if (obj_obj_pdfcompresslevel(pdf, k) > -1)  /* -1 = "unset" */
        pdf->compress_level = obj_obj_pdfcompresslevel(pdf, k);
    if (obj_obj_objstm_threshold(pdf, k) != OBJSTM_UNSET)
        os_threshold = obj_obj_objstm_threshold(pdf, k);
    if (obj_obj_is_stream(pdf, k)) {
        pdf_begin_obj(pdf, k, OBJSTM_NEVER);
        pdf_begin_dict(pdf);
        l = obj_obj_stream_attr(pdf, k);
        if (l != LUA_NOREF) {
            lua_rawgeti(Luas, LUA_REGISTRYINDEX, l);
            if (lua_type(Luas,-1) != LUA_TSTRING)
                normal_error("pdf backend","invalid object");
            st.s = lua_tolstring(Luas, -1, &li);
            st.l = li;
            pdf_out_block(pdf, st.s, st.l);
            if (st.s[st.l - 1] != '\n')
                pdf_out(pdf, '\n');
            luaL_unref(Luas, LUA_REGISTRYINDEX, l);
            obj_obj_stream_attr(pdf, k) = LUA_NOREF;
        }
        pdf_dict_add_streaminfo(pdf);
        pdf_end_dict(pdf);
        pdf_begin_stream(pdf);
    } else
        pdf_begin_obj(pdf, k, os_threshold);
    l = obj_obj_data(pdf, k);
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, l);
    if (lua_type(Luas,-1) != LUA_TSTRING)
        normal_error("pdf backend","invalid object");
    st.s = lua_tolstring(Luas, -1, &li);
    st.l = li;
    lua_pop(Luas, 1);
    if (obj_obj_is_file(pdf, k)) {
        boolean res = false;      /* callback status value */
        const char *fnam = NULL;  /* callback found filename */
        int callback_id;
        /* st.s is also |\0|-terminated, even as lstring */
        fnam = luatex_find_file(st.s, find_data_file_callback);
        callback_id = callback_defined(read_data_file_callback);
        if (fnam && callback_id > 0) {
            boolean file_opened = false;
            res = run_callback(callback_id, "S->bSd", fnam, &file_opened, &data.s, &ll);
            data.l = (size_t) ll;
            if (!file_opened)
                normal_error("pdf backend", "cannot open file for embedding");
        } else {
            byte_file f;        /* the data file's FILE* */
            if (!fnam)
                fnam = st.s;
            if (!luatex_open_input(&f, fnam, kpse_tex_format, FOPEN_RBIN_MODE, true))
                normal_error("pdf backend", "cannot open file for embedding");
            res = read_data_file(f, &data.s, &ll);
            data.l = (size_t) ll;
            close_file(f);
        }
        if (data.l == 0L)
            normal_error("pdf backend", "empty file for embedding");
        if (!res)
            normal_error("pdf backend", "error reading file for embedding");
        tprint("<<");
        tprint(st.s);
        pdf_out_block(pdf, (const char *) data.s, data.l);
        xfree(data.s);
        tprint(">>");
    } else {
        pdf_out_block(pdf, st.s, st.l);
    }
    if (obj_obj_is_stream(pdf, k)) {
        pdf_end_stream(pdf);
        pdf_end_obj(pdf);
    } else /* here we do the \n */
        pdf_end_obj(pdf);
    luaL_unref(Luas, LUA_REGISTRYINDEX, l);
    obj_obj_data(pdf, k) = LUA_NOREF;
    pdf->compress_level = saved_compress_level;
}

@ @c
void init_obj_obj(PDF pdf, int k)
{
    obj_obj_stream_attr(pdf, k) = LUA_NOREF;
    obj_obj_data(pdf, k) = LUA_NOREF;
    unset_obj_obj_is_stream(pdf, k);
    unset_obj_obj_is_file(pdf, k);
    obj_obj_pdfcompresslevel(pdf, k) = -1; /* unset */
    obj_obj_objstm_threshold(pdf, k) = OBJSTM_UNSET; /* unset */
}

@ The \.{\\pdfextension obj} primitive is used to create a ``raw'' object in the
PDF output file. The object contents will be hold in memory and will be written
out only when the object is referenced by \.{\\pdfextension refobj}. When
\.{\\pdfextension obj} is used with \.{\\immediate}, the object contents will be
written out immediately. Objects referenced in the current page are appended into
|pdf_obj_list|.

@c
void scan_obj(PDF pdf)
{
    int k;
    lstring *st = NULL;
    if (scan_keyword("reserveobjnum")) {
        get_x_token();
        if (cur_cmd != spacer_cmd)
            back_input();
        pdf->obj_count++;
        k = pdf_create_obj(pdf, obj_type_obj, 0);
    } else {
        if (scan_keyword("useobjnum")) {
            scan_int();
            k = cur_val;
            check_obj_type(pdf, obj_type_obj, k);
            if (is_obj_scheduled(pdf, k) || obj_data_ptr(pdf, k) != 0)
                luaL_error(Luas, "object in use");
        } else {
            pdf->obj_count++;
            k = pdf_create_obj(pdf, obj_type_obj, 0);
        }
        obj_data_ptr(pdf, k) = pdf_get_mem(pdf, pdfmem_obj_size);
        init_obj_obj(pdf, k);
        if (scan_keyword("uncompressed")) {
            obj_obj_pdfcompresslevel(pdf, k) = 0;
            obj_obj_objstm_threshold(pdf, k) = OBJSTM_NEVER;
        }
        if (scan_keyword("stream")) {
            set_obj_obj_is_stream(pdf, k);
            if (scan_keyword("attr")) {
                scan_toks(false, true);
                st = tokenlist_to_lstring(def_ref, true);
                flush_list(def_ref);
                lua_pushlstring(Luas, (char *) st->s, st->l);
                obj_obj_stream_attr(pdf, k) = luaL_ref(Luas, LUA_REGISTRYINDEX);
                free_lstring(st);
                st = NULL;
            }
        }
        if (scan_keyword("file"))
            set_obj_obj_is_file(pdf, k);
        scan_toks(false, true);
        st = tokenlist_to_lstring(def_ref, true);
        flush_list(def_ref);
        lua_pushlstring(Luas, (char *) st->s, st->l);
        obj_obj_data(pdf, k) = luaL_ref(Luas, LUA_REGISTRYINDEX);
        free_lstring(st);
        st = NULL;
    }
    pdf_last_obj = k;
}

@ @c
#define tail cur_list.tail_field

void scan_refobj(PDF pdf)
{
    scan_int();
    check_obj_type(pdf, obj_type_obj, cur_val);
    new_whatsit(pdf_refobj_node);
    pdf_obj_objnum(tail) = cur_val;
}

void scan_refobj_lua(PDF pdf, int k)
{
    check_obj_type(pdf, obj_type_obj, k);
    new_whatsit(pdf_refobj_node);
    pdf_obj_objnum(tail) = k;
}

@ @c
void pdf_ref_obj(PDF pdf, halfword p)
{
    if (!is_obj_scheduled(pdf, pdf_obj_objnum(p)))
        addto_page_resources(pdf, obj_type_obj, pdf_obj_objnum(p));
}

@ @c
void pdf_ref_obj_lua(PDF pdf, int k)
{
    if (!is_obj_scheduled(pdf, k))
        addto_page_resources(pdf, obj_type_obj, k);
}
