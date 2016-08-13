% texnodes.w
%
% Copyright 2006-2010 Taco Hoekwater <taco@@luatex.org>
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

/* we can consider less mode sizes: 2 4 6 8 */

@
This module started out using NDEBUG to trigger checking invalid node usage,
something that is needed because users can mess up nodes in Lua. At some point
that code was always enabled so it is now always on but still can be recognized
as additional code. And as the performance hit is close to zero so disabling
makes no sense, not even to make it configureable. There is a little more memory
used but that is neglectable compared to other memory usage.

@c
#define MAX_CHAIN_SIZE   13 /* why not a bit larger */
#define CHECK_NODE_USAGE  1 /* this triggers checking */

memory_word *volatile varmem = NULL;

#ifdef CHECK_NODE_USAGE
    char *varmem_sizes = NULL;
#endif

halfword var_mem_max = 0;
halfword rover = 0;

halfword free_chain[MAX_CHAIN_SIZE] = { null };

static int my_prealloc = 0;

int fix_node_lists = 1; /* used in font and lang */

halfword slow_get_node(int s);  /* defined below */

#define fake_node       100
#define fake_node_size  2
#define fake_node_name "fake"

#define variable_node_size 2

/* core nodes */

const char *node_fields_list[] = {
    "attr", "width", "depth", "height", "dir", "shift", "glue_order", "glue_sign",
    "glue_set", "head", NULL
};
const char *node_fields_rule[] = {
    "attr", "width", "depth", "height", "dir", "index", NULL
};
const char *node_fields_insert[] = {
    "attr", "cost", "depth", "height", "spec", "head", NULL
};
const char *node_fields_mark[] = {
    "attr", "class", "mark", NULL
};
const char *node_fields_adjust[] = {
    "attr", "head", NULL
};
const char *node_fields_disc[] = {
    "attr", "pre", "post", "replace", "penalty", NULL
};
const char *node_fields_math[] = {
    "attr", "surround", "width", "stretch", "shrink", "stretch_order", "shrink_order", NULL
};
const char *node_fields_glue[] = {
    "attr", "leader", "width", "stretch", "shrink", "stretch_order", "shrink_order", NULL
};
const char *node_fields_kern[] = {
    "attr", "kern", "expansion_factor", NULL
};
const char *node_fields_penalty[] = {
    "attr", "penalty", NULL
};
const char *node_fields_unset[] = {
    "attr", "width", "depth", "height", "dir", "shrink", "glue_order",
    "glue_sign", "stretch", "span", "head", NULL
};
const char *node_fields_margin_kern[]  = {
    "attr", "width", "glyph", NULL
};
const char *node_fields_glyph[] = {
    "attr", "char", "font", "lang", "left", "right", "uchyph", "components",
    "xoffset", "yoffset", "width", "height", "depth", "expansion_factor", NULL
};
const char *node_fields_inserting[] = {
    "height", "last_ins_ptr", "best_ins_ptr",
    "width", "stretch", "shrink", "stretch_order", "shrink_order", NULL
};
const char *node_fields_splitup[] = {
    "height", "last_ins_ptr", "best_ins_ptr", "broken_ptr", "broken_ins", NULL
};
const char *node_fields_attribute[] = {
    "number", "value", NULL
};
const char *node_fields_glue_spec[] = {
    "width", "stretch", "shrink", "stretch_order", "shrink_order", NULL
};
const char *node_fields_attribute_list[] = {
    NULL
};
const char *node_fields_local_par[] = {
    "attr", "pen_inter", "pen_broken", "dir", "box_left", "box_left_width",
    "box_right", "box_right_width", NULL
};
const char *node_fields_dir[] = {
    "attr", "dir", "level", NULL
};
const char *node_fields_boundary[] = {
    "attr", "value", NULL
};

/* math nodes */

const char *node_fields_noad[] = {
    "attr", "nucleus", "sub", "sup", NULL
};

const char *node_fields_style[] = {
    "attr", "style", NULL
};
const char *node_fields_choice[] = {
    "attr", "display", "text", "script", "scriptscript", NULL
};
const char *node_fields_radical[] = {
    "attr", "nucleus", "sub", "sup", "left", "degree", "width", "options", NULL
};
const char *node_fields_fraction[] = {
    "attr", "width", "num", "denom", "left", "right", "middle", "options", NULL
};
const char *node_fields_accent[] = {
    "attr", "nucleus", "sub", "sup", "accent", "bot_accent", "top_accent",
    "overlay_accent", "fraction", NULL
};
const char *node_fields_fence[] = {
    "attr", "delim", "italic", "height", "depth", "options", "class", NULL
};
const char *node_fields_math_char[] = {
    "attr", "fam", "char", NULL
};
const char *node_fields_sub_box[] = {
    "attr", "head", NULL
};
const char *node_fields_sub_mlist[] = {
    "attr", "head", NULL
};
const char *node_fields_math_text_char[] = {
    "attr", "fam", "char", NULL
};
const char *node_fields_delim[] = {
    "attr", "small_fam", "small_char", "large_fam", "large_char", NULL
};

/* whatsit nodes */

const char *node_fields_whatsit_open[] = {
    "attr", "stream", "name", "area", "ext", NULL
};
const char *node_fields_whatsit_write[] = {
    "attr", "stream", "data", NULL
};
const char *node_fields_whatsit_close[] = {
    "attr", "stream", NULL
};
const char *node_fields_whatsit_special[] = {
    "attr", "data", NULL
};
const char *node_fields_whatsit_save_pos[] = {
    "attr", NULL
};
const char *node_fields_whatsit_late_lua[] = {
    "attr", "reg", "data", "name", "string", NULL
};
const char *node_fields_whatsit_user_defined[] = {
    "attr", "user_id", "type", "value", NULL
};

/* pdf backend whatsit nodes */

const char *node_fields_whatsit_pdf_literal[] = {
    "attr", "mode", "data", NULL
};
const char *node_fields_whatsit_pdf_refobj[] = {
    "attr", "objnum", NULL
};
const char *node_fields_whatsit_pdf_annot[] = {
    "attr", "width", "depth", "height", "objnum", "data", NULL
};
const char *node_fields_whatsit_pdf_start_link[] = {
    "attr", "width", "depth", "height", "objnum", "link_attr", "action", NULL
};
const char *node_fields_whatsit_pdf_end_link[] = {
    "attr", NULL
};
const char *node_fields_whatsit_pdf_dest[] = {
    "attr", "width", "depth", "height", "named_id", "dest_id", "dest_type",
    "xyz_zoom", "objnum", NULL
};
const char *node_fields_whatsit_pdf_action[] = {
    "action_type", "named_id", "action_id", "file", "new_window", "data", NULL
};
const char *node_fields_whatsit_pdf_thread[] = {
    "attr", "width", "depth", "height",  "named_id", "thread_id", "thread_attr", NULL
};
const char *node_fields_whatsit_pdf_start_thread[] = {
    "attr", "width", "depth", "height", "named_id", "thread_id", "thread_attr", NULL
};
const char *node_fields_whatsit_pdf_end_thread[] = {
    "attr", NULL
};
const char *node_fields_whatsit_pdf_colorstack[] = {
    "attr", "stack", "cmd", "data", NULL
};
const char *node_fields_whatsit_pdf_setmatrix[] = {
    "attr", "data", NULL
};
const char *node_fields_whatsit_pdf_save[] = {
    "attr", NULL
};
const char *node_fields_whatsit_pdf_restore[] = {
    "attr", NULL
};

/* subtypes */

const char *node_subtypes_glue[] = {
    "userskip", "lineskip", "baselineskip", "parskip", "abovedisplayskip", "belowdisplayskip",
    "abovedisplayshortskip", "belowdisplayshortskip", "leftskip", "rightskip", "topskip",
    "splittopskip", "tabskip", "spaceskip", "xspaceskip", "parfillskip",
    "mathskip", "thinmuskip", "medmuskip", "thickmuskip", NULL
};
const char *node_subtypes_mathglue[] = { /* 98+ */
    "conditionalmathskip", "muglue", NULL
};
const char *node_subtypes_leader[] = { /* 100+ */
    "leaders", "cleaders", "xleaders", "gleaders", NULL
};
const char *node_subtypes_fill[] = {
    "stretch", "fi", "fil", "fill", "filll", NULL
};
const char *node_subtypes_boundary[] = {
    "cancel", "user", "protrusion", "word", NULL
};
const char *node_subtypes_penalty[] = {
    "userpenalty", NULL
};
const char *node_subtypes_kern[] = {
    "fontkern", "userkern", "accentkern", "italiccorrection", NULL
};
const char *node_subtypes_rule[] = {
    "normal", "box", "image", "empty", "user", NULL
};
const char *node_subtypes_glyph[] = {
    "character", "glyph", "ligature", "ghost", "left", "right", NULL
};
const char *node_subtypes_disc[] = {
    "discretionary", "explicit", "automatic", "regular", "first", "second", NULL
};
const char *node_subtypes_marginkern[] = {
    "left", "right", NULL
};
const char *node_subtypes_list[] = {
    "unknown", "line", "box", "indent", "alignment", "cell", "equation", "equationnumber", NULL
};
const char *node_subtypes_adjust[] = {
    "normal", "pre", NULL
};
const char *node_subtypes_math[] = {
    "beginmath", "endmath", NULL
};
const char *node_subtypes_noad[] = {
    "ord", "opdisplaylimits", "oplimits", "opnolimits", "bin", "rel", "open", "close",
    "punct", "inner", "under", "over", "vcenter", NULL
};
const char *node_subtypes_radical[] = {
    "radical", "uradical", "uroot", "uunderdelimiter", "uoverdelimiter", "udelimiterunder",
    "udelimiterover", NULL
};
const char *node_subtypes_accent[] = {
    "bothflexible", "fixedtop", "fixedbottom", "fixedboth", NULL,
};
const char *node_subtypes_fence[] = {
    "unset", "left", "middle", "right", NULL
};

node_info node_data[] = { /* the last entry in a row is the etex number */
    { hlist_node,          box_node_size,         node_fields_list,                          "hlist",           1 },
    { vlist_node,          box_node_size,         node_fields_list,                          "vlist",           2 },
    { rule_node,           rule_node_size,        node_fields_rule,                          "rule",            3 },
    { ins_node,            ins_node_size,         node_fields_insert,                        "ins",             4 },
    { mark_node,           mark_node_size,        node_fields_mark,                          "mark",            5 },
    { adjust_node,         adjust_node_size,      node_fields_adjust,                        "adjust",          6 },
    { boundary_node,       boundary_node_size,    node_fields_boundary,                      "boundary",       -1 },
    { disc_node,           disc_node_size,        node_fields_disc,                          "disc",            8 },
    { whatsit_node,        -1,                    NULL,                                      "whatsit",         9 },
    { local_par_node,      local_par_size,        node_fields_local_par,                     "local_par",      -1 },
    { dir_node,            dir_node_size,         node_fields_dir,                           "dir",            -1 },
    { math_node,           math_node_size,        node_fields_math,                          "math",           10 },
    { glue_node,           glue_node_size,        node_fields_glue,                          "glue",           11 },
    { kern_node,           kern_node_size,        node_fields_kern,                          "kern",           12 },
    { penalty_node,        penalty_node_size,     node_fields_penalty,                       "penalty",        13 },
    { unset_node,          box_node_size,         node_fields_unset,                         "unset",          14 },
    { style_node,          style_node_size,       node_fields_style,                         "style",          15 },
    { choice_node,         style_node_size,       node_fields_choice,                        "choice",         15 },
    { simple_noad,         noad_size,             node_fields_noad,                          "noad",           15 },
    { radical_noad,        radical_noad_size,     node_fields_radical,                       "radical",        15 },
    { fraction_noad,       fraction_noad_size,    node_fields_fraction,                      "fraction",       15 },
    { accent_noad,         accent_noad_size,      node_fields_accent,                        "accent",         15 },
    { fence_noad,          fence_noad_size,       node_fields_fence,                         "fence",          15 },
    { math_char_node,      math_kernel_node_size, node_fields_math_char,                     "math_char",      15 },
    { sub_box_node,        math_kernel_node_size, node_fields_sub_box,                       "sub_box",        15 },
    { sub_mlist_node,      math_kernel_node_size, node_fields_sub_mlist,                     "sub_mlist",      15 },
    { math_text_char_node, math_kernel_node_size, node_fields_math_text_char,                "math_text_char", 15 },
    { delim_node,          math_shield_node_size, node_fields_delim,                         "delim",          15 },
    { margin_kern_node,    margin_kern_node_size, node_fields_margin_kern,                   "margin_kern",    -1 },
    { glyph_node,          glyph_node_size,       node_fields_glyph,                         "glyph",           0 },
    { align_record_node,   box_node_size,         NULL,                                      "align_record",   -1 },
    { pseudo_file_node,    pseudo_file_node_size, NULL,                                      "pseudo_file",    -1 },
    { pseudo_line_node,    variable_node_size,    NULL,                                      "pseudo_line",    -1 },
    { inserting_node,      page_ins_node_size,    node_fields_inserting,                     "page_insert",    -1 },
    { split_up_node,       page_ins_node_size,    node_fields_splitup,                       "split_insert",   -1 },
    { expr_node,           expr_node_size,        NULL,                                      "expr_stack",     -1 },
    { nesting_node,        nesting_node_size,     NULL,                                      "nested_list",    -1 },
    { span_node,           span_node_size,        NULL,                                      "span",           -1 },
    { attribute_node,      attribute_node_size,   node_fields_attribute,                     "attribute",      -1 },
    { glue_spec_node,      glue_spec_size,        node_fields_glue_spec,                     "glue_spec",      -1 },
    { attribute_list_node, attribute_node_size,   node_fields_attribute_list,                "attribute_list", -1 },
    { temp_node,           temp_node_size,        NULL,                                      "temp",           -1 },
    { align_stack_node,    align_stack_node_size, NULL,                                      "align_stack",    -1 },
    { movement_node,       movement_node_size,    NULL,                                      "movement_stack", -1 },
    { if_node,             if_node_size,          NULL,                                      "if_stack",       -1 },
    { unhyphenated_node,   active_node_size,      NULL,                                      "unhyphenated",   -1 },
    { hyphenated_node,     active_node_size,      NULL,                                      "hyphenated",     -1 },
    { delta_node,          delta_node_size,       NULL,                                      "delta",          -1 },
    { passive_node,        passive_node_size,     NULL,                                      "passive",        -1 },
    { shape_node,          variable_node_size,    NULL,                                      "shape",          -1 },
    { -1,                 -1,                     NULL,                                      NULL,             -1 },
};

const char *node_subtypes_pdf_destination[] = {
    "xyz", "fit", "fith", "fitv", "fitb", "fitbh", "fitbv", "fitr", NULL
};
const char *node_subtypes_pdf_literal[] = {
    "origin", "page", "direct", NULL
};

