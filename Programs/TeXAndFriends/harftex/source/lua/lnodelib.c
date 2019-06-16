/* lnodelib.c

    Copyright 2006-2013 Taco Hoekwater <taco@luatex.org>

    This file is part of LuaTeX.

    LuaTeX is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    LuaTeX is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
    License for more details.

    You should have received a copy of the GNU General Public License along
    with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

/*

    After doing lots of tests with luatex and luajittex, with and without jit,
    and with and without ffi, we came to the conclusion that userdata prevents
    a speedup. We also found that the checking of metatables as well as assignment
    comes with overhead that can't be neglected. This is normally not really a
    problem but when processing fonts for more complex scripts it's quite some
    overhead.

    Because the userdata approach has some benefits, we stick to this. We did
    some experiments with fast access (assuming nodes), but eventually settled for
    the direct approach. For code that is proven to be okay, one can use the
    direct variants and operate on nodes more directly. Currently these are
    numbers but don't rely on that property; treat them as abstractions. An
    important aspect    is that one cannot mix both methods, although with
    node.direct.tonode and node.direct.todirect one can cast representations.

    So the advice is: use the indexed approach when possible and investigate the
    direct one when speed might be an issue. For that reason we also provide some
    get* and set* functions in the top level node namespace. There is a limited set
    of getters for nodes and a generic getfield to complement them. The direct
    namespace has a few more.

    Keep in mind that such speed considerations only make sense when we're accessing
    nodes millions of times (which happens in font processing for instance). Setters
    are less important as documents have not that many content related nodes and
    setting many thousands of properties is hardly a burden contrary to millions of
    consultations. And with millions, we're talking of tens of millions which is not
    that common.

    Another change is that __index and __newindex are (as expected) exposed to
    users but do no checking. The getfield and setfield functions do check. In
    fact, a fast mode can be simulated by fast_getfield = __index but the (measured)
    benefit on average runs is not that large (some 5% when we also use the other
    fast ones) which is easily nilled by inefficient coding. The direct variants
    on the other hand can be significantly faster but with the drawback of lack
    of userdata features. With respect to speed: keep in mind that measuring
    a speedup on these functions is not representative for a normal run, where
    much more happens.

*/

#include "ptexlib.h"
#include "lua/luatex-api.h"
#ifdef LuajitTeX
#include "lua/lauxlib_bridge.h"
#else
#include "lauxlib.h"
#endif

/*

    These macros create and access pointers (indices) to keys which is faster. The
    shortcuts are created as part of the initialization.

*/

/*

    When the first argument to an accessor is a node, we can use it's metatable
    entry when we are returning nodes, which saves a lookup.

*/

#define fast_metatable(n) do {                              \
     a = (halfword *) lua_newuserdata(L, sizeof(halfword)); \
     *a = n;                                                \
     lua_getmetatable(L,1);                                 \
     lua_setmetatable(L,-2);                                \
} while (0)

#define fast_metatable_or_nil(n) do {                          \
     if (n) {                                                  \
        a = (halfword *) lua_newuserdata(L, sizeof(halfword)); \
        *a = n;                                                \
        lua_getmetatable(L,1);                                 \
        lua_setmetatable(L,-2);                                \
    } else {                                                   \
        lua_pushnil(L);                                        \
    }                                                          \
} while (0)

#define fast_metatable_or_nil_alink(n) do {                    \
     if (n) {                                                  \
        alink(n) = null;                                       \
        a = (halfword *) lua_newuserdata(L, sizeof(halfword)); \
        *a = n;                                                \
        lua_getmetatable(L,1);                                 \
        lua_setmetatable(L,-2);                                \
    } else {                                                   \
        lua_pushnil(L);                                        \
   }                                                           \
} while (0)

#define fast_metatable_top(n) do {                          \
     a = (halfword *) lua_newuserdata(L, sizeof(halfword)); \
     *a = n;                                                \
     lua_getmetatable(L,-2);                                \
     lua_setmetatable(L,-2);                                \
} while (0)

/*

    This is a first step towards abstract direct nodes. When we have Lua 5.3 we
    need to check all returned values for being integers. This might be another
    level of abtraction.

*/

#define nodelib_pushdirect(n) lua_pushinteger(L,n)
#define nodelib_popdirect(n) lua_tointeger(L,n)

#define nodelib_pushdirect_or_nil(n) do { \
    if (n==null) {                        \
        lua_pushnil(L);                   \
    } else {                              \
        lua_pushinteger(L,n);             \
    }                                     \
} while (0)

#define nodelib_pushdirect_or_nil_alink(n) do { \
    if (n==null) {                              \
        lua_pushnil(L);                         \
    } else {                                    \
        alink(n) = null;                        \
        lua_pushinteger(L,n);                   \
    }                                           \
} while (0)

#define nodelib_setattr(L, s, n)     reassign_attribute(n,nodelib_getlist(L,s))

#define nodelib_gettoks(L,a)   tokenlist_from_lua(L)

#define nodelib_getspec        nodelib_getlist
#define nodelib_getaction      nodelib_getlist

/* fetching a field from a node .. we can often use the reuse bot-of-stack metatable */

#define nodelib_pushlist(L,n) { lua_pushinteger(L,n); lua_nodelib_push(L); }      /* can be: fast_metatable_or_nil(n) */
#define nodelib_pushattr(L,n) { lua_pushinteger(L,n); lua_nodelib_push(L); }      /* can be: fast_metatable_or_nil(n) */
#define nodelib_pushaction(L,n) { lua_pushinteger(L,n); lua_nodelib_push(L); }    /* can be: fast_metatable_or_nil(n) */
#define nodelib_pushstring(L,n) { char *ss=makecstring(n); lua_pushstring(L,ss); free(ss); }

/* find prev, and fix backlinks .. can be a macro instead (only used a few times) */

#define set_t_to_prev(head,current)      \
t = head;                                \
while (vlink(t)!=current && t != null) { \
    if (vlink(t)!=null)                  \
        alink(vlink(t)) = t;             \
    t = vlink(t);                        \
}

#define direct_check_index_range(j,s)                                      \
    if (j<0 || j > 65535) {                                                \
        luaL_error(L, "incorrect index value %d for tex.%s()", (int)j, s); \
    }

#define NODE_METATABLE  "luatex.node"

#define DEBUG 0
#define DEBUG_OUT stdout

/* maybe these qualify as macros, not functions */

static halfword *maybe_isnode(lua_State * L, int i)
{
    halfword *p = lua_touserdata(L, i);
    if (p != NULL) {
        if (lua_getmetatable(L, i)) {
            lua_get_metatablelua(luatex_node);
            if (!lua_rawequal(L, -1, -2))
                p = NULL;
            lua_pop(L, 2);
        }
    }
    return p;
}

/* we could make the message a function and just inline the rest (via a macro) */

halfword *check_isnode(lua_State * L, int i)
{
    halfword *p = maybe_isnode(L, i);
    if (p != NULL)
        return p;
    formatted_error("node lib","lua <node> expected, not an object with type %s", luaL_typename(L, i));
    return NULL;
}

static int nodelib_setdir_text(lua_State * L, int i, halfword n)
{
    if (lua_type(L, i) == LUA_TSTRING) {
        const char *s = lua_tostring(L, i);
        if (s==lua_key_plus(TLT)) {
            dir_dir(n) = 0 ;
            subtype(n) = normal_dir;
        } else if (s==lua_key_minus(TLT)) {
            dir_dir(n) = 0 ;
            subtype(n) = cancel_dir;
        } else if (s==lua_key_plus(TRT)) {
            dir_dir(n) = 1 ;
            subtype(n) = normal_dir;
        } else if (s==lua_key_minus(TRT)) {
            dir_dir(n) = 1 ;
            subtype(n) = cancel_dir;
        } else if (s==lua_key_plus(LTL)) {
            dir_dir(n) = 2 ;
            subtype(n) = normal_dir;
        } else if (s==lua_key_minus(LTL)) {
            dir_dir(n) = 2 ;
            subtype(n) = cancel_dir;
        } else if (s==lua_key_plus(RTT)) {
            dir_dir(n) = 3 ;
            subtype(n) = normal_dir;
        } else if (s==lua_key_minus(RTT)) {
            dir_dir(n) = 3 ;
            subtype(n) = cancel_dir;
        } else {
            luaL_error(L, "Bad direction specifier %s", s);
        }
    } else {
        luaL_error(L, "Direction specifiers have to be strings");
    }
    return 0;
}

static int nodelib_getdir_par(lua_State * L, int n)
{
    if (lua_type(L, n) == LUA_TSTRING) {
        const char *s = lua_tostring(L, n);
        if (s==lua_key(TLT))
            return 0 ;
        else if (s==lua_key(TRT))
            return 1 ;
        else if (s==lua_key(LTL))
            return 2 ;
        else if (s==lua_key(RTT))
            return 3 ;
        else
            luaL_error(L, "Bad direction specifier %s", s);
    } else {
        luaL_error(L, "Direction specifiers have to be strings");
    }
    return 0;
}

static int nodelib_getdirection(lua_State * L, int n)
{
    if (lua_type(L, n) == LUA_TNUMBER) {
        int i = lua_tointeger(L, n);
        check_dir_value(i);
        return i;
    } else {
        luaL_error(L, "Direction specifiers have to be numbers");
    }
    return 0;
}

int nodelib_getdir(lua_State * L, int n) /* the api public one */
{
    return nodelib_getdir_par(L,n);
}

/*

    This routine finds the numerical value of a string (or number) at
    lua stack index |n|. If it is not a valid node type, returns -1

*/

/* s = lua_tostring(L, 2); */
/* if (lua_key_eq(s, id)) { */

static int get_node_type_id_from_name(lua_State * L, int n, node_info * data)
{
    if (data != NULL) {
        int j;
        const char *s = lua_tostring(L, n);
        for (j = 0; data[j].id != -1; j++) {
            if (s == data[j].name) {
                return j;
            }
        }
    }
    return -1;
}

static int get_node_subtype_id_from_name(lua_State * L, int n, subtype_info * data)
{
    if (data != NULL) {
        int j;
        const char *s = lua_tostring(L, n);
        for (j = 0; data[j].id != -1; j++) {
            if (s == data[j].name) {
                return j;
            }
        }
    }
    return -1;
}

static int get_valid_node_type_id(lua_State * L, int n)
{
    int i = -1;
    int t = lua_type(L, n);
    if (t == LUA_TSTRING) {
        i = get_node_type_id_from_name(L,n,node_data);
        if (i < 0) {
            luaL_error(L, "invalid node type id: %s", lua_tostring(L, n));
        }
    } else if (t == LUA_TNUMBER) {
        i = lua_tointeger(L,n);
        if (! known_node_type(i)) {
            luaL_error(L, "invalid node type id: %d", i);
        }
    } else {
        luaL_error(L, "invalid node type id");
    }
    return i;
}

static int get_valid_node_subtype_id(lua_State * L, int n)
{
    int i = -1;
    int t = lua_type(L, n);
    if (t == LUA_TSTRING) {
        i = get_node_type_id_from_name(L,n,whatsit_node_data);
        if (i<0) {
            luaL_error(L, "invalid whatsit type id: %s", lua_tostring(L, n));
        }
    } else if (t == LUA_TNUMBER) {
        i = lua_tointeger(L,n);
        if (! known_whatsit_type(i)) {
            luaL_error(L, "invalid whatsit type id: %d", i);
        }
    } else {
        luaL_error(L, "invalid whatsit type id");
    }
    return i;
}

/* enable fixer ... more fixers can be added in the future */

static int lua_nodelib_fix_node_lists(lua_State * L)
{
    if (lua_type(L, -1) == LUA_TBOOLEAN) {
        fix_node_lists = lua_toboolean(L,-1);
    } else if (lua_type(L, -1) == LUA_TNUMBER) {
        fix_node_lists = lua_tointeger(L,-1);
    }
    return 0;
}


/* two simple helpers to speed up and simplify lua code (replaced by getnext and getprev) */

static int lua_nodelib_next(lua_State * L)
{
    halfword *p = maybe_isnode(L,1);
    if (p != NULL && *p && vlink(*p)) {
        lua_nodelib_push_fast(L,vlink(*p));
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_prev(lua_State * L)
{
    halfword *p = maybe_isnode(L,1);
    if (p != NULL && *p && alink(*p)) {
        lua_nodelib_push_fast(L,alink(*p));
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/*

    Creates a userdata object for a number found at the stack top, if it is
    representing a node (i.e. an pointer into |varmem|). It replaces the
    stack entry with the new userdata, or pushes |nil| if the number is |null|,
    or if the index is definately out of range. This test could be improved.

*/

void lua_nodelib_push(lua_State * L)
{
    halfword n;
    halfword *a;
    n = -1;
    if (lua_type(L, -1) == LUA_TNUMBER)
        n = (int) lua_tointeger(L, -1);
    lua_pop(L, 1);
    if ((n == null) || (n < 0) || (n > var_mem_max)) {
        lua_pushnil(L);
    } else {
        a = lua_newuserdata(L, sizeof(halfword));
        *a = n;
        lua_get_metatablelua(luatex_node);
        lua_setmetatable(L, -2);
    }
    return;
}

void lua_nodelib_push_fast(lua_State * L, halfword n)
{
    halfword *a;
    if (n) {
        a = lua_newuserdata(L, sizeof(halfword));
        *a = n;
        lua_get_metatablelua(luatex_node);
        lua_setmetatable(L, -2);
    } else {
        lua_pushnil(L);
    }
    return;
}

/* getting and setting fields (helpers) */

int nodelib_getlist(lua_State * L, int n)
{
    if (lua_isuserdata(L, n)) {
        halfword m = *check_isnode(L, n);
        return m;
    } else {
        return null;
    }
}

static str_number nodelib_getstring(lua_State * L, int a)
{
    size_t k;
    const char *s = lua_tolstring(L, a, &k);
    return maketexlstring(s, k);
}

static int nodelib_cantset(lua_State * L, int n, const char *s)
{
    luaL_error(L,"You cannot set field %s in a node of type %s",s,node_data[type(n)].name);
    return 0;
}

/* converts type strings to type ids */

static int lua_nodelib_id(lua_State * L)
{
    if (lua_type(L,1) == LUA_TSTRING) {
        int i = get_node_type_id_from_name(L, 1, node_data);
        if (i >= 0) {
            lua_pushinteger(L, i);
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* node.direct.getid */

static int lua_nodelib_direct_getid(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        lua_pushinteger(L, type(n));
    } else {
        lua_pushnil(L);
    }
    return 1;
}

    /* node.getid */

    static int lua_nodelib_getid(lua_State * L)
    {
        /* [given-node] [...] */
        halfword *p = lua_touserdata(L, 1);
        if ( (p == NULL) || (! lua_getmetatable(L,1)) ) {
            lua_pushnil(L);
            return 1;
        }
        /* [given-node] [mt-given-node] */
        lua_get_metatablelua(luatex_node);
        /* [given-node] [mt-given-node] [mt-node] */
        if (!lua_rawequal(L, -1, -2)) {
            lua_pushnil(L);
        } else {
            lua_pushinteger(L, type(*p));
        }
        return 1;
    }

/* node.direct.getsubtype */
/* node.direct.setsubtype */

static int lua_nodelib_direct_getsubtype(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        lua_pushinteger(L, subtype(n));
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_setsubtype(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if ((n) && (lua_type(L,2) == LUA_TNUMBER)) {
        subtype(n) = (halfword) lua_tointeger(L, 2);
    }
    return 0;
}

    /* node.getsubtype */

    static int lua_nodelib_getsubtype(lua_State * L)
    {
        halfword *p = lua_touserdata(L, 1);
        if ( (p == NULL) || (! lua_getmetatable(L,1)) ) {
            lua_pushnil(L);
        } else {
            lua_get_metatablelua(luatex_node);
            if ( (!lua_rawequal(L, -1, -2)) || (! nodetype_has_subtype(*p))) {
                lua_pushnil(L);
            } else {
                lua_pushinteger(L, subtype(*p));
            }
        }
        return 1;
    }

/* node.direct.getexpansion */
/* node.direct.setexpansion */

static int lua_nodelib_direct_getexpansion(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n != null) {
        halfword t = type(n);
        if (t == glyph_node) {
            lua_pushinteger(L, ex_glyph(n));
            return 1;
        } else if (t == kern_node) {
            lua_pushinteger(L, ex_kern(n));
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

static int lua_nodelib_direct_setexpansion(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        halfword e = 0;
        if (lua_type(L, 2) == LUA_TNUMBER) {
            e = (halfword) lua_tointeger(L, 2);
        }
        if (t == glyph_node) {
            ex_glyph(n) = e;
        } else if ( t == kern_node) {
            ex_kern(n) = e;
        }
    }
    return 0;
}

/* node.direct.getfont */
/* node.direct.setfont */

static int lua_nodelib_direct_getfont(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n != null) {
        halfword t = type(n);
        if (t == glyph_node) {
            lua_pushinteger(L, font(n));
        } else if ((t == math_char_node) || (t == math_text_char_node)) {
            lua_pushinteger(L, fam_fnt(math_fam(n), 0));
        } else if (t == delim_node) {
            lua_pushinteger(L, fam_fnt(small_fam(n), 0));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_setfont(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if ((n) && type(n) == glyph_node) {
        font(n) = (halfword) lua_tointeger(L,2);
        /* optional char */
        if ((lua_type(L, 3) == LUA_TNUMBER)) {
            character(n) = (halfword) lua_tointeger(L, 3);
        }
    }
    return 0;
}

    /* node.getfont */

    static int lua_nodelib_getfont(lua_State * L)
    {
        halfword *n = lua_touserdata(L, 1);
        if ((n == NULL) || (! lua_getmetatable(L,1)) ) {
            lua_pushnil(L);
        } else {
            halfword t = type(*n);
            if (t == glyph_node) {
                lua_pushinteger(L, font(*n));
            } else if ((t == math_char_node) || (t == math_text_char_node)) {
                lua_pushinteger(L, fam_fnt(math_fam(*n), 0));
            } else if (t == delim_node) {
                lua_pushinteger(L, fam_fnt(small_fam(*n), 0));
            } else {
                lua_pushnil(L);
            }
        }
        return 1;
    }

/* node.direct.getchar */
/* node.direct.setchar */

static int lua_nodelib_direct_getchar(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == glyph_node) {
            lua_pushinteger(L, character(n));
        } else if ((t == math_char_node) || (t == math_text_char_node)) {
            lua_pushinteger(L, math_character(n));
        } else if (t == delim_node) {
             /* used in wide fonts */
            lua_pushinteger(L, small_char(n));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_getfam(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if ((t == math_char_node) || (t == math_text_char_node)) {
            lua_pushinteger(L, math_fam(n));
        } else if (t == delim_node) {
            lua_pushinteger(L, small_fam(n));
        } else if (t == fraction_noad) {
            lua_pushinteger(L, fraction_fam(n));
        } else if (t == simple_noad) {
            lua_pushinteger(L, noad_fam(n));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_setchar(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if ((n) && (lua_type(L, 2) == LUA_TNUMBER)) {
        halfword t = type(n);
        if (t == glyph_node) {
            character(n) = (halfword) lua_tointeger(L, 2);
        } else if ((t == math_char_node) || (t == math_text_char_node)) {
            math_character(n) = (halfword) lua_tointeger(L, 2);
        } else if (t == delim_node) {
            /* used in wide fonts */
            small_char(n) = (halfword) lua_tointeger(L, 2);
        }
    }
    return 0;
}

static int lua_nodelib_direct_setfam(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if ((n) && (lua_type(L, 2) == LUA_TNUMBER)) {
        halfword t = type(n);
        if ((t == math_char_node) || (t == math_text_char_node)) {
            math_fam(n) = (halfword) lua_tointeger(L, 2);
        } else if (t == delim_node) {
            small_fam(n) = (halfword) lua_tointeger(L, 2);
        } else if (t == fraction_noad) {
            fraction_fam(n) = (halfword) lua_tointeger(L, 2);
        } else if (t == simple_noad) {
            noad_fam(n) = (halfword) lua_tointeger(L, 2);
        }
    }
    return 0;
}

    /* node.getchar */

static int lua_nodelib_getchar(lua_State * L)
    {
        halfword *n = lua_touserdata(L, 1);
        if ((n == NULL) || (! lua_getmetatable(L,1))) {
            lua_pushnil(L);
        } else {
            halfword t = type(*n);
            if (t == glyph_node) {
                lua_pushinteger(L, character(*n));
            } else if ((t == math_char_node) || (t == math_text_char_node)) {
                lua_pushinteger(L, math_character(*n));
            } else if (t == delim_node) {
                /* used in wide fonts */
                lua_pushinteger(L, small_char(*n));
            } else {
                lua_pushnil(L);
            }
        }
        return 1;
    }

/* node.direct.getcomponents */
/* node.direct.setcomponents */

static int lua_nodelib_direct_getcomponents(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if ((n) && (type(n) == glyph_node)) {
        nodelib_pushdirect_or_nil(lig_ptr(n));
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_setcomponents(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if ((n) && (type(n) == glyph_node)) {
        if (lua_type(L, 2) == LUA_TNUMBER) {
            lig_ptr(n) = (halfword) lua_tointeger(L, 2);
        } else {
            lig_ptr(n) = null;
        }
    }
    return 0;
}

/* node.direct.getlang */
/* node.direct.setlang */

static int lua_nodelib_direct_getlang(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if ((n) && (type(n) == glyph_node)) {
        lua_pushinteger(L, char_lang(n));
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_setlang(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if ((n) && (type(n) == glyph_node)) {
        if (lua_type(L, 2) == LUA_TNUMBER) {
            set_char_lang(n,lua_tointeger(L, 2));
        } else {
            /* nothing */
        }
    }
    return 0;
}

/* node.direct.getattributelist */
/* node.direct.setattributelist */

static int lua_nodelib_direct_getattributelist(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if ((n) && nodetype_has_attributes(type(n)) && node_attr(n) != null) {
        lua_pushinteger(L, node_attr(n));
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_setattributelist(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if ((n) && nodetype_has_attributes(type(n))) {
        int t = lua_type(L, 2);
        if (t == LUA_TNUMBER) {
            halfword a =lua_tointeger(L, 2);
            if (type(a) == attribute_list_node) {
                reassign_attribute(n,a);
            } else if (nodetype_has_attributes(type(a))) {
                reassign_attribute(n,node_attr(a));
            } else {
                reassign_attribute(n,null);
            }
        } else if (t == LUA_TBOOLEAN) {
            if (lua_toboolean(L,2)) {
                reassign_attribute(n,current_attribute_list());
            } else {
                reassign_attribute(n,null);
            }
        } else {
            reassign_attribute(n,null);
        }
        return 0;
    }
    return 0;
}
/* node.direct.getpenalty */
/* node.direct.setpenalty */

static int lua_nodelib_direct_getpenalty(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == penalty_node) {
            lua_pushinteger(L, penalty(n));
        } else if (t == disc_node) {
            lua_pushinteger(L, disc_penalty(n));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_setpenalty(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == penalty_node) {
            if (lua_type(L, 2) == LUA_TNUMBER) {
                penalty(n) = (halfword) lua_tointeger(L, 2);
            } else {
                penalty(n) = 0;
            }
        } else if (t == disc_node) {
            if (lua_type(L, 2) == LUA_TNUMBER) {
                disc_penalty(n) = (halfword) lua_tointeger(L, 2);
            } else {
                penalty(n) = 0;
            }
        }
    }
    return 0;
}

/* node.direct.getnucleus */
/* node.direct.getsub */
/* node.direct.getsup */

static int lua_nodelib_direct_getnucleus(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == simple_noad || t == accent_noad || t == radical_noad) {
            nodelib_pushdirect_or_nil(nucleus(n));
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

static int lua_nodelib_direct_setnucleus(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == simple_noad || t == accent_noad || t == radical_noad) {
            if (lua_type(L, 2) == LUA_TNUMBER) {
                nucleus(n) = lua_tointeger(L,2);
            } else {
                nucleus(n) = null;
            }
        }
    }
    return 0;
}

static int lua_nodelib_direct_getsub(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == simple_noad || t == accent_noad || t == radical_noad) {
            nodelib_pushdirect_or_nil(subscr(n));
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

static int lua_nodelib_direct_setsub(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == simple_noad || t == accent_noad || t == radical_noad) {
            if (lua_type(L, 2) == LUA_TNUMBER) {
                subscr(n) = lua_tointeger(L,2);
            } else {
                subscr(n) = null;
            }
        }
    }
    return 0;
}

static int lua_nodelib_direct_getsup(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == simple_noad || t == accent_noad || t == radical_noad) {
            nodelib_pushdirect_or_nil(supscr(n));
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

static int lua_nodelib_direct_setsup(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == simple_noad || t == accent_noad || t == radical_noad) {
            if (lua_type(L, 2) == LUA_TNUMBER) {
                supscr(n) = lua_tointeger(L,2);
            } else {
                supscr(n) = null;
            }
        }
    }
    return 0;
}

/* node.direct.getkern (overlaps with getwidth) */
/* node.direct.setkern (overlaps with getwidth) */

static int lua_nodelib_direct_getkern(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == kern_node) {
            lua_pushnumber(L, width(n));
            if (lua_toboolean(L,2)) {
                lua_pushinteger(L, ex_kern(n));
                return 2;
            }
        } else if (t == margin_kern_node) {
            lua_pushinteger(L, width(n));
        } else if (t == math_node) {
            lua_pushinteger(L, surround(n));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_setkern(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n != null) {
        halfword t = type(n);
        if (t == kern_node || t == margin_kern_node) {
            if (lua_type(L, 2) == LUA_TNUMBER) {
                width(n) = (halfword) lua_roundnumber(L, 2);
            } else {
                width(n) = 0;
            }
            if (lua_type(L, 3) == LUA_TNUMBER) {
                subtype(n) = (halfword) lua_tointeger(L, 3);
            }
        } else if (t == math_node) {
            if (lua_type(L, 2) == LUA_TNUMBER) {
                surround(n) = (halfword) lua_roundnumber(L, 2);
            } else {
                surround(n) = 0;
            }
        }
    }
    return 0;
}

/* node.direct.getdir */
/* node.direct.setdir */

static int lua_nodelib_direct_getdir(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == dir_node) {
            lua_push_dir_text(L, dir_dir(n), subtype(n));
        } else if (t == hlist_node || t == vlist_node) {
            lua_push_dir_par(L, box_dir(n));
        } else if (t == rule_node) {
            lua_push_dir_par(L, rule_dir(n));
        } else if (t == local_par_node) {
            lua_push_dir_par(L, local_par_dir(n));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_getdirection(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == dir_node) {
            lua_push_direction(L, dir_dir(n));
            lua_pushboolean(L, subtype(n));
            return 2;
        } else if (t == hlist_node || t == vlist_node) {
            lua_push_direction(L, box_dir(n));
        } else if (t == rule_node) {
            lua_push_direction(L, rule_dir(n));
        } else if (t == local_par_node) {
            lua_push_direction(L, local_par_dir(n));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_setdir(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == dir_node) {
            nodelib_setdir_text(L, 2, n);
        } else if (t == hlist_node || type(n) == vlist_node) {
            box_dir(n) = nodelib_getdir_par(L, 2);
        } else if (t == rule_node) {
            rule_dir(n) = nodelib_getdir_par(L, 2);
        } else if (t == local_par_node) {
            local_par_dir(n) = nodelib_getdir_par(L, 2);
        }
    }
    return 0;
}

static int lua_nodelib_direct_setdirection(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == dir_node) {
            dir_dir(n) = nodelib_getdirection(L, 2);
            if ((lua_type(L, 3) == LUA_TBOOLEAN)) {
                if (lua_toboolean(L, 3)) {
                    subtype(n) = cancel_dir;
                } else {
                    subtype(n) = normal_dir;
                }
            }
        } else if (t == hlist_node || type(n) == vlist_node) {
            box_dir(n) = nodelib_getdirection(L, 2);
        } else if (t == rule_node) {
            rule_dir(n) = nodelib_getdirection(L, 2);
        } else if (t == local_par_node) {
            local_par_dir(n) = nodelib_getdirection(L, 2);
        }
    }
    return 0;
}

/* node.direct.getoffsets */
/* node.direct.setoffsets */

static int lua_nodelib_direct_getoffsets(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        if (type(n) == glyph_node) {
            lua_pushinteger(L, x_displace(n));
            lua_pushinteger(L, y_displace(n));
            return 2;
        } else if (type(n) == rule_node) {
            lua_pushinteger(L, rule_left(n));
            lua_pushinteger(L, rule_right(n));
            return 2;
        }
    }
    return 0;
}

static int lua_nodelib_direct_setoffsets(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        if (type(n) == glyph_node) {
            if (lua_type(L, 2) == LUA_TNUMBER) {
                x_displace(n) = (halfword) lua_roundnumber(L, 2);
            }
            if (lua_type(L, 3) == LUA_TNUMBER) {
                y_displace(n) = (halfword) lua_roundnumber(L, 3);
            }
        } else if (type(n) == rule_node) {
            if (lua_type(L, 2) == LUA_TNUMBER) {
                rule_left(n) = (halfword) lua_roundnumber(L, 2);
            }
            if (lua_type(L, 3) == LUA_TNUMBER) {
                rule_right(n) = (halfword) lua_roundnumber(L, 3);
            }
        }
    }
    return 0;
}

/* node.direct.getdisc */
/* node.direct.setdisc */

static int lua_nodelib_direct_getdisc(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if ((n) && (type(n) == disc_node)) {
        nodelib_pushdirect_or_nil(vlink(pre_break(n)));
        nodelib_pushdirect_or_nil(vlink(post_break(n)));
        nodelib_pushdirect_or_nil(vlink(no_break(n)));
        if (lua_isboolean(L, 2) && lua_toboolean(L, 2)) {
            nodelib_pushdirect_or_nil(tlink(pre_break(n)));
            nodelib_pushdirect_or_nil(tlink(post_break(n)));
            nodelib_pushdirect_or_nil(tlink(no_break(n)));
            return 6;
        }
        return 3;
    } else {
        return 0;
    }
}

static int lua_nodelib_direct_setdisc(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (type(n) == disc_node) {
        int t = lua_gettop(L) ;
        if (t > 1) {
            set_disc_field(pre_break(n), lua_tointeger(L,2));
            if (t > 2) {
                set_disc_field(post_break(n), lua_tointeger(L,3));
                if (t > 3) {
                    set_disc_field(no_break(n), lua_tointeger(L,4));
                    if (t > 4) {
                        subtype(n) = (quarterword) lua_tointeger(L,5);
                        if (t > 5) {
                            disc_penalty(n) = lua_tointeger(L,6);
                        }
                    }
                } else {
                    set_disc_field(no_break(n), null);
                }
            } else {
                set_disc_field(post_break(n), null);
                set_disc_field(no_break(n), null);
            }
        } else {
            set_disc_field(pre_break(n), null);
            set_disc_field(post_break(n), null);
            set_disc_field(no_break(n), null);
        }
    }
    return 0;
}

    /* node.getdisc */

    static int lua_nodelib_getdisc(lua_State * L)
    {
        halfword *a;
        halfword *n = lua_touserdata(L, 1);
        if ((n != NULL) && (type(*n) == disc_node)) {
            fast_metatable_or_nil(vlink(pre_break(*n)));
            fast_metatable_or_nil(vlink(post_break(*n)));
            fast_metatable_or_nil(vlink(no_break(*n)));
            if (lua_isboolean(L, 2) && lua_toboolean(L, 2)) {
                fast_metatable_or_nil(tlink(pre_break(*n)));
                fast_metatable_or_nil(tlink(post_break(*n)));
                fast_metatable_or_nil(tlink(no_break(*n)));
                return 6;
            }
            return 3;
        }
        return 0;
    }

/* node.direct.getwhd */
/* node.direct.setwhd */

# define push_list_whd(n) do { \
    lua_pushinteger(L, width(n)); \
    lua_pushinteger(L, height(n)); \
    lua_pushinteger(L, depth(n)); \
} while (0)

# define push_char_whd(n) do { \
    lua_pushinteger(L, char_width(font(n),character(n))); \
    lua_pushinteger(L, char_height(font(n),character(n))); \
    lua_pushinteger(L, char_depth(font(n),character(n))); \
} while (0)

# define push_char_ehd(n) do { \
    lua_pushinteger(L, char_width(font(n),character(n))); \
    lua_pushinteger(L, char_height(font(n),character(n))); \
    lua_pushinteger(L, char_depth(font(n),character(n))); \
    lua_pushinteger(L, ex_glyph(n)); \
} while (0)

static int lua_nodelib_direct_getwhd(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n != null) {
        halfword t = type(n);
        if ((t == hlist_node) || (t == vlist_node) || (t == rule_node) || (t == unset_node)) {
            push_list_whd(n);
            return 3;
        } else if (t == glyph_node) {
            if (lua_toboolean(L,2)) {
                push_char_ehd(n);
                return 4;
            } else {
                push_char_whd(n);
                return 3;
            }
        } else if (t == glue_node) {
            halfword l = leader_ptr(n);
            if (l != null) {
                t = type(l) ;
                if ((t == hlist_node) || (t == vlist_node) || (t == rule_node)) {
                    push_list_whd(l);
                    return 3;
                }
            }
        }
    }
    return 0;
}

static int lua_nodelib_direct_setwhd(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n != null) {
        halfword t = type(n);
        if (t == glue_node) {
            n = leader_ptr(n);
            if (n == null) {
                return 0;
            } else {
                t = type(n);
            }
        }
        if ((t == hlist_node) || (t == vlist_node) || (t == rule_node) || (t == unset_node)) {
            int t = lua_gettop(L) ;
            if (t > 1) {
                if ((lua_type(L, 2) == LUA_TNUMBER)) {
                    width(n) = (halfword) lua_roundnumber(L, 2);
                } else {
                    /* leave as is */
                }
                if (t > 2) {
                    if ((lua_type(L, 3) == LUA_TNUMBER)) {
                        height(n) = (halfword) lua_roundnumber(L, 3);
                    } else {
                        /* leave as is */
                    }
                    if (t > 3) {
                        if ((lua_type(L, 4) == LUA_TNUMBER)) {
                            depth(n) = (halfword) lua_roundnumber(L, 4);
                        } else {
                            /* leave as is */
                        }
                    }
                }
            }
        }
    }
    return 0;
}

    /* node.getwhd */

    static int lua_nodelib_getwhd(lua_State * L)
    {
        halfword *n = lua_touserdata(L, 1);
        if (n != NULL) {
            halfword t = type(*n);
            if ((t == hlist_node) || (t == vlist_node) || (t == rule_node) || (t == unset_node)) {
                push_list_whd(*n);
                return 3;
            } else if (t == glyph_node) {
                if (lua_toboolean(L,2)) {
                    push_char_ehd(*n);
                    return 4;
                } else {
                    push_char_whd(*n);
                    return 3;
                }
            } else if (t == glue_node) {
                halfword l = leader_ptr(*n);
                if (l != null) {
                    t = type(l) ;
                    if ((t == hlist_node) || (t == vlist_node) || (t == rule_node)) {
                        push_list_whd(l);
                        return 3;
                    }
                }
            }
        }
        return 0;
    }

/* node.direct.getlist */

static int lua_nodelib_direct_getlist(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n == null) {
        lua_pushnil(L);
    } else {
        halfword t = type(n) ;
        if ((t == hlist_node) || (t == vlist_node)) {
            nodelib_pushdirect_or_nil_alink(list_ptr(n));
        } else if ((t == sub_box_node) || (t == sub_mlist_node)) {
            nodelib_pushdirect_or_nil_alink(math_list(n));
        } else if (t == ins_node) {
            nodelib_pushdirect_or_nil_alink(ins_ptr(n));
        } else if (t == adjust_node) {
            nodelib_pushdirect_or_nil_alink(adjust_ptr(n));
        } else {
            lua_pushnil(L);
        }
    }
    return 1;
}

static int lua_nodelib_direct_setlist(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n != null) {
        halfword t = type(n) ;
        if ((t == hlist_node) || (t == vlist_node)) {
            if (lua_type(L,2) == LUA_TNUMBER) {
                list_ptr(n) = (halfword) lua_tointeger(L, 2);
            } else {
                list_ptr(n) = null;
            }
        } else if ((t == sub_box_node) || (t == sub_mlist_node)) {
            if (lua_type(L,2) == LUA_TNUMBER) {
                math_list(n) = (halfword) lua_tointeger(L, 2);
            } else {
                math_list(n) = null;
            }
        } else if (t == ins_node) {
            if (lua_type(L,2) == LUA_TNUMBER) {
                ins_ptr(n) = (halfword) lua_tointeger(L, 2);
            } else {
                ins_ptr(n) = null;
            }
        } else if (t == adjust_node) {
            if (lua_type(L,2) == LUA_TNUMBER) {
                adjust_ptr(n) = (halfword) lua_tointeger(L, 2);
            } else {
                adjust_ptr(n) = null;
            }
        }
    }
    return 0;
}

    /* node.getlist */

    static int lua_nodelib_getlist(lua_State * L)
    {
        halfword *a;
        halfword *n = lua_touserdata(L, 1);
        if ((n == NULL) || (! lua_getmetatable(L,1))) {
            lua_pushnil(L);
        } else if ((type(*n) == hlist_node) || (type(*n) == vlist_node)) {
            fast_metatable_or_nil_alink(list_ptr(*n));
        } else if ((type(*n) == sub_box_node) || (type(*n) == sub_mlist_node)) {
            fast_metatable_or_nil_alink(math_list(*n));
        } else if (type(*n) == ins_node) {
            fast_metatable_or_nil_alink(ins_ptr(*n));
        } else if (type(*n) == adjust_node) {
            fast_metatable_or_nil_alink(adjust_ptr(*n));
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

/* node.direct.getleader */
/* node.direct.setleader */

static int lua_nodelib_direct_getleader(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if ((n) && (type(n) == glue_node)) {
        nodelib_pushdirect_or_nil(leader_ptr(n));
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_setleader(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if ((n) && (type(n) == glue_node)) {
        if (lua_type(L,2) == LUA_TNUMBER) {
            leader_ptr(n) = (halfword) lua_tointeger(L, 2);
        } else {
            leader_ptr(n) = null;
        }
    }
    return 0;
}

/* node.getleader */

    static int lua_nodelib_getleader(lua_State * L)
    {
        halfword *a;
        halfword *n = lua_touserdata(L, 1);
        if ((n == NULL) || (! lua_getmetatable(L,1)) ) {
            lua_pushnil(L);
        } else if (type(*n) == glue_node) {
            fast_metatable_or_nil(leader_ptr(*n));
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

/* node.direct.getdata */
/* node.direct.setdata */

#define get_user_node_direct_value(L, n) do {                          \
    switch (user_node_type(n)) {                                       \
        case 'a':                                                      \
            nodelib_pushdirect(user_node_value(n));                    \
            break;                                                     \
        case 'd':                                                      \
            lua_pushinteger(L, user_node_value(n));                    \
            break;                                                     \
        case 'l':                                                      \
            if (user_node_value(n) != 0) {                             \
                lua_rawgeti(L, LUA_REGISTRYINDEX, user_node_value(n)); \
            } else {                                                   \
                lua_pushnil(L);                                        \
            }                                                          \
            break;                                                     \
        case 'n':                                                      \
            nodelib_pushdirect(user_node_value(n));                    \
            break;                                                     \
        case 's':                                                      \
            nodelib_pushstring(L, user_node_value(n));                 \
            break;                                                     \
        case 't':                                                      \
            tokenlist_to_lua(L, user_node_value(n));                   \
            break;                                                     \
        default:                                                       \
            lua_pushinteger(L, user_node_value(n));                    \
            break;                                                     \
    }                                                                  \
} while (0)

#define set_user_node_direct_value(L,n,i) do {                       \
    switch (user_node_type(n)) {                                     \
        case 'a':                                                    \
            user_node_value(n) = nodelib_getlist(L, i);              \
            break;                                                   \
        case 'd':                                                    \
            user_node_value(n) = (halfword) lua_roundnumber(L, i);   \
            break;                                                   \
        case 'l':                                                    \
            lua_pushvalue(L, i);                                     \
            if (user_node_value(n) != 0) {                           \
                luaL_unref(L, LUA_REGISTRYINDEX,user_node_value(n)); \
            }                                                        \
            user_node_value(n) = luaL_ref(L, LUA_REGISTRYINDEX);     \
            break;                                                   \
        case 'n':                                                    \
            user_node_value(n) = nodelib_getlist(L, i);              \
            break;                                                   \
        case 's':                                                    \
            user_node_value(n) = nodelib_getstring(L, i);            \
            break;                                                   \
        case 't':                                                    \
            user_node_value(n) = nodelib_gettoks(L, i);              \
            break;                                                   \
        default:                                                     \
            user_node_value(n) = (halfword) lua_roundnumber(L, i);   \
            break;                                                   \
    }                                                                \
} while (0)

#define get_pdf_literal_direct_value(L,n) do {                  \
    if (pdf_literal_type(n) == lua_refid_literal) {             \
        lua_rawgeti(L, LUA_REGISTRYINDEX, pdf_literal_data(n)); \
    } else if (pdf_literal_type(n) == lua_refid_literal) {      \
        tokenlist_to_luastring(L, pdf_literal_data(n));         \
    }                                                           \
} while (0)

#define set_pdf_literal_direct_normal(L,n,i) do {             \
    if (ini_version) {                                        \
        pdf_literal_data(n) = nodelib_gettoks(L, i);          \
        pdf_literal_type(n) = normal;                         \
    } else {                                                  \
        lua_pushvalue(L, i);                                  \
        pdf_literal_data(n) = luaL_ref(L, LUA_REGISTRYINDEX); \
        pdf_literal_type(n) = lua_refid_literal;              \
    }                                                         \
} while (0)

#define set_pdf_literal_direct_token(L,n,i) do { \
    pdf_literal_data(n) = nodelib_gettoks(L, i); \
} while (0)

#define cleanup_late_lua(n) do {                               \
    if (late_lua_data(n) != 0) {                               \
        if (late_lua_type(n) == normal) {                      \
            delete_token_ref(late_lua_data(n));                \
        } else if (late_lua_type(n) == lua_refid_literal) {    \
            luaL_unref(L, LUA_REGISTRYINDEX,late_lua_data(n)); \
        }                                                      \
    }                                                          \
} while (0)

#define cleanup_late_lua_name(n) do {       \
    if (late_lua_name(n) != 0) {            \
        delete_token_ref(late_lua_name(n)); \
    }                                       \
} while (0)

#define set_late_lua_direct_normal(L,n,i) do {             \
    cleanup_late_lua(n) ;                                  \
    if (ini_version) {                                     \
        late_lua_data(n) = nodelib_gettoks(L, i);          \
        late_lua_type(n) = normal;                         \
    } else if (lua_type(L, i) == LUA_TNUMBER) {            \
        late_lua_data(n) = lua_tointeger(L,i);             \
        late_lua_type(n) = lua_refid_call;                 \
    } else {                                               \
        lua_pushvalue(L, i);                               \
        late_lua_data(n) = luaL_ref(L, LUA_REGISTRYINDEX); \
        late_lua_type(n) = lua_refid_literal;              \
    }                                                      \
} while (0)

#define set_late_lua_direct_token(L,n,i) do { \
    cleanup_late_lua(n) ;                     \
    late_lua_data(n) = nodelib_gettoks(L, i); \
    late_lua_type(n) = normal;                \
} while (0)

#define get_late_lua_direct_value(L,n) do {                  \
    if (late_lua_type(n) == lua_refid_literal) {             \
        lua_rawgeti(L, LUA_REGISTRYINDEX, late_lua_data(n)); \
    } else if (late_lua_type(n) == lua_refid_call) {         \
        lua_pushinteger(L, late_lua_data(n));                \
    } else if (late_lua_type(n) == normal) {                 \
        tokenlist_to_luastring(L, late_lua_data(n));         \
    }                                                        \
} while (0)

