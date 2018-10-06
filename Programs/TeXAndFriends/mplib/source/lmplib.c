/* lmplib.c

   Copyright 2012 Taco Hoekwater <taco@luatex.org>

   This file is part of the MetaPost tarball, but belongs to LuaTeX.

   LuaTeX is free software; you can redistribute it and/or modify it under
   the terms of the GNU Lesser General Public License as published by the Free
   Software Foundation; either version 3 of the License, or (at your
   option) any later version.

   LuaTeX is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU Lesser General Public License along
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

#include <w2c/config.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <math.h>

#ifndef pdfTeX
#  include <lua.h>
#  include <lauxlib.h>
#  include <lualib.h>
#else
#  include "lua.h"
#  include "lauxlib.h"
#  include "lualib.h"
#define luaL_reg luaL_Reg
#define lua_objlen lua_rawlen
#endif

#ifndef luaL_reg
#define luaL_reg luaL_Reg
#endif

#ifndef lua_objlen
#define lua_objlen lua_rawlen
#endif

#include "mplib.h"
#include "mplibps.h"
#include "mplibsvg.h"
#include "mplibpng.h"

int luaopen_mplib(lua_State * L);

/*tex

    We need a few metatable identifiers in order to access the metatables for the
    main object and result userdata.

*/

#define MPLIB_METATABLE     "MPlib.meta"
#define MPLIB_FIG_METATABLE "MPlib.fig"
#define MPLIB_GR_METATABLE  "MPlib.gr"

#define is_mp(L,b) (MP *)luaL_checkudata(L,b,MPLIB_METATABLE)
#define is_fig(L,b) (struct mp_edge_object **)luaL_checkudata(L,b,MPLIB_FIG_METATABLE)
#define is_gr_object(L,b) (struct mp_graphic_object **)luaL_checkudata(L,b,MPLIB_GR_METATABLE)

/*tex

    We pre-hash the \LUA\ strings which is much faster. The approach is similar to the one
    used at the \TEX\ end.

*/

#define mplib_init_S(a) do {                            \
    lua_pushliteral(L,#a);                              \
    mplib_##a##_ptr = lua_tostring(L,-1);               \
    mplib_##a##_index = luaL_ref (L,LUA_REGISTRYINDEX); \
} while (0)

#define mplib_push_S(a) do {                            \
    lua_rawgeti(L,LUA_REGISTRYINDEX,mplib_##a##_index); \
} while (0)

