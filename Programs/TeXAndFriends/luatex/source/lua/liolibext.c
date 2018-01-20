/* liolibext.c

   Copyright 2014 Taco Hoekwater <taco@luatex.org>

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

#ifdef LuajitTeX
#include "lua/lauxlib_bridge.h"
#else
#include "lauxlib.h"
#endif
#include "lualib.h"

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif
#ifdef LuajitTeX
/* luajit has its own way for io, which is a mix of    */
/* lua 5.1 and lua 5.2 . We use the stock luajit.      */
#else
/*
** {======================================================
** lua_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#if defined(_WIN32) && !defined(MIKTEX)

#ifdef _MSC_VER
#define lua_popen(L,c,m)                ((void)L, win32_popen(c,m))
#define lua_pclose(L,file)              ((void)L, win32_pclose(file))
#else
#define lua_popen(L,c,m)                ((void)L, _popen(c,m))
#define lua_pclose(L,file)              ((void)L, _pclose(file))
#endif

#else

#define lua_popen(L,c,m)        ((void)L, fflush(NULL), popen(c,m))
#define lua_pclose(L,file)      ((void)L, pclose(file))

#endif

/* }====================================================== */

#if defined(LUA_USE_POSIX)

#define l_fseek(f,o,w)          fseeko(f,o,w)
#define l_ftell(f)              ftello(f)
#define l_seeknum               off_t

#elif defined(LUA_WIN) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)
/* Windows (but not DDK) and Visual C++ 2005 or higher */

#define l_fseek(f,o,w)          _fseeki64(f,o,w)
#define l_ftell(f)              _ftelli64(f)
#define l_seeknum               __int64

#elif defined(__MINGW32__)

#define l_fseek(f,o,w)          fseeko64(f,o,w)
#define l_ftell(f)              ftello64(f)
#define l_seeknum               int64_t

#else

#define l_fseek(f,o,w)          fseek(f,o,w)
#define l_ftell(f)              ftell(f)
#define l_seeknum               long

#endif

#endif/* #ifdef LuajitTeX */

static FILE *tofile (lua_State *L) {
#ifdef LuajitTeX
    FILE **f = luaL_checkudata(L,1,LUA_FILEHANDLE);
    if (*f == NULL)
        luaL_error(L,"attempt to use a closed file");
    return *f;
#else
    luaL_Stream *p = ((luaL_Stream *)luaL_checkudata(L, 1, LUA_FILEHANDLE));
    if ((p)->closef == NULL)
        luaL_error(L, "attempt to use a closed file");
    lua_assert(p->f);
    return p->f;
#endif
}

#define uchar(c) ((unsigned char)(c))

/*
    HH: A few helpers to avoid reading numbers as strings. For now we put them in their
    own namespace. We also have a few helpers that can make io functions tex friendly.
*/

static int readcardinal1(lua_State *L) {
    FILE *f = tofile(L);
    int a = getc(f);
    if (a == EOF)
        lua_pushnil(L);
    else
        lua_pushinteger(L, a);
    return 1;
}

static int readcardinal1_s(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t p = luaL_checkinteger(L, 2) - 1;
    if (p < 0 || p >= l) {
        lua_pushnil(L);
    } else {
        int a = uchar(s[p]);
        lua_pushinteger(L, a);
    }
    return 1;
}

static int readcardinal2(lua_State *L) {
    FILE *f = tofile(L);
    int a = getc(f);
    int b = getc(f);
    if (b == EOF)
        lua_pushnil(L);
    else
        /* (a<<8) | b */
        lua_pushinteger(L, 0x100 * a + b);
    return 1;
}

static int readcardinal2_s(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t p = luaL_checkinteger(L, 2) - 1;
    if (p < 0 || p+1 >= l) {
        lua_pushnil(L);
    } else {
        int a = uchar(s[p++]);
        int b = uchar(s[p]);
        lua_pushinteger(L, 0x100 * a + b);
    }
    return 1;
}

