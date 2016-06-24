/* lstatslib.c

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

typedef struct statistic {
    const char *name;
    char type;
    void *value;
} statistic;

typedef const char *(*charfunc) (void);
typedef lua_Number(*numfunc) (void);
typedef int (*intfunc) (void);

static const char *getbanner(void)
{
    return (const char *) luatex_banner;
}

static const char *getlogname(void)
{
    return (const char *) texmf_log_name;
}

static const char *get_output_file_name(void)
{
    if (static_pdf != NULL)
        return (const char *) static_pdf->file_name;
    return NULL;
}

/*
static const char *getfilename(void)
{
    int t = 0;
    int level = in_open;
    while ((level > 0)) {
        t = input_stack[level--].name_field;
        if (t >= STRING_OFFSET) {
            return (const char *) str_string(t);
        }
    }
    return "";
}
*/

static const char *getfilename(void)
{
    const char * s;
    int level, t;
    level = in_open;
    while ((level > 0)) {
        s = full_source_filename_stack[level--];
        if (s != NULL) {
            return s;
        }
    }
    /* old method */
    level = in_open;
    while ((level > 0)) {
        t = input_stack[level--].name_field;
        if (t >= STRING_OFFSET) {
            return (const char *) str_string(t);
        }
    }
    return "";
}


static const char *getlasterror(void)
{
    return last_error;
}

static const char *getlastluaerror(void)
{
    return last_lua_error;
}

static const char *getlastwarningtag(void)
{
    return last_warning_tag;
}

static const char *getlastwarningstr(void)
{
    return last_warning_str;
}

static const char *getlasterrorcontext(void)
{
    return last_error_context;
}

static const char *luatexrevision(void)
{
    return (const char *) (strrchr(luatex_version_string, '.') + 1);
}

static lua_Number get_luatexhashchars(void)
{
  return (lua_Number) LUAI_HASHLIMIT;
}

static const char *get_luatexhashtype(void)
{
#ifdef LuajitTeX
     return (const char *)jithash_hashname;
#else
  return "lua";
#endif
}


static lua_Number get_pdf_gone(void)
{
    if (static_pdf != NULL)
        return (lua_Number) static_pdf->gone;
    return (lua_Number) 0;
}

static lua_Number get_pdf_ptr(void)
{
    if (static_pdf != NULL)
        return (lua_Number) (static_pdf->buf->p - static_pdf->buf->data);
    return (lua_Number) 0;
}

static lua_Number get_pdf_os_cntr(void)
{
    if (static_pdf != NULL)
        return (lua_Number) static_pdf->os->ostm_ctr;
    return (lua_Number) 0;
}

static lua_Number get_pdf_os_objidx(void)
{
    if (static_pdf != NULL)
        return (lua_Number) static_pdf->os->idx;
    return (lua_Number) 0;
}

static lua_Number get_pdf_mem_size(void)
{
    if (static_pdf != NULL)
        return (lua_Number) static_pdf->mem_size;
    return (lua_Number) 0;
}

static lua_Number get_pdf_mem_ptr(void)
{
    if (static_pdf != NULL)
        return (lua_Number) static_pdf->mem_ptr;
    return (lua_Number) 0;
}

static lua_Number get_obj_ptr(void)
{
    if (static_pdf != NULL)
        return (lua_Number) static_pdf->obj_ptr;
    return (lua_Number) 0;
}

static lua_Number get_obj_tab_size(void)
{
    if (static_pdf != NULL)
        return (lua_Number) static_pdf->obj_tab_size;
    return (lua_Number) 0;
}

static lua_Number get_dest_names_size(void)
{
    if (static_pdf != NULL)
        return (lua_Number) static_pdf->dest_names_size;
    return (lua_Number) 0;
}

static lua_Number get_dest_names_ptr(void)
{
    if (static_pdf != NULL)
        return (lua_Number) static_pdf->dest_names_ptr;
    return (lua_Number) 0;
}

