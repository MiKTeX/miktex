#include "../common.h"

#include <hb-subset.h>

typedef hb_subset_input_t* SubsetInput;

// Functions to create classes and push them onto the stack
int register_subset_input(lua_State *L);

int luaopen_luaharfbuzzsubset (lua_State *L);
