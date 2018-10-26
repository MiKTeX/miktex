/* lcallbacklib.c

   Copyright 2006-2008 Taco Hoekwater <taco@luatex.org>

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

int callback_count = 0;
int saved_callback_count = 0;
int direct_callback_count = 0;
int late_callback_count = 0;
int function_callback_count = 0;

int callback_set[total_callbacks] = { 0 };

/* See also callback_callback_type in luatexcallbackids.h: they must have the same order ! */

static const char *const callbacknames[] = {
    "", /* empty on purpose */
    "find_write_file",
    "find_output_file",
    "find_image_file",
    "find_format_file",
    "find_read_file", "open_read_file",
    "find_vf_file", "read_vf_file",
    "find_data_file", "read_data_file",
    "find_font_file", "read_font_file",
    "find_map_file", "read_map_file",
    "find_enc_file", "read_enc_file",
    "find_type1_file", "read_type1_file",
    "find_truetype_file", "read_truetype_file",
    "find_opentype_file", "read_opentype_file",
    "find_cidmap_file", "read_cidmap_file",
    "find_pk_file", "read_pk_file",
    "show_error_hook",
    "process_input_buffer", "process_output_buffer",
    "process_jobname",
    "start_page_number", "stop_page_number",
    "start_run", "stop_run",
    "define_font",
    "pre_output_filter",
    "buildpage_filter",
    "hpack_filter", "vpack_filter",
    "glyph_not_found",
    "hyphenate",
    "ligaturing",
    "kerning",
    "pre_linebreak_filter",
    "linebreak_filter",
    "post_linebreak_filter",
    "append_to_vlist_filter",
    "mlist_to_hlist",
    "finish_pdffile",
    "finish_pdfpage",
    "pre_dump","start_file", "stop_file",
    "show_error_message","show_lua_error_hook",
    "show_warning_message",
    "hpack_quality", "vpack_quality",
    "process_rule",
    "insert_local_par",
    "contribute_filter",
    "call_edit",
    "build_page_insert",
    "glyph_stream_provider",
    "font_descriptor_objnum_provider",
    "finish_synctex",
    "wrapup_run",
    "new_graf",
    "page_objnum_provider",
    "make_extensible",
    "process_pdf_image_content",
    NULL
};

int callback_callbacks_id = 0;

int debug_callback_defined(int i)
{
    printf ("callback_defined(%s)\n", callbacknames[i]);
    return callback_set[i];
}

void get_lua_boolean(const char *table, const char *name, boolean * target)
{
    int stacktop = lua_gettop(Luas);
    luaL_checkstack(Luas, 2, "out of stack space");
    lua_getglobal(Luas, table);
    if (lua_istable(Luas, -1)) {
        int t;
        lua_getfield(Luas, -1, name);
        t = lua_type(Luas, -1);
        if (t == LUA_TBOOLEAN) {
            *target = (boolean) (lua_toboolean(Luas, -1));
        } else if (t == LUA_TNUMBER) {
            *target = (boolean) (lua_tointeger(Luas, -1) == 0 ? 0 : 1);
        }
    }
    lua_settop(Luas, stacktop);
    return;
}

void get_saved_lua_boolean(int r, const char *name, boolean * target)
{
    int stacktop = lua_gettop(Luas);
    luaL_checkstack(Luas, 2, "out of stack space");
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, r);
    if (lua_istable(Luas, -1)) {
        int t;
        lua_getfield(Luas, -1, name);
        t = lua_type(Luas, -1);
        if (t == LUA_TBOOLEAN) {
            *target = (boolean) lua_toboolean(Luas, -1);
        } else if (t == LUA_TNUMBER) {
            *target = (boolean) (lua_tointeger(Luas, -1) == 0 ? 0 : 1);
        }
    }
    lua_settop(Luas, stacktop);
    return;
}