static int readcardinal3(lua_State *L) {
    FILE *f = tofile(L);
    int a = getc(f);
    int b = getc(f);
    int c = getc(f);
    if (c == EOF)
        lua_pushnil(L);
    else
        /* (a<<16) | (b<<8) | c */
        lua_pushinteger(L, 0x10000 * a + 0x100 * b + c);
    return 1;
}

static int readcardinal3_s(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t p = luaL_checkinteger(L, 2) - 1;
    if (p < 0 || p+2 >= l) {
        lua_pushnil(L);
    } else {
        int a = uchar(s[p++]);
        int b = uchar(s[p++]);
        int c = uchar(s[p]);
        lua_pushinteger(L, 0x10000 * a + 0x100 * b + c);
    }
    return 1;
}

static int readcardinal4(lua_State *L) {
    FILE *f = tofile(L);
    int a = getc(f);
    int b = getc(f);
    int c = getc(f);
    int d = getc(f);
    if (d == EOF)
        lua_pushnil(L);
    else
        /* (a<<24) | (b<<16) | (c<<8) | d */
        lua_pushinteger(L,0x1000000 * a + 0x10000 * b + 0x100 * c + d);
    return 1;
}

static int readcardinal4_s(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t p = luaL_checkinteger(L, 2) - 1;
    if (p < 0 || p+3 >= l) {
        lua_pushnil(L);
    } else {
        int a = uchar(s[p++]);
        int b = uchar(s[p++]);
        int c = uchar(s[p++]);
        int d = uchar(s[p]);
        lua_pushinteger(L,0x1000000 * a + 0x10000 * b + 0x100 * c + d);
    }
    return 1;
}

static int readinteger1(lua_State *L) {
    FILE *f = tofile(L);
    int a = getc(f);
    if (a == EOF)
        lua_pushnil(L);
    else if (a >= 0x80)
        lua_pushinteger(L, a - 0x100);
    else
        lua_pushinteger(L, a);
    return 1;
}

static int readinteger1_s(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t p = luaL_checkinteger(L, 2) - 1;
    if (p < 0 || p >= l) {
        lua_pushnil(L);
    } else {
        int a = uchar(s[p]);
        if (a >= 0x80)
            lua_pushinteger(L, a - 0x100);
        else
            lua_pushinteger(L, a);
    }
    return 1;
}

static int readinteger2(lua_State *L) {
    FILE *f = tofile(L);
    int a = getc(f);
    int b = getc(f);
    if (b == EOF)
        lua_pushnil(L);
    else if (a >= 0x80)
        lua_pushinteger(L, 0x100 * a + b - 0x10000);
    else
        lua_pushinteger(L, 0x100 * a + b);
    return 1;
}

static int readinteger2_s(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t p = luaL_checkinteger(L, 2) - 1;
    if (p < 0 || p+1 >= l) {
        lua_pushnil(L);
    } else {
        int a = uchar(s[p++]);
        int b = uchar(s[p]);
        if (a >= 0x80)
            lua_pushinteger(L, 0x100 * a + b - 0x10000);
        else
            lua_pushinteger(L, 0x100 * a + b);
    }
    return 1;
}

static int readinteger3(lua_State *L) {
    FILE *f = tofile(L);
    int a = getc(f);
    int b = getc(f);
    int c = getc(f);
    if (c == EOF)
        lua_pushnil(L);
    else if (a >= 0x80)
        lua_pushinteger(L, 0x10000 * a + 0x100 * b + c - 0x1000000);
    else
        lua_pushinteger(L, 0x10000 * a + 0x100 * b + c);
    return 1;
}