#define set_special_direct_value(L,n,i) do { \
    write_tokens(n) = nodelib_gettoks(L, i); \
} while (0)

#define get_special_direct_value(L,n) do {     \
    tokenlist_to_luastring(L, write_tokens(n)); \
} while (0)

#define set_write_direct_value(L,n,i) do {   \
    write_tokens(n) = nodelib_gettoks(L, i); \
} while (0)

#define xget_write_direct_value(L,n) do {  \
    tokenlist_to_lua(L, write_tokens(n)); \
} while (0)

#define get_write_direct_value(L,n) do {  \
    int l; \
    char *s; \
    expand_macros_in_tokenlist(n); \
    s = tokenlist_to_cstring(def_ref, 1, &l); \
    lua_pushlstring(L, s, (size_t) l); \
    free(s); \
    flush_list(def_ref); \
} while (0)

#define set_pdf_setmatrix_direct_value(L,n,i) do { \
    pdf_setmatrix_data(n) = nodelib_gettoks(L, i);  \
} while (0)

#define get_pdf_setmatrix_direct_value(L,n) do {     \
    tokenlist_to_luastring(L, pdf_setmatrix_data(n)); \
} while (0)

/*tex

    These getter and setter get |data| as well as |value| fields. One can
    make them equivalent to |getvalue| and |setvalue| if needed.

*/

static int lua_nodelib_direct_getdata(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n == null) {
        lua_pushnil(L);
    } else {
        halfword t = type(n) ;
        if (t == glyph_node) {
            lua_pushinteger(L,glyph_node_data(n));
        } else if (t == boundary_node) {
            lua_pushinteger(L,boundary_value(n));
        } else if (t == whatsit_node) {
            halfword s = subtype(n);
            if (s == user_defined_node) {
                get_user_node_direct_value(L, n);
            } else if (s == pdf_literal_node) {
                get_pdf_literal_direct_value(L, n);
                /*tex A bonus. */
                lua_pushinteger(L,pdf_literal_mode(n));
                return 2;
            } else if (s == late_lua_node) {
                get_late_lua_direct_value(L, n);
            } else if (s == pdf_setmatrix_node) {
                get_pdf_setmatrix_direct_value(L, n);
            } else if (s == special_node) {
                get_special_direct_value(L, n);
            } else if (s == write_node) {
                get_write_direct_value(L, n);
            } else {
                lua_pushnil(L);
            }
        } else {
            lua_pushnil(L);
        }
    }
    return 1;
}

static int lua_nodelib_direct_setdata(lua_State * L) /* data and value */
{
    halfword n = lua_tointeger(L, 1);
    if (n != null) {
        halfword t = type(n) ;
        if (t == glyph_node) {
            glyph_node_data(n) = lua_tointeger(L,2);
        } else if (t == boundary_node) {
            boundary_value(n) = lua_tointeger(L,2);
        } else if (t == whatsit_node) {
            halfword s = subtype(n);
            if (s == user_defined_node) {
                set_user_node_direct_value(L, n, 2);
            } else if (s == pdf_literal_node) {
                set_pdf_literal_direct_normal(L, n, 2);
                if (lua_type(L,2) == LUA_TNUMBER) {
                    /*tex A bonus. */
                    pdf_literal_mode(n) = lua_tointeger(L,2);
                }
            } else if (s == late_lua_node) {
                set_late_lua_direct_normal(L, n, 2);
            } else if (s == pdf_setmatrix_node) {
                set_pdf_setmatrix_direct_value(L, n, 2);
            } else if (s == special_node) {
                set_special_direct_value(L, n, 2);
            } else if (s == write_node) {
                set_write_direct_value(L,n,2);
            }
        }
    }
    return 0;
}

/* node.direct.getnext */
/* node.direct.setnext */

static int lua_nodelib_direct_getnext(lua_State * L)
{
    halfword p = lua_tointeger(L, 1);
    if (p == null) {
        lua_pushnil(L);
    } else {
        nodelib_pushdirect_or_nil(vlink(p));
    }
    return 1;
}

static int lua_nodelib_direct_setnext(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        if (lua_type(L, 2) == LUA_TNUMBER) {
            vlink(n) = (halfword) lua_tointeger(L, 2);
        } else {
            vlink(n) = null;
        }
    }
    return 0;
}

    /* node.getnext */

    static int lua_nodelib_getnext(lua_State * L)
    {
        halfword *a;
        /* [given-node] [...]*/
        halfword *p = lua_touserdata(L, 1);
        if ( (p == NULL) || (! lua_getmetatable(L,1)) ) {
            lua_pushnil(L);
        } else {
            /* [given-node] [mt-given-node]*/
            lua_get_metatablelua(luatex_node);
            /* [given-node] [mt-given-node] [mt-node]*/
            if (!lua_rawequal(L, -1, -2)) {
                lua_pushnil(L);
            } else {
                fast_metatable_or_nil(vlink(*p));
            }
        }
        return 1; /* just one*/
    }

/* node.direct.getprev */
/* node.direct.setprev */

static int lua_nodelib_direct_getprev(lua_State * L)
{
    halfword p = lua_tointeger(L, 1);
    if (p == null) {
        lua_pushnil(L);
    } else {
        nodelib_pushdirect_or_nil(alink(p));
    }
    return 1;
}

static int lua_nodelib_direct_setprev(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        if (lua_type(L, 2) == LUA_TNUMBER) {
            alink(n) = (halfword) lua_tointeger(L, 2);
        } else {
            alink(n) = null;
        }
    }
    return 0;
}

    /* node.getprev */

    static int lua_nodelib_getprev(lua_State * L)
    {
        halfword *a;
        halfword *p = lua_touserdata(L, 1);
        if ( (p == NULL) || (! lua_getmetatable(L,1)) ) {
            lua_pushnil(L);
        } else {
            lua_get_metatablelua(luatex_node);
            if (!lua_rawequal(L, -1, -2)) {
                lua_pushnil(L);
            } else {
                fast_metatable_or_nil(alink(*p));
            }
        }
        return 1;
    }

/* node.direct.getboth */
/* node.direct.setboth */

static int lua_nodelib_direct_getboth(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        nodelib_pushdirect_or_nil(alink(n));
        nodelib_pushdirect_or_nil(vlink(n));
    } else {
        lua_pushnil(L);
        lua_pushnil(L);
    }
    return 2;
}

static int lua_nodelib_direct_setboth(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        if (lua_type(L, 2) == LUA_TNUMBER) {
            alink(n) = (halfword) lua_tointeger(L, 2);
        } else {
            alink(n) = null;
        }
        if (lua_type(L, 3) == LUA_TNUMBER) {
            vlink(n) = (halfword) lua_tointeger(L, 3);
        } else {
            vlink(n) = null;
        }
    }
    return 0;
}

    /* node.getboth */

    static int lua_nodelib_getboth(lua_State * L)
    {
        halfword *a;
        halfword *p = lua_touserdata(L, 1);
        if ( (p == NULL) || (! lua_getmetatable(L,1)) ) {
            lua_pushnil(L);
            lua_pushnil(L);
        } else {
            lua_get_metatablelua(luatex_node);
            if (lua_rawequal(L, -1, -2)) {
                fast_metatable_or_nil(alink(*p));
                fast_metatable_or_nil(vlink(*p));
            } else {
                lua_pushnil(L);
                lua_pushnil(L);
            }
        }
        return 2;
    }

/* node.direct.setlink */
/* node.direct.setsplit  */

/*
    a b b nil c d         : prev-a-b-c-next
    nil a b b nil c d nil : nil-a-b-c-nil
*/

static int lua_nodelib_direct_setlink(lua_State * L)
{
    int n = lua_gettop(L);
    int i;
    halfword h = null; /* head node */
    halfword t = null; /* tail node */
    halfword c = null; /* current node */
    for (i=1;i<=n;i++) {
        /*
            we don't go for the tail of the current node because we can inject
            between existing nodes and the nodes themselves can have old values
            for prev and next, so ... only single nodes are looked at!
        */
        if (lua_type(L, i) == LUA_TNUMBER) {
            c = lua_tointeger(L, i);
            if (c != t) {
                if (t != null) {
                    vlink(t) = c;
                    alink(c) = t;
                } else if (i > 1) {
                    /* we assume that the first node is a kind of head */
                    alink(c) = null;
                }
                t = c;
                if (h == null) {
                    h = t;
                }
            } else {
                /* we ignore duplicate nodes which can be tails or the previous */
            }
        } else if (t == null) {
            /* we just ignore nil nodes and have no tail yet */
        } else {
            /* safeguard: a nil in the list can be meant as end so we nil the next of tail */
            vlink(t) = null;
        }
    }
    if (h == null) {
        /* no head */
        lua_pushnil(L);
    } else {
        /* first valid head */
        lua_pushinteger(L,h);
    }
    return 1;
}

static int lua_nodelib_direct_setsplit(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER && (lua_type(L, 2) == LUA_TNUMBER)) {
        halfword l = lua_tointeger(L, 1);
        halfword r = lua_tointeger(L, 2);
        if (l != r) {
            alink(vlink(l)) = null;
            vlink(alink(r)) = null;
        }
        vlink(l) = null;
        alink(r) = null;
    }
    return 0;
}

/* node.subtype (maybe also return them for other node types now) */

