/* lnewtokenlib.c

   Copyright 2006-2012 Taco Hoekwater <taco@luatex.org>

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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


/*
    This module is unfinished and an intermediate step to removal of the old
    token lib. Between version 0.80 and 0.85 the transition will be complete
    and at the same time the input (buffer) handling will be cleaned up and
    simplified. At that moment we can feed back tokens into the input.

    The code can be optimized a bit by faster key checking. The scan functions
    implemented here will stay functionally the same but can be improved if
    needed. The old triplet model will disappear.

*/

#include "ptexlib.h"
#include "lua/luatex-api.h"

/*
typedef struct lua_token {
    int token;
    int origin;
} lua_token;
*/

typedef struct saved_tex_scanner {
    int token;
    int origin;
    int save_cmd, save_chr, save_cs, save_tok;
} saved_tex_scanner;

#define save_tex_scanner(a) do {		  \
	a.save_cmd = cur_cmd;			  \
	a.save_chr = cur_chr;			  \
	a.save_cs  = cur_cs;			  \
	a.save_tok = cur_tok;			  \
    } while (0)

#define unsave_tex_scanner(a) do {		  \
	cur_cmd = a.save_cmd;			  \
	cur_chr = a.save_chr;			  \
	cur_cs = a.save_cs;			  \
	cur_tok = a.save_tok;			  \
    } while (0)

#define TEX_ORIGIN 0 /* not used yet */
#define LUA_ORIGIN 1

static lua_token *check_istoken(lua_State * L, int ud);

#define TOKEN_METATABLE  "luatex.token"

#define DEBUG 0
#define DEBUG_OUT stdout

#define DEFAULT_SCAN_CODE_SET 2048 + 4096 /* default: letter and other */

/* two core helpers */

#define  is_active_string(s) (strlen((char *)s)>3 && *s==0xEF && *(s+1)==0xBF && *(s+2)==0xBF)

void make_token_table(lua_State * L, int cmd, int chr, int cs)
{
    lua_createtable(L, 3, 0);
    lua_pushinteger(L, cmd);
    lua_rawseti(L, -2, 1);
    lua_pushinteger(L, chr);
    lua_rawseti(L, -2, 2);
    lua_pushinteger(L, cs);
    lua_rawseti(L, -2, 3);
}

static unsigned char *get_cs_text(int cs)
{
    if (cs == null_cs)
        return (unsigned char *) xstrdup("\\csname\\endcsname");
    else if ((cs_text(cs) < 0) || (cs_text(cs) >= str_ptr))
        return (unsigned char *) xstrdup("");
    else
        return (unsigned char *) makecstring(cs_text(cs));
}

/* maybe this qualify as  a macro, not function */

static lua_token *maybe_istoken(lua_State * L, int ud)
{
    lua_token *p = lua_touserdata(L, ud);
    if (p != NULL) {
        if (lua_getmetatable(L, ud)) {
            lua_get_metatablelua(luatex_token);
            if (!lua_rawequal(L, -1, -2))
                p = NULL;
            lua_pop(L, 2);
        }
    }
    return p;
}

/* we could make the message a function and just inline the rest (via a macro) */

lua_token *check_istoken(lua_State * L, int ud)
{
    lua_token *p = maybe_istoken(L, ud);
    if (p != NULL)
        return p;
    formatted_error("token lib","lua <token> expected, not an object with type %s", luaL_typename(L, ud));
    return NULL;
}

/* token library functions */

static void make_new_token(lua_State * L, int cmd, int chr, int cs)
{
    int tok = 0;
    lua_token *thetok = lua_newuserdata(L, sizeof(lua_token));
    thetok->origin = LUA_ORIGIN;
    fast_get_avail(thetok->token);
    tok = (cs ? cs_token_flag + cs : token_val(cmd, chr));
    set_token_info(thetok->token, tok);
    lua_get_metatablelua(luatex_token);
    lua_setmetatable(L, -2);
}

static void push_token(lua_State * L, int tok)
{
    lua_token *thetok = lua_newuserdata(L, sizeof(lua_token));
    thetok->origin = LUA_ORIGIN;
    thetok->token = tok;
    lua_get_metatablelua(luatex_token);
    lua_setmetatable(L, -2);
}

static int run_get_biggest_char(lua_State * L)
{
    lua_pushinteger(L, biggest_char);
    return 1;
}

/* not that useful:

static int run_get_cs_offset(lua_State * L)
{
    lua_pushinteger(L, cs_token_flag);
    return 1;
}

*/