static int get_hash_size(void)
{
    return hash_size;           /* is a #define */
}

static lua_Number shell_escape_state(void)
{
    if (shellenabledp <= 0) {
        return (lua_Number) 0;
    } else if (restrictedshell == 0) {
        return (lua_Number) 1;
    } else {
        return (lua_Number) 2;
    }
}

static int luastate_max = 1;    /* fixed value */

/* temp, for backward compat */
static int init_pool_ptr = 0;

static struct statistic stats[] = {

    /* most likely accessed */

    {"output_active", 'b', &output_active},
    {"best_page_break", 'n', &best_page_break},

    {"filename", 'S', (void *) &getfilename},
    {"inputid", 'g', &(iname)},
    {"linenumber", 'g', &line},

    {"lasterrorstring", 'S', (void *) &getlasterror},
    {"lastluaerrorstring", 'S', (void *) &getlastluaerror},
    {"lastwarningtag", 'S', (void *) &getlastwarningtag},
    {"lastwarningstring", 'S', (void *) &getlastwarningstr},
    {"lasterrorcontext", 'S', (void *) &getlasterrorcontext},

    /* seldom or never accessed */

    {"pdf_gone", 'N', &get_pdf_gone},
    {"pdf_ptr", 'N', &get_pdf_ptr},
    {"dvi_gone", 'g', &dvi_offset},
    {"dvi_ptr", 'g', &dvi_ptr},
    {"total_pages", 'g', &total_pages},
    {"output_file_name", 'S', (void *) &get_output_file_name},
    {"log_name", 'S', (void *) &getlogname},
    {"banner", 'S', (void *) &getbanner},
    {"luatex_version", 'G', &get_luatexversion},
    {"luatex_revision", 'S', (void *) &luatexrevision},
    {"luatex_hashtype", 'S', (void *) &get_luatexhashtype},
    {"luatex_hashchars", 'N',  &get_luatexhashchars},

    {"ini_version", 'b', &ini_version},

    {"shell_escape", 'N', &shell_escape_state}, /* be easy on old time usage */
    /*
     * mem stat
     */
    {"var_used", 'g', &var_used},
    {"dyn_used", 'g', &dyn_used},
    /*
     * traditional tex stats
     */
    {"str_ptr", 'g', &str_ptr},
    {"init_str_ptr", 'g', &init_str_ptr},
    {"max_strings", 'g', &max_strings},
    {"pool_ptr", 'g', &pool_size},
    {"init_pool_ptr", 'g', &init_pool_ptr},
    {"pool_size", 'g', &pool_size},
    {"var_mem_max", 'g', &var_mem_max},
    {"node_mem_usage", 'S', &sprint_node_mem_usage},
    {"fix_mem_max", 'g', &fix_mem_max},
    {"fix_mem_min", 'g', &fix_mem_min},
    {"fix_mem_end", 'g', &fix_mem_end},
    {"cs_count", 'g', &cs_count},
    {"hash_size", 'G', &get_hash_size},
    {"hash_extra", 'g', &hash_extra},
    {"font_ptr", 'G', &max_font_id},
    {"max_in_stack", 'g', &max_in_stack},
    {"max_nest_stack", 'g', &max_nest_stack},
    {"max_param_stack", 'g', &max_param_stack},
    {"max_buf_stack", 'g', &max_buf_stack},
    {"max_save_stack", 'g', &max_save_stack},
    {"stack_size", 'g', &stack_size},
    {"nest_size", 'g', &nest_size},
    {"param_size", 'g', &param_size},
    {"buf_size", 'g', &buf_size},
    {"save_size", 'g', &save_size},
    {"input_ptr", 'g', &input_ptr},
    {"obj_ptr", 'N', &get_obj_ptr},
    {"obj_tab_size", 'N', &get_obj_tab_size},
    {"pdf_os_cntr", 'N', &get_pdf_os_cntr},
    {"pdf_os_objidx", 'N', &get_pdf_os_objidx},
    {"pdf_dest_names_ptr", 'N', &get_dest_names_ptr},
    {"dest_names_size", 'N', &get_dest_names_size},
    {"pdf_mem_ptr", 'N', &get_pdf_mem_ptr},
    {"pdf_mem_size", 'N', &get_pdf_mem_size},
    {"largest_used_mark", 'g', &biggest_used_mark},
    {"luabytecodes", 'g', &luabytecode_max},
    {"luabytecode_bytes", 'g', &luabytecode_bytes},
    {"luastates", 'g', &luastate_max},
    {"luastate_bytes", 'g', &luastate_bytes},
    {"callbacks", 'g', &callback_count},
    {"indirect_callbacks", 'g', &saved_callback_count},

