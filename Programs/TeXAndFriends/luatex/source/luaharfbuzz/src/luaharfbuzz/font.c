#include "luaharfbuzz.h"

static int font_new(lua_State *L) {
  Font *f;
  Face *face = luaL_checkudata(L, 1, "harfbuzz.Face");

  f = (Font *)lua_newuserdata(L, sizeof(*f));
  luaL_getmetatable(L, "harfbuzz.Font");
  lua_setmetatable(L, -2);

  *f = hb_font_create(*face);

  // Set default scale to be the face's upem value
  unsigned int upem = hb_face_get_upem(*face);
  hb_font_set_scale(*f, upem, upem);

  // Set shaping functions to OpenType functions
  hb_ot_font_set_funcs(*f);
  return 1;
}

static int font_set_scale(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "harfbuzz.Font");
  int x_scale = luaL_checkinteger(L, 2);
  int y_scale = luaL_checkinteger(L, 3);

  hb_font_set_scale(*f, x_scale, y_scale);
  return 0;
}

static int font_get_scale(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "harfbuzz.Font");
  int x_scale, y_scale;

  hb_font_get_scale(*f, &x_scale, &y_scale);

  lua_pushinteger(L, x_scale);
  lua_pushinteger(L, y_scale);
  return 2;
}

static int font_get_h_extents(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "harfbuzz.Font");
  hb_font_extents_t extents;

  if (hb_font_get_h_extents(*f, &extents)) {
    lua_createtable(L, 0, 3);

    lua_pushnumber(L, extents.ascender);
    lua_setfield(L, -2, "ascender");

    lua_pushnumber(L, extents.descender);
    lua_setfield(L, -2, "descender");

    lua_pushnumber(L, extents.line_gap);
    lua_setfield(L, -2, "line_gap");
  } else {
    lua_pushnil(L);
  }

  return 1;
}

static int font_get_v_extents(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "harfbuzz.Font");
  hb_font_extents_t extents;

  if (hb_font_get_v_extents(*f, &extents)) {
    lua_createtable(L, 0, 3);

    lua_pushnumber(L, extents.ascender);
    lua_setfield(L, -2, "ascender");

    lua_pushnumber(L, extents.descender);
    lua_setfield(L, -2, "descender");

    lua_pushnumber(L, extents.line_gap);
    lua_setfield(L, -2, "line_gap");
  } else {
    lua_pushnil(L);
  }

  return 1;
}

static int font_get_glyph_extents(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "harfbuzz.Font");
  hb_codepoint_t glyph = luaL_checkinteger(L, 2);
  hb_glyph_extents_t extents;

  if (hb_font_get_glyph_extents(*f, glyph, &extents)) {
    lua_createtable(L, 0, 4);

    lua_pushnumber(L, extents.x_bearing);
    lua_setfield(L, -2, "x_bearing");

    lua_pushnumber(L, extents.y_bearing);
    lua_setfield(L, -2, "y_bearing");

    lua_pushnumber(L, extents.width);
    lua_setfield(L, -2, "width");

    lua_pushnumber(L, extents.height);
    lua_setfield(L, -2, "height");
  } else {
    lua_pushnil(L);
  }

  return 1;
}

static int font_get_glyph_name(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "harfbuzz.Font");
  hb_codepoint_t glyph = luaL_checkinteger(L, 2);

#define NAME_LEN 128
  char name[NAME_LEN];
  if (hb_font_get_glyph_name(*f, glyph, name, NAME_LEN))
    lua_pushstring(L, name);
  else
    lua_pushnil(L);
#undef NAME_LEN

  return 1;
}

static int font_get_glyph_from_name(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "harfbuzz.Font");
  const char *name = luaL_checkstring(L, 2);
  hb_codepoint_t glyph;

  if (hb_font_get_glyph_from_name(*f, name, -1, &glyph))
    lua_pushinteger(L, glyph);
  else
    lua_pushnil(L);

  return 1;
}

static int font_get_glyph_h_advance(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "harfbuzz.Font");
  hb_codepoint_t glyph = luaL_checkinteger(L, 2);

  lua_pushinteger(L, hb_font_get_glyph_h_advance(*f, glyph));
  return 1;
}

static int font_get_glyph_v_advance(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "harfbuzz.Font");
  hb_codepoint_t glyph = luaL_checkinteger(L, 2);

  lua_pushinteger(L, hb_font_get_glyph_v_advance(*f, glyph));
  return 1;
}

static int font_get_nominal_glyph(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "harfbuzz.Font");
  hb_codepoint_t uni = luaL_checkinteger(L, 2);
  hb_codepoint_t glyph;

  if (hb_font_get_nominal_glyph(*f, uni, &glyph))
    lua_pushinteger(L, glyph);
  else
    lua_pushnil(L);

  return 1;
}


static int font_destroy(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "harfbuzz.Font");

  hb_font_destroy(*f);
  return 0;
}

static int font_ot_color_glyph_get_png(lua_State *L) {
  Font *f = (Font *)luaL_checkudata(L, 1, "harfbuzz.Font");
  hb_codepoint_t gid = (hb_codepoint_t) luaL_checkinteger(L, 2);
  hb_blob_t* blob = hb_ot_color_glyph_reference_png(*f, gid);

  if (hb_blob_get_length(blob) != 0) {
    Blob *b = (Blob *)lua_newuserdata(L, sizeof(*b));
    luaL_getmetatable(L, "harfbuzz.Blob");
    lua_setmetatable(L, -2);

    *b = blob;
  } else {
    lua_pushnil(L);
  }

  return 1;
}

static const struct luaL_Reg font_methods[] = {
  { "__gc", font_destroy },
  { "set_scale", font_set_scale },
  { "get_scale", font_get_scale },
  { "get_h_extents", font_get_h_extents },
  { "get_v_extents", font_get_v_extents },
  { "get_glyph_extents", font_get_glyph_extents },
  { "get_glyph_name", font_get_glyph_name },
  { "get_glyph_from_name", font_get_glyph_from_name },
  { "get_glyph_h_advance", font_get_glyph_h_advance },
  { "get_glyph_v_advance", font_get_glyph_v_advance },
  { "get_nominal_glyph", font_get_nominal_glyph },
  { "ot_color_glyph_get_png", font_ot_color_glyph_get_png },
  { NULL, NULL }
};

static const struct luaL_Reg font_functions[] = {
  { "new", font_new },
  { NULL,  NULL }
};

int register_font(lua_State *L) {
  return register_class(L, "harfbuzz.Font", font_methods, font_functions, NULL);
}
