/* lpdflib.c

   Copyright 2006-2011 Taco Hoekwater <taco@luatex.org>

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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */

#include "ptexlib.h"
#include "lua/luatex-api.h"
#include "pdf/pdftables.h"

int luapdfprint(lua_State * L)
{
    int n;
    const_lstring st;
    const char *modestr_s;
    ctm_transform_modes literal_mode;
    st.s = modestr_s = NULL;
    n = lua_gettop(L);
    if (!lua_isstring(L, -1)) { /* or number */
        luaL_error(L, "no string to print");
    }
    literal_mode = set_origin;
    if (n == 2) {
        if (lua_type(L,-2) != LUA_TSTRING) {
            luaL_error(L, "invalid first argument for print literal mode");
        } else {
            modestr_s = lua_tostring(L, -2);
            if (lua_key_eq(modestr_s,direct))
                literal_mode = direct_always;
            else if (lua_key_eq(modestr_s,page))
                literal_mode = direct_page;
            else if (lua_key_eq(modestr_s,text))
                literal_mode = direct_text;
            else if (lua_key_eq(modestr_s,raw))
                literal_mode = direct_raw;
            else if (lua_key_eq(modestr_s,origin))
                literal_mode = set_origin;
            else  {
                luaL_error(L, "invalid first argument for print literal mode");
            }
        }
    } else if (n != 1) {
        luaL_error(L, "invalid number of arguments");
    }
    check_o_mode(static_pdf, "pdf.print()", 1 << OMODE_PDF, true);
    switch (literal_mode) {
        case (set_origin):
            pdf_goto_pagemode(static_pdf);
            pdf_set_pos(static_pdf, static_pdf->posstruct->pos);
            (void) calc_pdfpos(static_pdf->pstruct, static_pdf->posstruct->pos);
            break;
        case (direct_page):
            pdf_goto_pagemode(static_pdf);
            (void) calc_pdfpos(static_pdf->pstruct, static_pdf->posstruct->pos);
            break;
        case (direct_text):
            pdf_goto_textmode(static_pdf);
            (void) calc_pdfpos(static_pdf->pstruct, static_pdf->posstruct->pos);
            break;
        case (direct_always):
            pdf_end_string_nl(static_pdf);
            break;
        case (direct_raw):
            pdf_end_string_nl(static_pdf);
            break;
        default:
            assert(0);
    }
    st.s = lua_tolstring(L, n, &st.l);
    pdf_out_block(static_pdf, st.s, st.l);
/*  pdf_out(pdf, '\n'); */
    return 0;
}

static unsigned char *fread_to_buf(lua_State * L, const char *filename, size_t * len)
{
    int ilen = 0;
    FILE *f;
    unsigned char *buf = NULL;
    if ((f = fopen(filename, "rb")) == NULL)
        luaL_error(L, "pdf.immediateobj() cannot open input file");
    if (readbinfile(f, &buf, &ilen) == 0)
        luaL_error(L, "pdf.immediateobj() cannot read input file");
    fclose(f);
    *len = (size_t) ilen;
    return buf;
}

static int l_immediateobj(lua_State * L)
{
    int n, first_arg = 1;
    int k;
    lstring buf;
    const_lstring st1,st2, st3;
    const char *st1_s = NULL;
    st1.s = st2.s = st3.s = NULL;
    check_o_mode(static_pdf, "immediateobj()", 1 << OMODE_PDF, true);
    if (global_shipping_mode != NOT_SHIPPING)
        luaL_error(L, "pdf.immediateobj() can not be used with \\latelua");
    n = lua_gettop(L);
    if ((n > 0) && (lua_type(L, 1) == LUA_TNUMBER)) {
        first_arg++;
        k = (int) lua_tointeger(L, 1);
        check_obj_type(static_pdf, obj_type_obj, k);
        if (is_obj_scheduled(static_pdf, k) || obj_data_ptr(static_pdf, k) != 0)
            luaL_error(L, "pdf.immediateobj() object in use");
    } else {
        static_pdf->obj_count++;
        k = pdf_create_obj(static_pdf, obj_type_obj, static_pdf->obj_ptr + 1);
    }
    pdf_last_obj = k;
    switch (n - first_arg + 1) {
        case 0:
            luaL_error(L, "pdf.immediateobj() needs at least one argument");
            break;
        case 1:
            if (!lua_isstring(L, first_arg)) /* or number */
                luaL_error(L, "pdf.immediateobj() 1st argument must be string");
            pdf_begin_obj(static_pdf, k, OBJSTM_ALWAYS);
            st1.s = lua_tolstring(L, first_arg, &st1.l);
            pdf_out_block(static_pdf, st1.s, st1.l);
            pdf_end_obj(static_pdf);
            break;
        case 2:
        case 3:
            if (lua_type(L,first_arg) != LUA_TSTRING)
                luaL_error(L, "pdf.immediateobj() 1st argument must be string");
            if (!lua_isstring(L, first_arg + 1)) /* or number */
                luaL_error(L, "pdf.immediateobj() 2nd argument must be string");
            st1_s = lua_tostring(L, first_arg);
            st2.s = lua_tolstring(L, first_arg + 1, &st2.l);
            if (lua_key_eq(st1_s, file)) {
                if (n == first_arg + 2)
                    luaL_error(L, "pdf.immediateobj() 3rd argument forbidden in file mode");
                pdf_begin_obj(static_pdf, k, OBJSTM_ALWAYS);
                buf.s = fread_to_buf(L, st2.s, &buf.l);
                pdf_out_block(static_pdf, (const char *) buf.s, buf.l);
                xfree(buf.s);
                pdf_end_obj(static_pdf);
            } else {
                pdf_begin_obj(static_pdf, k, OBJSTM_NEVER); /* not an object stream candidate! */
                pdf_begin_dict(static_pdf);
                if (n == first_arg + 2) {   /* write attr text */
                    if (!lua_isstring(L, first_arg + 2)) /* or number (maybe only string as it's an attr) */
                        luaL_error(L, "pdf.immediateobj() 3rd argument must be string");
                    st3.s = lua_tolstring(L, first_arg + 2, &st3.l);
                    pdf_out_block(static_pdf, st3.s, st3.l);
                    if (st3.s[st3.l - 1] != '\n')
                        pdf_out(static_pdf, '\n');
                }
                pdf_dict_add_streaminfo(static_pdf);
                pdf_end_dict(static_pdf);
                pdf_begin_stream(static_pdf);
                if (lua_key_eq(st1_s, stream)) {
                    pdf_out_block(static_pdf, st2.s, st2.l);
                }  else if (lua_key_eq(st1_s, streamfile)) {
                    buf.s = fread_to_buf(L, st2.s, &buf.l);
                    pdf_out_block(static_pdf, (const char *) buf.s, buf.l);
                    xfree(buf.s);
                } else
                    luaL_error(L, "pdf.immediateobj() invalid argument");
                pdf_end_stream(static_pdf);
                pdf_end_obj(static_pdf);
            }
            break;
        default:
            luaL_error(L, "pdf.immediateobj() allows max. 3 arguments");
    }
    lua_pushinteger(L, k);
    return 1;
}