static int run_get_command_id(lua_State * L)
{
    int id = -1;
    if (lua_type(L, -1) == LUA_TSTRING) {
        id = get_command_id(lua_tostring(L, -1));
    }
    if (id >= 0) {
        lua_pushinteger(L, id);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* not that useful:

static int run_get_csname_id(lua_State * L)
{
    const char *s;
    size_t k, cs = 0;
    if (lua_type(L, -1) == LUA_TSTRING) {
        s = lua_tolstring(L, -1, &k);
        cs = (size_t) string_lookup(s, k);
    }
    lua_pushinteger(L, (lua_Number) cs);
    return 1;
}

*/

static int run_get_next(lua_State * L)
{
    saved_tex_scanner texstate;
    save_tex_scanner(texstate);
    get_next();
    make_new_token(L, cur_cmd, cur_chr, cur_cs);
    unsave_tex_scanner(texstate);
    return 1;
}

/*
    This is experimental code:

        local t1 = token.get_next()
        local t2 = token.get_next()
        local t3 = token.get_next()
        local t4 = token.get_next()
        -- watch out, we flush in sequence
        token.put_next { t1, t2 }
        -- but this one gets pushed in front
        token.put_next ( t3, t4 )
        -- so when we get wxyz we put yzwx!

    At some point we can consider a token.print that delays and goes via
    the same rope mechanism as texio.prints and friends but then one can
    as well serialize the tokens and do a normal print so there is no real
    gain in it. After all, the tokenlib operates at the input level so we
    might as well keep it there.

*/

inline static int run_put_next(lua_State * L)
{
    int n = lua_gettop(L);
    int m = 0;
    int i = 0;
    halfword h = null;
    halfword t = null;
    halfword x = null;
    lua_token *p ;
    if (n == 0) {
        /* we accept a single nil argument */
        return 0;
    }
    lua_get_metatablelua(luatex_token);
    m = lua_gettop(L);
    if (lua_type(L,1) == LUA_TTABLE) {
        if (n>1) {
            normal_error("token lib","only one table permitted in put_next");
        } else {
            for (i = 1;; i++) {
                lua_rawgeti(L, 1, i); /* table mt token */
                if (lua_type(L,-1) == LUA_TNIL) {
                    break;
                } else {
                    p = lua_touserdata(L, -1);
                    if (p == NULL) {
                        normal_error("token lib","lua <token> expected in put_next (1)");
                    } else if (!lua_getmetatable(L, -1)) { /* table mt token mt */
                        normal_error("token lib","lua <token> expected in put_next (2)");
                    } else if (!lua_rawequal(L, m, -1)) {
                        normal_error("token lib","lua <token> expected in put_next (3)");
                    } else {
                        fast_get_avail(x) ;
                        token_info(x) = token_info(p->token);
                        if (h == null) {
                            h = x;
                        } else {
                            token_link(t) = x;
                        }
                        t = x;
                    }
                    lua_pop(L, 1);
                }
            }
        }
    } else {
        for (i = 1; i <= n; i++) {
            p = lua_touserdata(L,i);
            if (p == NULL) {
                normal_error("token lib","lua <token> expected in put_next (4)");
            } else if (!lua_getmetatable(L, i)) { /* table mt token mt */
                normal_error("token lib","lua <token> expected in put_next (5)");
            } else if (!lua_rawequal(L, m, -1)) {
                normal_error("token lib","lua <token> expected in put_next (6)");
            } else {
                fast_get_avail(x) ;
                token_info(x) = token_info(p->token);
                if (h == null) {
                    h = x;
                } else {
                    token_link(t) = x;
                }
                t = x;
            }
            lua_pop(L, 1);
        }
    }
    if (h == null) {
        /* can't happen */
    } else {
        begin_token_list(h,0);
    }
    lua_settop(L,n);
    return 0;
}

static int run_scan_keyword(lua_State * L)
{
    saved_tex_scanner texstate;
    const char *s = luaL_checkstring(L, -1);
    int v = 0;
    if (s) {
        save_tex_scanner(texstate);
        if (scan_keyword(s)) {
            v = 1;
        }
        unsave_tex_scanner(texstate);
    }
    lua_pushboolean(L,v);
    return 1;
}

static int run_scan_keyword_cs(lua_State * L)
{
    saved_tex_scanner texstate;
    const char *s = luaL_checkstring(L, -1);
    int v = 0;
    if (s) {
        save_tex_scanner(texstate);
        if (scan_keyword_case_sensitive(s)) {
            v = 1;
        }
        unsave_tex_scanner(texstate);
    }
    lua_pushboolean(L,v);
    return 1;
}

static int run_scan_csname(lua_State * L)
{
    unsigned char *s;
    int t;
    saved_tex_scanner texstate;
    save_tex_scanner(texstate);
    get_next();
    t = (cur_cs ? cs_token_flag + cur_cs : token_val(cur_cmd, cur_chr));
    if (t >= cs_token_flag && ((s = get_cs_text(t - cs_token_flag)) != (unsigned char *) NULL)) {
        if (is_active_string(s))
            lua_pushstring(L, (char *) (s + 3));
        else
            lua_pushstring(L, (char *) s);
    } else {
        lua_pushnil(L);
    }
    unsave_tex_scanner(texstate);
    return 1;
}

static int run_scan_int(lua_State * L)
{
    saved_tex_scanner texstate;
    int v = 0;
    save_tex_scanner(texstate);
    scan_int();
    v = cur_val;
    unsave_tex_scanner(texstate);
    lua_pushinteger(L,(int)v);
    return 1;
}

/*
    char * str ;
*/

# define declare_buffer \
    unsigned char word[5 + 1]; \
    char *uindex = (char *)word; \
    luaL_Buffer b ; \
    luaL_buffinit(L,&b) ;

/*
    str = (char *) uni2str(cur_chr);
    luaL_addstring(&b,(char *) str);
    xfree(str);
*/

# define add_to_buffer(chr) \
    if (chr <= 127) { \
        luaL_addchar(&b,(unsigned) (char) chr); \
    } else { \
        uindex = uni2string((char *)word,(unsigned int) chr); \
        *uindex = '\0'; \
        luaL_addstring(&b,(char *) word); \
    }

# define push_buffer \
    luaL_pushresult(&b);

static int run_scan_float_indeed(lua_State * L, boolean exponent)
{
    saved_tex_scanner texstate;
    int ok;
    boolean negative = false;
    double d;
    declare_buffer;
    save_tex_scanner(texstate);
    /* we collapse as in scan_dimen */
    while(1) {
        do {
            get_x_token();
        } while (cur_cmd == spacer_cmd);
        if (cur_tok == minus_token) {
            negative = !negative;
        } else if (cur_tok != plus_token) {
            break;
        }
    }
    if (negative) {
        add_to_buffer('-');
    }
    /* we accept [.,]digits */
    if (cur_tok == point_token || cur_tok == comma_token) {
        add_to_buffer('.');
        while (1) {
            get_x_token();
            if ((cur_tok >= zero_token) && (cur_tok <= nine_token)) {
                add_to_buffer(cur_chr);
            } else if (exponent) {
                goto EXPONENT;
            } else {
                back_input();
                goto DONE;
            }
        }
    } else {
        back_input();
        while (1) {
            get_x_token();
            if ((cur_tok >= zero_token) && (cur_tok <= nine_token)) {
                add_to_buffer(cur_chr);
            } else {
                if (cur_tok == point_token || cur_tok == comma_token) {
                    add_to_buffer('.');
                    while (1) {
                        get_x_token();
                        if ((cur_tok >= zero_token) && (cur_tok <= nine_token)) {
                            add_to_buffer(cur_chr);
                        } else {
                            back_input();
                            break;
                        }
                    }
                } else if (exponent) {
                    goto EXPONENT;
                } else {
                    back_input();
                    goto DONE;
                }
            }
        }
    }
EXPONENT:
    if ((cur_chr == 'E') || (cur_chr == 'e')) {
        add_to_buffer(cur_chr);
        get_x_token();
        if ((cur_tok == minus_token) || (cur_tok == plus_token)) {
            add_to_buffer(cur_chr);
        } else if ((cur_tok >= zero_token) && (cur_tok <= nine_token)) {
            add_to_buffer(cur_chr);
        }
        while (1) {
            get_x_token();
            if ((cur_tok >= zero_token) && (cur_tok <= nine_token)) {
                add_to_buffer(cur_chr);
            } else {
                break;
            }
        }
    }
    back_input();
DONE:
    push_buffer;
    d = lua_tonumberx(L,1,&ok);
    if (ok) {
        lua_pushnumber(L,d);
    } else {
        lua_pushnil(L);
    }
    unsave_tex_scanner(texstate);
    return 1;
}

static int run_scan_float(lua_State * L)
{
    return run_scan_float_indeed(L,true);
}

static int run_scan_real(lua_State * L)
{
    return run_scan_float_indeed(L,false);
}

static int run_scan_dimen(lua_State * L)
{
    saved_tex_scanner texstate;
    int v = 0, o = 0;
    int inf = false, mu = false;
    int t = lua_gettop(L);
    if (t>0)
      inf = lua_toboolean(L,1); /* inf values allowed ?*/
    if (t>1)
      mu = lua_toboolean(L,2);  /* mu units required ?*/
    save_tex_scanner(texstate);
    scan_dimen(mu,inf, false); /* arg3 = shortcut */
    v = cur_val;
    o = cur_order;
    unsave_tex_scanner(texstate);
    lua_pushinteger(L,v);
    if (inf) {
        lua_pushinteger(L,(lua_Number)o);
        return 2;
    } else {
        return 1;
    }
}

static int run_scan_glue(lua_State * L)
{
    saved_tex_scanner texstate;
    int v = 0;
    int mu = false;
    int t = lua_gettop(L);
    if (t>0)
      mu = lua_toboolean(L,1); /* mu units required ?*/
    save_tex_scanner(texstate);
    scan_glue((mu ? mu_val_level : glue_val_level));
    v = cur_val; /* which is a glue_spec node */
    unsave_tex_scanner(texstate);
    lua_nodelib_push_fast(L,(halfword)v);
    return 1;
}

static int run_scan_toks(lua_State * L)
{
    saved_tex_scanner texstate;
    int macro_def = false, xpand = false;
    halfword t, saved_defref;
    int i = 1;
    int top = lua_gettop(L);
    if (top>0)
      macro_def = lua_toboolean(L,1); /* \\def ? */
    if (top>1)
      xpand = lua_toboolean(L,2); /* expand ? */
    save_tex_scanner(texstate);
    saved_defref = def_ref;
    (void) scan_toks(macro_def, xpand);
    t = def_ref;
    unsave_tex_scanner(texstate);
    def_ref = saved_defref;
    /* This function returns a pointer to the tail of a new token
       list, and it also makes |def_ref| point to the reference count at the
       head of that list. */
    lua_newtable(L);
    while (token_link(t)) {
        t = token_link(t);
        push_token(L,t);
        lua_rawseti(L,-2,i++);
    }
    return 1;
}

static int run_scan_string(lua_State * L) /* HH */
{   /* can be simplified, no need for intermediate list */
    saved_tex_scanner texstate;
    halfword t, saved_defref;
    save_tex_scanner(texstate);
    do {
        get_x_token();
    } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));
    if (cur_cmd == left_brace_cmd) {
        back_input();
        saved_defref = def_ref;
        (void) scan_toks(false, true);
        t = def_ref;
        def_ref = saved_defref;
        tokenlist_to_luastring(L,t);
        flush_list(t);
    } else if (cur_cmd == call_cmd) {
        t = token_link(cur_chr);
        tokenlist_to_luastring(L,t);
        flush_list(t);
    } else if (cur_cmd == 11 || cur_cmd == 12 ) {
        declare_buffer;
        while (1) {
            add_to_buffer(cur_chr);
            get_x_token();
            if (cur_cmd != 11 && cur_cmd != 12 ) {
                break ;
            }
        }
        back_input();
        push_buffer;
    } else {
        back_input();
        lua_pushnil(L);
    }
    unsave_tex_scanner(texstate);
    return 1;
}