    {NULL, 0, 0}
};

static int stats_name_to_id(const char *name)
{
    int i;
    for (i = 0; stats[i].name != NULL; i++) {
        if (strcmp(stats[i].name, name) == 0)
            return i;
    }
    return -1;
}

static int do_getstat(lua_State * L, int i)
{
    int t;
    const char *st;
    charfunc f;
    intfunc g;
    numfunc n;
    int str;
    t = stats[i].type;
    switch (t) {
    case 'S':
        f = stats[i].value;
        st = f();
        lua_pushstring(L, st);
        break;
    case 's':
        str = *(int *) (stats[i].value);
        if (str) {
            char *ss = makecstring(str);
            lua_pushstring(L, ss);
            free(ss);
        } else {
            lua_pushnil(L);
        }
        break;
    case 'N':
        n = stats[i].value;
        lua_pushinteger(L, n());
        break;
    case 'G':
        g = stats[i].value;
        lua_pushinteger(L, g());
        break;
    case 'g':
        lua_pushinteger(L, *(int *) (stats[i].value));
        break;
    case 'B':
        g = stats[i].value;
        lua_pushboolean(L, g());
        break;
    case 'n':
        if (*(halfword *) (stats[i].value) != 0)
            lua_nodelib_push_fast(L, *(halfword *) (stats[i].value));
        else
            lua_pushnil(L);
        break;
    case 'b':
        lua_pushboolean(L, *(int *) (stats[i].value));
        break;
    default:
        lua_pushnil(L);
    }
    return 1;
}

static int getstats(lua_State * L)
{
    const char *st;
    int i;
    if (lua_type(L,-1) == LUA_TSTRING) {
        st = lua_tostring(L, -1);
        i = stats_name_to_id(st);
        if (i >= 0) {
            return do_getstat(L, i);
        }
    }
    return 0;
}

static int setstats(lua_State * L)
{
    (void) L;
    return 0;
}

static int statslist(lua_State * L)
{
    int i;
    luaL_checkstack(L, 1, "out of stack space");
    lua_newtable(L);
    for (i = 0; stats[i].name != NULL; i++) {
        luaL_checkstack(L, 2, "out of stack space");
        lua_pushstring(L, stats[i].name);
        do_getstat(L, i);
        lua_rawset(L, -3);
    }
    return 1;
}

static int resetmessages(lua_State * L)
{
    xfree(last_warning_str);
    xfree(last_warning_tag);
    xfree(last_error);
    xfree(last_lua_error);
    last_warning_str = NULL;
    last_warning_tag = NULL;
    last_error = NULL;
    last_lua_error = NULL;
    return 0;
}

static const struct luaL_Reg statslib[] = {
    {"list", statslist},
    {"resetmessages", resetmessages},
    {NULL, NULL}                /* sentinel */
};

int luaopen_stats(lua_State * L)
{
    luaL_register(L, "status", statslib);
    luaL_newmetatable(L, "tex.stats");
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, getstats);
    lua_settable(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, setstats);
    lua_settable(L, -3);
    lua_setmetatable(L, -2);    /* meta to itself */
    return 1;
}