static int readinteger3_s(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t p = luaL_checkinteger(L, 2) - 1;
    if (p < 0 || p+2 >= l) {
        lua_pushnil(L);
    } else {
        int a = uchar(s[p++]);
        int b = uchar(s[p++]);
        int c = uchar(s[p]);
        if (a >= 0x80)
            lua_pushinteger(L, 0x10000 * a + 0x100 * b + c - 0x1000000);
        else
            lua_pushinteger(L, 0x10000 * a + 0x100 * b + c);
    }
    return 1;
}

static int readinteger4(lua_State *L) {
    FILE *f = tofile(L);
    int a = getc(f);
    int b = getc(f);
    int c = getc(f);
    int d = getc(f);
    if (d == EOF)
        lua_pushnil(L);
    else if (a >= 0x80)
        lua_pushinteger(L, 0x1000000 * a + 0x10000 * b + 0x100 * c + d - 0x100000000);
    else
        lua_pushinteger(L, 0x1000000 * a + 0x10000 * b + 0x100 * c + d);
    return 1;
}

static int readinteger4_s(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t p = luaL_checkinteger(L, 2) - 1;
    if (p < 0 || p+3 >= l) {
        lua_pushnil(L);
    } else {
        int a = uchar(s[p++]);
        int b = uchar(s[p++]);
        int c = uchar(s[p++]);
        int d = uchar(s[p]);
        if (a >= 0x80)
            lua_pushinteger(L, 0x1000000 * a + 0x10000 * b + 0x100 * c + d - 0x100000000);
        else
            lua_pushinteger(L, 0x1000000 * a + 0x10000 * b + 0x100 * c + d);
    }
    return 1;
}

static int readfixed2(lua_State *L) {
    FILE *f = tofile(L);
    int a = getc(f);
    int b = getc(f);
    if (b == EOF)
        lua_pushnil(L);
    else if (a >= 0x80)
        lua_pushinteger(L, (a - 0x100) + b/0x100);
    else
        lua_pushinteger(L, (a        ) + b/0x100);
    return 1;
}

static int readfixed2_s(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t p = luaL_checkinteger(L, 2) - 1;
    if (p < 0 || p+3 >= l) {
        lua_pushnil(L);
    } else {
        int a = uchar(s[p++]);
        int b = uchar(s[p]);
        if (a >= 0x80)
            lua_pushinteger(L, (a - 0x100) + b/0x100);
        else
            lua_pushinteger(L, (a        ) + b/0x100);
    }
    return 1;
}

static int readfixed4(lua_State *L) {
    FILE *f = tofile(L);
    int a = getc(f);
    int b = getc(f);
    int c = getc(f);
    int d = getc(f);
    if (d == EOF)
        lua_pushnil(L);
    else if (a >= 0x80)
        lua_pushnumber(L, (0x100 * a + b - 0x10000) + (0x100 * c + d)/0x10000);
    else
        lua_pushnumber(L, (0x100 * a + b          ) + (0x100 * c + d)/0x10000);
    /* from ff */
    /* int n = 0x1000000 * a + 0x10000 * b + 0x100 * c + d; */
    /* lua_pushnumber(L,(real) (n>>16) + ((n&0xffff)/65536.0)); */
    return 1;
}

static int readfixed4_s(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t p = luaL_checkinteger(L, 2) - 1;
    if (p < 0 || p+3 >= l) {
        lua_pushnil(L);
    } else {
        int a = uchar(s[p++]);
        int b = uchar(s[p++]);
        int c = uchar(s[p++]);
        int d = uchar(s[p]);
        if (a >= 0x80)
            lua_pushnumber(L, (0x100 * a + b - 0x10000) + (0x100 * c + d)/0x10000);
        else
            lua_pushnumber(L, (0x100 * a + b          ) + (0x100 * c + d)/0x10000);
    }
    return 1;
}

static int read2dot14(lua_State *L) {
    FILE *f = tofile(L);
    int a = getc(f);
    int b = getc(f);
    if (b == EOF) {
        lua_pushnil(L);
    } else {
        int n = 0x100 * a + b;
        /* from ff */
        lua_pushnumber(L,(real) ((n<<16)>>(16+14)) + ((n&0x3fff)/16384.0));
    }
    return 1;
}

