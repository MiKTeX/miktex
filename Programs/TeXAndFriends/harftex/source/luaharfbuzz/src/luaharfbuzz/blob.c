#include "luaharfbuzz.h"

static int blob_new(lua_State *L) {
  Blob *b;
  size_t data_l;
  const char *data = luaL_checklstring(L, 1, &data_l);

  b = (Blob *)lua_newuserdata(L, sizeof(*b));
  luaL_getmetatable(L, "harfbuzz.Blob");
  lua_setmetatable(L, -2);

  *b = hb_blob_create(data, data_l, HB_MEMORY_MODE_DUPLICATE, (void*)data, NULL);
  return 1;
}

static int blob_new_from_file(lua_State *L) {
  Blob *b;
  const char *file_name = luaL_checkstring(L, 1);

  b = (Blob *)lua_newuserdata(L, sizeof(*b));
  luaL_getmetatable(L, "harfbuzz.Blob");
  lua_setmetatable(L, -2);

  *b = hb_blob_create_from_file(file_name);
  return 1;
}

static int blob_get_length(lua_State *L) {
  Blob *b = (Blob *)luaL_checkudata(L, 1, "harfbuzz.Blob");

  lua_pushinteger(L, hb_blob_get_length(*b));
  return 1;
}

static int blob_get_data(lua_State *L) {
  Blob *b = (Blob *)luaL_checkudata(L, 1, "harfbuzz.Blob");
  unsigned int l;
  const char *d;

  d = hb_blob_get_data(*b, &l);
  lua_pushlstring(L, d, l);

  return 1;
}

static const struct luaL_Reg blob_methods[] = {
  { "get_length", blob_get_length },
  { "get_data", blob_get_data },
  { NULL, NULL }
};

static const struct luaL_Reg blob_functions[] = {
  { "new", blob_new },
  { "new_from_file", blob_new_from_file },
  { NULL,  NULL }
};

int register_blob(lua_State *L) {
  return register_class(L, "harfbuzz.Blob", blob_methods, blob_functions, NULL);
}
