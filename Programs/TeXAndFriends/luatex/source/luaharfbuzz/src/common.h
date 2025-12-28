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
typedef hb_set_t* Set;
typedef hb_feature_t Feature;
typedef hb_tag_t Tag;
typedef hb_script_t Script;
typedef hb_direction_t Direction;
typedef hb_language_t Language;
typedef hb_variation_t Variation;

typedef struct luahb_constant_t {
  const char *name;
  int value;
} luahb_constant_t;

// Functions to create classes and push them onto the stack
int register_class(lua_State *L, const char *name, const luaL_Reg * methods, const luaL_Reg *functions, const luahb_constant_t* constants);
