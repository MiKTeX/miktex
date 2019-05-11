/************************************************************************
* Author    : Tiago Dionizio (tngd@mega.ist.utl.pt)                     *
* Library   : lzlib - Lua 5 interface to access zlib library functions  *
*                                                                       *
* Permission is hereby granted, free of charge, to any person obtaining *
* a copy of this software and associated documentation files (the       *
* "Software"), to deal in the Software without restriction, including   *
* without limitation the rights to use, copy, modify, merge, publish,   *
* distribute, sublicense, and/or sell copies of the Software, and to    *
* permit persons to whom the Software is furnished to do so, subject to *
* the following conditions:                                             *
*                                                                       *
* The above copyright notice and this permission notice shall be        *
* included in all copies or substantial portions of the Software.       *
*                                                                       *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  *
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  *
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     *
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                *
************************************************************************/

/************************************************************************
$Id: lzlib.c,v 1.3 2004/07/22 19:10:47 tngd Exp $
Changes:
************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"

#include "zlib.h"

#if defined(MIKTEX_WINDOWS)
#  include <miktex/unxemu.h>
#endif

/*
** =========================================================================
** compile time options wich determine available functionality
** =========================================================================
*/


/*
** =========================================================================
** zlib stream metamethods
** =========================================================================
*/
#define ZSTREAMMETA "zlib.zstream"

#define LZANY     (void*)-1
#define LZNONE    (void*)0
#define LZDEFLATE (void*)1
#define LZINFLATE (void*)2

static z_stream *lzstream_new(lua_State *L)
{
    z_stream *s = (z_stream*)lua_newuserdata(L, sizeof(z_stream));

    luaL_getmetatable(L, ZSTREAMMETA);
    lua_setmetatable(L, -2);        /* set metatable */

    s->zalloc = Z_NULL;
    s->zfree = Z_NULL;

    s->next_out = Z_NULL;
    s->avail_out = 0;
    s->next_in = Z_NULL;
    s->avail_in = 0;

    s->opaque = LZNONE;

    return s;
}

static void lzstream_cleanup(lua_State *L, z_stream *s)
{
    if (s && s->opaque != LZNONE)
    {
        if (s->opaque == LZINFLATE)
            inflateEnd(s);
        if (s->opaque == LZDEFLATE)
            deflateEnd(s);
        s->opaque = LZNONE;
    }
}

/* ====================================================================== */

static z_stream *lzstream_get(lua_State *L, int index)
{
    z_stream *s = (z_stream*)luaL_checkudata(L, index, ZSTREAMMETA);
    if (s == NULL) luaL_argerror(L, index, "bad zlib stream");
    return s;
}

static z_stream *lzstream_check(lua_State *L, int index, void *state)
{
    z_stream *s = lzstream_get(L, index);
    if ((state != LZANY && s->opaque != state) || s->opaque == LZNONE)
        luaL_argerror(L, index, "attempt to use invalid zlib stream");
    return s;
}

/* ====================================================================== */

static int lzstream_tostring(lua_State *L)
{
    char buf[100];
    z_stream *s = (z_stream*)luaL_checkudata(L, 1, ZSTREAMMETA);
    if (s == NULL) luaL_argerror(L, 1, "bad zlib stream");

    if (s->opaque == LZNONE)
        strncpy(buf, "zlib stream (closed)", 100);
    else if (s->opaque == LZDEFLATE)
        snprintf(buf, 100, "zlib deflate stream (%p)", s);
    else if (s->opaque == LZINFLATE)
        snprintf(buf, 100, "zlib inflate stream (%p)", s);
    else
        snprintf(buf, 100, "%p", s);

    lua_pushstring(L, buf);
    return 1;
}

/* ====================================================================== */

static int lzstream_gc(lua_State *L)
{
    z_stream *s = lzstream_get(L, 1);
    lzstream_cleanup(L, s);
    return 0;
}

/* ====================================================================== */

