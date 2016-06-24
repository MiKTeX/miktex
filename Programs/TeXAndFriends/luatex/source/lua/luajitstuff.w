% luastuff.w
%
% Copyright 2006-2013 Taco Hoekwater <taco@@luatex.org>
%
% This file is part of LuaTeX.
%
% LuaTeX is free software; you can redistribute it and/or modify it under
% the terms of the GNU General Public License as published by the Free
% Software Foundation; either version 2 of the License, or (at your
% option) any later version.
%
% LuaTeX is distributed in the hope that it will be useful, but WITHOUT
% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
% FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
% License for more details.
%
% You should have received a copy of the GNU General Public License along
% with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

@ @c
#include "ptexlib.h"
#include "lua/luatex-api.h"
#include "lua/lauxlib_bridge.h"


@ @c
lua_State *Luas = NULL;

int luastate_bytes = 0;
int lua_active = 0;

@ @c
void make_table(lua_State * L, const char *tab, const char *mttab, const char *getfunc,
                const char *setfunc)
{
    /* make the table *//* |[{<tex>}]| */
    lua_pushstring(L, tab);     /* |[{<tex>},"dimen"]| */
    lua_newtable(L);            /* |[{<tex>},"dimen",{}]| */
    lua_settable(L, -3);        /* |[{<tex>}]| */
    /* fetch it back */
    lua_pushstring(L, tab);     /* |[{<tex>},"dimen"]| */
    lua_gettable(L, -2);        /* |[{<tex>},{<dimen>}]| */
    /* make the meta entries */
    luaL_newmetatable(L, mttab);  /* |[{<tex>},{<dimen>},{<dimen_m>}]| */
    lua_pushstring(L, "__index");       /* |[{<tex>},{<dimen>},{<dimen_m>},"__index"]| */
    lua_pushstring(L, getfunc); /* |[{<tex>},{<dimen>},{<dimen_m>},"__index","getdimen"]| */
    lua_gettable(L, -5);        /* |[{<tex>},{<dimen>},{<dimen_m>},"__index",<tex.getdimen>]| */
    lua_settable(L, -3);        /* |[{<tex>},{<dimen>},{<dimen_m>}]|  */
    lua_pushstring(L, "__newindex");    /* |[{<tex>},{<dimen>},{<dimen_m>},"__newindex"]| */
    lua_pushstring(L, setfunc); /* |[{<tex>},{<dimen>},{<dimen_m>},"__newindex","setdimen"]| */
    lua_gettable(L, -5);        /* |[{<tex>},{<dimen>},{<dimen_m>},"__newindex",<tex.setdimen>]| */
    lua_settable(L, -3);        /* |[{<tex>},{<dimen>},{<dimen_m>}]| */
    lua_setmetatable(L, -2);    /* |[{<tex>},{<dimen>}]| : assign the metatable */
    lua_pop(L, 1);              /* |[{<tex>}]| : clean the stack */
}

@ @c
static const char *getS(lua_State * L, void *ud, size_t * size)
{
    LoadS *ls = (LoadS *) ud;
    (void) L;
    if (ls->size == 0)
        return NULL;
    *size = ls->size;
    ls->size = 0;
    return ls->s;
}

@ @c
#if 0 /* currently unused */
static void *my_luaalloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
    void *ret = NULL;
    (void) ud;                  /* for -Wunused */
    if (nsize == 0)
        free(ptr);
    else
        ret = realloc(ptr, nsize);
    luastate_bytes += (int) (nsize - osize);
    return ret;
}
#endif

@ @c
static int my_luapanic(lua_State * L)
{
    (void) L;                   /* to avoid warnings */
    fprintf(stderr, "PANIC: unprotected error in call to Lua API (%s)\n",
            lua_tostring(L, -1));
    return 0;
}

