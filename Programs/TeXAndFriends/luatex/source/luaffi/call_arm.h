/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.3.0, DynASM arm version 1.3.0
** DO NOT EDIT! The original file is in "call_arm.dasc".
*/

#if DASM_VERSION != 10300
#error "Version mismatch between DynASM and included encoding engine"
#endif

/* vim: ts=4 sw=4 sts=4 et tw=78
 * Portions copyright (c) 2015-present, Facebook, Inc. All rights reserved.
 * Portions copyright (c) 2011 James R. McKaskill.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
static const unsigned int build_actionlist[546] = {
0xe1a0c00d,
0xe92d000f,
0xe92d50f0,
0xe24c6010,
0xe59f5008,
0xe59f2008,
0xe59f1008,
0xea000000,
0x00050001,
0x00090000,
0x00090000,
0x00090000,
0x0006000b,
0xe1a00005,
0xeb000000,
0x00030000,
0x00000000,
0xe3a02000,
0x000b0000,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030000,
0x00000000,
0xe3a02000,
0x000b0000,
0xe3e01000,
0x000b0000,
0xe1a00005,
0xeb000000,
0x00030000,
0xe59f2000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030001,
0xe4962004,
0xe5802000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x00030002,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030003,
0xe59f2000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030001,
0xe4962004,
0xe4963004,
0xe5802000,
0xe5803004,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x00030002,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030003,
0xe59f2000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030001,
0xe4962004,
0xe5802000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x00030002,
0x00000000,
0xe4961004,
0xe1a00005,
0xeb000000,
0x00030004,
0x00000000,
0xe4961004,
0xe1a01c01,
0x00000000,
0xe1a01c21,
0x00000000,
0xe1a01c41,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030005,
0x00000000,
0xe4961004,
0xe1a01801,
0x00000000,
0xe1a01821,
0x00000000,
0xe1a01841,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030005,
0x00000000,
0xe4961004,
0xe1a00005,
0xeb000000,
0x00030005,
0x00000000,
0xe4961004,
0xe1a00005,
0xeb000000,
0x00030006,
0x00000000,
0xe8b60006,
0xe1a00005,
0xeb000000,
0x00030007,
0x00000000,
0xe3a03000,
0xe3a02000,
0x000b0000,
0xe3a01000,
0x000b0000,
0xe1a00005,
0xeb000000,
0x00030008,
0x00000000,
0xe3a02000,
0x000b0000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x00030000,
0xe59f3000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3a02000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x00030009,
0xe1a06000,
0xe3e01003,
0xe1a00005,
0xeb000000,
0x0003000a,
0xe1a00006,
0x00000000,
0xe3a02000,
0x000b0000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x00030000,
0xe59f3000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3e02000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x0003000b,
0xe1a06000,
0xe3e01003,
0xe1a00005,
0xeb000000,
0x0003000a,
0xe1a00006,
0x00000000,
0xe3e01001,
0xe1a00005,
0xeb000000,
0x0003000a,
0x00000000,
0xe3e01000,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000c,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000d,
0x00000000,
0xe3e01000,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000e,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000f,
0x00000000,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030010,
0x00000000,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030011,
0x00000000,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030012,
0x00000000,
0xe1a06000,
0xe3e01002,
0xe1a00005,
0xeb000000,
0x0003000a,
0xe1a00006,
0x00000000,
0xe1a06000,
0xe1a07001,
0xe3e01002,
0xe1a00005,
0xeb000000,
0x0003000a,
0xe1a00006,
0xe1a01007,
0x00000000,
0xe89da0f0,
0x00000000,
0xe1a0c00d,
0xe92d0001,
0xe92d58f0,
0xe24cb004,
0xe1a05000,
0xe1a00005,
0xeb000000,
0x00030013,
0xe1a04000,
0xe3540000,
0x000b0000,
0x00000000,
0xaa000000,
0x00050001,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe1a00005,
0xeb000000,
0x00030014,
0x0006000b,
0x00000000,
0x0a000000,
0x00050001,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe1a00005,
0xeb000000,
0x00030014,
0x0006000b,
0x00000000,
0xe04dd184,
0xe24dd010,
0xe1a0600d,
0x00000000,
0xe59f3004,
0xe59f2004,
0xea000000,
0x00050001,
0x00090000,
0x00090000,
0x0006000b,
0xe3a01000,
0x000b0000,
0xe1a00005,
0x00000000,
0xeb000000,
0x00030009,
0x00000000,
0xeb000000,
0x00030015,
0x00000000,
0xeb000000,
0x0003000b,
0x00000000,
0xe4860004,
0x00000000,
0xe3a01000,
0x000b0000,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000d,
0x00000000,
0xe20000ff,
0x00000000,
0xe1a00c00,
0xe1a00c40,
0x00000000,
0xe4860004,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000d,
0x00000000,
0xe1a00800,
0xe1a00820,
0x00000000,
0xe1a00800,
0xe1a00840,
0x00000000,
0xe4860004,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000c,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000d,
0x00000000,
0xe4860004,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000e,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003000f,
0x00000000,
0xe4860004,
0xe4861004,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030012,
0xe4860004,
0xe4861004,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030010,
0xe4860004,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030011,
0xe4860004,
0x00000000,
0xe1a03006,
0xe1a02004,
0xe3a01000,
0x000b0000,
0xe1a00005,
0xeb000000,
0x00030016,
0x00000000,
0xe59f0000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5900000,
0xeb000000,
0x00030017,
0x00000000,
0xe8bd000f,
0xeb000000,
0x00030018,
0x00000000,
0xe1a06000,
0xeb000000,
0x00030019,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe59f2004,
0xe59f1004,
0xea000000,
0x00050001,
0x00090000,
0x00090000,
0x0006000b,
0xe1a00005,
0xeb000000,
0x00030001,
0xe5806000,
0xe3a00001,
0xe91ba870,
0x00000000,
0xe1a06000,
0xe1a07001,
0xeb000000,
0x00030019,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe1a00005,
0xeb000000,
0x00030003,
0xe59f2000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030001,
0xe5806000,
0xe5807004,
0xe3a00001,
0xe91ba870,
0x00000000,
0xe1a06000,
0xeb000000,
0x00030019,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe1a00005,
0xeb000000,
0x00030003,
0xe59f2000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe3e01000,
0xe1a00005,
0xeb000000,
0x00030001,
0xe5806000,
0xe3a00001,
0xe91ba870,
0x00000000,
0xeb000000,
0x00030019,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe3a00000,
0xe91ba870,
0x00000000,
0xe1a06000,
0xeb000000,
0x00030019,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe1a01006,
0xe1a00005,
0xeb000000,
0x00030004,
0xe3a00001,
0xe91ba870,
0x00000000,
0xe1a06000,
0xeb000000,
0x00030019,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe1a01006,
0x00000000,
0xe1a00005,
0xeb000000,
0x0003001a,
0x00000000,
0xe1a00005,
0xeb000000,
0x00030005,
0x00000000,
0xe3a00001,
0xe91ba870,
0x00000000,
0xe1a06000,
0xeb000000,
0x00030019,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe1a01006,
0xe1a00005,
0xeb000000,
0x00030006,
0xe3a00001,
0xe91ba870,
0x00000000,
0xe1a06000,
0xe1a07001,
0xeb000000,
0x00030019,
0xe59f1000,
0xea000000,
0x00050005,
0x00090000,
0x0006000f,
0xe5810000,
0xe1a02007,
0xe1a01006,
0xe1a00005,
0xeb000000,
0x00030007,
0xe3a00001,
0xe91ba870,
0x00000000
};

static const char *const globnames[] = {
  (const char *)0
};
static const char *const extnames[] = {
  "lua_rawgeti",
  "push_cdata",
  "lua_remove",
  "lua_pushnil",
  "lua_pushboolean",
  "push_int",
  "push_float",
  "lua_pushnumber",
  "lua_callk",
  "to_typed_pointer",
  "lua_settop",
  "to_enum",
  "check_uint32",
  "check_int32",
  "check_uint64",
  "check_int64",
  "check_intptr",
  "check_float",
  "check_double",
  "lua_gettop",
  "luaL_error",
  "to_typed_function",
  "unpack_varargs_stack",
  "SetLastError",
  "FUNCTION",
  "GetLastError",
  "push_uint",
  (const char *)0
};

#define JUMP_SIZE 8
#define MIN_BRANCH ((INT32_MIN) >> 8)
#define MAX_BRANCH ((INT32_MAX) >> 8)
#define BRANCH_OFF 4

static void compile_extern_jump(struct jit* jit, lua_State* L, cfunction func, uint8_t* code)
{
    /* The jump code is the function pointer followed by a stub to call the
     * function pointer. The stub exists so we can jump to functions with an
     * offset greater than 32MB.
     *
     * Note we have to manually set this up since there are commands buffered
     * in the jit state.
     */
    *(cfunction*) code = func;
    /* ldr pc, [pc - 12] */
    *(uint32_t*) &code[4] = 0xE51FF00CU;
}




