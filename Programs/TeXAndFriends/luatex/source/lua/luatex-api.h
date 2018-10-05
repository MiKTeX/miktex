/* luatex-api.h

   Copyright 2006-2012 Taco Hoekwater <taco@luatex.org>

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

#ifndef LUATEX_API_H
#  define LUATEX_API_H 1

/* output modes, a bad place but this compiles at least */

typedef enum { OMODE_NONE, OMODE_DVI, OMODE_PDF } output_mode ;

#  define MAX_OMODE 2           /* largest index in enum output_mode */

extern int output_mode_used;
extern int output_mode_option;
extern int output_mode_value;
extern int draft_mode_option;
extern int draft_mode_value;

/* get_o_mode translates from output_mode to output_mode_used */
/* fix_o_mode freezes output_mode as soon as anything goes through the backend */

extern output_mode get_o_mode(void);
extern void fix_o_mode(void);

/* till here */

#  include <stdlib.h>
#  include <stdio.h>
#  include <stdarg.h>
#  include "lua.h"
#  include "lauxlib.h"
#  include "lualib.h"
#ifdef LuajitTeX
#  include "luajit.h"
#  define MyName "LuajitTeX"
#  define my_name "luajittex"
#else
#  define MyName "LuaTeX"
#  define my_name "luatex"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LoadS {
    char *s;
    size_t size;
} LoadS;

extern lua_State *Luas;

extern void make_table(lua_State * L, const char *tab, const char *mttab, const char *getfunc,
                       const char *setfunc);

extern int luac_main(int argc, char *argv[]);

extern int luaopen_tex(lua_State * L);
extern int luaopen_pdf(lua_State * L);
extern int luaopen_texio(lua_State * L);
extern int luaopen_lang(lua_State * L);

#  define LUA_TEXFILEHANDLE "TEXFILE*"

extern lua_State *luatex_error(lua_State * L, int fatal);

extern int luaopen_unicode(lua_State * L);
extern int luaopen_zip(lua_State * L);
extern int luaopen_lfs(lua_State * L);
extern int luaopen_lpeg(lua_State * L);
extern int luaopen_md5(lua_State * L);

#ifndef LuajitTeX
 extern int luaopen_ffi(lua_State * L);
#endif

extern int luaopen_zlib(lua_State * L);
extern int luaopen_gzip(lua_State * L);
extern int luaopen_ff(lua_State * L);
extern int luaopen_profiler(lua_State * L);

extern int luaopen_socket_core(lua_State * L);
extern int luaopen_mime_core(lua_State * L);
extern void luatex_socketlua_open(lua_State * L);

extern int luaopen_img(lua_State * L);
extern int l_new_image(lua_State * L);
extern int luaopen_epdf(lua_State * L);
extern int luaopen_pdfscanner(lua_State * L);
extern int luaopen_mplib(lua_State * L);
extern int luaopen_fio(lua_State * L);

extern void open_oslibext(lua_State * L);
extern void open_strlibext(lua_State * L);
extern void open_lfslibext(lua_State * L);

extern void initfilecallbackids(int max);
extern void setinputfilecallbackid(int n, int i);
extern void setreadfilecallbackid(int n, int i);
extern int getinputfilecallbackid(int n);
extern int getreadfilecallbackid(int n);

extern void lua_initialize(int ac, char **av);

extern int luaopen_kpse(lua_State * L);

extern int luaopen_callback(lua_State * L);

extern int luaopen_lua(lua_State * L, char *fname);

extern int luaopen_stats(lua_State * L);

extern int luaopen_font(lua_State * L);
extern int luaopen_vf(lua_State * L);
extern int font_to_lua(lua_State * L, int f);
extern int font_from_lua(lua_State * L, int f); /* return is boolean */
extern int characters_from_lua(lua_State * L, int f); /* return is boolean */

extern int luaopen_token(lua_State * L);
extern void tokenlist_to_lua(lua_State * L, int p);
extern void tokenlist_to_luastring(lua_State * L, int p);
extern int tokenlist_from_lua(lua_State * L);

extern void lua_nodelib_push(lua_State * L);
extern int nodelib_getdir(lua_State * L, int n, int absolute_only);
extern int nodelib_getlist(lua_State * L, int n);

extern int luaopen_node(lua_State * L);
extern void nodelist_to_lua(lua_State * L, int n);
extern int nodelist_from_lua(lua_State * L);

extern int dimen_to_number(lua_State * L, const char *s);

extern int get_command_id(const char *s);

extern void dump_luac_registers(void);

extern void undump_luac_registers(void);

extern int lua_only;
extern const char *lc_ctype;
extern const char *lc_collate;
extern const char *lc_numeric;



#ifdef LuajitTeX
extern int luajiton;
extern char *jithash_hashname ;
#endif

#if !defined(LUAI_HASHLIMIT)
#define LUAI_HASHLIMIT		5
#endif
extern unsigned char show_luahashchars ;

extern void unhide_lua_table(lua_State * lua, const char *name, int r);
extern int hide_lua_table(lua_State * lua, const char *name);

extern void unhide_lua_value(lua_State * lua, const char *name, const char *item, int r);
extern int hide_lua_value(lua_State * lua, const char *name, const char *item);

typedef struct command_item_ {
    const char *cmd_name;
    int command_offset;
    const char **commands;
} command_item;

extern command_item command_names[];
extern int callback_callbacks_id;

extern void luainterpreter(void);

extern int luabytecode_max;
extern unsigned int luabytecode_bytes;
extern int luastate_bytes;

extern int callback_count;
extern int saved_callback_count;

extern const char *luatex_banner;
extern const char *engine_name;

/* luastuff.h */

typedef struct {
    const char *name;           /* parameter name */
    int idx;                    /* index within img_parms array */
} parm_struct;

extern void preset_environment(lua_State * L, const parm_struct * p, const char *s);

extern char *startup_filename;
extern int safer_option;
extern int nosocket_option;
extern int utc_option;

extern char *last_source_name;
extern int last_lineno;

extern int program_name_set;    /* in lkpselib.c */

/* for topenin() */
extern char **argv;
extern int argc;

extern int loader_C_luatex(lua_State * L, const char *name, const char *filename);
extern int loader_Call_luatex(lua_State * L, const char *name, const char *filename);

extern void init_tex_table(lua_State * L);

/*
extern int tex_table_id;
extern int pdf_table_id;
extern int token_table_id;
extern int node_table_id;
*/

extern int main_initialize(void);

extern int do_run_callback(int special, const char *values, va_list vl);
extern int lua_traceback(lua_State * L);

extern int luainit;

extern char *luanames[];

extern int ff_get_ttc_index(char *ffname, char *psname);        /* luafontloader/src/luafflib.c */
extern int ff_createcff(char *, unsigned char **, int *);       /* luafontloader/src/luafflib.c */

extern char *FindResourceTtfFont(char *filename, char *fontname);       /* luafontloader/fontforge/fontforge/macbinary.c */

extern char charsetstr[];       /* from mpdir/psout.w */

#ifndef WIN32
extern char **environ;
#endif

#ifdef __cplusplus
}
#endif

extern int luatwrite(lua_State * L);

