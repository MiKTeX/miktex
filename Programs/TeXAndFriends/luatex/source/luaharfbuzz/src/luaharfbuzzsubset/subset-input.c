#include "luaharfbuzzsubset.h"

int subset_input_new(lua_State *L) {
  SubsetInput *ud;
  hb_subset_input_t *input;
  input = hb_subset_input_create_or_fail();
  if (input == NULL) {
    lua_pushnil(L);
  } else {
    ud = (SubsetInput *)lua_newuserdata(L, sizeof(hb_subset_input_t *));
    *ud = input;
    luaL_getmetatable(L, "harfbuzz.SubsetInput");
    lua_setmetatable(L, -2);
  }
  return 1;
}

// hb_subset_input_pin_axis_location
static int subset_input_pin_axis_location(lua_State *L) {
  // arguments: input, face, tag, value
  SubsetInput *input = (SubsetInput *)luaL_checkudata(L, 1, "harfbuzz.SubsetInput");
  Face *face = (Face *)luaL_checkudata(L, 2, "harfbuzz.Face");
  if (*face == NULL) {
    lua_pushboolean(L, 0);
    return 1;
  }

  Tag *tag = (Tag *)luaL_checkudata(L, 3, "harfbuzz.Tag");
  lua_Number value = luaL_checknumber(L, 4);
  hb_bool_t ok = hb_subset_input_pin_axis_location(*input, *face, *tag, value);
  lua_pushboolean(L, ok);
  return 1;
}

// hb_subset_input_keep_everything
static int subset_input_keep_everything(lua_State *L) {
  // arguments: input only
  SubsetInput *input = (SubsetInput *)luaL_checkudata(L, 1, "harfbuzz.SubsetInput");
  hb_subset_input_keep_everything(*input);
  return 0;
}


int subset_input_unicode_set(lua_State *L) {
  SubsetInput *input = (SubsetInput *)luaL_checkudata(L, 1, "harfbuzz.SubsetInput");
  hb_set_t *unicode_set = hb_subset_input_unicode_set(*input);
  if (unicode_set == NULL) {
    lua_pushnil(L);
  } else {
    // add metatable for hb_set_t
    hb_set_t **ud = (hb_set_t **)lua_newuserdata(L, sizeof(hb_set_t *));
    *ud = unicode_set;
    luaL_getmetatable(L, "harfbuzz.Set");
    lua_setmetatable(L, -2);
  }
  return 1;
}

// subset_input_destroy
static int subset_input_destroy(lua_State *L) {
  SubsetInput *input = (SubsetInput *)luaL_checkudata(L, 1, "harfbuzz.SubsetInput");
  hb_subset_input_destroy(*input);
  return 0;
}

static const struct luaL_Reg subset_methods[] = {
    {"__gc", subset_input_destroy},
    {"unicode_set", subset_input_unicode_set},
    {"pin_axis_location", subset_input_pin_axis_location},
    {"keep_everything", subset_input_keep_everything},
    {NULL, NULL}};

static const struct luaL_Reg subset_functions[] = {
    {"new", subset_input_new},
    {NULL, NULL}};

int register_subset_input(lua_State *L) {
  return register_class(L, "harfbuzz.SubsetInput", subset_methods, subset_functions,
                        NULL);
}