static int table_obj(lua_State * L)
{
    const char *type;
    int k, obj_compression;
    int compress_level = -1;    /* unset */
    int os_threshold = OBJSTM_ALWAYS;   /* default: put non-stream objects into object streams */
    int saved_compress_level = static_pdf->compress_level;
    const_lstring attr, st;
    lstring buf;
    int immediate = 0;          /* default: not immediate */
    int nolength = 0;
    attr.s = st.s = NULL;
    attr.l = 0;
    assert(lua_istable(L, 1));  /* t */
    lua_key_rawgeti(type);
    if (lua_isnil(L, -1))       /* !vs t */
        luaL_error(L, "pdf.obj(): object \"type\" missing");
    if (lua_type(L,-1) != LUA_TSTRING)   /* !vs t */
        luaL_error(L, "pdf.obj(): object \"type\" must be string");
    type = lua_tostring(L, -1);
    if (! (lua_key_eq(type, raw) || lua_key_eq(type, stream))) {
        luaL_error(L, "pdf.obj(): \"%s\" is not a valid object type", type);     /* i vs t */
    }
    lua_pop(L, 1);              /* t */
    lua_key_rawgeti(immediate);
    if (!lua_isnil(L, -1)) {    /* b? t */
        if (!lua_isboolean(L, -1))      /* !b t */
            luaL_error(L, "pdf.obj(): \"immediate\" must be boolean");
        immediate = lua_toboolean(L, -1);       /* 0 or 1 */
    }
    lua_pop(L, 1);              /* t */
    lua_key_rawgeti(nolength);
    if (!lua_isnil(L, -1)) {    /* b? t */
        if (lua_isboolean(L, -1))      /* !b t */
            nolength = lua_toboolean(L, -1);       /* 0 or 1 */
    }
    lua_pop(L, 1);              /* t */

    /* is a reserved object referenced by "objnum"? */

    lua_key_rawgeti(objnum);
    if (!lua_isnil(L, -1)) {    /* vi? t */
        if (lua_type(L,-1) != LUA_TNUMBER)       /* !vi t */
            luaL_error(L, "pdf.obj(): \"objnum\" must be integer");
        k = (int) lua_tointeger(L, -1); /* vi t */
        check_obj_type(static_pdf, obj_type_obj, k);
        if (is_obj_scheduled(static_pdf, k) || obj_data_ptr(static_pdf, k) != 0)
            luaL_error(L, "pdf.obj() object in use");
    } else {
        static_pdf->obj_count++;
        k = pdf_create_obj(static_pdf, obj_type_obj, static_pdf->obj_ptr + 1);
    }
    pdf_last_obj = k;
    if (immediate == 0) {
        obj_data_ptr(static_pdf, k) = pdf_get_mem(static_pdf, pdfmem_obj_size);
        init_obj_obj(static_pdf, k);
    }
    lua_pop(L, 1);              /* t */

    /* get optional "attr" (allowed only for stream case) */

    lua_key_rawgeti(attr);
    if (!lua_isnil(L, -1)) {    /* attr-s? t */
        if (! lua_key_eq(type, stream))
            luaL_error(L, "pdf.obj(): \"attr\" key not allowed for non-stream object");
        if (!lua_isstring(L, -1)) /* or number */      /* !attr-s t */
            luaL_error(L, "pdf.obj(): object \"attr\" must be string");
        if (immediate == 1) {
            attr.s = lua_tolstring(L, -1, &attr.l);     /* attr-s t */
            lua_pop(L, 1);      /* t */
        } else
            obj_obj_stream_attr(static_pdf, k) = luaL_ref(Luas, LUA_REGISTRYINDEX);     /* t */
    } else {
        lua_pop(L, 1);          /* t */
    }

    /* get optional "compresslevel" (allowed only for stream case) */

    lua_key_rawgeti(compresslevel);
    if (!lua_isnil(L, -1)) {    /* vi? t */
        if (lua_key_eq(type, raw))
            luaL_error(L, "pdf.obj(): \"compresslevel\" key not allowed for raw object");
        if (lua_type(L, -1) != LUA_TNUMBER)       /* !vi t */
            luaL_error(L, "pdf.obj(): \"compresslevel\" must be integer");
        compress_level = (int) lua_tointeger(L, -1);    /* vi t */
        if (compress_level > 9)
            luaL_error(L, "pdf.obj(): \"compresslevel\" must be <= 9");
        else if (compress_level < 0)
            luaL_error(L, "pdf.obj(): \"compresslevel\" must be >= 0");
        if (immediate == 0)
            obj_obj_pdfcompresslevel(static_pdf, k) = compress_level;
    }
    lua_pop(L, 1);              /* t */

    /* get optional "objcompression" (allowed only for non-stream case) */

    lua_key_rawgeti(objcompression);
    if (!lua_isnil(L, -1)) {    /* b? t */
        if (lua_key_eq(type, stream))
            luaL_error(L, "pdf.obj(): \"objcompression\" key not allowed for stream object");
        if (!lua_isboolean(L, -1))      /* !b t */
            luaL_error(L, "pdf.obj(): \"objcompression\" must be boolean");
        obj_compression = lua_toboolean(L, -1); /* 0 or 1 */
        /* OBJSTM_NEVER: never into object stream; OBJSTM_ALWAYS: depends then on \pdfobjcompresslevel */
        if (obj_compression > 0)
            os_threshold = OBJSTM_ALWAYS;
        else
            os_threshold = OBJSTM_NEVER;
        if (immediate == 0)
            obj_obj_objstm_threshold(static_pdf, k) = os_threshold;
    }
    lua_pop(L, 1);              /* t */

    /* now the object contents for all cases are handled */

    lua_key_rawgeti(string);
    lua_key_rawgeti_n(file,-2);

    if (!lua_isnil(L, -1) && !lua_isnil(L, -2)) /* file-s? string-s? t */
        luaL_error(L, "pdf.obj(): \"string\" and \"file\" must not be given together");
    if (lua_isnil(L, -1) && lua_isnil(L, -2))   /* nil nil t */
        luaL_error(L, "pdf.obj(): no \"string\" or \"file\" given");

    if (lua_key_eq(type, raw)) {
        if (immediate == 1)
            pdf_begin_obj(static_pdf, k, os_threshold);
        if (!lua_isnil(L, -2)) {        /* file-s? string-s? t */
            /* from string */
            lua_pop(L, 1);      /* string-s? t */
            if (!lua_isstring(L, -1)) /* or number */   /* !string-s t */
                luaL_error(L, "pdf.obj(): \"string\" must be string for raw object");
            if (immediate == 1) {
                st.s = lua_tolstring(L, -1, &st.l);
                pdf_out_block(static_pdf, st.s, st.l);
            } else
                obj_obj_data(static_pdf, k) = luaL_ref(L, LUA_REGISTRYINDEX);   /* t */
        } else {
            /* from file */
            if (lua_type(L, -1) != LUA_TSTRING)   /* !file-s nil t */
                luaL_error(L, "pdf.obj(): \"file\" name must be string for raw object");
            if (immediate == 1) {
                st.s = lua_tolstring(L, -1, &st.l);     /* file-s nil t */
                buf.s = fread_to_buf(L, st.s, &buf.l);
                pdf_out_block(static_pdf, (const char *) buf.s, buf.l);
                /* already in pdf_end_obj:
                    if (buf.s[buf.l - 1] != '\n')
                        pdf_out(static_pdf, '\n');
                */
                xfree(buf.s);
            } else {
                set_obj_obj_is_file(static_pdf, k);
                obj_obj_data(static_pdf, k) = luaL_ref(L, LUA_REGISTRYINDEX);   /* nil t */
            }
        }
        if (immediate == 1)
            pdf_end_obj(static_pdf);
    } else {
        if (immediate == 1) {
            pdf_begin_obj(static_pdf, k, OBJSTM_NEVER); /* 0 = not an object stream candidate! */
            if (nolength && attr.s != NULL) {
                /* we have a direct copy possible with compressed data */
                pdf_begin_dict(static_pdf);
                pdf_out_block(static_pdf, attr.s, attr.l);
                static_pdf->compress_level = 0;
                static_pdf->stream_deflate = false;
                pdf_end_dict(static_pdf);
            } else {
                pdf_begin_dict(static_pdf);
                if (attr.s != NULL) {
                    pdf_check_space(static_pdf);
                    pdf_out_block(static_pdf, attr.s, attr.l);
                    pdf_set_space(static_pdf);
                }
                if (compress_level > -1)
                    static_pdf->compress_level = compress_level;
                pdf_dict_add_streaminfo(static_pdf);
                pdf_end_dict(static_pdf);
            }
            pdf_begin_stream(static_pdf);
        } else {
            set_obj_obj_is_stream(static_pdf, k);
            set_obj_obj_no_length(static_pdf, k);
            if (compress_level > -1)
                obj_obj_pdfcompresslevel(static_pdf, k) = compress_level;
        }
        if (!lua_isnil(L, -2)) {        /* file-s? string-s? t */
            /* from string */
            lua_pop(L, 1);      /* string-s? t */
            if (!lua_isstring(L, -1)) /* or number */  /* !string-s t */
                luaL_error(L, "pdf.obj(): \"string\" must be string for stream object");
            if (immediate == 1) {
                st.s = lua_tolstring(L, -1, &st.l);     /* string-s t */
                pdf_out_block(static_pdf, st.s, st.l);
            } else
                obj_obj_data(static_pdf, k) = luaL_ref(L, LUA_REGISTRYINDEX);   /* t */
        } else {
            /* from file */
            if (lua_type(L, -1) != LUA_TSTRING)   /* !file-s nil t */
                luaL_error(L, "pdf.obj(): \"file\" name must be string for stream object");
            if (immediate == 1) {
                st.s = lua_tolstring(L, -1, &st.l);     /* file-s nil t */
                buf.s = fread_to_buf(L, st.s, &buf.l);
                pdf_out_block(static_pdf, (const char *) buf.s, buf.l);
                xfree(buf.s);
            } else {
                set_obj_obj_is_file(static_pdf, k);
                obj_obj_data(static_pdf, k) = luaL_ref(L, LUA_REGISTRYINDEX);   /* nil t */
            }
        }
        if (immediate == 1) {
            pdf_end_stream(static_pdf);
            pdf_end_obj(static_pdf);
        }
    }
    static_pdf->compress_level = saved_compress_level;
    return k;
}