static int run_scan_argument(lua_State * L) /* HH */
{   /* can be simplified, no need for intermediate list */
    saved_tex_scanner texstate;
    halfword t, saved_defref;
    save_tex_scanner(texstate);
    do {
        get_token();
    } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));
    if (cur_cmd == left_brace_cmd) {
        back_input();
        saved_defref = def_ref;
        (void) scan_toks(false, true);
        t = def_ref;
        def_ref = saved_defref;
        tokenlist_to_luastring(L,t);
        flush_list(t);
    } else if (cur_cmd == call_cmd) {
        halfword saved_cur_tok = cur_tok;
        cur_tok = right_brace_token + '}';
        back_input();
        cur_tok = saved_cur_tok;
        back_input();
        cur_tok = left_brace_token + '{';
        back_input();
        saved_defref = def_ref;
        (void) scan_toks(false, true);
        t = def_ref;
        def_ref = saved_defref;
        tokenlist_to_luastring(L,t);
        flush_list(t);
    } else if (cur_cmd == 11 || cur_cmd == 12 ) {
        declare_buffer;
        while (1) {
            add_to_buffer(cur_chr);
            get_x_token();
            if (cur_cmd != 11 && cur_cmd != 12 ) {
                break ;
            }
        }
        back_input();
        push_buffer;
    } else {
        back_input();
        lua_pushnil(L);
    }
    unsave_tex_scanner(texstate);
    return 1;
}

