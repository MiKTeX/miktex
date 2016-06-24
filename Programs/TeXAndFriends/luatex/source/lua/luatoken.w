% luatoken.w
%
% Copyright 2006-2012 Taco Hoekwater <taco@@luatex.org>
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

@ @c
command_item command_names[] = {
    {"relax", relax_cmd, NULL},
    {"left_brace", left_brace_cmd, NULL},
    {"right_brace", right_brace_cmd, NULL},
    {"math_shift", math_shift_cmd, NULL},
    {"tab_mark", tab_mark_cmd, NULL},
    {"car_ret", car_ret_cmd, NULL},
    {"mac_param", mac_param_cmd, NULL},
    {"sup_mark", sup_mark_cmd, NULL},
    {"sub_mark", sub_mark_cmd, NULL},
    {"endv", endv_cmd, NULL},
    {"spacer", spacer_cmd, NULL},
    {"letter", letter_cmd, NULL},
    {"other_char", other_char_cmd, NULL},
    {"par_end", par_end_cmd, NULL},
    {"stop", stop_cmd, NULL},
    {"delim_num", delim_num_cmd, NULL},
    {"char_num", char_num_cmd, NULL},
    {"math_char_num", math_char_num_cmd, NULL},
    {"mark", mark_cmd, NULL},
    {"xray", xray_cmd, NULL},
    {"make_box", make_box_cmd, NULL},
    {"hmove", hmove_cmd, NULL},
    {"vmove", vmove_cmd, NULL},
    {"un_hbox", un_hbox_cmd, NULL},
    {"un_vbox", un_vbox_cmd, NULL},
    {"remove_item", remove_item_cmd, NULL},
    {"hskip", hskip_cmd, NULL},
    {"vskip", vskip_cmd, NULL},
    {"mskip", mskip_cmd, NULL},
    {"kern", kern_cmd, NULL},
    {"mkern", mkern_cmd, NULL},
    {"leader_ship", leader_ship_cmd, NULL},
    {"halign", halign_cmd, NULL},
    {"valign", valign_cmd, NULL},
    {"no_align", no_align_cmd, NULL},
    {"novrule", no_vrule_cmd, NULL},
    {"nohrule", no_hrule_cmd, NULL},
    {"vrule", vrule_cmd, NULL},
    {"hrule", hrule_cmd, NULL},
    {"insert", insert_cmd, NULL},
    {"vadjust", vadjust_cmd, NULL},
    {"ignore_spaces", ignore_spaces_cmd, NULL},
    {"after_assignment", after_assignment_cmd, NULL},
    {"after_group", after_group_cmd, NULL},
    {"break_penalty", break_penalty_cmd, NULL},
    {"start_par", start_par_cmd, NULL},
    {"ital_corr", ital_corr_cmd, NULL},
    {"accent", accent_cmd, NULL},
    {"math_accent", math_accent_cmd, NULL},
    {"discretionary", discretionary_cmd, NULL},
    {"eq_no", eq_no_cmd, NULL},
    {"left_right", left_right_cmd, NULL},
    {"math_comp", math_comp_cmd, NULL},
    {"limit_switch", limit_switch_cmd, NULL},
    {"above", above_cmd, NULL},
    {"math_style", math_style_cmd, NULL},
    {"math_choice", math_choice_cmd, NULL},
    {"non_script", non_script_cmd, NULL},
    {"vcenter", vcenter_cmd, NULL},
    {"case_shift", case_shift_cmd, NULL},
    {"message", message_cmd, NULL},
    {"normal", normal_cmd, NULL},
    {"extension", extension_cmd, NULL},
    {"option", option_cmd, NULL},
    {"in_stream", in_stream_cmd, NULL},
    {"begin_group", begin_group_cmd, NULL},
    {"end_group", end_group_cmd, NULL},
    {"omit", omit_cmd, NULL},
    {"ex_space", ex_space_cmd, NULL},
    {"boundary", boundary_cmd, NULL},
    {"radical", radical_cmd, NULL},
    {"super_sub_script", super_sub_script_cmd, NULL},
    {"math_shift_cs", math_shift_cs_cmd, NULL},
    {"end_cs_name", end_cs_name_cmd, NULL},
    {"char_ghost", char_ghost_cmd, NULL},
    {"assign_local_box", assign_local_box_cmd, NULL},
    {"char_given", char_given_cmd, NULL},
    {"math_given", math_given_cmd, NULL},
    {"xmath_given", xmath_given_cmd, NULL},
    {"last_item", last_item_cmd, NULL},
    {"toks_register", toks_register_cmd, NULL},
    {"assign_toks", assign_toks_cmd, NULL},
    {"assign_int", assign_int_cmd, NULL},
    {"assign_attr", assign_attr_cmd, NULL},
    {"assign_dimen", assign_dimen_cmd, NULL},
    {"assign_glue", assign_glue_cmd, NULL},
    {"assign_mu_glue", assign_mu_glue_cmd, NULL},
    {"assign_font_dimen", assign_font_dimen_cmd, NULL},
    {"assign_font_int", assign_font_int_cmd, NULL},
    {"set_aux", set_aux_cmd, NULL},
    {"set_prev_graf", set_prev_graf_cmd, NULL},
    {"set_page_dimen", set_page_dimen_cmd, NULL},
    {"set_page_int", set_page_int_cmd, NULL},
    {"set_box_dimen", set_box_dimen_cmd, NULL},
    {"set_tex_shape", set_tex_shape_cmd, NULL},
    {"set_etex_shape", set_etex_shape_cmd, NULL},
    {"def_char_code", def_char_code_cmd, NULL},
    {"def_del_code", def_del_code_cmd, NULL},
    {"extdef_math_code", extdef_math_code_cmd, NULL},
    {"extdef_del_code", extdef_del_code_cmd, NULL},
    {"def_family", def_family_cmd, NULL},
    {"set_math_param", set_math_param_cmd, NULL},
    {"set_font", set_font_cmd, NULL},
    {"def_font", def_font_cmd, NULL},
    {"register", register_cmd, NULL},
    {"assign_box_dir", assign_box_dir_cmd, NULL},
    {"assign_dir", assign_dir_cmd, NULL},
    {"advance", advance_cmd, NULL},
    {"multiply", multiply_cmd, NULL},
    {"divide", divide_cmd, NULL},
    {"prefix", prefix_cmd, NULL},
    {"let", let_cmd, NULL},
    {"shorthand_def", shorthand_def_cmd, NULL},
    {"read_to_cs", read_to_cs_cmd, NULL},
    {"def", def_cmd, NULL},
    {"set_box", set_box_cmd, NULL},
    {"hyph_data", hyph_data_cmd, NULL},
    {"set_interaction", set_interaction_cmd, NULL},
    {"letterspace_font", letterspace_font_cmd, NULL},
    {"expand_font",expand_font_cmd, NULL},
    {"copy_font", copy_font_cmd, NULL},
    {"set_font_id", set_font_id_cmd, NULL},
    {"undefined_cs", undefined_cs_cmd, NULL},
    {"expand_after", expand_after_cmd, NULL},
    {"no_expand", no_expand_cmd, NULL},
    {"input", input_cmd, NULL},
    {"if_test", if_test_cmd, NULL},
    {"fi_or_else", fi_or_else_cmd, NULL},
    {"cs_name", cs_name_cmd, NULL},
    {"convert", convert_cmd, NULL},
    {"variable", variable_cmd, NULL},
    {"feedback", feedback_cmd, NULL},
    {"the", the_cmd, NULL},
    {"combinetoks", combine_toks_cmd, NULL},
    {"top_bot_mark", top_bot_mark_cmd, NULL},
    {"call", call_cmd, NULL},
    {"long_call", long_call_cmd, NULL},
    {"outer_call", outer_call_cmd, NULL},
    {"long_outer_call", long_outer_call_cmd, NULL},
    {"end_template", end_template_cmd, NULL},
    {"dont_expand", dont_expand_cmd, NULL},
    {"glue_ref", glue_ref_cmd, NULL},
    {"shape_ref", shape_ref_cmd, NULL},
    {"box_ref", box_ref_cmd, NULL},
    {"data", data_cmd, NULL},
    {NULL, 0, NULL}
};