static int orig_obj(lua_State * L)
{
    int n, first_arg = 1;
    int k;
    const char *st_s = NULL ;
    n = lua_gettop(L);
    if ((n > 0) && (lua_type(L, 1) == LUA_TNUMBER)) {
        first_arg++;
        k = (int) lua_tointeger(L, 1);
        check_obj_type(static_pdf, obj_type_obj, k);
        if (is_obj_scheduled(static_pdf, k) || obj_data_ptr(static_pdf, k) != 0)
            luaL_error(L, "pdf.obj() object in use");
    } else {
        static_pdf->obj_count++;
        k = pdf_create_obj(static_pdf, obj_type_obj, static_pdf->obj_ptr + 1);
    }
    pdf_last_obj = k;
    obj_data_ptr(static_pdf, k) = pdf_get_mem(static_pdf, pdfmem_obj_size);
    init_obj_obj(static_pdf, k);
    switch (n - first_arg + 1) {
        case 0:
            luaL_error(L, "pdf.obj() needs at least one argument");
            break;
        case 1:
            if (!lua_isstring(L, first_arg)) /* or number */
                luaL_error(L, "pdf.obj() 1st argument must be string");
            break;
        case 2:
        case 3:
            if (lua_type(L, first_arg) != LUA_TSTRING)
                luaL_error(L, "pdf.obj() 1st argument must be string");
            if (!lua_isstring(L, first_arg + 1)) /* or number */
                luaL_error(L, "pdf.obj() 2nd argument must be string");
            st_s = lua_tostring(L, first_arg);
            if (lua_key_eq(st_s, file)) {
                if (n == first_arg + 2)
                    luaL_error(L, "pdf.obj() 3rd argument forbidden in file mode");
                set_obj_obj_is_file(static_pdf, k);
            } else {
                if (n == first_arg + 2) {   /* write attr text */
                    if (!lua_isstring(L, -1)) /* or number */
                        luaL_error(L, "pdf.obj() 3rd argument must be string");
                    obj_obj_stream_attr(static_pdf, k) =
                        luaL_ref(Luas, LUA_REGISTRYINDEX);
                }
                if (lua_key_eq(st_s, stream)) {
                    set_obj_obj_is_stream(static_pdf, k);
                } else if (lua_key_eq(st_s, streamfile)) {
                    set_obj_obj_is_stream(static_pdf, k);
                    set_obj_obj_is_file(static_pdf, k);
                } else
                    luaL_error(L, "pdf.obj() invalid argument");
            }
            break;
        default:
            luaL_error(L, "pdf.obj() allows max. 3 arguments");
    }
    obj_obj_data(static_pdf, k) = luaL_ref(L, LUA_REGISTRYINDEX);
    return k;
}