void compile_globals(struct jit* jit, lua_State* L)
{
    (void) jit;
}

cfunction compile_callback(lua_State* L, int fidx, int ct_usr, const struct ctype* ct)
{
    struct jit* Dst = get_jit(L);;
    int i, nargs, num_upvals, ref;
    const struct ctype* mt;

    int top = lua_gettop(L);

    ct_usr = lua_absindex(L, ct_usr);
    fidx = lua_absindex(L, fidx);
    nargs = (int) lua_rawlen(L, ct_usr);

    dasm_setup(Dst, build_actionlist);

    lua_newtable(L);
    lua_pushvalue(L, -1);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);
    num_upvals = 0;

    if (ct->has_var_arg) {
        luaL_error(L, "can't create callbacks with varargs");
    }

    /* prolog and get the upval table */
    dasm_put(Dst, 0, (uintptr_t)(L), (uintptr_t)(ref), (uintptr_t)(LUA_REGISTRYINDEX));

    /* get the lua function */
    lua_pushvalue(L, fidx);
    lua_rawseti(L, -2, ++num_upvals);
    dasm_put(Dst, 17, num_upvals);

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, i);
        mt = (const struct ctype*) lua_touserdata(L, -1);

        if (mt->pointers || mt->is_reference) {
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */

            dasm_put(Dst, 24, num_upvals-1, i, (uintptr_t)(mt));

        } else {
            switch (mt->type) {
            case INT64_TYPE:
                lua_rawseti(L, -2, ++num_upvals); /* mt */
                dasm_put(Dst, 47, (uintptr_t)(mt));
                break;

            case INTPTR_TYPE:
                lua_rawseti(L, -2, ++num_upvals); /* mt */
                dasm_put(Dst, 68, (uintptr_t)(mt));
                break;

            case BOOL_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 87);
                break;

            case INT8_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 92);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 95);
                } else {
                    dasm_put(Dst, 97);
                }
                dasm_put(Dst, 99);
                break;

            case INT16_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 103);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 106);
                } else {
                    dasm_put(Dst, 108);
                }
                dasm_put(Dst, 110);
                break;

            case ENUM_TYPE:
            case INT32_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 114);
                break;

            case FLOAT_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 119);
                break;

            case DOUBLE_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 124);
                break;

            default:
                luaL_error(L, "NYI: callback arg type");
            }
        }
    }

    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);

    dasm_put(Dst, 129, ((mt->pointers || mt->is_reference || mt->type != VOID_TYPE) ? 1 : 0), nargs);

    if (mt->pointers || mt->is_reference) {
        lua_getuservalue(L, -1);
        lua_rawseti(L, -3, ++num_upvals); /* usr value */
        lua_rawseti(L, -2, ++num_upvals); /* mt */

        dasm_put(Dst, 138, num_upvals-1, (uintptr_t)(mt));
    } else {
        switch (mt->type) {
        case ENUM_TYPE:
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */

            dasm_put(Dst, 161, num_upvals-1, (uintptr_t)(mt));
            break;

        case VOID_TYPE:
            dasm_put(Dst, 184);
            lua_pop(L, 1);
            break;

        case BOOL_TYPE:
        case INT8_TYPE:
        case INT16_TYPE:
        case INT32_TYPE:
            dasm_put(Dst, 189);
            if (mt->is_unsigned) {
                dasm_put(Dst, 191);
            } else {
                dasm_put(Dst, 195);
            }
            goto single;

        case INT64_TYPE:
            dasm_put(Dst, 199);
            if (mt->is_unsigned) {
                dasm_put(Dst, 201);
            } else {
                dasm_put(Dst, 205);
            }
            goto dual;

        case INTPTR_TYPE:
            dasm_put(Dst, 209);
            goto single;

        case FLOAT_TYPE:
            dasm_put(Dst, 214);
            goto single;

        case DOUBLE_TYPE:
            dasm_put(Dst, 219);
            goto dual;

        single:
            dasm_put(Dst, 224);
            lua_pop(L, 1);
            break;

        dual:
            dasm_put(Dst, 231);
            lua_pop(L, 1);
            break;

        default:
            luaL_error(L, "NYI: callback return type");
        }
    }

    dasm_put(Dst, 240);

    lua_pop(L, 1); /* upval table - already in registry */
    assert(lua_gettop(L) == top);

    {
        void* p;
        struct ctype ft;
        cfunction func;

        func = compile(Dst, L, NULL, ref);

        ft = *ct;
        ft.is_jitted = 1;
        p = push_cdata(L, ct_usr, &ft);
        *(cfunction*) p = func;

        assert(lua_gettop(L) == top + 1);

        return func;
    }
}

