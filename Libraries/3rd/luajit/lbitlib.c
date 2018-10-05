/*
** $Id: lbitlib.c,v 1.16 2011/06/20 16:35:23 roberto Exp $
** Standard library for bitwise operations
** See Copyright Notice in lua.h
*/

#define lbitlib_c
#define LUA_LIB

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#include "lj_obj.h"
#include "lj_state.h"

/* http://lua-users.org/lists/lua-l/2011-01/msg01039.html */
typedef unsigned int lua_Unsigned;
#define LUA_UNSIGNED unsigned LUA_INT32


#if !defined(lua_unsigned2number)
/* on several machines, coercion from unsigned to double is slow,
   so it may be worth to avoid */
#define lua_unsigned2number(u)  \
    (((u) <= (lua_Unsigned)INT_MAX) ? (lua_Number)(int)(u) : (lua_Number)( unsigned int )(u))
#endif

#if !defined(lua_lock)
#define lua_lock(L)     ((void) 0)
#define lua_unlock(L)   ((void) 0)
#endif


LUA_API void lua_pushunsigned (lua_State *L, lua_Unsigned u) {
  lua_Number n=0;
  lua_lock(L);
/*  printf("u=%d,%u,%x\n",u,u,u);    
  printf("INT_MAX=%d %u %x, (lua_Unsigned)INT_MAX=%d %u %x (unsigned int)INT_MAX=%d %u %x\n",
			    INT_MAX,INT_MAX,INT_MAX,
			    (lua_Unsigned)INT_MAX,(lua_Unsigned)INT_MAX,(lua_Unsigned)INT_MAX,
			    (unsigned int)INT_MAX,(unsigned int)INT_MAX,(unsigned int)INT_MAX);
  printf("((u) <= (lua_Unsigned)INT_MAX) :%d\n",(((unsigned int)u) <= (lua_Unsigned)INT_MAX));
  printf("(lua_Number)(int)(u) =%d,%u,%x\n",(lua_Number)(int)(u),(lua_Number)(int)(u),(lua_Number)(int)(u));
  printf("(lua_Number)(u) =%f,%u,%x\n",(lua_Number)(u),(lua_Number)(u),(lua_Number)(u));
  printf("u=%d,%u,%x\n",u,u,u);    
*/
  /*printf("n=%d,%u,%x u=%d,%u,%x\n",n,n,n,u,u,u);    */
  n = lua_unsigned2number(u);
/*  printf("n=%f \n",n);    
  printf("u=%d %u %x n=%d %u %x\n",u,u,u,n,n,n);    
  printf("n=%d %u %x\n",n,n,n);    
  printf("u=%d,%u,%x\n",u,u,u);    
*/
/*  setintptrV(L->top, n);
  incr_top(L);
  setnumV(L->top, n);
*/
  setnumV(L->top, n);
  if (LJ_UNLIKELY(tvisnan(L->top)))
    setnanV(L->top);  /* Canonicalize injected NaNs. */
  incr_top(L);
  /*setnvalue(L->top, n);
  api_incr_top(L);
  */
  lua_unlock(L);
}






#define luaL_checkunsigned (lua_Unsigned) luaL_checkinteger
/*#define lua_pushunsigned lua_pushinteger*/



/* number of bits to consider in a number */
#if !defined(LUA_NBITS)
#define LUA_NBITS       32
#endif


#define ALLONES         (~(((~(lua_Unsigned)0) << (LUA_NBITS - 1)) << 1))

/* macro to trim extra bits */
#define trim(x)         ((x) & ALLONES)


/* builds a number with 'n' ones (1 <= n <= LUA_NBITS) */
#define mask(n)         (~((ALLONES << 1) << ((n) - 1)))


typedef lua_Unsigned b_uint;



static b_uint andaux (lua_State *L) {
  int i, n = lua_gettop(L);
  b_uint r = ~(b_uint)0;
/*  printf("1 r=%d,%u,%x\n",r,r,r);*/
  for (i = 1; i <= n; i++)
    r &= luaL_checkunsigned(L, i);
/*  printf("2 r=%d,%u,%x\n",r,r,r);
  printf("3 r=%d,%u,%x\n",trim(r),trim(r),trim(r));
*/
  return trim(r);
}


static int b_and (lua_State *L) {
  b_uint r = andaux(L);
/*  printf("4 r=%d,%u,%x\n",r,r,r);*/
  lua_pushunsigned(L, r);
  return 1;
}


static int b_test (lua_State *L) {
  b_uint r = andaux(L);
  lua_pushboolean(L, r != 0);
  return 1;
}