node_info whatsit_node_data[] = {
    { open_node,         open_node_size,               node_fields_whatsit_open,             "open",             -1 },
    { write_node,        write_node_size,              node_fields_whatsit_write,            "write",            -1 },
    { close_node,        close_node_size,              node_fields_whatsit_close,            "close",            -1 },
    { special_node,      special_node_size,            node_fields_whatsit_special,          "special",          -1 },
    { fake_node,         fake_node_size,               NULL,                                 fake_node_name,     -1 },
    { fake_node,         fake_node_size,               NULL,                                 fake_node_name,     -1 },
    { save_pos_node,     save_pos_node_size,           node_fields_whatsit_save_pos,         "save_pos",         -1 },
    { late_lua_node,     late_lua_node_size,           node_fields_whatsit_late_lua,         "late_lua",         -1 },
    { user_defined_node, user_defined_node_size,       node_fields_whatsit_user_defined,     "user_defined",     -1 },
    { fake_node,         fake_node_size,               NULL,                                 fake_node_name,     -1 },
    { fake_node,         fake_node_size,               NULL,                                 fake_node_name,     -1 },
    { fake_node,         fake_node_size,               NULL,                                 fake_node_name,     -1 },
    { fake_node,         fake_node_size,               NULL,                                 fake_node_name,     -1 },
    { fake_node,         fake_node_size,               NULL,                                 fake_node_name,     -1 },
    { fake_node,         fake_node_size,               NULL,                                 fake_node_name,     -1 },
    { fake_node,         fake_node_size,               NULL,                                 fake_node_name,     -1 },
    /* here starts the dvi backend section, todo: a separate list  */
    /* nothing for dvi */
    /* here starts the pdf backend section, todo: a separate list  */
    { pdf_literal_node,      write_node_size,          node_fields_whatsit_pdf_literal,      "pdf_literal",      -1 },
    { pdf_refobj_node,       pdf_refobj_node_size,     node_fields_whatsit_pdf_refobj,       "pdf_refobj",       -1 },
    { pdf_annot_node,        pdf_annot_node_size,      node_fields_whatsit_pdf_annot,        "pdf_annot",        -1 },
    { pdf_start_link_node,   pdf_annot_node_size,      node_fields_whatsit_pdf_start_link,   "pdf_start_link",   -1 },
    { pdf_end_link_node,     pdf_end_link_node_size,   node_fields_whatsit_pdf_end_link,     "pdf_end_link",     -1 },
    { pdf_dest_node,         pdf_dest_node_size,       node_fields_whatsit_pdf_dest,         "pdf_dest",         -1 },
    { pdf_action_node,       pdf_action_size,          node_fields_whatsit_pdf_action,       "pdf_action",       -1 },
    { pdf_thread_node,       pdf_thread_node_size,     node_fields_whatsit_pdf_thread,       "pdf_thread",       -1 },
    { pdf_start_thread_node, pdf_thread_node_size,     node_fields_whatsit_pdf_start_thread, "pdf_start_thread", -1 },
    { pdf_end_thread_node,   pdf_end_thread_node_size, node_fields_whatsit_pdf_end_thread,   "pdf_end_thread",   -1 },
    { pdf_thread_data_node,  pdf_thread_node_size,     NULL,                                 "pdf_thread_data",  -1 },
    { pdf_link_data_node,    pdf_annot_node_size,      NULL,                                 "pdf_link_data",    -1 },
    { pdf_colorstack_node,   pdf_colorstack_node_size, node_fields_whatsit_pdf_colorstack,   "pdf_colorstack",   -1 },
    { pdf_setmatrix_node,    pdf_setmatrix_node_size,  node_fields_whatsit_pdf_setmatrix,    "pdf_setmatrix",    -1 },
    { pdf_save_node,         pdf_save_node_size,       node_fields_whatsit_pdf_save,         "pdf_save",         -1 },
    { pdf_restore_node,      pdf_restore_node_size,    node_fields_whatsit_pdf_restore,      "pdf_restore",      -1 },
    /* done */
    { -1,                    -1,                       NULL,                                 NULL,               -1 },
};

#define last_whatsit_node pdf_restore_node

@
When we copy a node list, there are several possibilities: we do the same as a new node,
we copy the entry to the table in properties (a reference), we do a deep copy of a table
in the properties, we create a new table and give it the original one as a metatable.
After some experiments (that also included timing) with these scenarios I decided that a
deep copy made no sense, nor did nilling. In the end both the shallow copy and the metatable
variant were both ok, although the second ons is slower. The most important aspect to keep
in mind is that references to other nodes in properties no longer can be valid for that
copy. We could use two tables (one unique and one shared) or metatables but that only
complicates matters.

When defining a new node, we could already allocate a table but it is rather easy to do
that at the lua end e.g. using a metatable __index method. That way it is under macro
package control.

When deleting a node, we could keep the slot (e.g. setting it to false) but it could make
memory consumption raise unneeded when we have temporary large node lists and after that
only small lists.

So, in the end this is what we ended up with. For the record, I also experimented with the
following:

- copy attributes to the properties so that we have fast access at the lua end: in the end
  the overhead is not compensated by speed and convenience, in fact, attributes are not
  that slow when it comes to accessing them

- a bitset in the node but again the gain compared to attributes is neglectable and it also
  demands a pretty string agreement over what bit represents what, and this is unlikely to
  succeed in the tex community (I could use it for font handling, which is cross package,
  but decided that it doesn't pay off

In case one wonders why properties make sense then, well, it is not so much speed that we
gain, but more convenience: storing all kind of (temporary) data in attributes is no fun and
this mechanism makes sure that properties are cleaned up when a node is freed. Also, the
advantage of a more or less global properties table is that we stay at the lua end. An
alternative is to store a reference in the node itself but that is complicated by the fact
that the register has some limitations (no numeric keys) and we also don't want to mess with
it too much.

@c
int lua_properties_level         = 0 ; /* can be private */
int lua_properties_enabled       = 0 ;
int lua_properties_use_metatable = 0 ;

@
We keep track of nesting so that we don't oveflow the stack, and, what is more
important, don't keep resolving the registry index.

@c
#define lua_properties_push do { \
    if (lua_properties_enabled) { \
        lua_properties_level = lua_properties_level + 1 ; \
        if (lua_properties_level == 1) { \
            lua_get_metatablelua_l(Luas,node_properties); \
        } \
    } \
} while(0)

#define lua_properties_pop do { \
    if (lua_properties_enabled) { \
        if (lua_properties_level == 1) \
            lua_pop(Luas,1); \
        lua_properties_level = lua_properties_level - 1 ; \
    } \
} while(0)

/* No setting is needed: */

#define lua_properties_set(target) do { \
} while(0)

/* Resetting boils down to nilling. */

#define lua_properties_reset(target) do { \
    if (lua_properties_enabled) { \
        if (lua_properties_level == 0) { \
            lua_get_metatablelua_l(Luas,node_properties); \
            lua_pushnil(Luas); \
            lua_rawseti(Luas,-2,target); \
            lua_pop(Luas,1); \
        } else { \
            lua_pushnil(Luas); \
            lua_rawseti(Luas,-2,target); \
        } \
    } \
} while(0)

/*
    For a moment I considered supporting all kind of data types but in practice
    that makes no sense. So we stick to a cheap shallow copy with as option a
    metatable. Btw, a deep copy would look like this:

    static void copy_lua_table(lua_State* L, int index) {
        lua_newtable(L);
        lua_pushnil(L);
        while(lua_next(L, index-1) != 0) {
            lua_pushvalue(L, -2);
            lua_insert(L, -2);
            if (lua_type(L,-1)==LUA_TTABLE)
                copy_lua_table(L,-1);
            lua_settable(L, -4);
        }
        lua_pop(L,1);
    }

    #define lua_properties_copy(target, source) do { \
        if (lua_properties_enabled) { \
            lua_pushinteger(Luas,source); \
            lua_rawget(Luas,-2); \
            if (lua_type(Luas,-1)==LUA_TTABLE) { \
                copy_lua_table(Luas,-1); \
                lua_pushinteger(Luas,target); \
                lua_insert(Luas,-2); \
                lua_rawset(Luas,-3); \
            } else { \
                lua_pop(Luas,1); \
            } \
        } \
    } while(0)

*/

/* isn't there a faster way to metatable? */

/*

#define lua_properties_copy(target,source) do { \
    if (lua_properties_enabled) { \
        if (lua_properties_level == 0) { \
            lua_get_metatablelua_l(Luas,node_properties); \
            lua_rawgeti(Luas,-1,source); \
            if (lua_type(Luas,-1)==LUA_TTABLE) { \
                if (lua_properties_use_metatable) { \
                    lua_newtable(Luas); \
                    lua_insert(Luas,-2); \
                    lua_setfield(Luas,-2,"__index"); \
                    lua_newtable(Luas); \
                    lua_insert(Luas,-2); \
                    lua_setmetatable(Luas,-2); \
                } \
                lua_rawseti(Luas,-2,target); \
            } else { \
                lua_pop(Luas,1); \
            } \
            lua_pop(Luas,1); \
        } else { \
            lua_rawgeti(Luas,-1,source); \
            if (lua_type(Luas,-1)==LUA_TTABLE) { \
                if (lua_properties_use_metatable) { \
                    lua_newtable(Luas); \
                    lua_insert(Luas,-2); \
                    lua_setfield(Luas,-2,"__index"); \
                    lua_newtable(Luas); \
                    lua_insert(Luas,-2); \
                    lua_setmetatable(Luas,-2); \
                } \
                lua_rawseti(Luas,-2,target); \
            } else { \
                lua_pop(Luas,1); \
            } \
        } \
    } \
} while(0)

*/

/*
    A simple testrun on many pages of dumb text shows 1% gain (of course it depends
    on how properties are used but some other tests confirm it).
*/

#define lua_properties_copy(target,source) do { \
    if (lua_properties_enabled) { \
        if (lua_properties_level == 0) { \
            lua_get_metatablelua_l(Luas,node_properties); \
            lua_rawgeti(Luas,-1,source); \
            if (lua_type(Luas,-1)==LUA_TTABLE) { \
                if (lua_properties_use_metatable) { \
                    lua_newtable(Luas); \
                    lua_insert(Luas,-2); \
                    lua_push_string_by_name(Luas,__index); \
                    lua_insert(Luas,-2); \
                    lua_rawset(Luas, -3); \
                    lua_newtable(Luas); \
                    lua_insert(Luas,-2); \
                    lua_setmetatable(Luas,-2); \
                } \
                lua_rawseti(Luas,-2,target); \
            } else { \
                lua_pop(Luas,1); \
            } \
            lua_pop(Luas,1); \
        } else { \
            lua_rawgeti(Luas,-1,source); \
            if (lua_type(Luas,-1)==LUA_TTABLE) { \
                if (lua_properties_use_metatable) { \
                    lua_newtable(Luas); \
                    lua_insert(Luas,-2); \
                    lua_push_string_by_name(Luas,__index); \
                    lua_insert(Luas,-2); \
                    lua_rawset(Luas, -3); \
                    lua_newtable(Luas); \
                    lua_insert(Luas,-2); \
                    lua_setmetatable(Luas,-2); \
                } \
                lua_rawseti(Luas,-2,target); \
            } else { \
                lua_pop(Luas,1); \
            } \
        } \
    } \
} while(0)

/* Here end the property handlers. */

@ @c
int valid_node(halfword p)
{
    if (p > my_prealloc && p < var_mem_max) {
#ifdef CHECK_NODE_USAGE
        if (varmem_sizes[p] > 0) {
            return 1;
        }
#else
        return 1;
#endif
    }
    return 0;
}

@ @c
static int test_count = 1;

#define dorangetest(a,b,c)  do {                                 \
    if (!(b>=0 && b<c)) {                                        \
        fprintf(stdout,"For node p:=%d, 0<=%d<%d (l.%d,r.%d)\n", \
            (int)a, (int)b, (int)c, __LINE__,test_count);        \
        confusion("node range test failed");                     \
    } } while (0)

#define dotest(a,b,c) do {                                     \
    if (b!=c) {                                                \
        fprintf(stdout,"For node p:=%d, %d==%d (l.%d,r.%d)\n", \
            (int)a, (int)b, (int)c, __LINE__,test_count);      \
        confusion("node test failed");                         \
    } } while (0)

#define check_action_ref(a)    { dorangetest(p,a,var_mem_max); }
#define check_attribute_ref(a) { dorangetest(p,a,var_mem_max); }
#define check_token_ref(a) { \
    if (type(p) == whatsit_node) { \
        formatted_error("nodes","fuzzy token cleanup in whatsit node with id %i and subtype %i",type(p),subtype(p)); \
    } else { \
        formatted_error("nodes","fuzzy token cleanup in node with id %i",type(p)); \
    } \
}

#ifdef CHECK_NODE_USAGE

static void check_static_node_mem(void)
{
    dotest(zero_glue, width(zero_glue), 0);
    dotest(zero_glue, type(zero_glue), glue_spec_node);
    dotest(zero_glue, vlink(zero_glue), null);
    dotest(zero_glue, stretch(zero_glue), 0);
    dotest(zero_glue, stretch_order(zero_glue), normal);
    dotest(zero_glue, shrink(zero_glue), 0);
    dotest(zero_glue, shrink_order(zero_glue), normal);

    dotest(sfi_glue, width(sfi_glue), 0);
    dotest(sfi_glue, type(sfi_glue), glue_spec_node);
    dotest(sfi_glue, vlink(sfi_glue), null);
    dotest(sfi_glue, stretch(sfi_glue), 0);
    dotest(sfi_glue, stretch_order(sfi_glue), sfi);
    dotest(sfi_glue, shrink(sfi_glue), 0);
    dotest(sfi_glue, shrink_order(sfi_glue), normal);

    dotest(fil_glue, width(fil_glue), 0);
    dotest(fil_glue, type(fil_glue), glue_spec_node);
    dotest(fil_glue, vlink(fil_glue), null);
    dotest(fil_glue, stretch(fil_glue), unity);
    dotest(fil_glue, stretch_order(fil_glue), fil);
    dotest(fil_glue, shrink(fil_glue), 0);
    dotest(fil_glue, shrink_order(fil_glue), normal);

    dotest(fill_glue, width(fill_glue), 0);
    dotest(fill_glue, type(fill_glue), glue_spec_node);
    dotest(fill_glue, vlink(fill_glue), null);
    dotest(fill_glue, stretch(fill_glue), unity);
    dotest(fill_glue, stretch_order(fill_glue), fill);
    dotest(fill_glue, shrink(fill_glue), 0);
    dotest(fill_glue, shrink_order(fill_glue), normal);

    dotest(ss_glue, width(ss_glue), 0);
    dotest(ss_glue, type(ss_glue), glue_spec_node);
    dotest(ss_glue, vlink(ss_glue), null);
    dotest(ss_glue, stretch(ss_glue), unity);
    dotest(ss_glue, stretch_order(ss_glue), fil);
    dotest(ss_glue, shrink(ss_glue), unity);
    dotest(ss_glue, shrink_order(ss_glue), fil);

    dotest(fil_neg_glue, width(fil_neg_glue), 0);
    dotest(fil_neg_glue, type(fil_neg_glue), glue_spec_node);
    dotest(fil_neg_glue, vlink(fil_neg_glue), null);
    dotest(fil_neg_glue, stretch(fil_neg_glue), -unity);
    dotest(fil_neg_glue, stretch_order(fil_neg_glue), fil);
    dotest(fil_neg_glue, shrink(fil_neg_glue), 0);
    dotest(fil_neg_glue, shrink_order(fil_neg_glue), normal);
}