static int read2dot14_s(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t p = luaL_checkinteger(L, 2) - 1;
    if (p < 0 || p+1 >= l) {
        lua_pushnil(L);
    } else {
        int a = uchar(s[p++]);
        int b = uchar(s[p]);
        int n = 0x100 * a + b;
        lua_pushnumber(L,(real) ((n<<16)>>(16+14)) + ((n&0x3fff)/16384.0));
    }
    return 1;
}

static int getposition(lua_State *L) {
    FILE *f = tofile(L);
    long p = ftell(f);
    if (p<0)
        lua_pushnil(L);
    else
        lua_pushinteger(L, p);
    return 1;
}

static int setposition(lua_State *L) {
    FILE *f = tofile(L);
    long p = lua_tointeger(L,2);
    p = fseek(f,p,SEEK_SET);
    if (p<0)
        lua_pushnil(L);
    else
        lua_pushinteger(L, p);
    return 1;
}

static int skipposition(lua_State *L) {
    FILE *f = tofile(L);
    long p = lua_tointeger(L,2);
    p = fseek(f,ftell(f)+p,SEEK_SET);
    if (p<0)
        lua_pushnil(L);
    else
        lua_pushinteger(L, p);
    return 1;
}

static int readbytetable(lua_State *L) {
    FILE *f = tofile(L);
    int n = lua_tointeger(L,2);
    int i ;
    lua_createtable(L, n, 0);
    for (i=1;i<=n;i++) {
        int a = getc(f);
        if (a == EOF) {
            break;
        } else {
            /*
                lua_pushinteger(L, i);
                lua_pushinteger(L, a);
                lua_rawset(L, -3);
            */
            lua_pushinteger(L, a);
            lua_rawseti(L,-2,i);
        }
    }
    return 1;
}

static int readbytetable_s(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t p = luaL_checkinteger(L, 2) - 1;
    int n = lua_tointeger(L,3);
    if (p < 0 || p >= l) {
        lua_pushnil(L);
    } else {
        int i ;
        if (p + n >= l) {
            n = l - p ;
        }
        lua_createtable(L, n, 0);
        for (i=1;i<=n;i++) {
            int a = uchar(s[p++]);
            lua_pushinteger(L, a);
            lua_rawseti(L,-2,i);
        }
    }
    return 1;
}

static int readbytes(lua_State *L) {
    FILE *f = tofile(L);
    int n = lua_tointeger(L,2);
    int i = 0;
    for (i=1;i<=n;i++) {
        int a = getc(f);
        if (a == EOF) {
            return i-1;
        } else {
            lua_pushinteger(L, a);
        }
    }
    return n;
}

static int readbytes_s(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    size_t p = luaL_checkinteger(L, 2) - 1;
    int n = lua_tointeger(L,3);
    if (p < 0 || p >= l) {
        return 0;
    } else {
        int i ;
        if (p + n >= l) {
            n = l - p ;
        }
        lua_createtable(L, n, 0);
        for (i=1;i<=n;i++) {
            int a = uchar(s[p++]);
            lua_pushinteger(L, a);
        }
        return n;
    }
}

static int recordfilename(lua_State *L)
{
    const char *fname = luaL_checkstring(L, 1);
    const char *ftype = lua_tostring(L, 2);
    if (fname != NULL && ftype != NULL) {
        switch (ftype[1]) {
            case 'r':
                recorder_record_input(fname);
                break;
            case 'w':
                recorder_record_output(fname);
                break;
            default:
                /* silently ignore */
                break;
        }
    } else {
        /* silently ignore */
    }
    return 0;
}