@ @c
int get_command_id(const char *s)
{
    int i;
    int cmd = -1;
    for (i = 0; command_names[i].cmd_name != NULL; i++) {
        if (strcmp(s, command_names[i].cmd_name) == 0)
            break;
    }
    if (command_names[i].cmd_name != NULL) {
        cmd = i;
    }
    return cmd;
}

/*
static int get_cur_cmd(lua_State * L)
{
    int r = 0;
    size_t len = lua_rawlen(L, -1);
    cur_cs = 0;
    if (len == 3 || len == 2) {
        r = 1;
        lua_rawgeti(L, -1, 1);
        cur_cmd = (int) lua_tointeger(L, -1);
        lua_rawgeti(L, -2, 2);
        cur_chr = (halfword) lua_tointeger(L, -1);
        if (len == 3) {
            lua_rawgeti(L, -3, 3);
            cur_cs = (halfword) lua_tointeger(L, -1);
        }
        lua_pop(L, (int) len);
        if (cur_cs == 0)
            cur_tok = token_val(cur_cmd, cur_chr);
        else
            cur_tok = cs_token_flag + cur_cs;
    }
    return r;
}
*/

@ @c
static int token_from_lua(lua_State * L)
{
    int cmd, chr;
    int cs = 0;
    size_t len = lua_rawlen(L, -1);
    if (len == 3 || len == 2) {
        lua_rawgeti(L, -1, 1);
        cmd = (int) lua_tointeger(L, -1);
        lua_rawgeti(L, -2, 2);
        chr = (int) lua_tointeger(L, -1);
        if (len == 3) {
            lua_rawgeti(L, -3, 3);
            cs = (int) lua_tointeger(L, -1);
        }
        lua_pop(L, (int) len);
        if (cs == 0) {
            return token_val(cmd, chr);
        } else {
            return cs_token_flag + cs;
        }
    }
    return -1;
}