static void node_mem_dump(halfword p)
{
    halfword r;
    for (r = my_prealloc + 1; r < var_mem_max; r++) {
        if (vlink(r) == p) {
            halfword s = r;
            while (s > my_prealloc && varmem_sizes[s] == 0) {
                s--;
            }
            if (s != null
                && s != my_prealloc
                && s != var_mem_max
                && (r - s) < get_node_size(type(s), subtype(s))
                && alink(s) != p) {
                if (type(s) == disc_node) {
                    fprintf(stdout,"  pointed to from %s node %d (vlink %d, alink %d): ",
                            get_node_name(type(s), subtype(s)), (int) s,
                            (int) vlink(s), (int) alink(s));
                    fprintf(stdout, "pre_break(%d,%d,%d), ",
                            (int) vlink_pre_break(s), (int) tlink(pre_break(s)),
                            (int) alink(pre_break(s)));
                    fprintf(stdout, "post_break(%d,%d,%d), ",
                            (int) vlink_post_break(s),
                            (int) tlink(post_break(s)),
                            (int) alink(post_break(s)));
                    fprintf(stdout, "no_break(%d,%d,%d)",
                            (int) vlink_no_break(s), (int) tlink(no_break(s)),
                            (int) alink(no_break(s)));
                    fprintf(stdout, "\n");
                } else {
                    if (vlink(s) == p
                        || (type(s) == glyph_node && lig_ptr (s) == p)
                        || (type(s) == vlist_node && list_ptr(s) == p)
                        || (type(s) == hlist_node && list_ptr(s) == p)
                        || (type(s) == unset_node && list_ptr(s) == p)
                        || (type(s) == ins_node   && ins_ptr (s) == p)
                        ) {
                        fprintf(stdout,"  pointed to from %s node %d (vlink %d, alink %d): ",
                                get_node_name(type(s), subtype(s)), (int) s,
                                (int) vlink(s), (int) alink(s));
                        if (type(s) == glyph_node) {
                            fprintf(stdout, "lig_ptr(%d)", (int) lig_ptr(s));
                        } else if (type(s) == vlist_node || type(s) == hlist_node) {
                            fprintf(stdout, "list_ptr(%d)", (int) list_ptr(s));
                        }
                        fprintf(stdout, "\n");
                    } else {
                        if ((type(s) != penalty_node) && (type(s) != math_node) && (type(s) != kern_node)) {
                            fprintf(stdout, "  pointed to from %s node %d\n",
                                get_node_name(type(s), subtype(s)), (int) s);
                        }
                    }
                }
            }
        }
    }
}

#endif

static int free_error(halfword p)
{
    if (p > my_prealloc && p < var_mem_max) {
#ifdef CHECK_NODE_USAGE
        int i;
        if (varmem_sizes[p] == 0) {
            check_static_node_mem();
            for (i = (my_prealloc + 1); i < var_mem_max; i++) {
                if (varmem_sizes[i] > 0) {
                    check_node(i);
                }
            }
            test_count++;
            if (type(p) == glyph_node) {
                formatted_error("nodes", "attempt to double-free glyph (%c) node %d, ignored", (int) character(p), (int) p);
            } else {
                formatted_error("nodes", "attempt to double-free %s node %d, ignored", get_node_name(type(p), subtype(p)), (int) p);
            }
            node_mem_dump(p);
            return 1;
        }
#endif
    } else {
        formatted_error("nodes", "attempt to free an impossible node %d", (int) p);
        return 1;
    }
    return 0;
}

@ @c
static int copy_error(halfword p)
{
    if (p >= 0 && p < var_mem_max) {
#ifdef CHECK_NODE_USAGE
        if (p > my_prealloc && varmem_sizes[p] == 0) {
            if (type(p) == glyph_node) {
                formatted_warning("nodes", "attempt to copy free glyph (%c) node %d, ignored", (int) character(p), (int) p);
            } else {
                formatted_warning("nodes", "attempt to copy free %s node %d, ignored", get_node_name(type(p), subtype(p)), (int) p);
            }
            return 1;
        }
#endif
    } else {
        formatted_error("nodes", "attempt to copy an impossible node %d", (int) p);
        return 1;
    }
    return 0;
}

@ @c
halfword new_node(int i, int j)
{
    int s = get_node_size(i, j);
    halfword n = get_node(s);
    /*
        It should be possible to do this memset at |free_node()|.

        Both type() and subtype() will be set below, and vlink() is
        set to null by |get_node()|, so we can do we clearing one
        word less than |s|
    */
    (void) memset((void *) (varmem + n + 1), 0, (sizeof(memory_word) * ((unsigned) s - 1)));
    switch (i) {
        case glyph_node:
            init_lang_data(n);
            break;
        case hlist_node:
        case vlist_node:
            box_dir(n) = -1;
            break;
        case disc_node:
            pre_break(n) = pre_break_head(n);
            type(pre_break(n)) = nesting_node;
            subtype(pre_break(n)) = pre_break_head(0);
            post_break(n) = post_break_head(n);
            type(post_break(n)) = nesting_node;
            subtype(post_break(n)) = post_break_head(0);
            no_break(n) = no_break_head(n);
            type(no_break(n)) = nesting_node;
            subtype(no_break(n)) = no_break_head(0);
            break;
        case rule_node:
            depth(n) = null_flag;
            height(n) = null_flag;
            width(n) = null_flag;
            rule_dir(n) = -1;
            rule_index(n) = 0;
            rule_transform(n) = 0;
            break;
        case whatsit_node:
            if (j == open_node) {
                open_name(n) = get_nullstr();
                open_area(n) = open_name(n);
                open_ext(n) = open_name(n);
            }
            break;
        case unset_node:
            width(n) = null_flag;
            break;
        case pseudo_line_node:
        case shape_node:
            /* this is a trick that makes |pseudo_files| slightly slower,
             but the overall allocation faster then an explicit test
             at the top of |new_node()|.
             */
            if (j>0) {
              free_node(n, variable_node_size);
              n = slow_get_node(j);
              (void) memset((void *) (varmem + n + 1), 0, (sizeof(memory_word) * ((unsigned) j - 1)));
            }
            break;
        default:
            break;
    }
    if (synctex_par) {
        /* handle synctex extension */
        switch (i) {
            case math_node:
                synctex_tag_math(n) = cur_input.synctex_tag_field;
                synctex_line_math(n) = line;
                break;
            case glue_node:
                synctex_tag_glue(n) = cur_input.synctex_tag_field;
                synctex_line_glue(n) = line;
                break;
            case kern_node:
                if (j != 0) {
                    synctex_tag_kern(n) = cur_input.synctex_tag_field;
                    synctex_line_kern(n) = line;
                }
                break;
            case hlist_node:
            case vlist_node:
            case unset_node:
                synctex_tag_box(n) = cur_input.synctex_tag_field;
                synctex_line_box(n) = line;
                break;
            case rule_node:
                synctex_tag_rule(n) = cur_input.synctex_tag_field;
                synctex_line_rule(n) = line;
                break;
        }
    }
    /* take care of attributes */
    if (nodetype_has_attributes(i)) {
        build_attribute_list(n);
        /* lua_properties_set */
    }
    type(n) = (quarterword) i;
    subtype(n) = (quarterword) j;
    return n;
}

halfword raw_glyph_node(void)
{
    register halfword n = get_node(glyph_node_size);
    (void) memset((void *) (varmem + n + 1), 0, (sizeof(memory_word) * (glyph_node_size - 1)));
    type(n) = glyph_node;
    subtype(n) = 0;
    return n;
}

halfword new_glyph_node(void)
{
    register halfword n = get_node(glyph_node_size);
    (void) memset((void *) (varmem + n + 1), 0, (sizeof(memory_word) * (glyph_node_size - 1)));
    type(n) = glyph_node;
    subtype(n) = 0;
    build_attribute_list(n);
    /* lua_properties_set */
    return n;
}

@ makes a duplicate of the node list that starts at |p| and returns a
pointer to the new list

@c
halfword do_copy_node_list(halfword p, halfword end)
{
    halfword q = null; /* previous position in new list */
    halfword h = null; /* head of the list */
    register halfword s ;
    lua_properties_push; /* saves stack and time */
    while (p != end) {
        s = copy_node(p);
        if (h == null) {
            h = s;
        } else {
            couple_nodes(q, s);
        }
        q = s;
        p = vlink(p);
    }
    lua_properties_pop; /* saves stack and time */
    return h;
}

halfword copy_node_list(halfword p)
{
    return do_copy_node_list(p, null);
}

#define copy_sub_list(target,source) do { \
     if (source != null) { \
         s = do_copy_node_list(source, null); \
         target = s; \
     } else { \
         target = null; \
     } \
 } while (0)

#define copy_sub_node(target,source) do { \
    if (source != null) { \
        s = copy_node(source); \
        target = s ; \
    } else { \
        target = null; \
    } \
} while (0)

@ make a dupe of a single node

@c
static void copy_node_wrapup_core(halfword p, halfword r)
{
    halfword s ;
    switch (subtype(p)) {
        case write_node:
        case special_node:
            add_token_ref(write_tokens(p));
            break;
        case late_lua_node:
            copy_late_lua(r, p);
            break;
        case user_defined_node:
            switch (user_node_type(p)) {
            case 'a':
                add_node_attr_ref(user_node_value(p));
                break;
            case 'l':
                copy_user_lua(r, p);
                break;
            case 'n':
                s = copy_node_list(user_node_value(p));
                user_node_value(r) = s;
                break;
            case 's':
                /* |add_string_ref(user_node_value(p));| */
                break;
            case 't':
                add_token_ref(user_node_value(p));
                break;
            }
            break;
        default:
            break ;
    }
}

void copy_node_wrapup_dvi(halfword p, halfword r)
{
}

void copy_node_wrapup_pdf(halfword p, halfword r)
{
    switch(subtype(p)) {
        case pdf_literal_node:
            copy_pdf_literal(r, p);
            break;
        case pdf_colorstack_node:
            if (pdf_colorstack_cmd(p) <= colorstack_data)
                add_token_ref(pdf_colorstack_data(p));
            break;
        case pdf_setmatrix_node:
            add_token_ref(pdf_setmatrix_data(p));
            break;
        case pdf_annot_node:
            add_token_ref(pdf_annot_data(p));
            break;
        case pdf_start_link_node:
            if (pdf_link_attr(r) != null)
                add_token_ref(pdf_link_attr(r));
            add_action_ref(pdf_link_action(r));
            break;
        case pdf_dest_node:
            if (pdf_dest_named_id(p) > 0)
                add_token_ref(pdf_dest_id(p));
            break;
        case pdf_thread_node:
        case pdf_start_thread_node:
            if (pdf_thread_named_id(p) > 0)
                add_token_ref(pdf_thread_id(p));
            if (pdf_thread_attr(p) != null)
                add_token_ref(pdf_thread_attr(p));
            break;
        default:
            break;
    }
}

halfword copy_node(const halfword p)
{
    halfword r;                 /* current node being fabricated for new list */
    halfword w ;                /* whatsit subtype */
    register halfword s;        /* a helper variable for copying into variable mem  */
    register int i;
    if (copy_error(p)) {
        r = new_node(temp_node, 0);
        return r;
    }
    i = get_node_size(type(p), subtype(p));
    r = get_node(i);

    (void) memcpy((void *) (varmem + r), (void *) (varmem + p), (sizeof(memory_word) * (unsigned) i));

    if (synctex_par) {
        /* handle synctex extension */
        switch (type(p)) {
            case math_node:
                synctex_tag_math(r) = cur_input.synctex_tag_field;
                synctex_line_math(r) = line;
                break;
            case kern_node:
                synctex_tag_kern(r) = cur_input.synctex_tag_field;
                synctex_line_kern(r) = line;
                break;
        }
    }
    if (nodetype_has_attributes(type(p))) {
        add_node_attr_ref(node_attr(p));
        alink(r) = null;
        lua_properties_copy(r,p);
    }
    vlink(r) = null;

    switch (type(p)) {
        case glyph_node:
            copy_sub_list(lig_ptr(r),lig_ptr(p)) ;
            break;
        case glue_node:
            copy_sub_list(leader_ptr(r),leader_ptr(p)) ;
            break;
        case hlist_node:
        case vlist_node:
        case unset_node:
            copy_sub_list(list_ptr(r),list_ptr(p)) ;
            break;
        case disc_node:
            pre_break(r) = pre_break_head(r);
            if (vlink_pre_break(p) != null) {
                s = copy_node_list(vlink_pre_break(p));
                alink(s) = pre_break(r);
                tlink_pre_break(r) = tail_of_list(s);
                vlink_pre_break(r) = s;
            } else {
                assert(tlink(pre_break(r)) == null);
            }
            post_break(r) = post_break_head(r);
            if (vlink_post_break(p) != null) {
                s = copy_node_list(vlink_post_break(p));
                alink(s) = post_break(r);
                tlink_post_break(r) = tail_of_list(s);
                vlink_post_break(r) = s;
            } else {
                assert(tlink_post_break(r) == null);
            }
            no_break(r) = no_break_head(r);
            if (vlink(no_break(p)) != null) {
                s = copy_node_list(vlink_no_break(p));
                alink(s) = no_break(r);
                tlink_no_break(r) = tail_of_list(s);
                vlink_no_break(r) = s;
            } else {
                assert(tlink_no_break(r) == null);
            }
            break;
        case math_node:
            break;
        case ins_node:
            copy_sub_list(ins_ptr(r),ins_ptr(p)) ;
            break;
        case margin_kern_node:
            copy_sub_node(margin_char(r),margin_char(p));
            break;
        case mark_node:
            add_token_ref(mark_ptr(p));
            break;
        case adjust_node:
            copy_sub_list(adjust_ptr(r),adjust_ptr(p));
            break;
        case choice_node:
            copy_sub_list(display_mlist(r),display_mlist(p)) ;
            copy_sub_list(text_mlist(r),text_mlist(p)) ;
            copy_sub_list(script_mlist(r),script_mlist(p)) ;
            copy_sub_list(script_script_mlist(r),script_script_mlist(p)) ;
            break;
        case simple_noad:
            copy_sub_list(nucleus(r),nucleus(p)) ;
            copy_sub_list(subscr(r),subscr(p)) ;
            copy_sub_list(supscr(r),supscr(p)) ;
            break;
        case radical_noad:
            copy_sub_list(nucleus(r),nucleus(p)) ;
            copy_sub_list(subscr(r),subscr(p)) ;
            copy_sub_list(supscr(r),supscr(p)) ;
            copy_sub_node(left_delimiter(r),left_delimiter(p)) ;
            copy_sub_list(degree(r),degree(p)) ;
            break;
        case accent_noad:
            copy_sub_list(nucleus(r),nucleus(p)) ;
            copy_sub_list(subscr(r),subscr(p)) ;
            copy_sub_list(supscr(r),supscr(p)) ;
            copy_sub_list(top_accent_chr(r),top_accent_chr(p)) ;
            copy_sub_list(bot_accent_chr(r),bot_accent_chr(p)) ;
            copy_sub_list(overlay_accent_chr(r),overlay_accent_chr(p)) ;
            break;
        case fence_noad:
            copy_sub_node(delimiter(r),delimiter(p)) ;
            break;
        case sub_box_node:
        case sub_mlist_node:
            copy_sub_list(math_list(r),math_list(p)) ;
            break;
        case fraction_noad:
            copy_sub_list(numerator(r),numerator(p)) ;
            copy_sub_list(denominator(r),denominator(p)) ;
            copy_sub_node(left_delimiter(r),left_delimiter(p)) ;
            copy_sub_node(right_delimiter(r),right_delimiter(p)) ;
            break;
        case glue_spec_node:
        case dir_node:
        case local_par_node:
        case boundary_node:
            break;
        case whatsit_node:
            w = subtype(p) ;
            if (w >= backend_first_pdf_whatsit) {
                copy_node_wrapup_pdf(p,r);
            } else if (w >= backend_first_dvi_whatsit) {
                copy_node_wrapup_dvi(p,r);
            } else {
                copy_node_wrapup_core(p,r);
            }
            break;
    }
    return r;
}

/* x */

#define free_sub_list(source) if (source != null) flush_node_list(source);
#define free_sub_node(source) if (source != null) flush_node(source);

@ @c