static int l_obj(lua_State * L)
{
    int k, n;
    ensure_output_state(static_pdf, ST_HEADER_WRITTEN);
    n = lua_gettop(L);
    if (n == 1 && lua_istable(L, 1))
        k = table_obj(L);       /* new */
    else
        k = orig_obj(L);
    lua_pushinteger(L, k);
    return 1;
}

static int l_refobj(lua_State * L)
{
    int k, n;
    n = lua_gettop(L);
    if (n != 1)
        luaL_error(L, "pdf.refobj() needs exactly 1 argument");
    k = (int) luaL_checkinteger(L, 1);
    if (global_shipping_mode == NOT_SHIPPING)
        scan_refobj_lua(static_pdf, k);
    else
        pdf_ref_obj_lua(static_pdf, k);
    return 0;
}

static int l_reserveobj(lua_State * L)
{
    int n;
    const char *st_s = NULL;
    n = lua_gettop(L);
    switch (n) {
        case 0:
            static_pdf->obj_count++;
            pdf_last_obj = pdf_create_obj(static_pdf, obj_type_obj, static_pdf->obj_ptr + 1);
            break;
        case 1:
            if (lua_type(L, -1) != LUA_TSTRING)
                luaL_error(L, "pdf.reserveobj() optional argument must be string");
            st_s = luaL_checkstring(L, 1);
            if (lua_key_eq(st_s, annot)) {
                pdf_last_annot = pdf_create_obj(static_pdf, obj_type_annot, 0);
            } else {
                luaL_error(L, "pdf.reserveobj() optional string must be \"annot\"");
            }
            lua_pop(L, 1);
            break;
        default:
            luaL_error(L, "pdf.reserveobj() allows max. 1 argument");
    }
    lua_pushinteger(L, static_pdf->obj_ptr);
    return 1;
}

static int l_registerannot(lua_State * L)
{
    int n, i;
    n = lua_gettop(L);
    switch (n) {
        case 1:
            if (global_shipping_mode == NOT_SHIPPING)
                luaL_error(L, "pdf.registerannot() can only be used in late lua");
            i = (int) luaL_checkinteger(L, 1);
            if (i <= 0)
                luaL_error(L, "pdf.registerannot() can only register positive object numbers");
            addto_page_resources(static_pdf, obj_type_annot, i);
            break;
        default:
            luaL_error(L, "pdf.registerannot() needs exactly 1 argument");
    }
    return 0;
}

/*

# define valid_pdf_key ( \
    lua_key_eq(s,pageresources) \
    lua_key_eq(s,pageattributes) || \
    lua_key_eq(s,pagesattributes) || \
    lua_key_eq(s,catalog) || \
    lua_key_eq(s,info) || \
    lua_key_eq(s,names) || \
    lua_key_eq(s,trailer) || \
    lua_key_eq(s,xformresources) || \
    lua_key_eq(s,xformattributes) || \
    lua_key_eq(s,trailerid) \
)

*/

#define l_get_pdf_value(key) \
    lua_get_metatablelua(pdf_data); \
    lua_key_rawgeti(key); \
    return 1;

static int l_get_pageresources  (lua_State * L) { l_get_pdf_value(pageresources); }
static int l_get_pageattributes (lua_State * L) { l_get_pdf_value(pageattributes); }
static int l_get_pagesattributes(lua_State * L) { l_get_pdf_value(pagesattributes); }
static int l_get_catalog        (lua_State * L) { l_get_pdf_value(catalog); }
static int l_get_info           (lua_State * L) { l_get_pdf_value(info); }
static int l_get_names          (lua_State * L) { l_get_pdf_value(names); }
static int l_get_trailer        (lua_State * L) { l_get_pdf_value(trailer); }
static int l_get_xformresources (lua_State * L) { l_get_pdf_value(xformresources); }
static int l_get_xformattributes(lua_State * L) { l_get_pdf_value(xformattributes); }
static int l_get_trailerid      (lua_State * L) { l_get_pdf_value(trailerid); }

/*

static int getpdf(lua_State * L)
{
    const char *s ;
    if (lua_gettop(L) != 2) {
        return 0;
    }
    if (lua_type(L,-1) == LUA_TSTRING) {
        s =  lua_tostring(L, -1);
        if (lua_key_eq(s,h)) {
            lua_pushinteger(L, static_pdf->posstruct->pos.h);
            return 1;
        } else if (lua_key_eq(s,v)) {
            lua_pushinteger(L, static_pdf->posstruct->pos.v);
            return 1;
        } else if (valid_pdf_key) {
            lua_get_metatablelua(pdf_data);
            lua_replace(L, -3);
            lua_rawget(L, -2);
            return 1;
        }
    }
    return 0;
}
*/