static int lua_nodelib_subtype(lua_State * L)
{
    if (lua_type(L,1) == LUA_TSTRING) {
        int i = get_node_type_id_from_name(L,1,whatsit_node_data);
        if (i >= 0) {
            lua_pushinteger(L, i);
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* node.type (converts id numbers to type names) */

static int lua_nodelib_type(lua_State * L)
{
    if (lua_type(L,1) == LUA_TNUMBER) {
        int i = lua_tointeger(L, 1);
        if (known_node_type(i)) {
         /* lua_pushstring(L, node_data[i].name); */
            lua_push_string_by_index(L, node_data[i].lua);
            return 1;
        }
    } else if (maybe_isnode(L, 1) != NULL) {
     /* lua_pushstring(L,"node"); */
        lua_push_string_by_name(L,node);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

/* node.new (allocate a new node) */

static halfword lua_nodelib_new_node(lua_State * L)
{
    int i, j;
    int t = lua_type(L, 1);
    if (t == LUA_TNUMBER) {
        i = lua_tointeger(L,1);
        if (! known_node_type(i)) {
            i = -1;
        }
    } else if (t == LUA_TSTRING) {
        i = get_node_type_id_from_name(L,1,node_data);
    } else {
        i = -1;
    }
    if (i < 0) {
        luaL_error(L, "invalid node id for creating new node");
    }
    t = lua_type(L, 2);
    if (i == whatsit_node) {
        if (t == LUA_TNUMBER) {
            j = lua_tointeger(L,2);
            if (! known_whatsit_type(j)) {
                j = -1;
            }
        } else if (t == LUA_TSTRING) {
            j = get_node_type_id_from_name(L,2,whatsit_node_data);
        } else {
            j = -1;
        }
        if (j < 0) {
            luaL_error(L, "creating a whatsit requires the subtype number as a second argument");
        }
    } else if (t == LUA_TNUMBER) {
        j = (int) lua_tointeger(L, 2);
    } else if (t == LUA_TSTRING) {
        j = get_node_subtype_id_from_name(L,2,node_data[i].subtypes);
    } else {
        j = 0;
    }
    return new_node(i, j);
}

static int lua_nodelib_new(lua_State * L)
{
    halfword n = lua_nodelib_new_node(L);
    lua_nodelib_push_fast(L, n);
    return 1;
}

/* node.direct.new */

static int lua_nodelib_direct_new(lua_State * L)
{
    halfword n = lua_nodelib_new_node(L);
    lua_pushinteger(L,n);
    return 1;
}

/* node.free (this function returns the 'next' node, because that may be helpful) */

static int lua_nodelib_free(lua_State * L)
{
    halfword n;
    halfword p;
    if (lua_gettop(L) < 1) {
        lua_pushnil(L);
        return 1;
    } else if (lua_isnil(L, 1)) {
        return 1;               /* the nil itself */
    }
    n = *(check_isnode(L, 1));
    p = vlink(n);
    flush_node(n);
    /* can be: lua_nodelib_push_fast(L, p); */
    lua_pushinteger(L, p);
    lua_nodelib_push(L);
    return 1;
}

/* node.direct.free */

static int lua_nodelib_direct_free(lua_State * L)
{
    halfword n = lua_tointeger(L,1);
    if (n == null) {
        lua_pushnil(L);
    } else {
        halfword p = vlink(n);
        flush_node(n);
        if (p == 0) {
            lua_pushnil(L);
        } else {
            lua_pushinteger(L,p);
        }
    }
    return 1;
}

/* node.flush_node (no next returned) */

static int lua_nodelib_flush_node(lua_State * L)
{
    if ((lua_gettop(L) < 1) || lua_isnil(L, 1)) {
        return 0;
    } else {
        halfword n = *(check_isnode(L, 1));
        flush_node(n);
        return 0;
    }
}

/* node.direct.flush_node */

static int lua_nodelib_direct_flush_node(lua_State * L)
{
    halfword n = lua_tointeger(L,1);
    if (n == null)
        return 0;
    flush_node(n);
    return 0;
}

/* node.flush_list */

static int lua_nodelib_flush_list(lua_State * L)
{
    if ((lua_gettop(L) < 1) || lua_isnil(L, 1)) {
        return 0;
    } else {
        halfword n_ptr = *check_isnode(L, 1);
        flush_node_list(n_ptr);
        return 0;
    }
}

/* node.direct.flush_list */

static int lua_nodelib_direct_flush_list(lua_State * L)
{
    halfword n = lua_tointeger(L,1);
    if (n == null)
        return 0;
    flush_node_list(n);
    return 0;
}

/* remove a node from a list */

#if DEBUG

    static void show_node_links (halfword l, const char * p)
    {
        halfword t = l;
        while (t) {
            fprintf(DEBUG_OUT, "%s t = %d, prev = %d, next = %d\n", p, (int)t, (int)alink(t), (int)vlink(t));
            t = vlink(t);
        }
    }

#endif

/* node.remove */

static int lua_nodelib_remove(lua_State * L)
{
    halfword head, current, t;
    if (lua_gettop(L) < 2)
        luaL_error(L, "Not enough arguments for node.remove()");
    head = *(check_isnode(L, 1));
    if (lua_isnil(L, 2))
        return 2;               /* the arguments, as they are */
    current = *(check_isnode(L, 2));
    if (head == current) {
      if (alink(current)){
        vlink(alink(current)) = vlink(current);
      }
      if (vlink(current)){
        alink( vlink(current)) = alink(current);
      }

      head = vlink(current);
      current = vlink(current);
    } else {
        t = alink(current);
        if (t == null || vlink(t) != current) {
            set_t_to_prev(head, current);
            if (t == null)     /* error! */
                luaL_error(L,"Attempt to node.remove() a non-existing node");
        }
        /* t is now the previous node */
        vlink(t) = vlink(current);
        if (vlink(current) != null)
            alink(vlink(current)) = t;
        current = vlink(current);
    }
    /* can be: lua_nodelib_push_fast(L, head); */
    lua_pushinteger(L, head);
    lua_nodelib_push(L);
    /* can be: lua_nodelib_push_fast(L, current); */
    lua_pushinteger(L, current);
    lua_nodelib_push(L);
    return 2;
}

/* node.direct.remove */

static int lua_nodelib_direct_remove(lua_State * L)
{
    halfword current, t;
    halfword head = lua_tointeger(L,1);
    if (head == null) {
        lua_pushnil(L);
        lua_pushnil(L);
        return 2 ;
    }
    current = (halfword) lua_tointeger(L,2);
    if (current == null) {
        lua_pushinteger(L, head);
        lua_pushnil(L);
        return 2 ;
    }
    if (head == current) {
      if (alink(current)){
        vlink( alink(current) ) = vlink(current);
      }
      if (vlink(current)){
        alink( vlink(current) ) = alink(current);
      }
      head = vlink(current);
      current = vlink(current);
    } else {
        t = alink(current);
        if (t == null || vlink(t) != current) {
            set_t_to_prev(head, current);
            if (t == null) {
                luaL_error(L,"Attempt to node.direct.remove() a non-existing node");
            }
        }
        vlink(t) = vlink(current);
        if (vlink(current) != null)
            alink(vlink(current)) = t;
        current = vlink(current);
    }
    if (head == null) {
        lua_pushnil(L);
    } else {
        lua_pushinteger(L, head);
    }
    if (current == null) {
        lua_pushnil(L);
    } else {
        lua_pushinteger(L, current);
    }
    return 2;
}

/* node.insert_before (insert a node in a list) */

static int lua_nodelib_insert_before(lua_State * L)
{
    halfword head, current, n, t;
    if (lua_gettop(L) < 3) {
        luaL_error(L, "Not enough arguments for node.insert_before()");
    }
    if (lua_isnil(L, 3)) {
        lua_pop(L, 1);
        return 2;
    } else {
        n = *(check_isnode(L, 3));
    }
    if (lua_isnil(L, 1)) {      /* no head */
        vlink(n) = null;
        alink(n) = null;
        lua_nodelib_push_fast(L, n);
        lua_pushvalue(L, -1);
        return 2;
    } else {
        head = *(check_isnode(L, 1));
    }
    if (lua_isnil(L, 2)) {
        current = tail_of_list(head);
    } else {
        current = *(check_isnode(L, 2));
    }
    if (head != current) {
        t = alink(current);
        if (t == null || vlink(t) != current) {
            set_t_to_prev(head, current);
            if (t == null) {    /* error! */
                luaL_error(L, "Attempt to node.insert_before() a non-existing node");
            }
        }
        couple_nodes(t, n);
    }
    couple_nodes(n, current);
    if (head == current) {
        lua_nodelib_push_fast(L, n);
    } else {
        lua_nodelib_push_fast(L, head);
    }
    lua_nodelib_push_fast(L, n);
    return 2;
}

/* node.direct.insert_before */

static int lua_nodelib_direct_insert_before(lua_State * L)
{
    halfword head, current;
    halfword n = lua_tointeger(L,3);
    if (n == null){
        /* no node */
        lua_pop(L, 1);
        return 2 ;
    }
    head = (halfword) lua_tointeger(L,1);
    current = (halfword) lua_tointeger(L,2);
    /* no head, ignore current */
    if (head == null) {
        vlink(n) = null;
        alink(n) = null;
        lua_pushinteger(L, n);
        lua_pushvalue(L, -1);
        /* n, n */
        return 2;
    }
    /* no current */
    if (current == null)
        current = tail_of_list(head);
    if (head != current) {
        halfword t = alink(current);
        if (t == null || vlink(t) != current) {
            set_t_to_prev(head, current);
        }
        couple_nodes(t, n);
    }
    couple_nodes(n, current); /*  nice but incompatible: couple_nodes(tail_of_list(n),current) */
    if (head == current) {
        lua_pushinteger(L, n);
    } else {
        lua_pushinteger(L, head);
    }
    lua_pushinteger(L, n);
    return 2;
}

/* node.insert_after */

static int lua_nodelib_insert_after(lua_State * L)
{
    halfword head, current, n;
    if (lua_gettop(L) < 3) {
        luaL_error(L, "Not enough arguments for node.insert_after()");
    }
    if (lua_isnil(L, 3)) {
        lua_pop(L, 1);
        return 2;
    } else {
        n = *(check_isnode(L, 3));
    }
    if (lua_isnil(L, 1)) {      /* no head */
        vlink(n) = null;
        alink(n) = null;
        lua_nodelib_push_fast(L, n);
        lua_pushvalue(L, -1);
        return 2;
    } else {
        head = *(check_isnode(L, 1));
    }
    if (lua_isnil(L, 2)) {
        current = head;
        while (vlink(current) != null)
            current = vlink(current);
    } else {
        current = *(check_isnode(L, 2));
    }
    try_couple_nodes(n, vlink(current));
    couple_nodes(current, n);

    lua_pop(L, 2);
    lua_nodelib_push_fast(L, n);
    return 2;
}

/* node.direct.insert_after */

static int lua_nodelib_direct_insert_after(lua_State * L)
{
    halfword head, current;
    /*[head][current][new]*/
    halfword n = lua_tointeger(L,3);
    if (n == null) {
        /* no node */
        return 2 ;
    }
    head = (halfword) lua_tointeger(L,1);
    current = (halfword) lua_tointeger(L,2);
    if (head == null) {
        /* no head, ignore current */
        vlink(n) = null;
        alink(n) = null;
        lua_pushinteger(L,n);
        lua_pushvalue(L, -1);
        /* n, n */
        return 2;
    }
    if (current == null) {
        /* no current */
        current = head;
        while (vlink(current) != null)
            current = vlink(current);
    }
    try_couple_nodes(n, vlink(current)); /* nice but incompatible: try_couple_nodes(tail_of_list(n), vlink(current)); */
    couple_nodes(current, n);
    lua_pop(L, 2);
    lua_pushinteger(L, n);
    return 2;
}

/* node.copy_list */

/*
    we need to use an intermediate variable as otherwise target is used in the loop
    and subfields get overwritten (or something like that) which results in crashes
    and unexpected side effects
*/

static int lua_nodelib_copy_list(lua_State * L)
{
    halfword n, s = null;
    halfword m;
    if (lua_isnil(L, 1))
        return 1;               /* the nil itself */
    n = *check_isnode(L, 1);
    if ((lua_gettop(L) > 1) && (!lua_isnil(L,2)))
        s = *check_isnode(L, 2);
    m = do_copy_node_list(n, s);
    lua_nodelib_push_fast(L,m);
    return 1;
}

/* node.direct.copy_list */

static int lua_nodelib_direct_copy_list(lua_State * L)
{
    halfword n = lua_tointeger(L,1);
    if (n == null) {
        lua_pushnil(L);
    } else {
        halfword s = lua_tointeger(L,2);
        halfword m;
        if (s == null) {
            m = do_copy_node_list(n,null);
        } else {
            m = do_copy_node_list(n,s);
        }
        lua_pushinteger(L,m);
    }
    return 1;
}

/* node.copy (deep copy) */

static int lua_nodelib_copy(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 1))
        return 1;               /* the nil itself */
    n = *check_isnode(L, 1);
    n = copy_node(n);
    lua_nodelib_push_fast(L, n);
    return 1;
}

/* node.direct.copy (deep copy) */

static int lua_nodelib_direct_copy(lua_State * L)
{
    if (lua_isnil(L, 1)) {
        return 1;               /* the nil itself */
    } else {
        /* beware, a glue node can have number 0 (zeropt) so we cannot test for null) */
        halfword n = lua_tointeger(L, 1);
        n = copy_node(n);
        lua_pushinteger(L, n);
        return 1;
    }
}


/* node.write (output a node to tex's processor) */

static int lua_nodelib_append(lua_State * L)
{
    halfword n;
    int i;
    int j = lua_gettop(L);
    for (i = 1; i <= j; i++) {
        n = *check_isnode(L, i);
        tail_append(n);
        if (nodetype_has_attributes(type(n)) && node_attr(n) == null) {
            build_attribute_list(n);
        }
        while (vlink(n) != null) {
            n = vlink(n);
            tail_append(n);
            if (nodetype_has_attributes(type(n)) && node_attr(n) == null) {
                build_attribute_list(n);
            }
        }
    }
    return 0;
}

/* node.direct.write */

static int lua_nodelib_direct_append(lua_State * L)
{
    halfword m;
    int i;
    int j = lua_gettop(L);
    for (i = 1; i <= j; i++) {
        halfword n = lua_tointeger(L,i); /*lua_getnumber(L, i);*/
        if (n != null) {
            m = n ;
            tail_append(m);
            if (nodetype_has_attributes(type(n)) && node_attr(n) == null) {
                build_attribute_list(n);
            }
            while (vlink(m) != null) {
                m = vlink(m);
                tail_append(m);
                if (nodetype_has_attributes(type(n)) && node_attr(n) == null) {
                    build_attribute_list(n);
                }
            }
        }
    }
    return 0;
}

/* node.last */

static int lua_nodelib_last_node(lua_State * L)
{
    halfword m = pop_tail();
    /* can be: lua_nodelib_push_fast(L, m); */
    lua_pushinteger(L, m);
    lua_nodelib_push(L);
    return 1;
}

/* node.direct.last */

static int lua_nodelib_direct_last_node(lua_State * L)
{
    halfword m = pop_tail();
    lua_pushinteger(L, m);
    return 1;
}

/* node.hpack (build a hbox) */

static int lua_nodelib_hpack(lua_State * L)
{
    halfword p;
    const char *s;
    int w = 0;
    int m = 1;
    int d = -1;
    halfword n = *(check_isnode(L, 1));
    if (lua_gettop(L) > 1) {
        w = lua_roundnumber(L, 2);
        if (lua_gettop(L) > 2) {
            if (lua_type(L, 3) == LUA_TSTRING) {
                s = lua_tostring(L, 3);
                if (lua_key_eq(s, exactly)) {
                    m = 0;
                } else if (lua_key_eq(s, additional)) {
                    m = 1;
                } else if (lua_key_eq(s, cal_expand_ratio)) {
                    m = 2;
                } else if (lua_key_eq(s, subst_ex_font)) {
                    m = 3;
                }
            } else if (lua_type(L, 3) == LUA_TNUMBER) {
                m = (int) lua_tointeger(L, 3);
            }
            if ((m<0) || (m>3)) {
                luaL_error(L, "wrong mode in hpack");
            }
            if (lua_gettop(L) > 3) {
                if (lua_type(L, 4) == LUA_TNUMBER) {
                    d = nodelib_getdirection(L, 4);
                } else if (lua_type(L, 4) == LUA_TSTRING) {
                    d = nodelib_getdir_par(L, 4);
                } else {
                    lua_pushstring(L, "incorrect 4th argument");
                }
            }
        }
    }
    p = hpack(n, w, m, d);
    lua_nodelib_push_fast(L, p);
    lua_pushinteger(L, last_badness);
    return 2;
}

/* node.direct.hpack */

static int lua_nodelib_direct_hpack(lua_State * L)
{
    halfword p;
    const char *s;
    int w = 0;
    int m = 1;
    int d = -1;
    halfword n = lua_tointeger(L,1);
    /* could be macro */
    if (lua_gettop(L) > 1) {
        w = lua_roundnumber(L, 2);
        if (lua_gettop(L) > 2) {
            if (lua_type(L, 3) == LUA_TSTRING) {
                s = lua_tostring(L, 3);
                if (lua_key_eq(s, additional)) {
                    m = 1;
                } else if (lua_key_eq(s, exactly)) {
                    m = 0;
                } else if (lua_key_eq(s, cal_expand_ratio)) {
                    m = 2;
                } else if (lua_key_eq(s, subst_ex_font)) {
                    m = 3;
                } else {
                    luaL_error(L, "3rd argument should be either additional or exactly");
                }
            } else if (lua_type(L, 3) == LUA_TNUMBER) {
                m = (int) lua_tointeger(L, 3);
            } else {
                lua_pushstring(L, "incorrect 3rd argument");
            }
            if (lua_gettop(L) > 3) {
                if (lua_type(L, 4) == LUA_TNUMBER) {
                    d = nodelib_getdirection(L, 4);
                } else if (lua_type(L, 4) == LUA_TSTRING) {
                    d = nodelib_getdir_par(L, 4);
                } else {
                    lua_pushstring(L, "incorrect 4th argument");
                }
            }
        }
    }
    /* till here */
    p = hpack(n, w, m, d);
    lua_pushinteger(L, p);
    lua_pushinteger(L, last_badness);
    return 2;
}

/* node.vpack (build a vbox) */

static int lua_nodelib_vpack(lua_State * L)
{
    halfword p;
    const char *s;
    int w = 0;
    int m = 1;
    int d = -1;
    halfword n = *(check_isnode(L, 1));
    if (lua_gettop(L) > 1) {
        w = lua_roundnumber(L, 2);
        if (lua_gettop(L) > 2) {
            if (lua_type(L, 3) == LUA_TSTRING) {
                s = lua_tostring(L, 3);
                if (lua_key_eq(s, additional)) {
                    m = 1;
                } else if (lua_key_eq(s, exactly)) {
                    m = 0;
                } else {
                    luaL_error(L, "3rd argument should be either additional or exactly");
                }

                if (lua_gettop(L) > 3) {
                    if (lua_type(L, 4) == LUA_TNUMBER) {
                        d = nodelib_getdirection(L, 4);
                    } else if (lua_type(L, 4) == LUA_TSTRING) {
                        d = nodelib_getdir_par(L, 4);
                    } else {
                        lua_pushstring(L, "incorrect 4th argument");
                    }
                }
            }

            else if (lua_type(L, 3) == LUA_TNUMBER) {
                m= (int) lua_tointeger(L, 3);
            } else {
                lua_pushstring(L, "incorrect 3rd argument");
            }
        }
    }
    p = vpackage(n, w, m, max_dimen, d);
    lua_nodelib_push_fast(L, p);
    lua_pushinteger(L, last_badness);
    return 2;
}

/* node.direct.vpack */

static int lua_nodelib_direct_vpack(lua_State * L)
{
    halfword p;
    const char *s;
    int w = 0;
    int m = 1;
    int d = -1;
    halfword n = (halfword) lua_tointeger(L,1);
    if (lua_gettop(L) > 1) {
        w = lua_roundnumber(L, 2);
        if (lua_gettop(L) > 2) {
            if (lua_type(L, 3) == LUA_TSTRING) {
                s = lua_tostring(L, 3);
                if (lua_key_eq(s, additional)) {
                    m = 1;
                } else if (lua_key_eq(s, exactly)) {
                    m = 0;
                } else {
                    luaL_error(L, "3rd argument should be either additional or exactly");
                }

                if (lua_gettop(L) > 3) {
                    if (lua_type(L, 4) == LUA_TNUMBER) {
                        d = nodelib_getdirection(L, 4);
                    } else if (lua_type(L, 4) == LUA_TSTRING) {
                        d = nodelib_getdir_par(L, 4);
                    } else {
                        lua_pushstring(L, "incorrect 4th argument");
                    }
                }
            }

            else if (lua_type(L, 3) == LUA_TNUMBER) {
                m= (int) lua_tointeger(L, 3);
            } else {
                lua_pushstring(L, "incorrect 3rd argument");
            }
        }
    }
    p = vpackage(n, w, m, max_dimen, d);
    lua_pushinteger(L, p);
    lua_pushinteger(L, last_badness);
    return 2;
}

/* node.dimensions (of a hlist or vlist) */

static int lua_nodelib_dimensions(lua_State * L)
{
    int top = lua_gettop(L);
    if (top > 0) {
        scaled_whd siz;
        glue_ratio g_mult = 1.0;
        int g_sign = normal;
        int g_order = normal;
        int i = 1;
        int d = -1;
        halfword n = null, p = null;
        if (lua_type(L, 1) == LUA_TNUMBER) {
            if (top < 4) {
                lua_pushnil(L);
                return 1;
            }
            i += 3;
            g_mult = (glue_ratio) lua_tonumber(L, 1); /* integer or float */
            g_sign = (int) lua_tointeger(L, 2);
            g_order = (int) lua_tointeger(L, 3);
        }
        n = *(check_isnode(L, i));
        if (lua_gettop(L) > i && !lua_isnil(L, (i + 1))) {
            if (lua_type(L, (i + 1)) == LUA_TSTRING) {
                d = nodelib_getdir_par(L, (i + 1));
            } else {
                p = *(check_isnode(L, (i + 1)));
            }
        }
        if (lua_gettop(L) > (i + 1)) {
            if (lua_type(L, (i + 2)) == LUA_TNUMBER) {
                d = nodelib_getdirection(L, (i + 2));
            } else if (lua_type(L, (i + 2)) == LUA_TSTRING) {
                d = nodelib_getdir_par(L, (i + 2));
            }
        }
        siz = natural_sizes(n, p, g_mult, g_sign, g_order, d);
        lua_pushinteger(L, siz.wd);
        lua_pushinteger(L, siz.ht);
        lua_pushinteger(L, siz.dp);
        return 3;
    } else {
        luaL_error(L, "missing argument to 'dimensions' (node expected)");
    }
    return 0;                   /* not reached */
}

static int lua_nodelib_rangedimensions(lua_State * L) /* parent, first, last */
{
    int top = lua_gettop(L);
    if (top > 1) {
        scaled_whd siz;
        halfword l = *(check_isnode(L, 1)); /* parent */
        halfword n = *(check_isnode(L, 2)); /* first */
        halfword p = null;
        if (top > 2) {
            p = *(check_isnode(L, 3)); /* last */
        }
        siz = natural_sizes(n, p, (glue_ratio) glue_set(l), glue_sign(l), glue_order(l), box_dir(l));
        lua_pushinteger(L, siz.wd);
        lua_pushinteger(L, siz.ht);
        lua_pushinteger(L, siz.dp);
        return 3;
    } else {
        luaL_error(L, "missing argument to 'rangedimensions' (2 or more nodes expected)");
    }
    return 0;                   /* not reached */
}

/* node.direct.dimensions*/

static int lua_nodelib_direct_dimensions(lua_State * L)
{
    int top = lua_gettop(L);
    if (top > 0) {
        scaled_whd siz;
        glue_ratio g_mult = 1.0;
        int g_sign = normal;
        int g_order = normal;
        int i = 1;
        int d = -1;
        halfword n = null;
        halfword p = null;
        if (top > 3) {
            i += 3;
            g_mult = (glue_ratio) lua_tonumber(L, 1); /* integer or float */
            g_sign = (int) lua_tointeger(L, 2);
            g_order = (int) lua_tointeger(L, 3);
        }
        n = (halfword) lua_tointeger(L,i);
        if (lua_gettop(L) > i && !lua_isnil(L, (i + 1))) {
            if (lua_type(L, (i + 1)) == LUA_TSTRING) {
                d = nodelib_getdir_par(L, (i + 1));
            } else {
                p = (halfword) lua_tointeger(L,i+1);
            }
        }
        if (lua_gettop(L) > (i + 1)) {
            if (lua_type(L, (i + 2)) == LUA_TNUMBER) {
                d = nodelib_getdirection(L, (i + 2));
            } else if (lua_type(L, (i + 2)) == LUA_TSTRING) {
                d = nodelib_getdir_par(L, (i + 2));
            }
        }
        siz = natural_sizes(n, p, g_mult, g_sign, g_order, d);
        lua_pushinteger(L, siz.wd);
        lua_pushinteger(L, siz.ht);
        lua_pushinteger(L, siz.dp);
        return 3;
    } else {
        luaL_error(L, "missing argument to 'dimensions' (direct node expected)");
    }
    return 0;                   /* not reached */
}

static int lua_nodelib_direct_rangedimensions(lua_State * L) /* parent, first, last */
{
    int top = lua_gettop(L);
    if (top > 1) {
        scaled_whd siz;
        halfword l = (halfword) lua_tointeger(L,1); /* parent */
        halfword n = (halfword) lua_tointeger(L,2); /* first */
        halfword p = null;
        if (top > 2) {
            p = (halfword) lua_tointeger(L,3); /* last */
        }
        siz = natural_sizes(n, p, (glue_ratio) glue_set(l), glue_sign(l), glue_order(l), box_dir(l));
        lua_pushinteger(L, siz.wd);
        lua_pushinteger(L, siz.ht);
        lua_pushinteger(L, siz.dp);
        return 3;
    } else {
        luaL_error(L, "missing argument to 'rangedimensions' (2 or more direct nodes expected)");
    }
    return 0;                   /* not reached */
}

/* node.mlist_to_hlist (create a hlist from a formula) */

static int lua_nodelib_mlist_to_hlist(lua_State * L)
{
    int w;
    boolean m;
    halfword n = *(check_isnode(L, 1));
    assign_math_style(L,2,w);
    luaL_checkany(L, 3);
    m = lua_toboolean(L, 3);
    mlist_to_hlist(n, m, w);
    alink(vlink(temp_head)) = null;
    lua_nodelib_push_fast(L, vlink(temp_head));
    return 1;
}

/* node.family_font */

static int lua_nodelib_mfont(lua_State * L)
{
    int s;
    int f = luaL_checkinteger(L, 1);
    if (lua_gettop(L) == 2)
        s = lua_tointeger(L, 2);  /* this should be a multiple of 256 ! */
    else
        s = 0;
    lua_pushinteger(L, fam_fnt(f,s));
    return 1;
}

/*
    This function is similar to |get_node_type_id|, for field
    identifiers.  It has to do some more work, because not all
    identifiers are valid for all types of nodes.

    We can make this faster if needed but when this needs to
    be called often something is wrong with the code.

*/

static int get_node_field_id(lua_State * L, int n, int node)
{
    int t = type(node);
    const char *s = lua_tostring(L, n);

    if (s == NULL)
        return -2;

    if (lua_key_eq(s, next)) {
        return 0;
    } else if (lua_key_eq(s, id)) {
        return 1;
    } else if (lua_key_eq(s, subtype)) {
        if (nodetype_has_subtype(t)) {
            return 2;
        }
    } else if (lua_key_eq(s, attr)) {
        if (nodetype_has_attributes(t)) {
            return 3;
        }
    } else if (lua_key_eq(s, prev)) {
        if (nodetype_has_prev(t)) {
            return -1;
        }
    } else {
        int j;
        field_info *fields ;
        if (t == whatsit_node) {
            fields = whatsit_node_data[subtype(node)].fields;
        } else {
            fields = node_data[t].fields;
        }
        if (lua_key_eq(s, list)) {
            s = lua_key(head);
        }
        if (fields != NULL) {
            for (j = 0; fields[j].lua != 0; j++) {
             // if (strcmp(s, fields[j]) == 0) {
                if (fields[j].name == s) {
                    return j + 3;
                }
            }
        }
    }
    return -2;
}

/* node.has_field */

static int lua_nodelib_has_field(lua_State * L)
{
    int i = -2;
    if (!lua_isnil(L, 1))
        i = get_node_field_id(L, 2, *(check_isnode(L, 1)));
    lua_pushboolean(L, (i != -2));
    return 1;
}

/* node.is_char (node[,font]) */

static int lua_nodelib_is_char(lua_State * L)
{
    halfword n = *(check_isnode(L, 1));
    if (type(n) != glyph_node) {
        lua_pushnil(L);             /* no glyph at all */
        lua_pushinteger(L,type(n)); /* can save a lookup call */
        return 2;
    } else if (subtype(n) >= 256) {
        lua_pushboolean(L,0); /* a done glyph */
    } else if (lua_type(L,2) == LUA_TNUMBER) {
        halfword f = lua_tointeger(L, 2);
        if (f && f == font(n)) {
            lua_pushinteger(L,character(n)); /* a todo glyph in the asked font */
        } else {
            lua_pushboolean(L,0); /* a todo glyph in another font */
        }
    } else {
        lua_pushinteger(L,character(n)); /* a todo glyph */
    }
    return 1;
}

static int lua_nodelib_is_glyph(lua_State * L)
{
    halfword n = *(check_isnode(L, 1));
    if (type(n) != glyph_node) {
        lua_pushboolean(L,0);
        lua_pushinteger(L,type(n));
    } else {
        lua_pushinteger(L,character(n));
        lua_pushinteger(L,font(n));
    }
    return 2;
}

/* node.direct.has_field */

static int lua_nodelib_direct_has_field(lua_State * L)
{
    int i = -2;
    halfword n = lua_tointeger(L, 1);
    if (n != null)
        i = get_node_field_id(L, 2, n);
    lua_pushboolean(L, (i != -2));
    return 1;
}

/* fetch the list of valid node types */

static int do_lua_nodelib_types(lua_State * L, node_info * data)
{
    int i;
    lua_newtable(L);
    for (i = 0; data[i].id != -1; i++) {
        lua_pushstring(L, data[i].name);
        lua_rawseti(L, -2, data[i].id);
    }
    return 1;
}

/* node.types */

static int lua_nodelib_types(lua_State * L)
{
    return do_lua_nodelib_types(L, node_data);
}

/* node.whatsits */

static int lua_nodelib_whatsits(lua_State * L)
{
    return do_lua_nodelib_types(L, whatsit_node_data);
}

/* node.fields (fetch the list of valid fields) */

static int lua_nodelib_fields(lua_State * L)
{
    int i = -1;
    int offset = 2;
    field_info *fields;
    int t = get_valid_node_type_id(L, 1);
    if (t == whatsit_node) {
        t = get_valid_node_subtype_id(L, 2);
        fields = whatsit_node_data[t].fields;
    } else {
        fields = node_data[t].fields;
    }
    lua_checkstack(L, 2);
    lua_newtable(L);
    lua_push_string_by_name(L,next);
    lua_rawseti(L, -2, 0);
    lua_push_string_by_name(L,id);
    lua_rawseti(L, -2, 1);
    if (nodetype_has_subtype(t)) {
        lua_push_string_by_name(L,subtype);
        lua_rawseti(L, -2, 2);
        offset++;
    }
    if (nodetype_has_prev(t)) {
        lua_push_string_by_name(L,prev);
        lua_rawseti(L, -2, -1);
    }
    if (fields != NULL) {
        for (i = 0; fields[i].lua != 0; i++) {
         // lua_pushstring(L, fields[i]); /* todo */
            lua_rawgeti(L, LUA_REGISTRYINDEX, fields[i].lua);
            lua_rawseti(L, -2, (i + offset));
        }
    }
    return 1;
}

static int lua_nodelib_values(lua_State * L)
{
    int i = -1;
    subtype_info *values = NULL;
    const char *s ;
    int t = lua_type(L,1);
    if (t == LUA_TSTRING) {
        /*
            delimiter options (bit set)
            delimiter modes   (bit set)
        */
        s = lua_tostring(L,1);
             if (lua_key_eq(s,dir))         values = node_values_dir;
        else if (lua_key_eq(s,direction))   values = node_values_dir;
        else if (lua_key_eq(s,glue))        values = node_values_fill;
        /* backend */
        else if (lua_key_eq(s,pdf_literal)) values = node_values_pdf_literal;
        else if (lua_key_eq(s,pdf_action))  values = node_values_pdf_action;
        else if (lua_key_eq(s,pdf_window))  values = node_values_pdf_window;
        else if (lua_key_eq(s,color_stack)) values = node_values_color_stack;
        /* extras */
        else if (lua_key_eq(s,pagestate))   values = other_values_page_states;
    }
    if (values != NULL) {
        lua_checkstack(L, 2);
        lua_newtable(L);
        for (i = 0; values[i].id >= 0 ; i++) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, values[i].lua);
            lua_rawseti(L, -2, values[i].id);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_subtypes(lua_State * L)
{
    int i = -1;
    subtype_info *subtypes = NULL;
    const char *s ;
    int t = lua_type(L,1);
    if (t == LUA_TSTRING) {
        /* official accessors */
        s = lua_tostring(L,1);
             if (lua_key_eq(s,glyph))           subtypes = node_subtypes_glyph;
        else if (lua_key_eq(s,glue))            subtypes = node_subtypes_glue;
        else if (lua_key_eq(s,dir))             subtypes = node_subtypes_dir;
        else if (lua_key_eq(s,boundary))        subtypes = node_subtypes_boundary;
        else if (lua_key_eq(s,penalty))         subtypes = node_subtypes_penalty;
        else if (lua_key_eq(s,kern))            subtypes = node_subtypes_kern;
        else if (lua_key_eq(s,rule))            subtypes = node_subtypes_rule;
        else if (lua_key_eq(s,list)
             ||  lua_key_eq(s,hlist)
             ||  lua_key_eq(s,vlist))           subtypes = node_subtypes_list; /* too many but ok as reserved */
        else if (lua_key_eq(s,adjust))          subtypes = node_subtypes_adjust;
        else if (lua_key_eq(s,disc))            subtypes = node_subtypes_disc;
        else if (lua_key_eq(s,fill))            subtypes = node_values_fill;
        else if (lua_key_eq(s,leader))          subtypes = node_subtypes_leader;
        else if (lua_key_eq(s,marginkern))      subtypes = node_subtypes_marginkern;
        else if (lua_key_eq(s,math))            subtypes = node_subtypes_math;
        else if (lua_key_eq(s,noad))            subtypes = node_subtypes_noad;
        else if (lua_key_eq(s,radical))         subtypes = node_subtypes_radical;
        else if (lua_key_eq(s,accent))          subtypes = node_subtypes_accent;
        else if (lua_key_eq(s,fence))           subtypes = node_subtypes_fence;
        /* backend */
        else if (lua_key_eq(s,pdf_destination)) subtypes = node_values_pdf_destination;
        else if (lua_key_eq(s,pdf_literal))     subtypes = node_values_pdf_literal;
    } else if (t == LUA_TNUMBER) {
        /* maybe */
        t = lua_tointeger(L,1);
             if (t == glyph_node)               subtypes = node_subtypes_glyph;
        else if (t == glue_node)                subtypes = node_subtypes_glue;
        else if (t == dir_node)                 subtypes = node_subtypes_dir;
        else if (t == boundary_node)            subtypes = node_subtypes_boundary;
        else if (t == penalty_node)             subtypes = node_subtypes_penalty;
        else if (t == kern_node)                subtypes = node_subtypes_kern;
        else if (t == rule_node)                subtypes = node_subtypes_rule;
        else if((t == hlist_node)
             || (t == vlist_node))              subtypes = node_subtypes_list;
        else if (t == adjust_node)              subtypes = node_subtypes_adjust;
        else if (t == disc_node)                subtypes = node_subtypes_disc;
        else if (t == glue_spec_node)           subtypes = node_values_fill;
        else if (t == margin_kern_node)         subtypes = node_subtypes_marginkern;
        else if (t == math_node)                subtypes = node_subtypes_math;
        else if (t == simple_noad)              subtypes = node_subtypes_noad;
        else if (t == radical_noad)             subtypes = node_subtypes_radical;
        else if (t == accent_noad)              subtypes = node_subtypes_accent;
        else if (t == fence_noad)               subtypes = node_subtypes_fence;
        /* backend */
        else if (t == pdf_dest_node)            subtypes = node_values_pdf_destination;
        else if (t == pdf_literal_node)         subtypes = node_values_pdf_literal;
    }
    if (subtypes != NULL) {
        lua_checkstack(L, 2);
        lua_newtable(L);
        for (i = 0; subtypes[i].id >= 0 ; i++) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, subtypes[i].lua);
            lua_rawseti(L, -2, subtypes[i].id);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* node.slide (find the end of a list and add prev links) */

static int lua_nodelib_slide(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 1))
        return 1;               /* the nil itself */
    n = *check_isnode(L, 1);
    if (n == null)
        return 1;               /* the old userdata */
    /* alink(t) = null; */ /* don't do this, |t|'s |alink| may be a valid pointer */
    while (vlink(n) != null) {
        alink(vlink(n)) = n;
        n = vlink(n);
    }
    lua_nodelib_push_fast(L, n);
    return 1;
}

/* node.direct.slide */

static int lua_nodelib_direct_slide(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n == null) {
        lua_pushnil(L);
    } else {
        while (vlink(n) != null) {
            alink(vlink(n)) = n;
            n = vlink(n);
        }
        lua_pushinteger(L, n);
    }
    return 1;
}

/* node.tail (find the end of a list) */

static int lua_nodelib_tail(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 1))
        return 1;               /* the nil itself */
    n = *check_isnode(L, 1);
    if (n == null)
        return 1;               /* the old userdata */
    while (vlink(n) != null)
        n = vlink(n);
    lua_nodelib_push_fast(L, n);
    return 1;
}

/* node.direct.tail */

static int lua_nodelib_direct_tail(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n == null) {
        lua_pushnil(L);
    } else {
        while (vlink(n) != null)
            n = vlink(n);
        lua_pushinteger(L, n);
    }
    return 1;
}

/* node.end_of_math (skip over math and return last) */

static int lua_nodelib_end_of_math(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 1))
        return 0;
    n = *check_isnode(L, 1);
    if (n == null)
        return 0;
    if (type(n) == math_node && (subtype(n) == 1)) {
        lua_nodelib_push_fast(L, n);
        return 1;
    }
    while (vlink(n) != null) {
        n = vlink(n);
        if (n && (type(n) == math_node) && (subtype(n) == 1)) {
            lua_nodelib_push_fast(L, n);
            return 1;
        }
    }
    return 0;
}

/* node.direct.end_of_math */

static int lua_nodelib_direct_end_of_math(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n == null)
        return 0;
    if ((type(n)==math_node && (subtype(n)==1))) {
        lua_pushinteger(L, n);
        return 1;
    }
    while (vlink(n) != null) {
        n = vlink(n);
        if (n && (type(n)==math_node) && (subtype(n)==1)) {
            lua_pushinteger(L, n);
            return 1;
        }
    }
    return 0;
}


/* node.has_attribute (gets attribute) */

