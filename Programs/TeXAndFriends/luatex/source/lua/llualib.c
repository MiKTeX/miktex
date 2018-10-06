/* llualib.c

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

#define LOAD_BUF_SIZE 64*1024
#define UINT_MAX32 0xFFFFFFFF

typedef struct {
    unsigned char *buf;
    int size;
 /* int done; */
    int alloc;
} bytecode;

static bytecode *lua_bytecode_registers = NULL;

int luabytecode_max = -1;
unsigned int luabytecode_bytes = 0;

char *luanames[65536] = { NULL };

char *get_lua_name(int i)
{
    if (i < 0 || i > 65535)
        return NULL;
    return luanames[i];
}

void dump_luac_registers(void)
{
    int x;
    int k, n;
    bytecode b;
    dump_int(luabytecode_max);
    if (lua_bytecode_registers != NULL) {
        n = 0;
        for (k = 0; k <= luabytecode_max; k++) {
            if (lua_bytecode_registers[k].size != 0)
                n++;
        }
        dump_int(n);
        for (k = 0; k <= luabytecode_max; k++) {
            b = lua_bytecode_registers[k];
            if (b.size != 0) {
                dump_int(k);
                dump_int(b.size);
                do_zdump((char *) b.buf, 1, (b.size), DUMP_FILE);
            }
        }
    }
    for (k = 0; k < 65536; k++) {
        char *a = luanames[k];
        if (a != NULL) {
            x = (int) strlen(a) + 1;
            dump_int(x);
            dump_things(*a, x);
        } else {
            x = 0;
            dump_int(x);
        }
    }
}

void undump_luac_registers(void)
{
    int x;
    int k, n;
    unsigned int i;
    bytecode b;
    undump_int(luabytecode_max);
    if (luabytecode_max >= 0) {
        i = (unsigned) (luabytecode_max + 1);
        if ((int) (UINT_MAX32 / (int) sizeof(bytecode) + 1) <= i) {
            fatal_error("Corrupt format file");
        }
        lua_bytecode_registers = xmalloc((unsigned) (i * sizeof(bytecode)));
        luabytecode_bytes = (unsigned) (i * sizeof(bytecode));
        for (i = 0; i <= (unsigned) luabytecode_max; i++) {
         /* lua_bytecode_registers[i].done = 0; */
            lua_bytecode_registers[i].size = 0;
            lua_bytecode_registers[i].buf = NULL;
        }
        undump_int(n);
        for (i = 0; i < (unsigned) n; i++) {
            undump_int(k);
            undump_int(x);
            b.size = x;
            b.buf = xmalloc((unsigned) b.size);
            luabytecode_bytes += (unsigned) b.size;
            memset(b.buf, 0, (size_t) b.size);
            do_zundump((char *) b.buf, 1, b.size, DUMP_FILE);
            lua_bytecode_registers[k].size = b.size;
            lua_bytecode_registers[k].alloc = b.size;
            lua_bytecode_registers[k].buf = b.buf;
        }
    }
    for (k = 0; k < 65536; k++) {
        undump_int(x);
        if (x > 0) {
            char *s = xmalloc((unsigned) x);
            undump_things(*s, x);
            luanames[k] = s;
        }
    }
}

static void bytecode_register_shadow_set(lua_State * L, int k)
{
    /* the stack holds the value to be set */
    lua_get_metatablelua(lua_bytecodes_indirect);
    if (lua_istable(L, -1)) {
        lua_pushvalue(L, -2);
        lua_rawseti(L, -2, k);
    }
    lua_pop(L, 1);              /* pop table or nil */
    lua_pop(L, 1);              /* pop value */
}


static int bytecode_register_shadow_get(lua_State * L, int k)
{
    /* the stack holds the value to be set */
    int ret = 0;
    lua_get_metatablelua(lua_bytecodes_indirect);
    if (lua_istable(L, -1)) {
        lua_rawgeti(L, -1, k);
        if (!lua_isnil(L, -1))
            ret = 1;
        lua_insert(L, -3);      /* store the value or nil, deeper down  */
        lua_pop(L, 1);          /* pop the value or nil at top */
    }
    lua_pop(L, 1);              /* pop table or nil */
    return ret;
}


