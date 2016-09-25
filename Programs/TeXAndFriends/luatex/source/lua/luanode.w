% luanode.w
%
% Copyright 2006-2008 Taco Hoekwater <taco@@luatex.org>
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

/* hh-ls: we make sure that lua never sees prev of head but also that when
nodes are removed or inserted, temp nodes don't interfere */

@ @c

#include "ptexlib.h"
#include "lua/luatex-api.h"

@ @c
void lua_node_filter_s(int filterid, int extrainfo)
{
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
    lua_push_string_by_index(Luas,extrainfo); /* arg 1 */
    if (lua_pcall(Luas, 1, 0, 0) != 0) {
        fprintf(stdout, "error: %s\n", lua_tostring(Luas, -1));
        lua_settop(Luas, s_top);
        error();
        return;
    }
    lua_settop(Luas, s_top);
    return;
}

@ @c
void lua_node_filter(int filterid, int extrainfo, halfword head_node, halfword * tail_node)
{
    halfword start_node, start_done, last_node;
    int s_top = lua_gettop(Luas);
    int callback_id = callback_defined(filterid);
    if (head_node == null || callback_id <= 0) {
        lua_settop(Luas, s_top);
        return;
    }
    /* we start after head */
    start_node = vlink(head_node);
    if (start_node == null || !get_callback(Luas, callback_id)) {
        lua_settop(Luas, s_top);
        return;
    }
    /* we make sure we have no prev */
    alink(start_node) = null ;
    /* the action */
    nodelist_to_lua(Luas, start_node);
    lua_push_group_code(Luas,extrainfo);
    if (lua_pcall(Luas, 2, 1, 0) != 0) {
        fprintf(stdout, "error: %s\n", lua_tostring(Luas, -1));
        lua_settop(Luas, s_top);
        error();
        return;
    }
    /* the result */
    if (lua_isboolean(Luas, -1)) {
        if (lua_toboolean(Luas, -1) != 1) {
            /* discard */
            flush_node_list(start_node);
            vlink(head_node) = null;
        } else {
            /* keep */
        }
    } else {
        /* append to old head */
        start_done = nodelist_from_lua(Luas);
        try_couple_nodes(head_node,start_done);
    }
    /* redundant as we set top anyway */
    lua_pop(Luas, 2);
    /* find tail in order to update tail */
    start_node = vlink(head_node);
    if (start_node != null) {
        /* maybe just always slide (harmless and fast) */
        last_node = vlink(start_node);
        if (fix_node_lists) {
            while (last_node != null) {
                alink(last_node) = start_node;
                start_node = last_node;
                last_node = vlink(start_node);
            }
        } else {
            while (last_node != null) {
                start_node = last_node;
                last_node = vlink(start_node);
            }
        }
        /* we're at the end now */
        *tail_node = start_node;
    } else {
        /* we're already at the end */
        *tail_node = head_node;
    }
    /* clean up */
    lua_settop(Luas, s_top);
    return;
}

@ @c
int lua_linebreak_callback(int is_broken, halfword head_node, halfword * new_head)
{
    int a;
    register halfword *p;
    int ret = 0;                /* failure */
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
    alink(vlink(head_node)) = null ; /* hh-ls */
    nodelist_to_lua(Luas, vlink(head_node));       /* arg 1 */
    lua_pushboolean(Luas, is_broken);      /* arg 2 */
    if (lua_pcall(Luas, 2, 1, 0) != 0) {   /* no arg, 1 result */
        fprintf(stdout, "error: %s\n", lua_tostring(Luas, -1));
        lua_settop(Luas, s_top);
        error();
        return ret;
    }
    p = lua_touserdata(Luas, -1);
    if (p != NULL) {
        a = nodelist_from_lua(Luas);
        try_couple_nodes(*new_head,a);
        ret = 1;
    }
    lua_settop(Luas, s_top);
    return ret;
}