@ @c
void luafunctioncall(int slot)
{
    int i ;
    int stacktop = lua_gettop(Luas);
    lua_active++;
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, lua_key_index(lua_functions));
    lua_gettable(Luas, LUA_REGISTRYINDEX);
    lua_rawgeti(Luas, -1,slot);
    if (lua_isfunction(Luas,-1)) {
        int base = lua_gettop(Luas); /* function index */
        lua_pushinteger(Luas, slot);
        lua_pushcfunction(Luas, lua_traceback); /* push traceback function */
        lua_insert(Luas, base); /* put it under chunk  */
        i = lua_pcall(Luas, 1, 0, base);
        lua_remove(Luas, base); /* remove traceback function */
        if (i != 0) {
            lua_gc(Luas, LUA_GCCOLLECT, 0);
            Luas = luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
        }
    }
    lua_settop(Luas,stacktop);
    lua_active--;
}

@ @c
static const luaL_Reg lualibs[] = {
    {"", luaopen_base},
    {"package", luaopen_package},
    /*{"coroutine", luaopen_coroutine},*/
    {"table", luaopen_table},
    {"io", open_iolibext},
    {"os", luaopen_os},
    {"string", luaopen_string},
    {"math", luaopen_math},
    {"debug", luaopen_debug},
    {"unicode", luaopen_unicode},
    {"zip", luaopen_zip},
    {"bit32", luaopen_bit32},
    {"md5", luaopen_md5},
    {"lfs", luaopen_lfs},
    {"profiler", luaopen_profiler},
    {"jit", luaopen_jit},
    {"ffi", luaopen_ffi},
    {"bit",	luaopen_bit },
    {NULL, NULL}
};

static const luaL_Reg lualibs_nofenv[] = {
    {"lpeg", luaopen_lpeg},
    {NULL, NULL}
};

@ @c
static void do_openlibs(lua_State * L)
{
    const luaL_Reg *lib = lualibs;
    for (; lib->func; lib++) {
        lua_pushcfunction(L, lib->func);
        lua_pushstring(L, lib->name);
        lua_call(L, 1, 0);
    }
    lib = lualibs_nofenv;
    for (; lib->func; lib++) {
        lua_pushcfunction(L, lib->func);
        lua_newtable(L);
        lua_setfenv(L,-2);
        lua_pushstring(L, lib->name);
        lua_call(L, 1, 0);
    }
}


@ @c
static int luatex_loadfile (lua_State *L) {
  int status = 0;
  const char *fname = luaL_optstring(L, 1, NULL);
  const char *mode = luaL_optstring(L, 2, NULL);
  /*int env = !lua_isnone(L, 3);*/  /* 'env' parameter? */
  if (!lua_only && !fname && interaction == batch_mode) {
     lua_pushnil(L);
     lua_pushstring(L, "reading from stdin is disabled in batch mode");
     return 2;  /* return nil plus error message */
  }
  status = luaL_loadfilex(L, fname, mode);
  if (status == LUA_OK) {
    recorder_record_input(fname);
    /* if (env) {  *//* 'env' parameter? */
    /*  lua_pushvalue(L, 3); */
    /*  lua_setupvalue(L, -2, 1); *//* set it as 1st upvalue of loaded chunk */
    /*}                       */
  }
  return RESERVED_load_aux_JIT(L, status,3);
}

@ @c
static int luatex_dofile (lua_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  int n = lua_gettop(L);
  if (!lua_only && !fname) {
      if (interaction == batch_mode) {
	  lua_pushnil(L);
	  lua_pushstring(L, "reading from stdin is disabled in batch mode");
	  return 2;  /* return nil plus error message */
      } else {
	  tprint_nl("lua> ");
      }
  }
  if (luaL_loadfile(L, fname) != 0) lua_error(L);
  recorder_record_input(fname);
  lua_call(L, 0, LUA_MULTRET);
  return lua_gettop(L) - n;
}

