/* vim: ts=4 sw=4 sts=4 et tw=78
 * Portions copyright (c) 2015-present, Facebook, Inc. All rights reserved.
 * Portions copyright (c) 2011 James R. McKaskill.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
#include "ffi.h"

#define LOCAL_ALIGN_MASK 15
#define LOCAL_ALIGNED_PTR(p) (void*) ( ((uintptr_t)p&LOCAL_ALIGN_MASK)?(((uintptr_t)p & ~LOCAL_ALIGN_MASK) + 0x10 ):(uintptr_t)p )

static int to_define_key;

static void update_on_definition(lua_State* L, int ct_usr, int ct_idx)
{
    ct_usr = lua_absindex(L, ct_usr);
    ct_idx = lua_absindex(L, ct_idx);

    lua_pushlightuserdata(L, &to_define_key);
    lua_rawget(L, ct_usr);

    if (lua_isnil(L, -1)) {
        lua_pop(L, 1); /* pop the nil */

        /* {} */
        lua_newtable(L);

        /* {__mode='k'} */
        lua_newtable(L);
        lua_pushliteral(L, "k");
        lua_setfield(L, -2, "__mode");

        /* setmetatable({}, {__mode='k'}) */
        lua_setmetatable(L, -2);

        /* usr[TO_UPDATE_KEY] = setmetatable({}, {__mode='k'}) */
        lua_pushlightuserdata(L, &to_define_key);
        lua_pushvalue(L, -2);
        lua_rawset(L, ct_usr);

        /* leave the table on the stack */
    }

    /* to_update[ctype or cdata] = true */
    lua_pushvalue(L, ct_idx);
    lua_pushboolean(L, 1);
    lua_rawset(L, -3);

    /* pop the to_update table */
    lua_pop(L, 1);
}

void set_defined(lua_State* L, int ct_usr, struct ctype* ct)
{
    ct_usr = lua_absindex(L, ct_usr);

    ct->is_defined = 1;

    /* update ctypes and cdatas that were created before the definition came in */
    lua_pushlightuserdata(L, &to_define_key);
    lua_rawget(L, ct_usr);

    if (!lua_isnil(L, -1)) {
        lua_pushnil(L);

        while (lua_next(L, -2)) {
            struct ctype* upd = (struct ctype*) lua_touserdata(L, -2);
            upd->base_size = ct->base_size;
            upd->align_mask = ct->align_mask;
            upd->is_defined = 1;
            upd->is_variable_struct = ct->is_variable_struct;
            upd->variable_increment = ct->variable_increment;
            assert(!upd->variable_size_known);
            lua_pop(L, 1);
        }

        lua_pop(L, 1);
        /* usr[TO_UPDATE_KEY] = nil */
        lua_pushlightuserdata(L, &to_define_key);
        lua_pushnil(L);
        lua_rawset(L, ct_usr);
    } else {
        lua_pop(L, 1);
    }
}

struct ctype* push_ctype(lua_State* L, int ct_usr, const struct ctype* ct)
{
    struct ctype* ret;
    ct_usr = lua_absindex(L, ct_usr);

    ret = (struct ctype*) lua_newuserdata(L, sizeof(struct ctype));
	/*
	{
		void * p = NULL;
		p = lua_newuserdata(L, 16 + sizeof(struct ctype));
		ret = (struct ctype*)LOCAL_ALIGNED_PTR(p);
	}
	*/
	//*ret = *ct; 03-May-2022 trial fix
	memcpy(ret, ct, sizeof(struct ctype));

    push_upval(L, &ctype_mt_key);
    lua_setmetatable(L, -2);

#if LUA_VERSION_NUM == 501
    if (!ct_usr || lua_isnil(L, ct_usr)) {
        push_upval(L, &niluv_key);
        lua_setfenv(L, -2);
    }
#endif

    if (ct_usr && !lua_isnil(L, ct_usr)) {
        lua_pushvalue(L, ct_usr);
        lua_setuservalue(L, -2);
    }

    if (!ct->is_defined && ct_usr && !lua_isnil(L, ct_usr)) {
        update_on_definition(L, ct_usr, -1);
    }

    return ret;
}

size_t ctype_size(lua_State* L, const struct ctype* ct)
{
    if (ct->pointers - ct->is_array) {
        return sizeof(void*) * (ct->is_array ? ct->array_size : 1);

    } else if (!ct->is_defined || ct->type == VOID_TYPE) {
        return luaL_error(L, "can't calculate size of an undefined type");

    } else if (ct->variable_size_known) {
        assert(ct->is_variable_struct && !ct->is_array);
        return ct->base_size + ct->variable_increment;

    } else if (ct->is_variable_array || ct->is_variable_struct) {
        return luaL_error(L, "internal error: calc size of variable type with unknown size");

    } else {
        return ct->base_size * (ct->is_array ? ct->array_size : 1);
    }
}

