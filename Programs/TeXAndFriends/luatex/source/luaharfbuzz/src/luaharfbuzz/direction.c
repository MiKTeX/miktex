// harfbuzz.Feature
#include "luaharfbuzz.h"

static int direction_new(lua_State *L) {
  Direction *d;
  const char *dir = luaL_checkstring(L, 1);

  d = (Direction *)lua_newuserdata(L, sizeof(*d));
  luaL_getmetatable(L, "harfbuzz.Direction");
  lua_setmetatable(L, -2);

  *d = hb_direction_from_string(dir, -1);
  return 1;
}

static int direction_to_string(lua_State *L) {
  Direction* d = (Direction *)luaL_checkudata(L, 1, "harfbuzz.Direction");

  lua_pushstring(L, hb_direction_to_string(*d));
  return 1;
}

static int direction_equals(lua_State *L) {
  Direction* lhs = (Direction *)luaL_checkudata(L, 1, "harfbuzz.Direction");
  Direction* rhs = (Direction *)luaL_checkudata(L, 2, "harfbuzz.Direction");

  lua_pushboolean(L, *lhs == *rhs);
  return 1;
}

static int direction_is_valid(lua_State *L) {
  Direction* d = (Direction *)luaL_checkudata(L, 1, "harfbuzz.Direction");

  lua_pushboolean(L, HB_DIRECTION_IS_VALID(*d));
  return 1;
}

static int direction_is_horizontal(lua_State *L) {
  Direction* d = (Direction *)luaL_checkudata(L, 1, "harfbuzz.Direction");

  lua_pushboolean(L, HB_DIRECTION_IS_HORIZONTAL(*d));
  return 1;
}

static int direction_is_vertical(lua_State *L) {
  Direction* d = (Direction *)luaL_checkudata(L, 1, "harfbuzz.Direction");

  lua_pushboolean(L, HB_DIRECTION_IS_VERTICAL(*d));
  return 1;
}

static int direction_is_forward(lua_State *L) {
  Direction* d = (Direction *)luaL_checkudata(L, 1, "harfbuzz.Direction");

  lua_pushboolean(L, HB_DIRECTION_IS_FORWARD(*d));
  return 1;
}

static int direction_is_backward(lua_State *L) {
  Direction* d = (Direction *)luaL_checkudata(L, 1, "harfbuzz.Direction");

  lua_pushboolean(L, HB_DIRECTION_IS_BACKWARD(*d));
  return 1;
}

static const struct luaL_Reg direction_methods[] = {
  { "__tostring", direction_to_string },
  { "__eq", direction_equals },
  { "is_valid", direction_is_valid },
  { "is_horizontal", direction_is_horizontal },
  { "is_vertical", direction_is_vertical },
  { "is_forward", direction_is_forward },
  { "is_backward", direction_is_backward },
  { NULL, NULL }
};

static const struct luaL_Reg direction_functions[] = {
  { "new", direction_new },
  { NULL,  NULL }
};

int register_direction(lua_State *L) {
  return register_class(L, "harfbuzz.Direction", direction_methods, direction_functions, NULL);
}
