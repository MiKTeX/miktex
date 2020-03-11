#include "luaharfbuzz.h"

#ifdef LuajitTeX

static int lua_absindex (lua_State *L, int i) {
  if (i < 0 && i > LUA_REGISTRYINDEX)
    i += lua_gettop(L) + 1;
  return i;
}
static int lua_geti (lua_State *L, int index, lua_Integer i) {
  index = lua_absindex(L, index);
  lua_pushinteger(L, i);
  lua_gettable(L, index);
  return lua_type(L, -1);
}
#endif

static int buffer_new(lua_State *L) {
  Buffer *b;

  b = (Buffer *)lua_newuserdata(L, sizeof(*b));
  luaL_getmetatable(L, "harfbuzz.Buffer");
  lua_setmetatable(L, -2);

  *b = hb_buffer_create();
  return 1;
}

static int buffer_guess_segment_properties(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  hb_buffer_guess_segment_properties(*b);
  return 0;
}

static int buffer_get_direction(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  Direction *dp = (Direction *)lua_newuserdata(L, sizeof(*dp));
  luaL_getmetatable(L, "harfbuzz.Direction");
  lua_setmetatable(L, -2);

  *dp = hb_buffer_get_direction(*b);
  return 1;
}

static int buffer_set_direction(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");
  Direction* dir = (Direction *)luaL_checkudata(L, 2, "harfbuzz.Direction");

  hb_buffer_set_direction(*b, *dir);
  return 0;
}

static int buffer_get_flags(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  lua_pushinteger(L, hb_buffer_get_flags(*b));
  return 1;
}

static int buffer_set_flags(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  unsigned int l = luaL_checkinteger(L, 2);

  hb_buffer_set_flags(*b, l);
  return 0;
}

static int buffer_get_language(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  Language *lp = (Language *)lua_newuserdata(L, sizeof(*lp));
  luaL_getmetatable(L, "harfbuzz.Language");
  lua_setmetatable(L, -2);

  *lp = hb_buffer_get_language(*b);
  return 1;
}

static int buffer_set_language(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");
  Language *lang = (Language *)luaL_checkudata(L, 2, "harfbuzz.Language");

  hb_buffer_set_language(*b, *lang);
  return 0;
}

static int buffer_get_script(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  Script *sp = (Script *)lua_newuserdata(L, sizeof(*sp));
  luaL_getmetatable(L, "harfbuzz.Script");
  lua_setmetatable(L, -2);

  *sp = hb_buffer_get_script(*b);
  return 1;
}

static int buffer_set_script(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");
  Script *script = (Script *)luaL_checkudata(L, 2, "harfbuzz.Script");

  hb_buffer_set_script(*b, *script);
  return 0;
}

static int buffer_get_invisible_glyph(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  lua_pushinteger(L, hb_buffer_get_invisible_glyph(*b));
  return 1;
}

static int buffer_set_invisible_glyph(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  hb_codepoint_t cp = (hb_codepoint_t) luaL_checkinteger(L, 2);

  hb_buffer_set_invisible_glyph(*b, cp);
  return 0;
}

static int buffer_get_replacement_codepoint(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  lua_pushinteger(L, hb_buffer_get_replacement_codepoint(*b));
  return 1;
}

static int buffer_set_replacement_codepoint(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  hb_codepoint_t cp = (hb_codepoint_t) luaL_checkinteger(L, 2);

  hb_buffer_set_replacement_codepoint(*b, cp);
  return 0;
}

static int buffer_add(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  hb_codepoint_t c = (hb_codepoint_t) luaL_checkinteger(L, 2);
  unsigned int cluster = luaL_checkinteger(L, 3);

  hb_buffer_add(*b, c, cluster);

  return 0;
}

static int buffer_add_codepoints(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");
  unsigned int item_offset;
  int item_length;
  int i;

  luaL_checktype(L, 2, LUA_TTABLE);
  item_offset = luaL_optinteger(L, 3, 0);
  item_length = luaL_optinteger(L, 4, -1);

  int n = lua_rawlen(L, 2);

  hb_codepoint_t *text = (hb_codepoint_t *) malloc(n * sizeof(hb_codepoint_t));

  for (i = 0; i != n; ++i) {
    lua_geti(L, 2, i + 1);
    hb_codepoint_t c = (hb_codepoint_t) luaL_checkinteger(L, -1);
    text[i] = c;
    lua_pop(L, 1);
  }

  hb_buffer_add_codepoints(*b, text, n, item_offset, item_length);

  free(text);

  return 0;
}

static int buffer_clear_contents(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  hb_buffer_clear_contents(*b);
  return 0;
}

static int buffer_reset(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  hb_buffer_reset(*b);
  return 0;
}

static int buffer_add_utf8(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");
  const char *text;
  unsigned int item_offset;
  int item_length;

  text = luaL_checkstring(L, 2);
  item_offset = luaL_optinteger(L, 3, 0);
  item_length = luaL_optinteger(L, 4, -1);

  hb_buffer_add_utf8(*b, text, -1, item_offset, item_length);

  return 0;
}

static int buffer_destroy(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  hb_buffer_destroy(*b);
  return 0;
}

