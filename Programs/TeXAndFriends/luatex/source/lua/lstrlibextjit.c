/* lstrlibext.c for luajitex

   Copyright 2013 Luigi Scarso

   Code from lstrlibext.c for LuaTeX 
   Original version copyright 2012 Taco Hoekwater <taco@luatex.org>

   This file is part of LuajitTeX.

   LuajitTeX is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   LuajitTeX is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU General Public License along
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. 
*/

#include "ptexlib.h"
#include <stdio.h>

#define LUA_CORE

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"




/*#include "lua51/lj_obj.h"*/
/*#include "lua51/lj_gc.h"*/
/*#include "lua51/lj_err.h"*/
/*#include "lua51/lj_str.h"*/
/*#include "lua51/lj_tab.h"*/
/*#include "lua51/lj_meta.h"*/
/*#include "lua51/lj_state.h"*/
/*#include "lua51/lj_ff.h" */
/*#include "lua51/lj_bcdump.h"*/
/*#include "lua51/lj_char.h"*/
/*#include "lua51/lj_lib.h"*/



static int bytepairs_aux (lua_State *L) {
  size_t ls;
  unsigned char i;
  const char *s = lua_tolstring(L, lua_upvalueindex(1), &ls);
  int ind       = lua_tointeger(L, lua_upvalueindex(2));
  if (ind<(int)ls) {
	if (ind+1<(int)ls) {
	  lua_pushinteger(L, (ind+2));  /* iterator */
	} else {
	  lua_pushinteger(L, (ind+1));  /* iterator */
	}
	lua_replace(L, lua_upvalueindex(2));
	i = (unsigned char)*(s+ind);
	lua_pushinteger(L, i);     /* byte one */
	if (ind+1<(int)ls) {
	  i = (unsigned char)*(s+ind+1);
	  lua_pushinteger(L, i);     /* byte two */
	} else {
	  lua_pushnil(L);     /* odd string length */
	}
	return 2;
  }
  return 0;  /* string ended */
}


static int str_bytepairs (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_settop(L, 1);
  lua_pushinteger(L, 0);
  lua_pushcclosure(L, bytepairs_aux, 2);
  return 1;
}


static int bytes_aux (lua_State *L) {
  size_t ls;
  unsigned char i;
  const char *s = lua_tolstring(L, lua_upvalueindex(1), &ls);
  int ind       = lua_tointeger(L, lua_upvalueindex(2));
  if (ind<(int)ls) {
    lua_pushinteger(L, (ind+1));  /* iterator */
	lua_replace(L, lua_upvalueindex(2));
	i = (unsigned char)*(s+ind);
	lua_pushinteger(L, i);     /* byte */
	return 1;
  }
  return 0;  /* string ended */
}


static int str_bytes (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_settop(L, 1);
  lua_pushinteger(L, 0);
  lua_pushcclosure(L, bytes_aux, 2);
  return 1;
}


static int utf_failed(lua_State *L, int new_ind) {
  static char fffd [3] = {0xEF,0xBF,0xBD};
  lua_pushinteger(L, new_ind);  /* iterator */
  lua_replace(L, lua_upvalueindex(2));
  lua_pushlstring(L, fffd, 3);
  return 1;
}


static int utfcharacters_aux (lua_State *L) {
  static const unsigned char mask[4] = {0x80,0xE0,0xF0,0xF8};
  static const unsigned char mequ[4] = {0x00,0xC0,0xE0,0xF0};
  size_t ls;
  unsigned char c;
  int j;
  const char *s = lua_tolstring(L, lua_upvalueindex(1), &ls);
  int       ind = lua_tointeger(L, lua_upvalueindex(2));
  if (ind>=(int)ls) return 0; /* end of string */
  c = (unsigned) s[ind];
  for (j=0;j<4;j++) {
    if ((c&mask[j])==mequ[j]) {
      int k;
      if (ind+1+j>(int)ls) return utf_failed(L,ls); /* will not fit */
      for (k=1; k<=j; k++) {
        c = (unsigned) s[ind+k];
        if ((c&0xC0)!=0x80) return utf_failed(L,ind+k); /* bad follow */
      }
      lua_pushinteger(L, ind+1+j);  /* iterator */
      lua_replace(L, lua_upvalueindex(2));
      lua_pushlstring(L, s+ind, 1+j);
      return 1;
    }
  }
  return utf_failed(L,ind+1); /* we found a follow byte! */
} 