static int run_scan_word(lua_State * L) /* HH */
{
    saved_tex_scanner texstate;
    save_tex_scanner(texstate);
    do {
        get_x_token();
    } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));
    if (cur_cmd == 11 || cur_cmd == 12 ) {
        declare_buffer;
        while (1) {
            add_to_buffer(cur_chr);
            get_x_token();
            if (cur_cmd != 11 && cur_cmd != 12 ) {
                break ;
            }
        }
        back_input();
        push_buffer;
    } else {
        back_input();
        lua_pushnil(L);
    }
    unsave_tex_scanner(texstate);
    return 1;
}

static int run_scan_code(lua_State * L) /* HH */
{
    saved_tex_scanner texstate;
    int cc = DEFAULT_SCAN_CODE_SET ;
    save_tex_scanner(texstate);
    get_x_token();
    if (cur_cmd < 16) {
        if (lua_gettop(L)>0) {
            cc = (int) lua_tointeger(L,-1);
            if (cc == null) {
                /* todo: message that we choose a default */
                cc = DEFAULT_SCAN_CODE_SET ;
            }
        }
        if (cc & (1<<(cur_cmd))) {
            lua_pushinteger(L,(int)cur_chr);
        } else {
            lua_pushnil(L);
            back_input();
        }
    } else {
        lua_pushnil(L);
        back_input();
    }
    unsave_tex_scanner(texstate);
    return 1;
}

static int lua_tokenlib_is_token(lua_State * L) /* HH */
{
    lua_pushboolean(L,maybe_istoken(L,1)==NULL ? 0 : 1);
    return 1;
}