static int lua_nodelib_has_attribute(lua_State * L)
{
    int i, val;
    halfword n = *check_isnode(L, 1);
    if (n != null) {
        i = lua_tointeger(L, 2);
        val = luaL_optinteger(L, 3, UNUSED_ATTRIBUTE);
        if ((val = has_attribute(n, i, val)) > UNUSED_ATTRIBUTE) {
            lua_pushinteger(L, val);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

/* node.direct.has_attribute */

static int lua_nodelib_direct_has_attribute(lua_State * L)
{
    int i, val;
    halfword n = lua_tointeger(L, 1);
    if (n != null) {
        i = lua_tointeger(L, 2);
        val = luaL_optinteger(L, 3, UNUSED_ATTRIBUTE);
        if ((val = has_attribute(n, i, val)) > UNUSED_ATTRIBUTE) {
            lua_pushinteger(L, val);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

/* node.get_attribute */

static int lua_nodelib_get_attribute(lua_State * L)
{
    halfword p = *check_isnode(L, 1);
    if (nodetype_has_attributes(type(p))) {
        p = node_attr(p);
        if (p != null) {
            p = vlink(p);
            if (p != null) {
                int i = 0;
                if (lua_gettop(L) > 1) {
                    i = lua_tointeger(L, 2);
                }
                while (p != null) {
                    if (attribute_id(p) == i) {
                        int ret = attribute_value(p);
                        if (ret == UNUSED_ATTRIBUTE) {
                            break;
                        } else {
                            lua_pushinteger(L,ret);
                            return 1;
                        }
                    } else if (attribute_id(p) > i) {
                        break;
                    }
                    p = vlink(p);
                }
            }
        }
    }
    lua_pushnil(L);
    return 1;
}

static int lua_nodelib_find_attribute(lua_State * L) /* returns attr value and node */
{
    halfword c = *check_isnode(L, 1);
    halfword p ;
    int i = lua_tointeger(L, 2);
    while (c != null) {
        if (nodetype_has_attributes(type(c))) {
            p = node_attr(c);
            if (p != null) {
                p = vlink(p);
                while (p != null) {
                    if (attribute_id(p) == i) {
                        int ret = attribute_value(p);
                        if (ret == UNUSED_ATTRIBUTE) {
                            break;
                        } else {
                            lua_pushinteger(L,ret);
                            lua_nodelib_push_fast(L, c  );
                            return 2;
                        }
                    } else if (attribute_id(p) > i) {
                        break;
                    }
                    p = vlink(p);
                }
            }
        }
        c = vlink(c);
    }
    /*
        lua_pushnil(L);
        lua_pushnil(L);
        return 2;
    */
    return 0;
}

/* node.direct.get_attribute */
/* node.direct.set_attribute */
/* node.direct.unset_attribute */
/* node.direct.find_attribute */

static int lua_nodelib_direct_get_attribute(lua_State * L)
{
    register halfword p = lua_tointeger(L, 1);
    if (nodetype_has_attributes(type(p))) {
        p = node_attr(p);
        if (p != null) {
            p = vlink(p);
            if (p != null) {
                int i = 0;
                if (lua_gettop(L) > 1) {
                    i = lua_tointeger(L, 2);
                }
                while (p != null) {
                    if (attribute_id(p) == i) {
                        int ret = attribute_value(p);
                        if (ret == UNUSED_ATTRIBUTE) {
                            break;
                        } else {
                            lua_pushinteger(L,ret);
                            return 1;
                        }
                    } else if (attribute_id(p) > i) {
                        break;
                    }
                    p = vlink(p);
                }
            }
        }
    }
    lua_pushnil(L);
    return 1;
}

static int lua_nodelib_direct_set_attribute(lua_State * L)
{
    int i, val;
    halfword n = lua_tointeger(L, 1);
    if (n == null)
        return 0;
    if (lua_gettop(L) == 3) {
        i = (int) lua_tointeger(L, 2);
        val = (int) lua_tointeger(L, 3);
        if (val == UNUSED_ATTRIBUTE) {
            (void) unset_attribute(n, i, val);
        } else {
            set_attribute(n, i, val);
        }
    } else {
        luaL_error(L, "incorrect number of arguments");
    }
    return 0;
}

static int lua_nodelib_direct_find_attribute(lua_State * L) /* returns attr value and node */
{
    halfword c = lua_tointeger(L, 1);
    halfword p ;
    int i = lua_tointeger(L, 2);
    while (c != null) {
        if (nodetype_has_attributes(type(c))) {
            p = node_attr(c);
            if (p != null) {
                p = vlink(p);
                while (p != null) {
                    if (attribute_id(p) == i) {
                        int ret = attribute_value(p);
                        if (ret == UNUSED_ATTRIBUTE) {
                            break;
                        } else {
                            lua_pushinteger(L,ret);
                            lua_pushinteger(L,c);
                            return 2;
                        }
                    } else if (attribute_id(p) > i) {
                        break;
                    }
                    p = vlink(p);
                }
            }
        }
        c = vlink(c);
    }
    /*
        lua_pushnil(L);
        lua_pushnil(L);
        return 2;
    */
    return 0;
}

static int lua_nodelib_direct_unset_attribute(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n == null) {
        lua_pushnil(L);
    } else if (lua_gettop(L) <= 3) { /* a useless test, we never test for that elsewhere */
        int i = (int)luaL_checknumber(L, 2);
        int val = (int)luaL_optnumber(L, 3, UNUSED_ATTRIBUTE);
        int ret = unset_attribute(n, i, val);
        if (ret > UNUSED_ATTRIBUTE) {
            lua_pushinteger(L, ret);
        } else {
            lua_pushnil(L);
        }
    } else { /* can go */
        return luaL_error(L, "incorrect number of arguments");
    }
    return 1;
}

/* node.set_attribute */
/* node.unset_attribute */

static int lua_nodelib_set_attribute(lua_State * L)
{
    if (lua_gettop(L) == 3) {
        int i = lua_tointeger(L, 2);
        int val = lua_tointeger(L, 3);
        halfword n = *check_isnode(L, 1);
        if (val == UNUSED_ATTRIBUTE) {
            (void) unset_attribute(n, i, val);
        } else {
            set_attribute(n, i, val);
        }
    } else {
        luaL_error(L, "incorrect number of arguments");
    }
    return 0;
}

static int lua_nodelib_unset_attribute(lua_State * L)
{
    if (lua_gettop(L) <= 3) {
        int i = luaL_checknumber(L, 2);
        int val = luaL_optnumber(L, 3, UNUSED_ATTRIBUTE);
        halfword n = *check_isnode(L, 1);
        int ret = unset_attribute(n, i, val);
        if (ret > UNUSED_ATTRIBUTE) {
            lua_pushinteger(L, ret);
        } else {
            lua_pushnil(L);
        }
        return 1;
    } else {
        return luaL_error(L, "incorrect number of arguments");
    }
}

/* node.direct.getwidth  */
/* node.direct.setwidth  */
/* node.direct.getheight (for consistency) */
/* node.direct.setheight (for consistency) */
/* node.direct.getdepth  (for consistency) */
/* node.direct.setdepth  (for consistency) */

/* split ifs for clearity .. compiler will optimize */

static int lua_nodelib_direct_getwidth(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == hlist_node || t == vlist_node || t == rule_node) {
            lua_pushinteger(L,width(n));
        } else if (t == glyph_node) {
            lua_pushnumber(L, char_width(font(n),character(n)));
            if (lua_toboolean(L,2)) {
                lua_pushinteger(L, ex_glyph(n));
                return 2;
            }
        } else if (t == glue_node || t == glue_spec_node || t == math_node || t == ins_node) {
            lua_pushinteger(L,width(n));
        } else if (t == kern_node) {
            lua_pushinteger(L, width(n));
            if (lua_toboolean(L,2)) {
                lua_pushinteger(L, ex_kern(n));
                return 2;
            }
        } else if (t == margin_kern_node) {
            lua_pushinteger(L,width(n));
        } else if (t == unset_node) {
            lua_pushinteger(L,width(n));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_setwidth(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == hlist_node || t == vlist_node || t == rule_node || t == glue_node || t == glue_spec_node || t == math_node ||
                t == kern_node || t == margin_kern_node ||  t == ins_node || t == unset_node ||
                t == fraction_noad || t == radical_noad ) {
            if (lua_type(L, 2) == LUA_TNUMBER) {
                width(n) = lua_roundnumber(L,2);
            } else {
                width(n) = 0;
            }
        }
    }
    return 0;
}

static int lua_nodelib_direct_getheight(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == hlist_node || t == vlist_node || t == rule_node) {
            lua_pushinteger(L,height(n));
        } else if (t == glyph_node) {
            lua_pushinteger(L, char_height(font(n),character(n)));
        } else if (t == unset_node || t == ins_node) {
            lua_pushinteger(L,height(n));
        } else if (t == fence_noad) {
            lua_pushinteger(L,delimiterheight(n));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_setheight(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        halfword h = 0;
        if (lua_type(L, 2) == LUA_TNUMBER) {
            h = lua_roundnumber(L,2);
        }
        if (t == hlist_node || t == vlist_node || t == rule_node || t == unset_node) {
            height(n) = h;
        } else if (t == fence_noad) {
            delimiterheight(n) = h;
        }
    }
    return 0;
}

static int lua_nodelib_direct_getdepth(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == hlist_node || t == vlist_node || t == rule_node) {
            lua_pushinteger(L,depth(n));
        } else if (t == glyph_node) {
            lua_pushinteger(L, char_depth(font(n),character(n)));
        } else if (t == unset_node || t == ins_node) {
            lua_pushinteger(L,depth(n));
        } else if (t == fence_noad) {
            lua_pushinteger(L,delimiterdepth(n));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_direct_setdepth(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        halfword d = 0;
        if (lua_type(L, 2) == LUA_TNUMBER) {
            d = lua_roundnumber(L,2);
        }
        if (t == hlist_node || t == vlist_node || t == rule_node || t == unset_node) {
            depth(n) = d;
        } else if (t == fence_noad) {
            delimiterdepth(n) = d;
        }
    }
    return 0;
}

/* node.direct.getshift */
/* node.direct.setshift */

static int lua_nodelib_direct_getshift(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == hlist_node || t == vlist_node) {
            lua_pushinteger(L,shift_amount(n));
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

static int lua_nodelib_direct_setshift(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == hlist_node || t == vlist_node) {
            if (lua_type(L, 2) == LUA_TNUMBER) {
                shift_amount(n) = lua_roundnumber(L,2);
            } else {
                shift_amount(n) = 0;
            }
        }
    }
    return 0;
}

/* node.direct.getglue */
/* node.direct.setglue */
/* node.direct.is_zero_glue */

static int lua_nodelib_direct_get_glue(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == glue_node || t == glue_spec_node || t == math_node || t == ins_node) {
            lua_pushinteger(L,width(n));
            lua_pushinteger(L,stretch(n));
            lua_pushinteger(L,shrink(n));
            lua_pushinteger(L,stretch_order(n));
            lua_pushinteger(L,shrink_order(n));
            return 5;
        } else if (t == hlist_node || t == vlist_node) {
            lua_pushnumber(L, (double) glue_set(n)); /* float */
            lua_pushinteger(L,glue_order(n));
            lua_pushinteger(L,glue_sign(n));
            return 3;
        }
    }
    return 0;
}

static int lua_nodelib_direct_set_glue(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        int top = lua_gettop(L);
        halfword t = type(n);
        if (t == glue_node || t == glue_spec_node || t == math_node) {
            width(n)         = ((top > 1 && lua_type(L, 2) == LUA_TNUMBER)) ? lua_roundnumber(L,2) : 0;
            stretch(n)       = ((top > 2 && lua_type(L, 3) == LUA_TNUMBER)) ? lua_roundnumber(L,3) : 0;
            shrink(n)        = ((top > 3 && lua_type(L, 4) == LUA_TNUMBER)) ? lua_roundnumber(L,4) : 0;
            stretch_order(n) = ((top > 4 && lua_type(L, 5) == LUA_TNUMBER)) ? lua_tointeger(L,5) : 0;
            shrink_order(n)  = ((top > 5 && lua_type(L, 6) == LUA_TNUMBER)) ? lua_tointeger(L,6) : 0;
        } else if (t == hlist_node || t == vlist_node) {
            glue_set(n)   = ((top > 1 && lua_type(L, 2) == LUA_TNUMBER)) ? (glue_ratio) lua_tonumber(L,2) : 0;
            glue_order(n) = ((top > 2 && lua_type(L, 3) == LUA_TNUMBER)) ? lua_tointeger(L,3) : 0;
            glue_sign(n)  = ((top > 3 && lua_type(L, 4) == LUA_TNUMBER)) ? lua_tointeger(L,4) : 0;
            return 3;
        }
    }
    return 0;
}

static int lua_nodelib_direct_is_zero_glue(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == glue_node || t == glue_spec_node || t == math_node || t == ins_node) {
            lua_pushboolean(L,(width(n) == 0 && stretch(n) == 0 && shrink(n) == 0));
            return 1;
        } else if (t == hlist_node || t == vlist_node) {
            lua_pushboolean(L,(glue_set(n) == 0 && glue_order(n) == 0 && glue_sign(n) == 0));
            return 1;
        }
    }
    return 0;
}

/* node.getglue */
/* node.setglue */
/* node.is_zero_glue */

static int lua_nodelib_get_glue(lua_State * L)
{
    halfword n = *check_isnode(L, 1);
    if (n) {
        halfword t = type(n);
        if (t == glue_node || t == glue_spec_node || t == math_node || t == ins_node) {
            lua_pushinteger(L,width(n));
            lua_pushinteger(L,stretch(n));
            lua_pushinteger(L,shrink(n));
            lua_pushinteger(L,stretch_order(n));
            lua_pushinteger(L,shrink_order(n));
            return 5;
        } else if (t == hlist_node || t == vlist_node) {
            lua_pushnumber(L, (double) glue_set(n)); /* float */
            lua_pushinteger(L,glue_order(n));
            lua_pushinteger(L,glue_sign(n));
            return 3;
        }
    }
    return luaL_error(L, "glue (spec) or list expected");
}

static int lua_nodelib_set_glue(lua_State * L)
{
    halfword n = *check_isnode(L, 1);
    int top = lua_gettop(L) ;
    if (n) {
        halfword t = type(n);
        if (t == glue_node || t == glue_spec_node || t == math_node) {
            width(n)         = ((top > 1 && lua_type(L, 2) == LUA_TNUMBER)) ? lua_roundnumber(L,2) : 0;
            stretch(n)       = ((top > 2 && lua_type(L, 3) == LUA_TNUMBER)) ? lua_roundnumber(L,3) : 0;
            shrink(n)        = ((top > 3 && lua_type(L, 4) == LUA_TNUMBER)) ? lua_roundnumber(L,4) : 0;
            stretch_order(n) = ((top > 4 && lua_type(L, 5) == LUA_TNUMBER)) ? lua_tointeger(L,5) : 0;
            shrink_order(n)  = ((top > 5 && lua_type(L, 6) == LUA_TNUMBER)) ? lua_tointeger(L,6) : 0;
        } else if (t == hlist_node || t == vlist_node) {
            glue_set(n)   = ((top > 1 && lua_type(L, 2) == LUA_TNUMBER)) ? (glue_ratio) lua_tonumber(L,2) : 0;
            glue_order(n) = ((top > 2 && lua_type(L, 3) == LUA_TNUMBER)) ? lua_tointeger(L,3) : 0;
            glue_sign(n)  = ((top > 3 && lua_type(L, 4) == LUA_TNUMBER)) ? lua_tointeger(L,4) : 0;
            return 3;
        }
        return 0;
    }
    return luaL_error(L, "glue (spec) or list expected");
}

static int lua_nodelib_is_zero_glue(lua_State * L)
{
    halfword n = *check_isnode(L, 1);
    if (n != null) {
        halfword t = type(n);
        if (t == glue_node || t == glue_spec_node || t == math_node || t == ins_node) {
            lua_pushboolean(L,(width(n) == 0 && stretch(n) == 0 && shrink(n) == 0));
            return 1;
        } else if (t == hlist_node || t == vlist_node) {
            lua_pushboolean(L,(glue_set(n) == 0 && glue_order(n) == 0 && glue_sign(n) == 0));
            return 1;
        }
    }
    return luaL_error(L, "glue (spec) or list expected");
}

/* iteration */

static int nodelib_aux_nil(lua_State * L)
{
    lua_pushnil(L);
    return 1;
}

/* node.direct.traverse */
/* node.direct.traverse_id */
/* node.direct.traverse_char */
/* node.direct.traverse_glyph */
/* node.direct.traverse_list */

static int nodelib_direct_aux_next(lua_State * L)
{
    halfword t;
    if (lua_isnil(L, 2)) {
        t = lua_tointeger(L,1) ;
        lua_settop(L,1);
    } else {
        t = lua_tointeger(L,2) ;
        t = vlink(t);
        lua_settop(L,2);
    }
    if (t == null) {
        lua_pushnil(L);
        return 1;
    } else {
        lua_pushinteger(L,t);
        lua_pushinteger(L,type(t));
        lua_pushinteger(L,subtype(t));
        return 3;
    }
}

static int lua_nodelib_direct_traverse(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 1)) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    n = (halfword) lua_tointeger(L, 1);
    if (n == null) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    lua_pushcclosure(L, nodelib_direct_aux_next, 0);
    lua_pushinteger(L,n);
    lua_pushnil(L);
    return 3;
}

static int nodelib_direct_aux_next_filtered(lua_State * L)
{
    halfword t;        /* traverser */
    int i = (int) lua_tointeger(L, lua_upvalueindex(1));
    if (lua_isnil(L, 2)) {      /* first call */
        t = lua_tointeger(L,1) ;
        lua_settop(L,1);
    } else {
        t = lua_tointeger(L,2) ;
        t = vlink(t);
        lua_settop(L,2);
    }
    while (t != null && type(t) != i) {
        t = vlink(t);
    }
    if (t == null) {
        lua_pushnil(L);
        return 1;
    } else {
        lua_pushinteger(L,t);
        lua_pushinteger(L,subtype(t));
        return 2;
    }
}

static int lua_nodelib_direct_traverse_filtered(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 2)) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    n = (halfword) lua_tointeger(L, 2);
    if (n == null)
        return 0;
    lua_pop(L, 1);
    lua_pushcclosure(L, nodelib_direct_aux_next_filtered, 1);
    lua_pushinteger(L,n);
    lua_pushnil(L);
    return 3;
}

static int nodelib_direct_aux_next_char(lua_State * L)
{
    halfword t;            /* traverser */
    if (lua_isnil(L, 2)) { /* first call */
        t = lua_tointeger(L,1) ;
        lua_settop(L,1);
    } else {
        t = lua_tointeger(L,2) ;
        t = vlink(t);
        lua_settop(L,2);
    }
    while (! ((t == null) || (type(t) == glyph_node && subtype(t) < 256))) {
        t = vlink(t);
    }
    if (t == null) {
        lua_pushnil(L);
        return 1;
    } else {
        lua_pushinteger(L,t);
        lua_pushinteger(L,character(t));
        lua_pushinteger(L,font(t));
        return 3;
    }
}

static int lua_nodelib_direct_traverse_char(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 1)) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    n = (halfword) lua_tointeger(L, 1);
    if (n == null) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    lua_pushcclosure(L, nodelib_direct_aux_next_char, 0);
    lua_pushinteger(L,n);
    lua_pushnil(L);
    return 3;
}

static int nodelib_direct_aux_next_glyph(lua_State * L)
{
    halfword t;            /* traverser */
    if (lua_isnil(L, 2)) { /* first call */
        t = lua_tointeger(L,1) ;
        lua_settop(L,1);
    } else {
        t = lua_tointeger(L,2) ;
        t = vlink(t);
        lua_settop(L,2);
    }
    while (t != null && type(t) != glyph_node) {
        t = vlink(t);
    }
    if (t == null) {
        lua_pushnil(L);
        return 1;
    } else {
        lua_pushinteger(L,t);
        lua_pushinteger(L,character(t));
        lua_pushinteger(L,font(t));
        return 3;
    }
}

static int lua_nodelib_direct_traverse_glyph(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 1)) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    n = (halfword) lua_tointeger(L, 1);
    if (n == null) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    lua_pushcclosure(L, nodelib_direct_aux_next_glyph, 0);
    lua_pushinteger(L,n);
    lua_pushnil(L);
    return 3;
}

static int nodelib_direct_aux_next_list(lua_State * L)
{
    halfword t;        /* traverser */
    if (lua_isnil(L, 2)) {      /* first call */
        t = lua_tointeger(L,1) ;
        lua_settop(L,1);
    } else {
        t = lua_tointeger(L,2) ;
        t = vlink(t);
        lua_settop(L,2);
    }
    while (t != null && type(t) != hlist_node && type(t) != vlist_node) {
        t = vlink(t);
    }
    if (t == null) {
        lua_pushnil(L);
        return 1;
    } else {
        lua_pushinteger(L,t);
        lua_pushinteger(L,type(t));
        lua_pushinteger(L,subtype(t));
        nodelib_pushdirect_or_nil(list_ptr(t));
        return 4;
    }
}

static int lua_nodelib_direct_traverse_list(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 1)) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    n = (halfword) lua_tointeger(L, 1);
    if (n == null) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    lua_pushcclosure(L, nodelib_direct_aux_next_list, 1);
    lua_pushinteger(L, n);
    lua_pushnil(L);
    return 3;
}

/* node.traverse */
/* node.traverse_id */
/* node.traverse_char */
/* node.traverse_glyph */
/* node.traverse_list */

static int nodelib_aux_next(lua_State * L)
{
    halfword t;
    halfword *a;
    if (lua_isnil(L, 2)) {
        t = *check_isnode(L, 1);
        lua_settop(L,1);
    } else {
        t = *check_isnode(L, 2);
        t = vlink(t);
        lua_settop(L,2);
    }
    if (t == null) {
        lua_pushnil(L);
        return 1;
    } else {
        fast_metatable_top(t);
        lua_pushinteger(L,type(t));
        lua_pushinteger(L,subtype(t));
        return 3;
    }
}

static int lua_nodelib_traverse(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 1)) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    n = *check_isnode(L, 1);
    lua_pushcclosure(L, nodelib_aux_next, 0);
    lua_nodelib_push_fast(L, n);
    lua_pushnil(L);
    return 3;
}

static int nodelib_aux_next_filtered(lua_State * L)
{
    halfword t;        /* traverser */
    halfword *a;
    int i = (int) lua_tointeger(L, lua_upvalueindex(1));
    if (lua_isnil(L, 2)) {      /* first call */
        t = *check_isnode(L, 1);
        lua_settop(L,1);
    } else {
        t = *check_isnode(L, 2);
        t = vlink(t);
        lua_settop(L,2);
    }
    while (t != null && type(t) != i) {
        t = vlink(t);
    }
    if (t == null) {
        lua_pushnil(L);
        return 1;
    } else {
        fast_metatable_top(t);
        lua_pushinteger(L,subtype(t));
        return 2;
    }
}

static int lua_nodelib_traverse_filtered(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 2)) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    n = *check_isnode(L, 2);
    lua_pop(L, 1);              /* the node, integer remains */
    lua_pushcclosure(L, nodelib_aux_next_filtered, 1);
    lua_nodelib_push_fast(L, n);
    lua_pushnil(L);
    return 3;
}

static int nodelib_aux_next_char(lua_State * L)
{
    halfword t;            /* traverser */
    halfword *a;
    if (lua_isnil(L, 2)) { /* first call */
        t = *check_isnode(L, 1);
        lua_settop(L,1);
    } else {
        t = *check_isnode(L, 2);
        t = vlink(t);
        lua_settop(L,2);
    }
    while (! ((t == null) || (type(t) == glyph_node && subtype(t) < 256))) {
        t = vlink(t);
    }
    if (t == null) {
        lua_pushnil(L);
        return 1;
    } else {
        fast_metatable_top(t);
        lua_pushinteger(L,character(t));
        lua_pushinteger(L,font(t));
        return 3;
    }
}

static int lua_nodelib_traverse_char(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 1)) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    n = *check_isnode(L, 1);
    lua_pushcclosure(L, nodelib_aux_next_char, 0);
    lua_nodelib_push_fast(L, n);
    lua_pushnil(L);
    return 3;
}

static int nodelib_aux_next_glyph(lua_State * L)
{
    halfword t;            /* traverser */
    halfword *a;
    if (lua_isnil(L, 2)) { /* first call */
        t = *check_isnode(L, 1);
        lua_settop(L,1);
    } else {
        t = *check_isnode(L, 2);
        t = vlink(t);
        lua_settop(L,2);
    }
    while (t != null && type(t) != glyph_node) {
        t = vlink(t);
    }
    if (t == null) {
        lua_pushnil(L);
        return 1;
    } else {
        fast_metatable_top(t);
        lua_pushinteger(L,character(t));
        lua_pushinteger(L,font(t));
        return 3;
    }
}

static int lua_nodelib_traverse_glyph(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 1)) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    n = *check_isnode(L, 1);
    lua_pushcclosure(L, nodelib_aux_next_glyph, 0);
    lua_nodelib_push_fast(L, n);
    lua_pushnil(L);
    return 3;
}

static int nodelib_aux_next_list(lua_State * L)
{
    halfword t;        /* traverser */
    halfword *a;
    if (lua_isnil(L, 2)) {      /* first call */
        t = *check_isnode(L, 1);
        lua_settop(L,1);
    } else {
        t = *check_isnode(L, 2);
        t = vlink(t);
        lua_settop(L,2);
    }
    while (t != null && type(t) != hlist_node && type(t) != vlist_node) {
        t = vlink(t);
    }
    if (t == null) {
        lua_pushnil(L);
        return 1;
    } else {
        fast_metatable_top(t);
        lua_pushinteger(L,type(t));
        lua_pushinteger(L,subtype(t));
        fast_metatable_or_nil(list_ptr(t));
        return 4;
    }
}

static int lua_nodelib_traverse_list(lua_State * L)
{
    halfword n;
    if (lua_isnil(L, 1)) {
        lua_pushcclosure(L, nodelib_aux_nil, 0);
        return 1;
    }
    n = *check_isnode(L, 1);
    lua_pushcclosure(L, nodelib_aux_next_list, 1);
    lua_nodelib_push_fast(L, n);
    lua_pushnil(L);
    return 3;
}

/* counting */

static int do_lua_nodelib_count(lua_State * L, halfword match, int i, halfword first1)
{
    int count = 0;
    int t = first1;
    while (t != match) {
        if (i < 0 || type(t) == i)
            count++;
        t = vlink(t);
    }
    lua_pushinteger(L, count);
    return 1;
}

/* node.direct.length */
/* node.direct.count */

static int lua_nodelib_direct_length(lua_State * L)
{
    halfword m;
    halfword n = lua_tointeger(L, 1);
    if (n == 0) {
        lua_pushinteger(L, 0);
        return 1;
    }
    m = (halfword) lua_tointeger(L, 2);
    return do_lua_nodelib_count(L, m, -1, n);
}

static int lua_nodelib_direct_count(lua_State * L)
{
    return do_lua_nodelib_count(L,
        (halfword) lua_tointeger(L, 3), /* m */
        (int) lua_tointeger(L, 1),      /* i */
        (halfword) lua_tointeger(L, 2)  /* n */
    );
}

/* node.length */
/* node.count */

static int lua_nodelib_length(lua_State * L)
{
    halfword n;
    halfword m = null;
    if (lua_isnil(L, 1)) {
        lua_pushinteger(L, 0);
        return 1;
    }
    n = *check_isnode(L, 1);
    if (lua_gettop(L) == 2)
        m = *check_isnode(L, 2);
    return do_lua_nodelib_count(L, m, -1, n);
}

static int lua_nodelib_count(lua_State * L)
{
    halfword n;
    halfword m = null;
    int i = lua_tointeger(L, 1);
    if (lua_isnil(L, 2)) {
        lua_pushinteger(L, 0);
        return 1;
    }
    n = *check_isnode(L, 2);
    if (lua_gettop(L) == 3)
        m = *check_isnode(L, 3);
    return do_lua_nodelib_count(L, m, i, n);
}

/* node.direct.getfield */