static void flush_node_wrapup_core(halfword p)
{
    switch (subtype(p)) {
        case open_node:
        case write_node:
        case close_node:
        case save_pos_node:
            break;
        case special_node:
            delete_token_ref(write_tokens(p));
            break;
        case late_lua_node:
            free_late_lua(p);
            break;
        case user_defined_node:
            switch (user_node_type(p)) {
            case 'a':
                delete_attribute_ref(user_node_value(p));
                break;
            case 'd':
                break;
            case 'l':
                free_user_lua(user_node_value(p));
                break;
            case 'n':
                flush_node_list(user_node_value(p));
                break;
            case 's':
                /* |delete_string_ref(user_node_value(p));| *//* if this was mpost .. */
                break;
            case 't':
                delete_token_ref(user_node_value(p));
                break;
            default:
                {
                    const char *hlp[] = {
                        "The type of the value in a user defined whatsit node should be one",
                        "of 'a' (attribute list), 'd' (number), 'n' (node list), 's' (string),",
                        "or 't' (tokenlist). Yours has an unknown type, and therefore I don't",
                        "know how to free the node's value. A memory leak may result.",
                        NULL
                    };
                    tex_error("Unidentified user defined whatsit", hlp);
                }
                break;
            }
            break;
    }
}

void flush_node_wrapup_dvi(halfword p)
{
}

void flush_node_wrapup_pdf(halfword p)
{
    switch(subtype(p)) {
        case pdf_save_node:
        case pdf_restore_node:
        case pdf_refobj_node:
        case pdf_end_link_node:
        case pdf_end_thread_node:
            break;
        case pdf_literal_node:
            free_pdf_literal(p);
            break;
        case pdf_colorstack_node:
            if (pdf_colorstack_cmd(p) <= colorstack_data)
                delete_token_ref(pdf_colorstack_data(p));
            break;
        case pdf_setmatrix_node:
            delete_token_ref(pdf_setmatrix_data(p));
            break;
        case pdf_annot_node:
            delete_token_ref(pdf_annot_data(p));
            break;
        case pdf_link_data_node:
            break;
        case pdf_start_link_node:
            if (pdf_link_attr(p) != null)
                delete_token_ref(pdf_link_attr(p));
            delete_action_ref(pdf_link_action(p));
            break;
        case pdf_dest_node:
            if (pdf_dest_named_id(p) > 0)
                delete_token_ref(pdf_dest_id(p));
            break;
        case pdf_action_node:
            if (pdf_action_type(p) == pdf_action_user) {
                delete_token_ref(pdf_action_tokens(p));
            } else {
                if (pdf_action_file(p) != null)
                    delete_token_ref(pdf_action_file(p));
                if (pdf_action_type(p) == pdf_action_page)
                    delete_token_ref(pdf_action_tokens(p));
                else if (pdf_action_named_id(p) > 0)
                    delete_token_ref(pdf_action_id(p));
            }
            break;
        case pdf_thread_data_node:
            break;
        case pdf_thread_node:
        case pdf_start_thread_node:
            if (pdf_thread_named_id(p) > 0)
                delete_token_ref(pdf_thread_id(p));
            if (pdf_thread_attr(p) != null)
                delete_token_ref(pdf_thread_attr(p));
            break;
    }
}

void flush_node(halfword p)
{
    halfword w;
    if (p == null)              /* legal, but no-op */
        return;
    if (free_error(p))
        return;
    switch (type(p)) {
        case glyph_node:
            free_sub_list(lig_ptr(p));
            break;
        case glue_node:
            free_sub_list(leader_ptr(p));
            break;
        case hlist_node:
        case vlist_node:
        case unset_node:
            free_sub_list(list_ptr(p));
            break;
        case disc_node:
            /* watch the start at temp node hack */
            free_sub_list(vlink(pre_break(p)));
            free_sub_list(vlink(post_break(p)));
            free_sub_list(vlink(no_break(p)));
            break;
        case rule_node:
        case kern_node:
        case penalty_node:
        case math_node:
            break;
        case glue_spec_node:
            /* this allows free-ing of lua-allocated glue specs */
//if (valid_node(p)) {
//    free_node(p, subtype(p));
//}
//            return ;
            break ;
        case dir_node:
        case local_par_node:
        case boundary_node:
            break;
        case whatsit_node:
            w = subtype(p) ;
            if (w >= backend_first_pdf_whatsit) {
                flush_node_wrapup_pdf(p);
            } else if (w >= backend_first_dvi_whatsit) {
                flush_node_wrapup_dvi(p);
            } else {
                flush_node_wrapup_core(p);
            }
            break;
        case ins_node:
            flush_node_list(ins_ptr(p));
            break;
        case margin_kern_node:
            flush_node(margin_char(p));
            break;
        case mark_node:
            delete_token_ref(mark_ptr(p));
            break;
        case adjust_node:
            flush_node_list(adjust_ptr(p));
            break;
        case style_node:           /* nothing to do */
            break;
        case choice_node:
            free_sub_list(display_mlist(p));
            free_sub_list(text_mlist(p));
            free_sub_list(script_mlist(p));
            free_sub_list(script_script_mlist(p));
            break;
        case simple_noad:
            free_sub_list(nucleus(p));
            free_sub_list(subscr(p));
            free_sub_list(supscr(p));
            break;
        case radical_noad:
            free_sub_list(nucleus(p));
            free_sub_list(subscr(p));
            free_sub_list(supscr(p));
            free_sub_node(left_delimiter(p));
            free_sub_list(degree(p));
            break;
        case accent_noad:
            free_sub_list(nucleus(p));
            free_sub_list(subscr(p));
            free_sub_list(supscr(p));
            free_sub_list(top_accent_chr(p));
            free_sub_list(bot_accent_chr(p));
            free_sub_list(overlay_accent_chr(p));
            break;
        case fence_noad:
            free_sub_list(delimiter(p));
            break;
        case delim_node:           /* nothing to do */
        case math_char_node:
        case math_text_char_node:
            break;
        case sub_box_node:
        case sub_mlist_node:
            free_sub_list(math_list(p));
            break;
        case fraction_noad:
            free_sub_list(numerator(p));
            free_sub_list(denominator(p));
            free_sub_node(left_delimiter(p));
            free_sub_node(right_delimiter(p));
            break;
        case pseudo_file_node:
            free_sub_list(pseudo_lines(p));
            break;
        case pseudo_line_node:
        case shape_node:
            free_node(p, subtype(p));
            return;
            break;
        case align_stack_node:
        case span_node:
        case movement_node:
        case if_node:
        case nesting_node:
        case unhyphenated_node:
        case hyphenated_node:
        case delta_node:
        case passive_node:
        case inserting_node:
        case split_up_node:
        case expr_node:
        case attribute_node:
        case attribute_list_node:
        case temp_node:
            break;
        default:
            formatted_error("nodes","flushing weird node type %d", type(p));
            return;
    }
    if (nodetype_has_attributes(type(p))) {
        delete_attribute_ref(node_attr(p));
        lua_properties_reset(p);
    }
    free_node(p, get_node_size(type(p), subtype(p)));
    return;
}

@ @c
void flush_node_list(halfword pp)
{                               /* erase list of nodes starting at |p| */
    register halfword p = pp;
    if (p == null)              /* legal, but no-op */
        return;
    if (free_error(p))
        return;
    lua_properties_push; /* saves stack and time */
    while (p != null) {
        register halfword q = vlink(p);
        flush_node(p);
        p = q;
    }
    lua_properties_pop; /* saves stack and time */
}

@ @c
static void check_node_wrapup_core(halfword p)
{
    switch (subtype(p)) {
        /* frontend code */
        case special_node:
            check_token_ref(write_tokens(p));
            break;
        case user_defined_node:
            switch (user_node_type(p)) {
                case 'a':
                    check_attribute_ref(user_node_value(p));
                    break;
                case 't':
                    check_token_ref(user_node_value(p));
                    break;
                case 'n':
                    dorangetest(p, user_node_value(p), var_mem_max);
                    break;
                case 's':
                case 'd':
                    break;
                default:
                    confusion("unknown user node type");
                    break;
            }
            break;
        case open_node:
        case write_node:
        case close_node:
        case save_pos_node:
            break;
    }
}

void check_node_wrapup_dvi(halfword p)
{
}

void check_node_wrapup_pdf(halfword p)
{
    switch (subtype(p)) {
        case pdf_literal_node:
            if (pdf_literal_type(p) == normal)
                check_token_ref(pdf_literal_data(p));
            break;
        case pdf_colorstack_node:
            if (pdf_colorstack_cmd(p) <= colorstack_data)
                check_token_ref(pdf_colorstack_data(p));
            break;
        case pdf_setmatrix_node:
            check_token_ref(pdf_setmatrix_data(p));
            break;
        case late_lua_node:
            if (late_lua_name(p) > 0)
                check_token_ref(late_lua_name(p));
            if (late_lua_type(p) == normal)
                check_token_ref(late_lua_data(p));
            break;
        case pdf_annot_node:
            check_token_ref(pdf_annot_data(p));
            break;
        case pdf_start_link_node:
            if (pdf_link_attr(p) != null)
                check_token_ref(pdf_link_attr(p));
            check_action_ref(pdf_link_action(p));
            break;
        case pdf_dest_node:
            if (pdf_dest_named_id(p) > 0)
                check_token_ref(pdf_dest_id(p));
            break;
        case pdf_thread_node:
        case pdf_start_thread_node:
            if (pdf_thread_named_id(p) > 0)
                check_token_ref(pdf_thread_id(p));
            if (pdf_thread_attr(p) != null)
                check_token_ref(pdf_thread_attr(p));
            break;
        case pdf_save_node:
        case pdf_restore_node:
        case pdf_refobj_node:
        case pdf_end_link_node:
        case pdf_end_thread_node:
            break;
        default:
            confusion("wrapup pdf nodes");
            break;
    }
}

void check_node(halfword p)
{
    halfword w ;
    switch (type(p)) {
        case glyph_node:
            dorangetest(p, lig_ptr(p), var_mem_max);
            break;
        case glue_node:
            dorangetest(p, leader_ptr(p), var_mem_max);
            break;
        case hlist_node:
        case vlist_node:
        case unset_node:
        case align_record_node:
            dorangetest(p, list_ptr(p), var_mem_max);
            break;
        case ins_node:
            dorangetest(p, ins_ptr(p), var_mem_max);
            break;
        case whatsit_node:
            w = subtype(p) ;
            if (w >= backend_first_pdf_whatsit) {
                check_node_wrapup_pdf(p);
            } else if (w >= backend_first_dvi_whatsit) {
                check_node_wrapup_dvi(p);
            } else {
                check_node_wrapup_core(p);
            }
            break;
        case margin_kern_node:
            check_node(margin_char(p));
            break;
        case math_node:
            break;
        case disc_node:
            dorangetest(p, vlink(pre_break(p)), var_mem_max);
            dorangetest(p, vlink(post_break(p)), var_mem_max);
            dorangetest(p, vlink(no_break(p)), var_mem_max);
            break;
        case adjust_node:
            dorangetest(p, adjust_ptr(p), var_mem_max);
            break;
        case pseudo_file_node:
            dorangetest(p, pseudo_lines(p), var_mem_max);
            break;
        case pseudo_line_node:
        case shape_node:
            break;
        case choice_node:
            dorangetest(p, display_mlist(p), var_mem_max);
            dorangetest(p, text_mlist(p), var_mem_max);
            dorangetest(p, script_mlist(p), var_mem_max);
            dorangetest(p, script_script_mlist(p), var_mem_max);
            break;
        case fraction_noad:
            dorangetest(p, numerator(p), var_mem_max);
            dorangetest(p, denominator(p), var_mem_max);
            dorangetest(p, left_delimiter(p), var_mem_max);
            dorangetest(p, right_delimiter(p), var_mem_max);
            break;
        case simple_noad:
            dorangetest(p, nucleus(p), var_mem_max);
            dorangetest(p, subscr(p), var_mem_max);
            dorangetest(p, supscr(p), var_mem_max);
            break;
        case radical_noad:
            dorangetest(p, nucleus(p), var_mem_max);
            dorangetest(p, subscr(p), var_mem_max);
            dorangetest(p, supscr(p), var_mem_max);
            dorangetest(p, degree(p), var_mem_max);
            dorangetest(p, left_delimiter(p), var_mem_max);
            break;
        case accent_noad:
            dorangetest(p, nucleus(p), var_mem_max);
            dorangetest(p, subscr(p), var_mem_max);
            dorangetest(p, supscr(p), var_mem_max);
            dorangetest(p, top_accent_chr(p), var_mem_max);
            dorangetest(p, bot_accent_chr(p), var_mem_max);
            dorangetest(p, overlay_accent_chr(p), var_mem_max);
            break;
        case fence_noad:
            dorangetest(p, delimiter(p), var_mem_max);
            break;
        /*
        case rule_node:
        case kern_node:
        case penalty_node:
        case mark_node:
        case style_node:
        case attribute_list_node:
        case attribute_node:
        case glue_spec_node:
        case temp_node:
        case align_stack_node:
        case movement_node:
        case if_node:
        case nesting_node:
        case span_node:
        case unhyphenated_node:
        case hyphenated_node:
        case delta_node:
        case passive_node:
        case expr_node:
        case dir_node:
        case boundary_node:
        case local_par_node:
            break;
        default:
            fprintf(stdout, "check_node: type is %d\n", type(p));
        */
    }
}

@ @c
void fix_node_list(halfword head)
{
    halfword p, q;
    if (head == null)
        return;
    p = head;
    q = vlink(p);
    while (q != null) {
        alink(q) = p;
        p = q;
        q = vlink(p);
    }
}

@ @c
halfword get_node(int s)
{
    register halfword r;

    if (s < MAX_CHAIN_SIZE) {
        r = free_chain[s];
        if (r != null) {
            free_chain[s] = vlink(r);
#ifdef CHECK_NODE_USAGE
            varmem_sizes[r] = (char) s;
#endif
            vlink(r) = null;
            var_used += s; /* maintain usage statistics */
            return r;
        }
        /* this is the end of the 'inner loop' */
        return slow_get_node(s);
    } else {
        normal_error("nodes","there is a problem in getting a node, case 1");
        return null;
    }
}

@ @c
void free_node(halfword p, int s)
{
    if (p <= my_prealloc) {
        formatted_error("nodes", "node number %d of type %d should not be freed", (int) p, type(p));
        return;
    }
#ifdef CHECK_NODE_USAGE
    varmem_sizes[p] = 0;
#endif
    if (s < MAX_CHAIN_SIZE) {
        vlink(p) = free_chain[s];
        free_chain[s] = p;
    } else {
        /* todo ? it is perhaps possible to merge this node with an existing rover */
        node_size(p) = s;
        vlink(p) = rover;
        while (vlink(rover) != vlink(p)) {
            rover = vlink(rover);
        }
        vlink(rover) = p;
    }
    /* maintain statistics */
    var_used -= s;
}

@ @c
static void free_node_chain(halfword q, int s)
{
    register halfword p = q;
    while (vlink(p) != null) {
#ifdef CHECK_NODE_USAGE
        varmem_sizes[p] = 0;
#endif
        var_used -= s;
        p = vlink(p);
    }
    var_used -= s;
#ifdef CHECK_NODE_USAGE
    varmem_sizes[p] = 0;
#endif
    vlink(p) = free_chain[s];
    free_chain[s] = q;
}