static int str_utfcharacters (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_settop(L, 1);
  lua_pushinteger(L, 0);
  lua_pushcclosure(L, utfcharacters_aux, 2);
  return 1;
}



static int utfvalues_aux (lua_State *L) {
  size_t ls;
  unsigned char i = 0;
  unsigned char j = 0;
  unsigned char k = 0;
  unsigned char l = 0;
  unsigned int  v = 0xFFFD;
  int numbytes = 1;
  const char *s = lua_tolstring(L, lua_upvalueindex(1), &ls);
  int ind       = lua_tointeger(L, lua_upvalueindex(2));

  if (ind<(int)ls) {
	i = *(s+ind);
	if (i<0x80) {
	  v = i;
	} else if (i>=0xF0) {
	  if ((ind+3)<(int)ls && ((unsigned)*(s+ind+1))>=0x80 
		  && ((unsigned)*(s+ind+2))>=0x80 && ((unsigned)*(s+ind+3))>=0x80) {
		numbytes  = 4;
		j = ((unsigned)*(s+ind+1))-128;
		k = ((unsigned)*(s+ind+2))-128;
		l = ((unsigned)*(s+ind+3))-128;
		v = (((((i-0xF0)*64) + j)*64) + k)*64 + l;
	  }
	} else if (i>=0xE0) {
	  if ((ind+2)<(int)ls && ((unsigned)*(s+ind+1))>=0x80 && ((unsigned)*(s+ind+2))>=0x80) {
		numbytes  = 3;
		j = ((unsigned)*(s+ind+1))-128;
		k = ((unsigned)*(s+ind+2))-128;
		v = (((i-0xE0)*64) + j)*64 + k;
	  }

	} else if (i>=0xC0) {
	  if ((ind+1)<(int)ls && ((unsigned)*(s+ind+1))>=0x80) {
		numbytes  = 2;
		j = ((unsigned)*(s+ind+1))-128;
		v = ((i-0xC0)*64) + j;
	  }
	}
	lua_pushinteger(L, (ind+numbytes));  /* iterator */
	lua_replace(L, lua_upvalueindex(2));
	lua_pushinteger(L, v);
	return 1;
  }
  return 0;  /* string ended */
}


static int str_utfvalues (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_settop(L, 1);
  lua_pushinteger(L, 0);
  lua_pushcclosure(L, utfvalues_aux, 2);
  return 1;
}


static int characterpairs_aux (lua_State *L) {
  size_t ls;
  char b[2];
  const char *s = lua_tolstring(L, lua_upvalueindex(1), &ls);
  int ind       = lua_tointeger(L, lua_upvalueindex(2));
  if (ind<(int)ls) {
	if (ind+1<(int)ls) {
	  lua_pushinteger(L, (ind+2));  /* iterator */
	} else {
	  lua_pushinteger(L, (ind+1));  /* iterator */
	}
	lua_replace(L, lua_upvalueindex(2));
	b[0] = *(s+ind); b[1] = 0;
	lua_pushlstring(L, b, 1);
	if (ind+1<(int)ls) {
	  b[0] = *(s+ind+1); 
	  lua_pushlstring(L, b, 1);
	} else {
	  lua_pushlstring(L, b+1, 0);
	}
	return 2;
  }
  return 0;  /* string ended */
}


static int str_characterpairs (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_settop(L, 1);
  lua_pushinteger(L, 0);
  lua_pushcclosure(L, characterpairs_aux, 2);
  return 1;
}


static int characters_aux (lua_State *L) {
  size_t ls;
  char b[2];
  const char *s = lua_tolstring(L, lua_upvalueindex(1), &ls);
  int ind  = lua_tointeger(L, lua_upvalueindex(2));
  if (ind<(int)ls) {
    lua_pushinteger(L, (ind+1));  /* iterator */
	lua_replace(L, lua_upvalueindex(2));
	b[0] = *(s+ind); b[1] = 0;
	lua_pushlstring(L, b, 1);
	return 1;
  }
  return 0;  /* string ended */
}


