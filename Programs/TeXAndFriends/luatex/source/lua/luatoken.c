/*

luatoken.w

Copyright 2006-2012 Taco Hoekwater <taco@@luatex.org>

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

command_item command_names[] = {
    { relax_cmd,                NULL, 0},
    { left_brace_cmd,           NULL, 0},
    { right_brace_cmd,          NULL, 0},
    { math_shift_cmd,           NULL, 0},
    { tab_mark_cmd,             NULL, 0},
    { car_ret_cmd,              NULL, 0},
    { mac_param_cmd,            NULL, 0},
    { sup_mark_cmd,             NULL, 0},
    { sub_mark_cmd,             NULL, 0},
    { endv_cmd,                 NULL, 0},
    { spacer_cmd,               NULL, 0},
    { letter_cmd,               NULL, 0},
    { other_char_cmd,           NULL, 0},
    { par_end_cmd,              NULL, 0},
    { stop_cmd,                 NULL, 0},
    { delim_num_cmd,            NULL, 0},
    { char_num_cmd,             NULL, 0},
    { math_char_num_cmd,        NULL, 0},
    { mark_cmd,                 NULL, 0},
    { node_cmd,                 NULL, 0},
    { xray_cmd,                 NULL, 0},
    { make_box_cmd,             NULL, 0},
    { hmove_cmd,                NULL, 0},
    { vmove_cmd,                NULL, 0},
    { un_hbox_cmd,              NULL, 0},
    { un_vbox_cmd,              NULL, 0},
    { remove_item_cmd,          NULL, 0},
    { hskip_cmd,                NULL, 0},
    { vskip_cmd,                NULL, 0},
    { mskip_cmd,                NULL, 0},
    { kern_cmd,                 NULL, 0},
    { mkern_cmd,                NULL, 0},
    { leader_ship_cmd,          NULL, 0},
    { halign_cmd,               NULL, 0},
    { valign_cmd,               NULL, 0},
    { no_align_cmd,             NULL, 0},
    { no_vrule_cmd,             NULL, 0},
    { no_hrule_cmd,             NULL, 0},
    { vrule_cmd,                NULL, 0},
    { hrule_cmd,                NULL, 0},
    { insert_cmd,               NULL, 0},
    { vadjust_cmd,              NULL, 0},
    { ignore_spaces_cmd,        NULL, 0},
    { after_assignment_cmd,     NULL, 0},
    { after_group_cmd,          NULL, 0},
    { break_penalty_cmd,        NULL, 0},
    { start_par_cmd,            NULL, 0},
    { ital_corr_cmd,            NULL, 0},
    { accent_cmd,               NULL, 0},
    { math_accent_cmd,          NULL, 0},
    { discretionary_cmd,        NULL, 0},
    { eq_no_cmd,                NULL, 0},
    { left_right_cmd,           NULL, 0},
    { math_comp_cmd,            NULL, 0},
    { limit_switch_cmd,         NULL, 0},
    { above_cmd,                NULL, 0},
    { math_style_cmd,           NULL, 0},
    { math_choice_cmd,          NULL, 0},
    { non_script_cmd,           NULL, 0},
    { vcenter_cmd,              NULL, 0},
    { case_shift_cmd,           NULL, 0},
    { message_cmd,              NULL, 0},
    { normal_cmd,               NULL, 0},
    { extension_cmd,            NULL, 0},
    { option_cmd,               NULL, 0},
    { lua_function_call_cmd,    NULL, 0},
    { lua_bytecode_call_cmd,    NULL, 0},
    { lua_call_cmd,             NULL, 0},
    { in_stream_cmd,            NULL, 0},
    { begin_group_cmd,          NULL, 0},
    { end_group_cmd,            NULL, 0},
    { omit_cmd,                 NULL, 0},
    { ex_space_cmd,             NULL, 0},
    { boundary_cmd,             NULL, 0},
    { radical_cmd,              NULL, 0},
    { super_sub_script_cmd,     NULL, 0},
    { no_super_sub_script_cmd,  NULL, 0},
    { math_shift_cs_cmd,        NULL, 0},
    { end_cs_name_cmd,          NULL, 0},
    { char_ghost_cmd,           NULL, 0},
    { assign_local_box_cmd,     NULL, 0},
    { char_given_cmd,           NULL, 0},
    { math_given_cmd,           NULL, 0},
    { xmath_given_cmd,          NULL, 0},
    { last_item_cmd,            NULL, 0},
    { toks_register_cmd,        NULL, 0},
    { assign_toks_cmd,          NULL, 0},
    { assign_int_cmd,           NULL, 0},
    { assign_attr_cmd,          NULL, 0},
    { assign_dimen_cmd,         NULL, 0},
    { assign_glue_cmd,          NULL, 0},
    { assign_mu_glue_cmd,       NULL, 0},
    { assign_font_dimen_cmd,    NULL, 0},
    { assign_font_int_cmd,      NULL, 0},
    { assign_hang_indent_cmd,   NULL, 0},
    { set_aux_cmd,              NULL, 0},
    { set_prev_graf_cmd,        NULL, 0},
    { set_page_dimen_cmd,       NULL, 0},
    { set_page_int_cmd,         NULL, 0},
    { set_box_dimen_cmd,        NULL, 0},
    { set_tex_shape_cmd,        NULL, 0},
    { set_etex_shape_cmd,       NULL, 0},
    { def_char_code_cmd,        NULL, 0},
    { def_del_code_cmd,         NULL, 0},
    { extdef_math_code_cmd,     NULL, 0},
    { extdef_del_code_cmd,      NULL, 0},
    { def_family_cmd,           NULL, 0},
    { set_math_param_cmd,       NULL, 0},
    { set_font_cmd,             NULL, 0},
    { def_font_cmd,             NULL, 0},
    { register_cmd,             NULL, 0},
    { assign_box_direction_cmd, NULL, 0},
    { assign_box_dir_cmd,       NULL, 0},
    { assign_direction_cmd,     NULL, 0},
    { assign_dir_cmd,           NULL, 0},
    { advance_cmd,              NULL, 0},
    { multiply_cmd,             NULL, 0},
    { divide_cmd,               NULL, 0},
    { prefix_cmd,               NULL, 0},
    { let_cmd,                  NULL, 0},
    { shorthand_def_cmd,        NULL, 0},
    { def_lua_call_cmd,         NULL, 0},
    { read_to_cs_cmd,           NULL, 0},
    { def_cmd,                  NULL, 0},
    { set_box_cmd,              NULL, 0},
    { hyph_data_cmd,            NULL, 0},
    { set_interaction_cmd,      NULL, 0},
    { letterspace_font_cmd,     NULL, 0},
    { expand_font_cmd,          NULL, 0},
    { copy_font_cmd,            NULL, 0},
    { set_font_id_cmd,          NULL, 0},
    { undefined_cs_cmd,         NULL, 0},
    { expand_after_cmd,         NULL, 0},
    { no_expand_cmd,            NULL, 0},
    { input_cmd,                NULL, 0},
    { lua_expandable_call_cmd,  NULL, 0},
    { lua_local_call_cmd,       NULL, 0},
    { if_test_cmd,              NULL, 0},
    { fi_or_else_cmd,           NULL, 0},
    { cs_name_cmd,              NULL, 0},
    { convert_cmd,              NULL, 0},
    { variable_cmd,             NULL, 0},
    { feedback_cmd,             NULL, 0},
    { the_cmd,                  NULL, 0},
    { combine_toks_cmd,         NULL, 0},
    { top_bot_mark_cmd,         NULL, 0},
    { call_cmd,                 NULL, 0},
    { long_call_cmd,            NULL, 0},
    { outer_call_cmd,           NULL, 0},
    { long_outer_call_cmd,      NULL, 0},
    { end_template_cmd,         NULL, 0},
    { dont_expand_cmd,          NULL, 0},
    { glue_ref_cmd,             NULL, 0},
    { shape_ref_cmd,            NULL, 0},
    { box_ref_cmd,              NULL, 0},
    { data_cmd,                 NULL, 0},
    { -1,                       NULL, 0}
};

# define init_token_key(target,n,key) \
    target[n].lua  = luaS_##key##_index; \
    target[n].name = luaS_##key##_ptr;

void l_set_token_data(void)
{
    init_token_key(command_names, relax_cmd,                relax);
    init_token_key(command_names, left_brace_cmd,           left_brace);
    init_token_key(command_names, right_brace_cmd,          right_brace);
    init_token_key(command_names, math_shift_cmd,           math_shift);
    init_token_key(command_names, tab_mark_cmd,             tab_mark);
    init_token_key(command_names, car_ret_cmd,              car_ret);
    init_token_key(command_names, mac_param_cmd,            mac_param);
    init_token_key(command_names, sup_mark_cmd,             sup_mark);
    init_token_key(command_names, sub_mark_cmd,             sub_mark);
    init_token_key(command_names, endv_cmd,                 endv);
    init_token_key(command_names, spacer_cmd,               spacer);
    init_token_key(command_names, letter_cmd,               letter);
    init_token_key(command_names, other_char_cmd,           other_char);
    init_token_key(command_names, par_end_cmd,              par_end);
    init_token_key(command_names, stop_cmd,                 stop);
    init_token_key(command_names, delim_num_cmd,            delim_num);
    init_token_key(command_names, char_num_cmd,             char_num);
    init_token_key(command_names, math_char_num_cmd,        math_char_num);
    init_token_key(command_names, mark_cmd,                 mark);
    init_token_key(command_names, node_cmd,                 node);
    init_token_key(command_names, xray_cmd,                 xray);
    init_token_key(command_names, make_box_cmd,             make_box);
    init_token_key(command_names, hmove_cmd,                hmove);
    init_token_key(command_names, vmove_cmd,                vmove);
    init_token_key(command_names, un_hbox_cmd,              un_hbox);
    init_token_key(command_names, un_vbox_cmd,              un_vbox);
    init_token_key(command_names, remove_item_cmd,          remove_item);
    init_token_key(command_names, hskip_cmd,                hskip);
    init_token_key(command_names, vskip_cmd,                vskip);
    init_token_key(command_names, mskip_cmd,                mskip);
    init_token_key(command_names, kern_cmd,                 kern);
    init_token_key(command_names, mkern_cmd,                mkern);
    init_token_key(command_names, leader_ship_cmd,          leader_ship);
    init_token_key(command_names, halign_cmd,               halign);
    init_token_key(command_names, valign_cmd,               valign);
    init_token_key(command_names, no_align_cmd,             no_align);
    init_token_key(command_names, vrule_cmd,                vrule);
    init_token_key(command_names, hrule_cmd,                hrule);
    init_token_key(command_names, no_vrule_cmd,             novrule);
    init_token_key(command_names, no_hrule_cmd,             nohrule);
    init_token_key(command_names, insert_cmd,               insert);
    init_token_key(command_names, vadjust_cmd,              vadjust);
    init_token_key(command_names, ignore_spaces_cmd,        ignore_spaces);
    init_token_key(command_names, after_assignment_cmd,     after_assignment);
    init_token_key(command_names, after_group_cmd,          after_group);
    init_token_key(command_names, break_penalty_cmd,        break_penalty);
    init_token_key(command_names, start_par_cmd,            start_par);
    init_token_key(command_names, ital_corr_cmd,            ital_corr);
    init_token_key(command_names, accent_cmd,               accent);
    init_token_key(command_names, math_accent_cmd,          math_accent);
    init_token_key(command_names, discretionary_cmd,        discretionary);
    init_token_key(command_names, eq_no_cmd,                eq_no);
    init_token_key(command_names, left_right_cmd,           left_right);
    init_token_key(command_names, math_comp_cmd,            math_comp);
    init_token_key(command_names, limit_switch_cmd,         limit_switch);
    init_token_key(command_names, above_cmd,                above);
    init_token_key(command_names, math_style_cmd,           math_style);
    init_token_key(command_names, math_choice_cmd,          math_choice);
    init_token_key(command_names, non_script_cmd,           non_script);
    init_token_key(command_names, vcenter_cmd,              vcenter);
    init_token_key(command_names, case_shift_cmd,           case_shift);
    init_token_key(command_names, message_cmd,              message);
    init_token_key(command_names, normal_cmd,               normal);
    init_token_key(command_names, extension_cmd,            extension);
    init_token_key(command_names, option_cmd,               option);
    init_token_key(command_names, lua_function_call_cmd,    lua_function_call);
    init_token_key(command_names, lua_bytecode_call_cmd,    lua_bytecode_call);
    init_token_key(command_names, lua_call_cmd,             lua_call);
    init_token_key(command_names, in_stream_cmd,            in_stream);
    init_token_key(command_names, begin_group_cmd,          begin_group);
    init_token_key(command_names, end_group_cmd,            end_group);
    init_token_key(command_names, omit_cmd,                 omit);
    init_token_key(command_names, ex_space_cmd,             ex_space);
    init_token_key(command_names, boundary_cmd,             boundary);
    init_token_key(command_names, radical_cmd,              radical);
    init_token_key(command_names, super_sub_script_cmd,     super_sub_script);
    init_token_key(command_names, no_super_sub_script_cmd,  no_super_sub_script);
    init_token_key(command_names, math_shift_cs_cmd,        math_shift_cs);
    init_token_key(command_names, end_cs_name_cmd,          end_cs_name);
    init_token_key(command_names, char_ghost_cmd,           char_ghost);
    init_token_key(command_names, assign_local_box_cmd,     assign_local_box);
    init_token_key(command_names, char_given_cmd,           char_given);
    init_token_key(command_names, math_given_cmd,           math_given);
    init_token_key(command_names, xmath_given_cmd,          xmath_given);
    init_token_key(command_names, last_item_cmd,            last_item);
    init_token_key(command_names, toks_register_cmd,        toks_register);
    init_token_key(command_names, assign_toks_cmd,          assign_toks);
    init_token_key(command_names, assign_int_cmd,           assign_int);
    init_token_key(command_names, assign_attr_cmd,          assign_attr);
    init_token_key(command_names, assign_dimen_cmd,         assign_dimen);
    init_token_key(command_names, assign_glue_cmd,          assign_glue);
    init_token_key(command_names, assign_mu_glue_cmd,       assign_mu_glue);
    init_token_key(command_names, assign_font_dimen_cmd,    assign_font_dimen);
    init_token_key(command_names, assign_font_int_cmd,      assign_font_int);
    init_token_key(command_names, assign_hang_indent_cmd,   assign_hang_indent);
    init_token_key(command_names, set_aux_cmd,              set_aux);
    init_token_key(command_names, set_prev_graf_cmd,        set_prev_graf);
    init_token_key(command_names, set_page_dimen_cmd,       set_page_dimen);
    init_token_key(command_names, set_page_int_cmd,         set_page_int);
    init_token_key(command_names, set_box_dimen_cmd,        set_box_dimen);
    init_token_key(command_names, set_tex_shape_cmd,        set_tex_shape);
    init_token_key(command_names, set_etex_shape_cmd,       set_etex_shape);
    init_token_key(command_names, def_char_code_cmd,        def_char_code);
    init_token_key(command_names, def_del_code_cmd,         def_del_code);
    init_token_key(command_names, extdef_math_code_cmd,     extdef_math_code);
    init_token_key(command_names, extdef_del_code_cmd,      extdef_del_code);
    init_token_key(command_names, def_family_cmd,           def_family);
    init_token_key(command_names, set_math_param_cmd,       set_math_param);
    init_token_key(command_names, set_font_cmd,             set_font);
    init_token_key(command_names, def_font_cmd,             def_font);
    init_token_key(command_names, def_lua_call_cmd,         def_lua_call);
    init_token_key(command_names, register_cmd,             register);
    init_token_key(command_names, assign_box_direction_cmd, assign_box_direction);
    init_token_key(command_names, assign_box_dir_cmd,       assign_box_dir);
    init_token_key(command_names, assign_direction_cmd,     assign_direction);
    init_token_key(command_names, assign_dir_cmd,           assign_dir);
    init_token_key(command_names, advance_cmd,              advance);
    init_token_key(command_names, multiply_cmd,             multiply);
    init_token_key(command_names, divide_cmd,               divide);
    init_token_key(command_names, prefix_cmd,               prefix);
    init_token_key(command_names, let_cmd,                  let);
    init_token_key(command_names, shorthand_def_cmd,        shorthand_def);
    init_token_key(command_names, read_to_cs_cmd,           read_to_cs);
    init_token_key(command_names, def_cmd,                  def);
    init_token_key(command_names, set_box_cmd,              set_box);
    init_token_key(command_names, hyph_data_cmd,            hyph_data);
    init_token_key(command_names, set_interaction_cmd,      set_interaction);
    init_token_key(command_names, letterspace_font_cmd,     letterspace_font);
    init_token_key(command_names, expand_font_cmd,          expand_font);
    init_token_key(command_names, copy_font_cmd,            copy_font);
    init_token_key(command_names, set_font_id_cmd,          set_font_id);
    init_token_key(command_names, undefined_cs_cmd,         undefined_cs);
    init_token_key(command_names, expand_after_cmd,         expand_after);
    init_token_key(command_names, no_expand_cmd,            no_expand);
    init_token_key(command_names, input_cmd,                input);
    init_token_key(command_names, lua_expandable_call_cmd,  lua_expandable_call);
    init_token_key(command_names, lua_local_call_cmd,       lua_local_call);
    init_token_key(command_names, if_test_cmd,              if_test);
    init_token_key(command_names, fi_or_else_cmd,           fi_or_else);
    init_token_key(command_names, cs_name_cmd,              cs_name);
    init_token_key(command_names, convert_cmd,              convert);
    init_token_key(command_names, variable_cmd,             variable);
    init_token_key(command_names, feedback_cmd,             feedback);
    init_token_key(command_names, the_cmd,                  the);
    init_token_key(command_names, combine_toks_cmd,         combinetoks);
    init_token_key(command_names, top_bot_mark_cmd,         top_bot_mark);
    init_token_key(command_names, call_cmd,                 call);
    init_token_key(command_names, long_call_cmd,            long_call);
    init_token_key(command_names, outer_call_cmd,           outer_call);
    init_token_key(command_names, long_outer_call_cmd,      long_outer_call);
    init_token_key(command_names, end_template_cmd,         end_template);
    init_token_key(command_names, dont_expand_cmd,          dont_expand);
    init_token_key(command_names, glue_ref_cmd,             glue_ref);
    init_token_key(command_names, shape_ref_cmd,            shape_ref);
    init_token_key(command_names, box_ref_cmd,              box_ref);
    init_token_key(command_names, data_cmd,                 data);
}

int get_command_id(const char *s)
{
    int i;
    for (i = 0; command_names[i].id != -1; i++) {
        if (s == command_names[i].name)
            return i;
    }
    return -1;
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

void tokenlist_to_luastring(lua_State * L, int p)
{
    int l;
    char *s;
    s = tokenlist_to_cstring(p, 1, &l);
    lua_pushlstring(L, s, (size_t) l);
    free(s);
}

int tokenlist_from_lua(lua_State * L)
{
    const char *s;
    int tok, t;
    size_t i, j;
    halfword p, q, r;
    r = get_avail();
    token_info(r) = 0;
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
    while (1) {
        if (!get_callback(Luas, callback_id)) {
            get_next();
            lua_pop(Luas, 2);
            break;
        }
        if (lua_pcall(Luas, 0, 1, 0) != 0) {
            tex_error(lua_tostring(Luas, -1), NULL);
            lua_pop(Luas, 2);
            break;
        }
        if (lua_istable(Luas, -1)) {
            lua_rawgeti(Luas, -1, 1);
            if (lua_istable(Luas, -1)) {
                int p, q, r;
                size_t i, j;
                lua_pop(Luas, 1);
                r = get_avail();
                p = r;
                j = lua_rawlen(Luas, -1);
                if (j > 0) {
                    for (i = 1; i <= j; i++) {
                        lua_rawgeti(Luas, -1, (int) i);
                        if (get_cur_cmd(Luas) || get_cur_cs(Luas)) {
                            store_new_token(cur_tok);
                        }
                        lua_pop(Luas, 1);
                    }
                }
                if (p != r) {
                    p = token_link(r);
                    free_avail(r);
                    begin_token_list(p, inserted);
                    cur_input.nofilter_field = true;
                    get_next();
                } else {
                    tex_error("error: illegal or empty token list returned", NULL);
                }
                lua_pop(Luas, 2);
                break;
            } else {
                lua_pop(Luas, 1);
                if (get_cur_cmd(Luas) || get_cur_cs(Luas)) {
                    lua_pop(Luas, 2);
                    break;
                } else {
                    lua_pop(Luas, 2);
                    continue;
                }
            }
        } else {
            lua_pop(Luas, 2);
        }
    }
    return;
}
*/