@ @c
void init_node_mem(int t)
{
    my_prealloc = var_mem_stat_max;

    /*  message ?

        assert(whatsit_node_data[user_defined_node].id == user_defined_node);
        assert(node_data[passive_node].id == passive_node);
    */

    varmem = (memory_word *) realloc((void *) varmem, sizeof(memory_word) * (unsigned) t);
    if (varmem == NULL) {
        overflow("node memory size", (unsigned) var_mem_max);
    }
    memset((void *) (varmem), 0, (unsigned) t * sizeof(memory_word));
#ifdef CHECK_NODE_USAGE
    varmem_sizes = (char *) realloc(varmem_sizes, sizeof(char) * (unsigned) t);
    if (varmem_sizes == NULL) {
        overflow("node memory size", (unsigned) var_mem_max);
    }
    memset((void *) varmem_sizes, 0, sizeof(char) * (unsigned) t);
#endif
    var_mem_max = t;
    rover = var_mem_stat_max + 1;
    vlink(rover) = rover;
    node_size(rover) = (t - rover);
    var_used = 0;
    /* initialize static glue specs */
    width(zero_glue) = 0;
    type(zero_glue) = glue_spec_node;
    vlink(zero_glue) = null;
    stretch(zero_glue) = 0;
    stretch_order(zero_glue) = normal;
    shrink(zero_glue) = 0;
    shrink_order(zero_glue) = normal;
    width(sfi_glue) = 0;
    type(sfi_glue) = glue_spec_node;
    vlink(sfi_glue) = null;
    stretch(sfi_glue) = 0;
    stretch_order(sfi_glue) = sfi;
    shrink(sfi_glue) = 0;
    shrink_order(sfi_glue) = normal;
    width(fil_glue) = 0;
    type(fil_glue) = glue_spec_node;
    vlink(fil_glue) = null;
    stretch(fil_glue) = unity;
    stretch_order(fil_glue) = fil;
    shrink(fil_glue) = 0;
    shrink_order(fil_glue) = normal;
    width(fill_glue) = 0;
    type(fill_glue) = glue_spec_node;
    vlink(fill_glue) = null;
    stretch(fill_glue) = unity;
    stretch_order(fill_glue) = fill;
    shrink(fill_glue) = 0;
    shrink_order(fill_glue) = normal;
    width(ss_glue) = 0;
    type(ss_glue) = glue_spec_node;
    vlink(ss_glue) = null;
    stretch(ss_glue) = unity;
    stretch_order(ss_glue) = fil;
    shrink(ss_glue) = unity;
    shrink_order(ss_glue) = fil;
    width(fil_neg_glue) = 0;
    type(fil_neg_glue) = glue_spec_node;
    vlink(fil_neg_glue) = null;
    stretch(fil_neg_glue) = -unity;
    stretch_order(fil_neg_glue) = fil;
    shrink(fil_neg_glue) = 0;
    shrink_order(fil_neg_glue) = normal;
    /* initialize node list heads */
    vinfo(page_ins_head) = 0;
    type(page_ins_head) = temp_node;
    vlink(page_ins_head) = null;
    alink(page_ins_head) = null;
    vinfo(contrib_head) = 0;
    type(contrib_head) = temp_node;
    vlink(contrib_head) = null;
    alink(contrib_head) = null;
    vinfo(page_head) = 0;
    type(page_head) = temp_node;
    vlink(page_head) = null;
    alink(page_head) = null;
    vinfo(temp_head) = 0;
    type(temp_head) = temp_node;
    vlink(temp_head) = null;
    alink(temp_head) = null;
    vinfo(hold_head) = 0;
    type(hold_head) = temp_node;
    vlink(hold_head) = null;
    alink(hold_head) = null;
    vinfo(adjust_head) = 0;
    type(adjust_head) = temp_node;
    vlink(adjust_head) = null;
    alink(adjust_head) = null;
    vinfo(pre_adjust_head) = 0;
    type(pre_adjust_head) = temp_node;
    vlink(pre_adjust_head) = null;
    alink(pre_adjust_head) = null;
    vinfo(active) = 0;
    type(active) = unhyphenated_node;
    vlink(active) = null;
    alink(active) = null;
    vinfo(align_head) = 0;
    type(align_head) = temp_node;
    vlink(align_head) = null;
    alink(align_head) = null;
    vinfo(end_span) = 0;
    type(end_span) = span_node;
    vlink(end_span) = null;
    alink(end_span) = null;
    type(begin_point) = glyph_node;
    subtype(begin_point) = 0;
    vlink(begin_point) = null;
    vinfo(begin_point + 1) = null;
    alink(begin_point) = null;
    font(begin_point) = 0;
    character(begin_point) = '.';
    vinfo(begin_point + 3) = 0;
    vlink(begin_point + 3) = 0;
    vinfo(begin_point + 4) = 0;
    vlink(begin_point + 4) = 0;
    type(end_point) = glyph_node;
    subtype(end_point) = 0;
    vlink(end_point) = null;
    vinfo(end_point + 1) = null;
    alink(end_point) = null;
    font(end_point) = 0;
    character(end_point) = '.';
    vinfo(end_point + 3) = 0;
    vlink(end_point + 3) = 0;
    vinfo(end_point + 4) = 0;
    vlink(end_point + 4) = 0;
}

@ @c
void dump_node_mem(void)
{
    dump_int(var_mem_max);
    dump_int(rover);
    dump_things(varmem[0], var_mem_max);
#ifdef CHECK_NODE_USAGE
    dump_things(varmem_sizes[0], var_mem_max);
#endif
    dump_things(free_chain[0], MAX_CHAIN_SIZE);
    dump_int(var_used);
    dump_int(my_prealloc);
}

@ it makes sense to enlarge the varmem array immediately
@c

void undump_node_mem(void)
{
    int x;
    undump_int(x);
    undump_int(rover);
    var_mem_max = (x < 100000 ? 100000 : x);
    varmem = xmallocarray(memory_word, (unsigned) var_mem_max);
    undump_things(varmem[0], x);
#ifdef CHECK_NODE_USAGE
    varmem_sizes = xmallocarray(char, (unsigned) var_mem_max);
    memset((void *) varmem_sizes, 0, (unsigned) var_mem_max * sizeof(char));
    undump_things(varmem_sizes[0], x);
#endif
    undump_things(free_chain[0], MAX_CHAIN_SIZE);
    undump_int(var_used);
    undump_int(my_prealloc);
    if (var_mem_max > x) {
        /* todo ? it is perhaps possible to merge the new node with an existing rover */
        vlink(x) = rover;
        node_size(x) = (var_mem_max - x);
        while (vlink(rover) != vlink(x)) {
            rover = vlink(rover);
        }
        vlink(rover) = x;
    }
}

@ @c
halfword slow_get_node(int s)
{
    register int t;

  RETRY:
    t = node_size(rover);
    if (vlink(rover) < var_mem_max && vlink(rover) != 0) {
        if (t > s) {
            /* allocating from the bottom helps decrease page faults */
            register halfword r = rover;
            rover += s;
            vlink(rover) = vlink(r);
            node_size(rover) = node_size(r) - s;
            if (vlink(rover) != r) {        /* list is longer than one */
                halfword q = r;
                while (vlink(q) != r) {
                    q = vlink(q);
                }
                vlink(q) += s;
            } else {
                vlink(rover) += s;
            }
            if (vlink(rover) < var_mem_max) {
#ifdef CHECK_NODE_USAGE
                varmem_sizes[r] = (char) (s > 127 ? 127 : s);
#endif
                vlink(r) = null;
                var_used += s;          /* maintain usage statistics */
                return r;               /* this is the only exit */
            } else {
                normal_error("nodes","there is a problem in getting a node, case 2");
                return null;
            }
        } else {
            /* attempt to keep the free list small */
            int x;
            if (vlink(rover) != rover) {
                if (t < MAX_CHAIN_SIZE) {
                    halfword l = vlink(rover);
                    vlink(rover) = free_chain[t];
                    free_chain[t] = rover;
                    rover = l;
                    while (vlink(l) != free_chain[t]) {
                        l = vlink(l);
                    }
                    vlink(l) = rover;
                    goto RETRY;
                } else {
                    halfword l = rover;
                    while (vlink(rover) != l) {
                        if (node_size(rover) > s) {
                            goto RETRY;
                        }
                        rover = vlink(rover);
                    }
                }
            }
            /* if we are still here, it was apparently impossible to get a match */
            x = (var_mem_max >> 2) + s;
            varmem = (memory_word *) realloc((void *) varmem, sizeof(memory_word) * (unsigned) (var_mem_max + x));
            if (varmem == NULL) {
                overflow("node memory size", (unsigned) var_mem_max);
            }
            memset((void *) (varmem + var_mem_max), 0, (unsigned) x * sizeof(memory_word));
#ifdef CHECK_NODE_USAGE
            varmem_sizes = (char *) realloc(varmem_sizes, sizeof(char) * (unsigned) (var_mem_max + x));
            if (varmem_sizes == NULL) {
                overflow("node memory size", (unsigned) var_mem_max);
            }
            memset((void *) (varmem_sizes + var_mem_max), 0, (unsigned) (x) * sizeof(char));
#endif
            /* todo ? it is perhaps possible to merge the new memory with an existing rover */
            vlink(var_mem_max) = rover;
            node_size(var_mem_max) = x;
            while (vlink(rover) != vlink(var_mem_max)) {
                rover = vlink(rover);
            }
            vlink(rover) = var_mem_max;
            rover = var_mem_max;
            var_mem_max += x;
            goto RETRY;
        }
    } else {
        normal_error("nodes","there is a problem in getting a node, case 3");
        return null;
    }
}

@ @c
char *sprint_node_mem_usage(void)
{
    char *s;
#ifdef CHECK_NODE_USAGE
    char *ss;
    int i;
    int b = 0;
    char msg[256];
    int node_counts[last_normal_node + last_whatsit_node + 2] = { 0 };
    s = strdup("");
    for (i = (var_mem_max - 1); i > my_prealloc; i--) {
        if (varmem_sizes[i] > 0) {
            if (type(i) > last_normal_node + last_whatsit_node) {
                node_counts[last_normal_node + last_whatsit_node + 1]++;
            } else if (type(i) == whatsit_node) {
                node_counts[(subtype(i) + last_normal_node + 1)]++;
            } else {
                node_counts[type(i)]++;
            }
        }
    }
    for (i = 0; i < last_normal_node + last_whatsit_node + 2; i++) {
        if (node_counts[i] > 0) {
            int j =
                (i > (last_normal_node + 1) ? (i - last_normal_node - 1) : 0);
            snprintf(msg, 255, "%s%d %s", (b ? ", " : ""), (int) node_counts[i],
                     get_node_name((i > last_normal_node ? whatsit_node : i), j));
            ss = xmalloc((unsigned) (strlen(s) + strlen(msg) + 1));
            strcpy(ss, s);
            strcat(ss, msg);
            free(s);
            s = ss;
            b = 1;
        }
    }
#else
    s = strdup("");
#endif
    return s;
}

@ @c
halfword list_node_mem_usage(void)
{
    halfword q = null;
#ifdef CHECK_NODE_USAGE
    halfword p = null;
    halfword i, j;
    char *saved_varmem_sizes = xmallocarray(char, (unsigned) var_mem_max);
    memcpy(saved_varmem_sizes, varmem_sizes, (size_t) var_mem_max);
    for (i = my_prealloc + 1; i < (var_mem_max - 1); i++) {
        if (saved_varmem_sizes[i] > 0) {
            j = copy_node(i);
            if (p == null) {
                q = j;
            } else {
                vlink(p) = j;
            }
            p = j;
        }
    }
    free(saved_varmem_sizes);
#endif
    return q;
}

@ @c
void print_node_mem_stats(void)
{
    int i, b;
    halfword j;
    char msg[256];
    char *s;
    int free_chain_counts[MAX_CHAIN_SIZE] = { 0 };
    snprintf(msg, 255, " %d words of node memory still in use:", (int) (var_used + my_prealloc));
    tprint_nl(msg);
    s = sprint_node_mem_usage();
    tprint_nl("   ");
    tprint(s);
    free(s);
    tprint(" nodes");
    tprint_nl("   avail lists: ");
    b = 0;
    for (i = 1; i < MAX_CHAIN_SIZE; i++) {
        for (j = free_chain[i]; j != null; j = vlink(j))
            free_chain_counts[i]++;
        if (free_chain_counts[i] > 0) {
            snprintf(msg, 255, "%s%d:%d", (b ? "," : ""), i, (int) free_chain_counts[i]);
            tprint(msg);
            b = 1;
        }
    }
    /* newline, if needed */
    print_nlp();
}

/* this belongs in the web but i couldn't find the correct syntactic place */

halfword new_span_node(halfword n, int s, scaled w)
{
    halfword p = new_node(span_node, 0);
    span_link(p) = n;
    span_span(p) = s;
    width(p) = w;
    return p;
}

@* Attribute stuff.

@c
static halfword new_attribute_node(unsigned int i, int v)
{
    register halfword r = get_node(attribute_node_size);
    type(r) = attribute_node;
    attribute_id(r) = (halfword) i;
    attribute_value(r) = v;
    /* not used but nicer in print */
    subtype(r) = 0;
    return r;
}

@ @c
halfword copy_attribute_list(halfword n)
{
    halfword q = get_node(attribute_node_size);
    register halfword p = q;
    type(p) = attribute_list_node;
    attr_list_ref(p) = 0;
    n = vlink(n);
    while (n != null) {
        register halfword r = get_node(attribute_node_size);
        /* the link will be fixed automatically in the next loop */
        (void) memcpy((void *) (varmem + r), (void *) (varmem + n),
                      (sizeof(memory_word) * attribute_node_size));
        vlink(p) = r;
        p = r;
        n = vlink(n);
    }
    return q;
}

@ @c
void update_attribute_cache(void)
{
    halfword p;
    register int i;
    attr_list_cache = get_node(attribute_node_size);
    type(attr_list_cache) = attribute_list_node;
    attr_list_ref(attr_list_cache) = 0;
    p = attr_list_cache;
    for (i = 0; i <= max_used_attr; i++) {
        register int v = attribute(i);
        if (v > UNUSED_ATTRIBUTE) {
            register halfword r = new_attribute_node((unsigned) i, v);
            vlink(p) = r;
            p = r;
        }
    }
    if (vlink(attr_list_cache) == null) {
        free_node(attr_list_cache, attribute_node_size);
        attr_list_cache = null;
    }
    return;
}

@ @c
void build_attribute_list(halfword b)
{
    if (max_used_attr >= 0) {
        if (attr_list_cache == cache_disabled|| attr_list_cache == null) {
            update_attribute_cache();
            if (attr_list_cache == null)
                return;
        }
        attr_list_ref(attr_list_cache)++;
        node_attr(b) = attr_list_cache;
    }
}

@ @c
halfword current_attribute_list(void)
{
    if (max_used_attr >= 0) {
      if (attr_list_cache == cache_disabled) {
            update_attribute_cache();
      }
      return attr_list_cache ;
    }
    return null ;
}


@ @c
void reassign_attribute(halfword n, halfword new)
{
    halfword old;
    old = node_attr(n);
    if (new == null) {
         /* there is nothing to assign but we need to check for an old value */
        if (old != null)
            delete_attribute_ref(old); /* also nulls attr field of n */
    } else if (old == null) {
         /* nothing is assigned so we just do that now */
        assign_attribute_ref(n,new);
    } else if (old != new) {
         /* something is assigned so we need to clean up and assign then */
        delete_attribute_ref(old);
        assign_attribute_ref(n,new);
    }
     /* else: same value so there is no need to assign and change the refcount */
    node_attr(n) = new ;
}

