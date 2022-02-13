// harfbuzz.Variation
#include "luaharfbuzz.h"

static int variation_new(lua_State *L) {
  Variation v;
  const char *variation = luaL_checkstring(L, 1);

  if (hb_variation_from_string(variation, -1, &v)) {
    Variation *vp = (Variation *)lua_newuserdata(L, sizeof(*vp));
    luaL_getmetatable(L, "harfbuzz.Variation");
    lua_setmetatable(L, -2);
    *vp = v;
  } else {
    lua_pushnil(L);
  }

  return 1;
}

static int variation_to_string(lua_State *L) {
  Variation* v = (Variation *)luaL_checkudata(L, 1, "harfbuzz.Variation");
  char variation[128];

  hb_variation_to_string(v, variation, 128);
  lua_pushstring(L, variation);
  return 1;
}

static const char *variation_tag_ptr;
static const char *variation_value_ptr;

static int variation_index(lua_State *L) {
  Variation* v = (Variation *)luaL_checkudata(L, 1, "harfbuzz.Variation");
  const char *key = lua_tostring(L, 2);

  if (key == variation_tag_ptr) {
    Tag *tag = (Tag *)lua_newuserdata(L, sizeof(*tag));
    luaL_getmetatable(L, "harfbuzz.Tag");
    lua_setmetatable(L, -2);
    *tag = v->tag;
  } else if (key == variation_value_ptr) {
    lua_pushnumber(L, v->value);
  } else {
    lua_pushnil(L);
  }
  return 1;
}

static int variation_newindex(lua_State *L) {
  Variation* v = (Variation *)luaL_checkudata(L, 1, "harfbuzz.Variation");
  const char *key = lua_tostring(L, 2);

  if (key == variation_tag_ptr) {
    v->tag = *(Tag *)luaL_checkudata(L, 3, "harfbuzz.Tag");
  } else if (key == variation_value_ptr) {
    v->value = luaL_checknumber(L, 3);
  }
  return 0;
}

static const struct luaL_Reg variation_methods[] = {
  { "__index", variation_index },
  { "__newindex", variation_newindex },
  { "__tostring", variation_to_string },
  { NULL, NULL },
};

static const struct luaL_Reg variation_functions[] = {
  { "new", variation_new },
  { NULL,  NULL }
};

int register_variation(lua_State *L) {
  lua_pushliteral(L, "tag");
  variation_tag_ptr = lua_tostring(L, -1);
  (void) luaL_ref (L, LUA_REGISTRYINDEX);
  lua_pushliteral(L, "value");
  variation_value_ptr = lua_tostring(L, -1);
  (void) luaL_ref (L, LUA_REGISTRYINDEX);

  return register_class(L, "harfbuzz.Variation", variation_methods, variation_functions, NULL);
}