#define mplib_is_S(a,i) \
    (mplib_##a##_ptr==lua_tostring(L,i))

#define mplib_make_S(a)                     \
    static int mplib_##a##_index = 0;         \
    static const char *mplib_##a##_ptr = NULL

/*tex In the next array entry 0 is not used */

static int mplib_type_Ses[mp_special_code + 1] = { 0 };

mplib_make_S(term);
mplib_make_S(error);
mplib_make_S(log);
mplib_make_S(fig);
mplib_make_S(status);

mplib_make_S(memory);
mplib_make_S(hash);
mplib_make_S(params);
mplib_make_S(open);
mplib_make_S(cycle);

mplib_make_S(offset);
mplib_make_S(dashes);

mplib_make_S(fill);
mplib_make_S(outline);
mplib_make_S(text);
mplib_make_S(special);
mplib_make_S(start_bounds);
mplib_make_S(stop_bounds);
mplib_make_S(start_clip);
mplib_make_S(stop_clip);

mplib_make_S(left_type);
mplib_make_S(right_type);
mplib_make_S(x_coord);
mplib_make_S(y_coord);
mplib_make_S(left_x);
mplib_make_S(left_y);
mplib_make_S(right_x);
mplib_make_S(right_y);
mplib_make_S(left_tension);
mplib_make_S(right_tension);
mplib_make_S(left_curl);
mplib_make_S(right_curl);
mplib_make_S(direction_x);
mplib_make_S(direction_y);

mplib_make_S(color);
mplib_make_S(dash);
mplib_make_S(depth);
mplib_make_S(dsize);
mplib_make_S(font);
mplib_make_S(height);
mplib_make_S(htap);
mplib_make_S(linecap);
mplib_make_S(linejoin);
mplib_make_S(miterlimit);
mplib_make_S(path);
mplib_make_S(pen);
mplib_make_S(postscript);
mplib_make_S(prescript);
mplib_make_S(transform);
mplib_make_S(type);
mplib_make_S(width);
mplib_make_S(elliptical);

static void mplib_init_Ses(lua_State * L)
{
    mplib_init_S(term);
    mplib_init_S(error);
    mplib_init_S(log);
    mplib_init_S(fig);
    mplib_init_S(status);

    mplib_init_S(memory);
    mplib_init_S(hash);
    mplib_init_S(params);
    mplib_init_S(open);
    mplib_init_S(cycle);

    mplib_init_S(offset);
    mplib_init_S(dashes);

    mplib_init_S(fill);
    mplib_init_S(outline);
    mplib_init_S(text);
    mplib_init_S(start_bounds);
    mplib_init_S(stop_bounds);
    mplib_init_S(start_clip);
    mplib_init_S(stop_clip);
    mplib_init_S(special);

    mplib_type_Ses[mp_fill_code] = mplib_fill_index;
    mplib_type_Ses[mp_stroked_code] = mplib_outline_index;
    mplib_type_Ses[mp_text_code] = mplib_text_index;
    mplib_type_Ses[mp_start_bounds_code] = mplib_start_bounds_index;
    mplib_type_Ses[mp_stop_bounds_code] = mplib_stop_bounds_index;
    mplib_type_Ses[mp_start_clip_code] = mplib_start_clip_index;
    mplib_type_Ses[mp_stop_clip_code] = mplib_stop_clip_index;
    mplib_type_Ses[mp_special_code] = mplib_special_index;

    mplib_init_S(left_type);
    mplib_init_S(right_type);
    mplib_init_S(x_coord);
    mplib_init_S(y_coord);
    mplib_init_S(left_x);
    mplib_init_S(left_y);
    mplib_init_S(right_x);
    mplib_init_S(right_y);
    mplib_init_S(left_tension);
    mplib_init_S(right_tension);
    mplib_init_S(left_curl);
    mplib_init_S(right_curl);
    mplib_init_S(direction_x);
    mplib_init_S(direction_y);

    mplib_init_S(color);
    mplib_init_S(dash);
    mplib_init_S(depth);
    mplib_init_S(dsize);
    mplib_init_S(font);
    mplib_init_S(height);
    mplib_init_S(htap);
    mplib_init_S(linecap);
    mplib_init_S(linejoin);
    mplib_init_S(miterlimit);
    mplib_init_S(path);
    mplib_init_S(pen);
    mplib_init_S(postscript);
    mplib_init_S(prescript);
    mplib_init_S(transform);
    mplib_init_S(type);
    mplib_init_S(width);
    mplib_init_S(elliptical);
}

/*tex

    Here are some enumeration arrays to map MPlib enums to \LUA\ strings. If needed
    we can also predefine keys here, as we do with nodes.

*/

static const char *math_options[] =
    { "scaled", "double", "binary", "decimal", NULL };

static const char *interaction_options[] =
    { "unknown", "batch", "nonstop", "scroll", "errorstop", NULL };

static const char *mplib_filetype_names[] =
    { "term", "error", "mp", "log", "ps", "png", "mem", "tfm", "map", "pfb", "enc", NULL };

static const char *knot_type_enum[] =
    { "endpoint", "explicit", "given", "curl", "open", "end_cycle" };

static const char *fill_fields[] =
    { "type", "path", "htap", "pen", "color", "linejoin", "miterlimit",
      "prescript", "postscript", NULL };

static const char *stroked_fields[] =
    { "type", "path", "pen", "color", "linejoin", "miterlimit", "linecap",
      "dash", "prescript", "postscript", NULL };

static const char *text_fields[] =
    { "type", "text", "dsize", "font", "color", "width", "height", "depth",
      "transform", "prescript", "postscript", NULL };

static const char *special_fields[] =
    { "type", "prescript", NULL };

static const char *start_bounds_fields[] =
    { "type", "path", NULL };

static const char *start_clip_fields[] =
    { "type", "path", NULL };

static const char *stop_bounds_fields[] =
    { "type", NULL };

static const char *stop_clip_fields[] =
    { "type", NULL };

static const char *no_fields[] =
    { NULL };

/*tex

    The list of supported MPlib options (not all make sense).

*/

typedef enum {
    P_ERROR_LINE,
    P_MAX_LINE,
    P_RANDOM_SEED,
    P_MATH_MODE,
    P_INTERACTION,
    P_INI_VERSION,
    P_MEM_NAME,
    P_JOB_NAME,
    P_FIND_FILE,
    P_RUN_SCRIPT,
    P_MAKE_TEXT,
    P_SCRIPT_ERROR,
    P_EXTENSIONS,
    P__SENTINEL
} mplib_parm_idx;

typedef struct {
    /*tex parameter name */
    const char *name;
    /*tex parameter index */
    mplib_parm_idx idx;
} mplib_parm_struct;

static mplib_parm_struct mplib_parms[] = {
    {"error_line",   P_ERROR_LINE   },
    {"print_line",   P_MAX_LINE     },
    {"random_seed",  P_RANDOM_SEED  },
    {"interaction",  P_INTERACTION  },
    {"job_name",     P_JOB_NAME     },
    {"find_file",    P_FIND_FILE    },
    {"run_script",   P_RUN_SCRIPT   },
    {"make_text",    P_MAKE_TEXT    },
    {"script_error", P_SCRIPT_ERROR },
    {"extensions",   P_EXTENSIONS   },
    {"math_mode",    P_MATH_MODE    },
    {NULL,           P__SENTINEL    }
};

/*tex

    We start by defining the needed callback routines for the library.

*/

static char *mplib_find_file(MP mp, const char *fname, const char *fmode, int ftype)
{
    lua_State *L = (lua_State *)mp_userdata(mp);
    lua_checkstack(L, 4);
    lua_getfield(L, LUA_REGISTRYINDEX, "mplib.file_finder");
    if (lua_isfunction(L, -1)) {
        char *s = NULL;
        const char *x = NULL;
        lua_pushstring(L, fname);
        lua_pushstring(L, fmode);
        if (ftype >= mp_filetype_text) {
            lua_pushinteger(L, (ftype - mp_filetype_text));
        } else {
            lua_pushstring(L, mplib_filetype_names[ftype]);
        }
        if (lua_pcall(L, 3, 1, 0) != 0) {
            fprintf(stdout, "Error in mp.find_file: %s\n", lua_tostring(L, -1));
            return NULL;
        }
        x = lua_tostring(L, -1);
        if (x != NULL)
            s = strdup(x);
        /*tex pop the string */
        lua_pop(L, 1);
        return s;
    } else {
        lua_pop(L, 1);
    }
    if (fmode[0] != 'r' || (!access(fname, R_OK)) || ftype) {
        return strdup(fname);
    }
    return NULL;
}

static int mplib_find_file_function(lua_State * L)
{
    if (!(lua_isfunction(L, -1) || lua_isnil(L, -1))) {
        /*tex An error. */
        return 1;
    }
    lua_pushstring(L, "mplib.file_finder");
    lua_pushvalue(L, -2);
    lua_rawset(L, LUA_REGISTRYINDEX);
    return 0;
}

static void mplib_warning(const char *str)
{
    fprintf(stdout,"mplib warning: %s\n",str);
}

static void mplib_script_error(MP mp, const char *str)
{
    lua_State *L = (lua_State *)mp_userdata(mp);
    lua_checkstack(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, "mplib.script_error");
    if (lua_isfunction(L, -1)) {
        lua_pushstring(L, str);
        /*tex We assume that the function is okay. */
        lua_pcall(L, 1, 0, 0);
    } else {
        mplib_warning(str);
        lua_pop(L, 1);
    }
}

static int mplib_script_error_function(lua_State * L)
{
    if (!(lua_isfunction(L, -1) || lua_isnil(L, -1))) {
        /*tex An error. */
        return 1;
    }
    lua_pushstring(L, "mplib.script_error");
    lua_pushvalue(L, -2);
    lua_rawset(L, LUA_REGISTRYINDEX);
    return 0;
}

static char *mplib_run_script(MP mp, const char *str)
{
    lua_State *L = (lua_State *)mp_userdata(mp);
    lua_checkstack(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, "mplib.run_script");
    if (lua_isfunction(L, -1)) {
        char *s = NULL;
        const char *x = NULL;
        lua_pushstring(L, str);
        if (lua_pcall(L, 1, 1, 0) != 0) {
            fprintf(stdout,"mplib warning: error in script: %s\n",lua_tostring(L, -1));
            return NULL;
        }
        x = lua_tostring(L, -1);
        if (x != NULL)
            s = strdup(x);
        /*tex Pop the string. */
        lua_pop(L, 1);
        return s;
    } else {
        lua_pop(L, 1);
    }
    return NULL;
}

static int mplib_run_script_function(lua_State * L)
{
    if (!(lua_isfunction(L, -1) || lua_isnil(L, -1))) {
        return 1; /* error */
    }
    lua_pushstring(L, "mplib.run_script");
    lua_pushvalue(L, -2);
    lua_rawset(L, LUA_REGISTRYINDEX);
    return 0;
}

static char *mplib_make_text(MP mp, const char *str, int mode)
{
    lua_State *L = (lua_State *)mp_userdata(mp);
    lua_checkstack(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, "mplib.make_text");
    if (lua_isfunction(L, -1)) {
        char *s = NULL;
        const char *x = NULL;
        lua_pushstring(L, str);
        lua_pushinteger(L, mode);
        if (lua_pcall(L, 2, 1, 0) != 0) {
            mplib_script_error(mp, lua_tostring(L, -1));
            return NULL;
        }
        x = lua_tostring(L, -1);
        if (x != NULL)
            s = strdup(x);
        /*tex Pop the string. */
        lua_pop(L, 1);
        return s;
    } else {
        lua_pop(L, 1);
    }
    return NULL;
}

static int mplib_make_text_function(lua_State * L)
{
    if (!(lua_isfunction(L, -1) || lua_isnil(L, -1))) {
        /*tex An error. */
        return 1;
    }
    lua_pushstring(L, "mplib.make_text");
    lua_pushvalue(L, -2);
    lua_rawset(L, LUA_REGISTRYINDEX);
    return 0;
}

static int mplib_get_numeric(lua_State * L)
{
    MP *mp = is_mp(L, 1);
    if (*mp != NULL) {
        size_t l;
        const char *s = lua_tolstring(L, 2, &l);
        if (s != NULL) {
            lua_pushnumber(L, mp_get_numeric_value(*mp,s,l));
            return 1;
        }
    }
    lua_pushnumber(L,0);
    return 1;
}

static int mplib_get_boolean(lua_State * L)
{
    MP *mp = is_mp(L, 1);
    if (*mp != NULL) {
        size_t l;
        const char *s = lua_tolstring(L, 2, &l);
        if (s != NULL) {
            lua_pushboolean(L, mp_get_boolean_value(*mp,s,l));
            return 1;
        }
    }
    lua_pushboolean(L,0);
    return 1;
}

static int mplib_get_string(lua_State * L)
{
    MP *mp = is_mp(L, 1);
    if (*mp != NULL) {
        size_t l;
        const char *s = lua_tolstring(L, 2, &l);
        if (s != NULL) {
            char *r = mp_get_string_value(*mp,s,l) ;
            if (r != NULL) {
                lua_pushstring(L, r);
                return 1;
            }
        }
    }
    lua_pushstring(L,"");
    return 1;
}

#define xfree(A) if ((A)!=NULL) { free((A)); A = NULL; }

static int mplib_new(lua_State * L)
{
    MP *mp_ptr;
    mp_ptr = lua_newuserdata(L, sizeof(MP *));
    if (mp_ptr) {
        int i;
        struct MP_options *options = mp_options();
        options->userdata = (void *) L;
        /*tex Required: */
        options->noninteractive = 1;
        options->extensions = 0 ;
        options->find_file = mplib_find_file;
        options->run_script = mplib_run_script;
        options->make_text = mplib_make_text;
    /*  options->script_error = mplib_script_error; */
        options->print_found_names = 1;
        options->ini_version = 1;
        if (lua_type(L, 1) == LUA_TTABLE) {
            for (i = 0; mplib_parms[i].name != NULL; i++) {
                lua_getfield(L, 1, mplib_parms[i].name);
                if (lua_isnil(L, -1)) {
                    lua_pop(L, 1);
                    continue;
                }
                switch (mplib_parms[i].idx) {
                    case P_ERROR_LINE:
                        options->error_line = (int)lua_tointeger(L, -1);
                        if (options->error_line < 60)
                            options->error_line = 60;
                        if (options->error_line > 250)
                            options->error_line = 250;
                        options->half_error_line = (options->error_line/2)+10;
                        break;
                    case P_MAX_LINE:
                        options->max_print_line = (int)lua_tointeger(L, -1);
                        if (options->max_print_line < 60)
                            options->max_print_line = 60;
                        break;
                    case P_RANDOM_SEED:
                        options->random_seed = (int)lua_tointeger(L, -1);
                        break;
                    case P_INTERACTION:
                        options->interaction = luaL_checkoption(L, -1, "errorstopmode", interaction_options);
                        break;
                    case P_MATH_MODE:
                        options->math_mode = luaL_checkoption(L, -1, "scaled", math_options);
                        break;
                    case P_JOB_NAME:
                        options->job_name = strdup(lua_tostring(L, -1));
                        break;
                    case P_FIND_FILE:
                        if (mplib_find_file_function(L)) {
                            mplib_warning("function expected for 'find_file'");
                        }
                        break;
                    case P_RUN_SCRIPT:
                        if (mplib_run_script_function(L)) {
                            mplib_warning("function expected for 'run_script'");
                        }
                        break;
                    case P_MAKE_TEXT:
                        if (mplib_make_text_function(L)) {
                            mplib_warning("function expected for 'make_text'");
                        }
                        break;
                    case P_SCRIPT_ERROR:
                        if (mplib_script_error_function(L)) {
                            mplib_warning("function expected for 'script_error'");
                        }
                        break;
                    case P_EXTENSIONS:
                        options->extensions = (int)lua_tointeger(L, -1);
                        break;
                    default:
                        break;
                }
                lua_pop(L, 1);
            }
        }
        *mp_ptr = mp_initialize(options);
        xfree(options->command_line);
        xfree(options->mem_name);
        free(options);
        if (*mp_ptr) {
            luaL_getmetatable(L, MPLIB_METATABLE);
            lua_setmetatable(L, -2);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

static int mplib_collect(lua_State * L)
{
    MP *mp_ptr = is_mp(L, 1);
    if (*mp_ptr != NULL) {
      (void)mp_finish(*mp_ptr);
      *mp_ptr = NULL;
    }
    return 0;
}

static int mplib_tostring(lua_State * L)
{
    MP *mp_ptr = is_mp(L, 1);
    if (*mp_ptr != NULL) {
        (void) lua_pushfstring(L, "<MP %p>", *mp_ptr);
        return 1;
    }
    return 0;
}

static int mplib_wrapresults(lua_State * L, mp_run_data *res, int status)
{
    lua_checkstack(L, 5);
    lua_newtable(L);
    if (res->term_out.used != 0) {
        mplib_push_S(term);
        lua_pushlstring(L, res->term_out.data, res->term_out.used);
        lua_rawset(L,-3);
    }
    if (res->error_out.used != 0) {
        mplib_push_S(error);
        lua_pushlstring(L, res->error_out.data, res->error_out.used);
        lua_rawset(L,-3);
    }
    if (res->log_out.used != 0) {
        mplib_push_S(log);
        lua_pushlstring(L, res->log_out.data, res->log_out.used);
        lua_rawset(L,-3);
    }
    if (res->edges != NULL) {
        struct mp_edge_object **v;
        struct mp_edge_object *p = res->edges;
        int i = 1;
        mplib_push_S(fig);
        lua_newtable(L);
        while (p != NULL) {
            v = lua_newuserdata(L, sizeof(struct mp_edge_object *));
            *v = p;
            luaL_getmetatable(L, MPLIB_FIG_METATABLE);
            lua_setmetatable(L, -2);
            lua_rawseti(L, -2, i);
            i++;
            p = p->next;
        }
        lua_rawset(L,-3);
        res->edges = NULL;
    }
    mplib_push_S(status);
    lua_pushinteger(L, status);
    lua_rawset(L,-3);
    return 1;
}

static int mplib_execute(lua_State * L)
{
    MP *mp_ptr;
    if (lua_gettop(L)!=2) {
        lua_pushnil(L);
        return 1;
    }
    mp_ptr = is_mp(L, 1);
    if (*mp_ptr != NULL && lua_isstring(L, 2)) {
        size_t l;
        char *s = xstrdup(lua_tolstring(L, 2, &l));
        int h = mp_execute(*mp_ptr, s, l);
        mp_run_data *res = mp_rundata(*mp_ptr);
        free(s);
        return mplib_wrapresults(L, res, h);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int mplib_finish(lua_State * L)
{
    MP *mp_ptr = is_mp(L, 1);
    if (*mp_ptr != NULL) {
        int i;
        int h = mp_execute(*mp_ptr,NULL,0);
        mp_run_data *res = mp_rundata(*mp_ptr);
        i = mplib_wrapresults(L, res, h);
        (void)mp_finish(*mp_ptr);
        *mp_ptr = NULL;
        return i;
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int mplib_char_dimension(lua_State * L, int t)
{
    MP *mp_ptr = is_mp(L, 1);
    if (*mp_ptr != NULL) {
        char *fname = xstrdup(luaL_checkstring(L,2));
        int charnum = (int)luaL_checkinteger(L,3);
        if (charnum<0 || charnum>255) {
            lua_pushnumber(L, (lua_Number)0);
        } else {
            lua_pushnumber(L,(lua_Number)mp_get_char_dimension(*mp_ptr,fname,charnum,t));
        }
        free(fname);
    } else {
        lua_pushnumber(L, (lua_Number)0);
    }
    return 1;
}

static int mplib_charwidth(lua_State * L)
{
    return mplib_char_dimension(L, 'w');
}

static int mplib_chardepth(lua_State * L)
{
    return mplib_char_dimension(L, 'd');
}

static int mplib_charheight(lua_State * L)
{
    return mplib_char_dimension(L, 'h');
}

static int mplib_version(lua_State * L)
{
    char *s = mp_metapost_version();
    lua_pushstring(L, s);
    free(s);
    return 1;
}

static int mplib_statistics(lua_State * L)
{
    MP *mp_ptr = is_mp(L, 1);
    if (*mp_ptr != NULL) {
        lua_newtable(L);
        mplib_push_S(memory);
        lua_pushinteger(L, mp_memory_usage(*mp_ptr));
        lua_rawset(L,-3);
        mplib_push_S(hash);
        lua_pushinteger(L, mp_hash_usage(*mp_ptr));
        lua_rawset(L,-3);
        mplib_push_S(params);
        lua_pushinteger(L, mp_param_usage(*mp_ptr));
        lua_rawset(L,-3);
        mplib_push_S(open);
        lua_pushinteger(L, mp_open_usage(*mp_ptr));
        lua_rawset(L,-3);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int set_direction (lua_State * L, MP mp, mp_knot p) {
    double direction_x = 0, direction_y = 0;
    direction_x = (double)lua_tonumber(L,-1);
    lua_pop(L,1);
    mplib_push_S(direction_y);
    lua_rawget(L,-2);
    if (!lua_isnumber(L,-1)) {
        return 0;
    }
    direction_y = (double)lua_tonumber(L,-1);
    lua_pop(L,1);
    if (!mp_set_knot_direction(mp, p, direction_x, direction_y)) {
        return 0;
    }
    return 1;
}

static int set_left_curl (lua_State * L, MP mp, mp_knot p) {
    double curl = (double)lua_tonumber(L,-1);
    lua_pop(L,1);
    if (!mp_set_knot_left_curl(mp, p, curl)) {
        return 0;
    }
    return 1;
}

static int set_left_tension (lua_State * L, MP mp, mp_knot p) {
    double tension = (double)lua_tonumber(L,-1);
    lua_pop(L,1);
    if (!mp_set_knot_left_tension(mp, p, tension)) {
        return 0;
    }
    return 1;
}

static int set_left_control (lua_State * L, MP mp, mp_knot p) {
    double x, y;
    x = (double)lua_tonumber(L,-1);
    lua_pop(L,1);
    mplib_push_S(left_y);
    lua_rawget(L,-2);
    if (!lua_isnumber(L,-1)) {
        return 0;
    }
    y = (double)lua_tonumber(L,-1);
    lua_pop(L,1);
    if (!mp_set_knot_left_control(mp, p, x, y)) {
        return 0;
    }
    return 1;
}

static int set_right_curl (lua_State * L, MP mp, mp_knot p) {
    double curl = (double)lua_tonumber(L,-1);
    lua_pop(L,1);
    if (!mp_set_knot_right_curl(mp, p, curl)) {
        return 0;
    }
    return 1;
}

static int set_right_tension (lua_State * L, MP mp, mp_knot p) {
    double tension = (double)lua_tonumber(L,-1);
    lua_pop(L,1);
    if (!mp_set_knot_right_tension(mp, p, tension)) {
        return 0;
    }
    return 1;
}

static int set_right_control (lua_State * L, MP mp, mp_knot p) {
    double x, y;
    x = (double)lua_tonumber(L,-1);
    lua_pop(L,1);
    mplib_push_S(right_y);
    lua_rawget(L,-2);
    if (!lua_isnumber(L,-1)) {
        return 0;
    }
    y = (double)lua_tonumber(L,-1);
    lua_pop(L,1);
    if (!mp_set_knot_right_control(mp, p, x, y)) {
        return 0;
    }
    return 1;
}

#if 0

#define ROUNDED_ZERO(v) (fabs((v))<0.00001 ? 0 : (v))
#define PI 3.1415926535897932384626433832795028841971
#define RADIANS(a) (mp_number_as_double(mp,(a)) / 16.0) * PI/180.0

void mp_dump_path (MP mp, mp_knot h) {
    mp_knot p, q;
    if (h == NULL)
        return;
    p = h;
    do {
        q=mp_knot_next(mp,p);
        if ( (p==NULL)||(q==NULL) ) {
            printf("\n???");
            return; /* this won't happen */
        }
        printf ("(%g,%g)", mp_number_as_double(mp,mp_knot_x_coord(mp,p)),
                           mp_number_as_double(mp,mp_knot_y_coord(mp,p)));
        switch (mp_knot_right_type(mp,p)) {
            case mp_endpoint:
                if ( mp_knot_left_type(mp,p)==mp_open )
                    printf("{open?}");
                if ( (mp_knot_left_type(mp,q)!=mp_endpoint)||(q!=h) )
                    q=NULL; /* force an error */
                goto DONE;
            break;
        case mp_explicit:
            printf ("..controls (%g,%g)",
                mp_number_as_double(mp,mp_knot_right_x(mp,p)),
                mp_number_as_double(mp,mp_knot_right_y(mp,p)));
            printf(" and ");
            if ( mp_knot_left_type(mp,q)!=mp_explicit ) {
                printf("??");
            } else {
                printf ("(%g,%g)",mp_number_as_double(mp,mp_knot_left_x(mp,q)),
                                  mp_number_as_double(mp,mp_knot_left_y(mp,q)));
            }
            goto DONE;
            break;
        case mp_open:
            if ( (mp_knot_left_type(mp,p)!=mp_explicit) &&
                 (mp_knot_left_type(mp,p)!=mp_open) ) {
                printf("{open?}");
            }
            break;
        case mp_curl:
        case mp_given:
            if ( mp_knot_left_type(mp,p)==mp_open )
                printf("??");
            if ( mp_knot_right_type(mp,p)==mp_curl ) {
                printf("{curl %g}", mp_number_as_double(mp,mp_knot_right_curl(mp,p)));
            } else {
                double rad = RADIANS(mp_knot_right_curl(mp,p));
                double n_cos = ROUNDED_ZERO(cos(rad)*4096);
                double n_sin = ROUNDED_ZERO(sin(rad)*4096);
                printf("{%g,%g}", n_cos, n_sin);
            }
            break;
        }
        if ( mp_knot_left_type(mp,q)<=mp_explicit ) {
            printf("..control?"); /* can't happen */
        } else if ((mp_number_as_double(mp,mp_knot_right_tension(mp,p))!=(1.0))||
                   (mp_number_as_double(mp,mp_knot_left_tension(mp,q)) !=(1.0))) {
            printf("..tension ");
            if ( mp_number_as_double(mp,mp_knot_right_tension(mp,p))<0.0 )
                printf("atleast ");
            printf("%g", fabs(mp_number_as_double(mp,mp_knot_right_tension(mp,p))));
            if (mp_number_as_double(mp,mp_knot_right_tension(mp,p)) !=
                mp_number_as_double(mp,mp_knot_left_tension(mp,q))) {
                printf(" and ");
                if (mp_number_as_double(mp,mp_knot_left_tension(mp,q))< 0.0)
                    printf("atleast ");
                printf("%g", fabs(mp_number_as_double(mp,mp_knot_left_tension(mp,q))));
            }
        }
        DONE:
        p=q;
        if ( p!=h || mp_knot_left_type(mp,h)!=mp_endpoint) {
            printf ("\n ..");
            if ( mp_knot_left_type(mp,p) == mp_given ) {
                double rad = RADIANS(mp_knot_left_curl(mp,p));
                double n_cos = ROUNDED_ZERO(cos(rad)*4096);
                double n_sin = ROUNDED_ZERO(sin(rad)*4096);
                printf("{%g,%g}", n_cos, n_sin);
            } else if ( mp_knot_left_type(mp,p) ==mp_curl ){
                printf("{curl %g}", mp_number_as_double(mp,mp_knot_left_curl(mp,p)));
            }
        }
    } while (p!=h);
    if ( mp_knot_left_type(mp,h)!=mp_endpoint )
        printf("cycle");
    printf (";\n");
}

#endif

static int mplib_solve_path(lua_State * L)
{
    MP *mp_ptr;
    MP mp = NULL;
    int cyclic;
    const char *errormsg = NULL;
    mp_knot p, q, first;
    int numpoints, i;
    p = q = first = NULL;
    if (lua_gettop(L) != 3) {
        errormsg = "Wrong number of arguments";
        goto BAD;
    }
    mp_ptr = is_mp(L, 1);
    if (*mp_ptr == NULL || (!lua_istable(L,2)) || lua_objlen(L,2)<=0 || (!lua_isboolean(L,3))) {
        errormsg = "Wrong argument types";
        goto BAD;
    }
    mp = *mp_ptr;
    cyclic = lua_toboolean(L,3);
    lua_pop(L,1);
    /*tex We build up the path. */
    numpoints = lua_objlen(L,2);
    first = p = NULL;
    for (i=1;i<=numpoints;i++) {
        int left_set = 0, right_set = 0;
        double x_coord, y_coord;
        lua_rawgeti(L,-1,i);
        if (!lua_istable(L,-1)) {
            errormsg = "Wrong argument types";
            goto BAD;
        }
        mplib_push_S(x_coord);
        lua_rawget(L,-2);
        if (!lua_isnumber(L,-1)) {
            errormsg = "Missing X coordinate";
            goto BAD;
        }
        x_coord = (double)lua_tonumber(L,-1);
        lua_pop(L,1);
        mplib_push_S(y_coord);
        lua_rawget(L,-2);
        if (!lua_isnumber(L,-1)) {
            errormsg = "Missing y coordinate";
            goto BAD;
        }
        y_coord = (double)lua_tonumber(L,-1);
        lua_pop(L,1);
        q = p;
        if (q!=NULL) {
            /*tex

                We have to save the right_tension because |mp_append_knot|
                trashes it, believing that it is as yet uninitialized.

            */
            double saved_tension = mp_number_as_double(mp, mp_knot_right_tension(mp,p));
            p = mp_append_knot(mp, p, x_coord, y_coord);
            if ( ! p ) {
                errormsg = "knot creation failure";
                goto BAD;
            }
            (void)mp_set_knot_right_tension(mp, q, saved_tension);
        } else {
            p = mp_append_knot(mp, p, x_coord, y_coord);
            if ( ! p ) {
                errormsg = "knot creation failure";
                goto BAD;
            }
        }
        if (first == NULL)
            first = p;
        mplib_push_S(left_curl);
        lua_rawget(L,-2);
        if (lua_isnumber(L,-1)) {
            if (!set_left_curl (L, mp, p)) {
                errormsg = "failed to set left curl";
                goto BAD;
            }
            left_set  = 1;
        } else {
            /*tex A |nil| value. */
            lua_pop(L,1);
        }
        mplib_push_S(left_tension);
        lua_rawget(L,-2);
        if (lua_isnumber(L,-1)) {
            if (left_set) {
                errormsg = "Left side already set";
                goto BAD;
            } else {
                if (!set_left_tension(L, mp, p)) {
                    errormsg = "Failed to set left tension";
                    goto BAD;
                }
                left_set  = 1;
            }
        } else {
            /*tex A |nil| value. */
            lua_pop(L,1);
        }
        mplib_push_S(left_x);
        lua_rawget(L,-2);
        if (lua_isnumber(L,-1)) {
            if (left_set) {
                errormsg = "Left side already set";
                goto BAD;
            } else {
                if (!set_left_control(L, mp, p)) {
                    errormsg = "Failed to set left control";
                    goto BAD;
                }
            }
        } else {
            lua_pop(L,1);
        }
        mplib_push_S(right_curl);
        lua_rawget(L,-2);
        if (lua_isnumber(L,-1)) {
            if (!set_right_curl (L, mp, p)) {
                errormsg = "failed to set right curl";
                goto BAD;
            }
            right_set  = 1;
        } else {
            /*tex A |nil| value. */
            lua_pop(L,1);
        }
        mplib_push_S(right_tension);
        lua_rawget(L,-2);
        if (lua_isnumber(L,-1)) {
            if (right_set) {
                errormsg = "Right side already set";
                goto BAD;
            } else {
            if (!set_right_tension(L, mp, p)) {
                errormsg = "Failed to set right tension";
                goto BAD;
            }
            right_set = 1;
            }
        } else {
            lua_pop(L,1);
        }
        mplib_push_S(right_x);
        lua_rawget(L,-2);
        if (lua_isnumber(L,-1)) {
            if (right_set) {
                errormsg = "Right side already set";
                goto BAD;
            } else {
                if (!set_right_control(L, mp, p)) {
                    errormsg = "Failed to set right control";
                    goto BAD;
                }
            }
        } else {
            lua_pop(L,1);
        }
        mplib_push_S(direction_x);
        lua_rawget(L,-2);
        if (lua_isnumber(L,-1)) {
            if (!set_direction (L, mp, p)) {
                errormsg = "failed to set direction";
                goto BAD;
            }
        } else {
            /*tex A |nil| value. */
            lua_pop(L,1);
        }
        /*tex Up the next item */
        lua_pop(L,1);
    }
    if (cyclic) {
        mp_close_path_cycle (mp, p, first);
    } else {
        mp_close_path (mp, p, first);
    }
#if 0
    mp_dump_path(mp,first);
#endif
    /*tex We're finished reading arguments. */
    if (!mp_solve_path(mp,first)) {
        errormsg = "Failed to solve the path";
        goto BAD;
    }
    /*tex Squeeze the new values back into the table. */
    p = first;
    for (i=1;i<=numpoints;i++) {
        lua_rawgeti(L,-1, i);
        mplib_push_S(left_x);  lua_pushnumber(L, mp_number_as_double(mp, mp_knot_left_x(mp, p)));  lua_rawset(L,-3);
        mplib_push_S(left_y);  lua_pushnumber(L, mp_number_as_double(mp, mp_knot_left_y(mp, p)));  lua_rawset(L,-3);
        mplib_push_S(right_x); lua_pushnumber(L, mp_number_as_double(mp, mp_knot_right_x(mp, p))); lua_rawset(L,-3);
        mplib_push_S(right_y); lua_pushnumber(L, mp_number_as_double(mp, mp_knot_right_y(mp, p))); lua_rawset(L,-3);
        /*tex This is a bit overkill \unknown */
        mplib_push_S(left_tension);  lua_pushnil(L); lua_rawset(L,-3);
        mplib_push_S(right_tension); lua_pushnil(L); lua_rawset(L,-3);
        mplib_push_S(left_curl);     lua_pushnil(L); lua_rawset(L,-3);
        mplib_push_S(right_curl);    lua_pushnil(L); lua_rawset(L,-3);
        mplib_push_S(direction_x);   lua_pushnil(L); lua_rawset(L,-3);
        mplib_push_S(direction_y);   lua_pushnil(L); lua_rawset(L,-3);
        /*tex \unknown\ till here. */
        mplib_push_S(left_type);  lua_pushstring(L, knot_type_enum[mp_knot_left_type(mp, p)]);  lua_rawset(L, -3);
        mplib_push_S(right_type); lua_pushstring(L, knot_type_enum[mp_knot_right_type(mp, p)]); lua_rawset(L, -3);
        lua_pop(L,1);
        p = mp_knot_next(mp,p);
    }
    lua_pushboolean(L, 1);
    return 1;
  BAD:
    if (p != NULL) {
        mp_close_path (mp, p, first);
        mp_free_path (mp, p);
    }
    lua_pushboolean(L, 0);
    lua_pushstring(L, errormsg);
    return 2;
}

/*tex

    The next methods are for collecting the results from |fig|.

*/

static int mplib_fig_collect(lua_State * L)
{
    struct mp_edge_object **hh = is_fig(L, 1);
    if (*hh != NULL) {
        mp_gr_toss_objects(*hh);
        *hh = NULL;
    }
    return 0;
}

static int mplib_fig_body(lua_State * L)
{
    int i = 1;
    struct mp_graphic_object **v;
    struct mp_graphic_object *p;
    struct mp_edge_object **hh = is_fig(L, 1);
    lua_newtable(L);
    p = (*hh)->body;
    while (p != NULL) {
        v = lua_newuserdata(L, sizeof(struct mp_graphic_object *));
        *v = p;
        luaL_getmetatable(L, MPLIB_GR_METATABLE);
        lua_setmetatable(L, -2);
        lua_rawseti(L, -2, i);
        i++;
        p = p->next;
    }
    /*tex Prevent a double free: */
    (*hh)->body = NULL;
    return 1;
}

static int mplib_fig_copy_body(lua_State * L)
{
    int i = 1;
    struct mp_graphic_object **v;
    struct mp_graphic_object *p;
    struct mp_edge_object **hh = is_fig(L, 1);
    lua_newtable(L);
    p = (*hh)->body;
    while (p != NULL) {
        v = lua_newuserdata(L, sizeof(struct mp_graphic_object *));
        *v = mp_gr_copy_object((*hh)->parent, p);
        luaL_getmetatable(L, MPLIB_GR_METATABLE);
        lua_setmetatable(L, -2);
        lua_rawseti(L, -2, i);
        i++;
        p = p->next;
    }
    return 1;
}

static int mplib_fig_tostring(lua_State * L)
{
    struct mp_edge_object **hh = is_fig(L, 1);
    (void)lua_pushfstring(L, "<figure %p>", *hh);
    return 1;
}

static int mplib_fig_postscript(lua_State * L)
{
    mp_run_data *res;
    struct mp_edge_object **hh = is_fig(L, 1);
    int prologues = (int)luaL_optnumber(L, 2, (lua_Number)-1);
    int procset = (int)luaL_optnumber(L, 3, (lua_Number)-1);
    if (mp_ps_ship_out(*hh, prologues, procset)
        && (res = mp_rundata((*hh)->parent))
        && (res->ship_out.size != 0)) {
        lua_pushstring(L, res->ship_out.data);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int mplib_fig_svg(lua_State * L)
{
    mp_run_data *res;
    struct mp_edge_object **hh = is_fig(L, 1);
    int prologues = (int)luaL_optnumber(L, 2, (lua_Number)-1);
    if (mp_svg_ship_out(*hh, prologues)
        && (res = mp_rundata((*hh)->parent))
        && (res->ship_out.size != 0)) {
        lua_pushstring(L, res->ship_out.data);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int mplib_fig_png(lua_State * L)
{
    mp_run_data *res;
    struct mp_edge_object **hh = is_fig(L, 1);
    const char *string = luaL_optstring(L, 2, NULL);
    if (mp_png_ship_out(*hh, string)
        && (res = mp_rundata((*hh)->parent))
        && (res->ship_out.size != 0)) {
        lua_pushlstring(L, res->ship_out.data, res->ship_out.size);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int mplib_fig_filename(lua_State * L)
{
    struct mp_edge_object **hh = is_fig(L, 1);
    if (*hh != NULL) {
        char *s = (*hh)->filename;
        lua_pushstring(L, s);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int mplib_fig_width(lua_State * L)
{
    struct mp_edge_object **hh = is_fig(L, 1);
    if (*hh != NULL) {
      lua_pushnumber(L, (double) (*hh)->width);
    } else {
      lua_pushnil(L);
    }
    return 1;
}

static int mplib_fig_height(lua_State * L)
{
    struct mp_edge_object **hh = is_fig(L, 1);
    if (*hh != NULL) {
      lua_pushnumber(L, (double) (*hh)->height);
    } else {
      lua_pushnil(L);
    }
    return 1;
}

static int mplib_fig_depth(lua_State * L)
{
    struct mp_edge_object **hh = is_fig(L, 1);
    if (*hh != NULL) {
      lua_pushnumber(L, (double) (*hh)->depth);
    } else {
      lua_pushnil(L);
    }
    return 1;
}

static int mplib_fig_italcorr(lua_State * L)
{
    struct mp_edge_object **hh = is_fig(L, 1);
    if (*hh != NULL) {
      lua_pushnumber(L, (double) (*hh)->ital_corr);
    } else {
      lua_pushnil(L);
    }
    return 1;
}

static int mplib_fig_charcode(lua_State * L)
{
    struct mp_edge_object **hh = is_fig(L, 1);
    if (*hh != NULL) {
      lua_pushnumber(L, (lua_Number)(*hh)->charcode);
    } else {
      lua_pushnil(L);
    }
    return 1;
}

static int mplib_fig_bb(lua_State * L)
{
    struct mp_edge_object **hh = is_fig(L, 1);
    lua_newtable(L);
    lua_pushnumber(L, (double) (*hh)->minx);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, (double) (*hh)->miny);
    lua_rawseti(L, -2, 2);
    lua_pushnumber(L, (double) (*hh)->maxx);
    lua_rawseti(L, -2, 3);
    lua_pushnumber(L, (double) (*hh)->maxy);
    lua_rawseti(L, -2, 4);
    return 1;
}

/*tex

    The methods for the figure objects plus a few helpers.

*/

static int mplib_gr_collect(lua_State * L)
{
    struct mp_graphic_object **hh = is_gr_object(L, 1);
    if (*hh != NULL) {
        mp_gr_toss_object(*hh);
        *hh = NULL;
    }
    return 0;
}

static int mplib_gr_tostring(lua_State * L)
{
    struct mp_graphic_object **hh = is_gr_object(L, 1);
    (void)lua_pushfstring(L, "<object %p>", *hh);
    return 1;
}

#define pyth(a,b) (sqrt((a)*(a) + (b)*(b)))

#define aspect_bound   (10.0/65536.0)
#define aspect_default 1.0

static double eps  = 0.0001;

static double coord_range_x (mp_gr_knot h, double dz) {
    double z;
    double zlo = 0.0, zhi = 0.0;
    mp_gr_knot f = h;
    while (h != NULL) {
        z = h->x_coord;
        if (z < zlo)
            zlo = z;
        else if (z > zhi)
            zhi = z;
        z = h->right_x;
        if (z < zlo)
            zlo = z;
        else if (z > zhi)
            zhi = z;
        z = h->left_x;
        if (z < zlo)
            zlo = z;
        else if (z > zhi)
            zhi = z;
        h = h->next;
        if (h==f)
            break;
    }
    return (zhi - zlo <= dz ? aspect_bound : aspect_default);
}

static double coord_range_y (mp_gr_knot h, double dz) {
    double z;
    double zlo = 0.0, zhi = 0.0;
    mp_gr_knot f = h;
    while (h != NULL) {
        z = h->y_coord;
        if (z < zlo)
            zlo = z;
        else if (z > zhi)
            zhi = z;
        z = h->right_y;
        if (z < zlo)
            zlo = z;
        else if (z > zhi)
            zhi = z;
        z = h->left_y;
        if (z < zlo)
            zlo = z;
        else if (z > zhi)
            zhi = z;
        h = h->next;
        if (h==f)
            break;
    }
    return (zhi - zlo <= dz ? aspect_bound : aspect_default);
}

static int mplib_gr_peninfo(lua_State * L) {
    double x_coord, y_coord, left_x, left_y, right_x, right_y;
    double wx, wy;
    double rx = 1.0, sx = 0.0, sy = 0.0, ry = 1.0, tx = 0.0, ty = 0.0;
    double width = 1.0;
    mp_gr_knot p = NULL, path = NULL;
    struct mp_graphic_object **hh = is_gr_object(L, -1);
    if (!*hh) {
      lua_pushnil(L);
      return 1;
    }
    if ((*hh)->type == mp_fill_code) {
        p    = ((mp_fill_object *)(*hh))->pen_p;
        path = ((mp_fill_object *)(*hh))->path_p;
    } else if ((*hh)->type == mp_stroked_code) {
        p    = ((mp_stroked_object *)(*hh))->pen_p;
        path = ((mp_stroked_object *)(*hh))->path_p;
    }
    if (p==NULL || path == NULL) {
        lua_pushnil(L);
        return 1;
    }
    x_coord = p->x_coord;
    y_coord = p->y_coord;
    left_x  = p->left_x;
    left_y  = p->left_y;
    right_x = p->right_x;
    right_y = p->right_y;
    if ((right_x == x_coord) && (left_y == y_coord)) {
        wx = fabs(left_x  - x_coord);
        wy = fabs(right_y - y_coord);
    } else {
        wx = pyth(left_x - x_coord, right_x - x_coord);
        wy = pyth(left_y - y_coord, right_y - y_coord);
    }
    if ((wy/coord_range_x(path, wx)) >= (wx/coord_range_y(path, wy)))
        width = wy;
    else
        width = wx;
    tx = x_coord;
    ty = y_coord;
    sx = left_x - tx;
    rx = left_y - ty;
    ry = right_x - tx;
    sy = right_y - ty;
    if (width !=1.0) {
        if (width == 0.0) {
            sx = 1.0; sy = 1.0;
        } else {
            rx/=width; ry/=width; sx/=width; sy/=width;
        }
    }
    if (fabs(sx) < eps)
        sx = eps;
    if (fabs(sy) < eps)
        sy = eps;
    lua_newtable(L);
    lua_pushnumber(L,width); lua_setfield(L,-2,"width");
    lua_pushnumber(L,rx); lua_setfield(L,-2,"rx");
    lua_pushnumber(L,sx); lua_setfield(L,-2,"sx");
    lua_pushnumber(L,sy); lua_setfield(L,-2,"sy");
    lua_pushnumber(L,ry); lua_setfield(L,-2,"ry");
    lua_pushnumber(L,tx); lua_setfield(L,-2,"tx");
    lua_pushnumber(L,ty); lua_setfield(L,-2,"ty");
    return 1;
}

/*tex

    Here is a helper that reports the valid field names of the possible
    objects.

*/

static int mplib_gr_fields(lua_State * L)
{
    const char **fields;
    int i;
    struct mp_graphic_object **hh = is_gr_object(L, 1);
    if (*hh) {
        switch ((*hh)->type) {
        case mp_fill_code:
            fields = fill_fields;
            break;
        case mp_stroked_code:
            fields = stroked_fields;
            break;
        case mp_text_code:
            fields = text_fields;
            break;
        case mp_special_code:
            fields = special_fields;
            break;
        case mp_start_clip_code:
            fields = start_clip_fields;
            break;
        case mp_start_bounds_code:
            fields = start_bounds_fields;
            break;
        case mp_stop_clip_code:
            fields = stop_clip_fields;
            break;
        case mp_stop_bounds_code:
            fields = stop_bounds_fields;
            break;
        default:
            fields = no_fields;
        }
        lua_newtable(L);
        for (i = 0; fields[i] != NULL; i++) {
            lua_pushstring(L, fields[i]);
            lua_rawseti(L, -2, (i + 1));
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

#define mplib_push_number(L,x) lua_pushnumber(L,(lua_Number)(x))

#define MPLIB_PATH 0
#define MPLIB_PEN 1

static void mplib_push_path(lua_State * L, mp_gr_knot h, int is_pen)
{
    mp_gr_knot p;
    int i = 1;
    p = h;
    if (p != NULL) {
        lua_newtable(L);
        do {
            lua_createtable(L, 0, 6);
            if (!is_pen) {
                if (p->data.types.left_type != mp_explicit) {
                    mplib_push_S(left_type);
                    lua_pushstring(L, knot_type_enum[p->data.types.left_type]);
                    lua_rawset(L, -3);
                }
                if (p->data.types.right_type != mp_explicit) {
                    mplib_push_S(right_type);
                    lua_pushstring(L, knot_type_enum[p->data.types.right_type]);
                    lua_rawset(L, -3);
                }
            }
            mplib_push_S(x_coord);
            mplib_push_number(L, p->x_coord);
            lua_rawset(L, -3);
            mplib_push_S(y_coord);
            mplib_push_number(L, p->y_coord);
            lua_rawset(L, -3);
            mplib_push_S(left_x);
            mplib_push_number(L, p->left_x);
            lua_rawset(L, -3);
            mplib_push_S(left_y);
            mplib_push_number(L, p->left_y);
            lua_rawset(L, -3);
            mplib_push_S(right_x);
            mplib_push_number(L, p->right_x);
            lua_rawset(L, -3);
            mplib_push_S(right_y);
            mplib_push_number(L, p->right_y);
            lua_rawset(L, -3);
            lua_rawseti(L, -2, i);
            i++;
            if (p->data.types.right_type == mp_endpoint) {
                return;
            }
            p = p->next;
        } while (p != h);
    } else {
        lua_pushnil(L);
    }
}

static int mplib_get_path(lua_State * L)
{
    MP *mp = is_mp(L, 1);
    if (*mp != NULL) {
        size_t l;
        const char *s = lua_tolstring(L, 2, &l);
        if (s != NULL) {
            mp_knot p = mp_get_path_value(*mp,s,l) ;
            if (p != NULL) {
                int i = 1;
                mp_knot h = p;
                lua_newtable(L);
                do {
                    lua_createtable(L, 6, 1);
                    mplib_push_number(L, mp_number_as_double(*mp,p->x_coord));
                    lua_rawseti(L,-2,1);
                    mplib_push_number(L, mp_number_as_double(*mp,p->y_coord));
                    lua_rawseti(L,-2,2);
                    mplib_push_number(L, mp_number_as_double(*mp,p->left_x));
                    lua_rawseti(L,-2,3);
                    mplib_push_number(L, mp_number_as_double(*mp,p->left_y));
                    lua_rawseti(L,-2,4);
                    mplib_push_number(L, mp_number_as_double(*mp,p->right_x));
                    lua_rawseti(L,-2,5);
                    mplib_push_number(L, mp_number_as_double(*mp,p->right_y));
                    lua_rawseti(L,-2,6);
                    lua_rawseti(L,-2, i);
                    i++;
                    if (p->data.types.right_type == mp_endpoint) {
                        mplib_push_S(cycle);
                        lua_pushboolean(L,0);
                        lua_rawset(L,-3);
                        return 1;
                    }
                    p = p->next;
                } while (p != h);
                mplib_push_S(cycle);
                lua_pushboolean(L,1);
                lua_rawset(L,-3);
                return 1;
            }
        }
    }
    return 0;
}

/*tex

    This assumes that the top of the stack is a table or nil already in the case.
*/

static void mplib_push_pentype(lua_State * L, mp_gr_knot h)
{
    mp_gr_knot p;
    p = h;
    if (p == NULL) {
        /*tex Do nothing. */
    } else if (p == p->next) {
        mplib_push_S(type);
        mplib_push_S(elliptical);
        lua_rawset(L, -3);
    } else {
    }
}

#define set_color_objects(pq) \
object_color_model = pq->color_model; \
object_color_a = pq->color.a_val; \
object_color_b = pq->color.b_val; \
object_color_c = pq->color.c_val; \
object_color_d = pq->color.d_val;

static void mplib_push_color(lua_State * L, struct mp_graphic_object *p)
{
    int object_color_model;
    double object_color_a, object_color_b, object_color_c, object_color_d;
    if (p != NULL) {
        if (p->type == mp_fill_code) {
            mp_fill_object *h = (mp_fill_object *) p;
            set_color_objects(h);
        } else if (p->type == mp_stroked_code) {
            mp_stroked_object *h = (mp_stroked_object *) p;
            set_color_objects(h);
        } else {
            mp_text_object *h = (mp_text_object *) p;
            set_color_objects(h);
        }
        lua_newtable(L);
        if (object_color_model >= mp_grey_model) {
            mplib_push_number(L, object_color_a);
            lua_rawseti(L, -2, 1);
            if (object_color_model >= mp_rgb_model) {
                mplib_push_number(L, object_color_b);
                lua_rawseti(L, -2, 2);
                mplib_push_number(L, object_color_c);
                lua_rawseti(L, -2, 3);
                if (object_color_model == mp_cmyk_model) {
                    mplib_push_number(L, object_color_d);
                    lua_rawseti(L, -2, 4);
                }
            }
        }
    } else {
        lua_pushnil(L);
    }
}

/*tex

    The dash scale is not exported, the field has no external value.

*/

static void mplib_push_dash(lua_State * L, struct mp_stroked_object *h)
{
    mp_dash_object *d;
    double ds;
    if (h != NULL && h->dash_p != NULL) {
        d = h->dash_p;
        lua_newtable(L);
        mplib_push_S(offset);
        mplib_push_number(L, d->offset);
        lua_rawset(L,-3);
        if (d->array != NULL) {
            int i = 0;
            mplib_push_S(dashes);
            lua_newtable(L);
            while (*(d->array + i) != -1) {
                ds = *(d->array + i);
                lua_pushnumber(L, ds);
                i++;
                lua_rawseti(L, -2, i);
            }
            lua_rawset(L,-3);
        }
    } else {
        lua_pushnil(L);
    }
}

static void mplib_push_transform(lua_State * L, struct mp_text_object *h)
{
    int i = 1;
    if (h != NULL) {
        lua_createtable(L, 6, 0);
        mplib_push_number(L, h->tx);
        lua_rawseti(L, -2, i);
        i++;
        mplib_push_number(L, h->ty);
        lua_rawseti(L, -2, i);
        i++;
        mplib_push_number(L, h->txx);
        lua_rawseti(L, -2, i);
        i++;
        mplib_push_number(L, h->tyx);
        lua_rawseti(L, -2, i);
        i++;
        mplib_push_number(L, h->txy);
        lua_rawseti(L, -2, i);
        i++;
        mplib_push_number(L, h->tyy);
        lua_rawseti(L, -2, i);
        i++;
    } else {
        lua_pushnil(L);
    }
}

#define FIELD(A) (mplib_is_S(A,2))

static void mplib_fill(lua_State * L, struct mp_fill_object *h)
{
    if (FIELD(path)) {
        mplib_push_path(L, h->path_p, MPLIB_PATH);
    } else if (FIELD(htap)) {
        mplib_push_path(L, h->htap_p, MPLIB_PATH);
    } else if (FIELD(pen)) {
        mplib_push_path(L, h->pen_p, MPLIB_PEN);
        mplib_push_pentype(L, h->pen_p);
    } else if (FIELD(color)) {
        mplib_push_color(L, (mp_graphic_object *) h);
    } else if (FIELD(linejoin)) {
      lua_pushnumber(L, (lua_Number)h->ljoin);
    } else if (FIELD(miterlimit)) {
        mplib_push_number(L, h->miterlim);
    } else if (FIELD(prescript)) {
        lua_pushstring(L, h->pre_script);
    } else if (FIELD(postscript)) {
        lua_pushstring(L, h->post_script);
    } else {
        lua_pushnil(L);
    }
}

static void mplib_stroked(lua_State * L, struct mp_stroked_object *h)
{
    if (FIELD(path)) {
        mplib_push_path(L, h->path_p, MPLIB_PATH);
    } else if (FIELD(pen)) {
        mplib_push_path(L, h->pen_p, MPLIB_PEN);
        mplib_push_pentype(L, h->pen_p);
    } else if (FIELD(color)) {
        mplib_push_color(L, (mp_graphic_object *) h);
    } else if (FIELD(dash)) {
        mplib_push_dash(L, h);
    } else if (FIELD(linecap)) {
        lua_pushnumber(L, (lua_Number)h->lcap);
    } else if (FIELD(linejoin)) {
      lua_pushnumber(L, (lua_Number)h->ljoin);
    } else if (FIELD(miterlimit)) {
        mplib_push_number(L, h->miterlim);
    } else if (FIELD(prescript)) {
        lua_pushstring(L, h->pre_script);
    } else if (FIELD(postscript)) {
        lua_pushstring(L, h->post_script);
    } else {
        lua_pushnil(L);
    }
}

static void mplib_text(lua_State * L, struct mp_text_object *h)
{
    if (FIELD(text)) {
        lua_pushstring(L, h->text_p);
    } else if (FIELD(dsize)) {
        mplib_push_number(L, (h->font_dsize / 16));
    } else if (FIELD(font)) {
        lua_pushstring(L, h->font_name);
    } else if (FIELD(color)) {
        mplib_push_color(L, (mp_graphic_object *) h);
    } else if (FIELD(width)) {
        mplib_push_number(L, h->width);
    } else if (FIELD(height)) {
        mplib_push_number(L, h->height);
    } else if (FIELD(depth)) {
        mplib_push_number(L, h->depth);
    } else if (FIELD(transform)) {
        mplib_push_transform(L, h);
    } else if (FIELD(prescript)) {
        lua_pushstring(L, h->pre_script);
    } else if (FIELD(postscript)) {
        lua_pushstring(L, h->post_script);
    } else {
        lua_pushnil(L);
    }
}

static void mplib_special(lua_State * L, struct mp_special_object *h)
{
    if (FIELD(prescript)) {
        lua_pushstring(L, h->pre_script);
    } else {
        lua_pushnil(L);
    }
}

static void mplib_start_bounds(lua_State * L, struct mp_bounds_object *h)
{
    if (FIELD(path)) {
        mplib_push_path(L, h->path_p, MPLIB_PATH);
    } else {
        lua_pushnil(L);
    }
}

static void mplib_start_clip(lua_State * L, struct mp_clip_object *h)
{
    if (FIELD(path)) {
        mplib_push_path(L, h->path_p, MPLIB_PATH);
    } else {
        lua_pushnil(L);
    }
}

static int mplib_gr_index(lua_State * L)
{
    struct mp_graphic_object **hh = is_gr_object(L, 1);
    if (*hh) {
        struct mp_graphic_object *h = *hh;
        if (mplib_is_S(type, 2)) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, mplib_type_Ses[h->type]);
        } else {
            switch (h->type) {
                case mp_fill_code:
                    mplib_fill(L, (mp_fill_object *) h);
                    break;
                case mp_stroked_code:
                    mplib_stroked(L, (mp_stroked_object *) h);
                    break;
                case mp_text_code:
                    mplib_text(L, (mp_text_object *) h);
                    break;
                case mp_special_code:
                    mplib_special(L, (mp_special_object *) h);
                    break;
                case mp_start_clip_code:
                    mplib_start_clip(L, (mp_clip_object *) h);
                    break;
                case mp_start_bounds_code:
                    mplib_start_bounds(L, (mp_bounds_object *) h);
                    break;
                case mp_stop_clip_code:
                case mp_stop_bounds_code:
                default:
                    lua_pushnil(L);
            }
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static const struct luaL_reg mplib_meta[] = {
    { "__gc",       mplib_collect },
    { "__tostring", mplib_tostring },
    /*tex sentinel */
    { NULL,         NULL}
};

static const struct luaL_reg mplib_fig_meta[] = {
    { "__gc",         mplib_fig_collect },
    { "__tostring",   mplib_fig_tostring },
    { "objects",      mplib_fig_body },
    { "copy_objects", mplib_fig_copy_body },
    { "filename",     mplib_fig_filename },
    { "postscript",   mplib_fig_postscript },
    { "png",          mplib_fig_png },
    { "svg",          mplib_fig_svg },
    { "boundingbox",  mplib_fig_bb },
    { "width",        mplib_fig_width },
    { "height",       mplib_fig_height },
    { "depth",        mplib_fig_depth },
    { "italcorr",     mplib_fig_italcorr },
    { "charcode",     mplib_fig_charcode },
    /*tex sentinel */
    { NULL,           NULL}
};

static const struct luaL_reg mplib_gr_meta[] = {
    { "__gc",       mplib_gr_collect},
    { "__tostring", mplib_gr_tostring},
    { "__index",    mplib_gr_index},
    /*tex sentinel */
    { NULL,         NULL}
};

static const struct luaL_reg mplib_d[] = {
    { "execute",     mplib_execute },
    { "finish",      mplib_finish },
    { "char_width",  mplib_charwidth },
    { "char_height", mplib_charheight },
    { "char_depth",  mplib_chardepth },
    { "statistics",  mplib_statistics },
    { "solve_path",  mplib_solve_path },
    { "get_numeric", mplib_get_numeric },
    { "get_number",  mplib_get_numeric },
    { "get_boolean", mplib_get_boolean },
    { "get_string",  mplib_get_string },
    { "get_path",    mplib_get_path },
    /*tex sentinel */
    {NULL,           NULL }
};

static const struct luaL_reg mplib_m[] = {
    { "new",         mplib_new },
    { "version",     mplib_version },
    { "fields",      mplib_gr_fields },
    /* indirect */
    { "execute",     mplib_execute },
    { "finish",      mplib_finish },
    { "char_width",  mplib_charwidth },
    { "char_height", mplib_charheight },
    { "char_depth",  mplib_chardepth },
    { "statistics",  mplib_statistics },
    { "solve_path",  mplib_solve_path },
    /* helpers */
    { "pen_info",    mplib_gr_peninfo },
    { "get_numeric", mplib_get_numeric },
    { "get_number",  mplib_get_numeric },
    { "get_boolean", mplib_get_boolean },
    { "get_string",  mplib_get_string },
    { "get_path",    mplib_get_path },
    /*tex sentinel */
    { NULL,          NULL}
};

int luaopen_mplib(lua_State * L)
{
    mplib_init_Ses(L);

    luaL_newmetatable(L, MPLIB_GR_METATABLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_register(L, NULL, mplib_gr_meta);
    lua_pop(L, 1);

    luaL_newmetatable(L, MPLIB_FIG_METATABLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_register(L, NULL, mplib_fig_meta);
    lua_pop(L, 1);

    luaL_newmetatable(L, MPLIB_METATABLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_register(L, NULL, mplib_meta);
    luaL_register(L, NULL, mplib_d);
    luaL_register(L, "mplib", mplib_m);

    return 1;
}
