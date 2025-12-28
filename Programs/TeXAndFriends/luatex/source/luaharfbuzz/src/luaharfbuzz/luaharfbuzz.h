#include "../common.h"

// Functions to create classes and push them onto the stack
int register_blob(lua_State *L);
int register_face(lua_State *L);
int register_font(lua_State *L);
int register_buffer(lua_State *L);
int register_feature(lua_State *L);
int register_tag(lua_State *L);
int register_script(lua_State *L);
int register_direction(lua_State *L);
int register_language(lua_State *L);
int register_variation(lua_State *L);
int register_ot(lua_State *L);
int register_unicode(lua_State *L);
int register_set(lua_State *L);

int luaopen_luaharfbuzz (lua_State *L);
