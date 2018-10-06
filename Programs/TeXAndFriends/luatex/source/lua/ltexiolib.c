/* ltexiolib.c

   Copyright 2006-2010 Taco Hoekwater <taco@luatex.org>

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

typedef void (*texio_printer) (const char *);

static char *loggable_info = NULL;

static boolean get_selector_value(lua_State * L, int i, int *l)
{
    boolean r = false;
    int t = lua_type(L,i);
    if (t == LUA_TSTRING) {
        const char *s = lua_tostring(L, i);
        if (lua_key_eq(s,term_and_log)) {
            *l = term_and_log;
            r = true;
        } else if (lua_key_eq(s,log)) {
            *l = log_only;
            r = true;
        } else if (lua_key_eq(s,term)) {
            *l = term_only;
            r = true;
        } else {
            *l = term_and_log;
            r = true;
        }
    } else if (t == LUA_TNUMBER) {
        int n = lua_tointeger(L,i);
        if (file_can_be_written(n)) {
            *l = n;
            r = true;
        } else {
            *l = term_and_log;
            r = true;
        }
    } else {
        luaL_error(L, "first argument is not 'term and log', 'term', 'log' or a number");
    }
    return r;
}

static int do_texio_print(lua_State * L, texio_printer printfunction)
{
    const char *s;
    int i = 1;
    int save_selector = selector;
    int n = lua_gettop(L);
    if (n == 0 || !lua_isstring(L, -1)) { /* or number */
        luaL_error(L, "no string to print"); /* or number */
    }
    if (n > 1) {
        if (get_selector_value(L, i, &selector))
            i++;
    }
    if (selector != term_and_log && selector != log_only && selector != term_only) {
        if (! valid_write_file(selector)) {
            normalize_selector();   /* sets selector */
        }
    }
    for (; i <= n; i++) {
        if (lua_isstring(L, i)) { /* or number */
            s = lua_tostring(L, i);
            printfunction(s);
        } else {
            luaL_error(L, "argument is not a string");
        }
    }
    selector = save_selector;
    return 0;
}

static void do_texio_ini_print(lua_State * L, const char *extra)
{
    const char *s;
    int i = 1;
    int l = term_and_log;
    int n = lua_gettop(L);
    if (n > 1) {
        if (get_selector_value(L, i, &l))
            i++;
    }
    for (; i <= n; i++) {
        if (lua_isstring(L, i)) { /* or number */
            s = lua_tostring(L, i);
            if (l == term_and_log || l == term_only)
                fprintf(stdout, "%s%s", extra, s);
            if (l == log_only || l == term_and_log) {
                if (loggable_info == NULL) {
                    loggable_info = strdup(s);
                } else {
                    char *v = concat3(loggable_info, extra, s);
                    free(loggable_info);
                    loggable_info = v;
                }
            }
        }
    }
}

static int texio_print(lua_State * L)
{
    if (ready_already != 314159 || job_name == 0) {
        do_texio_ini_print(L, "");
        return 0;
    }
    return do_texio_print(L, tprint);
}

static int texio_printnl(lua_State * L)
{
    if (ready_already != 314159 || job_name == 0) {
        do_texio_ini_print(L, "\n");
        return 0;
    }
    return do_texio_print(L, tprint_nl);
}

/* at the point this function is called, the selector is log_only */

void flush_loggable_info(void)
{
    if (loggable_info != NULL) {
        fprintf(log_file, "%s\n", loggable_info);
        free(loggable_info);
        loggable_info = NULL;
    }
}

static int texio_setescape(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TBOOLEAN) {
        escape_controls = lua_toboolean(L,1);
    } else {
        escape_controls = lua_tointeger(L,1);
    }
    return 0 ;
}

static int texio_closeinput(lua_State * L)
{
    /*
        printf("before, first %i, index %i, iname %i, inopen %i, pointer %i\n",istart,iindex,iname,in_open,input_ptr);
    */
    if (iindex > 0) {
        end_token_list();
        end_file_reading();
        /*
            printf("after, first %i, index %i, iname %i, inopen %i, pointer %i\n",istart,iindex,iname,in_open,input_ptr);
        */
    }
    return 0 ;
}

static const struct luaL_Reg texiolib[] = {
    {"write", texio_print},
    {"write_nl", texio_printnl},
    {"setescape", texio_setescape},
    {"closeinput",texio_closeinput},
    {NULL, NULL}
};

int luaopen_texio(lua_State * L)
{
    luaL_openlib(L, "texio", texiolib, 0);
    return 1;
}