static int lzstream_close(lua_State *L)
{
    z_stream *s = lzstream_check(L, 1, LZANY);
    lzstream_cleanup(L, s);
    return 0;
}

/* ====================================================================== */

static int lzstream_adler(lua_State *L)
{
    z_stream *s = lzstream_check(L, 1, LZANY);
    lua_pushnumber(L, s->adler);
    return 1;
}

/* ====================================================================== */

static int lzlib_deflate(lua_State *L)
{
    int level = luaL_optint(L, 1, Z_DEFAULT_COMPRESSION);
    int method = luaL_optint(L, 2, Z_DEFLATED);
    int windowBits = luaL_optint(L, 3, 15);
    int memLevel = luaL_optint(L, 4, 8);
    int strategy = luaL_optint(L, 5, Z_DEFAULT_STRATEGY);

    z_stream *s = lzstream_new(L);

    if (deflateInit2(s, level, method, windowBits, memLevel, strategy) != Z_OK)
    {
        lua_pushliteral(L, "failed to start decompressing");
        lua_error(L);
    }
    s->opaque = LZDEFLATE;
    return 1;
}

/* ====================================================================== */

static int lzlib_inflate(lua_State *L)
{
    int windowBits = luaL_optint(L, 1, 15);

    z_stream *s = lzstream_new(L);

    if (inflateInit2(s, windowBits) != Z_OK)
    {
        lua_pushliteral(L, "failed to start compressing");
        lua_error(L);
    }

    s->opaque = LZINFLATE;
    return 1;
}

/* ====================================================================== */

static int lzstream_decompress(lua_State *L)
{
    z_stream *s = lzstream_check(L, 1, LZINFLATE);
    s->next_in = (Bytef*)luaL_checkstring(L, 2);
    s->avail_in = lua_rawlen(L, 2);

    {
        int r;
        luaL_Buffer b;
        luaL_buffinit(L, &b);

        do {
            s->next_out = (Bytef*)luaL_prepbuffer(&b);
            s->avail_out = LUAL_BUFFERSIZE;

            /* munch some more */
            r = inflate(s, Z_SYNC_FLUSH);
            if (r != Z_OK && r != Z_STREAM_END) {
                lua_pushfstring(L, "failed to decompress [%d]", r);
                lua_error(L);
            }

            /* push gathered data */
            luaL_addsize(&b, LUAL_BUFFERSIZE - s->avail_out);

            /* until we have free space in the output buffer - meaning we are done */
        } while (s->avail_in > 0 || s->avail_out == 0);

        /* send gathered data if any */
        luaL_pushresult(&b);
    }
    return 1;
}

/* ====================================================================== */

static int lzstream_compress(lua_State *L)
{
    z_stream *s = lzstream_check(L, 1, LZDEFLATE);
    s->next_in = (Bytef*)luaL_checkstring(L, 2);
    s->avail_in = lua_rawlen(L, 2);

    {
        int r;
        luaL_Buffer b;
        luaL_buffinit(L, &b);

        do {
            s->next_out = (Bytef*)luaL_prepbuffer(&b);
            s->avail_out = LUAL_BUFFERSIZE;

            /* bake some more */
            if ((r = deflate(s, Z_NO_FLUSH)) != Z_OK)
            {
                lua_pushfstring(L, "failed to compress [%d]", r);
                lua_error(L);
            }

            /* push gathered data */
            luaL_addsize(&b, LUAL_BUFFERSIZE - s->avail_out);

            /* until we have free space in the output buffer - meaning we are done */
        } while (s->avail_out == 0);

        /* send gathered data if any */
        luaL_pushresult(&b);
    }
    return 1;
}

/* ====================================================================== */

