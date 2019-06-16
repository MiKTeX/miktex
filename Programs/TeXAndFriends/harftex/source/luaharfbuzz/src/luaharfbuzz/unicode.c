#include "luaharfbuzz.h"

static int script(lua_State *L) {
  hb_codepoint_t c = (hb_codepoint_t) luaL_checkinteger(L, -1);

  Script *sp = (Script *)lua_newuserdata(L, sizeof(*sp));
  luaL_getmetatable(L, "harfbuzz.Script");
  lua_setmetatable(L, -2);

  *sp = hb_unicode_script(hb_unicode_funcs_get_default(), c);
  return 1;
}

static const struct luaL_Reg unicode_functions[] = {
  { "script", script },
  { NULL,  NULL }
};

int register_unicode(lua_State *L) {
  luaL_newlib(L, unicode_functions);
  return 1;
}
