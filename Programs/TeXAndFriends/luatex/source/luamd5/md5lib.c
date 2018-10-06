/**
*  $Id: md5lib.c,v 1.2 2003/04/28 16:25:44 roberto Exp $
*  Cryptographic and Hash functions for Lua
*  @version  1.0
*  @author  Roberto Ierusalimschy
*/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <lua.h>
#include <lauxlib.h>

#include "luamd5.h"

/**
*  Hash function. Returns a hash for a given string.
*  @param message: arbitrary binary string.
*  @return  A 128-bit hash string.
*/

static int lmd5 (lua_State *L) {
    char buff[16];
    size_t l;
    const char *message = luaL_checklstring(L, 1, &l);
    md5(message, l, buff);
    lua_pushlstring(L, buff, 16L);
    return 1;
}

/**
*  X-Or. Does a bit-a-bit exclusive-or of two strings.
*  @param s1: arbitrary binary string.
*  @param s2: arbitrary binary string with same length as s1.
*  @return  a binary string with same length as s1 and s2,
*   where each bit is the exclusive-or of the corresponding bits in s1-s2.
*/

static int ex_or (lua_State *L) {
    size_t l1, l2;
    const char *s1 = luaL_checklstring(L, 1, &l1);
    const char *s2 = luaL_checklstring(L, 2, &l2);
    luaL_Buffer b;
    luaL_argcheck( L, l1 == l2, 2, "lengths must be equal" );
    luaL_buffinit(L, &b);
    while (l1--)
        luaL_addchar(&b, (*s1++)^(*s2++));
    luaL_pushresult(&b);
    return 1;
}

static void checkseed (lua_State *L) {
    if (lua_isnone(L, 3)) {  /* no seed? */
        time_t tm = time(NULL);  /* for `random' seed */
        lua_pushlstring(L, (char *)&tm, sizeof(tm));
    }
}

#define MAXKEY    256
#define BLOCKSIZE  16

static int initblock (lua_State *L, const char *seed, int lseed, char *block) {
    size_t lkey;
    const char *key = luaL_checklstring(L, 2, &lkey);
    if (lkey > MAXKEY)
        luaL_error(L, "key too long (> %d)", MAXKEY);
    memset(block, 0, BLOCKSIZE);
    memcpy(block, seed, lseed);
    memcpy(block+BLOCKSIZE, key, lkey);
    return (int)lkey+BLOCKSIZE;
}

static void codestream (lua_State *L, const char *msg, size_t lmsg,
                                      char *block, int lblock) {
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    while (lmsg > 0) {
        char code[BLOCKSIZE];
        int i;
        md5(block, lblock, code);
        for (i=0; i<BLOCKSIZE && lmsg > 0; i++, lmsg--)
            code[i] ^= *msg++;
        luaL_addlstring(&b, code, i);
        memcpy(block, code, i); /* update seed */
    }
    luaL_pushresult(&b);
}

static void decodestream (lua_State *L, const char *cypher, size_t lcypher,
                          char *block, int lblock) {
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    while (lcypher > 0) {
        char code[BLOCKSIZE];
        int i;
        md5(block, lblock, code);  /* update seed */
        for (i=0; i<BLOCKSIZE && lcypher > 0; i++, lcypher--)
            code[i] ^= *cypher++;
        luaL_addlstring(&b, code, i);
        memcpy(block, cypher-i, i);
    }
    luaL_pushresult(&b);
}

/**
*  Encrypts a string. Uses the hash function md5 in CFB (Cipher-feedback
*  mode).
*  @param message: arbitrary binary string to be encrypted.
*  @param key: arbitrary binary string to be used as a key.
*  @param [seed]: optional arbitrary binary string to be used as a seed.
*  if no seed is provided, the function uses the result of
*  <code>time()</code> as a seed.
*  @return  The cyphertext (as a binary string).
*/