static int lzstream_flush(lua_State *L)
{
    z_stream *s = lzstream_check(L, 1, LZANY);

    if (s->opaque == LZINFLATE) {
        lua_pushliteral(L, "");
        return 1;
    }

    s->next_in = (Bytef*)"";
    s->avail_in = 0;

    {
        int r = 0;
        luaL_Buffer b;
        luaL_buffinit(L, &b);

        do {
            s->next_out = (Bytef*)luaL_prepbuffer(&b);
            s->avail_out = LUAL_BUFFERSIZE;

            r = deflate(s, Z_FINISH);

            if (r != Z_OK && r != Z_STREAM_END) {
                lua_pushfstring(L, "failed to flush [%d]", r);
                lua_error(L);
            }

            /* push gathered data */
            luaL_addsize(&b, LUAL_BUFFERSIZE - s->avail_out);
        } while (r != Z_STREAM_END);

        /* send gathered data if any */
        luaL_pushresult(&b);
    }
    return 1;
}

/* ====================================================================== */

static int lzstream_reset(lua_State *L)
{
    z_stream *s = lzstream_check(L, 1, LZANY);

    if (s->state == LZDEFLATE)
        lua_pushnumber(L, deflateReset(s));
    else if (s->opaque == LZINFLATE)
        lua_pushnumber(L, inflateReset(s));
    else
    {
        lua_pushliteral(L, "invalid zlib stream state");
        lua_error(L);
    }

    return 1;
}


/*
** =========================================================================
** zlib functions
** =========================================================================
*/

static int lzlib_version(lua_State *L)
{
    lua_pushstring(L, zlibVersion());
    return 1;
}

/* ====================================================================== */
static int lzlib_adler32(lua_State *L)
{
    if (lua_gettop(L) == 0)
    {
        /* adler32 initial value */
        lua_pushnumber(L, adler32(0L, Z_NULL, 0));
    }
    else
    {
        /* update adler32 checksum */
        int adler = luaL_checkint(L, 1);
        const char* buf = luaL_checkstring(L, 2);
        int len = lua_rawlen(L, 2);

        lua_pushnumber(L, adler32(adler, (const Bytef*)buf, len));
    }
    return 1;
}

/* ====================================================================== */
static int lzlib_crc32(lua_State *L)
{
    if (lua_gettop(L) == 0)
    {
        /* crc32 initial value */
        lua_pushnumber(L, crc32(0L, Z_NULL, 0));
    }
    else
    {
        /* update crc32 checksum */
        int crc = luaL_checkint(L, 1);
        const char* buf = luaL_checkstring(L, 2);
        int len = lua_rawlen(L, 2);

        lua_pushnumber(L, crc32(crc, (const Bytef*)buf, len));
    }
    return 1;
}

/* ====================================================================== */


static int lzlib_compress(lua_State *L)
{
    const char *next_in = luaL_checkstring(L, 1);
    int avail_in = lua_rawlen(L, 1);
    int level = luaL_optint(L, 2, Z_DEFAULT_COMPRESSION);
    int method = luaL_optint(L, 3, Z_DEFLATED);
    int windowBits = luaL_optint(L, 4, 15);
    int memLevel = luaL_optint(L, 5, 8);
    int strategy = luaL_optint(L, 6, Z_DEFAULT_STRATEGY);

    int ret;
    z_stream zs;
    luaL_Buffer b;
    luaL_buffinit(L, &b);

    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;

    zs.next_out = Z_NULL;
    zs.avail_out = 0;
    zs.next_in = Z_NULL;
    zs.avail_in = 0;

    ret = deflateInit2(&zs, level, method, windowBits, memLevel, strategy);

    if (ret != Z_OK)
    {
        lua_pushnil(L);
        lua_pushnumber(L, ret);
        return 2;
    }

    zs.next_in = (Bytef*)next_in;
    zs.avail_in = avail_in;

    for(;;)
    {
        zs.next_out = (Bytef*)luaL_prepbuffer(&b);
        zs.avail_out = LUAL_BUFFERSIZE;

        /* munch some more */
        ret = deflate(&zs, Z_FINISH);

        /* push gathered data */
        luaL_addsize(&b, LUAL_BUFFERSIZE - zs.avail_out);

        /* done processing? */
        if (ret == Z_STREAM_END)
            break;

        /* error condition? */
        if (ret != Z_OK)
            break;
    }

    /* cleanup */
    deflateEnd(&zs);

    luaL_pushresult(&b);
    lua_pushnumber(L, ret);
    return 2;
}

