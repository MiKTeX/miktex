/*
** lauxlib_bridge.h, 
** Auxiliary functions bridge from luatex lua5.2 luajit 2.0
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_bridge_h
#define lauxlib_bridge_h

#include "lauxlib.h"



/* compatibility with luatex lua5.2  */
/*
#define LUA_OK  0
#define LUA_OPEQ 0
#define LUA_OPLT 1
#define LUA_OPLE 2
*/

/* http://lua.2524044.n2.nabble.com/converting-modules-in-written-in-C-for-Lua-5-1-to-Lua-5-2-td7642941.html */
#define luaL_setmetatable(L, tname) luaL_getmetatable(L, tname);lua_setmetatable(L, -2);

/* see http://comments.gmane.org/gmane.comp.programming.swig/18673 */
/*
# define lua_rawlen lua_objlen 
*/


LUALIB_API void *(luaL_testudata) (lua_State *L, int ud, const char *tname);
LUALIB_API void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup); 
LUALIB_API char *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) ;
LUA_API int lua_compare (lua_State *L, int index1, int index2, int op);

#if !defined(lua_lock)
#define lua_lock(L)     ((void) 0)
#define lua_unlock(L)   ((void) 0)
#endif

#define luaL_newlibtable(L,l)   \
  lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)

#define luaL_newlib(L,l)        (luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

/*  Is this ok as replacement for lua_copy of lua 5.2.4 ? */
/*
#if defined(LUAJIT)
LUA_API void lua_copy (lua_State *L, int fromidx, int toidx) {
  TValue *fr, *to;
  lua_lock(L);
  fr = index2adr(L, fromidx);
  to = index2adr(L, toidx);
  api_checkvalidindex(L, to);
  copyTV(L,to,fr);
  lua_unlock(L);
}
#endif
*/
#endif
