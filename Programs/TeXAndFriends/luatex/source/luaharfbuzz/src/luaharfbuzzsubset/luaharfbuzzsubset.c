#include "luaharfbuzzsubset.h"

static int subset(lua_State *L) {
  // arguments: face and input
  Face *face = (Face *)luaL_checkudata(L, 1, "harfbuzz.Face");
  SubsetInput *input = (SubsetInput*)luaL_checkudata(L, 2, "harfbuzz.SubsetInput");

  // perform the subset
  hb_face_t* subset_face = hb_subset_or_fail(*face, *input);
  if (subset_face == NULL) {
      lua_pushnil(L);
      return 0;
  }
  // create a new userdata for the subset face
  hb_face_t **ud = (hb_face_t **)lua_newuserdata(L, sizeof(hb_face_t *));
  *ud = subset_face;
  luaL_getmetatable(L, "harfbuzz.Face");
  lua_setmetatable(L, -2);
  return 1;
}

static int version (lua_State *L) {
  lua_pushstring(L, hb_version_string());
  return 1;
}

static const struct luaL_Reg lib_table [] = {
  {"subset", subset},
  {"version", version},
  {NULL, NULL}
};

int luaopen_luaharfbuzzsubset (lua_State *L) {
  lua_newtable(L);

  register_subset_input(L);
  lua_setfield(L, -2, "SubsetInput");

  luaL_setfuncs(L, lib_table, 0);

  return 1;
}

