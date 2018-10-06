/*

luanode.w

Copyright 2006-2008 Taco Hoekwater <taco@@luatex.org>

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
#include "lua/luatex-api.h"

void lua_node_filter_s(int filterid, int extrainfo)
{
    int i;
    int callback_id = callback_defined(filterid);
    int s_top = lua_gettop(Luas);
    if (callback_id <= 0) {
        lua_settop(Luas, s_top);
        return;
    }
    if (!get_callback(Luas, callback_id)) {
        lua_settop(Luas, s_top);
        return;
    }
    lua_push_string_by_index(Luas,extrainfo);
    if ((i=lua_pcall(Luas, 1, 0, 0)) != 0) {
        formatted_warning("node filter","error: %s", lua_tostring(Luas, -1));
        lua_settop(Luas, s_top);
        luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
        return;
    }
    lua_settop(Luas, s_top);
    return;
}

void lua_node_filter(int filterid, int extrainfo, halfword head_node, halfword * tail_node)
{
    int i;
    halfword start_node, start_done, last_node;
    int s_top = lua_gettop(Luas);
    int callback_id = callback_defined(filterid);
    if (head_node == null || callback_id <= 0) {
        lua_settop(Luas, s_top);
        return;
    }
    /*tex We start after head. */
    start_node = vlink(head_node);
    if (start_node == null || !get_callback(Luas, callback_id)) {
        lua_settop(Luas, s_top);
        return;
    }
    /*tex We make sure we have no prev */
    alink(start_node) = null ;
    /*tex the action */
    nodelist_to_lua(Luas, start_node);
    lua_push_group_code(Luas,extrainfo);
    if ((i=lua_pcall(Luas, 2, 1, 0)) != 0) {
        formatted_warning("node filter", "error: %s\n", lua_tostring(Luas, -1));
        lua_settop(Luas, s_top);
        luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
        return;
    }
    /*tex the result */
    if (lua_isboolean(Luas, -1)) {
        if (lua_toboolean(Luas, -1) != 1) {
            /*tex discard */
            flush_node_list(start_node);
            vlink(head_node) = null;
        } else {
            /*tex keep */
        }
    } else {
        /*tex append to old head */
        start_done = nodelist_from_lua(Luas,-1);
        try_couple_nodes(head_node,start_done);
    }
    /*tex redundant as we set top anyway */
    lua_pop(Luas, 2);
    /*tex find tail in order to update tail */
    start_node = vlink(head_node);
    if (start_node != null) {
        /*tex maybe just always slide (harmless and fast) */
        if (fix_node_lists) {
            /*tex slides and returns last node */
            *tail_node = fix_node_list(start_node);
        } else {
            last_node = vlink(start_node);
            while (last_node != null) {
                start_node = last_node;
                last_node = vlink(start_node);
            }
            /*tex we're at the end now */
            *tail_node = start_node;
        }
    } else {
        /*tex we're already at the end */
        *tail_node = head_node;
    }
    /*tex clean up */
    lua_settop(Luas, s_top);
    return;
}

int lua_linebreak_callback(int is_broken, halfword head_node, halfword * new_head)
{
    int a, i;
    register halfword *p;
    int ret = 0;
    int s_top = lua_gettop(Luas);
    int callback_id = callback_defined(linebreak_filter_callback);
    if (head_node == null || vlink(head_node) == null || callback_id <= 0) {
        lua_settop(Luas, s_top);
        return ret;
    }
    if (!get_callback(Luas, callback_id)) {
        lua_settop(Luas, s_top);
        return ret;
    }
    alink(vlink(head_node)) = null ;
    nodelist_to_lua(Luas, vlink(head_node));
    lua_pushboolean(Luas, is_broken);
    if ((i=lua_pcall(Luas, 2, 1, 0)) != 0) {
        formatted_warning("linebreak", "error: %s", lua_tostring(Luas, -1));
        lua_settop(Luas, s_top);
        luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
        return ret;
    }
    lua_settop(Luas, s_top);
    p = lua_touserdata(Luas, -1);
    if (p != NULL) {
        a = nodelist_from_lua(Luas,-1);
        try_couple_nodes(*new_head,a);
        ret = 1;
    }
    return ret;
}

