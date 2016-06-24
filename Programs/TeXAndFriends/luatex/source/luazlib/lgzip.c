/************************************************************************
* Author    : Tiago Dionizio (tngd@mega.ist.utl.pt)                     *
* Library   : lgzip - a gzip file access binding for Lua 5              *
*             based on liolib.c from Lua 5.0 library                    *
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
$Id: lgzip.c,v 1.2 2003/12/28 01:26:16 tngd Exp $
Changes:
************************************************************************/
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zlib.h"

#include "lua.h"
#include "lauxlib.h"



/*
** {======================================================
** FILE Operations
** =======================================================
*/


#define FILEHANDLE		"zlib.gzFile"


static int pushresult (lua_State *L, int i, const char *filename) {
  if (i) {
    lua_pushboolean(L, 1);
    return 1;
  }
  else {
    lua_pushnil(L);
    if (filename)
      lua_pushfstring(L, "%s: %s", filename, strerror(errno));
    else
      lua_pushfstring(L, "%s", strerror(errno));
    lua_pushnumber(L, errno);
    return 3;
  }
}


static gzFile *topfile (lua_State *L, int findex) {
  gzFile*f = (gzFile*)luaL_checkudata(L, findex, FILEHANDLE);
  if (f == NULL) luaL_argerror(L, findex, "bad file");
  return f;
}


static gzFile tofile (lua_State *L, int findex) {
  gzFile*f = topfile(L, findex);
  if (*f == NULL)
    luaL_error(L, "attempt to use a closed file");
  return *f;
}



/*
** When creating file handles, always creates a `closed' file handle
** before opening the actual file; so, if there is a memory error, the
** file is not left opened.
*/
static gzFile *newfile (lua_State *L) {
  gzFile *pf = (gzFile*)lua_newuserdata(L, sizeof(gzFile));
  *pf = NULL;  /* file handle is currently `closed' */
  luaL_getmetatable(L, FILEHANDLE);
  lua_setmetatable(L, -2);
  return pf;
}


static int aux_close (lua_State *L) {
  gzFile f = tofile(L, 1);
  int ok = (gzclose(f) == Z_OK);
  if (ok)
    *(gzFile*)lua_touserdata(L, 1) = NULL;  /* mark file as closed */
  return ok;
}


static int io_close (lua_State *L) {
  return pushresult(L, aux_close(L), NULL);
}


static int io_gc (lua_State *L) {
  gzFile *f = topfile(L, 1);
  if (*f != NULL)  /* ignore closed files */
    aux_close(L);
  return 0;
}


static int io_tostring (lua_State *L) {
  char buff[128];
  gzFile *f = topfile(L, 1);
  if (*f == NULL)
    strcpy(buff, "closed");
  else
    sprintf(buff, "%p", lua_touserdata(L, 1));
  lua_pushfstring(L, "gzip file (%s)", buff);
  return 1;
}


static int io_open (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  const char *mode = luaL_optstring(L, 2, "rb");
  gzFile *pf = newfile(L);
  *pf = gzopen(filename, mode);
  return (*pf == NULL) ? pushresult(L, 0, filename) : 1;
}


static int io_readline (lua_State *L);

static void aux_lines (lua_State *L, int idx, int close) {
  lua_pushliteral(L, FILEHANDLE);
  lua_rawget(L, LUA_REGISTRYINDEX);
  lua_pushvalue(L, idx);
  lua_pushboolean(L, close);  /* close/not close file when finished */
  lua_pushcclosure(L, io_readline, 3);
}


static int f_lines (lua_State *L) {
  tofile(L, 1);  /* check that it's a valid file handle */
  aux_lines(L, 1, 0);
  return 1;
}


static int io_lines (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  gzFile *pf = newfile(L);
  *pf = gzopen(filename, "rb");
  luaL_argcheck(L, *pf, 1,  strerror(errno));
  aux_lines(L, lua_gettop(L), 1);
  return 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


static int test_eof (lua_State *L, gzFile f) {
  lua_pushlstring(L, NULL, 0);
  return (gzeof(f) != 1);
}

#if defined(MIKTEX) && defined(read_line)
#  undef read_line
#endif
static int read_line (lua_State *L, gzFile f) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  for (;;) {
    size_t l;
    char *p = luaL_prepbuffer(&b);
    if (gzgets(f, p, LUAL_BUFFERSIZE) == NULL) {  /* eof? */
      luaL_pushresult(&b);  /* close buffer */
      return (lua_rawlen(L, -1) > 0);  /* check whether read something */
    }
    l = strlen(p);
    if (p[l-1] != '\n')
      luaL_addsize(&b, l);
    else {
      luaL_addsize(&b, l - 1);  /* do not include `eol' */
      luaL_pushresult(&b);  /* close buffer */
      return 1;  /* read at least an `eol' */
    }
  }
}