#define l_set_pdf_value(key) \
    if (lua_type(L,-1) == LUA_TSTRING) { \
        lua_get_metatablelua(pdf_data); \
        lua_rawgeti(L, LUA_REGISTRYINDEX, lua_key_index(key)); \
        lua_pushvalue(L, -3); \
        lua_rawset(L,-3); \
    } \
    return 0;

static int l_set_pageresources  (lua_State * L) { l_set_pdf_value(pageresources); }
static int l_set_pageattributes (lua_State * L) { l_set_pdf_value(pageattributes); }
static int l_set_pagesattributes(lua_State * L) { l_set_pdf_value(pagesattributes); }
static int l_set_catalog        (lua_State * L) { l_set_pdf_value(catalog); }
static int l_set_info           (lua_State * L) { l_set_pdf_value(info); }
static int l_set_names          (lua_State * L) { l_set_pdf_value(names); }
static int l_set_trailer        (lua_State * L) { l_set_pdf_value(trailer); }
static int l_set_xformresources (lua_State * L) { l_set_pdf_value(xformresources); }
static int l_set_xformattributes(lua_State * L) { l_set_pdf_value(xformattributes); }
static int l_set_trailerid      (lua_State * L) { l_set_pdf_value(trailerid); }

static int getpdfobjtype(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int n = (int) lua_tointeger(L, 1);
        if (n > 0 && n <= static_pdf->obj_ptr) {
            lua_pushstring(L, pdf_obj_typenames[obj_type(static_pdf, n)]);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

static int getpdfmaxobjnum(lua_State * L)
{
    lua_pushinteger(L, static_pdf->obj_ptr);
    return 1;
}

static int l_mapfile(lua_State * L)
{
    const char *st;
    if ((lua_type(L, 1) == LUA_TSTRING) && (st = lua_tostring(L, 1)) != NULL) {
        char *s = xstrdup(st);
        process_map_item(s, MAPFILE);
        free(s);
    }
    return 0;
}

static int l_mapline(lua_State * L)
{
    const char *st;
    if ((lua_type(L, 1) == LUA_TSTRING) && (st = lua_tostring(L, 1)) != NULL) {
        char *s = xstrdup(st);
        process_map_item(s, MAPLINE);
        free(s);
    }
    return 0;
}

static int l_getpos(lua_State * L)
{
    lua_pushinteger(L, static_pdf->posstruct->pos.h);
    lua_pushinteger(L, static_pdf->posstruct->pos.v);
    return 2;
}

static int l_gethpos(lua_State * L)
{
    lua_pushinteger(L, static_pdf->posstruct->pos.h);
    return 1;
}

static int l_getvpos(lua_State * L)
{
    lua_pushinteger(L, static_pdf->posstruct->pos.v);
    return 1;
}

static int l_getmatrix(lua_State * L)
{
    if (matrix_stack_used > 0) {
        matrix_entry *m = &matrix_stack[matrix_stack_used - 1];
        lua_pushnumber(L, m->a);
        lua_pushnumber(L, m->b);
        lua_pushnumber(L, m->c);
        lua_pushnumber(L, m->d);
        lua_pushnumber(L, m->e);
        lua_pushnumber(L, m->f);
    } else {
        lua_pushinteger(L, 1);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 1);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
    }
    return 6 ;
}

static int l_hasmatrix(lua_State * L)
{
    lua_pushboolean(L, (matrix_stack_used > 0));
    return 1 ;
}

static int l_get_lastlink(lua_State * L)
{
    lua_pushinteger(L, (pdf_last_link));
    return 1 ;
}

static int l_get_retval(lua_State * L)
{
    lua_pushinteger(L, (pdf_retval));
    return 1 ;
}

static int l_get_lastobj(lua_State * L)
{
    lua_pushinteger(L, (pdf_last_obj));
    return 1 ;
}

static int l_get_lastannot(lua_State * L)
{
    lua_pushinteger(L, (pdf_last_annot));
    return 1 ;
}

/* maybe:

    get_fontname    : set_ff(i)  obj_info(static_pdf, pdf_font_num(ff))
    get_fontobjnum  : set_ff(i)  pdf_font_num(ff)
    get_fontsize    : font_size(i)
    get_xformname   : obj_info(static_pdf, i)

*/

static int l_get_compress_level(lua_State * L)
{
    lua_pushinteger(L, (pdf_compress_level));
    return 1 ;
}

static int l_get_obj_compress_level(lua_State * L)
{
    lua_pushinteger(L, (pdf_obj_compress_level));
    return 1 ;
}

static int l_get_recompress(lua_State * L)
{
    lua_pushinteger(L, (pdf_recompress));
    return 1 ;
}

static int l_set_compress_level(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int c = (int) lua_tointeger(L, 1);
        if (c<0)
            c = 0 ;
        else if (c>9)
            c = 9 ;
        set_pdf_compress_level(c);
    }
    return 0 ;
}

static int l_set_obj_compress_level(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int c = (int) lua_tointeger(L, 1);
        if (c<0)
            c = 0 ;
        else if (c>9)
            c = 9 ;
        set_pdf_obj_compress_level(c);
    }
    return 0 ;
}

    static int l_set_recompress(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int c = (int) lua_tointeger(L, 1);
        if (c<0)
            c = 0 ;
        else if (c>9)
            c = 9 ;
        set_pdf_recompress(c);
    }
    return 0 ;
}

/* fonts */

static int getpdfgentounicode(lua_State * L)
{
    lua_pushinteger(L, (pdf_gen_tounicode));
    return 1 ;
}

static int getpdfomitcidset(lua_State * L)
{
    lua_pushinteger(L, (pdf_omit_cidset));
    return 1 ;
}

static int getpdfomitcharset(lua_State * L)
{
    lua_pushinteger(L, (pdf_omit_charset));
    return 1 ;
}

static int getpdfomitinfodict(lua_State * L)
{
    lua_pushinteger(L, (pdf_omit_infodict));
    return 1 ;
}

static int getpdfomitmediabox(lua_State * L)
{
    lua_pushinteger(L, (pdf_omit_mediabox));
    return 1 ;
}

static int getpdfomitprocset(lua_State * L)
{
    lua_pushinteger(L, (pdf_omit_procset));
    return 1 ;
}

static int getpdfptexprefix(lua_State * L)
{
    lua_pushinteger(L, (pdf_ptex_prefix));
    return 1 ;
}

