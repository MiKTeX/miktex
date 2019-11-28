#include "luaharfbuzz.h"

static int new_script_from_string(lua_State *L) {
  Script *sp = (Script *)lua_newuserdata(L, sizeof(*sp));
  luaL_getmetatable(L, "harfbuzz.Script");
  lua_setmetatable(L, -2);

  if (lua_gettop(L) == 1 || lua_isnil(L, -2))
    *sp = HB_SCRIPT_INVALID;
  else
    *sp = hb_script_from_string(luaL_checkstring(L, -2), -1);

  return 1;
}

static int new_script_from_tag(lua_State *L) {
  Tag* tp = (Tag *)luaL_checkudata(L, 1, "harfbuzz.Tag");

  Script *sp = (Script *)lua_newuserdata(L, sizeof(*sp));
  luaL_getmetatable(L, "harfbuzz.Script");
  lua_setmetatable(L, -2);

  *sp = hb_script_from_iso15924_tag(*tp);
  return 1;
}

static int script_to_string(lua_State *L) {
  Script *script = (Script *)luaL_checkudata(L, 1, "harfbuzz.Script");
  char s[5];

  hb_tag_to_string(hb_script_to_iso15924_tag(*script), s);
  s[4] = '\0';
  lua_pushstring(L, s);
  return 1;
}

static int script_to_tag(lua_State *L) {
  Script *script = (Script *)luaL_checkudata(L, 1, "harfbuzz.Script");

  Tag *tp = (Tag *)lua_newuserdata(L, sizeof(*tp));
  luaL_getmetatable(L, "harfbuzz.Tag");
  lua_setmetatable(L, -2);

  *tp = hb_script_to_iso15924_tag(*script);
  return 1;
}

static int script_equals(lua_State *L) {
  Script* lhs = (Script *)luaL_checkudata(L, 1, "harfbuzz.Script");
  Script* rhs = (Script *)luaL_checkudata(L, 2, "harfbuzz.Script");

  lua_pushboolean(L, *lhs == *rhs);
  return 1;
}

static const struct luaL_Reg script_methods[] = {
  { "__tostring", script_to_string },
  { "__eq", script_equals },
  { "to_iso15924_tag", script_to_tag },
  { NULL, NULL }
};

static const struct luaL_Reg script_functions[] = {
  { "new", new_script_from_string },
  { "from_iso15924_tag", new_script_from_tag },
  { NULL,  NULL }
};

int register_script(lua_State *L) {
  return register_class(L, "harfbuzz.Script", script_methods, script_functions, NULL);
}
