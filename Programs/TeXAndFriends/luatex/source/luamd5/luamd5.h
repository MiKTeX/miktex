/**
*  $Id: md5.h,v 1.2 2003/04/28 16:25:44 roberto Exp $
*  Cryptographic module for Lua.
*  @author  Roberto Ierusalimschy
*/


#ifndef luamd5_h
#define luamd5_h

#include <lua.h>


#define HASHSIZE       16

void md5 (const char *message, long len, char *output);
int luaopen_md5 (lua_State *L);


#endif