int lua_appendtovlist_callback(halfword box, int location, halfword prev_depth,
    boolean is_mirrored, halfword * result, int * next_depth, boolean * prev_set)
{
    register halfword *p;
    int i;
    int s_top = lua_gettop(Luas);
    int callback_id = callback_defined(append_to_vlist_filter_callback);
    if (box == null || callback_id <= 0) {
        lua_settop(Luas, s_top);
        return 0;
    }
    if (!get_callback(Luas, callback_id)) {
        lua_settop(Luas, s_top);
        return 0;
    }
    nodelist_to_lua(Luas, box);
    lua_push_string_by_index(Luas,location);
    lua_pushinteger(Luas, (int) prev_depth);
    lua_pushboolean(Luas, is_mirrored);
    if ((i=lua_pcall(Luas, 4, 2, 0)) != 0) {
        formatted_warning("append to vlist","error: %s", lua_tostring(Luas, -1));
        lua_settop(Luas, s_top);
        luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
        return 0;
    }
    if (lua_type(Luas,-1) == LUA_TNUMBER) {
        *next_depth = lua_roundnumber(Luas,-1);
        *prev_set = true;
        if (lua_type(Luas, -2) != LUA_TNIL) {
            p = check_isnode(Luas, -2);
            *result = *p;
        }
    } else if (lua_type(Luas, -1) != LUA_TNIL) {
        p = check_isnode(Luas, -1);
        *result = *p;
    }
    return 1;
}

halfword lua_hpack_filter(halfword head_node, scaled size, int pack_type, int extrainfo,
    int pack_direction, halfword attr)
{
    int i;
    halfword ret;
    int s_top = lua_gettop(Luas);
    int callback_id = callback_defined(hpack_filter_callback);
    if (head_node == null || callback_id <= 0) {
        lua_settop(Luas, s_top);
        return head_node;
    }
    if (!get_callback(Luas, callback_id)) {
        lua_settop(Luas, s_top);
        return head_node;
    }
    alink(head_node) = null ;
    nodelist_to_lua(Luas, head_node);
    lua_push_group_code(Luas,extrainfo);
    lua_pushinteger(Luas, size);
    lua_push_pack_type(Luas, pack_type);
    if (pack_direction >= 0) {
        lua_push_dir_par(Luas, pack_direction);
    } else {
        lua_pushnil(Luas);
    }
    if (attr != null) {
        nodelist_to_lua(Luas, attr);
    } else {
        lua_pushnil(Luas);
    }
    if ((i=lua_pcall(Luas, 6, 1, 0)) != 0) {
        formatted_warning("hpack filter", "error: %s\n", lua_tostring(Luas, -1));
        lua_settop(Luas, s_top);
        luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
        return head_node;
    }
    ret = head_node;
    if (lua_isboolean(Luas, -1)) {
        if (lua_toboolean(Luas, -1) != 1) {
            flush_node_list(head_node);
            ret = null;
        }
    } else {
        ret = nodelist_from_lua(Luas,-1);
    }
    lua_settop(Luas, s_top);
    if (fix_node_lists)
        fix_node_list(ret);
    return ret;
}

halfword lua_vpack_filter(halfword head_node, scaled size, int pack_type, scaled maxd,
    int extrainfo, int pack_direction, halfword attr)
{
    halfword ret;
    int i;
    int callback_id;
    int s_top = lua_gettop(Luas);
    if (head_node == null) {
        lua_settop(Luas, s_top);
        return head_node;
    }
    if  (extrainfo == 8)  {
        callback_id = callback_defined(pre_output_filter_callback);
    } else {
        callback_id = callback_defined(vpack_filter_callback);
    }
    if (callback_id <= 0) {
        lua_settop(Luas, s_top);
        return head_node;
    }
    if (!get_callback(Luas, callback_id)) {
        lua_settop(Luas, s_top);
        return head_node;
    }
    alink(head_node) = null ;
    nodelist_to_lua(Luas, head_node);
    lua_push_group_code(Luas, extrainfo);
    lua_pushinteger(Luas, size);
    lua_push_pack_type(Luas, pack_type);
    lua_pushinteger(Luas, maxd);
    if (pack_direction >= 0) {
         lua_push_dir_par(Luas, pack_direction);
    } else {
        lua_pushnil(Luas);
    }
    if (attr != null) {
        nodelist_to_lua(Luas, attr);
    } else {
        lua_pushnil(Luas);
    }
    if ((i=lua_pcall(Luas, 7, 1, 0)) != 0) {
        formatted_warning("vpack filter", "error: %s", lua_tostring(Luas, -1));
        lua_settop(Luas, s_top);
        luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
        return head_node;
    }
    ret = head_node;
    if (lua_isboolean(Luas, -1)) {
        if (lua_toboolean(Luas, -1) != 1) {
            flush_node_list(head_node);
            ret = null;
        }
    } else {
        ret = nodelist_from_lua(Luas,-1);
    }
    lua_settop(Luas, s_top);
    if (fix_node_lists)
        fix_node_list(ret);
    return ret;
}

