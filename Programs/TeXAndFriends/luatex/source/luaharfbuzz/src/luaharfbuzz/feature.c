// harfbuzz.Feature
#include "luaharfbuzz.h"

static int feature_new(lua_State *L) {
  Feature f;
  const char *feature = luaL_checkstring(L, 1);

  if (hb_feature_from_string(feature, -1, &f)) {
    Feature *fp = (Feature *)lua_newuserdata(L, sizeof(*fp));
    luaL_getmetatable(L, "harfbuzz.Feature");
    lua_setmetatable(L, -2);
    *fp = f;
  } else {
    lua_pushnil(L);
  }

  return 1;
}

static int feature_to_string(lua_State *L) {
  Feature* f = (Feature *)luaL_checkudata(L, 1, "harfbuzz.Feature");
  char feature[128];

  hb_feature_to_string(f, feature, 128);
  lua_pushstring(L, feature);
  return 1;
}

static const char *feature_tag_ptr;
static const char *feature_value_ptr;
static const char *feature_start_ptr;
static const char *feature_end_ptr;

static int feature_index(lua_State *L) {
  Feature* f = (Feature *)luaL_checkudata(L, 1, "harfbuzz.Feature");
  const char *key = lua_tostring(L, 2);

  if (key == feature_tag_ptr) {
    Tag *tag = (Tag *)lua_newuserdata(L, sizeof(*tag));
    luaL_getmetatable(L, "harfbuzz.Tag");
    lua_setmetatable(L, -2);
    *tag = f->tag;
  } else if (key == feature_value_ptr) {
    lua_pushinteger(L, f->value);
  } else if (key == feature_start_ptr) {
    if (f->start == HB_FEATURE_GLOBAL_START)
      lua_pushnil(L);
    else
      lua_pushinteger(L, f->start);
  } else if (key == feature_end_ptr) {
    if (f->end == HB_FEATURE_GLOBAL_END)
      lua_pushnil(L);
    else
      lua_pushinteger(L, f->end);
  } else {
    lua_pushnil(L);
  }
  return 1;
}

static int feature_newindex(lua_State *L) {
  Feature* f = (Feature *)luaL_checkudata(L, 1, "harfbuzz.Feature");
  const char *key = lua_tostring(L, 2);

  if (key == feature_tag_ptr) {
    f->tag = *(Tag *)luaL_checkudata(L, 3, "harfbuzz.Tag");
  } else if (key == feature_value_ptr) {
    f->value = luaL_checkinteger(L, 3);
  } else if (key == feature_start_ptr) {
    if (lua_toboolean(L, 3))
      f->start = luaL_checkinteger(L, 3);
    else
      f->start = HB_FEATURE_GLOBAL_START;
  } else if (key == feature_end_ptr) {
    if (lua_toboolean(L, 3))
      f->end = luaL_checkinteger(L, 3);
    else
      f->end = HB_FEATURE_GLOBAL_END;
  }
  return 0;
}

static const struct luaL_Reg feature_methods[] = {
  { "__index", feature_index },
  { "__newindex", feature_newindex },
  { "__tostring", feature_to_string },
  { NULL, NULL },
};

static const struct luaL_Reg feature_functions[] = {
  { "new", feature_new },
  { NULL,  NULL }
};

int register_feature(lua_State *L) {
  lua_pushliteral(L, "tag");
  feature_tag_ptr = lua_tostring(L, -1);
  (void) luaL_ref (L, LUA_REGISTRYINDEX);
  lua_pushliteral(L, "value");
  feature_value_ptr = lua_tostring(L, -1);
  (void) luaL_ref (L, LUA_REGISTRYINDEX);
  lua_pushliteral(L, "start");
  feature_start_ptr = lua_tostring(L, -1);
  (void) luaL_ref (L, LUA_REGISTRYINDEX);
  lua_pushliteral(L, "_end"); /* _end instead of end to avoid Lua keyword */
  feature_end_ptr = lua_tostring(L, -1);
  (void) luaL_ref (L, LUA_REGISTRYINDEX);

  return register_class(L, "harfbuzz.Feature", feature_methods, feature_functions, NULL);
}
