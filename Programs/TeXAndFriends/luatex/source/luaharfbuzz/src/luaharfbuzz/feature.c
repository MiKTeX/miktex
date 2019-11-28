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

static const struct luaL_Reg feature_methods[] = {
  { "__tostring", feature_to_string },
  { NULL, NULL },
};

static const struct luaL_Reg feature_functions[] = {
  { "new", feature_new },
  { NULL,  NULL }
};

int register_feature(lua_State *L) {
  return register_class(L, "harfbuzz.Feature", feature_methods, feature_functions, NULL);
}