void get_lua_number(const char *table, const char *name, int *target)
{
    int stacktop = lua_gettop(Luas);
    luaL_checkstack(Luas, 2, "out of stack space");
    lua_getglobal(Luas, table);
    if (lua_istable(Luas, -1)) {
        lua_getfield(Luas, -1, name);
        if (lua_type(Luas, -1) == LUA_TNUMBER) {
            *target = (int) lua_roundnumber(Luas, -1); /* was lua_tointeger */
        }
    }
    lua_settop(Luas, stacktop);
    return;
}

void get_saved_lua_number(int r, const char *name, int *target)
{
    int stacktop = lua_gettop(Luas);
    luaL_checkstack(Luas, 2, "out of stack space");
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, r);
    if (lua_istable(Luas, -1)) {
        lua_getfield(Luas, -1, name);
        if (lua_type(Luas, -1) == LUA_TNUMBER) {
            *target = (int) lua_tointeger(Luas, -1);
        }
    }
    lua_settop(Luas, stacktop);
    return;
}

void get_lua_string(const char *table, const char *name, char **target)
{
    int stacktop = lua_gettop(Luas);
    luaL_checkstack(Luas, 2, "out of stack space");
    lua_getglobal(Luas, table);
    if (lua_type(Luas, -1) == LUA_TTABLE) {
        lua_getfield(Luas, -1, name);
        if (lua_type(Luas, -1) == LUA_TSTRING) {
            *target = xstrdup(lua_tostring(Luas, -1));
        }
    }
    lua_settop(Luas, stacktop);
    return;
}

void get_saved_lua_string(int r, const char *name, char **target)
{
    int stacktop = lua_gettop(Luas);
    luaL_checkstack(Luas, 2, "out of stack space");
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, r);
    if (lua_type(Luas, -1) == LUA_TTABLE) {
        lua_getfield(Luas, -1, name);
        if (lua_type(Luas, -1) == LUA_TSTRING) {
            *target = xstrdup(lua_tostring(Luas, -1));
        }
    }
    lua_settop(Luas, stacktop);
    return;
}

#define CALLBACK_BOOLEAN        'b'
#define CALLBACK_INTEGER        'd'
#define CALLBACK_LINE           'l'
#define CALLBACK_STRNUMBER      's'
#define CALLBACK_STRING         'S'
#define CALLBACK_RESULT         'R' /* a string but nil is also ok */
#define CALLBACK_CHARNUM        'c'
#define CALLBACK_LSTRING        'L'
#define CALLBACK_NODE           'N'
#define CALLBACK_DIR            'D'

int run_saved_callback(int r, const char *name, const char *values, ...)
{
    va_list args;
    int ret = 0;
    int stacktop = lua_gettop(Luas);
    va_start(args, values);
    luaL_checkstack(Luas, 2, "out of stack space");
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, r);
    lua_pushstring(Luas, name);
    lua_rawget(Luas, -2);
    if (lua_isfunction(Luas, -1)) {
        saved_callback_count++;
        ret = do_run_callback(2, values, args);
    }
    va_end(args);
    lua_settop(Luas, stacktop);
    return ret;
}

boolean get_callback(lua_State * L, int i)
{
    luaL_checkstack(L, 2, "out of stack space");
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback_callbacks_id);
    lua_rawgeti(L, -1, i);
    if (lua_isfunction(L, -1)) {
        callback_count++;
        return true;
    } else {
        return false;
    }
}

int run_and_save_callback(int i, const char *values, ...)
{
    va_list args;
    int ret = 0;
    int stacktop = lua_gettop(Luas);
    va_start(args, values);
    if (get_callback(Luas, i)) {
        ret = do_run_callback(1, values, args);
    }
    va_end(args);
    if (ret > 0) {
        ret = luaL_ref(Luas, LUA_REGISTRYINDEX);
    }
    lua_settop(Luas, stacktop);
    return ret;
}