@ @c
void luainterpreter(void)
{
    lua_State *L;

    if (jithash_hashname==NULL){
	/* default lua51 */
	luajittex_choose_hash_function = 0;
        jithash_hashname = (char *) xmalloc(strlen("lua51")+1);
        jithash_hashname = strcpy ( jithash_hashname, "lua51");
    } else {
      if (strcmp((const char*)jithash_hashname,"lua51")==0){
	luajittex_choose_hash_function = 0;
      }else if (strcmp((const char*)jithash_hashname,"luajit20")==0){
	luajittex_choose_hash_function = 1;
      } else {
	/* default lua51 */
	luajittex_choose_hash_function = 0;
	jithash_hashname = strcpy ( jithash_hashname, "lua51");
      }
    }

    L = luaL_newstate() ;
    /*L = lua_newstate(my_luaalloc, NULL);*/
    if (L == NULL) {
        fprintf(stderr, "Can't create the Lua state.\n");
        return;
    }
    lua_atpanic(L, &my_luapanic);

    do_openlibs(L);             /* does all the 'simple' libraries */

    if (luajiton){
     luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE|LUAJIT_MODE_ON);
    }
    else {
       luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE|LUAJIT_MODE_OFF);
    }

    lua_pushcfunction(L,luatex_dofile);
    lua_setglobal(L, "dofile");
    lua_pushcfunction(L,luatex_loadfile);
    lua_setglobal(L, "loadfile");

    luatex_md5_lua_open(L);

    open_oslibext(L, safer_option);
/*
    open_iolibext(L);
*/
    open_strlibext(L);
    open_lfslibext(L);

    /* luasockets */
    /* socket and mime are a bit tricky to open because
     they use a load-time  dependency that has to be
     worked around for luatex, where the C module is
     loaded way before the lua module.
     */
    if (!nosocket_option) {
        lua_getglobal(L, "package");
        lua_getfield(L, -1, "loaded");
        if (!lua_istable(L, -1)) {
            lua_newtable(L);
            lua_setfield(L, -2, "loaded");
            lua_getfield(L, -1, "loaded");
        }
        luaopen_socket_core(L);
        lua_setfield(L, -2, "socket.core");
        lua_pushnil(L);
        lua_setfield(L, -2, "socket");  /* package.loaded.socket = nil */

        luaopen_mime_core(L);
        lua_setfield(L, -2, "mime.core");
        lua_pushnil(L);
        lua_setfield(L, -2, "mime");    /* package.loaded.mime = nil */
        lua_pop(L, 2);          /* pop the tables */

        luatex_socketlua_open(L);       /* preload the pure lua modules */
    }
    /* zlib. slightly odd calling convention */
    luaopen_zlib(L);
    lua_setglobal(L, "zlib");
    luaopen_gzip(L);

    /* our own libraries */
    luaopen_ff(L);
    luaopen_tex(L);
    luaopen_token(L);
    luaopen_node(L);
    luaopen_texio(L);
    luaopen_kpse(L);
    luaopen_callback(L);
    luaopen_lua(L, startup_filename);
    luaopen_stats(L);
    luaopen_font(L);
    luaopen_lang(L);
    luaopen_mplib(L);
    luaopen_vf(L);

    /* |luaopen_pdf(L);| */
    /* environment table at |LUA_ENVIRONINDEX| needs to load this way: */
    lua_pushcfunction(L, luaopen_pdf);
    lua_pushstring(L, "pdf");
    lua_call(L, 1, 0);

    if (!lua_only) {
        /* |luaopen_img(L);| */
        lua_pushcfunction(L, luaopen_img);
        lua_pushstring(L, "img");
        lua_call(L, 1, 0);
    }

    /* |luaopen_epdf(L);| */
    lua_pushcfunction(L, luaopen_epdf);
    lua_pushstring(L, "epdf");
    lua_call(L, 1, 0);

    /* |luaopen_pdfscanner(L);| */
    lua_pushcfunction(L, luaopen_pdfscanner);
    lua_pushstring(L, "pdfscanner");
    lua_call(L, 1, 0);

    lua_createtable(L, 0, 0);
    lua_setglobal(L, "texconfig");

    if (safer_option) {
        /* disable some stuff if --safer */
        (void) hide_lua_value(L, "os", "execute");
        (void) hide_lua_value(L, "os", "rename");
        (void) hide_lua_value(L, "os", "remove");
        (void) hide_lua_value(L, "io", "popen");
        /* make io.open only read files */
        luaL_checkstack(L, 2, "out of stack space");
        lua_getglobal(L, "io");
        lua_getfield(L, -1, "open_ro");
        lua_setfield(L, -2, "open");
        (void) hide_lua_value(L, "io", "tmpfile");
        (void) hide_lua_value(L, "io", "output");
        (void) hide_lua_value(L, "lfs", "chdir");
        (void) hide_lua_value(L, "lfs", "lock");
        (void) hide_lua_value(L, "lfs", "touch");
        (void) hide_lua_value(L, "lfs", "rmdir");
        (void) hide_lua_value(L, "lfs", "mkdir");
    }
    /* fprintf(stdout, "\nLuajitTeX default hash function type:%s\n", */
    /* 		                                jithash_hashname); */
    Luas = L;
}