@ @c
void delete_attribute_ref(halfword b)
{
    if (b != null) {
        if (type(b) == attribute_list_node){
            attr_list_ref(b)--;
            if (attr_list_ref(b) == 0) {
                if (b == attr_list_cache)
                    attr_list_cache = cache_disabled;
                free_node_chain(b, attribute_node_size);
            }
            /* maintain sanity */
            if (attr_list_ref(b) < 0) {
                attr_list_ref(b) = 0;
            }
        } else {
            normal_error("nodes","trying to delete an attribute reference of a non attribute node");
        }
    }
}

void reset_node_properties(halfword b)
{
    if (b != null) {
        lua_properties_reset(b);
    }
}

@ |p| is an attr list head, or zero
@c
halfword do_set_attribute(halfword p, int i, int val)
{
    register halfword q;
    register int j = 0;
    if (p == null) {            /* add a new head \& node */
        q = get_node(attribute_node_size);
        type(q) = attribute_list_node;
        attr_list_ref(q) = 1;
        p = new_attribute_node((unsigned) i, val);
        vlink(q) = p;
        return q;
    }
    q = p;
    if (vlink(p) != null) {
        while (vlink(p) != null) {
            int t = attribute_id(vlink(p));
            if (t == i && attribute_value(vlink(p)) == val)
                return q;           /* no need to do anything */
            if (t >= i)
                break;
            j++;
            p = vlink(p);
        }

        p = q;
        while (j-- > 0)
            p = vlink(p);
        if (attribute_id(vlink(p)) == i) {
            attribute_value(vlink(p)) = val;
        } else {                    /* add a new node */
            halfword r = new_attribute_node((unsigned) i, val);
            vlink(r) = vlink(p);
            vlink(p) = r;
        }
        return q;
    } else {
        normal_error("nodes","trying to set an attribute fails, case 1");
        return null ;
    }
}

@ @c
void set_attribute(halfword n, int i, int val)
{
    register halfword p;
    register int j = 0;
    /* not all nodes can have an attribute list */
    if (!nodetype_has_attributes(type(n)))
        return;
    /* if we have no list, we create one and quit */
    p = node_attr(n);
    if (p == null) {            /* add a new head \& node */
        p = get_node(attribute_node_size);
        type(p) = attribute_list_node;
        attr_list_ref(p) = 1;
        node_attr(n) = p;
        p = new_attribute_node((unsigned) i, val);
        vlink(node_attr(n)) = p;
        return;
    }
    /* we check if we have this attribute already and quit if the value stays the same */
    if (vlink(p) != null) {
        while (vlink(p) != null) {
            int t = attribute_id(vlink(p));
            if (t == i && attribute_value(vlink(p)) == val)
                return;
            if (t >= i)
                break;
            j++;
            p = vlink(p);
        }
        /* j has now the position (if found) .. we assume a sorted list ! */
        p = node_attr(n);

        if (attr_list_ref(p) == 0 ) {
            /* the list is invalid i.e. freed already */
            formatted_warning("nodes","node %d has an attribute list that is free already, case 1",(int) n);
            /* the still dangling list gets ref count 1 */
            attr_list_ref(p) = 1;
        } else if (attr_list_ref(p) == 1) {
            /* this can really happen HH-LS */
            if (p == attr_list_cache) {
                /* we can invalidate the cache setting */
                /* attr_list_cache = cache_disabled    */
                /* or save the list, as done below     */
                p = copy_attribute_list(p);
                node_attr(n) = p;
                /* the copied list gets ref count 1 */
                attr_list_ref(p) = 1;
            }
        } else {
            /* the list is used multiple times so we make a copy */
            p = copy_attribute_list(p);
            /* we decrement the ref count or the original */
            delete_attribute_ref(node_attr(n));
            node_attr(n) = p;
            /* the copied list gets ref count 1 */
            attr_list_ref(p) = 1;
        }


        /* we go to position j in the list */
        while (j-- > 0)
            p = vlink(p);
        /* if we have a hit we just set the value otherwise we add a new node */
        if (attribute_id(vlink(p)) == i) {
            attribute_value(vlink(p)) = val;
        } else {                    /* add a new node */
            halfword r = new_attribute_node((unsigned) i, val);
            vlink(r) = vlink(p);
            vlink(p) = r;
        }
    } else {
        normal_error("nodes","trying to set an attribute fails, case 2");
    }
}

@ @c
int unset_attribute(halfword n, int i, int val)
{
    register halfword p;
    register int t;
    register int j = 0;

    if (!nodetype_has_attributes(type(n)))
        return null;
    p = node_attr(n);
    if (p == null)
        return UNUSED_ATTRIBUTE;
    if (attr_list_ref(p) == 0) {
        formatted_warning("nodes","node %d has an attribute list that is free already, case 2", (int) n);
        return UNUSED_ATTRIBUTE;
    }
    if (vlink(p) != null) {
        while (vlink(p) != null) {
            t = attribute_id(vlink(p));
            if (t > i)
                return UNUSED_ATTRIBUTE;
            if (t == i) {
                p = vlink(p);
                break;
            }
            j++;
            p = vlink(p);
        }
        if (attribute_id(p) != i)
            return UNUSED_ATTRIBUTE;
        /* if we are still here, the attribute exists */
        p = node_attr(n);
        if (attr_list_ref(p) > 1 || p == attr_list_cache) {
            halfword q = copy_attribute_list(p);
            if (attr_list_ref(p) > 1) {
                delete_attribute_ref(node_attr(n));
            }
            attr_list_ref(q) = 1;
            node_attr(n) = q;
        }
        p = vlink(node_attr(n));
        while (j-- > 0)
            p = vlink(p);
        t = attribute_value(p);
        if (val == UNUSED_ATTRIBUTE || t == val) {
            attribute_value(p) = UNUSED_ATTRIBUTE;
        }
        return t;
    } else {
        normal_error("nodes","trying to unset an attribute fails");
        return null;
    }
}

@ @c
int has_attribute(halfword n, int i, int val)
{
    register halfword p;
    if (!nodetype_has_attributes(type(n)))
        return UNUSED_ATTRIBUTE;
    p = node_attr(n);
    if (p == null || vlink(p) == null)
        return UNUSED_ATTRIBUTE;
    p = vlink(p);
    while (p != null) {
        if (attribute_id(p) == i) {
            int ret = attribute_value(p);
            if (val == UNUSED_ATTRIBUTE || val == ret)
                return ret;
            return UNUSED_ATTRIBUTE;
        } else if (attribute_id(p) > i) {
            return UNUSED_ATTRIBUTE;
        }
        p = vlink(p);
    }
    return UNUSED_ATTRIBUTE;
}

@ @c
void print_short_node_contents(halfword p)
{
    switch (type(p)) {
        case hlist_node:
        case vlist_node:
        case ins_node:
        case whatsit_node:
        case mark_node:
        case adjust_node:
        case unset_node:
            print_char('[');
            print_char(']');
            break;
        case rule_node:
            print_char('|');
            break;
        case glue_node:
            if (! glue_is_zero(p))
                print_char(' ');
            break;
        case math_node:
            print_char('$');
            break;
        case disc_node:
            short_display(vlink(pre_break(p)));
            short_display(vlink(post_break(p)));
            break;
    }
}

@ @c
static void show_pdftex_whatsit_rule_spec(int p)
{
    tprint("(");
    print_rule_dimen(height(p));
    print_char('+');
    print_rule_dimen(depth(p));
    tprint(")x");
    print_rule_dimen(width(p));
}

@ Each new type of node that appears in our data structure must be capable
of being displayed, copied, destroyed, and so on. The routines that we
need for write-oriented whatsits are somewhat like those for mark nodes;
other extensions might, of course, involve more subtlety here.

@c
static void print_write_whatsit(const char *s, pointer p)
{
    tprint_esc(s);
    if (write_stream(p) < 16)
        print_int(write_stream(p));
    else if (write_stream(p) == 16)
        print_char('*');
    else
        print_char('-');
}

@ @c
static void show_node_wrapup_core(int p)
{
    switch (subtype(p)) {
        case open_node:
            print_write_whatsit("openout", p);
            print_char('=');
            print_file_name(open_name(p), open_area(p), open_ext(p));
            break;
        case write_node:
            print_write_whatsit("write", p);
            print_mark(write_tokens(p));
            break;
        case close_node:
            print_write_whatsit("closeout", p);
            break;
        case special_node:
            tprint_esc("special");
            print_mark(write_tokens(p));
            break;
        case late_lua_node:
            show_late_lua(p);
            break;
        case save_pos_node:
            tprint_esc("savepos");
            break;
        case user_defined_node:
            tprint_esc("whatsit");
            print_int(user_node_id(p));
            print_char('=');
            switch (user_node_type(p)) {
            case 'a':
                tprint("<>");
                break;
            case 'n':
                tprint("[");
                show_node_list(user_node_value(p));
                tprint("]");
                break;
            case 's':
                print_char('"');
                print(user_node_value(p));
                print_char('"');
                break;
            case 't':
                print_mark(user_node_value(p));
                break;
            default:               /* only 'd' */
                print_int(user_node_value(p));
                break;
            }
            break;
    }
}

void show_node_wrapup_dvi(int p)
{
}

void show_node_wrapup_pdf(int p)
{
    switch (subtype(p)) {
        case pdf_literal_node:
            show_pdf_literal(p);
            break;
        case pdf_colorstack_node:
            tprint_esc("pdfcolorstack ");
            print_int(pdf_colorstack_stack(p));
            switch (pdf_colorstack_cmd(p)) {
            case colorstack_set:
                tprint(" set ");
                break;
            case colorstack_push:
                tprint(" push ");
                break;
            case colorstack_pop:
                tprint(" pop");
                break;
            case colorstack_current:
                tprint(" current");
                break;
            default:
                confusion("colorstack");
                break;
            }
            if (pdf_colorstack_cmd(p) <= colorstack_data)
                print_mark(pdf_colorstack_data(p));
            break;
        case pdf_setmatrix_node:
            tprint_esc("pdfsetmatrix");
            print_mark(pdf_setmatrix_data(p));
            break;
        case pdf_save_node:
            tprint_esc("pdfsave");
            break;
        case pdf_restore_node:
            tprint_esc("pdfrestore");
            break;
        case pdf_refobj_node:
            tprint_esc("pdfrefobj");
            if (obj_obj_is_stream(static_pdf, pdf_obj_objnum(p))) {
                if (obj_obj_stream_attr(static_pdf, pdf_obj_objnum(p)) != LUA_NOREF) {
                    tprint(" attr");
                    lua_rawgeti(Luas, LUA_REGISTRYINDEX,
                                obj_obj_stream_attr(static_pdf, pdf_obj_objnum(p)));
                    print_char(' ');
                    tprint((const char *) lua_tostring(Luas, -1));
                    lua_pop(Luas, 1);
                }
                tprint(" stream");
            }
            if (obj_obj_is_file(static_pdf, pdf_obj_objnum(p)))
                tprint(" file");
            if (obj_obj_data(static_pdf, pdf_obj_objnum(p)) != LUA_NOREF) {
                lua_rawgeti(Luas, LUA_REGISTRYINDEX,
                            obj_obj_data(static_pdf, pdf_obj_objnum(p)));
                print_char(' ');
                tprint((const char *) lua_tostring(Luas, -1));
                lua_pop(Luas, 1);
            }
            break;
        case pdf_annot_node:
            tprint_esc("pdfannot");
            show_pdftex_whatsit_rule_spec(p);
            print_mark(pdf_annot_data(p));
            break;
        case pdf_start_link_node:
            tprint_esc("pdfstartlink");
            show_pdftex_whatsit_rule_spec(p);
            if (pdf_link_attr(p) != null) {
                tprint(" attr");
                print_mark(pdf_link_attr(p));
            }
            tprint(" action");
            if (pdf_action_type(pdf_link_action(p)) == pdf_action_user) {
                tprint(" user");
                print_mark(pdf_action_tokens(pdf_link_action(p)));
                return;
            }
            if (pdf_action_file(pdf_link_action(p)) != null) {
                tprint(" file");
                print_mark(pdf_action_file(pdf_link_action(p)));
            }
            switch (pdf_action_type(pdf_link_action(p))) {
            case pdf_action_goto:
                if (pdf_action_named_id(pdf_link_action(p)) > 0) {
                    tprint(" goto name");
                    print_mark(pdf_action_id(pdf_link_action(p)));
                } else {
                    tprint(" goto num");
                    print_int(pdf_action_id(pdf_link_action(p)));
                }
                break;
            case pdf_action_page:
                tprint(" page");
                print_int(pdf_action_id(pdf_link_action(p)));
                print_mark(pdf_action_tokens(pdf_link_action(p)));
                break;
            case pdf_action_thread:
                if (pdf_action_named_id(pdf_link_action(p)) > 0) {
                    tprint(" thread name");
                    print_mark(pdf_action_id(pdf_link_action(p)));
                } else {
                    tprint(" thread num");
                    print_int(pdf_action_id(pdf_link_action(p)));
                }
                break;
            default:
                normal_error("pdf backend", "unknown action type for link");
                break;
            }
            break;
        case pdf_end_link_node:
            tprint_esc("pdfendlink");
            break;
        case pdf_dest_node:
            tprint_esc("pdfdest");
            if (pdf_dest_named_id(p) > 0) {
                tprint(" name");
                print_mark(pdf_dest_id(p));
            } else {
                tprint(" num");
                print_int(pdf_dest_id(p));
            }
            print_char(' ');
            switch (pdf_dest_type(p)) {
            case pdf_dest_xyz:
                tprint("xyz");
                if (pdf_dest_xyz_zoom(p) != null) {
                    tprint(" zoom");
                    print_int(pdf_dest_xyz_zoom(p));
                }
                break;
            case pdf_dest_fitbh:
                tprint("fitbh");
                break;
            case pdf_dest_fitbv:
                tprint("fitbv");
                break;
            case pdf_dest_fitb:
                tprint("fitb");
                break;
            case pdf_dest_fith:
                tprint("fith");
                break;
            case pdf_dest_fitv:
                tprint("fitv");
                break;
            case pdf_dest_fitr:
                tprint("fitr");
                show_pdftex_whatsit_rule_spec(p);
                break;
            case pdf_dest_fit:
                tprint("fit");
                break;
            default:
                tprint("unknown!");
                break;
            }
            break;
        case pdf_thread_node:
        case pdf_start_thread_node:
            if (subtype(p) == pdf_thread_node)
                tprint_esc("pdfthread");
            else
                tprint_esc("pdfstartthread");
            tprint("(");
            print_rule_dimen(height(p));
            print_char('+');
            print_rule_dimen(depth(p));
            tprint(")x");
            print_rule_dimen(width(p));
            if (pdf_thread_attr(p) != null) {
                tprint(" attr");
                print_mark(pdf_thread_attr(p));
            }
            if (pdf_thread_named_id(p) > 0) {
                tprint(" name");
                print_mark(pdf_thread_id(p));
            } else {
                tprint(" num");
                print_int(pdf_thread_id(p));
            }
            break;
        case pdf_end_thread_node:
            tprint_esc("pdfendthread");
            break;
        default:
            break;
    }
}

@  Now we are ready for |show_node_list| itself. This procedure has been
  written to be ``extra robust'' in the sense that it should not crash or get
  into a loop even if the data structures have been messed up by bugs in
  the rest of the program. You can safely call its parent routine
  |show_box(p)| for arbitrary values of |p| when you are debugging \TeX.
  However, in the presence of bad data, the procedure may
  fetch a |memory_word| whose variant is different from the way it was stored;
  for example, it might try to read |mem[p].hh| when |mem[p]|
  contains a scaled integer, if |p| is a pointer that has been
  clobbered or chosen at random.


@ |str_room| need not be checked; see |show_box|