int run_callback(int i, const char *values, ...)
{
    va_list args;
    int ret = 0;
    int stacktop = lua_gettop(Luas);
    va_start(args, values);
    if (get_callback(Luas, i)) {
        ret = do_run_callback(0, values, args);
    }
    va_end(args);
    lua_settop(Luas, stacktop);
    return ret;
}

int do_run_callback(int special, const char *values, va_list vl)
{
    int ret;
    size_t len;
    int narg, nres;
    const char *s;
    lstring *lstr;
    char cs;
    int *bufloc;
    char *ss = NULL;
    int retval = 0;
    if (special == 2) {         /* copy the enclosing table */
        luaL_checkstack(Luas, 1, "out of stack space");
        lua_pushvalue(Luas, -2);
    }
    ss = strchr(values, '>');
    luaL_checkstack(Luas, (int) (ss - values + 1), "out of stack space");
    ss = NULL;
    for (narg = 0; *values; narg++) {
        switch (*values++) {
            case CALLBACK_CHARNUM: /* an ascii char! */
                cs = (char) va_arg(vl, int);
                lua_pushlstring(Luas, &cs, 1);
                break;
            case CALLBACK_STRING:  /* C string */
                s = va_arg(vl, char *);
                lua_pushstring(Luas, s);
                break;
            case CALLBACK_LSTRING:  /* 'lstring' */
                lstr = va_arg(vl, lstring *);
                lua_pushlstring(Luas, (const char *)lstr->s, lstr->l);
                break;
            case CALLBACK_INTEGER: /* int */
                lua_pushinteger(Luas, va_arg(vl, int));
                break;
            case CALLBACK_STRNUMBER:       /* TeX string */
                s = makeclstring(va_arg(vl, int), &len);
                lua_pushlstring(Luas, s, len);
                break;
            case CALLBACK_BOOLEAN: /* boolean */
                lua_pushboolean(Luas, va_arg(vl, int));
                break;
            case CALLBACK_LINE:    /* a buffer section, with implied start */
                lua_pushlstring(Luas, (char *) (buffer + first), (size_t) va_arg(vl, int));
                break;
            case CALLBACK_NODE:
                lua_nodelib_push_fast(Luas, va_arg(vl, int));
                break;
            case CALLBACK_DIR:
                lua_push_dir_par(Luas, va_arg(vl, int));
                break;
            case '-':
                narg--;
                break;
            case '>':
                goto ENDARGS;
            default:
                ;
        }
    }
  ENDARGS:
    nres = (int) strlen(values);
    if (special == 1) {
        nres++;
    }
    if (special == 2) {
        narg++;
    }
    {
        int i;
        lua_active++;
        i = lua_pcall(Luas, narg, nres, 0);
        lua_active--;
        /* lua_remove(L, base); *//* remove traceback function */
        if (i != 0) {
            /* Can't be more precise here, could be called before
             * TeX initialization is complete
             */
            if (!log_opened_global) {
                fprintf(stderr, "error in callback: %s\n", lua_tostring(Luas, -1));
                error();
            } else {
                lua_gc(Luas, LUA_GCCOLLECT, 0);
                luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
            }
            return 0;
        }
    }
    if (nres == 0) {
        return 1;
    }
    nres = -nres;
    while (*values) {
        int b, t;
        halfword p;
        t = lua_type(Luas, nres);
        switch (*values++) {
            case CALLBACK_BOOLEAN:
                if (t == LUA_TNIL) {
                    b = 0;
                } else if (t != LUA_TBOOLEAN) {
                    fprintf(stderr, "callback should return a boolean, not: %s\n", lua_typename(Luas, t));
                    goto EXIT;
                } else {
                    b = lua_toboolean(Luas, nres);
                }
                *va_arg(vl, boolean *) = (boolean) b;
                break;
            case CALLBACK_INTEGER:
                if (t != LUA_TNUMBER) {
                    fprintf(stderr, "callback should return a number, not: %s\n", lua_typename(Luas, t));
                    goto EXIT;
                }
                b = lua_tointeger(Luas, nres);
                *va_arg(vl, int *) = b;
                break;
            case CALLBACK_LINE:    /* TeX line ... happens frequently when we have a plug-in */
                if (t == LUA_TNIL) {
                    bufloc = 0;
                    goto EXIT;
                } else if (t == LUA_TSTRING) {
                    s = lua_tolstring(Luas, nres, &len);
                    if (s == NULL) {    /* |len| can be zero */
                        bufloc = 0;
                    } else if (len == 0) {
                        bufloc = 0;
                    } else {
                        bufloc = va_arg(vl, int *);
                        ret = *bufloc;
                        check_buffer_overflow(ret + (int) len);
                        strncpy((char *) (buffer + ret), s, len);
                        *bufloc += (int) len;
                        /* while (len--) {  buffer[(*bufloc)++] = *s++; } */
                        while ((*bufloc) - 1 > ret && buffer[(*bufloc) - 1] == ' ')
                            (*bufloc)--;
                    }
                } else {
                    fprintf(stderr, "callback should return a string, not: %s\n", lua_typename(Luas, t));
                    goto EXIT;
                }
                break;
            case CALLBACK_STRNUMBER:       /* TeX string */
                if (t != LUA_TSTRING) {
                    fprintf(stderr, "callback should return a string, not: %s\n", lua_typename(Luas, t));
                    goto EXIT;
                }
                s = lua_tolstring(Luas, nres, &len);
                if (s == NULL)      /* |len| can be zero */
                    *va_arg(vl, int *) = 0;
                else {
                    *va_arg(vl, int *) = maketexlstring(s, len);
                }
                break;
            case CALLBACK_STRING:  /* C string aka buffer */
                if (t != LUA_TSTRING) {
                    fprintf(stderr, "callback should return a string, not: %s\n", lua_typename(Luas, t));
                    goto EXIT;
                }
                s = lua_tolstring(Luas, nres, &len);
                if (s == NULL)      /* |len| can be zero */
                    *va_arg(vl, int *) = 0;
                else {
                    ss = xmalloc((unsigned) (len + 1));
                    (void) memcpy(ss, s, (len + 1));
                    *va_arg(vl, char **) = ss;
                }
                break;
            case CALLBACK_RESULT:  /* C string aka buffer */
                if (t == LUA_TNIL) {
                    *va_arg(vl, int *) = 0;
                } else if (t == LUA_TBOOLEAN) {
                    b = lua_toboolean(Luas, nres);
                    if (b == 0) {
                        *va_arg(vl, int *) = 0;
                    } else {
                        fprintf(stderr, "callback should return a string, false or nil, not: %s\n", lua_typename(Luas, t));
                        goto EXIT;
                    }
                } else if (t != LUA_TSTRING) {
                    fprintf(stderr, "callback should return a string, false or nil, not: %s\n", lua_typename(Luas, t));
                    goto EXIT;
                } else {
                    s = lua_tolstring(Luas, nres, &len);
                    if (s == NULL)      /* |len| can be zero */
                        *va_arg(vl, int *) = 0;
                    else {
                        ss = xmalloc((unsigned) (len + 1));
                        (void) memcpy(ss, s, (len + 1));
                        *va_arg(vl, char **) = ss;
                    }
                }
                break;
            case CALLBACK_LSTRING:  /* lstring */
                if (t != LUA_TSTRING) {
                    fprintf(stderr, "callback should return a string, not: %s\n", lua_typename(Luas, t));
                    goto EXIT;
                }
                s = lua_tolstring(Luas, nres, &len);
                if (s == NULL)      /* |len| can be zero */
                    *va_arg(vl, int *) = 0;
                else {
                    lstring *ret = xmalloc(sizeof(lstring));
                    ret->s = xmalloc((unsigned) (len + 1));
                    (void) memcpy(ret->s, s, (len + 1));
                    ret->l = len;
                    *va_arg(vl, lstring **) = ret;
                }
                break;
            case CALLBACK_NODE:
                if (t == LUA_TNIL) {
                    p = null;
                } else {
                    p = *check_isnode(Luas,nres);
                }
                *va_arg(vl, int *) = p;
                break;
            default:
                fprintf(stdout, "callback returned an invalid value type");
                goto EXIT;
        }
        nres++;
    }
    retval = 1;
  EXIT:
    return retval;
}