@ @c
int hide_lua_table(lua_State * L, const char *name)
{
    int r = 0;
    lua_getglobal(L, name);
    if (lua_istable(L, -1)) {
        r = luaL_ref(L, LUA_REGISTRYINDEX);
        lua_pushnil(L);
        lua_setglobal(L, name);
    }
    return r;
}

@ @c
void unhide_lua_table(lua_State * L, const char *name, int r)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, r);
    lua_setglobal(L, name);
    luaL_unref(L, LUA_REGISTRYINDEX, r);
}

@ @c
int hide_lua_value(lua_State * L, const char *name, const char *item)
{
    int r = 0;
    lua_getglobal(L, name);
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, item);
        r = luaL_ref(L, LUA_REGISTRYINDEX);
        lua_pushnil(L);
        lua_setfield(L, -2, item);
    }
    return r;
}

@ @c
void unhide_lua_value(lua_State * L, const char *name, const char *item, int r)
{
    lua_getglobal(L, name);
    if (lua_istable(L, -1)) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, r);
        lua_setfield(L, -2, item);
        luaL_unref(L, LUA_REGISTRYINDEX, r);
    }
}

@ @c
int lua_traceback(lua_State * L)
{
    lua_getglobal(L, "debug");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return 1;
    }
    lua_getfield(L, -1, "traceback");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        return 1;
    }
    lua_pushvalue(L, 1);        /* pass error message */
    lua_pushinteger(L, 2);      /* skip this function and traceback */
    lua_call(L, 2, 1);          /* call debug.traceback */
    return 1;
}

