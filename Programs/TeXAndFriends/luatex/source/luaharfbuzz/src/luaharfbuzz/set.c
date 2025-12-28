#include "luaharfbuzz.h"

// implement hb_set_add for unicode
static int subset_set_add(lua_State *L) {
  Set *set = (Set *)luaL_checkudata(L, 1, "harfbuzz.Set");
  unsigned int codepoint = (unsigned int)luaL_checkinteger(L, 2);
  hb_set_add(*set, codepoint);
  return 0;
}

static const struct luaL_Reg set_methods[] = {
    {"add", subset_set_add},
    {NULL, NULL}
};

// register the set method add
static const luaL_Reg set_functions[] = {
    {NULL, NULL}
};

// register the namespace "harfbuzz.Set" and associate ith with the set_methods
int register_set(lua_State *L) {
  return register_class(L, "harfbuzz.Set", set_methods, set_functions, NULL);
}