/*tex

    This is a quick hack to fix \ETEX's \.{\\lastnodetype} now that there are many
    more visible node types.

*/

int visible_last_node_type(int n)
{
    int i = type(n);
    if (i != glyph_node) {
        return get_etex_code(i);
    } else if (is_ligature(n)) {
        /*tex old ligature value */
        return 7;
    } else {
        /*tex old character value */
        return 0;
    }
}

void lua_pdf_literal(PDF pdf, int i, int noline)
{
    const char *s = NULL;
    size_t l = 0;
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, i);
    s = lua_tolstring(Luas, -1, &l);
    if (noline) {
        pdf_check_space(pdf);
        pdf_out_block(pdf, s, l);
        pdf_set_space(pdf);
    } else {
        pdf_out_block(pdf, s, l);
        pdf_out(pdf, 10);
    }
    lua_pop(Luas, 1);
}

void copy_pdf_literal(pointer r, pointer p)
{
    int t = pdf_literal_type(p);
    pdf_literal_type(r) = t;
    pdf_literal_mode(r) = pdf_literal_mode(p);
    if (t == normal) {
        pdf_literal_data(r) = pdf_literal_data(p);
        add_token_ref(pdf_literal_data(p));
    } else if (t == lua_refid_literal) {
        lua_rawgeti(Luas, LUA_REGISTRYINDEX, pdf_literal_data(p));
        pdf_literal_data(r) = luaL_ref(Luas, LUA_REGISTRYINDEX);
    } else {
        /* maybe something user, we don't support a call here but best keep it sane anyway. */
        pdf_literal_data(r) = pdf_literal_data(p);
    }
}

void copy_late_lua(pointer r, pointer p)
{
    int t = late_lua_type(p);
    late_lua_type(r) = t;
    if (late_lua_name(p) > 0)
        add_token_ref(late_lua_name(p));
    if (t == normal) {
        late_lua_data(r) = late_lua_data(p);
        add_token_ref(late_lua_data(p));
    } else if (t == lua_refid_literal) {
        lua_rawgeti(Luas, LUA_REGISTRYINDEX, late_lua_data(p));
        late_lua_data(r) = luaL_ref(Luas, LUA_REGISTRYINDEX);
    }
}

void copy_user_lua(pointer r, pointer p)
{
    if (user_node_value(p) != 0) {
        lua_rawgeti(Luas, LUA_REGISTRYINDEX, user_node_value(p));
        user_node_value(r) = luaL_ref(Luas, LUA_REGISTRYINDEX);
    }
}

void free_pdf_literal(pointer p)
{
    int t = pdf_literal_type(p);
    if (t == normal) {
        delete_token_ref(pdf_literal_data(p));
    } else if (t == lua_refid_literal) {
        luaL_unref(Luas, LUA_REGISTRYINDEX, pdf_literal_data(p));
    }
}

void free_late_lua(pointer p)
{
    int t = late_lua_type(p);
    if (late_lua_name(p) > 0)
        delete_token_ref(late_lua_name(p));
    if (t == normal) {
        delete_token_ref(late_lua_data(p));
    } else if (t == lua_refid_literal) {
        luaL_unref(Luas, LUA_REGISTRYINDEX, late_lua_data(p));
    }
}

void free_user_lua(pointer p)
{
    if (user_node_value(p) != 0) {
        luaL_unref(Luas, LUA_REGISTRYINDEX, user_node_value(p));
    }
}

void show_pdf_literal(pointer p)
{
    int t = pdf_literal_type(p);
    tprint_esc("pdfliteral");
    switch (pdf_literal_mode(p)) {
        case set_origin:
            tprint(" origin");
            break;
        case direct_page:
            tprint(" page");
            break;
        case direct_always:
            tprint(" direct");
            break;
        case direct_raw:
            tprint(" raw");
            break;
        default:
            tprint(" <invalid mode>");
            break;
    }
    if (t == normal) {
        print_mark(pdf_literal_data(p));
    } else if (t == lua_refid_literal) {
        tprint(" <lua data reference ");
        print_int(pdf_literal_data(p));
        tprint(">");
    } else {
        tprint(" <invalid data>");
    }
}

void show_late_lua(pointer p)
{
    int t = late_lua_type(p);
    tprint_esc("latelua");
    print_int(late_lua_reg(p));
    if (t == normal) {
        print_mark(late_lua_data(p));
    } else if (t == lua_refid_literal) {
        tprint(" <function reference ");
        print_int(late_lua_data(p));
        tprint(">");
    } else if (t == lua_refid_call) {
        tprint(" <functioncall reference ");
        print_int(late_lua_data(p));
        tprint(">");
    } else {
        tprint(" <invalid data>");
    }
}