static int setpdfgentounicode(lua_State * L)
{
    /* ensures that glyph_unicode_tree is not null */
    glyph_unicode_new(); 
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_pdf_gen_tounicode(lua_tointeger(L, 1));
    }
    return 0 ;
}

static int setpdfomitcidset(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_pdf_omit_cidset(lua_tointeger(L, 1));
    }
    return 0 ;
}

static int setpdfomitcharset(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_pdf_omit_charset(lua_tointeger(L, 1));
    }
    return 0 ;
}

static int setpdfomitinfodict(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_pdf_omit_infodict(lua_tointeger(L, 1));
    }
    return 0 ;
}

static int setpdfomitmediabox(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_pdf_omit_mediabox(lua_tointeger(L, 1));
    }
    return 0 ;
}

static int setpdfomitprocset(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_pdf_omit_procset(lua_tointeger(L, 1));
    }
    return 0 ;
}

static int setpdfptexprefix(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_pdf_ptex_prefix(lua_tointeger(L, 1));
    }
    return 0 ;
}

/* for tracing purposes when no pages are flushed */

static int setforcefile(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TBOOLEAN) {
        static_pdf->force_file = lua_toboolean(L,1);
    } else {
        static_pdf->force_file = 0;
    }
    return 0 ;
}

/* accuracy */

static int l_get_decimal_digits(lua_State * L)
{
    lua_pushinteger(L, (pdf_decimal_digits));
    return 1 ;
}

static int l_set_decimal_digits(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int c = (int) lua_tointeger(L, 1);
        if (c<0) {
            c = 0 ;
        }
        set_pdf_decimal_digits(c);
    }
    return 0 ;
}

/* pk */

static int l_get_pk_resolution(lua_State * L)
{
    lua_pushinteger(L, (pdf_pk_resolution));
    lua_pushinteger(L, (pdf_pk_fixed_dpi));
    return 2 ;
}

static int l_set_pk_resolution(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int c = (int) lua_tointeger(L, 1);
        if (c < 72) {
            c = 72 ;
        } else if (c > 8000) {
            c = 8000 ;
        }
        set_pdf_pk_resolution(c);
    }
    if (lua_type(L, 2) == LUA_TNUMBER) {
        set_pdf_pk_fixed_dpi(lua_tointeger(L, 1));
    }
    return 0 ;
}

/* pdf stuff */

static int getpdffontname(lua_State * L)
{
    int c, ff ;
    if (lua_type(L, 1) == LUA_TNUMBER) {
        c = (int) lua_tointeger(L, 1);
     /* pdf_check_vf(c); */
        if (!font_used(c)) {
            pdf_init_font(static_pdf,c);
        }
        set_ff(c);
        lua_pushinteger(L, (obj_info(static_pdf, pdf_font_num(ff))));
    } else {
        lua_pushnil(L);
    }
    return 1 ;
}

static int getpdffontobjnum(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int ff;
        int c = (int) lua_tointeger(L, 1);
     /* pdf_check_vf(c); */
        if (!font_used(c)) {
            pdf_init_font(static_pdf,c);
        }
        set_ff(c);
        lua_pushinteger(L, (pdf_font_num(ff)));
    } else {
        lua_pushnil(L);
    }
    return 1 ;
}

static int getpdffontsize(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int c = (int) lua_tointeger(L, 1);
        lua_pushinteger(L, (font_size(c)));
    } else {
        lua_pushnil(L);
    }
    return 1 ;
}

static int getpdfpageref(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int c = (int) lua_tointeger(L, 1);
        if (c > 0) {
            lua_pushinteger(L, (pdf_get_obj(static_pdf, obj_type_page, c, false)));
            return 1;
        }
    }
    lua_pushnil(L);
    return 1 ;
}

static int getpdfxformname(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int c = (int) lua_tointeger(L, 1);
        check_obj_type(static_pdf, obj_type_xform, c);
        lua_pushinteger(L, (obj_info(static_pdf, c)));
    } else {
        lua_pushnil(L);
    }
    return 1 ;
}

static int getpdfcreationdate(lua_State * L)
{
    initialize_start_time(static_pdf);
    lua_pushstring(L,static_pdf->start_time_str);
    return 1 ;
}

static int getpdfmajorversion(lua_State * L)
{
 /* lua_pushinteger(L,static_pdf->major_version); */
    lua_pushinteger(L,pdf_major_version);
    return 1 ;
}

static int setpdfmajorversion(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int c = (int) lua_tointeger(L, 1);
        if ((c >= 1) && (c <= 2)) {
            static_pdf->major_version = c;
            set_pdf_major_version(c);
        }
    }
    return 0 ;
}

static int getpdfminorversion(lua_State * L)
{
 /* lua_pushinteger(L,static_pdf->minor_version); */
    lua_pushinteger(L,pdf_minor_version);
    return 1 ;
}

static int setpdfminorversion(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int c = (int) lua_tointeger(L, 1);
        if ((c >= 0) && (c <= 9)) {
            static_pdf->minor_version = c;
            set_pdf_minor_version(c);
        }
    }
    return 0 ;
}

static int setpdforigin(lua_State * L)
{
    int h = 0 ;
    int v = 0 ;
    if (lua_type(L, 1) == LUA_TNUMBER) {
        h = (int) lua_roundnumber(L, 1);
        if (lua_type(L, 2) == LUA_TNUMBER) {
            v = (int) lua_roundnumber(L, 2);
        } else {
            v = h;
        }
    }
    set_tex_extension_dimen_register(d_pdf_h_origin,h);
    set_tex_extension_dimen_register(d_pdf_v_origin,v);
    return 0 ;
}

static int getpdforigin(lua_State * L)
{
    lua_pushinteger(L,get_tex_extension_dimen_register(d_pdf_h_origin));
    lua_pushinteger(L,get_tex_extension_dimen_register(d_pdf_v_origin));
    return 2 ;
}

static int setpdfimageresolution(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_tex_extension_count_register(c_pdf_image_resolution,lua_tointeger(L, 1));
    }
    return 0;
}

static int getpdfimageresolution(lua_State * L)
{
    lua_pushinteger(L,get_tex_extension_count_register(c_pdf_image_resolution));
    return 1 ;
}

static int setpdfthreadmargin(lua_State * L) {
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_tex_extension_dimen_register(d_pdf_thread_margin,lua_roundnumber(L, 1));
    }
    return 0;
}