static int writer(lua_State * L, const void *b, size_t size, void *B)
{
    bytecode *buf = (bytecode *) B;
    (void) L;                   /* for -Wunused */
    if ((int) (buf->size + (int) size) > buf->alloc) {
        buf->buf = xrealloc(buf->buf, (unsigned) (buf->alloc + (int) size + LOAD_BUF_SIZE));
        buf->alloc = buf->alloc + (int) size + LOAD_BUF_SIZE;
    }
    memcpy(buf->buf + buf->size, b, size);
    buf->size += (int) size;
    luabytecode_bytes += (unsigned) size;
    return 0;
}

static const char *reader(lua_State * L, void *ud, size_t * size)
{
    bytecode *buf = (bytecode *) ud;
    (void) L; /* for -Wunused */
    /*
    if (buf->done == buf->size) {
        *size = 0;
        buf->done = 0;
        return NULL;
    }
    */
    *size = (size_t) buf->size;
    /*
    buf->done = buf->size;
    */
    return (const char *) buf->buf;
}

static int get_bytecode(lua_State * L)
{
    int k;
    k = (int) luaL_checkinteger(L, -1);
    if (k < 0) {
        lua_pushnil(L);
    } else if (!bytecode_register_shadow_get(L, k)) {
        if (k <= luabytecode_max && lua_bytecode_registers[k].buf != NULL) {
            if (lua_load
                (L, reader, (void *) (lua_bytecode_registers + k),
#ifdef LuajitTeX
                 "bytecode")) {
#else
                 "bytecode", NULL)) {
#endif
                return luaL_error(L, "bad bytecode register");
            } else {
                lua_pushvalue(L, -1);
                bytecode_register_shadow_set(L, k);
            }
        } else {
            lua_pushnil(L);
        }
    }
    return 1;
}

void luabytecodecall(int slot)
{
    int i;
    int stacktop = lua_gettop(Luas);
    lua_active++;
    if (slot < 0 || slot > luabytecode_max) {
        luaL_error(Luas, "bytecode register out of range");
    } else if (bytecode_register_shadow_get(Luas, slot) || lua_bytecode_registers[slot].buf == NULL) {
        luaL_error(Luas, "undefined bytecode register");
    } else if (lua_load(Luas, reader, (void *) (lua_bytecode_registers + slot),
#ifdef LuajitTeX
             "bytecode"))
#else
             "bytecode", NULL))
#endif
    {
        luaL_error(Luas, "bytecode register doesn't load well");
    } else {
        int base = lua_gettop(Luas); /* function index */
        lua_pushinteger(Luas, slot);
        lua_pushcfunction(Luas, lua_traceback); /* push traceback function */
        lua_insert(Luas, base); /* put it under chunk  */
++function_callback_count; /* this will be a dedicated counter */
        i = lua_pcall(Luas, 1, 0, base);
        lua_remove(Luas, base); /* remove traceback function */
        if (i != 0) {
            lua_gc(Luas, LUA_GCCOLLECT, 0);
            Luas = luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
        }
    }

    lua_settop(Luas,stacktop);
    lua_active--;
}