static int str_characters (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_settop(L, 1);
  lua_pushinteger(L, 0);
  lua_pushcclosure(L, characters_aux, 2);
  return 1;
}

static int str_split (lua_State *L) {
  size_t l;
  size_t i;
  int n;
  char *q, *p, *orig;
  int mult = 0;
  const char *s = luaL_checklstring(L, 1, &l);
  const char *joiner = luaL_optstring(L, 2, " +");
  lua_newtable(L);
  if (l == 0) {
	lua_pushvalue(L,1);
	lua_rawseti(L,-2,1);
	return 1;
  }
  orig = p = malloc(l+1);
  if (p==NULL) {
	fprintf(stderr, "fatal: memory exhausted (malloc of %u bytes).\n",(int)(l+1));
	exit(EXIT_FAILURE);
  }
  strcpy(p,s);
  n = 1;
  q = p;

  if (*joiner == 0) {
	for (i=0;i<l;i++) {
	  lua_pushlstring(L,q,1); q++;
	  lua_rawseti(L,-2,n); n++;
	}
	free(orig);
	return 1;
  }
  if (*(joiner+1) == '+') {
	mult = 1;
	while(*p==*joiner) {
	  p++;
	  l--;
	}
	q = p;
  }
  for (i=0;i<l;i++) {
	if (*(p+i)==*joiner) {
	  *(p+i) = 0;
	  lua_pushlstring(L,q,((p+i)-q));
	  lua_rawseti(L,-2,n); n++;
	  if (mult) {
		while(*(p+i+1)==*joiner) {
		  i++;
		}
	  }
	  q = p+i+1;
	}
  }
  if (mult && q==(p+l)) {
	free(orig);
	return 1;
  }
  if(q<=(p+l)) {
	lua_pushlstring(L,q,strlen(q));
	lua_rawseti(L,-2,n);
  }
  free(orig);
  return 1;
} 


static const luaL_Reg strlibext[] = {
  {"utfvalues", str_utfvalues}, 
  {"utfcharacters", str_utfcharacters},
  {"characters", str_characters},
  {"characterpairs", str_characterpairs},
  {"bytes", str_bytes},
  {"bytepairs", str_bytepairs},
  {"explode", str_split},
  /* {"dump", str_dump} already in luajit */
  {NULL, NULL}
};




/* ------------------------------------------------------------------------ */

/* lj_libdef.h is generated by buildvm, it's not available on source */
/* #include "lua51/lj_libdef.h" */

/* LUALIB_API int luaopen_string(lua_State *L) */
/* { */
/*   GCtab *mt; */
/*   global_State *g; */
/*   LJ_LIB_REG(L, LUA_STRLIBNAME, string); */
/*   luaL_register(L, LUA_STRLIBNAME, strlib); */
/*   //LJ_LIB_REG(L, LUA_STRLIBNAME, strlib); */
/* #if defined(LUA_COMPAT_GFIND) && !LJ_52 */
/*   lua_getfield(L, -1, "gmatch"); */
/*   lua_setfield(L, -2, "gfind"); */
/* #endif */
/*   mt = lj_tab_new(L, 0, 1); */
/*   /\* NOBARRIER: basemt is a GC root. *\/ */
/*   g = G(L); */
/*   setgcref(basemt_it(g, LJ_TSTR), obj2gco(mt)); */
/*   settabV(L, lj_tab_setstr(L, mt, mmname_str(g, MM_index)), tabV(L->top-1)); */
/*   mt->nomm = (uint8_t)(~(1u<<MM_index)); */
/*   return 1; */
/* } */

/* void open_strlibext(lua_State *L) */
/* { */
/*   int v; */
/*   v = luaopen_string(L); */
/* } */


void open_strlibext(lua_State * L)
{
    const luaL_Reg *lib;
    lua_getglobal(L, "string");
    for (lib=strlibext;lib->name;lib++) {
        lua_pushcfunction(L, lib->func);
        lua_setfield(L, -2, lib->name);
    }
    lua_pop(L,1);
}