static void lua_nodelib_getfield_whatsit(lua_State * L, int n, const char *s)
{
    int t = subtype(n);
    if (t == user_defined_node) {
        if (lua_key_eq(s, user_id)) {
            lua_pushinteger(L, user_node_id(n));
        } else if (lua_key_eq(s, type)) {
            lua_pushinteger(L, user_node_type(n));
        } else if (lua_key_eq(s, value)) {
            switch (user_node_type(n)) {
                case 'a':
                    nodelib_pushlist(L, user_node_value(n));
                    break;
                case 'd':
                    lua_pushinteger(L, user_node_value(n));
                    break;
                case 'l':
                    if (user_node_value(n) != 0) {
                        lua_rawgeti(L, LUA_REGISTRYINDEX, user_node_value(n));
                    } else {
                        lua_pushnil(L);
                    }
                    break;
                case 'n':
                    nodelib_pushlist(L, user_node_value(n));
                    break;
                case 's':
                    nodelib_pushstring(L, user_node_value(n));
                    break;
                case 't':
                    tokenlist_to_lua(L, user_node_value(n));
                    break;
                default:
                    lua_pushinteger(L, user_node_value(n));
                    break;
            }
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_literal_node) {
        /*tex The |string| key is obsolete. */
        if (lua_key_eq(s, mode)) {
            lua_pushinteger(L, pdf_literal_mode(n));
        } else if (lua_key_eq(s, data) || lua_key_eq(s, token) || lua_key_eq(s, string)) {
            get_pdf_literal_direct_value(L, n);
        } else {
            lua_pushnil(L);
        }
    } else if (t == late_lua_node) {
        /*tex The |string| key is obsolete. */
        if (lua_key_eq(s, data) || lua_key_eq(s, token) || lua_key_eq(s, string)) {
            get_late_lua_direct_value(L,n);
        } else if (lua_key_eq(s, name)) {
            tokenlist_to_luastring(L, late_lua_name(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_annot_node) {
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, objnum)) {
            lua_pushinteger(L, pdf_annot_objnum(n));
        } else if (lua_key_eq(s, data)) {
            tokenlist_to_luastring(L, pdf_annot_data(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_dest_node) {
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, named_id)) {
            lua_pushinteger(L, pdf_dest_named_id(n));
        } else if (lua_key_eq(s, dest_id)) {
            if (pdf_dest_named_id(n) == 1)
                tokenlist_to_luastring(L, pdf_dest_id(n));
            else
                lua_pushinteger(L, pdf_dest_id(n));
        } else if (lua_key_eq(s, dest_type)) {
            lua_pushinteger(L, pdf_dest_type(n));
        } else if (lua_key_eq(s, xyz_zoom)) {
            lua_pushinteger(L, pdf_dest_xyz_zoom(n));
        } else if (lua_key_eq(s, objnum)) {
            lua_pushinteger(L, pdf_dest_objnum(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_setmatrix_node) {
        if (lua_key_eq(s, data)) {
            get_pdf_setmatrix_direct_value(L,n);
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_colorstack_node) {
        if (lua_key_eq(s, stack)) {
            lua_pushinteger(L, pdf_colorstack_stack(n));
        } else if (lua_key_eq(s, command)) {
            lua_pushinteger(L, pdf_colorstack_cmd(n));
        } else if (lua_key_eq(s, data)) {
            tokenlist_to_luastring(L, pdf_colorstack_data(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_refobj_node) {
        if (lua_key_eq(s, objnum)) {
            lua_pushinteger(L, pdf_obj_objnum(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == write_node) {
        if (lua_key_eq(s, stream)) {
            lua_pushinteger(L, write_stream(n));
        } else if (lua_key_eq(s, data)) {
            get_write_direct_value(L,n);
        } else {
            lua_pushnil(L);
        }
    } else if (t == special_node) {
        if (lua_key_eq(s, data)) {
            get_special_direct_value(L,n);
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_start_link_node) {
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, objnum)) {
            lua_pushinteger(L, pdf_link_objnum(n));
        } else if (lua_key_eq(s, link_attr)) {
            tokenlist_to_luastring(L, pdf_link_attr(n));
        } else if (lua_key_eq(s, action)) {
            nodelib_pushaction(L, pdf_link_action(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_action_node) {
        if (lua_key_eq(s, action_type)) {
            lua_pushinteger(L, pdf_action_type(n));
        } else if (lua_key_eq(s, named_id)) {
            lua_pushinteger(L, pdf_action_named_id(n));
        } else if (lua_key_eq(s, action_id)) {
            if (pdf_action_named_id(n) == 1) {
                tokenlist_to_luastring(L, pdf_action_id(n));
            } else {
                lua_pushinteger(L, pdf_action_id(n));
            }
        } else if (lua_key_eq(s, file)) {
            tokenlist_to_luastring(L, pdf_action_file(n));
        } else if (lua_key_eq(s, new_window)) {
            lua_pushinteger(L, pdf_action_new_window(n));
        } else if (lua_key_eq(s, data)) {
            tokenlist_to_luastring(L, pdf_action_tokens(n));
        } else {
            lua_pushnil(L);
        }
    } else if ((t == pdf_thread_node) || (t == pdf_start_thread_node)) {
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, named_id)) {
            lua_pushinteger(L, pdf_thread_named_id(n));
        } else if (lua_key_eq(s, thread_id)) {
            if (pdf_thread_named_id(n) == 1) {
                tokenlist_to_luastring(L, pdf_thread_id(n));
            } else {
                lua_pushinteger(L, pdf_thread_id(n));
            }
        } else if (lua_key_eq(s, thread_attr)) {
            tokenlist_to_luastring(L, pdf_thread_attr(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == open_node) {
        if (lua_key_eq(s, stream)) {
            lua_pushinteger(L, write_stream(n));
        } else if (lua_key_eq(s, name)) {
            nodelib_pushstring(L, open_name(n));
        } else if (lua_key_eq(s, area)) {
            nodelib_pushstring(L, open_area(n));
        } else if (lua_key_eq(s, ext)) {
            nodelib_pushstring(L, open_ext(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == close_node) {
        if (lua_key_eq(s, stream)) {
            lua_pushinteger(L, write_stream(n));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
}

static int lua_nodelib_fast_getfield(lua_State * L)
{
    /*
        the order is somewhat determined by the occurance of nodes and
        importance of fields
    */

      halfword *a;
      const char *s;

      halfword n = *((halfword *) lua_touserdata(L, 1));
      int t = type(n);

    /*

        somenode[9] as interface to attributes ... 30% faster than has_attribute
        (1) because there is no lua function overhead, and (2) because we already
        know that we deal with a node so no checking is needed. The fast typecheck
        is needed (lua_check... is a slow down actually).

    */

    if (lua_type(L, 2) == LUA_TNUMBER) {

        halfword p;
        int i;

        if (! nodetype_has_attributes(t)) {
            lua_pushnil(L);
            return 1;
        }

        p = node_attr(n);
        if (p == null || vlink(p) == null) {
            lua_pushnil(L);
            return 1;
        }
        i = (int) lua_tointeger(L, 2);
        p = vlink(p);
        while (p != null) {
            if (attribute_id(p) == i) {
                if ((int) attribute_value(p) > UNUSED_ATTRIBUTE) {
                    lua_pushinteger(L, (int) attribute_value(p));
                } else {
                    lua_pushnil(L);
                }
                return 1;
            } else if (attribute_id(p) > i) {
                lua_pushnil(L);
                return 1;
            }
            p = vlink(p);
        }
        lua_pushnil(L);
        return 1;
    }

    s = lua_tostring(L, 2);

    if (lua_key_eq(s, id)) {
        lua_pushinteger(L, t);
    } else if (lua_key_eq(s, next)) {
        fast_metatable_or_nil(vlink(n));
    } else if (lua_key_eq(s, prev)) {
        fast_metatable_or_nil(alink(n));
    } else if (lua_key_eq(s, attr)) {
        if (! nodetype_has_attributes(t)) {
            lua_pushnil(L);
        } else {
            nodelib_pushattr(L, node_attr(n));
        }
    } else if (t == glyph_node) {
      /* candidates: fontchar (font,char) whd (width,height,depth) */
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, font)) {
            lua_pushinteger(L, font(n));
        } else if (lua_key_eq(s, char)) {
            lua_pushinteger(L, character(n));
        } else if (lua_key_eq(s, xoffset)) {
            lua_pushinteger(L, x_displace(n));
        } else if (lua_key_eq(s, yoffset)) {
            lua_pushinteger(L, y_displace(n));
        } else if (lua_key_eq(s, data)) {
            lua_pushinteger(L, glyph_node_data(n));
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, char_width(font(n),character(n)));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, char_height(font(n),character(n)));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, char_depth(font(n),character(n)));
        } else if (lua_key_eq(s, expansion_factor)) {
            lua_pushinteger(L, ex_glyph(n));
        } else if (lua_key_eq(s, components)) {
            fast_metatable_or_nil(lig_ptr(n));
        } else if (lua_key_eq(s, lang)) {
            lua_pushinteger(L, char_lang(n));
        } else if (lua_key_eq(s, left)) {
            lua_pushinteger(L, char_lhmin(n));
        } else if (lua_key_eq(s, right)) {
            lua_pushinteger(L, char_rhmin(n));
        } else if (lua_key_eq(s, uchyph)) {
            lua_pushinteger(L, char_uchyph(n));
        } else {
            lua_pushnil(L);
        }
    } else if ((t == hlist_node) || (t == vlist_node)) {
      /* candidates: whd (width,height,depth) */
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, list) || lua_key_eq(s, head)) {
            fast_metatable_or_nil_alink(list_ptr(n));
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, direction)) {
            lua_push_direction(L, box_dir(n));
        } else if (lua_key_eq(s, dir)) {
            lua_push_dir_par(L, box_dir(n));
        } else if (lua_key_eq(s, shift)) {
            lua_pushinteger(L, shift_amount(n));
        } else if (lua_key_eq(s, glue_order)) {
            lua_pushinteger(L, glue_order(n));
        } else if (lua_key_eq(s, glue_sign)) {
            lua_pushinteger(L, glue_sign(n));
        } else if (lua_key_eq(s, glue_set)) {
            lua_pushnumber(L, (double) glue_set(n)); /* float */
        } else {
            lua_pushnil(L);
        }
    } else if (t == disc_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, pre)) {
            fast_metatable_or_nil(vlink(pre_break(n)));
        } else if (lua_key_eq(s, post)) {
            fast_metatable_or_nil(vlink(post_break(n)));
        } else if (lua_key_eq(s, replace)) {
            fast_metatable_or_nil(vlink(no_break(n)));
        } else if (lua_key_eq(s, penalty)) {
            lua_pushinteger(L, disc_penalty(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == glue_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, stretch)) {
            lua_pushinteger(L, stretch(n));
        } else if (lua_key_eq(s, shrink)) {
            lua_pushinteger(L, shrink(n));
        } else if (lua_key_eq(s, stretch_order)) {
            lua_pushinteger(L, stretch_order(n));
        } else if (lua_key_eq(s, shrink_order)) {
            lua_pushinteger(L, shrink_order(n));
        } else if (lua_key_eq(s, leader)) {
            fast_metatable_or_nil(leader_ptr(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == kern_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, kern)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, expansion_factor)) {
            lua_pushinteger(L, ex_kern(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == penalty_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, penalty)) {
            lua_pushinteger(L, penalty(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == rule_node) {
        /* candidates: whd (width,height,depth) */
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, left)) {
            lua_pushinteger(L,rule_left(n));
        } else if (lua_key_eq(s, right)) {
            lua_pushinteger(L,rule_right(n));
        } else if (lua_key_eq(s, direction)) {
            lua_push_direction(L, rule_dir(n));
        } else if (lua_key_eq(s, dir)) {
            lua_push_dir_par(L, rule_dir(n));
        } else if (lua_key_eq(s, index)) {
            lua_pushinteger(L, rule_index(n));
        } else if (lua_key_eq(s, transform)) {
            lua_pushinteger(L,rule_transform(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == dir_node) {
        if (lua_key_eq(s, direction)) {
            lua_push_direction(L, dir_dir(n));
        } else if (lua_key_eq(s, dir)) {
            lua_push_dir_text(L, dir_dir(n),subtype(n));
        } else if (lua_key_eq(s, level)) {
            lua_pushinteger(L, dir_level(n));
        } else if (lua_key_eq(s, subtype)) { /* can be used for anything */
            lua_pushinteger(L, subtype(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == local_par_node) {
        if (lua_key_eq(s, pen_inter)) {
            lua_pushinteger(L, local_pen_inter(n));
        } else if (lua_key_eq(s, pen_broken)) {
            lua_pushinteger(L, local_pen_broken(n));
        } else if (lua_key_eq(s, direction)) {
            lua_push_direction(L, local_par_dir(n));
        } else if (lua_key_eq(s, dir)) {
            lua_push_dir_par(L, local_par_dir(n));
        } else if (lua_key_eq(s, box_left)) {
            /* can be: fast_metatable_or_nil(local_box_left(n)) */
            nodelib_pushlist(L, local_box_left(n));
        } else if (lua_key_eq(s, box_left_width)) {
            lua_pushinteger(L, local_box_left_width(n));
        } else if (lua_key_eq(s, box_right)) {
            /* can be: fast_metatable_or_nil(local_box_right(n)) */
            nodelib_pushlist(L, local_box_right(n));
        } else if (lua_key_eq(s, box_right_width)) {
            lua_pushinteger(L, local_box_right_width(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == glue_spec_node) {
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, stretch)) {
            lua_pushinteger(L, stretch(n));
        } else if (lua_key_eq(s, shrink)) {
            lua_pushinteger(L, shrink(n));
        } else if (lua_key_eq(s, stretch_order)) {
            lua_pushinteger(L, stretch_order(n));
        } else if (lua_key_eq(s, shrink_order)) {
            lua_pushinteger(L, shrink_order(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == whatsit_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else {
            lua_nodelib_getfield_whatsit(L, n, s);
        }
    } else if (t == simple_noad) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, nucleus)) {
            fast_metatable_or_nil(nucleus(n));
        } else if (lua_key_eq(s, sub)) {
            fast_metatable_or_nil(subscr(n));
        } else if (lua_key_eq(s, sup)) {
            fast_metatable_or_nil(supscr(n));
        } else if (lua_key_eq(s, options)) {
            lua_pushinteger(L, noadoptions(n));
        } else {
            lua_pushnil(L);
        }
    } else if ((t == math_char_node) || (t == math_text_char_node)) {
        /* candidates: famchar (fam,char) */
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, fam)) {
            lua_pushinteger(L, math_fam(n));
        } else if (lua_key_eq(s, char)) {
            lua_pushinteger(L, math_character(n));
        } else if (lua_key_eq(s, font)) {
            lua_pushinteger(L, fam_fnt(math_fam(n), 0));
        } else {
            lua_pushnil(L);
        }
    } else if (t == mark_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, class)) {
            lua_pushinteger(L, mark_class(n));
        } else if (lua_key_eq(s, mark)) {
            tokenlist_to_lua(L, mark_ptr(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == ins_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, cost)) {
            lua_pushinteger(L, float_cost(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))) { /* already mapped */
            fast_metatable_or_nil_alink(ins_ptr(n));
        /* glue parameters */
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, stretch)) {
            lua_pushinteger(L, stretch(n));
        } else if (lua_key_eq(s, shrink)) {
            lua_pushinteger(L, shrink(n));
        } else if (lua_key_eq(s, stretch_order)) {
            lua_pushinteger(L, stretch_order(n));
        } else if (lua_key_eq(s, shrink_order)) {
            lua_pushinteger(L, shrink_order(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == math_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, surround)) {
            lua_pushinteger(L, surround(n));
        /* glue parameters */
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, stretch)) {
            lua_pushinteger(L, stretch(n));
        } else if (lua_key_eq(s, shrink)) {
            lua_pushinteger(L, shrink(n));
        } else if (lua_key_eq(s, stretch_order)) {
            lua_pushinteger(L, stretch_order(n));
        } else if (lua_key_eq(s, shrink_order)) {
            lua_pushinteger(L, shrink_order(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == fraction_noad) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, thickness(n));
        } else if (lua_key_eq(s, num)) {
            fast_metatable_or_nil(numerator(n));
        } else if (lua_key_eq(s, denom)) {
            fast_metatable_or_nil(denominator(n));
        } else if (lua_key_eq(s, left)) {
            fast_metatable_or_nil(left_delimiter(n));
        } else if (lua_key_eq(s, right)) {
            fast_metatable_or_nil(right_delimiter(n));
        } else if (lua_key_eq(s, middle)) {
            fast_metatable_or_nil(middle_delimiter(n));
        } else if (lua_key_eq(s, fam)) {
            lua_pushinteger(L, fraction_fam(n));
        } else if (lua_key_eq(s, options)) {
            lua_pushinteger(L, fractionoptions(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == style_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, style)) {
            lua_push_math_style_name(L,subtype(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == accent_noad) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, nucleus)) {
            fast_metatable_or_nil(nucleus(n));
        } else if (lua_key_eq(s, sub)) {
            fast_metatable_or_nil(subscr(n));
        } else if (lua_key_eq(s, sup)) {
            fast_metatable_or_nil(supscr(n));
        } else if ((lua_key_eq(s, top_accent))||(lua_key_eq(s, accent))) {
            fast_metatable_or_nil(top_accent_chr(n));
        } else if (lua_key_eq(s, bot_accent)) {
            fast_metatable_or_nil(bot_accent_chr(n));
        } else if (lua_key_eq(s, overlay_accent)) {
            fast_metatable_or_nil(overlay_accent_chr(n));
        } else if (lua_key_eq(s, fraction)) {
            lua_pushinteger(L, accentfraction(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == fence_noad) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, delim)) {
            fast_metatable_or_nil(delimiter(n));
        } else if (lua_key_eq(s, italic)) {
            lua_pushinteger(L, delimiteritalic(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, delimiterheight(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, delimiterdepth(n));
        } else if (lua_key_eq(s, options)) {
            lua_pushinteger(L, delimiteroptions(n));
        } else if (lua_key_eq(s, class)) {
            lua_pushinteger(L, delimiterclass(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == delim_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, small_fam)) {
            lua_pushinteger(L, small_fam(n));
        } else if (lua_key_eq(s, small_char)) {
            lua_pushinteger(L, small_char(n));
        } else if (lua_key_eq(s, large_fam)) {
            lua_pushinteger(L, large_fam(n));
        } else if (lua_key_eq(s, large_char)) {
            lua_pushinteger(L, large_char(n));
        } else {
            lua_pushnil(L);
        }
    } else if ((t == sub_box_node) || (t == sub_mlist_node)) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))){
            fast_metatable_or_nil_alink(math_list(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == radical_noad) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, nucleus)) {
            fast_metatable_or_nil(nucleus(n));
        } else if (lua_key_eq(s, sub)) {
            fast_metatable_or_nil(subscr(n));
        } else if (lua_key_eq(s, sup)) {
            fast_metatable_or_nil(supscr(n));
        } else if (lua_key_eq(s, left)) {
            fast_metatable_or_nil(left_delimiter(n));
        } else if (lua_key_eq(s, degree)) {
            fast_metatable_or_nil(degree(n));
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, radicalwidth(n));
        } else if (lua_key_eq(s, options)) {
            lua_pushinteger(L, radicaloptions(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == margin_kern_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, glyph)) {
            fast_metatable_or_nil(margin_char(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == split_up_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, last_ins_ptr)) {
            fast_metatable_or_nil(last_ins_ptr(n));
        } else if (lua_key_eq(s, best_ins_ptr)) {
            fast_metatable_or_nil(best_ins_ptr(n));
        } else if (lua_key_eq(s, broken_ptr)) {
            fast_metatable_or_nil(broken_ptr(n));
        } else if (lua_key_eq(s, broken_ins)) {
            fast_metatable_or_nil(broken_ins(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == choice_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, display)) {
            fast_metatable_or_nil(display_mlist(n));
        } else if (lua_key_eq(s, text)) {
            fast_metatable_or_nil(text_mlist(n));
        } else if (lua_key_eq(s, script)) {
            fast_metatable_or_nil(script_mlist(n));
        } else if (lua_key_eq(s, scriptscript)) {
            fast_metatable_or_nil(script_script_mlist(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == inserting_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, last_ins_ptr)) {
            fast_metatable_or_nil(last_ins_ptr(n));
        } else if (lua_key_eq(s, best_ins_ptr)) {
            fast_metatable_or_nil(best_ins_ptr(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == attribute_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, number)) {
            lua_pushinteger(L, attribute_id(n));
        } else if (lua_key_eq(s, value)) {
            lua_pushinteger(L, attribute_value(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == adjust_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))) {
            fast_metatable_or_nil_alink(adjust_ptr(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == unset_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, direction)) {
            lua_push_direction(L, box_dir(n));
        } else if (lua_key_eq(s, dir)) {
            lua_push_dir_par(L, box_dir(n));
        } else if (lua_key_eq(s, shrink)) {
            lua_pushinteger(L, glue_shrink(n));
        } else if (lua_key_eq(s, glue_order)) {
            lua_pushinteger(L, glue_order(n));
        } else if (lua_key_eq(s, glue_sign)) {
            lua_pushinteger(L, glue_sign(n));
        } else if (lua_key_eq(s, stretch)) {
            lua_pushinteger(L, glue_stretch(n));
        } else if (lua_key_eq(s, count)) {
            lua_pushinteger(L, span_count(n));
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))){
            fast_metatable_or_nil_alink(list_ptr(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == attribute_list_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == boundary_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, value)) {
            lua_pushinteger(L, boundary_value(n));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_nodelib_getfield(lua_State * L)
{
    /* [given-node] [...]*/
    halfword *p = lua_touserdata(L, 1);
    if ( (p == NULL) || (! lua_getmetatable(L,1)) ) {
        lua_pushnil(L) ;
        return 1;
    }
    /* [given-node] [mt-given-node]*/
    lua_get_metatablelua(luatex_node);
    /* [given-node] [mt-given-node] [mt-node]*/
    if (!lua_rawequal(L, -1, -2)) {
        lua_pushnil(L) ;
        return 1;
    }
    /* prune stack and call getfield */
    lua_settop(L,2);
    return lua_nodelib_fast_getfield(L);
}

/* node.direct.getfield */

static void lua_nodelib_direct_getfield_whatsit(lua_State * L, int n, const char *s)
{
    int t = subtype(n);
    if (t == user_defined_node) {
        if (lua_key_eq(s, user_id)) {
            lua_pushinteger(L, user_node_id(n));
        } else if (lua_key_eq(s, type)) {
            lua_pushinteger(L, user_node_type(n));
        } else if (lua_key_eq(s, value)) {
            get_user_node_direct_value(L, n);
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_literal_node) {
        /*tex The |string| key is obsolete. */
        if (lua_key_eq(s, mode)) {
            lua_pushinteger(L, pdf_literal_mode(n));
        } else if (lua_key_eq(s, data) || lua_key_eq(s, token) || lua_key_eq(s, string)) {
            get_pdf_literal_direct_value(L, n);
        } else {
            lua_pushnil(L);
        }
    } else if (t == late_lua_node) {
        /*tex The |string| key is obsolete. */
        if (lua_key_eq(s, data) || lua_key_eq(s, token) || lua_key_eq(s, string)) {
            get_late_lua_direct_value(L,n);
        } else if (lua_key_eq(s, name)) {
            tokenlist_to_luastring(L, late_lua_name(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_annot_node) {
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, objnum)) {
            lua_pushinteger(L, pdf_annot_objnum(n));
        } else if (lua_key_eq(s, data)) {
            tokenlist_to_luastring(L, pdf_annot_data(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_dest_node) {
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, named_id)) {
            lua_pushinteger(L, pdf_dest_named_id(n));
        } else if (lua_key_eq(s, dest_id)) {
            if (pdf_dest_named_id(n) == 1)
                tokenlist_to_luastring(L, pdf_dest_id(n));
            else
                lua_pushinteger(L, pdf_dest_id(n));
        } else if (lua_key_eq(s, dest_type)) {
            lua_pushinteger(L, pdf_dest_type(n));
        } else if (lua_key_eq(s, xyz_zoom)) {
            lua_pushinteger(L, pdf_dest_xyz_zoom(n));
        } else if (lua_key_eq(s, objnum)) {
            lua_pushinteger(L, pdf_dest_objnum(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_setmatrix_node) {
        if (lua_key_eq(s, data)) {
            get_pdf_setmatrix_direct_value(L,n);
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_colorstack_node) {
        if (lua_key_eq(s, stack)) {
            lua_pushinteger(L, pdf_colorstack_stack(n));
        } else if (lua_key_eq(s, command)) {
            lua_pushinteger(L, pdf_colorstack_cmd(n));
        } else if (lua_key_eq(s, data)) {
            tokenlist_to_luastring(L, pdf_colorstack_data(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_refobj_node) {
        if (lua_key_eq(s, objnum)) {
            lua_pushinteger(L, pdf_obj_objnum(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == write_node) {
        if (lua_key_eq(s, stream)) {
            lua_pushinteger(L, write_stream(n));
        } else if (lua_key_eq(s, data)) {
            get_write_direct_value(L,n);
        } else {
            lua_pushnil(L);
        }
    } else if (t == special_node) {
        if (lua_key_eq(s, data)) {
            get_special_direct_value(L,n);
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_start_link_node) {
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, objnum)) {
            lua_pushinteger(L, pdf_link_objnum(n));
        } else if (lua_key_eq(s, link_attr)) {
            tokenlist_to_luastring(L, pdf_link_attr(n));
        } else if (lua_key_eq(s, action)) {
            nodelib_pushaction(L, pdf_link_action(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == pdf_action_node) {
        if (lua_key_eq(s, action_type)) {
            lua_pushinteger(L, pdf_action_type(n));
        } else if (lua_key_eq(s, named_id)) {
            lua_pushinteger(L, pdf_action_named_id(n));
        } else if (lua_key_eq(s, action_id)) {
            if (pdf_action_named_id(n) == 1) {
                tokenlist_to_luastring(L, pdf_action_id(n));
            } else {
                lua_pushinteger(L, pdf_action_id(n));
            }
        } else if (lua_key_eq(s, file)) {
            tokenlist_to_luastring(L, pdf_action_file(n));
        } else if (lua_key_eq(s, new_window)) {
            lua_pushinteger(L, pdf_action_new_window(n));
        } else if (lua_key_eq(s, data)) {
            tokenlist_to_luastring(L, pdf_action_tokens(n));
        } else {
            lua_pushnil(L);
        }
    } else if ((t == pdf_thread_node) || (t == pdf_start_thread_node)) {
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, named_id)) {
            lua_pushinteger(L, pdf_thread_named_id(n));
        } else if (lua_key_eq(s, thread_id)) {
            if (pdf_thread_named_id(n) == 1) {
                tokenlist_to_luastring(L, pdf_thread_id(n));
            } else {
                lua_pushinteger(L, pdf_thread_id(n));
            }
        } else if (lua_key_eq(s, thread_attr)) {
            tokenlist_to_luastring(L, pdf_thread_attr(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == open_node) {
        if (lua_key_eq(s, stream)) {
            lua_pushinteger(L, write_stream(n));
        } else if (lua_key_eq(s, name)) {
            nodelib_pushstring(L, open_name(n));
        } else if (lua_key_eq(s, area)) {
            nodelib_pushstring(L, open_area(n));
        } else if (lua_key_eq(s, ext)) {
            nodelib_pushstring(L, open_ext(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == close_node) {
        if (lua_key_eq(s, stream)) {
            lua_pushinteger(L, write_stream(n));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
}

static int lua_nodelib_direct_getfield(lua_State * L)
{

    const char *s;
    halfword n = lua_tointeger(L, 1);
    int t = type(n);
    if (lua_type(L, 2) == LUA_TNUMBER) {
        halfword p;
        int i;
        if (! nodetype_has_attributes(t)) {
            lua_pushnil(L) ;
            return 1;
        }
        p = node_attr(n);
        if (p == null || vlink(p) == null) {
            lua_pushnil(L) ;
            return 1;
        }
        i = (int) lua_tointeger(L, 2);
        p = vlink(p);
        while (p != null) {
            if (attribute_id(p) == i) {
                if ((int) attribute_value(p) > UNUSED_ATTRIBUTE) {
                    lua_pushinteger(L, (int) attribute_value(p));
                } else {
                    lua_pushnil(L);
                }
                return 1;
            } else if (attribute_id(p) > i) {
                lua_pushnil(L) ;
                return 1;
            }
            p = vlink(p);
        }
        lua_pushnil(L) ;
        return 1;
    }

    s = lua_tostring(L, 2);

    if (lua_key_eq(s, id)) {
        lua_pushinteger(L, t);
    } else if (lua_key_eq(s, next)) {
        nodelib_pushdirect_or_nil(vlink(n));
    } else if (lua_key_eq(s, prev)) {
        nodelib_pushdirect_or_nil(alink(n));
    } else if (lua_key_eq(s, subtype)) {
        if (t == glue_spec_node) {
            lua_pushinteger(L, 0); /* dummy, the only one */
        } else {
            lua_pushinteger(L, subtype(n));
        }
    } else if (lua_key_eq(s, attr)) {
        if (! nodetype_has_attributes(t)) {
            lua_pushnil(L);
        } else {
         /* nodelib_pushattr(L, node_attr(n)); */
            lua_pushinteger(L, node_attr(n));
        }
    } else if (t == glyph_node) {
        if (lua_key_eq(s, font)) {
            lua_pushinteger(L, font(n));
        } else if (lua_key_eq(s, char)) {
            lua_pushinteger(L, character(n));
        } else if (lua_key_eq(s, xoffset)) {
            lua_pushinteger(L, x_displace(n));
        } else if (lua_key_eq(s, yoffset)) {
            lua_pushinteger(L, y_displace(n));
        } else if (lua_key_eq(s, data)) {
            lua_pushinteger(L, glyph_node_data(n));
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, char_width(font(n),character(n)));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, char_height(font(n),character(n)));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, char_depth(font(n),character(n)));
        } else if (lua_key_eq(s, expansion_factor)) {
            lua_pushinteger(L, ex_glyph(n));
        } else if (lua_key_eq(s, components)) {
            nodelib_pushdirect_or_nil(lig_ptr(n));
        } else if (lua_key_eq(s, lang)) {
            lua_pushinteger(L, char_lang(n));
        } else if (lua_key_eq(s, left)) {
            lua_pushinteger(L, char_lhmin(n));
        } else if (lua_key_eq(s, right)) {
            lua_pushinteger(L, char_rhmin(n));
        } else if (lua_key_eq(s, uchyph)) {
            lua_pushinteger(L, char_uchyph(n));
        } else {
            lua_pushnil(L);
        }
    } else if ((t == hlist_node) || (t == vlist_node)) {
        /* candidates: whd (width,height,depth) */
        if (lua_key_eq(s, list) || lua_key_eq(s, head)) {
            nodelib_pushdirect_or_nil_alink(list_ptr(n));
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, direction)) {
            lua_push_direction(L, box_dir(n));
        } else if (lua_key_eq(s, dir)) {
            lua_push_dir_par(L, box_dir(n));
        } else if (lua_key_eq(s, shift)) {
            lua_pushinteger(L, shift_amount(n));
        } else if (lua_key_eq(s, glue_order)) {
            lua_pushinteger(L, glue_order(n));
        } else if (lua_key_eq(s, glue_sign)) {
            lua_pushinteger(L, glue_sign(n));
        } else if (lua_key_eq(s, glue_set)) {
            lua_pushnumber(L, (double) glue_set(n)); /* float */
        } else {
            lua_pushnil(L);
        }
    } else if (t == disc_node) {
        if (lua_key_eq(s, pre)) {
            nodelib_pushdirect_or_nil(vlink(pre_break(n)));
        } else if (lua_key_eq(s, post)) {
            nodelib_pushdirect_or_nil(vlink(post_break(n)));
        } else if (lua_key_eq(s, replace)) {
            nodelib_pushdirect_or_nil(vlink(no_break(n)));
        } else if (lua_key_eq(s, penalty)) {
            lua_pushinteger(L, disc_penalty(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == glue_node) {
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, stretch)) {
            lua_pushinteger(L, stretch(n));
        } else if (lua_key_eq(s, shrink)) {
            lua_pushinteger(L, shrink(n));
        } else if (lua_key_eq(s, stretch_order)) {
            lua_pushinteger(L, stretch_order(n));
        } else if (lua_key_eq(s, shrink_order)) {
            lua_pushinteger(L, shrink_order(n));
        } else if (lua_key_eq(s, leader)) {
            nodelib_pushdirect_or_nil(leader_ptr(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == kern_node) {
        if (lua_key_eq(s, kern)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, expansion_factor)) {
            lua_pushinteger(L, ex_kern(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == penalty_node) {
        if (lua_key_eq(s, penalty)) {
            lua_pushinteger(L, penalty(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == rule_node) {
        /* candidates: whd (width,height,depth) */
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, left)) {
            lua_pushinteger(L,rule_left(n));
        } else if (lua_key_eq(s, right)) {
            lua_pushinteger(L,rule_right(n));
        } else if (lua_key_eq(s, direction)) {
            lua_push_direction(L, rule_dir(n));
        } else if (lua_key_eq(s, dir)) {
            lua_push_dir_par(L, rule_dir(n));
        } else if (lua_key_eq(s, index)) {
            lua_pushinteger(L, rule_index(n));
        } else if (lua_key_eq(s, transform)) {
            lua_pushinteger(L,rule_transform(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == dir_node) {
        if (lua_key_eq(s, direction)) {
            lua_push_direction(L, dir_dir(n));
        } else if (lua_key_eq(s, dir)) {
            lua_push_dir_text(L, dir_dir(n), subtype(n));
        } else if (lua_key_eq(s, level)) {
            lua_pushinteger(L, dir_level(n));
        } else if (lua_key_eq(s, subtype)) { /* can be used for anything */
            lua_pushinteger(L, subtype(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == local_par_node) {
        if (lua_key_eq(s, pen_inter)) {
            lua_pushinteger(L, local_pen_inter(n));
        } else if (lua_key_eq(s, pen_broken)) {
            lua_pushinteger(L, local_pen_broken(n));
        } else if (lua_key_eq(s, direction)) {
            lua_push_direction(L, local_par_dir(n));
        } else if (lua_key_eq(s, dir)) {
            lua_push_dir_par(L, local_par_dir(n));
        } else if (lua_key_eq(s, box_left)) {
            nodelib_pushdirect_or_nil(local_box_left(n));
        } else if (lua_key_eq(s, box_left_width)) {
            lua_pushinteger(L, local_box_left_width(n));
        } else if (lua_key_eq(s, box_right)) {
            nodelib_pushdirect_or_nil(local_box_right(n));
        } else if (lua_key_eq(s, box_right_width)) {
            lua_pushinteger(L, local_box_right_width(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == whatsit_node) {
        lua_nodelib_direct_getfield_whatsit(L, n, s);
    } else if (t == simple_noad) {
        if (lua_key_eq(s, nucleus)) {
            nodelib_pushdirect_or_nil(nucleus(n));
        } else if (lua_key_eq(s, sub)) {
            nodelib_pushdirect_or_nil(subscr(n));
        } else if (lua_key_eq(s, sup)) {
            nodelib_pushdirect_or_nil(supscr(n));
        } else if (lua_key_eq(s, options)) {
            lua_pushinteger(L, noadoptions(n));
        } else {
            lua_pushnil(L);
        }
    } else if ((t == math_char_node) || (t == math_text_char_node)) {
        if (lua_key_eq(s, fam)) {
            lua_pushinteger(L, math_fam(n));
        } else if (lua_key_eq(s, char)) {
            lua_pushinteger(L, math_character(n));
        } else if (lua_key_eq(s, font)) {
            lua_pushinteger(L, fam_fnt(math_fam(n), 0));
        } else {
            lua_pushnil(L);
        }
    } else if (t == mark_node) {
        if (lua_key_eq(s, class)) {
            lua_pushinteger(L, mark_class(n));
        } else if (lua_key_eq(s, mark)) {
            tokenlist_to_lua(L, mark_ptr(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == ins_node) {
        if (lua_key_eq(s, cost)) {
            lua_pushinteger(L, float_cost(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))) {
            nodelib_pushdirect_or_nil_alink(ins_ptr(n));
        /* glue */
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, stretch)) {
            lua_pushinteger(L, stretch(n));
        } else if (lua_key_eq(s, shrink)) {
            lua_pushinteger(L, shrink(n));
        } else if (lua_key_eq(s, stretch_order)) {
            lua_pushinteger(L, stretch_order(n));
        } else if (lua_key_eq(s, shrink_order)) {
            lua_pushinteger(L, shrink_order(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == math_node) {
        if (lua_key_eq(s, surround)) {
            lua_pushinteger(L, surround(n));
        /* glue */
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, stretch)) {
            lua_pushinteger(L, stretch(n));
        } else if (lua_key_eq(s, shrink)) {
            lua_pushinteger(L, shrink(n));
        } else if (lua_key_eq(s, stretch_order)) {
            lua_pushinteger(L, stretch_order(n));
        } else if (lua_key_eq(s, shrink_order)) {
            lua_pushinteger(L, shrink_order(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == fraction_noad) {
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, thickness(n));
        } else if (lua_key_eq(s, num)) {
            nodelib_pushdirect_or_nil(numerator(n));
        } else if (lua_key_eq(s, denom)) {
            nodelib_pushdirect_or_nil(denominator(n));
        } else if (lua_key_eq(s, left)) {
            nodelib_pushdirect_or_nil(left_delimiter(n));
        } else if (lua_key_eq(s, right)) {
            nodelib_pushdirect_or_nil(right_delimiter(n));
        } else if (lua_key_eq(s, middle)) {
            nodelib_pushdirect_or_nil(middle_delimiter(n));
        } else if (lua_key_eq(s, fam)) {
            lua_pushinteger(L, fraction_fam(n));
        } else if (lua_key_eq(s, options)) {
            lua_pushinteger(L, fractionoptions(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == style_node) {
        if (lua_key_eq(s, style)) {
            lua_push_math_style_name(L,subtype(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == accent_noad) {
        if (lua_key_eq(s, nucleus)) {
            nodelib_pushdirect_or_nil(nucleus(n));
        } else if (lua_key_eq(s, sub)) {
            nodelib_pushdirect_or_nil(subscr(n));
        } else if (lua_key_eq(s, sup)) {
            nodelib_pushdirect_or_nil(supscr(n));
        } else if ((lua_key_eq(s, top_accent))||(lua_key_eq(s, accent))) {
            nodelib_pushdirect_or_nil(top_accent_chr(n));
        } else if (lua_key_eq(s, bot_accent)) {
            nodelib_pushdirect_or_nil(bot_accent_chr(n));
        } else if (lua_key_eq(s, overlay_accent)) {
            nodelib_pushdirect_or_nil(overlay_accent_chr(n));
        } else if (lua_key_eq(s, fraction)) {
            lua_pushinteger(L, accentfraction(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == fence_noad) {
        if (lua_key_eq(s, delim)) {
            nodelib_pushdirect_or_nil(delimiter(n));
        } else if (lua_key_eq(s, italic)) {
            lua_pushinteger(L, delimiteritalic(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, delimiterheight(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, delimiterdepth(n));
        } else if (lua_key_eq(s, options)) {
            lua_pushinteger(L, delimiteroptions(n));
        } else if (lua_key_eq(s, class)) {
            lua_pushinteger(L, delimiterclass(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == delim_node) {
        if (lua_key_eq(s, small_fam)) {
            lua_pushinteger(L, small_fam(n));
        } else if (lua_key_eq(s, small_char)) {
            lua_pushinteger(L, small_char(n));
        } else if (lua_key_eq(s, large_fam)) {
            lua_pushinteger(L, large_fam(n));
        } else if (lua_key_eq(s, large_char)) {
            lua_pushinteger(L, large_char(n));
        } else {
            lua_pushnil(L);
        }
    } else if ((t == sub_box_node) || (t == sub_mlist_node)) {
        if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))){
            nodelib_pushdirect_or_nil_alink(math_list(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == radical_noad) {
        if (lua_key_eq(s, nucleus)) {
            nodelib_pushdirect_or_nil(nucleus(n));
        } else if (lua_key_eq(s, sub)) {
            nodelib_pushdirect_or_nil(subscr(n));
        } else if (lua_key_eq(s, sup)) {
            nodelib_pushdirect_or_nil(supscr(n));
        } else if (lua_key_eq(s, left)) {
            nodelib_pushdirect_or_nil(left_delimiter(n));
        } else if (lua_key_eq(s, degree)) {
            nodelib_pushdirect_or_nil(degree(n));
        } else if (lua_key_eq(s, width)) {
            lua_pushinteger(L, radicalwidth(n));
        } else if (lua_key_eq(s, options)) {
            lua_pushinteger(L, radicaloptions(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == margin_kern_node) {
        if (lua_key_eq(s, kern) ||lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, glyph)) {
            nodelib_pushdirect_or_nil(margin_char(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == split_up_node) {
        if (lua_key_eq(s, last_ins_ptr)) {
            nodelib_pushdirect_or_nil(last_ins_ptr(n));
        } else if (lua_key_eq(s, best_ins_ptr)) {
            nodelib_pushdirect_or_nil(best_ins_ptr(n));
        } else if (lua_key_eq(s, broken_ptr)) {
            nodelib_pushdirect_or_nil(broken_ptr(n));
        } else if (lua_key_eq(s, broken_ins)) {
            nodelib_pushdirect_or_nil(broken_ins(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == choice_node) {
        if (lua_key_eq(s, display)) {
            nodelib_pushdirect_or_nil(display_mlist(n));
        } else if (lua_key_eq(s, text)) {
            nodelib_pushdirect_or_nil(text_mlist(n));
        } else if (lua_key_eq(s, script)) {
            nodelib_pushdirect_or_nil(script_mlist(n));
        } else if (lua_key_eq(s, scriptscript)) {
            nodelib_pushdirect_or_nil(script_script_mlist(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == inserting_node) {
        if (lua_key_eq(s, last_ins_ptr)) {
            nodelib_pushdirect_or_nil(last_ins_ptr(n));
        } else if (lua_key_eq(s, best_ins_ptr)) {
            nodelib_pushdirect_or_nil(best_ins_ptr(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == attribute_node) {
        if (lua_key_eq(s, number)) {
            lua_pushinteger(L, attribute_id(n));
        } else if (lua_key_eq(s, value)) {
            lua_pushinteger(L, attribute_value(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == adjust_node) {
        if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))) {
            nodelib_pushdirect_or_nil_alink(adjust_ptr(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == unset_node) {
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, height)) {
            lua_pushinteger(L, height(n));
        } else if (lua_key_eq(s, depth)) {
            lua_pushinteger(L, depth(n));
        } else if (lua_key_eq(s, direction)) {
            lua_push_direction(L, box_dir(n));
        } else if (lua_key_eq(s, dir)) {
            lua_push_dir_par(L, box_dir(n));
        } else if (lua_key_eq(s, shrink)) {
            lua_pushinteger(L, glue_shrink(n));
        } else if (lua_key_eq(s, glue_order)) {
            lua_pushinteger(L, glue_order(n));
        } else if (lua_key_eq(s, glue_sign)) {
            lua_pushinteger(L, glue_sign(n));
        } else if (lua_key_eq(s, stretch)) {
            lua_pushinteger(L, glue_stretch(n));
        } else if (lua_key_eq(s, count)) {
            lua_pushinteger(L, span_count(n));
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))){
            nodelib_pushdirect_or_nil_alink(list_ptr(n));
        } else {
            lua_pushnil(L);
        }
 /* } else if (t == attribute_list_node) { */
 /*     lua_pushnil(L); */
    } else if (t == boundary_node) {
        if (lua_key_eq(s, subtype)) {
            lua_pushinteger(L, subtype(n));
        } else if (lua_key_eq(s, value)) {
            lua_pushinteger(L, boundary_value(n));
        } else {
            lua_pushnil(L);
        }
    } else if (t == glue_spec_node) {
        if (lua_key_eq(s, width)) {
            lua_pushinteger(L, width(n));
        } else if (lua_key_eq(s, stretch)) {
            lua_pushinteger(L, stretch(n));
        } else if (lua_key_eq(s, shrink)) {
            lua_pushinteger(L, shrink(n));
        } else if (lua_key_eq(s, stretch_order)) {
            lua_pushinteger(L, stretch_order(n));
        } else if (lua_key_eq(s, shrink_order)) {
            lua_pushinteger(L, shrink_order(n));
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* msg could be preallocated and shared */

static void lua_nodelib_do_tostring(lua_State * L, halfword n, const char *tag)
{
    char *msg;
    char a[7] = { ' ', ' ', ' ', 'n', 'i', 'l', 0 };
    char v[7] = { ' ', ' ', ' ', 'n', 'i', 'l', 0 };
    msg = xmalloc(256);
    if ((alink(n) != null) && (type(n) != attribute_node))
        snprintf(a, 7, "%6d", (int) alink(n));
    if (vlink(n) != null)
        snprintf(v, 7, "%6d", (int) vlink(n));
    snprintf(msg, 255, "<%s %s < %6d > %s : %s %d>", tag, a, (int) n, v, node_data[type(n)].name, subtype(n));
    lua_pushstring(L, msg);
    free(msg);
    return ;
}

/* __tostring node.tostring */

static int lua_nodelib_tostring(lua_State * L)
{
    halfword n = *check_isnode(L, 1);
    lua_nodelib_do_tostring(L, n, "node");
    return 1;
}

/* node.direct.tostring */

static int lua_nodelib_direct_tostring(lua_State * L)
{
    halfword n = lua_tointeger(L,1);
    if (n==0) {
        lua_pushnil(L);
    } else {
        lua_nodelib_do_tostring(L, n, "direct");
    }
    return 1;
}

/* __eq */

static int lua_nodelib_equal(lua_State * L)
{
    halfword n = *((halfword *) lua_touserdata(L, 1));
    halfword m = *((halfword *) lua_touserdata(L, 2));
    lua_pushboolean(L, (n == m));
    return 1;
}

/* node.ligaturing */

static int font_tex_ligaturing(lua_State * L)
{
    /* on the stack are two nodes and a direction */
    /* we need to deal with prev nodes when a range starts with a ligature */
    halfword tmp_head;
    halfword h;
    halfword t = null;
    halfword p ;
    if (lua_gettop(L) < 1) {
        lua_pushnil(L);
        lua_pushboolean(L, 0);
        return 2;
    }
    h = *check_isnode(L, 1);
    if (lua_gettop(L) > 1) {
        t = *check_isnode(L, 2);
    }
    tmp_head = new_node(nesting_node, 1);
    p = alink(h);
    couple_nodes(tmp_head, h);
    tlink(tmp_head) = t;
    t = handle_ligaturing(tmp_head, t);
    if (p != null) {
        vlink(p) = vlink(tmp_head) ;
    }
    alink(vlink(tmp_head)) = p ;
    /*
    lua_pushinteger(L, vlink(tmp_head));
    lua_nodelib_push(L);
    */
    lua_nodelib_push_fast(L, vlink(tmp_head));
    /*
    lua_pushinteger(L, t);
    lua_nodelib_push(L);
    */
    lua_nodelib_push_fast(L, t);
    lua_pushboolean(L, 1);
    flush_node(tmp_head);
    return 3;
}

static int font_tex_direct_ligaturing(lua_State * L)
{
    /* on the stack are two nodes and a direction */
    /* we need to deal with prev nodes when a range starts with a ligature */
    halfword tmp_head;
    halfword h;
    halfword t = null;
    halfword p ;
    if (lua_gettop(L) < 1) {
        lua_pushnil(L);
        lua_pushboolean(L, 0);
        return 2;
    }
    h = lua_tointeger(L, 1);
    if (lua_gettop(L) > 1) {
        t = lua_tointeger(L, 2);
    }
    tmp_head = new_node(nesting_node, 1);
    p = alink(h);
    couple_nodes(tmp_head, h);
    tlink(tmp_head) = t;
    t = handle_ligaturing(tmp_head, t);
    if (p != null) {
        vlink(p) = vlink(tmp_head) ;
    }
    alink(vlink(tmp_head)) = p ;
    lua_pushinteger(L, vlink(tmp_head));
    lua_pushinteger(L, t);
    lua_pushboolean(L, 1);
    flush_node(tmp_head);
    return 3;
}

/* node.kerning */

static int font_tex_kerning(lua_State * L)
{
    /* on the stack are two nodes and a direction */

    halfword tmp_head;
    halfword h;
    halfword t = null;
    halfword p ;
    if (lua_gettop(L) < 1) {
        lua_pushnil(L);
        lua_pushboolean(L, 0);
        return 2;
    }
    h = *check_isnode(L, 1);
    if (lua_gettop(L) > 1) {
        t = *check_isnode(L, 2);
    }
    tmp_head = new_node(nesting_node, 1);
    p = alink(h);
    couple_nodes(tmp_head, h);
    tlink(tmp_head) = t;
    t = handle_kerning(tmp_head, t);
    if (p != null) {
        vlink(p) = vlink(tmp_head) ;
    }
    alink(vlink(tmp_head)) = p ;
    /*
    lua_pushinteger(L, vlink(tmp_head));
    lua_nodelib_push(L);
    */
    lua_nodelib_push_fast(L, vlink(tmp_head));
    /*
    lua_pushinteger(L, t);
    lua_nodelib_push(L);
    */
    lua_nodelib_push_fast(L, t);
    lua_pushboolean(L, 1);
    flush_node(tmp_head);
    return 3;
}

static int font_tex_direct_kerning(lua_State * L)
{
    /* on the stack are two nodes and a direction */

    halfword tmp_head;
    halfword h;
    halfword t = null;
    halfword p ;
    if (lua_gettop(L) < 1) {
        lua_pushnil(L);
        lua_pushboolean(L, 0);
        return 2;
    }
    h = lua_tointeger(L, 1);
    if (lua_gettop(L) > 1) {
        t = lua_tointeger(L, 2);
    }
    tmp_head = new_node(nesting_node, 1);
    p = alink(h);
    couple_nodes(tmp_head, h);
    tlink(tmp_head) = t;
    t = handle_kerning(tmp_head, t);
    if (p != null) {
        vlink(p) = vlink(tmp_head) ;
    }
    alink(vlink(tmp_head)) = p ;
    lua_pushinteger(L, vlink(tmp_head));
    if (t == null) {
        lua_pushnil(L);
    } else {
        lua_pushinteger(L, t);
    }
    lua_pushboolean(L, 1);
    flush_node(tmp_head);
    return 3;
}

/*tex
    It's more consistent to have it here (so we will alias in lang later). Todo:
    if no glyph then quit.
*/

static int lang_tex_hyphenating(lua_State * L)
{
    halfword t = null;
    halfword h = *check_isnode(L, 1);
    if (lua_isuserdata(L, 2)) {
        t = *check_isnode(L, 2);
    }
    if (t == null) {
        t = h;
        while (vlink(t) != null) {
            t = vlink(t);
        }
    }
    hnj_hyphenation(h, t);
    lua_nodelib_push_fast(L, h);
    lua_nodelib_push_fast(L, t);
    lua_pushboolean(L, 1);
    return 3;
}

static int lang_tex_direct_hyphenating(lua_State * L)
{
    halfword t = null;
    halfword h = lua_tointeger(L, 1);
    if (lua_type(L,1) != LUA_TNUMBER) {
        t = lua_tointeger(L, 2);
    }
    if (t == null) {
        t = h;
        while (vlink(t) != null) {
            t = vlink(t);
        }
    }
    hnj_hyphenation(h, t);
    lua_pushinteger(L, h);
    lua_pushinteger(L, t);
    lua_pushboolean(L, 1);
    return 3;
}

/* node.protect_glyphs (returns also boolean because that signals callback) */
/* node.unprotect_glyphs (returns also boolean because that signals callback) */

#define protect_one_indeed(n) \
    if (n != null) { \
        int s = subtype(n); \
        if (s <= 256) { \
            subtype(n) = (quarterword) (s == 1 ? 256 : 256 + s); \
        } \
    }

#define protect_all_indeed(n) \
    h = n ; \
    if (h != null) { \
        while (h != null) { \
            if (type(h) == glyph_node) { \
                int s = subtype(h); \
                if (s <= 256) { \
                    subtype(h) = (quarterword) (s == 1 ? 256 : 256 + s); \
                } \
            } \
            h = vlink(h); \
        } \
    }

#define protect_node_indeed(n) \
    if (type(n) == glyph_node) { \
        protect_one_indeed(n); \
    } else if (type(n) == disc_node) { \
        protect_all_indeed(vlink(no_break(n))); \
        protect_all_indeed(vlink(pre_break(n))); \
        protect_all_indeed(vlink(post_break(n))); \
    }

#define unprotect_one_indeed(n) \
    if (n != null) { \
        int s = subtype(n); \
        if (s > 256) { \
            subtype(n) = (quarterword) (s - 256); \
        } \
    }

#define unprotect_all_indeed(n) \
    h = n ; \
    if (h != null) { \
        while (h != null) { \
            if (type(h) == glyph_node) { \
                int s = subtype(h); \
                if (s <= 256) { \
                    subtype(h) = (quarterword) (s - 256); \
                } \
            } \
            h = vlink(h); \
        } \
    }

#define unprotect_node_indeed(n) \
    if (type(n) == glyph_node) { \
        unprotect_one_indeed(n); \
    } else if (type(n) == disc_node) { \
        unprotect_all_indeed(vlink(no_break(n))); \
        unprotect_all_indeed(vlink(pre_break(n))); \
        unprotect_all_indeed(vlink(post_break(n))); \
    }

static int lua_nodelib_protect_glyph(lua_State * L)
{
    halfword n = *check_isnode(L, 1);
    halfword h;
    protect_node_indeed(n);
    return 0;
}

static int lua_nodelib_unprotect_glyph(lua_State * L)
{
    halfword n = *check_isnode(L, 1);
    halfword h;
    unprotect_node_indeed(n);
    return 0;
}

static int lua_nodelib_protect_glyphs(lua_State * L)
{
    halfword head = *check_isnode(L, 1);
    halfword tail = null;
    halfword h;
    if (lua_gettop(L) > 1) {
        tail = *check_isnode(L, 2);
    }
    while (head != null) {
        protect_node_indeed(head);
        if (head == tail) {
            break;
        }
        head = vlink(head);
    }
    return 0;
}

static int lua_nodelib_unprotect_glyphs(lua_State * L)
{
    halfword head = *check_isnode(L, 1);
    halfword tail = null;
    halfword h;
    if (lua_gettop(L) > 1) {
        tail = *check_isnode(L, 2);
    }
    while (head != null) {
        unprotect_node_indeed(head);
        if (head == tail) {
            break;
        }
        head = vlink(head);
    }
    return 0;
}

/* node.direct.protect_glyphs */
/* node.direct.unprotect_glyphs */

static int lua_nodelib_direct_protect_glyph(lua_State * L)
{
    halfword n = (halfword) lua_tointeger(L,1);
    halfword h;
    protect_node_indeed(n);
    return 0;
}

static int lua_nodelib_direct_unprotect_glyph(lua_State * L)
{
    halfword n = (halfword) lua_tointeger(L,1);
    halfword h;
    unprotect_node_indeed(n);
    return 0;
}

static int lua_nodelib_direct_protect_glyphs(lua_State * L)
{
    halfword head = (halfword) lua_tointeger(L,1);
    halfword tail = (halfword) lua_tointeger(L,2);
    halfword h;
    while (head != null) {
        protect_node_indeed(head);
        if (head == tail) {
            break;
        }
        head = vlink(head);
    }
    return 0;
}

static int lua_nodelib_direct_unprotect_glyphs(lua_State * L)
{
    halfword head = (halfword) lua_tointeger(L,1);
    halfword tail = (halfword) lua_tointeger(L,2);
    halfword h;
    while (head != null) {
        unprotect_node_indeed(head);
        if (head == tail) {
            break;
        }
        head = vlink(head);
    }
    return 0;
}

/* node.first_glyph */

static int lua_nodelib_first_glyph(lua_State * L)
{
    /* on the stack are two nodes and a direction */
    halfword h, savetail = null, t = null;
    if (lua_gettop(L) < 1) {
        lua_pushnil(L);
        lua_pushboolean(L, 0);
        return 2;
    }
    h = *(check_isnode(L, 1));
    if (lua_gettop(L) > 1) {
        t = *(check_isnode(L, 2));
        savetail = vlink(t);
        vlink(t) = null;
    }
    while (h != null && (type(h) != glyph_node || !is_simple_character(h))) {
        h = vlink(h);
    }
    if (savetail != null) {
        vlink(t) = savetail;
    }
    lua_pushinteger(L, h);
    lua_nodelib_push(L);
    lua_pushboolean(L, (h == null ? 0 : 1));
    return 2;
}

/* node.direct.first_glyph */

static int lua_nodelib_direct_first_glyph(lua_State * L)
{
    halfword h,savetail,t;
    savetail = null;
    t = null;
    h = (halfword) lua_tointeger(L,1);
    if (h == null) {
        lua_pushnil(L);
        return 1;
    }
    t = (halfword) lua_tointeger(L,2);
    if (t != null) {
        savetail = vlink(t);
        vlink(t) = null;
    }
    while (h != null && (type(h) != glyph_node || !is_simple_character(h)))
        h = vlink(h);
    if (savetail != null)
        vlink(t) = savetail;
    lua_pushinteger(L, h);
    return 1; /* no need to also push a boolean if we have nil */
}

/* new, fast and dumb ones: only signals that something needs to be processed */

/* node.has_glyph */

static int lua_nodelib_has_glyph(lua_State * L)
{
    halfword *a;
    halfword h = (halfword) *(check_isnode(L,1)) ;
    while (h != null) {
        if ( (type(h) == glyph_node) || (type(h) == disc_node)) {
            fast_metatable(h);
            return 1;
        } else {
            h = vlink(h);
        }
    }
    lua_pushnil(L);
    return 1;
}

/* node.direct.has_glyph */

static int lua_nodelib_direct_has_glyph(lua_State * L)
{
    halfword h = (halfword) lua_tointeger(L,1) ;
    while (h != null) {
        if ((type(h) == glyph_node) || (type(h) == disc_node)) {
            nodelib_pushdirect(h);
            return 1;
        } else {
            h = vlink(h);
        }
    }
    lua_pushnil(L);
    return 1;
}

/* this is too simplistic, but it helps Hans to get going */

/*

static halfword do_ligature_n(halfword prev, halfword stop, halfword lig)
{
    vlink(lig) = vlink(stop);
    vlink(stop) = null;
    lig_ptr(lig) = vlink(prev);
    vlink(prev) = lig;
    return lig;
}

*/

/* node.do_ligature_n(node prev, node last, node lig) */

/*

static int lua_nodelib_do_ligature_n(lua_State * L)
{
    halfword p;
    halfword n = *check_isnode(L, 1);
    halfword m = *check_isnode(L, 2);
    halfword o = *check_isnode(L, 3);
    if (alink(n) == null || vlink(alink(n)) != n) {
        halfword tmp_head = new_node(temp_node, 0);
        couple_nodes(tmp_head, n);
        p = do_ligature_n(tmp_head, m, o);
        flush_node(tmp_head);
    } else {
        p = do_ligature_n(alink(n), m, o);
    }
    lua_pushinteger(L, p);
    lua_nodelib_push(L);
    return 1;
}

*/

/* node.direct.do_ligature_n(node prev, node last, node lig) */

/*

static int lua_nodelib_direct_do_ligature_n(lua_State * L)
{
    halfword p;
    halfword n = (halfword) lua_tointeger(L, 1);
    halfword m = (halfword) lua_tointeger(L, 2);
    halfword o = (halfword) lua_tointeger(L, 3);
    if ((n == null) || (m == null) || (o == null)) {
        lua_pushnil(L);
    } else {
        if (alink(n) == null || vlink(alink(n)) != n) {
            halfword tmp_head = new_node(temp_node, 0);
            couple_nodes(tmp_head, n);
            p = do_ligature_n(tmp_head, m, o);
            flush_node(tmp_head);
        } else {
            p = do_ligature_n(alink(n), m, o);
        }
        lua_pushinteger(L, p);
    }
    return 1;
}

*/

/* node.usedlist */

static int lua_nodelib_usedlist(lua_State * L)
{
    lua_pushinteger(L, list_node_mem_usage());
    lua_nodelib_push(L);
    return 1;
}

/* node.direct.usedlist */

static int lua_nodelib_direct_usedlist(lua_State * L)
{
    lua_pushinteger(L, list_node_mem_usage());
    return 1;
}

/* node.protrusion_skipable(node m) */

static int lua_nodelib_cp_skipable(lua_State * L)
{
    halfword n = *check_isnode(L, 1);
    lua_pushboolean(L, cp_skipable(n));
    return 1;
}

/* node.direct.protrusion_skipable(node m) */

static int lua_nodelib_direct_cp_skipable(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n == null) {
        lua_pushnil(L);
    } else {
        lua_pushboolean(L, cp_skipable(n));
    }
    return 1;
}


/* node.currentattr(node m) */

static int lua_nodelib_currentattr(lua_State * L)
{
    int u = lua_gettop(L);
    if (u == null) {
       /* query */
       halfword n ;
      /* current_attribute_list() return attr_list_cache */
      /* or null  (attr_list_cache can also be null)     */
       n = current_attribute_list();
       if (n) {
          lua_pushinteger(L, n);
          lua_nodelib_push(L);
       }
       else
          lua_pushnil(L);
       return 1;
    } else {
        /* assign */
        normal_warning("node lib","assignment via node.current_attr(<list>) is not supported (yet)");
        return 0;
    }
}


/* node.direct.currentattr(node m) */

static int lua_nodelib_direct_currentattr(lua_State * L)
{
    /* current_attribute_list() return attr_list_cache */
    /* or null  (attr_list_cache can also be null)     */
    halfword n = current_attribute_list();
    if (n)
        lua_pushinteger(L, n);
    else
        lua_pushnil(L);
    return 1;
}


/* node.direct.todirect */

static int lua_nodelib_direct_todirect(lua_State * L)
{
    if (lua_type(L,1) != LUA_TNUMBER) {
        /* assume node, no further testing, used in known situations */
        void *n ;
        n = lua_touserdata(L, 1);
        if (n == null) {
            lua_pushnil(L);
        } else {
            lua_pushinteger(L, *((halfword *)n) );
        }
    } /* else assume direct and returns argument */
    return 1;
}


/* node.direct.tonode */

static int lua_nodelib_direct_tonode(lua_State * L)
{
    halfword *a;
    halfword n = lua_tointeger(L, 1);
    if (n != null) {
        a = (halfword *) lua_newuserdata(L, sizeof(halfword));
        *a=n;
        lua_get_metatablelua(luatex_node);
        lua_setmetatable(L,-2);
    } /* else assume node and return argument */
    return 1;
}

/* node.setfield */

static int lua_nodelib_setfield_whatsit(lua_State * L, int n, const char *s)
{
    int t = subtype(n);

    if (t == pdf_literal_node) {
        /*tex The |string| key is obsolete. */
        if (lua_key_eq(s, mode)) {
            pdf_literal_mode(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, data) || lua_key_eq(s, string)) {
            set_pdf_literal_direct_normal(L, n, 3);
        } else if (lua_key_eq(s, token)) {
            set_pdf_literal_direct_token(L, n, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == late_lua_node) {
        /*tex The |string| key is obsolete. */
        if (lua_key_eq(s, data) || lua_key_eq(s, string)) {
            set_late_lua_direct_normal(L, n, 3);
        } else if (lua_key_eq(s, token)) {
            set_late_lua_direct_token(L, n, 3);
        } else if (lua_key_eq(s, name)) {
            cleanup_late_lua_name(n) ;
            late_lua_name(n) = nodelib_gettoks(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
        /* done */
    } else if (t == user_defined_node) {
        if (lua_key_eq(s, user_id)) {
            user_node_id(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, type)) {
            user_node_type(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, value)) {
            switch (user_node_type(n)) {
                case 'a':
                    user_node_value(n) = nodelib_getlist(L, 3);
                    break;
                case 'd':
                    user_node_value(n) = (halfword) lua_roundnumber(L, 3);
                    break;
                case 'l':
                    lua_pushvalue(L, 3);
                    if (user_node_value(n) != 0) {
                        luaL_unref(L, LUA_REGISTRYINDEX,user_node_value(n));
                    }
                    user_node_value(n) = luaL_ref(L, LUA_REGISTRYINDEX);
                    break;
                case 'n':
                    user_node_value(n) = nodelib_getlist(L, 3);
                    break;
                case 's':
                    user_node_value(n) = nodelib_getstring(L, 3);
                    break;
                case 't':
                    user_node_value(n) = nodelib_gettoks(L, 3);
                    break;
                default:
                    user_node_value(n) = (halfword) lua_roundnumber(L, 3);
                    break;
            }
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_annot_node) {
        if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, objnum)) {
            pdf_annot_objnum(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, data)) {
            pdf_annot_data(n) = nodelib_gettoks(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_dest_node) {
        if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, named_id)) {
            pdf_dest_named_id(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, dest_id)) {
            if (pdf_dest_named_id(n) == 1) {
                pdf_dest_id(n) = nodelib_gettoks(L, 3);
            } else {
                pdf_dest_id(n) = (halfword) lua_tointeger(L, 3);
            }
        } else if (lua_key_eq(s, dest_type)) {
            pdf_dest_type(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, xyz_zoom)) {
            pdf_dest_xyz_zoom(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, objnum)) {
            pdf_dest_objnum(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_setmatrix_node) {
        if (lua_key_eq(s, data)) {
            set_pdf_setmatrix_direct_value(L,n,3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_refobj_node) {
        if (lua_key_eq(s, objnum)) {
            pdf_obj_objnum(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_start_link_node) {
        if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, objnum)) {
            pdf_link_objnum(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, link_attr)) {
            pdf_link_attr(n) = nodelib_gettoks(L, 3);
        } else if (lua_key_eq(s, action)) {
            pdf_link_action(n) = nodelib_getaction(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == write_node) {
        if (lua_key_eq(s, stream)) {
            write_stream(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, data)) {
            set_write_direct_value(L,n,3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_colorstack_node) {
        if (lua_key_eq(s, stack)) {
            pdf_colorstack_stack(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, command)) {
            pdf_colorstack_cmd(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, data)) {
            pdf_colorstack_data(n) = nodelib_gettoks(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_action_node) {
        if (lua_key_eq(s, action_type)) {
            pdf_action_type(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, named_id)) {
            pdf_action_named_id(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, action_id)) {
            if (pdf_action_named_id(n) == 1) {
                pdf_action_id(n) = nodelib_gettoks(L, 3);
            } else {
                pdf_action_id(n) = (halfword) lua_tointeger(L, 3);
            }
        } else if (lua_key_eq(s, file)) {
            pdf_action_file(n) = nodelib_gettoks(L, 3);
        } else if (lua_key_eq(s, new_window)) {
            pdf_action_new_window(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, data)) {
            pdf_action_tokens(n) = nodelib_gettoks(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if ((t == pdf_thread_node) || (t == pdf_start_thread_node)) {
        if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, named_id)) {
            pdf_thread_named_id(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, thread_id)) {
            if (pdf_thread_named_id(n) == 1) {
                pdf_thread_id(n) = nodelib_gettoks(L, 3);
            } else {
                pdf_thread_id(n) = (halfword) lua_tointeger(L, 3);
            }
        } else if (lua_key_eq(s, thread_attr)) {
            pdf_thread_attr(n) = nodelib_gettoks(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == special_node) {
        if (lua_key_eq(s, data)) {
            set_special_direct_value(L,n,3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == open_node) {
        if (lua_key_eq(s, stream)) {
            write_stream(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, name)) {
            open_name(n) = nodelib_getstring(L, 3);
        } else if (lua_key_eq(s, area)) {
            open_area(n) = nodelib_getstring(L, 3);
        } else if (lua_key_eq(s, ext)) {
            open_ext(n) = nodelib_getstring(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == close_node) {
        if (lua_key_eq(s, stream)) {
            write_stream(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if ((t == pdf_end_link_node) || (t == pdf_end_thread_node) || (t == save_pos_node) ||
               (t == pdf_save_node)     || (t == pdf_restore_node)) {
        return nodelib_cantset(L, n, s);
    } else {
        /* do nothing */
    }
    return 0;
}

static int lua_nodelib_fast_setfield(lua_State * L)
{
    const char *s;
    halfword n = *((halfword *) lua_touserdata(L, 1));
    int t = type(n);

    if (lua_type(L, 2) == LUA_TNUMBER) {
        if (lua_gettop(L) == 3) {
            int i = lua_tointeger(L, 2);
            int val = lua_tointeger(L, 3);
            if (val == UNUSED_ATTRIBUTE) {
                (void) unset_attribute(n, i, val);
            } else {
                set_attribute(n, i, val);
            }
        } else {
            luaL_error(L, "incorrect number of arguments");
        }
        return 0;
    }

    s = lua_tostring(L, 2);

    /*if (lua_key_eq(s, id)) {
      type(n) = (quarteword) lua_tointeger(L, 3);
      }* else */
    if (lua_key_eq(s, next)) {
        halfword x = nodelib_getlist(L, 3);
        if (x>0 && type(x) == glue_spec_node) {
            return luaL_error(L, "You can't assign a %s node to a next field\n", node_data[type(x)].name);
        }
        vlink(n) = x;
    } else if (lua_key_eq(s, prev)) {
        halfword x = nodelib_getlist(L, 3);
        if (x>0 && type(x) == glue_spec_node) {
            return luaL_error(L, "You can't assign a %s node to a prev field\n", node_data[type(x)].name);
        }
        alink(n) = x;
    } else if (lua_key_eq(s, attr)) {
        if (nodetype_has_attributes(type(n))) {
            nodelib_setattr(L, 3, n);
        }
    } else if (t == glyph_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, font)) {
            font(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, char)) {
            character(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, xoffset)) {
            x_displace(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, yoffset)) {
            y_displace(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, data)) {
            glyph_node_data(n) = (halfword) lua_tointeger(L, 3);;
        } else if (lua_key_eq(s, width)) {
            /* not yet */
        } else if (lua_key_eq(s, height)) {
            /* not yet */
        } else if (lua_key_eq(s, depth)) {
            /* not yet */
        } else if (lua_key_eq(s, expansion_factor)) {
            ex_glyph(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, components)) {
            lig_ptr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, lang)) {
            set_char_lang(n, (halfword) lua_tointeger(L, 3));
        } else if (lua_key_eq(s, left)) {
            set_char_lhmin(n, (halfword) lua_tointeger(L, 3));
        } else if (lua_key_eq(s, right)) {
            set_char_rhmin(n, (halfword) lua_tointeger(L, 3));
        } else if (lua_key_eq(s, uchyph)) {
            set_char_uchyph(n, (halfword) lua_tointeger(L, 3));
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if ((t == hlist_node) || (t == vlist_node)) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, list) || lua_key_eq(s, head)) {
            list_ptr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, direction)) {
            box_dir(n) = nodelib_getdirection(L, 3);
        } else if (lua_key_eq(s, dir)) {
            box_dir(n) = nodelib_getdir_par(L, 3);
        } else if (lua_key_eq(s, shift)) {
            shift_amount(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, glue_order)) {
            glue_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, glue_sign)) {
            glue_sign(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, glue_set)) {
            glue_set(n) = (glue_ratio) lua_tonumber(L, 3); /* integer or float */
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == disc_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, pre)) {
            set_disc_field(pre_break(n), nodelib_getlist(L, 3));
        } else if (lua_key_eq(s, post)) {
            set_disc_field(post_break(n), nodelib_getlist(L, 3));
        } else if (lua_key_eq(s, replace)) {
            set_disc_field(no_break(n), nodelib_getlist(L, 3));
        } else if (lua_key_eq(s, penalty)) {
            disc_penalty(n) = (quarterword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == glue_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch)) {
            stretch(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, shrink)) {
            shrink(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch_order)) {
            stretch_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, shrink_order)) {
            shrink_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, leader)) {
            leader_ptr(n) = nodelib_getlist(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == kern_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, kern)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, expansion_factor)) {
            ex_kern(n) = (halfword) lua_roundnumber(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == penalty_node) {
        if (lua_key_eq(s, subtype)) {
            /* dummy subtype */
        } else if (lua_key_eq(s, penalty)) {
            penalty(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == rule_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, left)) {
            rule_left(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, right)) {
            rule_right(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, direction)) {
            rule_dir(n) = nodelib_getdirection(L, 3);
        } else if (lua_key_eq(s, dir)) {
            rule_dir(n) = nodelib_getdir_par(L, 3);
        } else if (lua_key_eq(s, index)) {
            rule_index(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, transform)) {
            rule_transform(n) = (halfword) lua_roundnumber(L, 3); /* can also be user value */
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == dir_node) {
        if (lua_key_eq(s, direction)) {
            dir_dir(n) = nodelib_getdirection(L, 3);
        } else if (lua_key_eq(s, dir)) {
            nodelib_setdir_text(L, 3, n);
        } else if (lua_key_eq(s, level)) {
            dir_level(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == local_par_node) {
        if (lua_key_eq(s, pen_inter)) {
            local_pen_inter(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, pen_broken)) {
            local_pen_broken(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, direction)) {
            local_par_dir(n) = nodelib_getdirection(L, 3);
        } else if (lua_key_eq(s, dir)) {
            local_par_dir(n) = nodelib_getdir_par(L, 3);
        } else if (lua_key_eq(s, box_left)) {
            local_box_left(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, box_left_width)) {
            local_box_left_width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, box_right)) {
            local_box_right(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, box_right_width)) {
            local_box_right_width(n) = (halfword) lua_roundnumber(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == whatsit_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else {
            lua_nodelib_setfield_whatsit(L, n, s);
        }
    } else if (t == simple_noad) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, nucleus)) {
            nucleus(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, sub)) {
            subscr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, sup)) {
            supscr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, options)) {
            noadoptions(n) = (quarterword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if ((t == math_char_node) || (t == math_text_char_node)) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, fam)) {
            math_fam(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, char)) {
            math_character(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == mark_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, class)) {
            mark_class(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, mark)) {
            mark_ptr(n) = nodelib_gettoks(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == ins_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, cost)) {
            float_cost(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))) {
            ins_ptr(n) = nodelib_getlist(L, 3);
		/* glue */
        } else if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch)) {
            stretch(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, shrink)) {
            shrink(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch_order)) {
            stretch_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, shrink_order)) {
            shrink_order(n) = (quarterword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == math_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, surround)) {
            surround(n) = (halfword) lua_roundnumber(L, 3);
		/* glue */
        } else if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch)) {
            stretch(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, shrink)) {
            shrink(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch_order)) {
            stretch_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, shrink_order)) {
            shrink_order(n) = (quarterword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == fraction_noad) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, width)) {
            thickness(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, num)) {
            numerator(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, denom)) {
            denominator(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, left)) {
            left_delimiter(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, right)) {
            right_delimiter(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, middle)) {
            middle_delimiter(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, fam)) {
            fraction_fam(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, options)) {
            fractionoptions(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == style_node) {
        if (lua_key_eq(s, subtype)) {
            /* dummy subtype */
        } else if (lua_key_eq(s, style)) {
            assign_math_style(L,3,subtype(n));
        } else {
            /* return nodelib_cantset(L, n, s); */
        }
    } else if (t == accent_noad) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, nucleus)) {
            nucleus(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, sub)) {
            subscr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, sup)) {
            supscr(n) = nodelib_getlist(L, 3);
        } else if ((lua_key_eq(s, top_accent))||(lua_key_eq(s, accent))) {
            top_accent_chr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, bot_accent)) {
            bot_accent_chr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, overlay_accent)) {
            overlay_accent_chr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, fraction)) {
            accentfraction(n) = (halfword) lua_roundnumber(L,3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == fence_noad) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, delim)) {
            delimiter(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, italic)) {
            delimiteritalic(n) = (halfword) lua_roundnumber(L,3);
        } else if (lua_key_eq(s, height)) {
            delimiterheight(n) = (halfword) lua_roundnumber(L,3);
        } else if (lua_key_eq(s, depth)) {
            delimiterdepth(n) = (halfword) lua_roundnumber(L,3);
        } else if (lua_key_eq(s, options)) {
            delimiteroptions(n) = (halfword) lua_tointeger(L,3);
        } else if (lua_key_eq(s, class)) {
            delimiterclass(n) = (halfword) lua_tointeger(L,3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == delim_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, small_fam)) {
            small_fam(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, small_char)) {
            small_char(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, large_fam)) {
            large_fam(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, large_char)) {
            large_char(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if ((t == sub_box_node) || (t == sub_mlist_node)) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))){
            math_list(n) = nodelib_getlist(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == radical_noad) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, nucleus)) {
            nucleus(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, sub)) {
            subscr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, sup)) {
            supscr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, left)) {
            left_delimiter(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, degree)) {
            degree(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, width)) {
            radicalwidth(n) = lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, options)) {
            radicaloptions(n) = (halfword) lua_tointeger(L,3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == margin_kern_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, width) || lua_key_eq(s, kern)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, glyph)) {
            margin_char(n) = nodelib_getlist(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == split_up_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, last_ins_ptr)) {
            last_ins_ptr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, best_ins_ptr)) {
            best_ins_ptr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, broken_ptr)) {
            broken_ptr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, broken_ins)) {
            broken_ins(n) = nodelib_getlist(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == choice_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, display)) {
            display_mlist(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, text)) {
            text_mlist(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, script)) {
            script_mlist(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, scriptscript)) {
            script_script_mlist(n) = nodelib_getlist(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == inserting_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, last_ins_ptr)) {
            last_ins_ptr(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, best_ins_ptr)) {
            best_ins_ptr(n) = nodelib_getlist(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == attribute_node) {
        if (lua_key_eq(s, subtype)) {
            /* dummy subtype */
        } else if (lua_key_eq(s, number)) {
            attribute_id(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, value)) {
            attribute_value(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == adjust_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))) {
            adjust_ptr(n) = nodelib_getlist(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == unset_node) {
        if (lua_key_eq(s, subtype)) {
            /* dummy subtype */
        } else if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, direction)) {
            box_dir(n) = nodelib_getdirection(L, 3);
        } else if (lua_key_eq(s, dir)) {
            box_dir(n) = nodelib_getdir_par(L, 3);
        } else if (lua_key_eq(s, shrink)) {
            glue_shrink(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, glue_order)) {
            glue_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, glue_sign)) {
            glue_sign(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, stretch)) {
            glue_stretch(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, count)) {
            span_count(n) = (quarterword) lua_tointeger(L, 3);
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))){
            list_ptr(n) = nodelib_getlist(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == attribute_list_node) {
        if (lua_key_eq(s, subtype)) {
            /* dummy subtype */
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == boundary_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, value)) {
            boundary_value(n) = lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == glue_spec_node) {
        if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch)) {
            stretch(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, shrink)) {
            shrink(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch_order)) {
            stretch_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, shrink_order)) {
            shrink_order(n) = (quarterword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else {
        return luaL_error(L, "You can't assign to this %s node (%d)\n", node_data[t].name, n);
    }
    return 0;
}

static int lua_nodelib_setfield(lua_State * L)
{
    /* [given-node] [...]*/
    halfword *p = lua_touserdata(L, 1);
    if ( (p == NULL) || (! lua_getmetatable(L,1)) ) {
        return 0;
    }
    /* [given-node] [mt-given-node]*/
    lua_get_metatablelua(luatex_node);
    /* [given-node] [mt-given-node] [mt-node]*/
    if ( (!lua_rawequal(L, -1, -2)) ) {
        return 0;
    }
    /* prune stack and call getfield */
    lua_settop(L,3);
    return lua_nodelib_fast_setfield(L);
}

/* node.direct.setfield */

static int lua_nodelib_direct_setfield_whatsit(lua_State * L, int n, const char *s)
{
    int t = subtype(n);
    if (t == pdf_literal_node) {
        /*tex The |string| key is obsolete. */
        if (lua_key_eq(s, mode)) {
            pdf_literal_mode(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, data) || lua_key_eq(s, string)) {
            set_pdf_literal_direct_normal(L, n, 3);
        } else if (lua_key_eq(s, token)) {
            set_pdf_literal_direct_token(L, n, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == late_lua_node) {
        /*tex The |string| key is obsolete. */
        if (lua_key_eq(s, data) || lua_key_eq(s, string)) {
            set_late_lua_direct_normal(L, n, 3);
        } else if (lua_key_eq(s, token)) {
            set_late_lua_direct_token(L, n, 3);
        } else if (lua_key_eq(s, name)) {
            cleanup_late_lua_name(n) ;
            late_lua_name(n) = nodelib_gettoks(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == user_defined_node) {
        if (lua_key_eq(s, user_id)) {
            user_node_id(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, type)) {
            user_node_type(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, value)) {
            set_user_node_direct_value(L, n, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_annot_node) {
        if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, objnum)) {
            pdf_annot_objnum(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, data)) {
            pdf_annot_data(n) = nodelib_gettoks(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_dest_node) {
        if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, named_id)) {
            pdf_dest_named_id(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, dest_id)) {
            if (pdf_dest_named_id(n) == 1) {
                pdf_dest_id(n) = nodelib_gettoks(L, 3);
            } else {
                pdf_dest_id(n) = (halfword) lua_tointeger(L, 3);
            }
        } else if (lua_key_eq(s, dest_type)) {
            pdf_dest_type(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, xyz_zoom)) {
            pdf_dest_xyz_zoom(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, objnum)) {
            pdf_dest_objnum(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_setmatrix_node) {
        if (lua_key_eq(s, data)) {
            set_pdf_setmatrix_direct_value(L, n, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_refobj_node) {
        if (lua_key_eq(s, objnum)) {
            pdf_obj_objnum(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_start_link_node) {
        if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, objnum)) {
            pdf_link_objnum(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, link_attr)) {
            pdf_link_attr(n) = nodelib_gettoks(L, 3);
        } else if (lua_key_eq(s, action)) {
            pdf_link_action(n) = nodelib_popdirect(n);  /*nodelib_getaction(L, 3);*/
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == write_node) {
        if (lua_key_eq(s, stream)) {
            set_write_direct_value(L,n,3);
        } else if (lua_key_eq(s, data)) {
            set_write_direct_value(L,n,3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_colorstack_node) {
        if (lua_key_eq(s, stack)) {
            pdf_colorstack_stack(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, command)) {
            pdf_colorstack_cmd(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, data)) {
            pdf_colorstack_data(n) = nodelib_gettoks(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == pdf_action_node) {
        if (lua_key_eq(s, action_type)) {
            pdf_action_type(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, named_id)) {
            pdf_action_named_id(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, action_id)) {
            if (pdf_action_named_id(n) == 1) {
                pdf_action_id(n) = nodelib_gettoks(L, 3);
            } else {
                pdf_action_id(n) = (halfword) lua_tointeger(L, 3);
            }
        } else if (lua_key_eq(s, file)) {
            pdf_action_file(n) = nodelib_gettoks(L, 3);
        } else if (lua_key_eq(s, new_window)) {
            pdf_action_new_window(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, data)) {
            pdf_action_tokens(n) = nodelib_gettoks(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if ((t == pdf_thread_node) || (t == pdf_start_thread_node)) {
        if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, named_id)) {
            pdf_thread_named_id(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, thread_id)) {
            if (pdf_thread_named_id(n) == 1) {
                pdf_thread_id(n) = nodelib_gettoks(L, 3);
            } else {
                pdf_thread_id(n) = (halfword) lua_tointeger(L, 3);
            }
        } else if (lua_key_eq(s, thread_attr)) {
            pdf_thread_attr(n) = nodelib_gettoks(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == special_node) {
        if (lua_key_eq(s, data)) {
            set_special_direct_value(L, n, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == open_node) {
        if (lua_key_eq(s, stream)) {
            write_stream(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, name)) {
            open_name(n) = nodelib_getstring(L, 3);
        } else if (lua_key_eq(s, area)) {
            open_area(n) = nodelib_getstring(L, 3);
        } else if (lua_key_eq(s, ext)) {
            open_ext(n) = nodelib_getstring(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == close_node) {
        if (lua_key_eq(s, stream)) {
            write_stream(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if ((t == pdf_end_link_node) || (t == pdf_end_thread_node) || (t == save_pos_node) ||
               (t == pdf_save_node)     || (t == pdf_restore_node)) {
        return nodelib_cantset(L, n, s);
    } else {
        /* do nothing */
    }
    return 0;
}

/* direct.is_char  */
/* direct.is_glyph */

static int lua_nodelib_direct_is_char(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (type(n) != glyph_node) {
        lua_pushnil(L); /* no glyph at all */
        lua_pushinteger(L,type(n)); /* can save a lookup call */
        return 2;
    } else if (subtype(n) >= 256) {
        lua_pushboolean(L,0); /* a done glyph */
    } else if (lua_type(L,2) == LUA_TNUMBER) {
        halfword f = lua_tointeger(L, 2);
        if (f && f == font(n)) {
            lua_pushinteger(L,character(n)); /* a todo glyph in the asked font */
        } else {
            lua_pushboolean(L,0); /* a todo glyph in another font */
        }
    } else {
        lua_pushinteger(L,character(n)); /* a todo glyph */
    }
    return 1;
}

static int lua_nodelib_direct_is_glyph(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (type(n) != glyph_node) {
        lua_pushboolean(L,0);
        lua_pushinteger(L,type(n));
    } else {
        lua_pushinteger(L,character(n));
        lua_pushinteger(L,font(n));
    }
    return 2;
}

/* uses_font */
/* direct.uses_font */

#define uses_font_disc(what,p,n) \
p = vlink(what(n)); \
while (p) { \
    if ((type(p) == glyph_node) && (font(p) == f)) { \
        lua_pushboolean(L,1); \
        return 1; \
    } \
    p = vlink(p); \
}


static int lua_nodelib_direct_uses_font(lua_State * L)
{
    halfword n = lua_tointeger(L,1);
    halfword f = lua_tointeger(L,2);
    halfword p;
    if (type(n) == glyph_node) {
        lua_pushboolean(L,font(n) == f);
    } else if (type(n) == disc_node) {
        uses_font_disc(pre_break,p,n);
        uses_font_disc(post_break,p,n);
        uses_font_disc(no_break,p,n);
    }
    /* todo: other node types */
    lua_pushboolean(L,0);
    return 1;
}

static int lua_nodelib_uses_font(lua_State * L)
{
    halfword n = *check_isnode(L, 1);
    halfword f = lua_tointeger(L,2);
    halfword p;
    if (type(n) == glyph_node) {
        lua_pushboolean(L,font(n) == f);
    } else if (type(n) == disc_node) {
        uses_font_disc(pre_break,p,n);
        uses_font_disc(post_break,p,n);
        uses_font_disc(no_break,p,n);
    }
    /* todo: other node types */
    lua_pushboolean(L,0);
    return 1;
}


/* direct.setfield */

static int lua_nodelib_direct_setfield(lua_State * L)
{
    const char *s;

    halfword n = lua_tointeger(L, 1);
    int t = type(n);

    if (lua_type(L, 2) == LUA_TNUMBER) {
        if (lua_gettop(L) == 3) {
            int i = lua_tointeger(L, 2);
            int val = lua_tointeger(L, 3);
            if (val == UNUSED_ATTRIBUTE) {
                (void) unset_attribute(n, i, val);
            } else {
                set_attribute(n, i, val);
            }
        } else {
            luaL_error(L, "incorrect number of arguments");
        }
        return 0;
    }

    s = lua_tostring(L, 2);

    /*if (lua_key_eq(s, id)) {
        type(n) = (quarteword) lua_tointeger(L, 3);
    } else*/
    if (lua_key_eq(s, next)) {
        halfword x = nodelib_popdirect(3);
        if (x>0 && type(x) == glue_spec_node) {
            return luaL_error(L, "You can't assign a %s node to a next field\n", node_data[type(x)].name);
        }
        vlink(n) = x;
    } else if (lua_key_eq(s, prev)) {
        halfword x = nodelib_popdirect(3);
        if (x>0 && type(x) == glue_spec_node) {
            return luaL_error(L, "You can't assign a %s node to a prev field\n", node_data[type(x)].name);
        }
        alink(n) = x;
    } else if (lua_key_eq(s, attr)) {
        if (nodetype_has_attributes(type(n))) {
         /* nodelib_setattr(L, 3, n); */
            reassign_attribute(n,lua_tointeger(L, 3));
        }
    } else if (t == glyph_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, font)) {
            font(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, char)) {
            character(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, xoffset)) {
            x_displace(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, yoffset)) {
            y_displace(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, data)) {
            glyph_node_data(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, expansion_factor)) {
            ex_glyph(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, components)) {
            lig_ptr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, lang)) {
            set_char_lang(n, (halfword) lua_tointeger(L, 3));
        } else if (lua_key_eq(s, left)) {
            set_char_lhmin(n, (halfword) lua_tointeger(L, 3));
        } else if (lua_key_eq(s, right)) {
            set_char_rhmin(n, (halfword) lua_tointeger(L, 3));
        } else if (lua_key_eq(s, uchyph)) {
            set_char_uchyph(n, (halfword) lua_tointeger(L, 3));
        } else if (lua_key_eq(s, width)) {
            /* not yet */
        } else if (lua_key_eq(s, height)) {
            /* not yet */
        } else if (lua_key_eq(s, depth)) {
            /* not yet */
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if ((t == hlist_node) || (t == vlist_node)) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, list) || lua_key_eq(s, head)) {
            list_ptr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, direction)) {
            box_dir(n) = nodelib_getdirection(L, 3);
        } else if (lua_key_eq(s, dir)) {
            box_dir(n) = nodelib_getdir_par(L, 3);
        } else if (lua_key_eq(s, shift)) {
            shift_amount(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, glue_order)) {
            glue_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, glue_sign)) {
            glue_sign(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, glue_set)) {
            glue_set(n) = (glue_ratio) lua_tonumber(L, 3);  /* integer or float */
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == disc_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, pre)) {
            set_disc_field(pre_break(n), nodelib_popdirect(3));
        } else if (lua_key_eq(s, post)) {
            set_disc_field(post_break(n), nodelib_popdirect(3));
        } else if (lua_key_eq(s, replace)) {
            set_disc_field(no_break(n), nodelib_popdirect(3));
        } else if (lua_key_eq(s, penalty)) {
            disc_penalty(n) = (quarterword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == glue_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch)) {
            stretch(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, shrink)) {
            shrink(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch_order)) {
            stretch_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, shrink_order)) {
            shrink_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, leader)) {
            leader_ptr(n) = nodelib_popdirect(3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == kern_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, kern)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, expansion_factor)) {
            ex_kern(n) = (halfword) lua_roundnumber(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == penalty_node) {
        if (lua_key_eq(s, subtype)) {
            /* dummy subtype */
        } else if (lua_key_eq(s, penalty)) {
            penalty(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == rule_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, left)) {
            rule_left(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, right)) {
            rule_right(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, direction)) {
            rule_dir(n) = nodelib_getdirection(L, 3);
        } else if (lua_key_eq(s, dir)) {
            rule_dir(n) = nodelib_getdir_par(L, 3);
        } else if (lua_key_eq(s, index)) {
            rule_index(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, transform)) {
            rule_transform(n) = (halfword) lua_roundnumber(L, 3); /* can also be user value */
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == dir_node) {
        if (lua_key_eq(s, direction)) {
            dir_dir(n) = nodelib_getdirection(L, 3);
        } else if (lua_key_eq(s, dir)) {
            nodelib_setdir_text(L, 3, n);
        } else if (lua_key_eq(s, level)) {
            dir_level(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == whatsit_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else {
            lua_nodelib_direct_setfield_whatsit(L, n, s);
        }
    } else if (t == local_par_node) {
        if (lua_key_eq(s, pen_inter)) {
            local_pen_inter(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, pen_broken)) {
            local_pen_broken(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, direction)) {
            local_par_dir(n) = nodelib_getdirection(L, 3);
        } else if (lua_key_eq(s, dir)) {
            local_par_dir(n) = nodelib_getdir_par(L, 3);
        } else if (lua_key_eq(s, box_left)) {
            local_box_left(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, box_left_width)) {
            local_box_left_width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, box_right)) {
            local_box_right(n) = nodelib_getlist(L, 3);
        } else if (lua_key_eq(s, box_right_width)) {
            local_box_right_width(n) = (halfword) lua_roundnumber(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == simple_noad) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, nucleus)) {
            nucleus(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, sub)) {
            subscr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, sup)) {
            supscr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, options)) {
            noadoptions(n) = (quarterword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if ((t == math_char_node) || (t == math_text_char_node)) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, fam)) {
            math_fam(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, char)) {
            math_character(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == mark_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, class)) {
            mark_class(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, mark)) {
            mark_ptr(n) = nodelib_gettoks(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == ins_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, cost)) {
            float_cost(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))) {
            ins_ptr(n) = nodelib_popdirect(3);
        /* glue */
        } else if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch)) {
            stretch(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, shrink)) {
            shrink(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch_order)) {
            stretch_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, shrink_order)) {
            shrink_order(n) = (quarterword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == math_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, surround)) {
            surround(n) = (halfword) lua_roundnumber(L, 3);
        /* glue */
        } else if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch)) {
            stretch(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, shrink)) {
            shrink(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch_order)) {
            stretch_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, shrink_order)) {
            shrink_order(n) = (quarterword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == fraction_noad) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, width)) {
            thickness(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, num)) {
            numerator(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, denom)) {
            denominator(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, left)) {
            left_delimiter(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, right)) {
            right_delimiter(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, middle)) {
            middle_delimiter(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, fam)) {
            fraction_fam(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, options)) {
            fractionoptions(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == style_node) {
        if (lua_key_eq(s, subtype)) {
            /* dummy subtype */
        } else if (lua_key_eq(s, style)) {
            assign_math_style(L,2,subtype(n));
        } else {
            /* return nodelib_cantset(L, n, s); */
        }
    } else if (t == accent_noad) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, nucleus)) {
            nucleus(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, sub)) {
            subscr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, sup)) {
            supscr(n) = nodelib_popdirect(3);
        } else if ((lua_key_eq(s, top_accent))||(lua_key_eq(s, accent))) {
            top_accent_chr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, bot_accent)) {
            bot_accent_chr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, overlay_accent)) {
            overlay_accent_chr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, fraction)) {
            accentfraction(n) = (halfword) lua_roundnumber(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == fence_noad) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, delim)) {
            delimiter(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, italic)) {
            delimiteritalic(n) = (halfword) lua_roundnumber(L,3);
        } else if (lua_key_eq(s, height)) {
            delimiterheight(n) = (halfword) lua_roundnumber(L,3);
        } else if (lua_key_eq(s, depth)) {
            delimiterdepth(n) = (halfword) lua_roundnumber(L,3);
        } else if (lua_key_eq(s, options)) {
            delimiteroptions(n) = (halfword) lua_tointeger(L,3);
        } else if (lua_key_eq(s, class)) {
            delimiterclass(n) = (halfword) lua_tointeger(L,3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == delim_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, small_fam)) {
            small_fam(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, small_char)) {
            small_char(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, large_fam)) {
            large_fam(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, large_char)) {
            large_char(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if ((t == sub_box_node) || (t == sub_mlist_node)) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))){
            math_list(n) = nodelib_popdirect(3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == radical_noad) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, nucleus)) {
            nucleus(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, sub)) {
            subscr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, sup)) {
            supscr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, left)) {
            left_delimiter(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, degree)) {
            degree(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, width)) {
            radicalwidth(n) = lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, options)) {
            radicaloptions(n) = (halfword) lua_tointeger(L,3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == margin_kern_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, width) || lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, glyph)) {
            margin_char(n) = nodelib_popdirect(3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == split_up_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, last_ins_ptr)) {
            last_ins_ptr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, best_ins_ptr)) {
            best_ins_ptr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, broken_ptr)) {
            broken_ptr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, broken_ins)) {
            broken_ins(n) = nodelib_popdirect(3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == choice_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, display)) {
            display_mlist(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, text)) {
            text_mlist(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, script)) {
            script_mlist(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, scriptscript)) {
            script_script_mlist(n) = nodelib_popdirect(3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == inserting_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, last_ins_ptr)) {
            last_ins_ptr(n) = nodelib_popdirect(3);
        } else if (lua_key_eq(s, best_ins_ptr)) {
            best_ins_ptr(n) = nodelib_popdirect(3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == attribute_node) {
        if (lua_key_eq(s, subtype)) {
            /* dummy subtype */
        } else if (lua_key_eq(s, number)) {
            attribute_id(n) = (halfword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, value)) {
            attribute_value(n) = (halfword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == adjust_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))) {
            adjust_ptr(n) = nodelib_popdirect(3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == unset_node) {
        if (lua_key_eq(s, subtype)) {
            /* dummy subtype */
        } else if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, height)) {
            height(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, depth)) {
            depth(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, direction)) {
            box_dir(n) = nodelib_getdirection(L, 3);
        } else if (lua_key_eq(s, dir)) {
            box_dir(n) = nodelib_getdir_par(L, 3);
        } else if (lua_key_eq(s, shrink)) {
            glue_shrink(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, glue_order)) {
            glue_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, glue_sign)) {
            glue_sign(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, stretch)) {
            glue_stretch(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, count)) {
            span_count(n) = (quarterword) lua_tointeger(L, 3);
        } else if ((lua_key_eq(s, list)) || (lua_key_eq(s, head))){
            list_ptr(n) = nodelib_popdirect(3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == attribute_list_node) {
        if (lua_key_eq(s, subtype)) {
            /* dummy subtype */
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == boundary_node) {
        if (lua_key_eq(s, subtype)) {
            subtype(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, value)) {
            boundary_value(n) = lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else if (t == glue_spec_node) {
        if (lua_key_eq(s, width)) {
            width(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch)) {
            stretch(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, shrink)) {
            shrink(n) = (halfword) lua_roundnumber(L, 3);
        } else if (lua_key_eq(s, stretch_order)) {
            stretch_order(n) = (quarterword) lua_tointeger(L, 3);
        } else if (lua_key_eq(s, shrink_order)) {
            shrink_order(n) = (quarterword) lua_tointeger(L, 3);
        } else {
            return nodelib_cantset(L, n, s);
        }
    } else {
        return luaL_error(L, "You can't assign to this %s node (%d)\n", node_data[t].name, n);
    }
    return 0;
}

/* boxes */

static int direct_get_box_id(lua_State * L, int i)
{
    const char *s;
    int cur_cs1, cur_cmd1;
    size_t k = 0;
    int j = -1;
    switch (lua_type(L, i)) {
        case LUA_TSTRING:
            s = lua_tolstring(L, i, &k);
            cur_cs1 = string_lookup(s, k);
            cur_cmd1 = eq_type(cur_cs1);
            if (cur_cmd1 == char_given_cmd ||
                cur_cmd1 == math_given_cmd) {
                j = equiv(cur_cs1);
            }
            break;
        case LUA_TNUMBER:
            j = lua_tointeger(L, (i));
            break;
        default:
            luaL_error(L, "argument must be a string or a number");
            j = -1;                 /* not a valid box id */
    }
    return j;
}

/* node.getbox = tex.getbox */
/* node.setbox = tex.setbox */

/* node.direct.getbox */
/* node.direct.setbox */

static int lua_nodelib_direct_getbox(lua_State * L)
{
    int t;
    int k = direct_get_box_id(L, -1);
    direct_check_index_range(k, "getbox");
    t = get_tex_box_register(k);
    if (t == null) {
        lua_pushnil(L);
    } else {
        lua_pushinteger(L, t);
    }
    return 1;
}

static int lua_nodelib_direct_setbox(lua_State * L)
{
    int isglobal = 0;
    int j, k, err, t;
    int save_global_defs ;
    int n = lua_gettop(L);
    if (n == 3 && (lua_type(L, 1) == LUA_TSTRING)) {
        const char *s = lua_tostring(L, 1);
        if (lua_key_eq(s, global))
            isglobal = 1;
    }
    t = lua_type(L, -1);
    k = direct_get_box_id(L, -2);
    direct_check_index_range(k, "setbox");
    if (t == LUA_TBOOLEAN) {
        j = lua_toboolean(L, -1);
        if (j == 0) {
            j = null;
        } else {
            return 0;
        }
    } else if (t == LUA_TNIL) {
        j = null;
    } else {
        j = nodelib_popdirect(-1);
        if (j != null && type(j) != hlist_node && type(j) != vlist_node) {
            luaL_error(L, "setbox: incompatible node type (%s)\n",get_node_name(type(j), subtype(j)));
            return 0;
        }
    }
    save_global_defs = global_defs_par;
    if (isglobal) {
        global_defs_par = 1;
    }
    err = set_tex_box_register(k, j);
    global_defs_par = save_global_defs;
    if (err) {
        luaL_error(L, "incorrect value");
    }
    return 0;
}

/* node.is_node(n) */

/*

static int lua_nodelib_is_node(lua_State * L)
{
    if (maybe_isnode(L,1) == NULL)
        lua_pushboolean(L,0);
    else
        lua_pushboolean(L,1);
    return 1;
}

*/

static int lua_nodelib_is_node(lua_State * L)
{
    halfword *p = maybe_isnode(L, 1);
    if (p == NULL)
        lua_pushboolean(L,0);
    else
        lua_pushinteger(L, *((halfword *)p));
    return 1;
}

/* node.direct.is_direct(n) (handy for mixed usage testing) */

static int lua_nodelib_direct_is_direct(lua_State * L)
{   /*
        quick and dirty and faster than not node.is_node, this helper
        returns false or <direct>
    */
    /*
        if (lua_type(L,1) == LUA_TNUMBER)
            lua_pushboolean(L,1);
        else
            lua_pushboolean(L,0);
    */
    if (lua_type(L,1) != LUA_TNUMBER)
        lua_pushboolean(L,0);
    /* else return direct */
    return 1;
}

/* node.direct.is_node(n) (handy for mixed usage testing) */

static int lua_nodelib_direct_is_node(lua_State * L)
{   /*
        quick and dirty, only checks userdata, node.is_node is slower but
        more exact, this helper returns false or <node>
    */
    /*
        halfword *n = lua_touserdata(L, 1);
        if (n != NULL) {
            lua_pushboolean(L,1);
        } else {
            lua_pushboolean(L,0);
        }
    */
    if (maybe_isnode(L,1) == NULL)
        lua_pushboolean(L,0);
    /* else assume and return node */
    return 1;
}

    /*
    Beware, enabling and disabling can result in an inconsistent properties table
    but it might make sense sometimes. Of course by default we have disabled this
    mechanism. And, one can always sweep the table empty.
*/

static int lua_nodelib_properties_set_mode(lua_State * L)
{   /* <boolean> */
    if (lua_isboolean(L,1)) {
        lua_properties_enabled = lua_toboolean(L,1);
    }
    if (lua_isboolean(L,2)) {
        lua_properties_use_metatable = lua_toboolean(L,2);
    }
    return 0;
}

/* We used to have variants in assigned defaults but they made no sense. */

static int lua_nodelib_properties_flush_table(lua_State * L)
{   /* <node|direct> <number> */
    lua_get_metatablelua(node_properties);
    lua_pushnil(L); /* initializes lua_next */
    while (lua_next(L,-2) != 0) {
        lua_pushvalue(L,-2);
        lua_pushnil(L);
        lua_settable(L,-5);
        lua_pop(L,1);
    }
    return 1;
}

/* maybe we should allocate a proper index 0..var_mem_max but not now */

static int lua_nodelib_get_property(lua_State * L)
{   /* <node> */
    halfword n = *((halfword *) lua_touserdata(L, 1));
    if (n == null) {
        lua_pushnil(L);
    } else {
        lua_get_metatablelua(node_properties);
        lua_rawgeti(L,-1,n);
    }
    return 1;
}

static int lua_nodelib_direct_get_property(lua_State * L)
{   /* <direct> */
    halfword n = lua_tointeger(L, 1);
    if (n == null) {
        lua_pushnil(L);
    } else {
        lua_get_metatablelua(node_properties);
        lua_rawgeti(L,-1,n);
    }
    return 1;
}

static int lua_nodelib_set_property(lua_State * L)
{
    /* <node> <value> */
    halfword n = *((halfword *) lua_touserdata(L, 1));
    if (n != null) {
        lua_settop(L,2);
        lua_get_metatablelua(node_properties);
        /* <node> <value> <propertytable> */
        lua_replace(L,-3);
        /* <propertytable> <value> */
        lua_rawseti(L,-2,n);
    }
    return 0;
}

static int lua_nodelib_direct_set_property(lua_State * L)
{
    /* <direct> <value> */
    halfword n = lua_tointeger(L, 1);
    if (n != null) {
        lua_settop(L,2);
        lua_get_metatablelua(node_properties);
        /* <node> <value> <propertytable> */
        lua_replace(L,1);
        /* <propertytable> <value> */
        lua_rawseti(L,1,n);
    }
    return 0;
}

static int lua_nodelib_direct_properties_get_table(lua_State * L)
{   /* <node|direct> */
    lua_get_metatablelua(node_properties);
    return 1;
}

static int lua_nodelib_properties_get_table(lua_State * L)
{   /* <node|direct> */
    lua_get_metatablelua(node_properties_indirect);
    return 1;
}

/* bonus */

static int lua_nodelib_get_property_t(lua_State * L)
{   /* <table> <node> */
    halfword n = *((halfword *) lua_touserdata(L, 2));
    if (n == null) {
        lua_pushnil(L);
    } else {
        lua_rawgeti(L,1,n);
    }
    return 1;
}

static int lua_nodelib_set_property_t(lua_State * L)
{
    /* <table> <node> <value> */
    halfword n = *((halfword *) lua_touserdata(L, 2));
    if (n != null) {
        lua_settop(L,3);
        lua_rawseti(L,1,n);
    }
    return 0;
}

/* extra helpers */

static int lua_nodelib_effective_glue(lua_State * L)
{
    halfword *glue;
    halfword *parent;
    glue = lua_touserdata(L, 1);
    if ((glue == NULL) || (type(*glue) != glue_node)) {
        lua_pushnil(L) ;
    } else {
        parent = lua_touserdata(L, 2);
        if ((parent != NULL) && ((type(*parent) == hlist_node) || (type(*parent) == vlist_node))) {
            double w = width(*glue) ;
            if ((int) glue_sign(*parent) == 1) {
                if (stretch_order(*glue) == glue_order(*parent)) {
                    w += stretch(*glue) * (double) glue_set(*parent);
                }
            } else if (glue_sign(*parent) == 2) {
                if (shrink_order(*glue) == glue_order(*parent)) {
                    w -= shrink(*glue) * (double) glue_set(*parent);
                }
            }
            if (lua_toboolean(L,3)) {
                lua_pushinteger(L,round(w));
            } else {
                lua_pushnumber(L,w);
            }
        } else {
            lua_pushinteger(L,width(*glue));
        }
    }
    return 1;
}

static int lua_nodelib_direct_effective_glue(lua_State * L)
{
    halfword glue = lua_tointeger(L, 1);
    if ((glue == null) || (type(glue) != glue_node)) {
        lua_pushnil(L) ;
    } else {
        halfword parent = lua_tointeger(L, 2);
        if ((parent != null) && ((type(parent) == hlist_node) || (type(parent) == vlist_node))) {
            double w = (double) width(glue) ;
            if ((int)glue_sign(parent) == 1) {
                if (stretch_order(glue) == glue_order(parent)) {
                    w += stretch(glue) * (double) glue_set(parent);
                }
            } else if (glue_sign(parent) == 2) {
                if (shrink_order(glue) == glue_order(parent)) {
                    w -= shrink(glue) * (double) glue_set(parent);
                }
            }
            if (lua_toboolean(L,3)) {
                lua_pushinteger(L,round(w));
            } else {
                lua_pushnumber(L,w);
            }
        } else {
            lua_pushinteger(L,width(glue));
        }
    }
    return 1;
}

/*
    Disc nodes are kind of special in the sense that their head is not the head as we
    see it, but a special node that has status info of which head and tail are part.
    Normally when proper set/get functions are used this status node is all right but
    if a macro package permits arbitrary messing around, then it can at some point
    call the following cleaner, just before linebreaking kicks in. This one is not
    called automatically because if significantly slows down the line break routing.

*/

#define check_disc(c) \
    p = c ; \
    if (p != null && vlink(p) != null) \
        tlink(p) = tail_of_list(vlink(p));

static int lua_nodelib_direct_check_discretionaries(lua_State * L) {
    halfword c = lua_tointeger(L, 1);
    halfword p ;
    while (c != null) {
        if (type(c) == disc_node) {
            check_disc(no_break(c)) ;
            check_disc(pre_break(c)) ;
            check_disc(post_break(c)) ;
        }
        c = vlink(c) ;
    }
    return 0;
}

static int lua_nodelib_direct_check_discretionary(lua_State * L) {
    halfword c = lua_tointeger(L, 1);
    if (c != null && type(c) == disc_node) {
        halfword p ;
        check_disc(no_break(c)) ;
        check_disc(pre_break(c)) ;
        check_disc(post_break(c)) ;
    }
    return 0;
}

static int lua_nodelib_direct_flatten_discretionaries(lua_State * L)
{
    halfword head = lua_tointeger(L, 1);
    halfword current = head;
    halfword next;
    halfword d, n, h, t;
    int c = 0;
    while (current != null) {
        next = vlink(current);
        if (type(current) == disc_node) {
            ++c;
            d = current;
            n = no_break(d);
            h = vlink(n);
            t = tlink(n);
            if (h != null) {
                try_couple_nodes(t,next);
                if (current == head) {
                    head = h;
                } else {
                    try_couple_nodes(alink(current),h);
                }
                vlink(n) = null ;
              /*tlink(n) = null; */
            } else {
                if (current == head) {
                    head = next;
                } else {
                    try_couple_nodes(alink(current),next);
                }
            }
            flush_node(d);
        }
        current = next;
    }
    nodelib_pushdirect_or_nil(head);
    lua_pushinteger(L,c);
    return 2;
}

static int lua_nodelib_check_discretionaries(lua_State * L) {
    halfword c = *check_isnode(L, 1);
    halfword p ;
    while (c != null) {
        if (type(c) == disc_node) {
            check_disc(no_break(c)) ;
            check_disc(pre_break(c)) ;
            check_disc(post_break(c)) ;
        }
        c = vlink(c) ;
    }
    return 0;
}

static int lua_nodelib_check_discretionary(lua_State * L) {
    halfword c = *check_isnode(L, 1);
    if (c != null && type(c) == disc_node) {
        halfword p ;
        check_disc(no_break(c)) ;
        check_disc(pre_break(c)) ;
        check_disc(post_break(c)) ;
    }
    return 0;
}

static int lua_nodelib_flatten_discretionaries(lua_State * L)
{
    halfword head = *check_isnode(L, 1);
    halfword current = head;
    halfword next;
    halfword d, n, h, t;
    halfword *a;
    int c = 0;
    while (current != null) {
        next = vlink(current);
        if (type(current) == disc_node) {
            ++c;
            d = current;
            n = no_break(d);
            h = vlink(n);
            t = tlink(n);
            if (h != null) {
                try_couple_nodes(t,next);
                if (current == head) {
                    head = h;
                } else {
                    try_couple_nodes(alink(current),h);
                }
                vlink(n) = null ;
              /*tlink(n) = null; */
            } else {
                if (current == head) {
                    head = next;
                } else {
                    try_couple_nodes(alink(current),next);
                }
            }
            flush_node(d);
        }
        current = next;
    }
    fast_metatable_or_nil(head);
    lua_pushinteger(L,c);
    return 2;
}

/* synctex but not */

static int lua_nodelib_direct_set_synctex_fields(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    halfword tag = lua_tointeger(L, 2);
    halfword line = lua_tointeger(L, 3);
    if (n != null) {
        switch (type(n)) {
            case glyph_node:
                if (tag)  synctex_tag_glyph(n)  = tag;
                if (line) synctex_line_glyph(n) = line;
                break;
            case glue_node:
                if (tag)  synctex_tag_glue(n)  = tag;
                if (line) synctex_line_glue(n) = line;
                break;
            case kern_node:
                if (tag)  synctex_tag_kern(n)  = tag;
                if (line) synctex_line_kern(n) = line;
                break;
            case hlist_node:
            case vlist_node:
            case unset_node:
                if (tag)  synctex_tag_box(n)  = tag;
                if (line) synctex_line_box(n) = line;
                break;
            case rule_node:
                if (tag)  synctex_tag_rule(n)  = tag;
                if (line) synctex_line_rule(n) = line;
                break;
            case math_node:
                if (tag)  synctex_tag_math(n)  = tag;
                if (line) synctex_line_math(n) = line;
                break;
        }
    }
    return 0;
}

static int lua_nodelib_direct_get_synctex_fields(lua_State * L)
{
    halfword n = lua_tointeger(L, 1);
    if (n != null) {
        switch (type(n)) {
            case glyph_node:
                lua_pushinteger(L,synctex_tag_glyph(n));
                lua_pushinteger(L,synctex_line_glyph(n));
                break;
            case glue_node:
                lua_pushinteger(L,synctex_tag_glue(n));
                lua_pushinteger(L,synctex_line_glue(n));
                break;
            case kern_node:
                lua_pushinteger(L,synctex_tag_kern(n));
                lua_pushinteger(L,synctex_line_kern(n));
                break;
            case hlist_node:
            case vlist_node:
            case unset_node:
                lua_pushinteger(L,synctex_tag_box(n));
                lua_pushinteger(L,synctex_line_box(n));
                break;
            case rule_node:
                lua_pushinteger(L,synctex_tag_rule(n));
                lua_pushinteger(L,synctex_line_rule(n));
                break;
            case math_node:
                lua_pushinteger(L,synctex_tag_math(n));
                lua_pushinteger(L,synctex_line_math(n));
                break;
            default:
                return 0;
                break;
        }
        return 2;
    }
    return 0;
}

/* helper, assumes one node, returns node and delta .. to be tested */

static int lua_nodelib_prepend_prevdepth(lua_State * L)
{
    halfword *a;
    halfword p;
    halfword prevdepth;
    boolean mirrored;
    halfword n = *check_isnode(L, 1);
    if (!(type(n) == hlist_node || type(n) == vlist_node)) {
        lua_pushnil(L);
        return 1;
    }
    prevdepth = lua_tointeger(L,2);
    mirrored = (type(n) == hlist_node) && is_mirrored(box_dir(n)) ;
    if (prevdepth > ignore_depth) {
        halfword d;
        if (mirrored) {
            d = width(baseline_skip_par) - prevdepth - depth(n);
        } else {
            d = width(baseline_skip_par) - prevdepth - height(n);
        }
        if (d < line_skip_limit_par) {
            p = new_param_glue(line_skip_code);
        } else {
            p = new_skip_param(baseline_skip_code);
            width(p) = d;
        }
        couple_nodes(p,n);
        fast_metatable_or_nil(p); /* glue */
    } else {
        fast_metatable_or_nil(n); /* node */
    }
    if (mirrored) {
        prevdepth = height(n);
    } else {
        prevdepth = depth(n);
    }
    lua_pushinteger(L,prevdepth); /* new prevdepth */
    return 2;
}

static int lua_nodelib_direct_prepend_prevdepth(lua_State * L)
{
    halfword p;
    halfword prevdepth;
    boolean mirrored;
    halfword n = lua_tointeger(L, 1);
    if (type(n) == hlist_node || type(n) == vlist_node) {
        lua_pushnil(L);
        return 1;
    }
    prevdepth = lua_tointeger(L,2);
    mirrored = (type(n) == hlist_node) && is_mirrored(box_dir(n)) ;
    if (prevdepth > ignore_depth) {
        halfword d;
        if (mirrored) {
            d = width(baseline_skip_par) - prevdepth - depth(n);
        } else {
            d = width(baseline_skip_par) - prevdepth - height(n);
        }
        if (d < line_skip_limit_par) {
            p = new_param_glue(line_skip_code);
        } else {
            p = new_skip_param(baseline_skip_code);
            width(p) = d;
        }
        couple_nodes(p,n);
        lua_pushinteger(L,p); /* glue */
    } else {
        lua_pushinteger(L,n); /* node */
    }
    if (mirrored) {
        prevdepth = height(n);
    } else {
        prevdepth = depth(n);
    }
    lua_pushinteger(L,prevdepth); /* new prevdepth */
    return 2;
}

static int lua_nodelib_make_extensible(lua_State * L)
{
    int top = lua_gettop(L);
    if (top >= 3) {
        halfword fnt = lua_tointeger(L,1);
        halfword chr = lua_tointeger(L,2);
        halfword size = lua_tointeger(L,3);
        halfword overlap = 65536 ;
        halfword attlist = null;
        halfword b = null;
        int horizontal = 0;
        if (top >= 4) {
            overlap = lua_tointeger(L,4);
        }
        if (top >= 5) {
            horizontal = lua_toboolean(L,5);
        }
        if (top >= 6) {
            attlist = *check_isnode(L, 6);
        }
        b = make_extensible(fnt,chr,size,overlap,horizontal,attlist);
        nodelib_pushlist(L,b);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* done */

static const struct luaL_Reg nodelib_p[] = {
    {"__index",    lua_nodelib_get_property_t},
    {"__newindex", lua_nodelib_set_property_t},
    {NULL, NULL} /* sentinel */
};

static void lua_new_properties_table(lua_State * L)
{
    lua_pushstring(L,"node.properties");
    lua_newtable(L);
    lua_settable(L,LUA_REGISTRYINDEX);

    lua_pushstring(L,"node.properties.indirect");
    lua_newtable(L);
    luaL_newmetatable(L,"node.properties.indirect.meta");
    luaL_openlib(L, NULL, nodelib_p, 0);
    lua_setmetatable(L,-2);
    lua_settable(L,LUA_REGISTRYINDEX);
}

/* node.direct.* */

static const struct luaL_Reg direct_nodelib_f[] = {
    {"copy", lua_nodelib_direct_copy},
    {"copy_list", lua_nodelib_direct_copy_list},
    {"count", lua_nodelib_direct_count},
    {"current_attr", lua_nodelib_direct_currentattr},
    {"dimensions", lua_nodelib_direct_dimensions},
    {"rangedimensions", lua_nodelib_direct_rangedimensions},
 /* {"do_ligature_n", lua_nodelib_direct_do_ligature_n}, */
    {"end_of_math", lua_nodelib_direct_end_of_math},
 /* {"family_font", lua_nodelib_mfont}, */ /* no node argument */
 /* {"fields", lua_nodelib_fields}, */ /* no node argument */
    {"first_glyph", lua_nodelib_direct_first_glyph},
    {"flush_list", lua_nodelib_direct_flush_list},
    {"flush_node", lua_nodelib_direct_flush_node},
    {"free", lua_nodelib_direct_free},
    {"getbox", lua_nodelib_direct_getbox},
    {"getchar", lua_nodelib_direct_getchar},
    {"getcomponents", lua_nodelib_direct_getcomponents},
    {"getlang", lua_nodelib_direct_getlang},
    {"getkern", lua_nodelib_direct_getkern},
    {"getpenalty", lua_nodelib_direct_getpenalty},
    {"getdir", lua_nodelib_direct_getdir},
    {"getoffsets", lua_nodelib_direct_getoffsets},
    {"getdisc", lua_nodelib_direct_getdisc},
    {"getwhd", lua_nodelib_direct_getwhd},
    {"getwidth", lua_nodelib_direct_getwidth},
    {"getheight", lua_nodelib_direct_getheight},
    {"getdepth", lua_nodelib_direct_getdepth},
    {"getshift", lua_nodelib_direct_getshift},
    {"getfield", lua_nodelib_direct_getfield},
    {"getfont", lua_nodelib_direct_getfont},
    {"getexpansion", lua_nodelib_direct_getexpansion},
    {"getfam", lua_nodelib_direct_getfam},
    {"getid", lua_nodelib_direct_getid},
    {"getnext", lua_nodelib_direct_getnext},
    {"getprev", lua_nodelib_direct_getprev},
    {"getboth", lua_nodelib_direct_getboth},
    {"getlist", lua_nodelib_direct_getlist},
    {"getleader", lua_nodelib_direct_getleader},
    {"getdata", lua_nodelib_direct_getdata},
    {"getsubtype", lua_nodelib_direct_getsubtype},
    {"getattributelist", lua_nodelib_direct_getattributelist},
    {"getnucleus", lua_nodelib_direct_getnucleus},
    {"getsub", lua_nodelib_direct_getsub},
    {"getsup", lua_nodelib_direct_getsup},
    {"getdirection", lua_nodelib_direct_getdirection},
    {"has_glyph", lua_nodelib_direct_has_glyph},
    {"has_attribute", lua_nodelib_direct_has_attribute},
    {"get_attribute", lua_nodelib_direct_get_attribute},
    {"find_attribute", lua_nodelib_direct_find_attribute},
    {"has_field", lua_nodelib_direct_has_field},
    {"is_char", lua_nodelib_direct_is_char},
    {"is_glyph", lua_nodelib_direct_is_glyph},
    {"uses_font", lua_nodelib_direct_uses_font},
    {"hpack", lua_nodelib_direct_hpack},
    {"hyphenating", lang_tex_direct_hyphenating},
 /* {"id", lua_nodelib_id}, */ /* no node argument */
    {"insert_after", lua_nodelib_direct_insert_after},
    {"insert_before", lua_nodelib_direct_insert_before},
    {"is_direct", lua_nodelib_direct_is_direct},
    {"is_node", lua_nodelib_direct_is_node},
    {"kerning", font_tex_direct_kerning},
    {"last_node", lua_nodelib_direct_last_node},
    {"length", lua_nodelib_direct_length},
    {"ligaturing", font_tex_direct_ligaturing},
 /* {"mlist_to_hlist", lua_nodelib_mlist_to_hset_properties_modelist}, */
    {"new", lua_nodelib_direct_new},
    {"tostring", lua_nodelib_direct_tostring},
    {"protect_glyphs", lua_nodelib_direct_protect_glyphs},
    {"protect_glyph", lua_nodelib_direct_protect_glyph},
    {"protrusion_skippable", lua_nodelib_direct_cp_skipable},
    {"remove", lua_nodelib_direct_remove},
    {"set_attribute", lua_nodelib_direct_set_attribute},
    {"setbox", lua_nodelib_direct_setbox},
    {"setfield", lua_nodelib_direct_setfield},
    {"setchar", lua_nodelib_direct_setchar},
    {"setfont", lua_nodelib_direct_setfont},
    {"setexpansion", lua_nodelib_direct_setexpansion},
    {"setfam", lua_nodelib_direct_setfam},
    {"setcomponents", lua_nodelib_direct_setcomponents},
    {"setlang", lua_nodelib_direct_setlang},
    {"setkern", lua_nodelib_direct_setkern},
    {"setpenalty", lua_nodelib_direct_setpenalty},
    {"setdir", lua_nodelib_direct_setdir},
    {"setdirection", lua_nodelib_direct_setdirection},
    {"setoffsets", lua_nodelib_direct_setoffsets},
    {"setdisc", lua_nodelib_direct_setdisc},
    {"setwhd", lua_nodelib_direct_setwhd},
    {"setwidth", lua_nodelib_direct_setwidth},
    {"setheight", lua_nodelib_direct_setheight},
    {"setdepth", lua_nodelib_direct_setdepth},
    {"setshift", lua_nodelib_direct_setshift},
    {"setnext", lua_nodelib_direct_setnext},
    {"setprev", lua_nodelib_direct_setprev},
    {"setboth", lua_nodelib_direct_setboth},
    {"setlink", lua_nodelib_direct_setlink},
    {"setsplit", lua_nodelib_direct_setsplit},
    {"setlist", lua_nodelib_direct_setlist},
    {"setleader", lua_nodelib_direct_setleader},
    {"setdata", lua_nodelib_direct_setdata},
    {"setsubtype", lua_nodelib_direct_setsubtype},
    {"setattributelist", lua_nodelib_direct_setattributelist},
    {"setnucleus", lua_nodelib_direct_setnucleus},
    {"setsub", lua_nodelib_direct_setsub},
    {"setsup", lua_nodelib_direct_setsup},
    {"slide", lua_nodelib_direct_slide},
 /* {"subtype", lua_nodelib_subtype}, */ /* no node argument */
    {"tail", lua_nodelib_direct_tail},
    {"todirect",  lua_nodelib_direct_todirect},
    {"tonode", lua_nodelib_direct_tonode},
    {"traverse", lua_nodelib_direct_traverse},
    {"traverse_id", lua_nodelib_direct_traverse_filtered},
    {"traverse_char", lua_nodelib_direct_traverse_char},
    {"traverse_glyph", lua_nodelib_direct_traverse_glyph},
    {"traverse_list", lua_nodelib_direct_traverse_list},
 /* {"type", lua_nodelib_type}, */ /* no node argument */
 /* {"types", lua_nodelib_types}, */ /* no node argument */
    {"unprotect_glyphs", lua_nodelib_direct_unprotect_glyphs},
    {"unprotect_glyph", lua_nodelib_direct_unprotect_glyph},
    {"unset_attribute", lua_nodelib_direct_unset_attribute},
    {"setglue",lua_nodelib_direct_set_glue},
    {"getglue",lua_nodelib_direct_get_glue},
    {"is_zero_glue",lua_nodelib_direct_is_zero_glue},
    {"usedlist", lua_nodelib_direct_usedlist},
    {"vpack", lua_nodelib_direct_vpack},
 /* {"whatsits", lua_nodelib_whatsits}, */ /* no node argument */
    {"write", lua_nodelib_direct_append},
    {"set_properties_mode",lua_nodelib_properties_set_mode},
    {"flush_properties_table",lua_nodelib_properties_flush_table},
    {"get_properties_table",lua_nodelib_direct_properties_get_table},
    {"getproperty", lua_nodelib_direct_get_property},
    {"setproperty", lua_nodelib_direct_set_property},
    {"effective_glue", lua_nodelib_direct_effective_glue},
    {"check_discretionary", lua_nodelib_direct_check_discretionary},
    {"check_discretionaries", lua_nodelib_direct_check_discretionaries},
    {"flatten_discretionaries",lua_nodelib_direct_flatten_discretionaries},
    {"prepend_prevdepth",lua_nodelib_direct_prepend_prevdepth},
    /* done */
    {"set_synctex_fields", lua_nodelib_direct_set_synctex_fields},
    {"get_synctex_fields", lua_nodelib_direct_get_synctex_fields},
    /* done */
    {NULL, NULL} /* sentinel */
};

/* node.* */

static const struct luaL_Reg nodelib_f[] = {
    {"copy", lua_nodelib_copy},
    {"copy_list", lua_nodelib_copy_list},
    {"count", lua_nodelib_count},
    {"current_attr", lua_nodelib_currentattr},
    {"dimensions", lua_nodelib_dimensions},
    {"rangedimensions", lua_nodelib_rangedimensions},
 /* {"do_ligature_n", lua_nodelib_do_ligature_n}, */
    {"end_of_math", lua_nodelib_end_of_math},
    {"family_font", lua_nodelib_mfont},
    {"fields", lua_nodelib_fields},
    {"subtypes", lua_nodelib_subtypes},
    {"values", lua_nodelib_values},
    {"first_glyph", lua_nodelib_first_glyph},
    {"flush_list", lua_nodelib_flush_list},
    {"flush_node", lua_nodelib_flush_node},
    {"free", lua_nodelib_free},
 /* {"getbox", lua_nodelib_getbox}, */ /* tex.getbox */
    {"getnext", lua_nodelib_getnext},
    {"getprev", lua_nodelib_getprev},
    {"getboth", lua_nodelib_getboth},
    {"getdisc", lua_nodelib_getdisc},
    {"getwhd", lua_nodelib_getwhd},
    {"getlist", lua_nodelib_getlist},
    {"getleader", lua_nodelib_getleader},
    {"getid", lua_nodelib_getid},
    {"getsubtype", lua_nodelib_getsubtype},
    {"getfont", lua_nodelib_getfont},
    {"getchar", lua_nodelib_getchar},
    {"getfield", lua_nodelib_getfield},
    {"setfield", lua_nodelib_setfield},
    {"has_glyph", lua_nodelib_has_glyph},
    {"has_attribute", lua_nodelib_has_attribute},
    {"get_attribute", lua_nodelib_get_attribute},
    {"find_attribute", lua_nodelib_find_attribute},
    {"has_field", lua_nodelib_has_field},
    {"is_char", lua_nodelib_is_char},
    {"is_glyph", lua_nodelib_is_glyph},
    {"uses_font", lua_nodelib_uses_font},
    {"hpack", lua_nodelib_hpack},
    {"hyphenating", lang_tex_hyphenating},
    {"id", lua_nodelib_id},
    {"insert_after", lua_nodelib_insert_after},
    {"insert_before", lua_nodelib_insert_before},
    {"is_node", lua_nodelib_is_node},
    {"kerning", font_tex_kerning},
    {"last_node", lua_nodelib_last_node},
    {"length", lua_nodelib_length},
    {"ligaturing", font_tex_ligaturing},
    {"mlist_to_hlist", lua_nodelib_mlist_to_hlist},
    {"new", lua_nodelib_new},
    {"next", lua_nodelib_next}, /* getnext is somewhat more efficient, and get* fits better in direct compatibility */
    {"prev", lua_nodelib_prev}, /* getprev is somewhat more efficient, and get* fits better in direct compatibility */
    {"tostring", lua_nodelib_tostring},
    {"protect_glyphs", lua_nodelib_protect_glyphs},
    {"protect_glyph", lua_nodelib_protect_glyph},
    {"protrusion_skippable", lua_nodelib_cp_skipable},
    {"remove", lua_nodelib_remove},
 /* {"setbox", lua_nodelib_setbox}, */ /* tex.setbox */
    {"set_attribute", lua_nodelib_set_attribute},
    {"slide", lua_nodelib_slide},
    {"subtype", lua_nodelib_subtype},
    {"tail", lua_nodelib_tail},
    {"traverse", lua_nodelib_traverse},
    {"traverse_id", lua_nodelib_traverse_filtered},
    {"traverse_char", lua_nodelib_traverse_char},
    {"traverse_glyph", lua_nodelib_traverse_glyph},
    {"traverse_list", lua_nodelib_traverse_list},
    {"type", lua_nodelib_type},
    {"types", lua_nodelib_types},
    {"unprotect_glyphs", lua_nodelib_unprotect_glyphs},
    {"unprotect_glyph", lua_nodelib_unprotect_glyph},
    {"unset_attribute", lua_nodelib_unset_attribute},
    {"setglue", lua_nodelib_set_glue},
    {"getglue", lua_nodelib_get_glue},
    {"is_zero_glue", lua_nodelib_is_zero_glue},
    {"usedlist", lua_nodelib_usedlist},
    {"vpack", lua_nodelib_vpack},
    {"whatsits", lua_nodelib_whatsits},
    {"write", lua_nodelib_append},
    /* experiment */
 /* {"attributes_to_table",lua_nodelib_attributes_to_table}, */
    /* experiment */
    {"set_properties_mode", lua_nodelib_properties_set_mode},
    {"flush_properties_table", lua_nodelib_properties_flush_table},
    {"get_properties_table", lua_nodelib_properties_get_table},
    {"getproperty", lua_nodelib_get_property},
    {"setproperty", lua_nodelib_set_property},
    {"effective_glue", lua_nodelib_effective_glue},
    {"check_discretionary", lua_nodelib_check_discretionary},
    {"check_discretionaries", lua_nodelib_check_discretionaries},
    {"flatten_discretionaries", lua_nodelib_flatten_discretionaries},
    {"prepend_prevdepth", lua_nodelib_prepend_prevdepth},
    {"make_extensible", lua_nodelib_make_extensible},
    /* done */
    {"fix_node_lists", lua_nodelib_fix_node_lists},
    {NULL, NULL} /* sentinel */
};

static const struct luaL_Reg nodelib_m[] = {
    {"__index", lua_nodelib_fast_getfield},
    {"__newindex", lua_nodelib_fast_setfield},
    {"__tostring", lua_nodelib_tostring},
    {"__eq", lua_nodelib_equal},
    {NULL, NULL} /* sentinel */
};

int luaopen_node(lua_State * L)
{

    lua_new_properties_table(L);

    /* the main metatable of node userdata */
    luaL_newmetatable(L, NODE_METATABLE);
    /* node.* */
    luaL_openlib(L, NULL, nodelib_m, 0);
    luaL_openlib(L, "node", nodelib_f, 0);
    /* node.direct */
    lua_pushstring(L,"direct");
    lua_newtable(L);
    luaL_openlib(L, NULL, direct_nodelib_f, 0);
    lua_rawset(L,-3);
    return 1;
}

void nodelist_to_lua(lua_State * L, int n)
{
    lua_pushinteger(L, n);
    lua_nodelib_push(L);
}

int nodelist_from_lua(lua_State * L, int n)
{
    if (lua_isnil(L, n)) {
        return null;
    } else {
        halfword list = *check_isnode(L, n);
        return (list ? list : null);
    }
}