@ @c
int lua_appendtovlist_callback(halfword box, int location, halfword prev_depth, boolean is_mirrored, halfword * result, int * next_depth, boolean * prev_set)
{
    register halfword *p;
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
    if (lua_pcall(Luas, 4, 2, 0) != 0) {
        fprintf(stdout, "error: %s\n", lua_tostring(Luas, -1));
        lua_settop(Luas, s_top);
        error();
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
    lua_settop(Luas, s_top);
    return 1;
}

@ @c
halfword lua_hpack_filter(halfword head_node, scaled size, int pack_type, int extrainfo, int pack_direction, halfword attr)
{
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
    alink(head_node) = null ; /* hh-ls */
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
    if (lua_pcall(Luas, 6, 1, 0) != 0) {
        fprintf(stdout, "error: %s\n", lua_tostring(Luas, -1));
        lua_settop(Luas, s_top);
        error();
        return head_node;
    }
    ret = head_node;
    if (lua_isboolean(Luas, -1)) {
        if (lua_toboolean(Luas, -1) != 1) {
            flush_node_list(head_node);
            ret = null;
        }
    } else {
        ret = nodelist_from_lua(Luas);
    }
    lua_settop(Luas, s_top);
#if 0
    lua_gc(Luas,LUA_GCSTEP, LUA_GC_STEP_SIZE);
#endif
    if (fix_node_lists)
        fix_node_list(ret);
    return ret;
}

@ @c
halfword lua_vpack_filter(halfword head_node, scaled size, int pack_type, scaled maxd,
                 int extrainfo, int pack_direction, halfword attr)
{
    halfword ret;
    int callback_id;
    int s_top = lua_gettop(Luas);
    if (head_node == null) {
        lua_settop(Luas, s_top);
        return head_node;
    }
    if  (extrainfo == 8)  { /* output */
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
    alink(head_node) = null ; /* hh-ls */
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
    if (lua_pcall(Luas, 7, 1, 0) != 0) {
        fprintf(stdout, "error: %s\n", lua_tostring(Luas, -1));
        lua_settop(Luas, s_top);
        error();
        return head_node;
    }
    ret = head_node;
    if (lua_isboolean(Luas, -1)) {
        if (lua_toboolean(Luas, -1) != 1) {
            flush_node_list(head_node);
            ret = null;
        }
    } else {
        ret = nodelist_from_lua(Luas);
    }
    lua_settop(Luas, s_top);
#if 0
    lua_gc(Luas,LUA_GCSTEP, LUA_GC_STEP_SIZE);
#endif
    if (fix_node_lists)
        fix_node_list(ret);
    return ret;
}

@ This is a quick hack to fix etex's \.{\\lastnodetype} now that
  there are many more visible node types. TODO: check the
  eTeX manual for the expected return values.

@c
int visible_last_node_type(int n)
{
    int i = type(n);
    if (i != glyph_node) {
        return get_etex_code(i);
    } else if (is_ligature(n)) {
        return 7; /* old ligature value */
    } else {
        return 0; /* old character value */
    }
}

@ @c
void lua_pdf_literal(PDF pdf, int i)
{
    const char *s = NULL;
    size_t l = 0;
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, i);
    s = lua_tolstring(Luas, -1, &l);
    pdf_out_block(pdf, s, l);
    pdf_out(pdf, 10);           /* |pdf_print_nl| */
    lua_pop(Luas, 1);
}

@ @c
void copy_pdf_literal(pointer r, pointer p)
{
    pdf_literal_type(r) = pdf_literal_type(p);
    pdf_literal_mode(r) = pdf_literal_mode(p);
    if (pdf_literal_type(p) == normal) {
        pdf_literal_data(r) = pdf_literal_data(p);
        add_token_ref(pdf_literal_data(p));
    } else {
        lua_rawgeti(Luas, LUA_REGISTRYINDEX, pdf_literal_data(p));
        pdf_literal_data(r) = luaL_ref(Luas, LUA_REGISTRYINDEX);
    }
}

@ @c
void copy_late_lua(pointer r, pointer p)
{
    late_lua_type(r) = late_lua_type(p);
    if (late_lua_name(p) > 0)
        add_token_ref(late_lua_name(p));
    if (late_lua_type(p) == normal) {
        late_lua_data(r) = late_lua_data(p);
        add_token_ref(late_lua_data(p));
    } else {
        lua_rawgeti(Luas, LUA_REGISTRYINDEX, late_lua_data(p));
        late_lua_data(r) = luaL_ref(Luas, LUA_REGISTRYINDEX);
    }
}

@ @c
void copy_user_lua(pointer r, pointer p)
{
    if (user_node_value(p) != 0) {
        lua_rawgeti(Luas, LUA_REGISTRYINDEX, user_node_value(p));
        user_node_value(r) = luaL_ref(Luas, LUA_REGISTRYINDEX);
    }
}

@ @c
void free_pdf_literal(pointer p)
{
    if (pdf_literal_type(p) == normal) {
        delete_token_ref(pdf_literal_data(p));
    } else {
        luaL_unref(Luas, LUA_REGISTRYINDEX, pdf_literal_data(p));
    }
}

void free_late_lua(pointer p)
{
    if (late_lua_name(p) > 0)
        delete_token_ref(late_lua_name(p));
    if (late_lua_type(p) == normal) {
        delete_token_ref(late_lua_data(p));
    } else {
        luaL_unref(Luas, LUA_REGISTRYINDEX, late_lua_data(p));
    }
}

@ @c
void free_user_lua(pointer p)
{
    if (user_node_value(p) != 0) {
        luaL_unref(Luas, LUA_REGISTRYINDEX, user_node_value(p));
    }
}

@ @c
void show_pdf_literal(pointer p)
{
    tprint_esc("pdfliteral");
    switch (pdf_literal_mode(p)) {
        case set_origin:
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
            confusion("literal2");
            break;
    }
    if (pdf_literal_type(p) == normal) {
        print_mark(pdf_literal_data(p));
    } else {
        lua_rawgeti(Luas, LUA_REGISTRYINDEX, pdf_literal_data(p));
        tprint("\"");
        tprint(lua_tostring(Luas, -1));
        tprint("\"");
        lua_pop(Luas, 1);
    }
}

@ @c
void show_late_lua(pointer p)
{
    tprint_esc("latelua");
    print_int(late_lua_reg(p));
    if (late_lua_type(p) == normal) {
        print_mark(late_lua_data(p));
    } else {
        tprint(" <function ");
        print_int(late_lua_data(p));
        tprint(">");
    }
}