static int set_bytecode(lua_State * L)
{
    int k, ltype;
    unsigned int i;
    int strip = 0;
    int top = lua_gettop(L);
    if (lua_type(L,top) == LUA_TBOOLEAN) {
        strip = lua_toboolean(L,top);
        lua_settop(L,top - 1);
    }
    k = (int) luaL_checkinteger(L, -2);
    i = (unsigned) k + 1;
    if ((int) (UINT_MAX32 / sizeof(bytecode) + 1) < i) {
        luaL_error(L, "value too large");
    }
    if (k < 0) {
        luaL_error(L, "negative values not allowed");
    }
    ltype = lua_type(L, -1);
    if (ltype != LUA_TFUNCTION && ltype != LUA_TNIL) {
        luaL_error(L, "unsupported type");
    }
    if (k > luabytecode_max) {
        i = (unsigned) (sizeof(bytecode) * ((unsigned) k + 1));
        lua_bytecode_registers = xrealloc(lua_bytecode_registers, i);
        if (luabytecode_max == -1) {
            luabytecode_bytes +=
                (unsigned) (sizeof(bytecode) * (unsigned) (k + 1));
        } else {
            luabytecode_bytes +=
                (unsigned) (sizeof(bytecode) *
                            (unsigned) (k + 1 - luabytecode_max));
        }
        for (i = (unsigned) (luabytecode_max + 1); i <= (unsigned) k; i++) {
            lua_bytecode_registers[i].buf = NULL;
            lua_bytecode_registers[i].size = 0;
         /* lua_bytecode_registers[i].done = 0; */
        }
        luabytecode_max = k;
    }
    if (lua_bytecode_registers[k].buf != NULL) {
        xfree(lua_bytecode_registers[k].buf);
        luabytecode_bytes -= (unsigned) lua_bytecode_registers[k].size;
        lua_bytecode_registers[k].size = 0;
     /* lua_bytecode_registers[k].done = 0; */
        lua_pushnil(L);
        bytecode_register_shadow_set(L, k);
    }
    if (ltype == LUA_TFUNCTION) {
        lua_bytecode_registers[k].buf = xmalloc(LOAD_BUF_SIZE);
        lua_bytecode_registers[k].alloc = LOAD_BUF_SIZE;
        memset(lua_bytecode_registers[k].buf, 0, LOAD_BUF_SIZE);
#ifdef LuajitTeX
        RESERVED_lua_dump(L, writer, (void *) (lua_bytecode_registers + k),strip);
        /*lua_dump(L, writer, (void *) (lua_bytecode_registers + k));*/
#else
#if LUA_VERSION_NUM == 503
        lua_dump(L, writer, (void *) (lua_bytecode_registers + k),strip);
#endif
#if LUA_VERSION_NUM == 502
        lua_dump(L, writer, (void *) (lua_bytecode_registers + k));
#endif
#endif
    }
    lua_pop(L, 1);
    return 0;
}


static int set_luaname(lua_State * L)
{
    int k;
    const char *s;
    if (lua_gettop(L) == 3) {
        k = (int) luaL_checkinteger(L, 2);
        if (k > 65535 || k < 0) {
            /* error */
        } else {
            if (luanames[k] != NULL) {
                free(luanames[k]);
                luanames[k] = NULL;
            }
            if (lua_type(L,3) == LUA_TSTRING) {
                s = lua_tostring(L, 3);
                if (s != NULL)
                    luanames[k] = xstrdup(s);
            }
        }
    }
    return 0;
}

static int get_luaname(lua_State * L)
{
    int k = (int) luaL_checkinteger(L, 2);
    if (k > 65535 || k < 0) {
        /* error */
        lua_pushnil(L);
    } else {
        if (luanames[k] != NULL)
            lua_pushstring(L, luanames[k]);
        else
            lua_pushnil(L);
    }
    return 1;
}

static int lua_functions_get_table(lua_State * L) /* hh */
{
    lua_get_metatablelua(lua_functions);
    return 1;
}

static int new_table(lua_State * L) /* hh */
{
    int i = (int) luaL_checkinteger(L, 1);
    int h = (int) luaL_checkinteger(L, 2);
    lua_createtable(L,i,h);
    return 1;
}

static int get_stack_top(lua_State * L) /* hh */
{
    lua_pushinteger(L, lua_gettop(L));
    return 1;
}

static int get_call_level(lua_State * L) /* hh */
{
    lua_pushinteger(L, lua_active);
    return 1;
}

static const struct luaL_Reg lualib[] = {
    /* *INDENT-OFF* */
    {"getluaname",  get_luaname},
    {"setluaname",  set_luaname},
    {"getbytecode", get_bytecode},
    {"setbytecode", set_bytecode},
    {"newtable",    new_table},
    {"get_functions_table",lua_functions_get_table},
    {"getstacktop",get_stack_top},
    {"getcalllevel", get_call_level},
    /* *INDENT-ON* */
    {NULL, NULL}                /* sentinel */
};

int luaopen_lua(lua_State * L, char *fname)
{
    luaL_openlib(L, "lua", lualib, 0);
    make_table(L, "bytecode", "tex.bytecode", "getbytecode", "setbytecode");
    make_table(L, "name",     "tex.name", "getluaname", "setluaname");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "lua.bytecodes.indirect");
    lua_pushstring(L, LUA_VERSION);
    lua_setfield(L, -2, "version");
    if (fname == NULL) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, fname);
    }
    lua_setfield(L, -2, "startupfile");
    return 1;
}