@ Recursive calls on |show_node_list| therefore use the following pattern:
@c
#define node_list_display(A) do { \
    append_char('.');             \
    show_node_list(A);            \
    flush_char();                 \
} while (0)

/* prints a node list symbolically */

void show_node_list(int p)
{
    int n = 0;                  /* the number of items already printed at this level */
    halfword w;
    real g;                     /* a glue ratio, as a floating point number */
    if ((int) cur_length > depth_threshold) {
        if (p > null)
            tprint(" []");      /* indicate that there's been some truncation */
        return;
    }
    while (p != null) {
        print_ln();
        print_current_string(); /* display the nesting history */
        if (tracing_online_par < -2)
            print_int(p);
        incr(n);
        if (n > breadth_max) {  /* time to stop */
            tprint("etc.");
            return;
        }
        /* Display node |p| */
        if (is_char_node(p)) {
            print_font_and_char(p);
            if (is_ligature(p)) {
                /* Display ligature |p|; */
                tprint(" (ligature ");
                if (is_leftboundary(p))
                    print_char('|');
                font_in_short_display = font(p);
                short_display(lig_ptr(p));
                if (is_rightboundary(p))
                    print_char('|');
                print_char(')');
            }
        } else {
            switch (type(p)) {
            case hlist_node:
            case vlist_node:
            case unset_node:
                /* Display box |p|; */
                if (type(p) == hlist_node)
                    tprint_esc("h");
                else if (type(p) == vlist_node)
                    tprint_esc("v");
                else
                    tprint_esc("unset");
                tprint("box(");
                print_scaled(height(p));
                print_char('+');
                print_scaled(depth(p));
                tprint(")x");
                print_scaled(width(p));
                if (type(p) == unset_node) {
                    /* Display special fields of the unset node |p|; */
                    if (span_count(p) != min_quarterword) {
                        tprint(" (");
                        print_int(span_count(p) + 1);
                        tprint(" columns)");
                    }
                    if (glue_stretch(p) != 0) {
                        tprint(", stretch ");
                        print_glue(glue_stretch(p), glue_order(p), NULL);
                    }
                    if (glue_shrink(p) != 0) {
                        tprint(", shrink ");
                        print_glue(glue_shrink(p), glue_sign(p), NULL);
                    }
                } else {
                    /* Display the value of |glue_set(p)| */
                    /* The code will have to change in this place if |glue_ratio| is
                       a structured type instead of an ordinary |real|. Note that this routine
                       should avoid arithmetic errors even if the |glue_set| field holds an
                       arbitrary random value. The following code assumes that a properly
                       formed nonzero |real| number has absolute value $2^{20}$ or more when
                       it is regarded as an integer; this precaution was adequate to prevent
                       floating point underflow on the author's computer.
                     */

                    g = (real) (glue_set(p));
                    if ((g != 0.0) && (glue_sign(p) != normal)) {
                        tprint(", glue set ");
                        if (glue_sign(p) == shrinking)
                            tprint("- ");
                        if (g > 20000.0 || g < -20000.0) {
                            if (g > 0.0)
                                print_char('>');
                            else
                                tprint("< -");
                            print_glue(20000 * unity, glue_order(p), NULL);
                        } else {
                            print_glue(round(unity * g), glue_order(p), NULL);
                        }
                    }

                    if (shift_amount(p) != 0) {
                        tprint(", shifted ");
                        print_scaled(shift_amount(p));
                    }
                    tprint(", direction ");
                    print_dir(box_dir(p));
                }
                node_list_display(list_ptr(p)); /* recursive call */
                break;
            case rule_node:
                /* Display rule |p|; */
                if (subtype(p) == normal_rule) {
                    tprint_esc("rule(");
                } else if (subtype(p) == empty_rule) {
                    tprint_esc("norule(");
                } else if (subtype(p) == user_rule) {
                    tprint_esc("userrule(");
                } else if (subtype(p) == box_rule) {
                    tprint_esc("box(");
                } else if (subtype(p) == image_rule) {
                    tprint_esc("image(");
                }
                print_rule_dimen(height(p));
                print_char('+');
                print_rule_dimen(depth(p));
                tprint(")x");
                print_rule_dimen(width(p));
                break;
            case ins_node:
                /* Display insertion |p|; */
                tprint_esc("insert");
                print_int(subtype(p));
                tprint(", natural size ");
                print_scaled(height(p));
                tprint("; split(");
                print_spec(split_top_ptr(p), NULL);
                print_char(',');
                print_scaled(depth(p));
                tprint("); float cost ");
                print_int(float_cost(p));
                node_list_display(ins_ptr(p));  /* recursive call */
                break;
            case dir_node:
                if (dir_dir(p) < 0) {
                    tprint_esc("enddir");
                    print_char(' ');
                    print_dir(dir_dir(p) + dir_swap);
                } else {
                    tprint_esc("begindir");
                    print_char(' ');
                    print_dir(dir_dir(p));
                }
                break;
            case local_par_node:
                tprint_esc("localpar");
                append_char('.');
                print_ln();
                print_current_string();
                tprint_esc("localinterlinepenalty");
                print_char('=');
                print_int(local_pen_inter(p));
                print_ln();
                print_current_string();
                tprint_esc("localbrokenpenalty");
                print_char('=');
                print_int(local_pen_broken(p));
                print_ln();
                print_current_string();
                tprint_esc("localleftbox");
                if (local_box_left(p) == null) {
                    tprint("=null");
                } else {
                    append_char('.');
                    show_node_list(local_box_left(p));
                    decr(cur_length);
                }
                print_ln();
                print_current_string();
                tprint_esc("localrightbox");
                if (local_box_right(p) == null) {
                    tprint("=null");
                } else {
                    append_char('.');
                    show_node_list(local_box_right(p));
                    decr(cur_length);
                }
                decr(cur_length);
                break;
            case boundary_node:
                if (subtype(p)==0) {
                    tprint_esc("noboundary");
                } else {
                    switch (subtype(p)) {
                        case 1:
                            tprint_esc("boundary");
                            break;
                        case 2:
                            tprint_esc("protrusionboundary");
                            break;
                        case 3:
                            tprint_esc("wordboundary");
                            break;
                        default:
                            tprint_esc("boundary");
                            print_char(':');
                            print_int(subtype(p));
                            break;
                    }
                    print_char('=');
                    print_int(boundary_value(p));
                }
                break;
            case whatsit_node:
                w = subtype(p) ;
                if (w >= backend_first_pdf_whatsit) {
                    show_node_wrapup_pdf(p);
                } else if (w >= backend_first_dvi_whatsit) {
                    show_node_wrapup_dvi(p);
                } else {
                    show_node_wrapup_core(p);
                }
                break;
            case glue_node:
                /* Display glue |p|; */
                if (subtype(p) >= a_leaders) {
                    /* Display leaders |p|; */
                    tprint_esc("");
                    switch (subtype(p)) {
                    case a_leaders:
                        break;
                    case c_leaders:
                        print_char('c');
                        break;
                    case x_leaders:
                        print_char('x');
                        break;
                    case g_leaders:
                        print_char('g');
                        break;
                    default:
                        normal_warning("nodes","weird glue leader subtype ignored");
                    }
                    tprint("leaders ");
                    print_spec(p, NULL);
                    node_list_display(leader_ptr(p));   /* recursive call */
                } else {
                    tprint_esc("glue");
                    if (subtype(p) != normal) {
                        print_char('(');
                        if ((subtype(p) - 1) < thin_mu_skip_code) {
                            print_cmd_chr(assign_glue_cmd, glue_base + (subtype(p) - 1));
                        } else if (subtype(p) < cond_math_glue) {
                            print_cmd_chr(assign_mu_glue_cmd, glue_base + (subtype(p) - 1));
                        } else if (subtype(p) == cond_math_glue) {
                            tprint_esc("nonscript");
                        } else {
                            tprint_esc("mskip");
                        }
                        print_char(')');
                    }
                    if (subtype(p) != cond_math_glue) {
                        print_char(' ');
                        if (subtype(p) < cond_math_glue)
                            print_spec(p, NULL);
                        else
                            print_spec(p, "mu");
                    }
                }
                break;
            case margin_kern_node:
                tprint_esc("kern");
                print_scaled(width(p));
                if (subtype(p) == left_side)
                    tprint(" (left margin)");
                else
                    tprint(" (right margin)");
                break;
            case kern_node:
                /* Display kern |p|; */
                /*  An ``explicit'' kern value is indicated implicitly by an explicit space. */
                if (subtype(p) != mu_glue) {
                    tprint_esc("kern");
                    if (subtype(p) != normal)
                        print_char(' ');
                    print_scaled(width(p));
                    if (subtype(p) == accent_kern)
                        tprint(" (for accent)");
                } else {
                    tprint_esc("mkern");
                    print_scaled(width(p));
                    tprint("mu");
                }
                break;
            case math_node:
                /* Display math node |p|; */
                tprint_esc("math");
                if (subtype(p) == before)
                    tprint("on");
                else
                    tprint("off");
                if (!glue_is_zero(p)) {
                    tprint(", glued ");
                    print_spec(p, NULL);
                } else if (surround(p) != 0) {
                    tprint(", surrounded ");
                    print_scaled(surround(p));
                }
                break;
            case penalty_node:
                /* Display penalty |p|; */
                tprint_esc("penalty ");
                print_int(penalty(p));
                break;
            case disc_node:
                /* Display discretionary |p|; */
                /* The |post_break| list of a discretionary node is indicated by a prefixed
                   `\.{\char'174}' instead of the `\..' before the |pre_break| list. */
                tprint_esc("discretionary");
                print_int(disc_penalty(p));
                print_char('|');
                if (vlink(no_break(p)) != null) {
                    tprint(" replacing ");
                    node_list_display(vlink(no_break(p)));
                }
                node_list_display(vlink(pre_break(p))); /* recursive call */
                append_char('|');
                show_node_list(vlink(post_break(p)));
                flush_char();   /* recursive call */
                break;
            case mark_node:
                /* Display mark |p|; */
                tprint_esc("mark");
                if (mark_class(p) != 0) {
                    print_char('s');
                    print_int(mark_class(p));
                }
                print_mark(mark_ptr(p));
                break;
            case adjust_node:
                /* Display adjustment |p|; */
                tprint_esc("vadjust");
                if (subtype(p) != 0)
                    tprint(" pre ");
                node_list_display(adjust_ptr(p));       /* recursive call */
                break;
            case glue_spec_node:
                tprint("<glue_spec ");
                print_spec(p, NULL);
                tprint(">");
                break;
            default:
                show_math_node(p);
                break;
            }
        }
        p = vlink(p);
    }
}

@ This routine finds the 'base' width of a horizontal box, using the same logic
  that \TeX82 used for \.{\\predisplaywidth} */

@c
pointer actual_box_width(pointer r, scaled base_width)
{
    scaled d;                                /* increment to |v| */
    scaled w = -max_dimen;                   /* calculated |size| */
    scaled v = shift_amount(r) + base_width; /* |w| plus possible glue amount */
    pointer p = list_ptr(r);                 /* current node when calculating |pre_display_size| */
    while (p != null) {
        if (is_char_node(p)) {
            d = glyph_width(p);
            goto FOUND;
        }
        switch (type(p)) {
            case hlist_node:
            case vlist_node:
            case rule_node:
                d = width(p);
                goto FOUND;
                break;
            case margin_kern_node:
                d = width(p);
                break;
            case kern_node:
                d = width(p);
                break;
            case math_node:
                /* begin mathskip code */
                if (glue_is_zero(p)) {
                    d = surround(p);
                    break;
                } else {
                    /* fall through */
                }
                /* end mathskip code */
            case glue_node:
                /* We need to be careful that |w|, |v|, and |d| do not depend on any |glue_set|
                   values, since such values are subject to system-dependent rounding.
                   System-dependent numbers are not allowed to infiltrate parameters like
                   |pre_display_size|, since \TeX82 is supposed to make the same decisions on all
                   machines.
                 */
                d = width(p);
                if (glue_sign(r) == stretching) {
                    if ((glue_order(r) == stretch_order(p)) && (stretch(p) != 0))
                        v = max_dimen;
                } else if (glue_sign(r) == shrinking) {
                    if ((glue_order(r) == shrink_order(p)) && (shrink(p) != 0))
                        v = max_dimen;
                }
                if (subtype(p) >= a_leaders)
                    goto FOUND;
                break;
            default:
                d = 0;
                break;
        }
        if (v < max_dimen)
            v = v + d;
        goto NOT_FOUND;
      FOUND:
        if (v < max_dimen) {
            v = v + d;
            w = v;
        } else {
            w = max_dimen;
            break;
        }
      NOT_FOUND:
        p = vlink(p);
    }
    return w;
}

@ @c
halfword tail_of_list(halfword p)
{
    halfword q = p;
    while (vlink(q) != null)
        q = vlink(q);
    return q;
}



@ @c
int var_used;

@ Attribute lists need two extra globals to increase processing efficiency.
|max_used_attr| limits the test loop that checks for set attributes, and
|attr_list_cache| contains a pointer to an already created attribute list.  It is
set to the special value |cache_disabled| when the current value can no longer be
trusted: after an assignment to an attribute register, and after a group has
ended.

@c
int max_used_attr;        /* maximum assigned attribute id  */
halfword attr_list_cache;

@ From the computer's standpoint, \TeX's chief mission is to create
horizontal and vertical lists. We shall now investigate how the elements
of these lists are represented internally as nodes in the dynamic memory.

A horizontal or vertical list is linked together by |link| fields in
the first word of each node. Individual nodes represent boxes, glue,
penalties, or special things like discretionary hyphens; because of this
variety, some nodes are longer than others, and we must distinguish different
kinds of nodes. We do this by putting a `|type|' field in the first word,
together with the link and an optional `|subtype|'.

@ Character nodes appear only in horizontal lists, never in vertical lists.

An |hlist_node| stands for a box that was made from a horizontal list.
Each |hlist_node| is seven words long, and contains the following fields
(in addition to the mandatory |type| and |link|, which we shall not
mention explicitly when discussing the other node types): The |height| and
|width| and |depth| are scaled integers denoting the dimensions of the
box.  There is also a |shift_amount| field, a scaled integer indicating
how much this box should be lowered (if it appears in a horizontal list),
or how much it should be moved to the right (if it appears in a vertical
list). There is a |list_ptr| field, which points to the beginning of the
list from which this box was fabricated; if |list_ptr| is |null|, the box
is empty. Finally, there are three fields that represent the setting of
the glue:  |glue_set(p)| is a word of type |glue_ratio| that represents
the proportionality constant for glue setting; |glue_sign(p)| is
|stretching| or |shrinking| or |normal| depending on whether or not the
glue should stretch or shrink or remain rigid; and |glue_order(p)|
specifies the order of infinity to which glue setting applies (|normal|,
|sfi|, |fil|, |fill|, or |filll|). The |subtype| field is not used.