static int buffer_get_glyphs(lua_State *L) {
  Buffer *buf = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  // Get glyph info and positions out of buffer
  unsigned int len = hb_buffer_get_length(*buf);
  hb_glyph_info_t *info = hb_buffer_get_glyph_infos(*buf, NULL);
  hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(*buf, NULL);
  hb_glyph_flags_t flags;
  unsigned int i;

  // Create Lua table and push glyph data onto it.
  lua_createtable(L, len, 0); // parent table

  for (i = 0; i < len; i++) {
    lua_pushinteger(L, i+1); // 1-indexed key parent table
    lua_createtable(L, 0, 7); // child table

    lua_pushinteger(L, info[i].codepoint);
    lua_setfield(L, -2, "codepoint");

    lua_pushinteger(L, info[i].cluster);
    lua_setfield(L, -2, "cluster");

    lua_pushnumber(L, pos[i].x_advance);
    lua_setfield(L, -2, "x_advance");

    lua_pushnumber(L, pos[i].y_advance);
    lua_setfield(L, -2, "y_advance");

    lua_pushnumber(L, pos[i].x_offset);
    lua_setfield(L, -2, "x_offset");

    lua_pushnumber(L, pos[i].y_offset);
    lua_setfield(L, -2, "y_offset");

    flags = hb_glyph_info_get_glyph_flags(&(info[i]));
    if (flags & HB_GLYPH_FLAG_DEFINED) {
      lua_pushnumber(L, flags);
      lua_setfield(L, -2, "flags");
    }

    lua_settable(L, -3); // Add child table at index i+1 to parent table
  }

  return 1;
}

static int buffer_reverse(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  hb_buffer_reverse(*b);
  return 0;
}

static int buffer_get_length(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  lua_pushinteger(L, hb_buffer_get_length(*b));
  return 1;
}

static int buffer_get_cluster_level(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  lua_pushinteger(L, hb_buffer_get_cluster_level(*b));
  return 1;
}

static int buffer_set_cluster_level(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  unsigned int l = luaL_checkinteger(L, 2);

  hb_buffer_set_cluster_level(*b, l);
  return 0;
}

static int buffer_pre_allocate(lua_State *L) {
  Buffer *b = (Buffer *)luaL_checkudata(L, 1, "harfbuzz.Buffer");

  unsigned int n = luaL_checkinteger(L, 2);

  lua_pushboolean(L, hb_buffer_pre_allocate(*b, n));
  return 1;
}

static const struct luaL_Reg buffer_methods[] = {
  { "__gc", buffer_destroy },
  { "add", buffer_add },
  { "add_utf8", buffer_add_utf8 },
  { "add_codepoints", buffer_add_codepoints },
  { "clear_contents", buffer_clear_contents },
  { "set_direction", buffer_set_direction },
  { "get_direction", buffer_get_direction },
  { "set_flags", buffer_set_flags },
  { "get_flags", buffer_get_flags },
  { "set_language", buffer_set_language },
  { "get_language", buffer_get_language },
  { "set_script", buffer_set_script },
  { "get_script", buffer_get_script },
  { "set_invisible_glyph", buffer_set_invisible_glyph },
  { "get_invisible_glyph", buffer_get_invisible_glyph },
  { "set_replacement_codepoint", buffer_set_replacement_codepoint },
  { "get_replacement_codepoint", buffer_get_replacement_codepoint },
  { "get_glyphs", buffer_get_glyphs },
  { "guess_segment_properties", buffer_guess_segment_properties },
  { "reset", buffer_reset },
  { "reverse", buffer_reverse },
  { "get_length", buffer_get_length },
  { "get_cluster_level", buffer_get_cluster_level },
  { "set_cluster_level", buffer_set_cluster_level },
  { "pre_allocate", buffer_pre_allocate },
  { NULL, NULL }
};

static const struct luaL_Reg buffer_functions[] = {
  { "new", buffer_new },
  { NULL,  NULL }
};

static const struct luahb_constant_t buffer_constants[] = {
  { "CLUSTER_LEVEL_MONOTONE_GRAPHEMES", HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES },
  { "CLUSTER_LEVEL_MONOTONE_CHARACTERS", HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS },
  { "CLUSTER_LEVEL_CHARACTERS", HB_BUFFER_CLUSTER_LEVEL_CHARACTERS },
  { "CLUSTER_LEVEL_DEFAULT", HB_BUFFER_CLUSTER_LEVEL_DEFAULT },
  { "FLAG_DEFAULT", HB_BUFFER_FLAG_DEFAULT },
  { "FLAG_BOT", HB_BUFFER_FLAG_BOT },
  { "FLAG_EOT", HB_BUFFER_FLAG_EOT },
  { "FLAG_PRESERVE_DEFAULT_IGNORABLES", HB_BUFFER_FLAG_PRESERVE_DEFAULT_IGNORABLES },
  { "FLAG_REMOVE_DEFAULT_IGNORABLES", HB_BUFFER_FLAG_REMOVE_DEFAULT_IGNORABLES },
  { "FLAG_DO_NOT_INSERT_DOTTED_CIRCLE", HB_BUFFER_FLAG_DO_NOT_INSERT_DOTTED_CIRCLE },
  { "GLYPH_FLAG_UNSAFE_TO_BREAK", HB_GLYPH_FLAG_UNSAFE_TO_BREAK },
  { "GLYPH_FLAG_DEFINED", HB_GLYPH_FLAG_DEFINED },
  { NULL, 0 }
};

int register_buffer(lua_State *L) {
  return register_class(L, "harfbuzz.Buffer", buffer_methods, buffer_functions, buffer_constants);
}
