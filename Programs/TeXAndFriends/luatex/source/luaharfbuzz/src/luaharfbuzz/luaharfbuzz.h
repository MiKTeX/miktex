#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <hb.h>
#include <hb-ot.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

typedef hb_blob_t* Blob;
typedef hb_face_t* Face;
typedef hb_font_t* Font;
typedef hb_buffer_t* Buffer;
typedef hb_feature_t Feature;
typedef hb_tag_t Tag;
typedef hb_script_t Script;
typedef hb_direction_t Direction;
typedef hb_language_t Language;

typedef struct luahb_constant_t {
  const char *name;
  int value;
} luahb_constant_t;

// Functions to create classes and push them onto the stack
int register_class(lua_State *L, const char *name, const luaL_Reg * methods, const luaL_Reg *functions, const luahb_constant_t* constants);
int register_blob(lua_State *L);
int register_face(lua_State *L);
int register_font(lua_State *L);
int register_buffer(lua_State *L);
int register_feature(lua_State *L);
int register_tag(lua_State *L);
int register_script(lua_State *L);
int register_direction(lua_State *L);
int register_language(lua_State *L);
int register_ot(lua_State *L);
int register_unicode(lua_State *L);

// Missed declaration
int luaopen_luaharfbuzz (lua_State *L);