@ The |new_null_box| function returns a pointer to an |hlist_node| in
which all subfields have the values corresponding to `\.{\\hbox\{\}}'.
The |subtype| field is set to |min_quarterword|, since that's the desired
|span_count| value if this |hlist_node| is changed to an |unset_node|.

@c
halfword new_null_box(void)
{                               /* creates a new box node */
    halfword p = new_node(hlist_node, min_quarterword);
    box_dir(p) = text_direction_par;
    return p;
}

@ A |vlist_node| is like an |hlist_node| in all respects except that it
contains a vertical list.

@ A |rule_node| stands for a solid black rectangle; it has |width|,
|depth|, and |height| fields just as in an |hlist_node|. However, if
any of these dimensions is $-2^{30}$, the actual value will be determined
by running the rule up to the boundary of the innermost enclosing box.
This is called a ``running dimension.'' The |width| is never running in
an hlist; the |height| and |depth| are never running in a~vlist.

@ A new rule node is delivered by the |new_rule| function. It
makes all the dimensions ``running,'' so you have to change the
ones that are not allowed to run.

@c
halfword new_rule(int s)
{
    halfword p = new_node(rule_node,s);
    return p;
}

@ Insertions are represented by |ins_node| records, where the |subtype|
indicates the corresponding box number. For example, `\.{\\insert 250}'
leads to an |ins_node| whose |subtype| is |250+min_quarterword|.
The |height| field of an |ins_node| is slightly misnamed; it actually holds
the natural height plus depth of the vertical list being inserted.
The |depth| field holds the |split_max_depth| to be used in case this
insertion is split, and the |split_top_ptr| points to the corresponding
|split_top_skip|. The |float_cost| field holds the |floating_penalty| that
will be used if this insertion floats to a subsequent page after a
split insertion of the same class.  There is one more field, the
|ins_ptr|, which points to the beginning of the vlist for the insertion.

@ A |mark_node| has a |mark_ptr| field that points to the reference count
of a token list that contains the user's \.{\\mark} text.
In addition there is a |mark_class| field that contains the mark class.

@ An |adjust_node|, which occurs only in horizontal lists,
specifies material that will be moved out into the surrounding
vertical list; i.e., it is used to implement \TeX's `\.{\\vadjust}'
operation.  The |adjust_ptr| field points to the vlist containing this
material.

@ A |glyph_node|, which occurs only in horizontal lists, specifies a
glyph in a particular font, along with its attribute list. Older
versions of \TeX\ could use token memory for characters, because the
font,char combination would fit in a single word (both values were
required to be strictly less than $2^{16}$). In LuaTeX, room is
needed for characters that are larger than that, as well as a pointer
to a potential attribute list, and the two displacement values.

In turn, that made the node so large that it made sense to merge
ligature glyphs as well, as that requires only one extra pointer.  A
few extra classes of glyph nodes will be introduced later.  The
unification of all those types makes it easier to manipulate lists of
glyphs. The subtype differentiates various glyph kinds.

First, here is a function that returns a pointer to a glyph node for a given
glyph in a given font. If that glyph doesn't exist, |null| is returned
instead.  Nodes of this subtype are directly created only for accents
and their base (through |make_accent|), and math nucleus items (in the
conversion from |mlist| to |hlist|).

@c
halfword new_glyph(int f, int c)
{
    halfword p = null;          /* the new node */
    if ((f == 0) || (char_exists(f, c))) {
        p = new_glyph_node();
        set_to_glyph(p);
        font(p) = f;
        character(p) = c;
    }
    return p;
}

@ A subset of the glyphs nodes represent ligatures: characters
fabricated from the interaction of two or more actual characters.  The
characters that generated the ligature have not been forgotten, since
they are needed for diagnostic messages; the |lig_ptr| field points to
a linked list of character nodes for all original characters that have
been deleted. (This list might be empty if the characters that
generated the ligature were retained in other nodes.)

The |subtype| field of these |glyph_node|s is 1, plus 2 and/or 1 if
the original source of the ligature included implicit left and/or
right boundaries. These nodes are created by the C function |new_ligkern|.

A third general type of glyphs could be called a character, as it
only appears in lists that are not yet processed by the ligaturing and
kerning steps of the program.

|main_control| inserts these, and they are later converted to
|subtype_normal| by |new_ligkern|.

@c
quarterword norm_min(int h)
{
    if (h <= 0)
        return 1;
    else if (h >= 255)
        return 255;
    else
        return (quarterword) h;
}

halfword new_char(int f, int c)
{
    halfword p;                 /* the new node */
    p = new_glyph_node();
    set_to_character(p);
    font(p) = f;
    character(p) = c;
    lang_data(p) = make_lang_data(uc_hyph_par, cur_lang_par, left_hyphen_min_par, right_hyphen_min_par);
    return p;
}

@ Left and right ghost glyph nodes are the result of \.{\\leftghost}
and \.{\\rightghost}, respectively. They are going to be removed by
|new_ligkern|, at the end of which they are no longer needed.

@ Here are a few handy helpers used by the list output routines.

@c
scaled glyph_width(halfword p)
{
    scaled w = char_width(font(p), character(p));
    return w;
}

scaled glyph_height(halfword p)
{
    scaled w = char_height(font(p), character(p)) + y_displace(p);
    if (w < 0)
        w = 0;
    return w;
}

scaled glyph_depth(halfword p)
{
    scaled w = char_depth(font(p), character(p));
    if (y_displace(p) > 0)
        w = w - y_displace(p);
    if (w < 0)
        w = 0;
    return w;
}

@ A |disc_node|, which occurs only in horizontal lists, specifies a
``dis\-cretion\-ary'' line break. If such a break occurs at node |p|, the text
that starts at |pre_break(p)| will precede the break, the text that starts at
|post_break(p)| will follow the break, and text that appears in
|no_break(p)| nodes will be ignored. For example, an ordinary
discretionary hyphen, indicated by `\.{\\-}', yields a |disc_node| with
|pre_break| pointing to a |char_node| containing a hyphen, |post_break=null|,
and |no_break=null|.

{TODO: Knuth said: All three of the discretionary texts must be lists
that consist entirely of character, kern, box and rule nodes.}

If |subtype(p)=automatic_disc|, the |ex_hyphen_penalty| will be charged for this
break.  Otherwise the |hyphen_penalty| will be charged.  The texts will
actually be substituted into the list by the line-breaking algorithm if it
decides to make the break, and the discretionary node will disappear at
that time; thus, the output routine sees only discretionaries that were
not chosen.

@c
halfword new_disc(void)
{                               /* creates an empty |disc_node| */
    halfword p = new_node(disc_node, 0);
    disc_penalty(p) = hyphen_penalty_par;
    return p;
}

@ A |whatsit_node| is a wild card reserved for extensions to \TeX. The
|subtype| field in its first word says what `\\{whatsit}' it is, and
implicitly determines the node size (which must be 2 or more) and the
format of the remaining words. When a |whatsit_node| is encountered
in a list, special actions are invoked; knowledgeable people who are
careful not to mess up the rest of \TeX\ are able to make \TeX\ do new
things by adding code at the end of the program. For example, there
might be a `\TeX nicolor' extension to specify different colors of ink,
@^extensions to \TeX@>
and the whatsit node might contain the desired parameters.

The present implementation of \TeX\ treats the features associated with
`\.{\\write}' and `\.{\\special}' as if they were extensions, in order to
illustrate how such routines might be coded. We shall defer further
discussion of extensions until the end of this program.

@ A |math_node|, which occurs only in horizontal lists, appears before and
after mathematical formulas. The |subtype| field is |before| before the
formula and |after| after it. There is a |surround| field, which represents
the amount of surrounding space inserted by \.{\\mathsurround}.

@c
halfword new_math(scaled w, int s)
{
    halfword p = new_node(math_node, s);
    surround(p) = w;
    return p;
}

@ \TeX\ makes use of the fact that |hlist_node|, |vlist_node|,
|rule_node|, |ins_node|, |mark_node|, |adjust_node|,
|disc_node|, |whatsit_node|, and |math_node| are at the low end of the
type codes, by permitting a break at glue in a list if and only if the
|type| of the previous node is less than |math_node|. Furthermore, a
node is discarded after a break if its type is |math_node| or~more.

@ A |glue_node| represents glue in a list. However, it is really only
a pointer to a separate glue specification, since \TeX\ makes use of the
fact that many essentially identical nodes of glue are usually present.
If |p| points to a |glue_node|, |glue_ptr(p)| points to
another packet of words that specify the stretch and shrink components, etc.

Glue nodes also serve to represent leaders; the |subtype| is used to
distinguish between ordinary glue (which is called |normal|) and the three
kinds of leaders (which are called |a_leaders|, |c_leaders|, and |x_leaders|).
The |leader_ptr| field points to a rule node or to a box node containing the
leaders; it is set to |null| in ordinary glue nodes.

Many kinds of glue are computed from \TeX's ``skip'' parameters, and
it is helpful to know which parameter has led to a particular glue node.
Therefore the |subtype| is set to indicate the source of glue, whenever
it originated as a parameter. We will be defining symbolic names for the
parameter numbers later (e.g., |line_skip_code=0|, |baseline_skip_code=1|,
etc.); it suffices for now to say that the |subtype| of parametric glue
will be the same as the parameter number, plus~one.

@ In math formulas there are two more possibilities for the |subtype| in a
glue node: |mu_glue| denotes an \.{\\mskip} (where the units are scaled \.{mu}
instead of scaled \.{pt}); and |cond_math_glue| denotes the `\.{\\nonscript}'
feature that cancels the glue node immediately following if it appears
in a subscript.

@ A glue specification has a halfword reference count in its first word,
@^reference counts@>
representing |null| plus the number of glue nodes that point to it (less one).
Note that the reference count appears in the same position as
the |link| field in list nodes; this is the field that is initialized
to |null| when a node is allocated, and it is also the field that is flagged
by |empty_flag| in empty nodes.

Glue specifications also contain three |scaled| fields, for the |width|,
|stretch|, and |shrink| dimensions. Finally, there are two one-byte
fields called |stretch_order| and |shrink_order|; these contain the
orders of infinity (|normal|, |sfi|, |fil|, |fill|, or |filll|)
corresponding to the stretch and shrink values.

@ Here is a function that returns a pointer to a copy of a glue spec.
The reference count in the copy is |null|, because there is assumed
to be exactly one reference to the new specification.

@c
halfword new_spec(halfword p)
{
    return copy_node(p == null ? zero_glue : p);
}

@ And here's a function that creates a glue node for a given parameter
identified by its code number; for example,
|new_param_glue(line_skip_code)| returns a pointer to a glue node for the
current \.{\\lineskip}.

@c
halfword new_param_glue(int n)
{
    halfword p = new_node(glue_node, n + 1);
    halfword q = glue_par(n);
    width(p) = width(q);
    stretch(p) = stretch(q);
    shrink(p) = shrink(q);
    stretch_order(p) = stretch_order(q);
    shrink_order(p) = shrink_order(q);
    return p;
}

@ Glue nodes that are more or less anonymous are created by |new_glue|,
whose argument points to a glue specification.

@c
halfword new_glue(halfword q)
{
    halfword p = new_node(glue_node, normal);
    width(p) = width(q);
    stretch(p) = stretch(q);
    shrink(p) = shrink(q);
    stretch_order(p) = stretch_order(q);
    shrink_order(p) = shrink_order(q);
    return p;
}

@ Still another subroutine is needed: This one is sort of a combination
of |new_param_glue| and |new_glue|. It creates a glue node for one of
the current glue parameters, but it makes a fresh copy of the glue
specification, since that specification will probably be subject to change,
while the parameter will stay put.

/*
    The global variable |temp_ptr| is set to the address of the new spec.
*/

@c
halfword new_skip_param(int n)
{
    halfword p = new_node(glue_node, n + 1);
    halfword q = glue_par(n);
    width(p) = width(q);
    stretch(p) = stretch(q);
    shrink(p) = shrink(q);
    stretch_order(p) = stretch_order(q);
    shrink_order(p) = shrink_order(q);
    return p;
}

@ A |kern_node| has a |width| field to specify a (normally negative)
amount of spacing. This spacing correction appears in horizontal lists
between letters like A and V when the font designer said that it looks
better to move them closer together or further apart. A kern node can
also appear in a vertical list, when its `|width|' denotes additional
spacing in the vertical direction. The |subtype| is either |normal| (for
kerns inserted from font information or math mode calculations) or |explicit|
(for kerns inserted from \.{\\kern} and \.{\\/} commands) or |acc_kern|
(for kerns inserted from non-math accents) or |mu_glue| (for kerns
inserted from \.{\\mkern} specifications in math formulas).

@ The |new_kern| function creates a kern node having a given width.

@c
halfword new_kern(scaled w)
{
    halfword p = new_node(kern_node, normal);
    width(p) = w;
    return p;
}

@ A |penalty_node| specifies the penalty associated with line or page
breaking, in its |penalty| field. This field is a fullword integer, but
the full range of integer values is not used: Any penalty |>=10000| is
treated as infinity, and no break will be allowed for such high values.
Similarly, any penalty |<=-10000| is treated as negative infinity, and a
break will be forced.

@ Anyone who has been reading the last few sections of the program will
be able to guess what comes next.

@c
halfword new_penalty(int m)
{
    halfword p = new_node(penalty_node, 0); /* the |subtype| is not used */
    penalty(p) = m;
    return p;
}

@ You might think that we have introduced enough node types by now. Well,
almost, but there is one more: An |unset_node| has nearly the same format
as an |hlist_node| or |vlist_node|; it is used for entries in \.{\\halign}
or \.{\\valign} that are not yet in their final form, since the box
dimensions are their ``natural'' sizes before any glue adjustment has been
made. The |glue_set| word is not present; instead, we have a |glue_stretch|
field, which contains the total stretch of order |glue_order| that is
present in the hlist or vlist being boxed.
Similarly, the |shift_amount| field is replaced by a |glue_shrink| field,
containing the total shrink of order |glue_sign| that is present.
The |subtype| field is called |span_count|; an unset box typically
contains the data for |qo(span_count)+1| columns.
Unset nodes will be changed to box nodes when alignment is completed.

In fact, there are still more types coming. When we get to math formula
processing we will see that a |style_node| has |type=14|; and a number
of larger type codes will also be defined, for use in math mode only.

Warning: If any changes are made to these data structure layouts, such as
changing any of the node sizes or even reordering the words of nodes,
the |copy_node_list| procedure and the memory initialization code
below may have to be changed. Such potentially dangerous parts of the
program are listed in the index under `data structure assumptions'.
@!@^data structure assumptions@>
However, other references to the nodes are made symbolically in terms of
the \.{WEB} macro definitions above, so that format changes will leave
\TeX's other algorithms intact.
@^system dependencies@>

@ This function creates a |local_paragraph| node

@c

halfword make_local_par_node(int mode)
{
    int callback_id;
    halfword q;
    halfword p = new_node(local_par_node,0);
    local_pen_inter(p) = local_inter_line_penalty_par;
    local_pen_broken(p) = local_broken_penalty_par;
    if (local_left_box_par != null) {
        q = copy_node_list(local_left_box_par);
        local_box_left(p) = q;
        local_box_left_width(p) = width(local_left_box_par);
    }
    if (local_right_box_par != null) {
        q = copy_node_list(local_right_box_par);
        local_box_right(p) = q;
        local_box_right_width(p) = width(local_right_box_par);
    }
    local_par_dir(p) = par_direction_par;
    /* callback with node passed */
    callback_id = callback_defined(insert_local_par_callback);
    if (callback_id > 0) {
        int sfix = lua_gettop(Luas);
        if (!get_callback(Luas, callback_id)) {
            lua_settop(Luas, sfix);
            return p;
        }
        nodelist_to_lua(Luas, p);
        lua_push_local_par_mode(Luas,mode)
        if (lua_pcall(Luas, 2, 0, 0) != 0) { /* 2 arg, 0 result */
            char errmsg[256]; /* temp hack ... we will have a formatted error */
            snprintf(errmsg, 255, "error: %s\n", lua_tostring(Luas, -1));
            errmsg[255]='\0';
            lua_settop(Luas, sfix);
            normal_error("insert_local_par",errmsg); /* to be done */
            return p;
        }
        lua_settop(Luas, sfix);
    }
    /* done */
    return p;
}