static int run_expand(lua_State * L)
{
    (void) L;
    expand();
    return 0;
}

static int run_lookup(lua_State * L)
{
    const char *s;
    size_t l;
    int cs, cmd, chr;
    if (lua_type(L, -1) == LUA_TSTRING) {
        s = lua_tolstring(L, -1, &l);
        if (l > 0) {
            cs = string_lookup(s, l);
            cmd = eq_type(cs);
            chr = equiv(cs);
            make_new_token(L, cmd, chr, cs);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

static int lua_tokenlib_is_defined(lua_State * L)
{
    const char *s;
    size_t l;
    if (lua_type(L, -1) == LUA_TSTRING) {
        s = lua_tolstring(L, -1, &l);
        if (l > 0) {
            lua_pushboolean(L,string_lookup(s, l) != undefined_control_sequence);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

static int run_build(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int cs = 0;
        int chr = (int) lua_tointeger(L, 1);
        int cmd = (int) luaL_optinteger(L, 2, get_cat_code(cat_code_table_par,chr));
        if (cmd == 0 || cmd == 9 || cmd == 14 || cmd == 15) {
            formatted_warning("token lib","not a good token, catcode %i can not be returned, so 12 will be used",(int) cmd);
            cmd = 12;
        } else if (cmd == 13) {
            cs = active_to_cs(chr, false);
            cmd = eq_type(cs);
            chr = equiv(cs);
        }
        make_new_token(L, cmd, chr, cs);
        return 1;
    } else {
        return run_lookup(L);
    }
}

static int run_new(lua_State * L)
{
    int cs = 0;
    int chr = (int) lua_tointeger(L, 1);
    int cmd = (int) lua_tointeger(L, 2);
    make_new_token(L, cmd, chr, cs);
    return 1;
}

/* token instance functions */

static int lua_tokenlib_free(lua_State * L)
{
    lua_token *n;
    n = check_istoken(L, 1);
    if (n->origin == LUA_ORIGIN) {
        free_avail(n->token);
    }
    return 1;
}

/* fast accessors */

inline static int lua_tokenlib_get_command(lua_State * L)
{
    lua_token *n = check_istoken(L, 1);
    halfword t = token_info(n->token);
    if (t >= cs_token_flag) {
        lua_pushinteger(L,(int) eq_type((t - cs_token_flag)));
    } else {
        lua_pushinteger(L, token_cmd(t));
    }
    return 1;
}

inline static int lua_tokenlib_get_index(lua_State * L)
{
    int cmd, chr;
    lua_token *n = check_istoken(L, 1);
    halfword t = token_info(n->token);
    if (t >= cs_token_flag) {
        cmd = eq_type(t - cs_token_flag);
        chr = equiv(t - cs_token_flag);
    } else {
        cmd = token_cmd(t);
        chr = token_chr(t);
    }
    switch (cmd) {
        case assign_int_cmd:
            chr -= count_base;
            break;
        case assign_attr_cmd:
            chr -= attribute_base;
            break;
        case assign_dimen_cmd:
            chr -= scaled_base;
            break;
        case assign_glue_cmd:
            chr -= skip_base;
            break;
        case assign_mu_glue_cmd:
            chr -= mu_skip_base;
            break;
        case assign_toks_cmd:
            chr -= toks_base;
            break;
        default:
            break;
    }
    if (chr >= 0 && chr <= 65535) {
        lua_pushinteger(L, chr);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

inline static int lua_tokenlib_get_mode(lua_State * L)
{
    lua_token *n = check_istoken(L, 1);
    halfword t = token_info(n->token);
    if (t >= cs_token_flag) {
        lua_pushinteger(L, equiv(t - cs_token_flag));
    } else {
        lua_pushinteger(L, token_chr(t));
    }
    return 1;
}

inline static int lua_tokenlib_get_cmdname(lua_State * L)
{
    lua_token *n = check_istoken(L, 1);
    halfword t = token_info(n->token);
    int cmd = (t >= cs_token_flag ? eq_type(t - cs_token_flag) : token_cmd(t));
    lua_push_string_by_index(L, command_names[cmd].lua);
    return 1;
}

inline static int lua_tokenlib_get_csname(lua_State * L)
{
    lua_token *n = check_istoken(L, 1);
    halfword t = token_info(n->token);
    unsigned char *s;
    if (t >= cs_token_flag && ((s = get_cs_text(t - cs_token_flag)) != (unsigned char *) NULL)) {
        if (is_active_string(s))
            lua_pushstring(L, (char *) (s + 3));
        else
            lua_pushstring(L, (char *) s);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

inline static int lua_tokenlib_get_id(lua_State * L)
{
    lua_token *n = check_istoken(L, 1);
    lua_pushinteger(L, n->token);
    return 1;
}

inline static int lua_tokenlib_get_tok(lua_State * L)
{
    lua_token *n = check_istoken(L, 1);
    halfword t = token_info(n->token);
    lua_pushinteger(L, t);
    return 1;
}

inline static int lua_tokenlib_get_active(lua_State * L)
{
    lua_token *n = check_istoken(L, 1);
    halfword t = token_info(n->token);
    unsigned char *s;
    if (t >= cs_token_flag && ((s = get_cs_text(t - cs_token_flag)) != (unsigned char *) NULL)) {
        if (is_active_string(s))
            lua_pushboolean(L,1);
        else
            lua_pushboolean(L,0);
        free(s);
    } else {
        lua_pushboolean(L,0);
    }
    return 1;
}

inline static int lua_tokenlib_get_expandable(lua_State * L)
{
    lua_token *n = check_istoken(L, 1);
    halfword t = token_info(n->token);
    int cmd = (t >= cs_token_flag ? eq_type(t - cs_token_flag) : token_cmd(t));
    if (cmd > max_command_cmd) {
        lua_pushboolean(L, 1);
    } else {
        lua_pushboolean(L, 0);
    }
    return 1;
}

inline static int lua_tokenlib_get_protected(lua_State * L)
{
    lua_token *n = check_istoken(L, 1);
    halfword t = token_info(n->token);
    int cmd = (t >= cs_token_flag ? eq_type(t - cs_token_flag) : token_cmd(t));
    if (cmd > max_command_cmd && (cmd >= call_cmd) && (cmd < end_template_cmd)) {
        int chr = (t >= cs_token_flag ? equiv(t - cs_token_flag) : token_chr(t));
        if (token_info(token_link(chr)) == protected_token) {
            lua_pushboolean(L, 1);
        } else {
            lua_pushboolean(L, 0);
        }
    } else {
        lua_pushboolean(L, 0);
    }
    return 1;
}

static int lua_tokenlib_getfield(lua_State * L)
{
    const char *s = lua_tostring(L, 2);
    if (lua_key_eq(s, command)) {
        return lua_tokenlib_get_command(L);
    } else if (lua_key_eq(s, index)) {
        return lua_tokenlib_get_index(L);
    } else if (lua_key_eq(s, mode)) {
        return lua_tokenlib_get_mode(L);
    } else if (lua_key_eq(s, cmdname)) {
        return lua_tokenlib_get_cmdname(L);
    } else if (lua_key_eq(s, csname)) {
        return lua_tokenlib_get_csname(L);
    } else if (lua_key_eq(s, id)) {
        return lua_tokenlib_get_id(L);
    } else if (lua_key_eq(s, tok)) {
        return lua_tokenlib_get_tok(L);
    } else if (lua_key_eq(s, active)) {
        return lua_tokenlib_get_active(L);
    } else if (lua_key_eq(s, expandable)) {
        return lua_tokenlib_get_expandable(L);
    } else if (lua_key_eq(s, protected)) {
        return lua_tokenlib_get_protected(L);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* end */

static int lua_tokenlib_equal(lua_State * L)
{
    lua_token *n, *m;
    n = check_istoken(L, 1);
    m = check_istoken(L, 2);
    if (token_info(n->token) == token_info(m->token)) {
    lua_pushboolean(L,1);
        return 1;
    }
    lua_pushboolean(L,0);
    return 1;
}

static int lua_tokenlib_tostring(lua_State * L)
{
    char *msg;
    lua_token *n;
    n = check_istoken(L, 1);
    msg = xmalloc(256);
    snprintf(msg, 255, "<%s token %d: %d>", (n->origin==LUA_ORIGIN?"lua":"tex"), n->token , token_info(n->token));
    lua_pushstring(L, msg);
    free(msg);
    return 1;
}

static int lua_tokenlib_type(lua_State * L)
{
    if (maybe_istoken(L,1)!=NULL) {
        lua_pushstring(L,"token");
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int run_scan_token(lua_State * L)
{
    saved_tex_scanner texstate;
    save_tex_scanner(texstate);
    get_x_token();
    make_new_token(L, cur_cmd, cur_chr, cur_cs);
    unsave_tex_scanner(texstate);
    return 1;
}

static int run_scan_list(lua_State * L)
{
    saved_tex_scanner texstate;
    save_tex_scanner(texstate);
    /*tex
        This is s tricky call as we are in \LUA\ and therefore
        mess with the main loop.
    */
    lua_nodelib_push_fast(L, local_scan_box());
    unsave_tex_scanner(texstate);
    return 1;
}

/* experiment */

/* [catcodetable] csname content        : \def\csname{content}  */
/* [catcodetable] csname content global : \gdef\csname{content} */
/* [catcodetable] csname                : \def\csname{}         */

/* TODO: check for a quick way to set a macro to empty (HH) */

static int get_meaning(lua_State * L)
{
    const char *name = null;
    size_t lname = 0;
    int cs, cmd;
    if (lua_type(L, 1) == LUA_TSTRING) {
        name = lua_tolstring(L, 1, &lname);
        cs = string_lookup(name, lname);
        cmd = eq_type(cs);
        if (cmd >= call_cmd) {
            int chr = equiv(cs);
            char *str = tokenlist_to_cstring(chr, true, NULL);
            lua_pushstring(L, str);
            free(str);
            return 1;
        }
    }
    return 0;
}

static int get_macro(lua_State * L)
{
    const char *name = null;
    size_t lname = 0;
    int cs, cmd;
    if (lua_type(L, 1) == LUA_TSTRING) {
        name = lua_tolstring(L, 1, &lname);
        cs = string_lookup(name, lname);
        cmd = eq_type(cs);
        if (cmd >= call_cmd) {
            /*
                Expanding would expand in-place, unless we make a copy which we don't want to
                do. So, we just pass the meaning i.e. no: expand_macros_in_tokenlist(chr).

                Actually it would be nice to adapt tokenlist_to_cstring with an extra argument
                indicating that we are not interested in the before -> part.
            */
            int chr = equiv(cs);
            char *str = tokenlist_to_xstring(chr, true, NULL);
            lua_pushstring(L, str);
            free(str);
            return 1;
        }
    }
    return 0;
}

static int set_lua(lua_State *L)
{
    const char *name = null;
    const char *s  = null;
    size_t lname = 0;
    int cs;
    int n = lua_gettop(L);
    int a = 0; /* global state */
    int p = 0; /* protected state */
    int f = 0; /* function index */
    int nncs = no_new_control_sequence;
    if (n < 2) {
        return 0 ;
    }
    name = lua_tolstring(L, 1, &lname);
    if (name == null) {
        return 0 ;
    }
    f = lua_tointeger(L, 2);
    if (n > 2)  {
        s = lua_tostring(L, 3);
        if (s) {
            if (lua_key_eq(s, global)) {
                a = 4;
            } else if (lua_key_eq(s, protected)) {
                p = 1;
            }
        }
        if (n > 3) {
            s = lua_tostring(L, 4);
            if (s) {
                if (lua_key_eq(s, global)) {
                    a = 4;
                } else if (lua_key_eq(s, protected)) {
                    p = 1;
                }
            }
        }
    }
    no_new_control_sequence = false ;
    cs = string_lookup(name, lname);
    no_new_control_sequence = nncs;
    if (p) {
        define(cs, lua_call_cmd, f);
    } else {
        define(cs, lua_expandable_call_cmd, f);
    }
    return 0;
}

static int set_macro(lua_State * L)
{
    const char *name = null;
    const char *str = null;
    const char *s  = null;
    size_t lname = 0;
    size_t lstr = 0;
    int cs, cc, ct;
    int n = lua_gettop(L);
    int a = 0 ; /* global state */
    int nncs = no_new_control_sequence;
    if (n == 0) {
        return 0 ;
    }
    if (lua_type(L, 1) == LUA_TNUMBER) {
        if (n == 1)
            return 0;
        ct = (int) lua_tointeger(L, 1);
        name = lua_tolstring(L, 2, &lname);
        if (n > 2)
            str = lua_tolstring(L, 3, &lstr);
        if (n > 3)
            s = lua_tostring(L, 4);
    } else {
        ct = cat_code_table_par;
        name = lua_tolstring(L, 1, &lname);
        if (n > 1)
            str = lua_tolstring(L, 2, &lstr);
        if (n > 2)
            s = lua_tostring(L, 3);
    }
    if (name == null) {
        return 0 ;
    }
    if (s && (lua_key_eq(s, global))) {
        a = 4;
    }
    no_new_control_sequence = false ;
    cs = string_lookup(name, lname);
    no_new_control_sequence = nncs;
    if (lstr > 0) {
        halfword p; /* tail of the token list */
        halfword q; /* new node being added to the token list via |store_new_token| */
        halfword t; /* token being appended */
        const char *se = str + lstr;
        p = temp_token_head;
        set_token_link(p, null);
        /* this left brace is used to store the number of arguments */
        fast_store_new_token(left_brace_token);
        /* and this ends the not present arguments, and no: we will not support arguments here*/
        fast_store_new_token(end_match_token);
        while (str < se) {
            /* hh: str2uni could return len too (also elsewhere) */
            t = (halfword) str2uni((const unsigned char *) str);
            str += utf8_size(t);
            cc = get_cat_code(ct,t);
            /* this is a relating simple converter; if more is needed one can just use     */
            /* tex.print with a regular \def or \gdef and feed the string into the regular */
            /* scanner;                                                                    */
            if (cc == 0) {
                /* we have a potential control sequence so we check for it */
                int _lname = 0 ;
                int _s = 0 ;
                int _c = 0 ;
                halfword _cs = null ;
                const char *_name  = str ;
                while (str < se) {
                    t = (halfword) str2uni((const unsigned char *) str);
                    _s = utf8_size(t);
                    _c = get_cat_code(ct,t);
                    if (_c == 11) {
                        str += _s ;
                        _lname = _lname + _s ;
                    } else if (_c == 10) {
                        /* we ignore a trailing space like normal scanning does */
                        str += _s ;
                        break ;
                    } else {
                        break ;
                    }
                }
                if (_s > 0) {
                    /* we have a potential \cs */
                    _cs = string_lookup(_name, _lname);
                    if (_cs == undefined_control_sequence) {
                        /* let's play safe and backtrack */
                        t = cc * (1<<21) + t ;
                        str = _name ;
                    } else {
                        t = cs_token_flag + _cs;
                    }
                } else {
                    /* just a character with some meaning, so \unknown becomes effectively */
                    /* \\unknown assuming that \\ has some useful meaning of course        */
                    t = cc * (1<<21) + t ;
                    str = _name ;
                }

            } else {
                /* whatever token, so for instance $x^2$ just works given a tex */
                /* catcode regime */
                t = cc * (1<<21) + t ;
            }
            fast_store_new_token(t);
        }
        /* there is no fast_store_new_token(right_brace_token) needed */
        define(cs, call_cmd + (a % 4), token_link(temp_token_head));
    } else {
        halfword p ;
        halfword q; /* new node being added to the token list via |store_new_token| */
        p = temp_token_head;
        set_token_info(p,null);
        fast_store_new_token(left_brace_token);
        fast_store_new_token(end_match_token);
        define(cs, call_cmd + (a % 4), token_link(temp_token_head));
    }
    return 0;
}

static int set_char(lua_State * L)
{
    const char *name = null;
    const char *s  = null;
    size_t lname = 0;
    int cs, value;
    int n = lua_gettop(L);
    int a = 0 ; /* global state */
    int nncs = no_new_control_sequence;
    if (n < 2)
        return 0;
    name = lua_tolstring(L, 1, &lname);
    if (name == NULL)
        return 0;
    value = lua_tointeger(L, 2);
    if (value < 0)
        return 0;
    if (n > 2)
        s = lua_tostring(L, 3);
    if (s && (lua_key_eq(s, global))) {
        a = 4;
    }
    no_new_control_sequence = false ;
    cs = string_lookup(name, lname);
    no_new_control_sequence = nncs;
    define(cs, char_given_cmd, value);
    return 0;
}

static int get_command_names(lua_State * L)
{
    int i;
    lua_createtable(L,data_cmd+1,0);
    for (i = 0; command_names[i].lua != 0; i++) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, command_names[i].lua);
        lua_rawseti(L, -2, i);
    }
    return 1;
}

static const struct luaL_Reg tokenlib[] = {
    { "type", lua_tokenlib_type },
    { "create", run_build },
    { "new", run_new },
    { "is_token", lua_tokenlib_is_token },
    { "is_defined", lua_tokenlib_is_defined },
    { "commands", get_command_names },
    { "command_id", run_get_command_id },
    { "biggest_char", run_get_biggest_char },
    /* scanners */
    { "get_next", run_get_next },
    { "scan_keyword", run_scan_keyword },
    { "scan_keyword_cs", run_scan_keyword_cs },
    { "scan_int", run_scan_int },
    { "scan_float", run_scan_float },
    { "scan_real", run_scan_real },
    { "scan_dimen", run_scan_dimen },
    { "scan_glue", run_scan_glue },
    { "scan_toks", run_scan_toks },
    { "scan_code", run_scan_code },
    { "scan_string", run_scan_string },
    { "scan_argument", run_scan_argument },
    { "scan_word", run_scan_word },
    { "scan_csname", run_scan_csname },
    { "scan_token", run_scan_token }, /* expands next token if needed */
    { "scan_list", run_scan_list },
    /* writers */
    { "put_next", run_put_next },
    { "expand", run_expand },
    /* getters */
    { "get_command", lua_tokenlib_get_command },
    { "get_index", lua_tokenlib_get_index },
    { "get_mode", lua_tokenlib_get_mode },
    { "get_cmdname", lua_tokenlib_get_cmdname },
    { "get_csname", lua_tokenlib_get_csname },
    { "get_id", lua_tokenlib_get_id },
    { "get_tok", lua_tokenlib_get_tok },
    { "get_active", lua_tokenlib_get_active },
    { "get_expandable", lua_tokenlib_get_expandable },
    { "get_protected", lua_tokenlib_get_protected },
    { "get_macro", get_macro },
    { "get_meaning", get_meaning },
    /* setters */
    { "set_macro", set_macro },
    { "set_char", set_char },
    { "set_lua", set_lua },
    {NULL, NULL}
};

static const struct luaL_Reg tokenlib_m[] = {
    {"__index", lua_tokenlib_getfield},
    {"__tostring", lua_tokenlib_tostring},
    {"__eq", lua_tokenlib_equal},
    {"__gc", lua_tokenlib_free},
    {NULL, NULL} /* sentinel */
};

int luaopen_token(lua_State * L)
{
    /* the main metatable of token userdata */
    luaL_newmetatable(L, TOKEN_METATABLE);
    luaL_openlib(L, NULL, tokenlib_m, 0);
    luaL_openlib(L, "token", tokenlib, 0);
    return 1;
}