void destroy_saved_callback(int i)
{
    luaL_unref(Luas, LUA_REGISTRYINDEX, i);
}

static int callback_register(lua_State * L)
{
    int cb;
    const char *s;
    int t1 = lua_type(L,1);
    int t2 = lua_type(L,2);
    if (t1 != LUA_TSTRING) {
        lua_pushnil(L);
        lua_pushstring(L, "Invalid arguments to callback.register, first argument must be string.");
        return 2;
    }
    if ((t2 != LUA_TFUNCTION) && (t2 != LUA_TNIL) && ((t2 != LUA_TBOOLEAN) && (lua_toboolean(L, 2) == 0))) {
        lua_pushnil(L);
        lua_pushstring(L, "Invalid arguments to callback.register.");
        return 2;
    }
    s = lua_tostring(L, 1);
    for (cb = 0; cb < total_callbacks; cb++) {
        if (strcmp(callbacknames[cb], s) == 0)
            break;
    }
    if (cb == total_callbacks) {
        lua_pushnil(L);
        lua_pushstring(L, "No such callback exists.");
        return 2;
    }
    if (t2 == LUA_TFUNCTION) {
        callback_set[cb] = cb;
    } else if (t2 == LUA_TBOOLEAN) {
        callback_set[cb] = -1;
    } else {
        callback_set[cb] = 0;
    }
    luaL_checkstack(L, 2, "out of stack space");
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback_callbacks_id);   /* push the table */
    lua_pushvalue(L, 2);        /* the function or nil */
    lua_rawseti(L, -2, cb);
    lua_rawseti(L, LUA_REGISTRYINDEX, callback_callbacks_id);
    lua_pushinteger(L, cb);
    return 1;
}