static int checkpermission(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);
    if (filename == NULL) {
        lua_pushboolean(L,0);
        lua_pushliteral(L,"no command name given");
    } else if (shellenabledp <= 0) {
        lua_pushboolean(L,0);
        lua_pushliteral(L,"all command execution is disabled");
    } else if (restrictedshell == 0) {
        lua_pushboolean(L,1);
        lua_pushstring(L,filename);
    } else {
        char *safecmd = NULL;
        char *cmdname = NULL;
        switch (shell_cmd_is_allowed(filename, &safecmd, &cmdname)) {
            case 0:
                lua_pushboolean(L,0);
                lua_pushliteral(L, "specific command execution disabled");
                break;
            case 1:
                /* doesn't happen */
                lua_pushboolean(L,1);
                lua_pushstring(L,filename);
                break;
            case 2:
                lua_pushboolean(L,1);
                lua_pushstring(L,safecmd);
                break;
            default:
                /* -1 */
                lua_pushboolean(L,0);
                lua_pushliteral(L, "bad command line quoting");
                break;
        }
    }
    return 2;
}

static int readline(lua_State *L)
{
    luaL_Buffer buf;
    int c, d;
    FILE *f = tofile(L);
    luaL_buffinit(L, &buf);
    while (1) {
        c = fgetc(f);
        if (c == EOF) {
            luaL_pushresult(&buf);
            if (lua_rawlen(L, -1) == 0) {
                lua_pop(L, 1);
                lua_pushnil(L);
            }
            return 1;
        } else if (c == '\n') {
            luaL_pushresult(&buf);
            return 1;
        } else if (c == '\r') {
            d = fgetc(f);
            if (d != EOF && d != '\n') {
                ungetc(d, f);
            }
            luaL_pushresult(&buf);
            return 1;
        } else {
            luaL_addchar(&buf, c);
        }
    }
}

static const luaL_Reg fiolib[] = {
    /* helpers */
    { "readcardinal1",   readcardinal1 },
    { "readcardinal2",   readcardinal2 },
    { "readcardinal3",   readcardinal3 },
    { "readcardinal4",   readcardinal4 },
    { "readinteger1",    readinteger1 },
    { "readinteger2",    readinteger2 },
    { "readinteger3",    readinteger3 },
    { "readinteger4",    readinteger4 },
    { "readfixed2",      readfixed2 },
    { "readfixed4",      readfixed4 },
    { "read2dot14",      read2dot14 },
    { "setposition",     setposition },
    { "getposition",     getposition },
    { "skipposition",    skipposition },
    { "readbytes",       readbytes },
    { "readbytetable",   readbytetable },
    { "readline",        readline },
    /* extras */
    { "recordfilename",  recordfilename },
    { "checkpermission", checkpermission },
    /* done */
    {NULL, NULL}
};

static const luaL_Reg siolib[] = {
    { "readcardinal1",   readcardinal1_s },
    { "readcardinal2",   readcardinal2_s },
    { "readcardinal3",   readcardinal3_s },
    { "readcardinal4",   readcardinal4_s },
    { "readinteger1",    readinteger1_s },
    { "readinteger2",    readinteger2_s },
    { "readinteger3",    readinteger3_s },
    { "readinteger4",    readinteger4_s },
    { "readfixed2",      readfixed2_s },
    { "readfixed4",      readfixed4_s },
    { "read2dot14",      read2dot14_s },
    { "readbytes",       readbytes_s },
    { "readbytetable",   readbytetable_s },
    /* done */
    {NULL, NULL}
};

/*
    The sio helpers are experimental and might be handy at some point. Speed-wise
    there is no gain over file access because with ssd and caching we basically
    operate in memory too. We keep them as complement to the file ones. I did
    consider using an userdata object for the position etc but some simple tests
    demonstrated that there is no real gain and the current ones permits to wrap
    up whatever interface one likes.
*/

int luaopen_fio(lua_State *L) {
    luaL_openlib(L, "fio", fiolib, 0);
    luaL_openlib(L, "sio", siolib, 0);
    return 1;
}
