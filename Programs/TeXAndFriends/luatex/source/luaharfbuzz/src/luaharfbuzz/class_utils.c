// Utility functions to create Lua classes.
#include "luaharfbuzz.h"

int register_class(lua_State *L, const char *name, const luaL_Reg *methods, const luaL_Reg *functions, const luahb_constant_t *constants) {
  luaL_newmetatable(L, name);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  if (constants) {
    while (constants->name) {
      lua_pushinteger(L, constants->value);
      lua_setfield(L, -2, constants->name);
      constants++;
    }
  }
#ifdef LuajitTeX
  luaL_register(L,NULL,methods);
#else
  luaL_setfuncs(L, methods, 0);
#endif
  lua_pop(L, 1);

  lua_newtable(L);
#ifdef LuajitTeX
  luaL_register(L,NULL,functions);
#else
  luaL_setfuncs(L, functions, 0);
#endif
  luaL_getmetatable(L, name);
  lua_setmetatable(L, -2);
  return 1;
}