static int b_or (lua_State *L) {
  int i, n = lua_gettop(L);
  b_uint r = 0;
  for (i = 1; i <= n; i++)
    r |= luaL_checkunsigned(L, i);
  lua_pushunsigned(L, trim(r));
  return 1;
}


static int b_xor (lua_State *L) {
  int i, n = lua_gettop(L);
  b_uint r = 0;
  for (i = 1; i <= n; i++)
    r ^= luaL_checkunsigned(L, i);
  lua_pushunsigned(L, trim(r));
  return 1;
}


static int b_not (lua_State *L) {
  b_uint r = ~luaL_checkunsigned(L, 1);
  lua_pushunsigned(L, trim(r));
  return 1;
}


static int b_shift (lua_State *L, b_uint r, int i) {
  if (i < 0) {  /* shift right? */
    i = -i;
    r = trim(r);
    if (i >= LUA_NBITS) r = 0;
    else r >>= i;
  }
  else {  /* shift left */
    if (i >= LUA_NBITS) r = 0;
    else r <<= i;
    r = trim(r);
  }
  lua_pushunsigned(L, r);
  return 1;
}


static int b_lshift (lua_State *L) {
  return b_shift(L, luaL_checkunsigned(L, 1), luaL_checkint(L, 2));
}


static int b_rshift (lua_State *L) {
  return b_shift(L, luaL_checkunsigned(L, 1), -luaL_checkint(L, 2));
}


static int b_arshift (lua_State *L) {
  b_uint r = luaL_checkunsigned(L, 1);
  int i = luaL_checkint(L, 2);
  if (i < 0 || !(r & ((b_uint)1 << (LUA_NBITS - 1))))
    return b_shift(L, r, -i);
  else {  /* arithmetic shift for 'negative' number */
    if (i >= LUA_NBITS) r = ALLONES;
    else
      r = trim((r >> i) | ~(~(b_uint)0 >> i));  /* add signal bit */
    lua_pushunsigned(L, r);
    return 1;
  }
}


static int b_rot (lua_State *L, int i) {
  b_uint r = luaL_checkunsigned(L, 1);
  i &= (LUA_NBITS - 1);  /* i = i % NBITS */
  r = trim(r);
  r = (r << i) | (r >> (LUA_NBITS - i));
  lua_pushunsigned(L, trim(r));
  return 1;
}


static int b_lrot (lua_State *L) {
  return b_rot(L, luaL_checkint(L, 2));
}


static int b_rrot (lua_State *L) {
  return b_rot(L, -luaL_checkint(L, 2));
}


/*
** get field and width arguments for field-manipulation functions,
** checking whether they are valid
*/
static int fieldargs (lua_State *L, int farg, int *width) {
  int f = luaL_checkint(L, farg);
  int w = luaL_optint(L, farg + 1, 1);
  luaL_argcheck(L, 0 <= f, farg, "field cannot be negative");
  luaL_argcheck(L, 0 < w, farg + 1, "width must be positive");
  if (f + w > LUA_NBITS)
    luaL_error(L, "trying to access non-existent bits");
  *width = w;
  return f;
}


static int b_extract (lua_State *L) {
  int w;
  b_uint r = luaL_checkunsigned(L, 1);
  int f = fieldargs(L, 2, &w);
  r = (r >> f) & mask(w);
  lua_pushunsigned(L, r);
  return 1;
}


static int b_replace (lua_State *L) {
  int w;
  b_uint r = luaL_checkunsigned(L, 1);
  b_uint v = luaL_checkunsigned(L, 2);
  int f = fieldargs(L, 3, &w);
  int m = mask(w);
  v &= m;  /* erase bits outside given width */
  r = (r & ~(m << f)) | (v << f);
  lua_pushunsigned(L, r);
  return 1;
}


static const luaL_Reg bitlib[] = {
  {"arshift", b_arshift},
  {"band", b_and},
  {"bnot", b_not},
  {"bor", b_or},
  {"bxor", b_xor},
  {"btest", b_test},
  {"extract", b_extract},
  {"lrotate", b_lrot},
  {"lshift", b_lshift},
  {"replace", b_replace},
  {"rrotate", b_rrot},
  {"rshift", b_rshift},
  {NULL, NULL}
};



/* http://lua-users.org/lists/lua-l/2011-01/msg01039.html */

/*LUAMOD_API int luaopen_bit32 (lua_State *L) {*/
/*  luaL_newlib(L, bitlib);*/
/*  return 1;*/
/*} */

int luaopen_bit32 (lua_State *L) {
  luaL_register(L, "bit32", bitlib);
  return 1;
} 