static int read_chars (lua_State *L, gzFile f, size_t n) {
  size_t rlen;  /* how much to read */
  size_t nr;  /* number of chars actually read */
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  rlen = LUAL_BUFFERSIZE;  /* try to read that much each time */
  do {
    char *p = luaL_prepbuffer(&b);
    if (rlen > n) rlen = n;  /* cannot read more than asked */
    nr = gzread(f, p, rlen);
    luaL_addsize(&b, nr);
    n -= nr;  /* still have to read `n' chars */
  } while (n > 0 && nr == rlen);  /* until end of count or eof */
  luaL_pushresult(&b);  /* close buffer */
  return (n == 0 || lua_rawlen(L, -1) > 0);
}


static int g_read (lua_State *L, gzFile f, int first) {
  int nargs = lua_gettop(L) - 1;
  int success;
  int n;
  if (nargs == 0) {  /* no arguments? */
    success = read_line(L, f);
    n = first+1;  /* to return 1 result */
  }
  else {  /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    for (n = first; nargs-- && success; n++) {
      if (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)lua_tonumber(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      else {
        const char *p = lua_tostring(L, n);
        luaL_argcheck(L, p && p[0] == '*', n, "invalid option");
        switch (p[1]) {
          case 'l':  /* line */
            success = read_line(L, f);
            break;
          case 'a':  /* file */
            read_chars(L, f, ~((size_t)0));  /* read MAX_SIZE_T chars */
            success = 1; /* always success */
            break;
          default:
            return luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  if (!success) {
    lua_pop(L, 1);  /* remove last result */
    lua_pushnil(L);  /* push nil instead */
  }
  return n - first;
}


static int f_read (lua_State *L) {
  return g_read(L, tofile(L, 1), 2);
}


static int io_readline (lua_State *L) {
  gzFile f = *(gzFile*)lua_touserdata(L, lua_upvalueindex(2));
  if (f == NULL)  /* file is already closed? */
    luaL_error(L, "file is already closed");
  if (read_line(L, f)) return 1;
  else {  /* EOF */
    if (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);
      lua_pushvalue(L, lua_upvalueindex(2));
      aux_close(L);  /* close it */
    }
    return 0;
  }
}

/* }====================================================== */


static int g_write (lua_State *L, gzFile f, int arg) {
  int nargs = lua_gettop(L) - 1;
  int status = 1;
  for (; nargs--; arg++) {
    if (lua_type(L, arg) == LUA_TNUMBER) {
      /* optimization: could be done exactly as for strings */
      status = status &&
          gzprintf(f, LUA_NUMBER_FMT, lua_tonumber(L, arg)) > 0;
    }
    else {
      size_t l;
      const char *s = luaL_checklstring(L, arg, &l);
      status = status && (gzwrite(f, (char*)s, l) == l);
    }
  }
  return pushresult(L, status, NULL);
}


static int f_write (lua_State *L) {
  return g_write(L, tofile(L, 1), 2);
}


static int f_seek (lua_State *L) {
  static const int mode[] = {SEEK_SET, SEEK_CUR};
  static const char *const modenames[] = {"set", "cur", NULL};
  gzFile f = tofile(L, 1);
  int op = luaL_checkoption(L, 2, "cur", modenames);
  long offset = luaL_optlong(L, 3, 0);
  luaL_argcheck(L, op != -1, 2, "invalid mode");
  op = gzseek(f, offset, mode[op]);
  if (op == -1)
    return pushresult(L, 0, NULL);  /* error */
  else {
    lua_pushnumber(L, op);
    return 1;
  }
}


static int f_flush (lua_State *L) {
  return pushresult(L, gzflush(tofile(L, 1), Z_FINISH) == Z_OK, NULL);
}


static const luaL_Reg iolib[] = {
  {"lines", io_lines},
  {"close", io_close},
  {"open", io_open},
  {NULL, NULL}
};


static const luaL_Reg flib[] = {
  {"flush", f_flush},
  {"read", f_read},
  {"lines", f_lines},
  {"seek", f_seek},
  {"write", f_write},
  {"close", io_close},
  {"__gc", io_gc},
  {"__tostring", io_tostring},
  {NULL, NULL}
};


static void createmeta (lua_State *L) {
  luaL_newmetatable(L, FILEHANDLE);  /* create new metatable for file handles */
  /* file methods */
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -2);  /* push metatable */
  lua_rawset(L, -3);  /* metatable.__index = metatable */
  luaL_openlib(L, NULL, flib, 0);
}


/* }====================================================== */



LUALIB_API int luaopen_gzip (lua_State *L) {
  createmeta(L);
  lua_pushvalue(L, -1);
  luaL_openlib(L, "gzip", iolib, 1);
  return 1;
}

