/* llfslibext.c
   
   Copyright 2010-2011 Taco Hoekwater <taco@luatex.org>

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

#include <kpathsea/c-stat.h>
#include <kpathsea/c-dir.h>
#include <time.h>


#ifdef _WIN32
#  include <windows.h>
#else
#endif

#ifdef _WIN32

static int get_short_name(lua_State * L)
{
    long length = 0;
    TCHAR *buffer = NULL;
    const char *lpszPath = luaL_checkstring(L, 1);
    length = GetShortPathName(lpszPath, NULL, 0);
    if (length == 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "operating system error: %d", (int) GetLastError());
        return 2;
    }
    buffer = (TCHAR *) xmalloc(length * sizeof(TCHAR));
    length = GetShortPathName(lpszPath, buffer, length);
    if (length == 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "operating system error: %d", (int) GetLastError());
        return 2;
    }
    lua_pushlstring(L, (const char *) buffer, (size_t) length);
    return 1;
}

static int read_link(lua_State * L)
{
    lua_pushboolean(L, 0);
    lua_pushliteral(L, "readlink not supported on this platform");
    return 2;
}

#else

static int pusherror(lua_State * L, const char *info)
{
    lua_pushnil(L);
    if (info == NULL)
        lua_pushstring(L, strerror(errno));
    else
        lua_pushfstring(L, "%s: %s", info, strerror(errno));
    lua_pushinteger(L, errno);
    return 3;
}

static int Preadlink(lua_State * L)
{
/** readlink(path) */
    const char *path = luaL_checkstring(L, 1);
    char *b = NULL;
    int allocated = 128;
    int n;
    while (1) {
        b = malloc(allocated);
        if (!b)
            return pusherror(L, path);
        n = readlink(path, b, allocated);
        if (n == -1) {
            free(b);
            return pusherror(L, path);
        }
        if (n < allocated)
            break;
        /* Not enough room, try bigger */
        allocated *= 2;
        free(b);
    }
    lua_pushlstring(L, b, n);
    free(b);
    return 1;
}


static int read_link(lua_State * L)
{
    return Preadlink(L);
}

static int get_short_name(lua_State * L __attribute__ ((unused)))
{
    /* simply do nothing */
    return 1;
}
#endif


/*
** Get file information
*/
static int file_is_directory(lua_State * L)
{
    struct stat info;
    const char *file = luaL_checkstring(L, 1);

    if (stat(file, &info)) {
        lua_pushnil(L);
        lua_pushfstring(L, "cannot obtain information from file `%s'", file);
        return 2;
    }
    if (S_ISDIR(info.st_mode))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}

static int file_is_file(lua_State * L)
{
    struct stat info;
    const char *file = luaL_checkstring(L, 1);

    if (stat(file, &info)) {
        lua_pushnil(L);
        lua_pushfstring(L, "cannot obtain information from file `%s'", file);
        return 2;
    }
    if (S_ISREG(info.st_mode))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}


void open_lfslibext(lua_State * L)
{

    lua_getglobal(L, "lfs");
    lua_pushcfunction(L, file_is_directory);
    lua_setfield(L, -2, "isdir");
    lua_pushcfunction(L, file_is_file);
    lua_setfield(L, -2, "isfile");
    lua_pushcfunction(L, read_link);
    lua_setfield(L, -2, "readlink");
    lua_pushcfunction(L, get_short_name);
    lua_setfield(L, -2, "shortname");
    lua_pop(L, 1);              /* pop the table */
}