static int setpdfdestmargin(lua_State * L) {
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_tex_extension_dimen_register(d_pdf_dest_margin,lua_roundnumber(L, 1));
    }
    return 0;
}

static int setpdflinkmargin(lua_State * L) {
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_tex_extension_dimen_register(d_pdf_link_margin,lua_roundnumber(L, 1));
    }
    return 0;
}

static int setpdfxformmargin(lua_State * L) {
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_tex_extension_dimen_register(d_pdf_xform_margin,lua_roundnumber(L, 1));
    }
    return 0;
}

static int getpdfthreadmargin(lua_State * L) {
    lua_pushinteger(L,get_tex_extension_dimen_register(d_pdf_thread_margin));
    return 1;
}

static int getpdfdestmargin(lua_State * L) {
    lua_pushinteger(L,get_tex_extension_dimen_register(d_pdf_dest_margin));
    return 1;
}

static int getpdflinkmargin(lua_State * L) {
    lua_pushinteger(L,get_tex_extension_dimen_register(d_pdf_link_margin));
    return 1;
}

static int getpdfxformmargin(lua_State * L) {
    lua_pushinteger(L,get_tex_extension_dimen_register(d_pdf_xform_margin));
    return 1;
}

static int setpdfinclusionerrorlevel(lua_State * L) {
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_tex_extension_count_register(c_pdf_inclusion_errorlevel,lua_tointeger(L, 1));
    }
    return 0;
}

static int setpdfignoreunknownimages(lua_State * L) {
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_tex_extension_count_register(c_pdf_ignore_unknown_images,lua_tointeger(L, 1));
    }
    return 0;
}

static int getpdfinclusionerrorlevel(lua_State * L) {
    lua_pushinteger(L,get_tex_extension_count_register(c_pdf_inclusion_errorlevel));
    return 1;
}

static int getpdfignoreunknownimages(lua_State * L) {
    lua_pushinteger(L,get_tex_extension_count_register(c_pdf_ignore_unknown_images));
    return 1;
}

    static int l_set_suppress_optional_info(lua_State * L) {
    if (lua_type(L, 1) == LUA_TNUMBER) {
        set_tex_extension_count_register(c_pdf_suppress_optional_info,lua_tointeger(L, 1));
    }
    return 0;
}

static int l_get_suppress_optional_info(lua_State * L) {
    lua_pushinteger(L,get_tex_extension_count_register(c_pdf_suppress_optional_info));
    return 1;
}

static int newpdfcolorstack(lua_State * L)
{
    const char *s = NULL;
    const char *l = NULL;
    int literal_mode = 0; /* set_origin */
    boolean page_start = false;
    int id ;
    if (lua_type(L,1) != LUA_TSTRING) {
        luaL_error(L, "pdf.newcolorstack() expects a string as first argument");
    }
    s =	lua_tostring(L, 1);
    if (lua_type(L,2) == LUA_TSTRING) {
        l =	lua_tostring(L, 2);
        if (lua_key_eq(l,origin)) {
            literal_mode = set_origin;
        } else if (lua_key_eq(l,page))  {
            literal_mode = direct_page;
        } else if (lua_key_eq(l,text))  {
            literal_mode = direct_text;
        } else if (lua_key_eq(l,direct)) {
            literal_mode = direct_always;
        } else if (lua_key_eq(l,raw)) {
            literal_mode = direct_raw;
        } else {
            luaL_error(L, "invalid literal mode in pdf.newcolorstack()");
        }
    }
    if (lua_isboolean(L, 3)) {
        page_start = lua_toboolean(L, 3);
    }
    id = newcolorstack(s, literal_mode, page_start);
    lua_pushinteger(L, id);
    return 1 ;
}


static int l_set_font_attributes(lua_State * L)
{
    int f = luaL_checkinteger(L, -2);
    int i ;
    /*char *s;*/
    const char *st;
    if ((lua_type(L,-1) == LUA_TSTRING) && (st = lua_tostring(L, -1)) != NULL) {
        /* is this dup needed? */
        /*s = xstrdup(st);*/
        i = maketexstring(st); /* brrr */
        set_pdf_font_attr(f, i);
        /*free(s);*/
    }
    return 0;
}

static int pdfincludechar(lua_State * L)
{
    int f = lua_tointeger(L, 1);
    if (lua_type(L,2) == LUA_TTABLE) {
        int i, c;
        int n = lua_rawlen(L, 2);
        for (i=1; i<=n; i++) {
            lua_rawgeti(L, 2, i);
            c = lua_tointeger(L, 3);
            pdf_mark_char(f,c);
            lua_pop(L, 1);
        }
    } else {
        int c = lua_tointeger(L, 2);
        pdf_mark_char(f,c);
    }
    return 0;
}

static int pdfincludefont(lua_State * L)
{
    int f = lua_tointeger(L, 1);
    pdf_init_font(static_pdf,f);
    return 0;
}

static int pdfincludeimage(lua_State * L)
{
    /*tex How to check for a valid entry? */
    image_dict *idict = idict_array[lua_tointeger(L,1)];
    int objnum = img_objnum(idict);
    if (img_state(idict) < DICT_OUTIMG) {
        img_state(idict) = DICT_OUTIMG;
    }
    if (! is_obj_written(static_pdf, objnum)) {
        pdf_write_image(static_pdf, objnum);
    }
    lua_pushinteger(L,img_type(idict));
    lua_pushinteger(L,img_xorig(idict));
    lua_pushinteger(L,img_yorig(idict));
    lua_pushinteger(L,img_xsize(idict));
    lua_pushinteger(L,img_ysize(idict));
    lua_pushinteger(L,img_rotation(idict));
    lua_pushinteger(L,objnum);
    if (img_type(idict) == IMG_TYPE_PNG) {
        lua_pushinteger(L,img_group_ref(idict));
    } else {
        lua_pushnil(L);
    }
    return 8;
}

static int getpdfnofobjects(lua_State * L)
{
    int k;
    int written = 0;
    int dropped = 0;
    for (k = 1; k <= static_pdf->obj_ptr; k++) {
        if (is_obj_written(static_pdf, k)) {
            written += 1;
        } else {
            dropped += 1;
        }
    }
    lua_pushinteger(L,written);
    lua_pushinteger(L,dropped);
    return 2;
}