static int callback_find(lua_State * L)
{
    int cb;
    const char *s;
    if (lua_type(L, 1) != LUA_TSTRING) {
        lua_pushnil(L);
        lua_pushstring(L, "Invalid arguments to callback.find.");
        return 2;
    }
    s = lua_tostring(L, 1);
    for (cb = 0; cb < total_callbacks; cb++) {
        if (strcmp(callbacknames[cb], s) == 0)
            break;
    }
    if (cb == total_callbacks) {
        lua_pushnil(L);
        lua_pushstring(L, "No such callback exists.");
        return 2;
    }
    luaL_checkstack(L, 2, "out of stack space");
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback_callbacks_id);   /* push the table */
    lua_rawgeti(L, -1, cb);
    return 1;
}

static int callback_listf(lua_State * L)
{
    int i;
    luaL_checkstack(L, 3, "out of stack space");
    lua_newtable(L);
    for (i = 1; callbacknames[i]; i++) {
        lua_pushstring(L, callbacknames[i]);
        if (callback_defined(i)) {
            lua_pushboolean(L, 1);
        } else {
            lua_pushboolean(L, 0);
        }
        lua_rawset(L, -3);
    }
    return 1;
}

static const struct luaL_Reg callbacklib[] = {
    {"find", callback_find},
    {"register", callback_register},
    {"list", callback_listf},
    {NULL, NULL}                /* sentinel */
};

int luaopen_callback(lua_State * L)
{
    luaL_register(L, "callback", callbacklib);
    luaL_checkstack(L, 1, "out of stack space");
    lua_newtable(L);
    callback_callbacks_id = luaL_ref(L, LUA_REGISTRYINDEX);
    return 1;
}