void compile_function(lua_State* L, cfunction func, int ct_usr, const struct ctype* ct)
{
    struct jit* Dst = get_jit(L);;
    int i, nargs, num_upvals;
    const struct ctype* mt;
    void* p;

    int top = lua_gettop(L);

    ct_usr = lua_absindex(L, ct_usr);
    nargs = (int) lua_rawlen(L, ct_usr);

    p = push_cdata(L, ct_usr, ct);
    *(cfunction*) p = func;
    num_upvals = 1;

    dasm_setup(Dst, build_actionlist);

    dasm_put(Dst, 242, nargs);
    if (ct->has_var_arg) {
        dasm_put(Dst, 254, (uintptr_t)("too few arguments"));
    } else {
        dasm_put(Dst, 266, (uintptr_t)("incorrect number of arguments"));
    }

    /* reserve enough stack space for all of the arguments (8 bytes per
     * argument for double and maintains alignment). Add an extra 16 bytes so
     * that the pop {r0, r1, r2, r3} doesn't clean out our stack frame */
    dasm_put(Dst, 278);

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, i);
        mt = (const struct ctype*) lua_touserdata(L, -1);

        if (mt->pointers || mt->is_reference || mt->type == FUNCTION_PTR_TYPE || mt->type == ENUM_TYPE) {
            lua_getuservalue(L, -1);
            num_upvals += 2;

            dasm_put(Dst, 282, (uintptr_t)(mt), (uintptr_t)(lua_upvalueindex(num_upvals)), i);

            if (mt->pointers || mt->is_reference) {
                dasm_put(Dst, 293);
            } else if (mt->type == FUNCTION_PTR_TYPE) {
                dasm_put(Dst, 296);
            } else if (mt->type == ENUM_TYPE) {
                dasm_put(Dst, 299);
            }

            dasm_put(Dst, 302);

        } else {
            lua_pop(L, 1);
            dasm_put(Dst, 304, i);

            switch (mt->type) {
            case INT8_TYPE:
                dasm_put(Dst, 307);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 311);
                } else {
                    dasm_put(Dst, 313);
                }
                dasm_put(Dst, 316);
                break;

            case INT16_TYPE:
                dasm_put(Dst, 318);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 322);
                } else {
                    dasm_put(Dst, 325);
                }
                dasm_put(Dst, 328);
                break;

            case INT32_TYPE:
                if (mt->is_unsigned) {
                    dasm_put(Dst, 330);
                } else {
                    dasm_put(Dst, 334);
                }
                dasm_put(Dst, 338);
                break;

            case INT64_TYPE:
                if (mt->is_unsigned) {
                    dasm_put(Dst, 340);
                } else {
                    dasm_put(Dst, 344);
                }
                dasm_put(Dst, 348);
                break;

            case DOUBLE_TYPE:
                dasm_put(Dst, 351);
                break;

            case INTPTR_TYPE:
                dasm_put(Dst, 357);
                break;

            case FLOAT_TYPE:
                dasm_put(Dst, 362);
                break;

            default:
                luaL_error(L, "NYI: call arg type");
            }
        }
    }

    if (ct->has_var_arg) {
        dasm_put(Dst, 367, nargs+1);
    }

    dasm_put(Dst, 375, (uintptr_t)(&Dst->last_errno));

    dasm_put(Dst, 384);



    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);

    if (mt->pointers || mt->is_reference) {
        lua_getuservalue(L, -1);
        num_upvals += 2;
        dasm_put(Dst, 388, (uintptr_t)(&Dst->last_errno), (uintptr_t)(mt), (uintptr_t)(lua_upvalueindex(num_upvals)));

    } else {
        switch (mt->type) {
        case INT64_TYPE:
            num_upvals++;
            dasm_put(Dst, 411, (uintptr_t)(&Dst->last_errno), (uintptr_t)(mt));
            break;

        case INTPTR_TYPE:
            num_upvals++;
            dasm_put(Dst, 438, (uintptr_t)(&Dst->last_errno), (uintptr_t)(mt));
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 463, (uintptr_t)(&Dst->last_errno));
            break;

        case BOOL_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 474, (uintptr_t)(&Dst->last_errno));
            break;

        case INT8_TYPE:
        case INT16_TYPE:
        case INT32_TYPE:
        case ENUM_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 490, (uintptr_t)(&Dst->last_errno));
            if (mt->is_unsigned) {
                dasm_put(Dst, 501);
            } else {
                dasm_put(Dst, 505);
            }
            dasm_put(Dst, 509);
            break;

        case FLOAT_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 512, (uintptr_t)(&Dst->last_errno));
            break;

        case DOUBLE_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 528, (uintptr_t)(&Dst->last_errno));
            break;

        default:
            luaL_error(L, "NYI: call return type");
        }
    }

    assert(lua_gettop(L) == top + num_upvals);
    lua_pushcclosure(L, (lua_CFunction) compile(Dst, L, func, LUA_NOREF), num_upvals);
}