@ @c
static void luacall(int p, int nameptr, boolean is_string) /* hh-ls: optimized lua_id resolving */
{
    LoadS ls;
    int i;
    size_t ll = 0;
    char *lua_id;
    char *s = NULL;

    if (Luas == NULL) {
        luainterpreter();
    }
    lua_active++;
    if (is_string) {
        const char *ss = NULL;
        lua_rawgeti(Luas, LUA_REGISTRYINDEX, p);
        if (lua_isfunction(Luas,-1)) {
            int base = lua_gettop(Luas);        /* function index */
            lua_checkstack(Luas, 1);
            lua_pushcfunction(Luas, lua_traceback);     /* push traceback function */
            lua_insert(Luas, base);     /* put it under chunk  */
            i = lua_pcall(Luas, 0, 0, base);
            lua_remove(Luas, base);     /* remove traceback function */
            if (i != 0) {
                lua_gc(Luas, LUA_GCCOLLECT, 0);
                Luas = luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
            }
            lua_active--;
            return ;
        }
        ss = lua_tolstring(Luas, -1, &ll);
        s = xmalloc(ll+1);
        memcpy(s,ss,ll+1);
        lua_pop(Luas,1);
    } else {
        int l = 0;
        s = tokenlist_to_cstring(p, 1, &l);
        ll = (size_t)l;
    }
    ls.s = s;
    ls.size = ll;
    if (ls.size > 0) {
        if (nameptr > 0) {
            int l = 0; /* not used */
            lua_id = tokenlist_to_cstring(nameptr, 1, &l);
            i = lua_load(Luas, getS, &ls, lua_id);
	    xfree(lua_id);
        } else if (nameptr < 0) {
            lua_id = get_lua_name((nameptr + 65536));
            if (lua_id != NULL) {
                i = lua_load(Luas, getS, &ls, lua_id);
            } else {
                i = lua_load(Luas, getS, &ls, "=[\\latelua]");
            }
        } else {
            i = lua_load(Luas, getS, &ls, "=[\\latelua]");
        }
        if (i != 0) {
            Luas = luatex_error(Luas, (i == LUA_ERRSYNTAX ? 0 : 1));
        } else {
            int base = lua_gettop(Luas);        /* function index */
            lua_checkstack(Luas, 1);
            lua_pushcfunction(Luas, lua_traceback);     /* push traceback function */
            lua_insert(Luas, base);     /* put it under chunk  */
            i = lua_pcall(Luas, 0, 0, base);
            lua_remove(Luas, base);     /* remove traceback function */
            if (i != 0) {
                lua_gc(Luas, LUA_GCCOLLECT, 0);
                Luas = luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
            }
        }
        xfree(ls.s);
    }
    lua_active--;
}

@ @c
void late_lua(PDF pdf, halfword p)
{
    (void) pdf;
    if (late_lua_type(p)==normal) {
        expand_macros_in_tokenlist(p);      /* sets |def_ref| */
        luacall(def_ref, late_lua_name(p), false);
        flush_list(def_ref);
    } else {
        luacall(late_lua_data(p), late_lua_name(p), true);
    }
}

@ @c
void luatokencall(int p, int nameptr) /* hh-ls: optimized lua_id resolving */
{
    LoadS ls;
    int i, l;
    char *s = NULL;
    char *lua_id;
    assert(Luas);
    l = 0;
    lua_active++;
    s = tokenlist_to_cstring(p, 1, &l);
    ls.s = s;
    ls.size = (size_t) l;
    if (ls.size > 0) {
        if (nameptr > 0) {
            lua_id = tokenlist_to_cstring(nameptr, 1, &l);
            i = lua_load(Luas, getS, &ls, lua_id);
	    xfree(lua_id);
        } else if (nameptr < 0) {
            lua_id = get_lua_name((nameptr + 65536));
            if (lua_id != NULL) {
                i = lua_load(Luas, getS, &ls, lua_id);
            } else {
                i = lua_load(Luas, getS, &ls, "=[\\directlua]");
            }
        } else {
            i = lua_load(Luas, getS, &ls, "=[\\directlua]");
        }
        xfree(s);
        if (i != 0) {
            Luas = luatex_error(Luas, (i == LUA_ERRSYNTAX ? 0 : 1));
        } else {
            int base = lua_gettop(Luas);        /* function index */
            lua_checkstack(Luas, 1);
            lua_pushcfunction(Luas, lua_traceback);     /* push traceback function */
            lua_insert(Luas, base);     /* put it under chunk  */
            i = lua_pcall(Luas, 0, 0, base);
            lua_remove(Luas, base);     /* remove traceback function */
            if (i != 0) {
                lua_gc(Luas, LUA_GCCOLLECT, 0);
                Luas = luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
            }
        }
    }
    lua_active--;
}