/*
    Same as in lnodelib.c, but with prefix G_ for now.
    These macros create and access pointers (indices) to keys which is faster. The
    shortcuts are created as part of the initialization.

*/

#define init_lua_key(a) do {                      \
    lua_pushliteral(Luas,#a);                             \
    luaS_##a##_ptr = lua_tostring(Luas,-1);               \
    luaS_##a##_index = luaL_ref (Luas,LUA_REGISTRYINDEX); \
} while (0)

#define init_lua_key_alias(a,b) do {              \
    lua_pushliteral(Luas,b);                              \
    luaS_##a##_ptr = lua_tostring(Luas,-1);               \
    luaS_##a##_index = luaL_ref (Luas,LUA_REGISTRYINDEX); \
} while (0)

#define make_lua_key(a)       \
    int luaS_##a##_index = 0;          \
    const char * luaS_##a##_ptr = NULL

#define lua_key_eq(a,b) (a==luaS_##b##_ptr)

#define lua_key_index(a) luaS_##a##_index
#define lua_key(a) luaS_##a##_ptr
#define lua_key_plus(a) luaS_p##a##_ptr
#define lua_key_minus(a) luaS_m##a##_ptr
#define use_lua_key(a)  \
  extern int luaS_##a##_index ;          \
  extern const char * luaS_##a##_ptr