/* ====================================================================== */

static int lzlib_decompress(lua_State *L)
{
    const char *next_in = luaL_checkstring(L, 1);
    int avail_in = lua_rawlen(L, 1);
    int windowBits = luaL_optint(L, 2, 15);

    int ret;
    z_stream zs;
    luaL_Buffer b;
    luaL_buffinit(L, &b);


    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;

    zs.next_out = Z_NULL;
    zs.avail_out = 0;
    zs.next_in = Z_NULL;
    zs.avail_in = 0;

    ret = inflateInit2(&zs, windowBits);

    if (ret != Z_OK)
    {
        lua_pushnil(L);
        lua_pushnumber(L, ret);
        return 2;
    }

    zs.next_in = (Bytef*)next_in;
    zs.avail_in = avail_in;

    for(;;)
    {
        zs.next_out = (Bytef*)luaL_prepbuffer(&b);
        zs.avail_out = LUAL_BUFFERSIZE;
        /* bake some more */
        ret = inflate(&zs, Z_NO_FLUSH);
        /* push gathered data */
        luaL_addsize(&b, LUAL_BUFFERSIZE - zs.avail_out);
        /* need dictionary? - no dictionary support here, so just quit */
        if (ret == Z_NEED_DICT)
            break;
        /* done processing? */
        if (ret == Z_STREAM_END)
            break;
        /* error condition? */
        if (ret != Z_OK)
            break;
    }

    /* cleanup */
    inflateEnd(&zs);

    luaL_pushresult(&b);
    lua_pushnumber(L, ret);
    return 2;
}


/*
** =========================================================================
** Register functions
** =========================================================================
*/


#if defined(MIKTEX)
int luaopen_zlib(lua_State *L)
#else
LUALIB_API int luaopen_zlib(lua_State *L)
#endif
{
    const luaL_Reg zstreamm[] =
    {
        {"reset",           lzstream_reset      },

        {"compress",        lzstream_compress   },
        {"decompress",      lzstream_decompress },
        {"flush",           lzstream_flush      },
        {"close",           lzstream_close      },

        {"adler",           lzstream_adler      },

        {"__tostring",      lzstream_tostring   },
        {"__gc",            lzstream_gc         },
        {NULL, NULL}
    };

    const luaL_Reg zlib[] =
    {
        {"version",         lzlib_version       },
        {"adler32",         lzlib_adler32       },
        {"crc32",           lzlib_crc32         },

        {"compressobj",     lzlib_deflate       },
        {"decompressobj",   lzlib_inflate       },

        {"compress",        lzlib_compress      },
        {"decompress",      lzlib_decompress    },

        {NULL, NULL}
    };

    /* ====================================================================== */

    /* make sure header and library version are consistent */
    const char* version = zlibVersion();
#if !defined(MIKTEX_WINDOWS)
    if (strncmp(version, ZLIB_VERSION, 4))
    {
        lua_pushfstring(L, "zlib library version does not match - header: %s, library: %s", ZLIB_VERSION, version);
        lua_error(L);
    }
#endif

    /* create new metatable for zlib compression structures */
    luaL_newmetatable(L, ZSTREAMMETA);
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -2);               /* push metatable */
    lua_rawset(L, -3);                  /* metatable.__index = metatable */

    /*
    ** Stack: metatable
    */
    luaL_openlib(L, NULL, zstreamm, 0);

    lua_pop(L, 1);                      /* remove metatable from stack */

    /*
    ** Stack:
    */
    lua_newtable(L);
    luaL_openlib(L, "zlib", zlib, 1);

    /*
    ** Stack: zlib table
    */
    return 1;
}