@ @c
lua_State *luatex_error(lua_State * L, int is_fatal)
{

    const_lstring luaerr;
    char *err = NULL;
    if (lua_type(L, -1) == LUA_TSTRING) {
        luaerr.s = lua_tolstring(L, -1, &luaerr.l);
        /* free last one ? */
        err = (char *) xmalloc((unsigned) (luaerr.l + 1));
        snprintf(err, (luaerr.l + 1), "%s", luaerr.s);
        last_lua_error = err; /* hm, what if we have several .. not freed */
    }
    if (is_fatal > 0) {
        /* Normally a memory error from lua.
           The pool may overflow during the |maketexlstring()|, but we
           are crashing anyway so we may as well abort on the pool size */
        normal_error("lua",err);
        /* never reached */
        lua_close(L);
        return (lua_State *) NULL;
    } else {
        normal_warning("lua",err);
        return L;
    }
}

@ @c
void preset_environment(lua_State * L, const parm_struct * p, const char *s)
{
    int i;
    assert(L != NULL);
    /* double call with same s gives assert(0) */
    lua_pushstring(L, s);       /* s */
    lua_gettable(L, LUA_REGISTRYINDEX); /* t */
    assert(lua_isnil(L, -1));
    lua_pop(L, 1);              /* - */
    lua_pushstring(L, s);       /* s */
    lua_newtable(L);            /* t s */
    for (i = 1, ++p; p->name != NULL; i++, p++) {
        assert(i == p->idx);
        lua_pushstring(L, p->name);     /* k t s */
        lua_pushinteger(L, p->idx);     /* v k t s */
        lua_settable(L, -3);    /* t s */
    }
    lua_settable(L, LUA_REGISTRYINDEX); /* - */
}

@ @c
/*
    int luaL_typerror (void *LL, int narg, const char *tname)
    {
      lua_State *L = (lua_State *)LL;
      const char *msg = lua_pushfstring(L, "%s expected, got %s",
                                        tname, luaL_typename(L, narg));
      return luaL_argerror(L, narg, msg);
    }
*/

/*
    Compatibility layer for luatex lua5.2
*/

@ @c
LUALIB_API void *luaL_testudata (lua_State *L, int ud, const char *tname) {
    void *p = lua_touserdata(L, ud);
    if (p != NULL) {  /* value is a userdata? */
        if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
            luaL_getmetatable(L, tname);  /* get correct metatable */
        if (!lua_rawequal(L, -1, -2))  /* not the same? */
            p = NULL;  /* value is a userdata with wrong metatable */
        lua_pop(L, 2);  /* remove both metatables */
        return p;
        }
    }
    return NULL;  /* value is not a userdata with a metatable */
}

@ @c

/* It's not ok. See lua-users.org/wiki/CompatibilityWithLuaFive for another solution */

LUALIB_API void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
    /*luaL_checkversion(L);*/
    luaL_checkstack(L, nup, "too many upvalues");
    for (; l->name != NULL; l++) {  /* fill the table with given functions */
        int i;
        for (i = 0; i < nup; i++)  /* copy upvalues to the top */
            lua_pushvalue(L, -nup);
        lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
        lua_setfield(L, -(nup + 2), l->name);
    }
    lua_pop(L, nup);  /* remove upvalues */
}

@ @c
LUALIB_API char *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
    lua_State *L = B->L;
    if (sz > LUAL_BUFFERSIZE )
        luaL_error(L, "buffer too large");
    return luaL_prepbuffer(B) ;
}

@ @c
LUA_API int lua_compare (lua_State *L, int o1, int o2, int op) {
    /*StkId o1, o2;*/
    int i = 0;
    lua_lock(L);  /* may call tag method */
    /* o1 = index2addr(L, index1); */
    /* o2 = index2addr(L, index2); */
    /*if (isvalid(o1) && isvalid(o2)) {*/
    switch (op) {
        case LUA_OPEQ: i = lua_equal(L, o1, o2); break;
        case LUA_OPLT: i = lua_lessthan(L, o1, o2); break;
        case LUA_OPLE: i = (lua_lessthan(L, o1, o2) || lua_equal(L, o1, o2)) ; break;
        default: luaL_error(L, "invalid option");
    }
    /*}*/
    lua_unlock(L);
    return i;
}