/*
static int get_cur_cs(lua_State * L)
{
    const char *s;
    unsigned j;
    size_t l;
    int cs;
    int save_nncs;
    int ret;
    ret = 0;
    cur_cs = 0;
    lua_getfield(L, -1, "name");
    if (lua_type(L, -1) == LUA_TSTRING) {
        s = lua_tolstring(L, -1, &l);
        if (l > 0) {
            if ((last + (int) l) > buf_size)
                check_buffer_overflow((last + (int) l));
            for (j = 0; j < l; j++) {
                buffer[(unsigned) last + 1 + j] = (packed_ASCII_code) * s++;
            }
            save_nncs = no_new_control_sequence;
            no_new_control_sequence = false;
            cs = id_lookup((last + 1), (int) l);
            cur_tok = cs_token_flag + cs;
            cur_cmd = eq_type(cs);
            cur_chr = equiv(cs);
            no_new_control_sequence = save_nncs;
            ret = 1;
        }
    }
    lua_pop(L, 1);
    return ret;
}
*/

@ @c
void tokenlist_to_lua(lua_State * L, int p)
{
    int cmd, chr, cs;
    int v;
    int i = 1;
    v = p;
    while (v != null && v < (int) fix_mem_end) {
        i++;
        v = token_link(v);
    }
    i = 1;
    lua_createtable(L, i, 0);
    while (p != null && p < (int) fix_mem_end) {
        if (token_info(p) >= cs_token_flag) {
            cs = token_info(p) - cs_token_flag;
            cmd = eq_type(cs);
            chr = equiv(cs);
            make_token_table(L, cmd, chr, cs);
        } else {
            cmd = token_cmd(token_info(p));
            chr = token_chr(token_info(p));
            make_token_table(L, cmd, chr, 0);
        }
        lua_rawseti(L, -2, i++);
        p = token_link(p);
    }
}

@ @c
void tokenlist_to_luastring(lua_State * L, int p)
{
    int l;
    char *s;
    s = tokenlist_to_cstring(p, 1, &l);
    lua_pushlstring(L, s, (size_t) l);
    free(s);
}


@ @c
int tokenlist_from_lua(lua_State * L)
{
    const char *s;
    int tok, t;
    size_t i, j;
    halfword p, q, r;
    r = get_avail();
    token_info(r) = 0;          /* ref count */
    token_link(r) = null;
    p = r;
    t = lua_type(L, -1);
    if (t == LUA_TTABLE) {
        j = lua_rawlen(L, -1);
        if (j > 0) {
            for (i = 1; i <= j; i++) {
                lua_rawgeti(L, -1, (int) i);
                tok = token_from_lua(L);
                if (tok >= 0) {
                    store_new_token(tok);
                }
                lua_pop(L, 1);
            };
        }
        return r;
    } else if (t == LUA_TSTRING) {
        s = lua_tolstring(L, -1, &j);
        for (i = 0; i < j; i++) {
            if (s[i] == 32) {
                tok = token_val(10, s[i]);
            } else {
                int j1 = (int) str2uni((const unsigned char *) (s + i));
                i = i + (size_t) (utf8_size(j1) - 1);
                tok = token_val(12, j1);
            }
            store_new_token(tok);
        }
        return r;
    } else {
        free_avail(r);
        return null;
    }
}

/*

static void do_get_token_lua(int callback_id)
{
    lua_State *L = Luas;
    while (1) {
        if (!get_callback(L, callback_id)) {
            get_next();
            lua_pop(L, 2);
            break;
        }
        if (lua_pcall(L, 0, 1, 0) != 0) {
            tex_error(lua_tostring(L, -1), NULL);
            lua_pop(L, 2);
            break;
        }
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1);
            if (lua_istable(L, -1)) {
                int p, q, r;
                size_t i, j;
                lua_pop(L, 1);
                r = get_avail();
                p = r;
                j = lua_rawlen(L, -1);
                if (j > 0) {
                    for (i = 1; i <= j; i++) {
                        lua_rawgeti(L, -1, (int) i);
                        if (get_cur_cmd(L) || get_cur_cs(L)) {
                            store_new_token(cur_tok);
                        }
                        lua_pop(L, 1);
                    }
                }
                if (p != r) {
                    p = token_link(r);
                    free_avail(r);
                    begin_token_list(p, inserted);
                    cur_input.nofilter_field = true;
                    get_next();
                } else {
                    tex_error("error: illegal or empty token list returned",
                              NULL);
                }
                lua_pop(L, 2);
                break;
            } else {
                lua_pop(L, 1);
                if (get_cur_cmd(L) || get_cur_cs(L)) {
                    lua_pop(L, 2);
                    break;
                } else {
                    lua_pop(L, 2);
                    continue;
                }
            }
        } else {
            lua_pop(L, 2);
        }
    }
    return;
}

*/