static int crypt (lua_State *L) {
    size_t lmsg;
    const char *msg = luaL_checklstring(L, 1, &lmsg);
    size_t lseed;
    const char *seed;
    int lblock;
    char block[BLOCKSIZE+MAXKEY];
    checkseed(L);
    seed = luaL_checklstring(L, 3, &lseed);
    if (lseed > BLOCKSIZE)
        luaL_error(L, "seed too long (> %d)", BLOCKSIZE);
      /* put seed and seed length at the beginning of result */
    block[0] = (char)lseed;
    memcpy(block+1, seed, lseed);
    lua_pushlstring(L, block, lseed+1);  /* to concat with result */
    lblock = initblock(L, seed, lseed, block);
    codestream(L, msg, lmsg, block, lblock);
    lua_concat(L, 2);
    return 1;
}


/**
*  Decrypts a string. For any message, key, and seed, we have that
*  <code>decrypt(crypt(msg, key, seed), key) == msg</code>.
*  @param cyphertext: message to be decrypted (this must be the result of
   a previous call to <code>crypt</code>.
*  @param key: arbitrary binary string to be used as a key.
*  @return  The plaintext.
*/
static int decrypt (lua_State *L) {
    size_t lcyphertext;
    const char *cyphertext = luaL_checklstring(L, 1, &lcyphertext);
    size_t lseed = cyphertext[0];
    const char *seed = cyphertext+1;
    int lblock;
    char block[BLOCKSIZE+MAXKEY];
    luaL_argcheck(L, lcyphertext >= lseed+1 && lseed <= BLOCKSIZE, 1,
        "invalid cyphered string");
    cyphertext += lseed+1;
    lcyphertext -= lseed+1;
    lblock = initblock(L, seed, lseed, block);
    decodestream(L, cyphertext, lcyphertext, block, lblock);
    return 1;
}

/* not now .. doesn't compile anyway

#include "../luapplib/util/utilmd5.h"

static int pdfelib_md_5(lua_State * L)
{
    if (lua_type(L,1) == LUA_TSTRING) {
        uint8_t result[16];
        size_t size = 0;
        const char *data = lua_tolstring(L,1,&size);
        md5(data,size,result);
        lua_pushlstring(L,(const char *)result,16);
        return 1;
    }
    return 0;
}

*/

static struct luaL_Reg md5lib[] = {
    { "sum",     lmd5},
    { "exor",    ex_or},
    { "crypt",   crypt},
    { "decrypt", decrypt},
    { NULL,      NULL}
};

int luaopen_md5(lua_State *L) {
    luaL_openlib(L, "md5",  md5lib,  0);
    return 1;
}

/* We could use a different file but this is as easy. */

#include "../luapplib/util/utilsha.h"

static int sha2_256(lua_State * L)
{
    if (lua_type(L,1) == LUA_TSTRING) {
        uint8_t result[SHA256_DIGEST_LENGTH];
        size_t size = 0;
        const char *data = lua_tolstring(L,1,&size);
        sha256(data,size,result);
        lua_pushlstring(L,(const char *)result,SHA256_DIGEST_LENGTH);
        return 1;
    }
    return 0;
}

static int sha2_384(lua_State * L)
{
    if (lua_type(L,1) == LUA_TSTRING) {
        size_t size = 0;
        uint8_t result[SHA384_DIGEST_LENGTH];
        const char *data = lua_tolstring(L,1,&size);
        sha384(data,size,result);
        lua_pushlstring(L,(const char *)result,SHA384_DIGEST_LENGTH);
        return 1;
    }
    return 0;
}

static int sha2_512(lua_State * L)
{
    if (lua_type(L,1) == LUA_TSTRING) {
        uint8_t result[SHA512_DIGEST_LENGTH];
        size_t size = 0;
        const char *data = lua_tolstring(L,1,&size);
        sha512(data,size,result);
        lua_pushlstring(L,(const char *)result,SHA512_DIGEST_LENGTH);
        return 1;
    }
    return 0;
}

static struct luaL_Reg sha2lib[] = {
    { "digest256", sha2_256 },
    { "digest384", sha2_384 },
    { "digest512", sha2_512 },
    { NULL,        NULL}
};

int luaopen_sha2(lua_State *L) {
    luaL_openlib(L, "sha2", sha2lib, 0);
    return 1;
}