/*tex

    The following option is not official and needs testing anyway. It's a
    prelude a followup where the dependencies are limited.

*/

static int settypeonewidemode(lua_State * L)
{
    t1_wide_mode = lua_tointeger(L,1);
    return 0;
}

/*tex For normal output see |pdflistout.c|: */

static const struct luaL_Reg pdflib[] = {
    { "gethpos", l_gethpos },
    { "getvpos", l_getvpos },
    { "obj", l_obj },
    { "immediateobj", l_immediateobj },
    { "refobj", l_refobj },
    { "registerannot", l_registerannot },
    { "reserveobj", l_reserveobj },
    { "getpos", l_getpos },
    { "getpageref", getpdfpageref },
    { "getmaxobjnum", getpdfmaxobjnum },
    { "print", luapdfprint },
    { "getobjtype", getpdfobjtype },
    { "getmatrix", l_getmatrix },
    { "hasmatrix", l_hasmatrix },
    { "setfontattributes", l_set_font_attributes },
    { "setcatalog", l_set_catalog },
    { "setinfo", l_set_info },
    { "setnames", l_set_names },
    { "settrailer", l_set_trailer },
    { "setpageresources", l_set_pageresources },
    { "setpageattributes", l_set_pageattributes },
    { "setpagesattributes", l_set_pagesattributes },
    { "setxformresources", l_set_xformresources },
    { "setxformattributes", l_set_xformattributes },
    { "settrailerid", l_set_trailerid },
    { "getcatalog", l_get_catalog },
    { "getinfo", l_get_info },
    { "getnames", l_get_names },
    { "gettrailer", l_get_trailer },
    { "getpageresources", l_get_pageresources },
    { "getpageattributes", l_get_pageattributes },
    { "getpagesattributes", l_get_pagesattributes },
    { "getxformresources", l_get_xformresources },
    { "getxformattributes", l_get_xformattributes },
    { "gettrailerid", l_get_trailerid },
    { "getlastlink", l_get_lastlink },
    { "getretval", l_get_retval },
    { "getlastobj", l_get_lastobj },
    { "getlastannot", l_get_lastannot },
    { "getcompresslevel", l_get_compress_level },
    { "getobjcompresslevel", l_get_obj_compress_level },
    { "getrecompress", l_get_recompress },
    { "setcompresslevel", l_set_compress_level },
    { "setobjcompresslevel", l_set_obj_compress_level },
    { "setrecompress", l_set_recompress },
    { "getdecimaldigits", l_get_decimal_digits },
    { "setdecimaldigits", l_set_decimal_digits },
    { "getpkresolution", l_get_pk_resolution },
    { "setpkresolution", l_set_pk_resolution },
    { "getsuppressoptionalinfo", l_get_suppress_optional_info },
    { "setsuppressoptionalinfo", l_set_suppress_optional_info },
    { "getfontname", getpdffontname },
    { "getfontobjnum", getpdffontobjnum },
    { "getfontsize", getpdffontsize },
    { "getxformname", getpdfxformname },
    { "getcreationdate", getpdfcreationdate },
    { "getmajorversion", getpdfmajorversion },
    { "setmajorversion", setpdfmajorversion },
    { "getminorversion", getpdfminorversion },
    { "setminorversion", setpdfminorversion },
    { "newcolorstack", newpdfcolorstack },
    { "setorigin", setpdforigin },
    { "getorigin", getpdforigin },
    { "setimageresolution", setpdfimageresolution },
    { "getimageresolution", getpdfimageresolution },
    { "setthreadmargin", setpdfthreadmargin },
    { "setdestmargin", setpdfdestmargin },
    { "setlinkmargin", setpdflinkmargin },
    { "setxformmargin", setpdfxformmargin },
    { "getthreadmargin", getpdfthreadmargin },
    { "getdestmargin", getpdfdestmargin },
    { "getlinkmargin", getpdflinkmargin },
    { "getxformmargin", getpdfxformmargin },
    { "getinclusionerrorlevel", getpdfinclusionerrorlevel },
    { "getignoreunknownimages", getpdfignoreunknownimages },
    { "getgentounicode", getpdfgentounicode },
    { "getomitcidset", getpdfomitcidset },
    { "getomitcharset", getpdfomitcharset },
    { "getomitinfo", getpdfomitinfodict },
    { "getomitmediabox", getpdfomitmediabox },
    { "getomitprocset", getpdfomitprocset },
    { "getptexprefix", getpdfptexprefix },
    { "setinclusionerrorlevel", setpdfinclusionerrorlevel },
    { "setignoreunknownimages", setpdfignoreunknownimages },
    { "setgentounicode", setpdfgentounicode },
    { "setomitcidset", setpdfomitcidset },
    { "setomitcharset", setpdfomitcharset },
    { "setomitinfo", setpdfomitinfodict },
    { "setomitmediabox", setpdfomitmediabox },
    { "setomitprocset", setpdfomitprocset },
    { "setptexprefix", setpdfptexprefix },
    { "setforcefile", setforcefile },
    { "mapfile", l_mapfile },
    { "mapline", l_mapline },
    { "includechar", pdfincludechar },
    { "includefont", pdfincludefont },
    /* might go, used when sanitizing backend */
    { "includeimage", pdfincludeimage },
    { "getnofobjects", getpdfnofobjects },
    /* for a while */
    { "maxobjnum", getpdfmaxobjnum },
    { "pageref", getpdfpageref },
    { "objtype", getpdfobjtype },
    { "fontname", getpdffontname },
    { "fontobjnum", getpdffontobjnum },
    { "fontsize", getpdffontsize },
    { "xformname", getpdfxformname },
    /* experimental */
    { "settypeonewidemode", settypeonewidemode},
    /* sentinel */
    {NULL, NULL}
};

int luaopen_pdf(lua_State * L)
{
    lua_pushstring(L,"pdf.data");
    lua_newtable(L);
    lua_settable(L,LUA_REGISTRYINDEX);
    /* */
    luaL_openlib(L, "pdf", pdflib, 0);
    /*
    luaL_newmetatable(L, "pdf.meta");
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, getpdf);
    lua_settable(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, setpdf);
    lua_settable(L, -3);
    lua_setmetatable(L, -2);
    */
    return 1;
}