#define lua_key_rawgeti(a) \
  lua_rawgeti(L, LUA_REGISTRYINDEX, luaS_##a##_index);\
  lua_rawget(L, -2)

#define lua_key_direct_rawgeti(i) \
  lua_rawgeti(L, LUA_REGISTRYINDEX, i);\
  lua_rawget(L, -2)

#define lua_key_rawgeti_n(a,n) \
  lua_rawgeti(L, LUA_REGISTRYINDEX, luaS_##a##_index);\
  lua_rawget(L, -1+n)

#define lua_key_direct_rawget_n(i,n) \
  lua_rawgeti(L, LUA_REGISTRYINDEX, i);\
  lua_rawget(L, -1+n)

#define lua_push_key(a) \
  lua_rawgeti(L, LUA_REGISTRYINDEX, luaS_##a##_index);

#define lua_get_metatablelua_l(L,a) do {  \
    lua_rawgeti(L, LUA_REGISTRYINDEX, luaS_##a##_index); \
    lua_gettable(L, LUA_REGISTRYINDEX); \
} while (0)

#define lua_get_metatablelua(a) lua_get_metatablelua_l(L,a)

/*
Unfortunately floor is already redefined as
#define floor ((integer)floor((double)(a)))
so
#define lua_uroundnumber(a,b) (unsigned int)floor((double)(lua_tonumber(a,b)+0.5))
is useless.
*/

#define lua_roundnumber(a,b)  (int)floor((double)lua_tonumber(a,b)+0.5)
#define lua_uroundnumber(a,b) (unsigned int)((double)(lua_tonumber(a,b)+0.5))
extern int lua_numeric_field_by_index(lua_State *, int , int);
extern unsigned int lua_unsigned_numeric_field_by_index(lua_State *, int , int);

/* Currently we sometimes use numbers and sometimes strings in node properties. We can
make that consistent by having a check on number and if not then assign a string. The
strings are prehashed and we make a bunch of lua tables that have these values. We can
preassign these at startup time. */

/* no need for L state argument */

#define PACK_TYPE_SIZE        4
#define GROUP_CODE_SIZE      23
#define LOCAL_PAR_SIZE        5
#define MATH_STYLE_NAME_SIZE  8
#define APPEND_LIST_SIZE      5
#define DIR_PAR_SIZE          8
#define DIR_TEXT_SIZE         8

extern int l_pack_type_index       [PACK_TYPE_SIZE];
extern int l_group_code_index      [GROUP_CODE_SIZE];
extern int l_local_par_index       [LOCAL_PAR_SIZE];
extern int l_math_style_name_index [MATH_STYLE_NAME_SIZE];
extern int l_dir_par_index         [DIR_PAR_SIZE];
extern int l_dir_text_index        [DIR_TEXT_SIZE];

#define lua_push_pack_type(L,pack_type)        lua_rawgeti(L, LUA_REGISTRYINDEX, l_pack_type_index[pack_type] );
#define lua_push_group_code(L,group_code)      lua_rawgeti(L, LUA_REGISTRYINDEX, l_group_code_index[group_code]);
#define lua_push_local_par_mode(L,par_mode)    lua_rawgeti(L, LUA_REGISTRYINDEX, l_local_par_index[par_mode]);
#define lua_push_math_style_name(L,style_name) lua_rawgeti(L, LUA_REGISTRYINDEX, l_math_style_name_index[style_name]);
#define lua_push_dir_par(L,dir)                lua_rawgeti(L, LUA_REGISTRYINDEX, l_dir_par_index[dir+dir_swap])
#define lua_push_dir_text(L,dir)               lua_rawgeti(L, LUA_REGISTRYINDEX, l_dir_text_index[dir+dir_swap])

#define lua_push_string_by_index(L,index)      lua_rawgeti(L, LUA_REGISTRYINDEX, index)
#define lua_push_string_by_name(L,index)       lua_rawgeti(L, LUA_REGISTRYINDEX, lua_key_index(index))

#define set_l_pack_type_index \
l_pack_type_index[exactly]          = lua_key_index(exactly); \
l_pack_type_index[additional]       = lua_key_index(additional); \
l_pack_type_index[cal_expand_ratio] = lua_key_index(cal_expand_ratio);\
l_pack_type_index[subst_ex_font]    = lua_key_index(subst_ex_font);

#define set_l_group_code_index \
l_group_code_index[bottom_level]        = lua_key_index(empty_string);\
l_group_code_index[simple_group]        = lua_key_index(simple);\
l_group_code_index[hbox_group]          = lua_key_index(hbox);\
l_group_code_index[adjusted_hbox_group] = lua_key_index(adjusted_hbox);\
l_group_code_index[vbox_group]          = lua_key_index(vbox);\
l_group_code_index[vtop_group]          = lua_key_index(vtop);\
l_group_code_index[align_group]         = lua_key_index(align);\
l_group_code_index[no_align_group]      = lua_key_index(no_align);\
l_group_code_index[output_group]        = lua_key_index(output);\
l_group_code_index[math_group]          = lua_key_index(math);\
l_group_code_index[disc_group]          = lua_key_index(disc);\
l_group_code_index[insert_group]        = lua_key_index(insert);\
l_group_code_index[vcenter_group]       = lua_key_index(vcenter);\
l_group_code_index[math_choice_group]   = lua_key_index(math_choice);\
l_group_code_index[semi_simple_group]   = lua_key_index(semi_simple);\
l_group_code_index[math_shift_group]    = lua_key_index(math_shift);\
l_group_code_index[math_left_group]     = lua_key_index(math_left);\
l_group_code_index[local_box_group]     = lua_key_index(local_box);\
l_group_code_index[split_off_group]     = lua_key_index(split_off);\
l_group_code_index[split_keep_group]    = lua_key_index(split_keep);\
l_group_code_index[preamble_group]      = lua_key_index(preamble);\
l_group_code_index[align_set_group]     = lua_key_index(align_set);\
l_group_code_index[fin_row_group]       = lua_key_index(fin_row)

#define set_l_local_par_index \
l_local_par_index[new_graf_par_code]  = lua_key_index(new_graf);\
l_local_par_index[local_box_par_code] = lua_key_index(local_box);\
l_local_par_index[hmode_par_par_code] = lua_key_index(hmode_par);\
l_local_par_index[penalty_par_code]   = lua_key_index(penalty);\
l_local_par_index[math_par_code]      = lua_key_index(math);

#define set_l_math_style_name_index \
l_math_style_name_index[display_style]               = lua_key_index(display);\
l_math_style_name_index[cramped_display_style]       = lua_key_index(crampeddisplay);\
l_math_style_name_index[text_style]                  = lua_key_index(text);\
l_math_style_name_index[cramped_text_style]          = lua_key_index(crampedtext);\
l_math_style_name_index[script_style]                = lua_key_index(script);\
l_math_style_name_index[cramped_script_style]        = lua_key_index(crampedscript);\
l_math_style_name_index[script_script_style]         = lua_key_index(scriptscript);\
l_math_style_name_index[cramped_script_script_style] = lua_key_index(crampedscriptscript)

#define set_l_dir_par_index \
l_dir_par_index[dir_TLT]   = lua_key_index(TLT);\
l_dir_par_index[dir_TRT]   = lua_key_index(TRT);\
l_dir_par_index[dir_LTL]   = lua_key_index(LTL);\
l_dir_par_index[dir_RTT]   = lua_key_index(RTT);\
l_dir_par_index[dir_TLT+4] = lua_key_index(TLT);\
l_dir_par_index[dir_TRT+4] = lua_key_index(TRT);\
l_dir_par_index[dir_LTL+4] = lua_key_index(LTL);\
l_dir_par_index[dir_RTT+4] = lua_key_index(RTT);\

#define set_l_dir_text_index \
l_dir_text_index[dir_TLT]   = lua_key_index(mTLT);\
l_dir_text_index[dir_TRT]   = lua_key_index(mTRT);\
l_dir_text_index[dir_LTL]   = lua_key_index(mLTL);\
l_dir_text_index[dir_RTT]   = lua_key_index(mRTT);\
l_dir_text_index[dir_TLT+4] = lua_key_index(pTLT);\
l_dir_text_index[dir_TRT+4] = lua_key_index(pTRT);\
l_dir_text_index[dir_LTL+4] = lua_key_index(pLTL);\
l_dir_text_index[dir_RTT+4] = lua_key_index(pRTT);\

#define img_parms_max     25
#define img_pageboxes_max  6

extern int img_parms     [img_parms_max];
extern int img_pageboxes [img_pageboxes_max];

# define set_l_img_keys_index \
img_parms[ 0] = lua_key_index(attr); \
img_parms[ 1] = lua_key_index(bbox); \
img_parms[ 2] = lua_key_index(colordepth); \
img_parms[ 3] = lua_key_index(colorspace); \
img_parms[ 4] = lua_key_index(depth); \
img_parms[ 5] = lua_key_index(filename); \
img_parms[ 6] = lua_key_index(filepath); \
img_parms[ 7] = lua_key_index(height); \
img_parms[ 8] = lua_key_index(imagetype); \
img_parms[ 9] = lua_key_index(index); \
img_parms[10] = lua_key_index(keepopen); \
img_parms[11] = lua_key_index(objnum); \
img_parms[12] = lua_key_index(pagebox); \
img_parms[13] = lua_key_index(page); \
img_parms[14] = lua_key_index(pages); \
img_parms[15] = lua_key_index(ref_count); \
img_parms[16] = lua_key_index(rotation); \
img_parms[17] = lua_key_index(stream); \
img_parms[18] = lua_key_index(transform); \
img_parms[19] = lua_key_index(visiblefilename); \
img_parms[20] = lua_key_index(width); \
img_parms[21] = lua_key_index(xres); \
img_parms[22] = lua_key_index(xsize); \
img_parms[23] = lua_key_index(yres); \
img_parms[24] = lua_key_index(ysize); \

# define set_l_img_pageboxes_index \
img_pageboxes[0] = lua_key_index(none); \
img_pageboxes[1] = lua_key_index(media); \
img_pageboxes[2] = lua_key_index(crop); \
img_pageboxes[3] = lua_key_index(bleed); \
img_pageboxes[4] = lua_key_index(trim); \
img_pageboxes[5] = lua_key_index(art); \

#define lua_push_img_key(L,key)     lua_rawgeti(L, LUA_REGISTRYINDEX, img_parms[key] );
#define lua_push_img_pagebox(L,box) lua_rawgeti(L, LUA_REGISTRYINDEX, img_pageboxes[box]);

extern int lua_show_valid_list(lua_State *L, const char **list, int max);
extern int lua_show_valid_keys(lua_State *L, int *list, int max);

#define set_make_keys \
make_lua_key(cmdname);make_lua_key(expandable);make_lua_key(protected);\
make_lua_key(LTL);\
make_lua_key(MathConstants);\
make_lua_key(RTT);\
make_lua_key(TLT);\
make_lua_key(TRT);\
make_lua_key(accent);\
make_lua_key(action);\
make_lua_key(action_id);\
make_lua_key(action_type);\
make_lua_key(active);\
make_lua_key(additional);\
make_lua_key(adjdemerits);\
make_lua_key(adjust);\
make_lua_key(adjust_head);\
make_lua_key(adjusted_hbox);\
make_lua_key(adjustspacing);\
make_lua_key(advance);\
make_lua_key(after_display);\
make_lua_key(after_output);\
make_lua_key(align);\
make_lua_key(align_head);\
make_lua_key(align_set);\
make_lua_key(alignment);\
make_lua_key(annot);\
make_lua_key(area);\
make_lua_key(art);\
make_lua_key(attr);\
make_lua_key(attributes);\
make_lua_key(bbox);\
make_lua_key(before_display);\
make_lua_key(best_ins_ptr);\
make_lua_key(best_page_break);\
make_lua_key(best_size);\
make_lua_key(bleed);\
make_lua_key(bot);\
make_lua_key(bot_accent);\
make_lua_key(bottom_left);\
make_lua_key(bottom_right);\
make_lua_key(boundary);\
make_lua_key(box);\
make_lua_key(box_left);\
make_lua_key(box_left_width);\
make_lua_key(box_right);\
make_lua_key(box_right_width);\
make_lua_key(broken_ins);\
make_lua_key(broken_ptr);\
make_lua_key(brokenpenalty);\
make_lua_key(cache);\
make_lua_key(cal_expand_ratio);\
make_lua_key(catalog);\
make_lua_key(char);\
make_lua_key(characters);\
make_lua_key(checksum);\
make_lua_key(cidinfo);\
make_lua_key(class);\
make_lua_key(clubpenalty);\
make_lua_key(colordepth);\
make_lua_key(colorspace);\
make_lua_key(command);\
make_lua_key(commands);\
make_lua_key(comment);\
make_lua_key(components);\
make_lua_key(compresslevel);\
make_lua_key(contrib_head);\
make_lua_key(core);\
make_lua_key(cost);\
make_lua_key(count);\
make_lua_key(crampeddisplay);\
make_lua_key(crampedscript);\
make_lua_key(crampedscriptscript);\
make_lua_key(crampedtext);\
make_lua_key(crop);\
make_lua_key(csname);\
make_lua_key(data);\
make_lua_key(degree);\
make_lua_key(delim);\
make_lua_key(delimptr);\
make_lua_key(demerits);\
make_lua_key(denom);\
make_lua_key(depth);\
make_lua_key(designsize);\
make_lua_key(dest_id);\
make_lua_key(dest_type);\
make_lua_key(dir);\
make_lua_key(dir_h);\
make_lua_key(direct);\
make_lua_key(direction);\
make_lua_key(dirs);\
make_lua_key(disc);\
make_lua_key(display);\
make_lua_key(doublehyphendemerits);\
make_lua_key(down);\
make_lua_key(embedding);\
make_lua_key(emergencystretch);\
make_lua_key(empty_string);\
make_lua_key(encodingbytes);\
make_lua_key(encodingname);\
make_lua_key(end);\
make_lua_key(etex);\
make_lua_key(equation);\
make_lua_key(equation_number);\
make_lua_key(exactly);\
make_lua_key(expansion_factor);\
make_lua_key(ext);\
make_lua_key(extend);\
make_lua_key(extender);\
make_lua_key(extensible);\
make_lua_key(extra_space);\
make_lua_key(fam);\
make_lua_key(fast);\
make_lua_key(fence);\
make_lua_key(file);\
make_lua_key(filename);\
make_lua_key(filepath);\
make_lua_key(fill);\
make_lua_key(fin_row);\
make_lua_key(finalhyphendemerits);\
make_lua_key(font);\
make_lua_key(fonts);\
make_lua_key(format);\
make_lua_key(fraction);\
make_lua_key(fullname);\
make_lua_key(global);\
make_lua_key(glue);\
make_lua_key(glue_order);\
make_lua_key(glue_set);\
make_lua_key(glue_sign);\
make_lua_key(glyph);\
make_lua_key(h);\
make_lua_key(hangafter);\
make_lua_key(hangindent);\
make_lua_key(hbox);\
make_lua_key(head);\
make_lua_key(height);\
make_lua_key(hlist);\
make_lua_key(hmode_par);\
make_lua_key(hold_head);\
make_lua_key(horiz_variants);\
make_lua_key(hsize);\
make_lua_key(hyphenchar);\
make_lua_key(id);\
make_lua_key(identity);\
make_lua_key(image);\
make_lua_key(imagetype);\
make_lua_key(immediate);\
make_lua_key(index);\
make_lua_key(info);\
make_lua_key(insert);\
make_lua_key(interlinepenalty);\
make_lua_key(italic);\
make_lua_key(keepopen);\
make_lua_key(kern);\
make_lua_key(kerns);\
make_lua_key(lang);\
make_lua_key(large_char);\
make_lua_key(large_fam);\
make_lua_key(last_ins_ptr);\
make_lua_key(lastlinefit);\
make_lua_key(leader);\
make_lua_key(least_page_cost);\
make_lua_key(left);\
make_lua_key(left_boundary);\
make_lua_key(left_protruding);\
make_lua_key(leftskip);\
make_lua_key(level);\
make_lua_key(ligatures);\
make_lua_key(linepenalty);\
make_lua_key(link_attr);\
make_lua_key(list);\
make_lua_key(local_box);\
make_lua_key(log);\
make_lua_key(looseness);\
make_lua_key(lua);\
make_lua_key(lua_bytecodes_indirect);\
make_lua_key(lua_functions);\
make_lua_key(luatex);\
make_lua_key(luatex_node);\
make_lua_key(luatex_token);\
make_lua_key(mLTL);\
make_lua_key(mRTT);\
make_lua_key(mTLT);\
make_lua_key(mTRT);\
make_lua_key(marginkern);\
make_lua_key(mark);\
make_lua_key(math);\
make_lua_key(math_choice);\
make_lua_key(math_left);\
make_lua_key(math_shift);\
make_lua_key(mathdir);\
make_lua_key(mathkern);\
make_lua_key(mathstyle);\
make_lua_key(media);\
make_lua_key(mid);\
make_lua_key(middle);\
make_lua_key(mode);\
make_lua_key(modeline);\
make_lua_key(name);\
make_lua_key(named_id);\
make_lua_key(names);\
make_lua_key(new_graf);\
make_lua_key(new_window);\
make_lua_key(next);\
make_lua_key(no);\
make_lua_key(no_align);\
make_lua_key(noad);\
make_lua_key(node);\
make_lua_key(node_properties);\
make_lua_key(node_properties_indirect);\
make_lua_key(nomath);\
make_lua_key(none);\
make_lua_key(nop);\
make_lua_key(nucleus);\
make_lua_key(num);\
make_lua_key(number);\
make_lua_key(objcompression);\
make_lua_key(objnum);\
make_lua_key(oldmath);\
make_lua_key(ordering);\
make_lua_key(options);\
make_lua_key(orientation);\
make_lua_key(origin);\
make_lua_key(output);\
make_lua_key(overlay_accent);\
make_lua_key(pLTL);\
make_lua_key(pRTT);\
make_lua_key(pTLT);\
make_lua_key(pTRT);\
make_lua_key(page);\
make_lua_key(pages);\
make_lua_key(page_discards_head);\
make_lua_key(page_head);\
make_lua_key(page_ins_head);\
make_lua_key(pageattributes);\
make_lua_key(pagebox);\
make_lua_key(pageresources);\
make_lua_key(pagesattributes);\
make_lua_key(parameters);\
make_lua_key(pardir);\
make_lua_key(parshape);\
make_lua_key(pdf);\
make_lua_key(pdf_data);\
make_lua_key(pdf_destination);\
make_lua_key(pdf_literal);\
make_lua_key(pen_broken);\
make_lua_key(pen_inter);\
make_lua_key(penalty);\
make_lua_key(pop);\
make_lua_key(post);\
make_lua_key(post_linebreak);\
make_lua_key(pre);\
make_lua_key(pre_adjust);\
make_lua_key(pre_adjust_head);\
make_lua_key(pre_align);\
make_lua_key(pre_box);\
make_lua_key(preamble);\
make_lua_key(pretolerance);\
make_lua_key(prev);\
make_lua_key(prevdepth);\
make_lua_key(prevgraf);\
make_lua_key(protrudechars);\
make_lua_key(psname);\
make_lua_key(ptr);\
make_lua_key(push);\
make_lua_key(quad);\
make_lua_key(radical);\
make_lua_key(raw);\
make_lua_key(ref_count);\
make_lua_key(reg);\
make_lua_key(registry);\
make_lua_key(renew);\
make_lua_key(rep);\
make_lua_key(replace);\
make_lua_key(resources);\
make_lua_key(right);\
make_lua_key(right_boundary);\
make_lua_key(right_protruding);\
make_lua_key(rightskip);\
make_lua_key(rotation);\
make_lua_key(rule);\
make_lua_key(scale);\
make_lua_key(script);\
make_lua_key(scriptscript);\
make_lua_key(semi_simple);\
make_lua_key(shift);\
make_lua_key(shrink);\
make_lua_key(shrink_order);\
make_lua_key(simple);\
make_lua_key(size);\
make_lua_key(skewchar);\
make_lua_key(slant);\
make_lua_key(slot);\
make_lua_key(small_char);\
make_lua_key(small_fam);\
make_lua_key(space);\
make_lua_key(space_shrink);\
make_lua_key(space_stretch);\
make_lua_key(spacefactor);\
make_lua_key(special);\
make_lua_key(split_discards_head);\
make_lua_key(split_keep);\
make_lua_key(split_off);\
make_lua_key(stack);\
make_lua_key(start);\
make_lua_key(step);\
make_lua_key(stream);\
make_lua_key(streamfile);\
make_lua_key(streamprovider);\
make_lua_key(stretch);\
make_lua_key(stretch_order);\
make_lua_key(string);\
make_lua_key(style);\
make_lua_key(sub);\
make_lua_key(subst_ex_font);\
make_lua_key(subtype);\
make_lua_key(sup);\
make_lua_key(supplement);\
make_lua_key(surround);\
make_lua_key(tail);\
make_lua_key(temp_head);\
make_lua_key(term);\
make_lua_key(term_and_log);\
make_lua_key(tex);\
make_lua_key(text);\
make_lua_key(thread_attr);\
make_lua_key(thread_id);\
make_lua_key(tolerance);\
make_lua_key(tok);\
make_lua_key(token);\
make_lua_key(top);\
make_lua_key(top_accent);\
make_lua_key(top_left);\
make_lua_key(top_right);\
make_lua_key(tounicode);\
make_lua_key(tracingparagraphs);\
make_lua_key(trailer);\
make_lua_key(trailerid);\
make_lua_key(transform);\
make_lua_key(trim);\
make_lua_key(type);\
make_lua_key(uchyph);\
make_lua_key(units_per_em);\
make_lua_key(used);\
make_lua_key(user_id);\
make_lua_key(v);\
make_lua_key(value);\
make_lua_key(vbox);\
make_lua_key(vcenter);\
make_lua_key(version);\
make_lua_key(vert_italic);\
make_lua_key(vert_variants);\
make_lua_key(vmode_par);\
make_lua_key(visiblefilename);\
make_lua_key(vlist);\
make_lua_key(vtop);\
make_lua_key(widowpenalty);\
make_lua_key(width);\
make_lua_key(x_height);\
make_lua_key(xadvance);\
make_lua_key(xformresources);\
make_lua_key(xformattributes);\
make_lua_key(xoffset);\
make_lua_key(xres);\
make_lua_key(xsize);\
make_lua_key(xyz_zoom);\
make_lua_key(yoffset); \
make_lua_key(yres); \
make_lua_key(ysize); \
make_lua_key(writingmode); \
make_lua_key(__index)

#define set_init_keys \
init_lua_key(cmdname);init_lua_key(expandable);init_lua_key(protected);\
init_lua_key(LTL);\
init_lua_key(MathConstants);\
init_lua_key(RTT);\
init_lua_key(TLT);\
init_lua_key(TRT);\
init_lua_key(accent);\
init_lua_key(action);\
init_lua_key(action_id);\
init_lua_key(action_type);\
init_lua_key(active);\
init_lua_key(additional);\
init_lua_key(adjdemerits);\
init_lua_key(adjust);\
init_lua_key(adjust_head);\
init_lua_key(adjusted_hbox);\
init_lua_key(adjustspacing);\
init_lua_key(advance);\
init_lua_key(after_display);\
init_lua_key(after_output);\
init_lua_key(align);\
init_lua_key(align_head);\
init_lua_key(align_set);\
init_lua_key(alignment);\
init_lua_key(annot);\
init_lua_key(area);\
init_lua_key(art);\
init_lua_key(attr);\
init_lua_key(attributes);\
init_lua_key(bbox);\
init_lua_key(before_display);\
init_lua_key(best_ins_ptr);\
init_lua_key(best_page_break);\
init_lua_key(best_size);\
init_lua_key(bleed);\
init_lua_key(bot);\
init_lua_key(bot_accent);\
init_lua_key(bottom_left);\
init_lua_key(bottom_right);\
init_lua_key(boundary);\
init_lua_key(box);\
init_lua_key(box_left);\
init_lua_key(box_left_width);\
init_lua_key(box_right);\
init_lua_key(box_right_width);\
init_lua_key(broken_ins);\
init_lua_key(broken_ptr);\
init_lua_key(brokenpenalty);\
init_lua_key(cache);\
init_lua_key(cal_expand_ratio);\
init_lua_key(catalog);\
init_lua_key(char);\
init_lua_key(characters);\
init_lua_key(checksum);\
init_lua_key(cidinfo);\
init_lua_key(class);\
init_lua_key(clubpenalty);\
init_lua_key(colordepth);\
init_lua_key(colorspace);\
init_lua_key(command);\
init_lua_key(commands);\
init_lua_key(comment);\
init_lua_key(components);\
init_lua_key(compresslevel);\
init_lua_key(contrib_head);\
init_lua_key(core);\
init_lua_key(cost);\
init_lua_key(count);\
init_lua_key(crampeddisplay);\
init_lua_key(crampedscript);\
init_lua_key(crampedscriptscript);\
init_lua_key(crampedtext);\
init_lua_key(crop);\
init_lua_key(csname);\
init_lua_key(data);\
init_lua_key(degree);\
init_lua_key(delim);\
init_lua_key(delimptr);\
init_lua_key(demerits);\
init_lua_key(denom);\
init_lua_key(depth);\
init_lua_key(designsize);\
init_lua_key(dest_id);\
init_lua_key(dest_type);\
init_lua_key(dir);\
init_lua_key(dir_h);\
init_lua_key(direct);\
init_lua_key(direction);\
init_lua_key(dirs);\
init_lua_key(disc);\
init_lua_key(display);\
init_lua_key(doublehyphendemerits);\
init_lua_key(down);\
init_lua_key(embedding);\
init_lua_key(emergencystretch);\
init_lua_key(encodingbytes);\
init_lua_key(encodingname);\
init_lua_key(end);\
init_lua_key(etex);\
init_lua_key(equation);\
init_lua_key(equation_number);\
init_lua_key(exactly);\
init_lua_key(expansion_factor);\
init_lua_key(ext);\
init_lua_key(extend);\
init_lua_key(extender);\
init_lua_key(extensible);\
init_lua_key(extra_space);\
init_lua_key(fam);\
init_lua_key(fast);\
init_lua_key(fence);\
init_lua_key(file);\
init_lua_key(filename);\
init_lua_key(filepath);\
init_lua_key(fill);\
init_lua_key(fin_row);\
init_lua_key(finalhyphendemerits);\
init_lua_key(font);\
init_lua_key(fonts);\
init_lua_key(format);\
init_lua_key(fraction);\
init_lua_key(fullname);\
init_lua_key(global);\
init_lua_key(glue);\
init_lua_key(glue_order);\
init_lua_key(glue_set);\
init_lua_key(glue_sign);\
init_lua_key(glyph);\
init_lua_key(h);\
init_lua_key(hangafter);\
init_lua_key(hangindent);\
init_lua_key(hbox);\
init_lua_key(head);\
init_lua_key(height);\
init_lua_key(hlist);\
init_lua_key(hmode_par);\
init_lua_key(hold_head);\
init_lua_key(horiz_variants);\
init_lua_key(hsize);\
init_lua_key(hyphenchar);\
init_lua_key(id);\
init_lua_key(identity);\
init_lua_key(image);\
init_lua_key(imagetype);\
init_lua_key(immediate);\
init_lua_key(index);\
init_lua_key(info);\
init_lua_key(insert);\
init_lua_key(interlinepenalty);\
init_lua_key(italic);\
init_lua_key(keepopen);\
init_lua_key(kern);\
init_lua_key(kerns);\
init_lua_key(lang);\
init_lua_key(large_char);\
init_lua_key(large_fam);\
init_lua_key(last_ins_ptr);\
init_lua_key(lastlinefit);\
init_lua_key(leftskip);\
init_lua_key(leader);\
init_lua_key(least_page_cost);\
init_lua_key(left);\
init_lua_key(left_boundary);\
init_lua_key(left_protruding);\
init_lua_key(leftskip);\
init_lua_key(level);\
init_lua_key(ligatures);\
init_lua_key(leftskip);\
init_lua_key(linepenalty);\
init_lua_key(link_attr);\
init_lua_key(list);\
init_lua_key(local_box);\
init_lua_key(log);\
init_lua_key(looseness);\
init_lua_key(lua);\
init_lua_key(lua_bytecodes_indirect);\
init_lua_key(lua_functions);\
init_lua_key(luatex);\
init_lua_key(luatex_node);\
init_lua_key(luatex_token);\
init_lua_key(marginkern);\
init_lua_key(mark);\
init_lua_key(math);\
init_lua_key(math_choice);\
init_lua_key(math_left);\
init_lua_key(math_shift);\
init_lua_key(mathdir);\
init_lua_key(mathkern);\
init_lua_key(mathstyle);\
init_lua_key(media);\
init_lua_key(mid);\
init_lua_key(middle);\
init_lua_key(mode);\
init_lua_key(modeline);\
init_lua_key(name);\
init_lua_key(named_id);\
init_lua_key(names);\
init_lua_key(new_graf);\
init_lua_key(new_window);\
init_lua_key(next);\
init_lua_key(no);\
init_lua_key(no_align);\
init_lua_key(noad);\
init_lua_key(node);\
init_lua_key(nomath);\
init_lua_key(none);\
init_lua_key(nop);\
init_lua_key(nucleus);\
init_lua_key(num);\
init_lua_key(number);\
init_lua_key(objcompression);\
init_lua_key(objnum);\
init_lua_key(oldmath);\
init_lua_key(options);\
init_lua_key(orientation);\
init_lua_key(origin);\
init_lua_key(ordering);\
init_lua_key(output);\
init_lua_key(overlay_accent);\
init_lua_key(page);\
init_lua_key(pages);\
init_lua_key(page_discards_head);\
init_lua_key(page_head);\
init_lua_key(page_ins_head);\
init_lua_key(pageattributes);\
init_lua_key(pagebox);\
init_lua_key(pageresources);\
init_lua_key(pagesattributes);\
init_lua_key(parameters);\
init_lua_key(pardir);\
init_lua_key(parshape);\
init_lua_key(pdf_destination);\
init_lua_key(pdf_literal);\
init_lua_key(pen_broken);\
init_lua_key(pen_inter);\
init_lua_key(penalty);\
init_lua_key(pop);\
init_lua_key(post);\
init_lua_key(post_linebreak);\
init_lua_key(pre);\
init_lua_key(pre_adjust);\
init_lua_key(pre_adjust_head);\
init_lua_key(pre_align);\
init_lua_key(pre_box);\
init_lua_key(preamble);\
init_lua_key(pretolerance);\
init_lua_key(prev);\
init_lua_key(prevdepth);\
init_lua_key(prevgraf);\
init_lua_key(protrudechars);\
init_lua_key(psname);\
init_lua_key(ptr);\
init_lua_key(push);\
init_lua_key(quad);\
init_lua_key(radical);\
init_lua_key(raw);\
init_lua_key(ref_count);\
init_lua_key(reg);\
init_lua_key(registry);\
init_lua_key(renew);\
init_lua_key(rep);\
init_lua_key(replace);\
init_lua_key(resources);\
init_lua_key(right);\
init_lua_key(right_boundary);\
init_lua_key(right_protruding);\
init_lua_key(rightskip);\
init_lua_key(rotation);\
init_lua_key(rule);\
init_lua_key(scale);\
init_lua_key(script);\
init_lua_key(scriptscript);\
init_lua_key(semi_simple);\
init_lua_key(shift);\
init_lua_key(shrink);\
init_lua_key(shrink_order);\
init_lua_key(simple);\
init_lua_key(size);\
init_lua_key(skewchar);\
init_lua_key(slant);\
init_lua_key(slot);\
init_lua_key(small_char);\
init_lua_key(small_fam);\
init_lua_key(space);\
init_lua_key(space_shrink);\
init_lua_key(space_stretch);\
init_lua_key(spacefactor);\
init_lua_key(special);\
init_lua_key(split_discards_head);\
init_lua_key(split_keep);\
init_lua_key(split_off);\
init_lua_key(stack);\
init_lua_key(start);\
init_lua_key(step);\
init_lua_key(stream);\
init_lua_key(streamfile);\
init_lua_key(streamprovider);\
init_lua_key(stretch);\
init_lua_key(stretch_order);\
init_lua_key(string);\
init_lua_key(style);\
init_lua_key(sub);\
init_lua_key(subst_ex_font);\
init_lua_key(subtype);\
init_lua_key(sup);\
init_lua_key(supplement);\
init_lua_key(surround);\
init_lua_key(tail);\
init_lua_key(temp_head);\
init_lua_key(term);\
init_lua_key(tex);\
init_lua_key(text);\
init_lua_key(thread_attr);\
init_lua_key(thread_id);\
init_lua_key(tolerance);\
init_lua_key(tok);\
init_lua_key(token);\
init_lua_key(top);\
init_lua_key(top_accent);\
init_lua_key(top_left);\
init_lua_key(top_right);\
init_lua_key(tounicode);\
init_lua_key(tracingparagraphs);\
init_lua_key(trailer);\
init_lua_key(trailerid);\
init_lua_key(transform);\
init_lua_key(trim);\
init_lua_key(type);\
init_lua_key(uchyph);\
init_lua_key(units_per_em);\
init_lua_key(used);\
init_lua_key(user_id);\
init_lua_key(v);\
init_lua_key(value);\
init_lua_key(vbox);\
init_lua_key(vcenter);\
init_lua_key(version);\
init_lua_key(vert_italic);\
init_lua_key(vert_variants);\
init_lua_key(vmode_par);\
init_lua_key(visiblefilename);\
init_lua_key(vlist);\
init_lua_key(vtop);\
init_lua_key(widowpenalty);\
init_lua_key(width);\
init_lua_key(x_height);\
init_lua_key(xadvance);\
init_lua_key(xformresources);\
init_lua_key(xformattributes);\
init_lua_key(xoffset);\
init_lua_key(xres);\
init_lua_key(xsize);\
init_lua_key(xyz_zoom);\
init_lua_key(yoffset);\
init_lua_key(yres);\
init_lua_key(ysize);\
init_lua_key(writingmode);\
init_lua_key(__index);\
init_lua_key_alias(empty_string,"");\
init_lua_key_alias(lua_bytecodes_indirect,"lua.bytecodes.indirect");\
init_lua_key_alias(lua_functions,"lua.functions");\
init_lua_key_alias(luatex_node, "luatex.node");\
init_lua_key_alias(luatex_token, "luatex.token");\
init_lua_key_alias(mLTL,"-LTL");\
init_lua_key_alias(mRTT,"-RTT");\
init_lua_key_alias(mTLT,"-TLT");\
init_lua_key_alias(mTRT,"-TRT");\
init_lua_key_alias(node_properties,"node.properties");\
init_lua_key_alias(node_properties_indirect,"node.properties.indirect");\
init_lua_key_alias(pLTL,"+LTL");\
init_lua_key_alias(pRTT,"+RTT");\
init_lua_key_alias(pTLT,"+TLT");\
init_lua_key_alias(pTRT,"+TRT");\
init_lua_key_alias(pdf,"pdf");\
init_lua_key_alias(pdf_data,"pdf.data");\
init_lua_key_alias(term_and_log,"term and log")

#define assign_math_style(L,n,target) do { \
    if (lua_type(L,n) == LUA_TNUMBER) { \
        /* new, often same as subtype anyway  */ \
        target = lua_tointeger(L,n); \
    } else if (lua_type(L,n) == LUA_TSTRING) { \
        const char *s = lua_tostring(L, n); \
        if (lua_key_eq(s,display)) { \
            target = 0; \
        } else if (lua_key_eq(s,crampeddisplay)) { \
            target = 1; \
        } else if (lua_key_eq(s,text)) { \
            target = 2; \
        } else if (lua_key_eq(s,crampedtext)) { \
            target = 3; \
        } else if (lua_key_eq(s,script)) { \
            target = 4; \
        } else if (lua_key_eq(s,crampedscript)) { \
            target = 5; \
        } else if (lua_key_eq(s,scriptscript)) { \
            target = 6; \
        } else if (lua_key_eq(s,crampedscriptscript)) { \
            target = 7; \
        } else { \
            target = 2; \
        } \
    } else { \
        target = 2; /* text by default */ \
    } \
} while(0)

#ifdef _WIN32
extern FILE *_cairo_win_tmpfile( void );
#define tmpfile() _cairo_win_tmpfile()
#endif /* _WIN32 */

#endif                          /* LUATEX_API_H */

/*                                                 */
/* These keys have to available to different files */
/*                                                 */

use_lua_key(cmdname);use_lua_key(expandable);use_lua_key(protected);

use_lua_key(LTL);
use_lua_key(MathConstants);
use_lua_key(RTT);
use_lua_key(TLT);
use_lua_key(TRT);
use_lua_key(accent);
use_lua_key(action);
use_lua_key(action_id);
use_lua_key(action_type);
use_lua_key(active);
use_lua_key(additional);
use_lua_key(adjdemerits);
use_lua_key(adjust);
use_lua_key(adjust_head);
use_lua_key(adjusted_hbox);
use_lua_key(adjustspacing);
use_lua_key(advance);
use_lua_key(after_display);
use_lua_key(after_output);
use_lua_key(align);
use_lua_key(align_head);
use_lua_key(align_set);
use_lua_key(alignment);
use_lua_key(annot);
use_lua_key(area);
use_lua_key(art);
use_lua_key(attr);
use_lua_key(attributes);
use_lua_key(bbox);
use_lua_key(before_display);
use_lua_key(best_ins_ptr);
use_lua_key(best_page_break);
use_lua_key(best_size);
use_lua_key(bleed);
use_lua_key(bot);
use_lua_key(bot_accent);
use_lua_key(bottom_left);
use_lua_key(bottom_right);
use_lua_key(boundary);
use_lua_key(box);
use_lua_key(box_left);
use_lua_key(box_left_width);
use_lua_key(box_right);
use_lua_key(box_right_width);
use_lua_key(broken_ins);
use_lua_key(broken_ptr);
use_lua_key(brokenpenalty);
use_lua_key(cache);
use_lua_key(cal_expand_ratio);
use_lua_key(catalog);
use_lua_key(char);
use_lua_key(characters);
use_lua_key(checksum);
use_lua_key(cidinfo);
use_lua_key(class);
use_lua_key(clubpenalty);
use_lua_key(colordepth);
use_lua_key(colorspace);
use_lua_key(command);
use_lua_key(commands);
use_lua_key(comment);
use_lua_key(components);
use_lua_key(compresslevel);
use_lua_key(contrib_head);
use_lua_key(core);
use_lua_key(cost);
use_lua_key(count);
use_lua_key(crampeddisplay);
use_lua_key(crampedscript);
use_lua_key(crampedscriptscript);
use_lua_key(crampedtext);
use_lua_key(crop);
use_lua_key(csname);
use_lua_key(data);
use_lua_key(degree);
use_lua_key(delim);
use_lua_key(delimptr);
use_lua_key(demerits);
use_lua_key(denom);
use_lua_key(depth);
use_lua_key(designsize);
use_lua_key(dest_id);
use_lua_key(dest_type);
use_lua_key(dir);
use_lua_key(dir_h);
use_lua_key(direct);
use_lua_key(direction);
use_lua_key(dirs);
use_lua_key(disc);
use_lua_key(display);
use_lua_key(doublehyphendemerits);
use_lua_key(down);
use_lua_key(embedding);
use_lua_key(emergencystretch);
use_lua_key(empty_string);
use_lua_key(encodingbytes);
use_lua_key(encodingname);
use_lua_key(end);
use_lua_key(etex);
use_lua_key(equation);\
use_lua_key(equation_number);\
use_lua_key(exactly);
use_lua_key(expansion_factor);
use_lua_key(ext);
use_lua_key(extend);
use_lua_key(extender);
use_lua_key(extensible);
use_lua_key(extra_space);
use_lua_key(fam);
use_lua_key(fast);
use_lua_key(fence);
use_lua_key(file);
use_lua_key(filename);
use_lua_key(filepath);
use_lua_key(fill);
use_lua_key(fin_row);
use_lua_key(finalhyphendemerits);
use_lua_key(font);
use_lua_key(fonts);
use_lua_key(format);
use_lua_key(fraction);
use_lua_key(fullname);
use_lua_key(global);
use_lua_key(glue);
use_lua_key(glue_order);
use_lua_key(glue_set);
use_lua_key(glue_sign);
use_lua_key(glyph);
use_lua_key(h);
use_lua_key(hangafter);
use_lua_key(hangindent);
use_lua_key(hbox);
use_lua_key(head);
use_lua_key(height);
use_lua_key(hlist);
use_lua_key(hmode_par);
use_lua_key(hold_head);
use_lua_key(horiz_variants);
use_lua_key(hsize);
use_lua_key(hyphenchar);
use_lua_key(id);
use_lua_key(identity);
use_lua_key(image);
use_lua_key(imagetype);
use_lua_key(immediate);
use_lua_key(index);
use_lua_key(info);
use_lua_key(insert);
use_lua_key(interlinepenalty);
use_lua_key(italic);
use_lua_key(keepopen);
use_lua_key(kern);
use_lua_key(kerns);
use_lua_key(lang);
use_lua_key(large_char);
use_lua_key(large_fam);
use_lua_key(last_ins_ptr);
use_lua_key(lastlinefit);
use_lua_key(leader);
use_lua_key(least_page_cost);
use_lua_key(left);
use_lua_key(left_boundary);
use_lua_key(left_protruding);
use_lua_key(leftskip);
use_lua_key(level);
use_lua_key(ligatures);
use_lua_key(linepenalty);
use_lua_key(link_attr);
use_lua_key(list);
use_lua_key(local_box);
use_lua_key(log);
use_lua_key(looseness);
use_lua_key(lua);
use_lua_key(lua_bytecodes_indirect);
use_lua_key(lua_functions);
use_lua_key(luatex);
use_lua_key(luatex_node);
use_lua_key(luatex_token);
use_lua_key(mLTL);
use_lua_key(mRTT);
use_lua_key(mTLT);
use_lua_key(mTRT);
use_lua_key(marginkern);
use_lua_key(mark);
use_lua_key(math);
use_lua_key(math_choice);
use_lua_key(math_left);
use_lua_key(math_shift);
use_lua_key(mathdir);
use_lua_key(mathkern);
use_lua_key(mathstyle);
use_lua_key(media);
use_lua_key(mid);
use_lua_key(middle);
use_lua_key(mode);
use_lua_key(modeline);
use_lua_key(name);
use_lua_key(named_id);
use_lua_key(names);
use_lua_key(new_graf);
use_lua_key(new_window);
use_lua_key(next);
use_lua_key(no);
use_lua_key(no_align);
use_lua_key(noad);
use_lua_key(node);
use_lua_key(node_properties);
use_lua_key(node_properties_indirect);
use_lua_key(nomath);
use_lua_key(none);
use_lua_key(nop);
use_lua_key(nucleus);
use_lua_key(num);
use_lua_key(number);
use_lua_key(objcompression);
use_lua_key(objnum);
use_lua_key(oldmath);
use_lua_key(options);
use_lua_key(orientation);
use_lua_key(origin);
use_lua_key(ordering);
use_lua_key(output);
use_lua_key(overlay_accent);
use_lua_key(pLTL);
use_lua_key(pRTT);
use_lua_key(pTLT);
use_lua_key(pTRT);
use_lua_key(page);
use_lua_key(pages);
use_lua_key(page_discards_head);
use_lua_key(page_head);
use_lua_key(page_ins_head);
use_lua_key(pagebox);
use_lua_key(pageattributes);
use_lua_key(pageresources);
use_lua_key(pagesattributes);
use_lua_key(parameters);
use_lua_key(pardir);
use_lua_key(parshape);
use_lua_key(pdf);
use_lua_key(pdf_data);
use_lua_key(pdf_destination);
use_lua_key(pdf_literal);
use_lua_key(pen_broken);
use_lua_key(pen_inter);
use_lua_key(penalty);
use_lua_key(pop);
use_lua_key(post);
use_lua_key(post_linebreak);
use_lua_key(pre);
use_lua_key(pre_adjust);
use_lua_key(pre_adjust_head);
use_lua_key(pre_align);
use_lua_key(pre_box);
use_lua_key(preamble);
use_lua_key(pretolerance);
use_lua_key(prev);
use_lua_key(prevdepth);
use_lua_key(prevgraf);
use_lua_key(protrudechars);
use_lua_key(psname);
use_lua_key(ptr);
use_lua_key(push);
use_lua_key(quad);
use_lua_key(radical);
use_lua_key(raw);
use_lua_key(ref_count);
use_lua_key(reg);
use_lua_key(registry);
use_lua_key(renew);
use_lua_key(rep);
use_lua_key(replace);
use_lua_key(resources);
use_lua_key(right);
use_lua_key(right_boundary);
use_lua_key(right_protruding);
use_lua_key(rightskip);
use_lua_key(rotation);
use_lua_key(rule);
use_lua_key(scale);
use_lua_key(script);
use_lua_key(scriptscript);
use_lua_key(semi_simple);
use_lua_key(shift);
use_lua_key(shrink);
use_lua_key(shrink_order);
use_lua_key(simple);
use_lua_key(size);
use_lua_key(skewchar);
use_lua_key(slant);
use_lua_key(slot);
use_lua_key(small_char);
use_lua_key(small_fam);
use_lua_key(space);
use_lua_key(space_shrink);
use_lua_key(space_stretch);
use_lua_key(spacefactor);
use_lua_key(special);
use_lua_key(split_discards_head);
use_lua_key(split_keep);
use_lua_key(split_off);
use_lua_key(stack);
use_lua_key(start);
use_lua_key(step);
use_lua_key(stream);
use_lua_key(streamfile);
use_lua_key(streamprovider);
use_lua_key(stretch);
use_lua_key(stretch_order);
use_lua_key(string);
use_lua_key(style);
use_lua_key(sub);
use_lua_key(subst_ex_font);
use_lua_key(subtype);
use_lua_key(sup);
use_lua_key(supplement);
use_lua_key(surround);
use_lua_key(tail);
use_lua_key(temp_head);
use_lua_key(term);
use_lua_key(term_and_log);
use_lua_key(tex);
use_lua_key(text);
use_lua_key(thread_attr);
use_lua_key(thread_id);
use_lua_key(tolerance);
use_lua_key(tok);
use_lua_key(token);
use_lua_key(top);
use_lua_key(top_accent);
use_lua_key(top_left);
use_lua_key(top_right);
use_lua_key(tounicode);
use_lua_key(tracingparagraphs);
use_lua_key(trailer);
use_lua_key(trailerid);
use_lua_key(transform);
use_lua_key(trim);
use_lua_key(type);
use_lua_key(uchyph);
use_lua_key(units_per_em);
use_lua_key(used);
use_lua_key(user_id);
use_lua_key(v);
use_lua_key(value);
use_lua_key(vbox);
use_lua_key(vcenter);
use_lua_key(version);
use_lua_key(vert_italic);
use_lua_key(vert_variants);
use_lua_key(vmode_par);
use_lua_key(visiblefilename);
use_lua_key(vlist);
use_lua_key(vtop);
use_lua_key(widowpenalty);
use_lua_key(width);
use_lua_key(x_height);
use_lua_key(xadvance);
use_lua_key(xformresources);
use_lua_key(xformattributes);
use_lua_key(xoffset);
use_lua_key(xres);
use_lua_key(xsize);
use_lua_key(xyz_zoom);
use_lua_key(yoffset);
use_lua_key(yres);
use_lua_key(ysize);
use_lua_key(writingmode);
use_lua_key(__index);