void* push_cdata(lua_State* L, int ct_usr, const struct ctype* ct)
{
    struct cdata* cd;
    size_t sz = ct->is_reference ? sizeof(void*) : ctype_size(L, ct);
    ct_usr = lua_absindex(L, ct_usr);

    /* This is to stop valgrind from complaining. Bitfields are accessed in 8
     * byte chunks so that the code doesn't have to deal with different access
     * patterns, but this means that occasionally it will read past the end of
     * the struct. As its not setting the bits past the end (only reading and
     * then writing the bits back) and the read is aligned its a non-issue,
     * but valgrind complains nonetheless.
     */
    if (ct->has_bitfield) {
        sz = ALIGN_UP(sz, 7);
    }

	{
		void * p = NULL;
		//p = lua_newuserdata(L, 16 + sizeof(struct cdata) + sz);
		p = lua_newuserdata(L, sizeof(struct cdata) + sz);
		//cd = (struct cdata*)LOCAL_ALIGNED_PTR(p);
		cd = (struct cdata*)p;
	}
    memcpy((struct ctype*) &(cd->type), ct, sizeof(struct ctype));
    memset((struct ctype*)cd+1, 0, sz);

#if 0
    /* TODO: handle cases where lua_newuserdata returns a pointer that is not
     * aligned */
    assert((uintptr_t) (cd + 1) % 8 == 0);
#endif

#if LUA_VERSION_NUM == 501
    if (!ct_usr || lua_isnil(L, ct_usr)) {
        push_upval(L, &niluv_key);
        lua_setfenv(L, -2);
    }
#endif

    if (ct_usr && !lua_isnil(L, ct_usr)) {
        lua_pushvalue(L, ct_usr);
        lua_setuservalue(L, -2);
    }

    push_upval(L, &cdata_mt_key);
    lua_setmetatable(L, -2);

    if (!ct->is_defined && ct_usr && !lua_isnil(L, ct_usr)) {
        update_on_definition(L, ct_usr, -1);
    }

    return cd+1;
}

void push_callback(lua_State* L, cfunction luafunc, cfunction cfunc)
{
    cfunction* pf = (cfunction*) lua_newuserdata(L, 2 * sizeof(cfunction));
    pf[0] = luafunc;
    pf[1] = cfunc;

    push_upval(L, &callback_mt_key);
    lua_setmetatable(L, -2);
}

/* returns the value as a ctype, pushes the user value onto the stack */
void check_ctype(lua_State* L, int idx, struct ctype* ct)
{
    if (lua_isstring(L, idx)) {
        struct parser P;
        P.line = 1;
        P.prev = P.next = lua_tostring(L, idx);
        P.align_mask = DEFAULT_ALIGN_MASK;
        parse_type(L, &P, ct);
        parse_argument(L, &P, -1, ct, NULL, NULL);
        lua_remove(L, -2); /* remove the user value from parse_type */

    } else if (lua_getmetatable(L, idx)) {
        if (!equals_upval(L, -1, &ctype_mt_key)
                && !equals_upval(L, -1, &cdata_mt_key)) {
            goto err;
        }

        lua_pop(L, 1); /* pop the metatable */
        //*ct = *(struct ctype*) lua_touserdata(L, idx);
		memcpy(ct, lua_touserdata(L, idx), sizeof(struct ctype));
        lua_getuservalue(L, idx);

    } else {
        goto err;
    }

    return;

err:
    luaL_error(L, "expected cdata, ctype or string for arg #%d", idx);
}

/* to_cdata returns the struct cdata* and pushes the user value onto the
 * stack. If the index is not a ctype then ct is set to the zero value such
 * that ct->type is INVALID_TYPE, a nil is pushed, and NULL is returned. */
void* to_cdata(lua_State* L, int idx, struct ctype* ct)
{
    struct cdata* cd;

    memset(ct, 0, sizeof(struct ctype));
    if (!lua_isuserdata(L, idx) || !lua_getmetatable(L, idx)) {
        lua_pushnil(L);
        return NULL;
    }

    if (!equals_upval(L, -1, &cdata_mt_key)) {
        lua_pop(L, 1); /* mt */
        lua_pushnil(L);
        return NULL;
    }

    lua_pop(L, 1); /* mt */

	/*
	*/
    cd = (struct cdata*) lua_touserdata(L, idx);
    memcpy(ct, &(cd->type), sizeof(struct ctype));
	/*
	{
		void * p = NULL;
		p = lua_touserdata(L, idx);
		// *(struct ctype*) &cd->type = *ct;
		cd = (struct cdata*)LOCAL_ALIGNED_PTR(p);
		printf("%s:%d p=[%p] cd = [%p]\n", __FILE__, __LINE__, p, cd);
	}
    memcpy(ct, &(cd->type), sizeof(struct ctype));
	*/

    lua_getuservalue(L, idx);

    if (ct->is_reference) {
        return *(void**) (cd+1);

    } else if (ct->pointers && !ct->is_array) {
        return *(void**) (cd+1);

    } else {
        return cd + 1;
    }
}

/* check_cdata returns the struct cdata* and pushes the user value onto the
 * stack. Also dereferences references. */
void* check_cdata(lua_State* L, int idx, struct ctype* ct)
{
    void* p = to_cdata(L, idx, ct);
    if (ct->type == INVALID_TYPE) {
        luaL_error(L, "expected cdata for arg #%d", idx);
    }
    return p;
}

