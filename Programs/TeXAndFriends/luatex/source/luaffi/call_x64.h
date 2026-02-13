/*
** This file has been pre-processed with DynASM.
** https://luajit.org/dynasm.html
** DynASM version 1.5.0, DynASM x64 version 1.4.0
** DO NOT EDIT! The original file is in "call_x64.dasc".
*/

/* vim: ts=4 sw=4 sts=4 et tw=78
 * Portions copyright (c) 2015-present, Facebook, Inc. All rights reserved.
 * Portions copyright (c) 2011 James R. McKaskill.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
//|.arch x64
#if DASM_VERSION != 10500
#error "Version mismatch between DynASM and included encoding engine"
#endif

//|.actionlist build_actionlist
static const unsigned char build_actionlist[1994] = {
  72,139,141,233,255,72,137,132,253,36,233,255,252,243,15,126,133,233,255,252,
  243,15,90,133,233,255,102,15,214,132,253,36,233,255,252,242,15,90,192,102,
  15,214,132,253,36,233,255,252,242,15,90,192,102,15,126,132,253,36,233,255,
  85,72,137,229,65,84,72,129,252,236,239,102,15,214,69,252,240,102,15,214,77,
  232,102,15,214,85,224,102,15,214,93,216,102,15,214,101,208,102,15,214,109,
  200,102,15,214,117,192,102,15,214,125,184,72,137,125,176,72,137,117,168,72,
  137,85,160,72,137,77,152,76,137,69,144,76,137,77,136,255,73,188,237,237,255,
  72,199,194,237,72,199,198,237,76,137,231,232,251,1,0,255,72,199,194,237,72,
  199,198,252,255,252,255,252,255,252,255,76,137,231,232,251,1,0,255,72,199,
  194,237,72,199,198,237,76,137,231,232,251,1,0,72,186,237,237,72,199,198,252,
  255,252,255,252,255,252,255,76,137,231,232,251,1,1,255,72,137,8,72,199,198,
  252,254,252,255,252,255,252,255,76,137,231,232,251,1,2,255,72,186,237,237,
  72,199,198,0,0,0,0,76,137,231,232,251,1,1,255,72,137,8,255,102,15,214,0,255,
  102,15,214,64,8,255,76,137,231,232,251,1,3,255,15,182,201,72,137,206,76,137,
  231,232,251,1,4,255,15,182,201,255,15,190,201,255,72,137,206,76,137,231,232,
  251,1,5,255,15,183,201,255,15,191,201,255,72,137,206,76,137,231,232,251,1,
  6,255,72,185,237,237,72,199,194,237,72,199,198,237,76,137,231,232,251,1,7,
  255,72,199,194,237,72,199,198,252,254,252,255,252,255,252,255,76,137,231,
  232,251,1,0,72,185,237,237,72,199,194,252,255,252,255,252,255,252,255,72,
  199,198,252,254,252,255,252,255,252,255,76,137,231,232,251,1,8,72,137,68,
  36,32,72,199,198,252,252,252,255,252,255,252,255,76,137,231,232,251,1,9,72,
  139,68,36,32,255,72,199,194,237,72,199,198,252,254,252,255,252,255,252,255,
  76,137,231,232,251,1,0,72,185,237,237,72,199,194,252,255,252,255,252,255,
  252,255,72,199,198,252,254,252,255,252,255,252,255,76,137,231,232,251,1,10,
  137,68,36,32,72,199,198,252,252,252,255,252,255,252,255,76,137,231,232,251,
  1,9,139,68,36,32,255,72,199,198,252,254,252,255,252,255,252,255,76,137,231,
  232,251,1,9,255,72,199,198,252,255,252,255,252,255,252,255,76,137,231,232,
  251,1,11,255,72,199,198,252,255,252,255,252,255,252,255,76,137,231,232,251,
  1,12,255,137,68,36,32,72,199,198,252,253,252,255,252,255,252,255,76,137,231,
  232,251,1,9,139,68,36,32,255,72,199,198,252,255,252,255,252,255,252,255,76,
  137,231,232,251,1,13,255,72,199,198,252,255,252,255,252,255,252,255,76,137,
  231,232,251,1,14,255,72,137,68,36,32,72,199,198,252,253,252,255,252,255,252,
  255,76,137,231,232,251,1,9,72,139,68,36,32,255,72,199,198,252,255,252,255,
  252,255,252,255,76,137,231,232,251,1,15,72,137,68,36,32,72,199,198,252,253,
  252,255,252,255,252,255,76,137,231,232,251,1,9,72,139,68,36,32,255,72,199,
  198,252,255,252,255,252,255,252,255,76,137,231,232,251,1,16,102,15,214,68,
  36,32,72,199,198,252,253,252,255,252,255,252,255,76,137,231,232,251,1,9,255,
  252,242,15,90,68,36,32,255,252,243,15,126,68,36,32,255,72,199,198,252,255,
  252,255,252,255,252,255,76,137,231,232,251,1,17,102,15,214,68,36,32,72,199,
  198,252,253,252,255,252,255,252,255,76,137,231,232,251,1,9,252,243,15,126,
  68,36,32,255,72,199,198,252,255,252,255,252,255,252,255,76,137,231,232,251,
  1,18,102,15,214,68,36,32,102,15,214,76,36,40,72,199,198,252,253,252,255,252,
  255,252,255,76,137,231,232,251,1,9,252,243,15,126,68,36,32,252,243,15,126,
  76,36,40,255,76,139,101,252,248,72,137,252,236,93,194,236,255,85,72,137,229,
  65,84,65,85,73,137,252,252,76,137,231,232,251,1,19,73,137,197,72,129,252,
  248,239,255,15,141,244,248.0,102,184,0,0,72,190,237,237,76,137,231,232,251,
  1,20,248,2,15,142,244,247.0,102,184,0,0,72,190,237,237,76,137,231,232,251,
  1,20,255,15,141,244,247.0,102,184,0,0,72,190,237,237,76,137,231,232,251,1,
  20,255,248,1,255,72,193,224,4,72,41,196,72,129,252,236,239,255,72,185,237,
  237,72,199,194,237,72,199,198,237,76,137,231,232,251,1,8,255,72,185,237,237,
  72,199,194,237,72,199,198,237,76,137,231,232,251,1,21,255,72,185,237,237,
  72,199,194,237,72,199,198,237,76,137,231,232,251,1,10,255,72,199,198,237,
  76,137,231,232,251,1,12,255,15,182,192,255,15,190,192,255,15,183,192,255,
  15,191,192,255,72,199,198,237,76,137,231,232,251,1,12,131,252,248,0,15,149,
  208,15,182,192,255,72,199,198,237,76,137,231,232,251,1,11,255,72,199,198,
  237,76,137,231,232,251,1,15,255,72,199,198,237,76,137,231,232,251,1,13,255,
  72,199,198,237,76,137,231,232,251,1,14,255,72,199,198,237,76,137,231,232,
  251,1,16,255,72,199,198,237,76,137,231,232,251,1,18,255,252,243,15,126,193,
  255,72,141,132,253,36,233,72,131,252,236,4,72,199,194,237,76,137,230,72,137,
  199,232,251,1,18,255,72,199,198,237,76,137,231,232,251,1,17,255,72,199,198,
  237,76,137,231,232,251,1,17,137,4,36,217,4,36,255,137,20,36,217,4,36,255,
  72,137,224,72,129,192,239,73,137,192,72,199,193,237,76,137,252,234,72,199,
  198,237,76,137,231,232,251,1,22,255,72,137,224,72,129,192,239,73,137,192,
  72,199,193,237,76,137,252,234,72,199,198,237,76,137,231,232,251,1,23,255,
  72,137,224,72,129,192,239,73,137,193,73,199,192,237,72,199,193,237,76,137,
  252,234,72,199,198,237,76,137,231,232,251,1,24,255,72,185,237,237,139,1,72,
  137,199,232,251,1,25,255,72,131,196,32,255,252,243,15,126,188,253,36,233,
  255,252,243,15,126,180,253,36,233,255,252,243,15,126,172,253,36,233,255,252,
  243,15,126,164,253,36,233,255,252,243,15,126,156,253,36,233,255,252,243,15,
  126,148,253,36,233,255,252,243,15,126,140,253,36,233,255,252,243,15,126,132,
  253,36,233,255,76,139,140,253,36,233,255,76,139,132,253,36,233,255,72,139,
  140,253,36,233,255,72,139,148,253,36,233,255,72,139,180,253,36,233,255,72,
  139,60,36,255,72,129,196,239,255,176,8,255,232,251,1,26,72,131,252,236,48,
  255,72,137,68,36,32,232,251,1,27,72,185,237,237,137,1,72,186,237,237,72,199,
  198,237,76,137,231,232,251,1,1,72,139,76,36,32,72,137,8,184,1,0,0,0,76,139,
  109,252,240,76,139,101,252,248,72,137,252,236,93,195,255,72,137,68,36,32,
  232,251,1,27,72,185,237,237,137,1,72,139,68,36,32,72,137,198,76,137,231,232,
  251,1,28,184,1,0,0,0,76,139,109,252,240,76,139,101,252,248,72,137,252,236,
  93,195,255,102,15,214,68,36,32,232,251,1,27,72,185,237,237,137,1,72,186,237,
  237,72,199,198,237,76,137,231,232,251,1,1,72,139,76,36,32,72,137,8,184,1,
  0,0,0,76,139,109,252,240,76,139,101,252,248,72,137,252,236,93,195,255,102,
  15,214,76,36,40,102,15,214,68,36,32,232,251,1,27,72,185,237,237,137,1,72,
  186,237,237,72,199,198,237,76,137,231,232,251,1,1,72,139,76,36,40,72,137,
  72,8,72,139,76,36,32,72,137,8,184,1,0,0,0,76,139,109,252,240,76,139,101,252,
  248,72,137,252,236,93,195,255,232,251,1,27,72,185,237,237,137,1,184,0,0,0,
  0,76,139,109,252,240,76,139,101,252,248,72,137,252,236,93,195,255,15,182,
  192,137,68,36,32,232,251,1,27,72,185,237,237,137,1,139,68,36,32,72,137,198,
  76,137,231,232,251,1,4,184,1,0,0,0,76,139,109,252,240,76,139,101,252,248,
  72,137,252,236,93,195,255,137,68,36,32,232,251,1,27,72,185,237,237,137,1,
  139,68,36,32,72,137,198,76,137,231,232,251,1,5,184,1,0,0,0,76,139,109,252,
  240,76,139,101,252,248,72,137,252,236,93,195,255,137,68,36,32,232,251,1,27,
  72,185,237,237,137,1,139,68,36,32,72,137,198,76,137,231,232,251,1,6,184,1,
  0,0,0,76,139,109,252,240,76,139,101,252,248,72,137,252,236,93,195,255,252,
  243,15,90,192,102,15,214,68,36,32,232,251,1,27,72,185,237,237,137,1,252,243,
  15,126,68,36,32,76,137,231,232,251,1,3,184,1,0,0,0,76,139,109,252,240,76,
  139,101,252,248,72,137,252,236,93,195,255
};

//|.globalnames globnames
static const char *const globnames[] = {
  (const char *)0
};
//|.externnames extnames
static const char *const extnames[] = {
  "lua_rawgeti",
  "push_cdata",
  "lua_remove",
  "lua_pushnumber",
  "lua_pushboolean",
  "push_int",
  "push_uint",
  "lua_callk",
  "check_typed_pointer",
  "lua_settop",
  "check_enum",
  "check_uint32",
  "check_int32",
  "check_uint64",
  "check_int64",
  "check_uintptr",
  "check_double",
  "check_complex_float",
  "check_complex_double",
  "lua_gettop",
  "luaL_error",
  "check_typed_cfunction",
  "unpack_varargs_float",
  "unpack_varargs_int",
  "unpack_varargs_stack_skip",
  "SetLastError",
  "FUNCTION",
  "GetLastError",
  "lua_pushinteger",
  (const char *)0
};

//|.define L_ARG, r12
//|.define TOP, r13

//| // the 5 and 6 arg forms are only used on posix x64
//|.macro call_rrrrrr, func, arg0, arg1, arg2, arg3, arg4, arg5
//| mov r9, arg5
//| mov r8, arg4
//| mov rcx, arg3
//| mov rdx, arg2
//| mov rsi, arg1
//| mov rdi, arg0
//| call func
//|.endmacro
//|.macro call_rrrrr, func, arg0, arg1, arg2, arg3, arg4
//| mov r8, arg4
//| mov rcx, arg3
//| mov rdx, arg2
//| mov rsi, arg1
//| mov rdi, arg0
//| call func
//|.endmacro
//|
//|.macro call_rrrp, func, arg0, arg1, arg2, arg3
//| mov64 rcx, arg3
//| mov rdx, arg2
//| mov rsi, arg1
//| mov rdi, arg0
//| call func
//|.endmacro
//|.macro call_rrrr, func, arg0, arg1, arg2, arg3
//| mov rcx, arg3
//| mov rdx, arg2
//| mov rsi, arg1
//| mov rdi, arg0
//| call func
//|.endmacro
//|
//|.macro call_rrp, func, arg0, arg1, arg2
//| mov64 rdx, arg2
//| mov rsi, arg1
//| mov rdi, arg0
//| call func
//|.endmacro
//|.macro call_rrr, func, arg0, arg1, arg2
//| mov rdx, arg2
//| mov rsi, arg1
//| mov rdi, arg0
//| call func
//|.endmacro
//|
//|.macro call_rp, func, arg0, arg1
//| mov64 rsi, arg1
//| mov rdi, arg0
//| call func
//|.endmacro
//|.macro call_rr, func, arg0, arg1
//| mov rsi, arg1
//| mov rdi, arg0
//| call func
//|.endmacro
//|
//|.macro call_r, func, arg0
//| mov rdi, arg0
//| call func
//|.endmacro


//|.macro epilog
//| mov TOP, [rbp-16]
//| mov L_ARG, [rbp-8]
//| mov rsp, rbp
//| pop rbp
//| ret
//|.endmacro

//|.macro get_errno // note trashes registers
//| call extern GetLastError
//| mov64 rcx, perr
//| mov dword [rcx], eax
//|.endmacro

//|.macro too_few_arguments
//| mov ax, 0
//| call_rp extern luaL_error, L_ARG, &"too few arguments"
//|.endmacro

//|.macro too_many_arguments
//| mov ax, 0
//| call_rp extern luaL_error, L_ARG, &"too many arguments"
//|.endmacro

//|.macro lua_return_arg
//| mov eax, 1
//| epilog
//|.endmacro

//|.macro lua_return_void
//| get_errno
//| mov eax, 0
//| epilog
//|.endmacro

//|.macro lua_return_double
//| movq qword [rsp+32], xmm0
//| get_errno
//| movq xmm0, qword [rsp+32]
//| mov rdi, L_ARG
//| call extern lua_pushnumber
//| lua_return_arg
//|.endmacro

//|.macro lua_return_bool
//| movzx eax, al
//| mov [rsp+32], eax
//| get_errno
//| mov eax, [rsp+32]
//| call_rr extern lua_pushboolean, L_ARG, rax
//| lua_return_arg
//|.endmacro

//|.macro lua_return_int
//| mov [rsp+32], eax
//| get_errno
//| mov eax, [rsp+32]
//| call_rr extern push_int, L_ARG, rax
//| lua_return_arg
//|.endmacro

//|.macro lua_return_uint
//| mov [rsp+32], eax
//| get_errno
//| mov eax, [rsp+32]
//| call_rr extern push_uint, L_ARG, rax
//| lua_return_arg
//|.endmacro

//|.macro lua_return_long
//| mov [rsp+32], rax
//| get_errno
//| mov rax, [rsp+32]
//| call_rr extern lua_pushinteger, L_ARG, rax
//| lua_return_arg
//|.endmacro

//|.macro lua_return_ulong
//| mov [rsp+32], rax
//| get_errno
//| mov rax, [rsp+32]
//| call_rr extern lua_pushinteger, L_ARG, rax
//| lua_return_arg
//|.endmacro

//|.macro save_registers
//| // use rbp relative so we store values in the outer stack frame
//| movq qword [rbp-16], xmm0
//| movq qword [rbp-24], xmm1
//| movq qword [rbp-32], xmm2
//| movq qword [rbp-40], xmm3
//| movq qword [rbp-48], xmm4
//| movq qword [rbp-56], xmm5
//| movq qword [rbp-64], xmm6
//| movq qword [rbp-72], xmm7
//| mov [rbp-80], rdi
//| mov [rbp-88], rsi
//| mov [rbp-96], rdx
//| mov [rbp-104], rcx
//| mov [rbp-112], r8
//| mov [rbp-120], r9
//|.endmacro

#define JUMP_SIZE 14
#define MIN_BRANCH INT32_MIN
#define MAX_BRANCH INT32_MAX
#define BRANCH_OFF 4

static void compile_extern_jump(struct jit* jit, lua_State* L, cfunction func, uint8_t* code)
{
    /* The jump code is the function pointer followed by a stub to call the
     * function pointer. The stub exists in 64 bit so we can jump to functions
     * with an offset greater than 2 GB.
     *
     * Note we have to manually set this up since there are commands buffered
     * in the jit state and dynasm doesn't support rip relative addressing.
     *
     * eg on 64 bit:
     * 0-8: function ptr
     * 8-14: jmp aword [rip-14]
     *
     */
    *(cfunction*) code = func;
    code[8] = 0xFF; /* FF /4 operand for jmp */
    code[9] = 0x25; /* RIP displacement */
    *(int32_t*) &code[10] = -14;
}

void compile_globals(struct jit* jit, lua_State* L)
{
    struct jit* Dst = jit;
    int* perr = &jit->last_errno;
    dasm_setup(Dst, build_actionlist);

    /* Note: since the return code uses EBP to reset the stack pointer, we
     * don't have to track the amount of stack space used. It also means we
     * can handle stdcall and cdecl with the same code.
     */

    /* Note the various call_* functions want 32 bytes of 16 byte aligned
     * stack
     */

    compile(Dst, L, NULL, LUA_NOREF);
}

int x86_return_size(lua_State* L, int usr, const struct ctype* ct)
{
    int ret = 0;
    const struct ctype* mt;

    if (ct->calling_convention != C_CALL) {
        size_t i;
        size_t argn = lua_rawlen(L, usr);
        for (i = 1; i <= argn; i++) {
            lua_rawgeti(L, usr, (int) i);
            mt = (const struct ctype*) lua_touserdata(L, -1);

            if (mt->pointers || mt->is_reference) {
                ret += sizeof(void*);
            } else {
                switch (mt->type) {
                case DOUBLE_TYPE:
                case COMPLEX_FLOAT_TYPE:
                case INT64_TYPE:
                    ret += 8;
                    break;
                case COMPLEX_DOUBLE_TYPE:
                    ret += 16;
                    break;
                case INTPTR_TYPE:
                    ret += sizeof(intptr_t);
                    break;
                case FUNCTION_PTR_TYPE:
                    ret += sizeof(cfunction);
                    break;
                case BOOL_TYPE:
                case FLOAT_TYPE:
                case INT8_TYPE:
                case INT16_TYPE:
                case INT32_TYPE:
                case ENUM_TYPE:
                    ret += 4;
                    break;
                default:
                    return luaL_error(L, "NYI - argument type");
                }
            }

            lua_pop(L, 1);
        }
    }

    return ret;
}

#define MAX_INT_REGISTERS(ct) 6 /* rdi, rsi, rdx, rcx, r8, r9 */
#define MAX_FLOAT_REGISTERS(ct) 8 /* xmm0-7 */

struct reg_alloc {
    int floats;
    int ints;
    int off;
};

#define REGISTER_STACK_SPACE(ct) (14*8)

/* Fastcall:
 * Uses ecx, edx as first two int registers
 * Everything else on stack (include 64bit ints)
 * No overflow stack space
 * Pops the stack before returning
 * Returns int in eax, float in ST0
 * We use the same register allocation logic as posix x64 with 2 int regs and 0 float regs
 */

static void get_int(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_int64)
{
    /* grab the register from the shadow space */
    if (reg->ints < MAX_INT_REGISTERS(ct)) {
        //| mov rcx, [rbp - 80 - 8*reg->ints]
        dasm_put(Dst, 0, - 80 - 8*reg->ints);
        reg->ints++;
    }
    else if (is_int64) {
        //| mov rcx, [rbp + reg->off]
        dasm_put(Dst, 0, reg->off);
        reg->off += 8;
    } else {
        //| mov ecx, [rbp + reg->off]
        dasm_put(Dst, 1, reg->off);
		/* The parameters to a function on stack are always 8 byte aligned. */
        reg->off += 8;
    }
}

static void add_int(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_int64)
{
    if (reg->ints < MAX_INT_REGISTERS(ct)) {
        //| mov [rsp + 32 + 8*reg->ints], rax
        dasm_put(Dst, 5, 32 + 8*reg->ints);
        reg->ints++;
    }
    else {
        if (reg->off % 8 != 0) {
            reg->off += 8 - (reg->off % 8);
        }
        if (is_int64) {
            //| mov [rsp + reg->off], rax
            dasm_put(Dst, 5, reg->off);
            reg->off += 8;
        } else {
            //| mov [rsp+reg->off], eax
            dasm_put(Dst, 6, reg->off);
            reg->off += 4;
        }
    }
}

static void get_float(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_double)
{
    int off;

    if (reg->floats < MAX_FLOAT_REGISTERS(ct)) {
        off = -16 - 8*reg->floats;
        reg->floats++;
    }
    else {
        off = reg->off;
        reg->off += is_double ? 8 : 4;
    }

    if (is_double) {
        //| movq xmm0, qword [rbp + off]
        dasm_put(Dst, 12, off);
    } else {
        //| cvtss2sd xmm0, dword [rbp + off]
        dasm_put(Dst, 19, off);
    }
}

static void add_float(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_double)
{
    if (reg->floats < MAX_FLOAT_REGISTERS(ct)) {
        if (is_double) {
            //| movq qword [rsp + 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats)], xmm0
            dasm_put(Dst, 26, 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats));
        } else {
            //| cvtsd2ss xmm0, xmm0
            //| movq qword [rsp + 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats)], xmm0
            dasm_put(Dst, 34, 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats));
        }
        reg->floats++;
    }
	else {
        if (reg->off % 8 != 0) {
            reg->off += 8 - (reg->off % 8);
        }
		if (is_double) {
			//| movq qword [rsp + reg->off], xmm0
			dasm_put(Dst, 26, reg->off);
			reg->off += 8;
		} else {
			//| cvtsd2ss xmm0, xmm0
			//| movd dword [rsp + reg->off], xmm0
			dasm_put(Dst, 47, reg->off);
			reg->off += 8;
		}
	}
}

#define add_pointer(jit, ct, reg) add_int(jit, ct, reg, 1)
#define get_pointer(jit, ct, reg) get_int(jit, ct, reg, 1)

cfunction compile_callback(lua_State* L, int fidx, int ct_usr, const struct ctype* ct)
{
    int i, nargs;
    cfunction* pf;
    struct ctype ct2 = *ct;
    const struct ctype* mt;
    struct reg_alloc reg;
    int num_upvals = 0;
    int top = lua_gettop(L);
    struct jit* Dst = get_jit(L);
    int ref;
    int hidden_arg_off = 0;

    ct_usr = lua_absindex(L, ct_usr);
    fidx = lua_absindex(L, fidx);

    assert(lua_isnil(L, fidx) || lua_isfunction(L, fidx));

    memset(&reg, 0, sizeof(reg));
    reg.off = 16;

    dasm_setup(Dst, build_actionlist);

    // add a table to store ctype and function upvalues
    // callback_set assumes the first value is the lua function
    nargs = (int) lua_rawlen(L, ct_usr);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    if (ct->has_var_arg) {
        luaL_error(L, "can't create callbacks with varargs");
    }

    // setup a stack frame to hold args for the call into lua_call

    //| push rbp
    //| mov rbp, rsp
    //| push L_ARG
    //| // stack is 4 or 8 (mod 16) (L_ARG, rbp, rip)
    //| // 8 to realign, 16 for return vars, 32 for local calls, rest to save registers
    //| sub rsp, 8 + 16 + 32 + REGISTER_STACK_SPACE(ct)
    //| save_registers
    dasm_put(Dst, 60, 8 + 16 + 32 + REGISTER_STACK_SPACE(ct));
    if (ct->calling_convention == FAST_CALL) {
    }

    // hardcode the lua_State* value into the assembly
    //| mov64 L_ARG, L
    dasm_put(Dst, 137, (unsigned int)((uintptr_t)(L)), (unsigned int)(((uintptr_t)(L))>>32));

    /* get the upval table */
    //| call_rrr extern lua_rawgeti, L_ARG, LUA_REGISTRYINDEX, ref
    dasm_put(Dst, 142, ref, LUA_REGISTRYINDEX);

    /* get the lua function */
    lua_pushvalue(L, fidx);
    lua_rawseti(L, -2, ++num_upvals);
    assert(num_upvals == CALLBACK_FUNC_USR_IDX);
    //| call_rrr extern lua_rawgeti, L_ARG, -1, num_upvals
    dasm_put(Dst, 158, num_upvals);

    (void) hidden_arg_off;

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, i);
        mt = (const struct ctype*) lua_touserdata(L, -1);

        if (mt->pointers || mt->is_reference) {
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */
            /* on the lua stack in the callback:
             * upval tbl, lua func, i-1 args
             */
            //| call_rrr extern lua_rawgeti, L_ARG, -i-1, num_upvals-1
            //| call_rrp extern push_cdata, L_ARG, -1, mt
            dasm_put(Dst, 181, num_upvals-1, -i-1, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
            get_pointer(Dst, ct, &reg);
            //| mov [rax], rcx
            //| call_rr, extern lua_remove, L_ARG, -2
            dasm_put(Dst, 219);
        } else {
            switch (mt->type) {
            case INT64_TYPE:
                lua_getuservalue(L, -1);
                lua_rawseti(L, -3, ++num_upvals); /* mt */
                lua_pop(L, 1);
                //| call_rrp extern push_cdata, L_ARG, 0, mt
                dasm_put(Dst, 241, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                get_int(Dst, ct, &reg, 1);
                //| mov [rax], rcx
                dasm_put(Dst, 260);
                break;

            case INTPTR_TYPE:
                lua_getuservalue(L, -1);
                lua_rawseti(L, -3, ++num_upvals); /* mt */
                lua_pop(L, 1);
                //| call_rrp extern push_cdata, L_ARG, 0, mt
                dasm_put(Dst, 241, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                get_pointer(Dst, ct, &reg);
                //| mov [rax], rcx
                dasm_put(Dst, 260);
                break;

            case COMPLEX_FLOAT_TYPE:
                lua_pop(L, 1);
                /* complex floats are two floats packed into a double */
                //| call_rrp extern push_cdata, L_ARG, 0, mt
                dasm_put(Dst, 241, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                get_float(Dst, ct, &reg, 1);
                //| movq qword [rax], xmm0
                dasm_put(Dst, 264);
                break;

            case COMPLEX_DOUBLE_TYPE:
                lua_pop(L, 1);
                //| call_rrp extern push_cdata, L_ARG, 0, mt
                dasm_put(Dst, 241, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                /* real */
                get_float(Dst, ct, &reg, 1);
                //| movq qword [rax], xmm0
                dasm_put(Dst, 264);
                /* imag */
                get_float(Dst, ct, &reg, 1);
                //| movq qword [rax+8], xmm0
                dasm_put(Dst, 269);
                break;

            case FLOAT_TYPE:
            case DOUBLE_TYPE:
                lua_pop(L, 1);
                get_float(Dst, ct, &reg, mt->type == DOUBLE_TYPE);
                //| // for 64bit xmm0 is already set
                //| mov rdi, L_ARG
                //| call extern lua_pushnumber
                dasm_put(Dst, 275);
                break;

            case BOOL_TYPE:
                lua_pop(L, 1);
                get_int(Dst, ct, &reg, 0);
                //| movzx ecx, cl
                //| call_rr extern lua_pushboolean, L_ARG, rcx
                dasm_put(Dst, 283);
                break;

            case INT8_TYPE:
                lua_pop(L, 1);
                get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    //| movzx ecx, cl
                    dasm_put(Dst, 297);
                } else {
                    //| movsx ecx, cl
                    dasm_put(Dst, 301);
                }
                //| call_rr extern push_int, L_ARG, rcx
                dasm_put(Dst, 305);
                break;

            case INT16_TYPE:
                lua_pop(L, 1);
                get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    //| movzx ecx, cx
                    dasm_put(Dst, 316);
                } else {
                    //| movsx ecx, cx
                    dasm_put(Dst, 320);
                }
                //| call_rr extern push_int, L_ARG, rcx
                dasm_put(Dst, 305);
                break;

            case ENUM_TYPE:
            case INT32_TYPE:
                lua_pop(L, 1);
                get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    //| call_rr extern push_uint, L_ARG, rcx
                    dasm_put(Dst, 324);
                } else {
                    //| call_rr extern push_int, L_ARG, rcx
                    dasm_put(Dst, 305);
                }
                break;

            default:
                luaL_error(L, "NYI: callback arg type");
            }
        }
    }

    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);

    //| call_rrrp extern lua_callk, L_ARG, nargs, (mt->pointers || mt->is_reference || mt->type != VOID_TYPE) ? 1 : 0, 0
    dasm_put(Dst, 335, (unsigned int)((uintptr_t)(0)), (unsigned int)(((uintptr_t)(0))>>32), (mt->pointers || mt->is_reference || mt->type != VOID_TYPE) ? 1 : 0, nargs);

    // Unpack the return argument if not "void", also clean-up the lua stack
    // to remove the return argument and bind table. Use lua_settop rather
    // than lua_pop as lua_pop is implemented as a macro.
    if (mt->pointers || mt->is_reference) {
        lua_getuservalue(L, -1);
        lua_rawseti(L, -3, ++num_upvals); /* usr value */
        lua_rawseti(L, -2, ++num_upvals); /* mt */
        //| call_rrr extern lua_rawgeti, L_ARG, -2, num_upvals-1
        //| call_rrrp extern check_typed_pointer, L_ARG, -2, -1, mt
        //| mov [rsp+32], rax
        //| call_rr extern lua_settop, L_ARG, -4
        //| mov rax, [rsp+32]
        dasm_put(Dst, 355, num_upvals-1, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));

    } else {
        switch (mt->type) {
        case ENUM_TYPE:
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */
            //| call_rrr extern lua_rawgeti, L_ARG, -2, num_upvals-1
            //| call_rrrp, extern check_enum, L_ARG, -2, -1, mt
            //| mov [rsp+32], eax
            //| call_rr extern lua_settop, L_ARG, -4
            //| mov eax, [rsp+32]
            dasm_put(Dst, 439, num_upvals-1, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            //| call_rr extern lua_settop, L_ARG, -2
            dasm_put(Dst, 521);
            break;

        case BOOL_TYPE:
        case INT8_TYPE:
        case INT16_TYPE:
        case INT32_TYPE:
            lua_pop(L, 1);
            if (mt->is_unsigned) {
                //| call_rr extern check_uint32, L_ARG, -1
                dasm_put(Dst, 540);
            } else {
                //| call_rr extern check_int32, L_ARG, -1
                dasm_put(Dst, 559);
            }
            //| mov [rsp+32], eax
            //| call_rr extern lua_settop, L_ARG, -3
            //| mov eax, [rsp+32]
            dasm_put(Dst, 578);
            break;

        case INT64_TYPE:
            lua_pop(L, 1);

            if (mt->is_unsigned) {
                //| call_rr extern check_uint64, L_ARG, -1
                dasm_put(Dst, 605);
            } else {
                //| call_rr extern check_int64, L_ARG, -1
                dasm_put(Dst, 624);
            }

            //| mov [rsp+32], rax
            //| call_rr extern lua_settop, L_ARG, -3
            //| mov rax, [rsp+32]
            dasm_put(Dst, 643);
            break;

        case INTPTR_TYPE:
            lua_pop(L, 1);
            //| call_rr extern check_uintptr, L_ARG, -1
            //| mov [rsp+32], rax
            //| call_rr extern lua_settop, L_ARG, -3
            //| mov rax, [rsp+32]
            dasm_put(Dst, 672);
            break;

        case FLOAT_TYPE:
        case DOUBLE_TYPE:
            lua_pop(L, 1);
            //| call_rr extern check_double, L_ARG, -1
            //| movq qword [rsp+32], xmm0
            //| call_rr extern lua_settop, L_ARG, -3
            dasm_put(Dst, 719);
            if (mt->type == FLOAT_TYPE) {
                //| cvtsd2ss xmm0, qword [rsp+32]
                dasm_put(Dst, 762);
            } else {
                //| movq xmm0, qword [rsp+32]
                dasm_put(Dst, 770);
            }
            break;

        case COMPLEX_FLOAT_TYPE:
            lua_pop(L, 1);
#if !defined HAVE_COMPLEX
            luaL_error(L, "ffi lib compiled without complex number support");
#endif
            /* on 64 bit complex floats are two floats packed into a double,
             * on 32 bit returned complex floats use eax and edx */
            //| call_rr extern check_complex_float, L_ARG, -1
            //|
            //| movq qword [rsp+32], xmm0
            //|
            //| call_rr extern lua_settop, L_ARG, -3
            //|
            //| movq xmm0, qword [rsp+32]
            dasm_put(Dst, 778);
            break;

        case COMPLEX_DOUBLE_TYPE:
            lua_pop(L, 1);
#if !defined HAVE_COMPLEX
            luaL_error(L, "ffi lib compiled without complex number support");
#endif
            /* on 64 bit, returned complex doubles use xmm0, xmm1, on 32 bit
             * there is a hidden first parameter that points to 16 bytes where
             * the returned arg is stored which is popped by the called
             * function */
            //| call_rr extern check_complex_double, L_ARG, -1
            //| movq qword [rsp+32], xmm0
            //| movq qword [rsp+40], xmm1
            //| call_rr extern lua_settop, L_ARG, -3
            //| movq xmm0, qword [rsp+32]
            //| movq xmm1, qword [rsp+40]
            dasm_put(Dst, 828);
            break;

        default:
            luaL_error(L, "NYI: callback return type");
        }
    }

    //| mov L_ARG, [rbp-8]
    //| mov rsp, rbp
    //| pop rbp
    //| ret x86_return_size(L, ct_usr, ct)
    dasm_put(Dst, 891, x86_return_size(L, ct_usr, ct));

    lua_pop(L, 1); /* upval table - already in registry */
    assert(lua_gettop(L) == top);

    ct2.is_jitted = 1;
    pf = (cfunction*) push_cdata(L, ct_usr, &ct2);
    *pf = compile(Dst, L, NULL, ref);

    assert(lua_gettop(L) == top + 1);

    return *pf;
}

void compile_function(lua_State* L, cfunction func, int ct_usr, const struct ctype* ct)
{
    size_t i, nargs;
    int num_upvals;
    int orig_top;
    const struct ctype* mbr_ct;
    struct jit* Dst = get_jit(L);
    struct reg_alloc reg;
    void* p;
    int top = lua_gettop(L);
    int* perr = &Dst->last_errno;

    ct_usr = lua_absindex(L, ct_usr);

    memset(&reg, 0, sizeof(reg));
    reg.off = 32 + REGISTER_STACK_SPACE(ct);

    dasm_setup(Dst, build_actionlist);

    orig_top = lua_gettop(L);
    p = push_cdata(L, ct_usr, ct);
    *(cfunction*) p = func;

    nargs = lua_rawlen(L, ct_usr);

    if (ct->calling_convention != C_CALL && ct->has_var_arg) {
        luaL_error(L, "vararg is only allowed with the c calling convention");
    }

    //| push rbp
    //| mov rbp, rsp
    //| push L_ARG
    //| push TOP
    //| // stack is 0 (mod 16) (TOP, L_ARG, rbp, rip)
    //|
    //| // Get L from our arguments and allocate some stack for lua_gettop
    //| mov L_ARG, rdi
    //|
    //| call_r extern lua_gettop, L_ARG
    //| mov TOP, rax // no need for movzxd rax, eax - high word guarenteed to be zero by x86-64
    //| cmp rax, nargs
    dasm_put(Dst, 904, nargs);
    if (!ct->has_var_arg) {
        //| jge >2
        //| too_few_arguments
        //| 2:
        //| jle >1
        //| too_many_arguments
        dasm_put(Dst, 932, (unsigned int)((uintptr_t)(&"too few arguments")), (unsigned int)(((uintptr_t)(&"too few arguments"))>>32), (unsigned int)((uintptr_t)(&"too many arguments")), (unsigned int)(((uintptr_t)(&"too many arguments"))>>32));
    } else {
        //| jge >1
        //| too_few_arguments
        dasm_put(Dst, 973, (unsigned int)((uintptr_t)(&"too few arguments")), (unsigned int)(((uintptr_t)(&"too few arguments"))>>32));
    }

    //| 1:
    dasm_put(Dst, 993);

    /* no need to zero extend eax returned by lua_gettop to rax as x86-64
     * preguarentees that the upper 32 bits will be zero */
    //| shl rax, 4 // reserve 16 bytes per argument - this maintains the alignment mod 16
    //| sub rsp, rax
    //| sub rsp, 32 + REGISTER_STACK_SPACE(ct) // reserve an extra 32 to call local functions
    dasm_put(Dst, 996, 32 + REGISTER_STACK_SPACE(ct));

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, (int) i);
        mbr_ct = (const struct ctype*) lua_touserdata(L, -1);

        if (mbr_ct->pointers || mbr_ct->is_reference) {
            lua_getuservalue(L, -1);
            num_upvals = lua_gettop(L) - orig_top;
            //| call_rrrp extern check_typed_pointer, L_ARG, i, lua_upvalueindex(num_upvals), mbr_ct
            dasm_put(Dst, 1009, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals), i);
            add_pointer(Dst, ct, &reg);
        } else {
            switch (mbr_ct->type) {
            case FUNCTION_PTR_TYPE:
                lua_getuservalue(L, -1);
                num_upvals = lua_gettop(L) - orig_top;
                //| call_rrrp extern check_typed_cfunction, L_ARG, i, lua_upvalueindex(num_upvals), mbr_ct
                dasm_put(Dst, 1029, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals), i);
                add_pointer(Dst, ct, &reg);
                break;

            case ENUM_TYPE:
                lua_getuservalue(L, -1);
                num_upvals = lua_gettop(L) - orig_top;
                //| call_rrrp, extern check_enum, L_ARG, i, lua_upvalueindex(num_upvals), mbr_ct
                dasm_put(Dst, 1049, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals), i);
                add_int(Dst, ct, &reg, 0);
                break;

            case INT8_TYPE:
                //| call_rr extern check_int32, L_ARG, i
                dasm_put(Dst, 1069, i);
                if (mbr_ct->is_unsigned) {
                    //| movzx eax, al
                    dasm_put(Dst, 1081);
                } else {
                    //| movsx eax, al
                    dasm_put(Dst, 1085);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INT16_TYPE:
                //| call_rr extern check_int32, L_ARG, i
                dasm_put(Dst, 1069, i);
                if (mbr_ct->is_unsigned) {
                    //| movzx eax, ax
                    dasm_put(Dst, 1089);
                } else {
                    //| movsx eax, ax
                    dasm_put(Dst, 1093);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case BOOL_TYPE:
                //| call_rr extern check_int32, L_ARG, i
                //| cmp eax, 0
                //| setne al
                //| movzx eax, al
                dasm_put(Dst, 1097, i);
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INT32_TYPE:
                if (mbr_ct->is_unsigned) {
                    //| call_rr extern check_uint32, L_ARG, i
                    dasm_put(Dst, 1119, i);
                } else {
                    //| call_rr extern check_int32, L_ARG, i
                    dasm_put(Dst, 1069, i);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INTPTR_TYPE:
                //| call_rr extern check_uintptr, L_ARG, i
                dasm_put(Dst, 1131, i);
                add_pointer(Dst, ct, &reg);
                lua_pop(L, 1);
                break;

            case INT64_TYPE:
                if (mbr_ct->is_unsigned) {
                    //| call_rr extern check_uint64, L_ARG, i
                    dasm_put(Dst, 1143, i);
                } else {
                    //| call_rr extern check_int64, L_ARG, i
                    dasm_put(Dst, 1155, i);
                }
                add_int(Dst, ct, &reg, 1);
                lua_pop(L, 1);
                break;

            case DOUBLE_TYPE:
                //| call_rr extern check_double, L_ARG, i
                dasm_put(Dst, 1167, i);
                add_float(Dst, ct, &reg, 1);
                lua_pop(L, 1);
                break;

            case COMPLEX_DOUBLE_TYPE:
                /* on 64 bit, returned complex doubles use xmm0, xmm1, on 32 bit
                 * there is a hidden first parameter that points to 16 bytes where
                 * the returned arg is stored (this is popped by the called
                 * function) */
#if defined _WIN64 || defined __amd64__
                //| call_rr extern check_complex_double, L_ARG, i
                dasm_put(Dst, 1179, i);
                add_float(Dst, ct, &reg, 1);
                //| movq xmm0, xmm1
                dasm_put(Dst, 1191);
                add_float(Dst, ct, &reg, 1);
#else
                //| lea rax, [rsp+reg.off]
                //| sub rsp, 4
                //| call_rrr extern check_complex_double, rax, L_ARG, i
                dasm_put(Dst, 1197, reg.off, i);
                reg.off += 16;
#endif
                lua_pop(L, 1);
                break;

            case FLOAT_TYPE:
                //| call_rr extern check_double, L_ARG, i
                dasm_put(Dst, 1167, i);
                add_float(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case COMPLEX_FLOAT_TYPE:
#if defined _WIN64 || defined __amd64__
                //| call_rr extern check_complex_float, L_ARG, i
                dasm_put(Dst, 1223, i);
                /* complex floats are two floats packed into a double */
                add_float(Dst, ct, &reg, 1);
#else
                /* returned complex floats use eax and edx */
                //| call_rr extern check_complex_float, L_ARG, i
                //| mov [rsp], eax
                //| fld dword [rsp]
                dasm_put(Dst, 1235, i);
                add_float(Dst, ct, &reg, 0);
                //| mov [rsp], edx
                //| fld dword [rsp]
                dasm_put(Dst, 1253);
                add_float(Dst, ct, &reg, 0);
#endif
                lua_pop(L, 1);
                break;

            default:
                luaL_error(L, "NYI: call arg type");
            }
        }
    }

    if (ct->has_var_arg) {
        if (reg.floats < MAX_FLOAT_REGISTERS(ct)) {
            //| mov rax, rsp
            //| add rax, 32 + 8*(MAX_INT_REGISTERS(ct) + reg.floats)
            //| call_rrrrr extern unpack_varargs_float, L_ARG, nargs+1, TOP, MAX_FLOAT_REGISTERS(ct) - reg.floats, rax
            dasm_put(Dst, 1260, 32 + 8*(MAX_INT_REGISTERS(ct) + reg.floats), MAX_FLOAT_REGISTERS(ct) - reg.floats, nargs+1);
        }

        if (reg.ints < MAX_INT_REGISTERS(ct)) {
            //| mov rax, rsp
            //| add rax, 32 + 8*(reg.ints)
            //| call_rrrrr extern unpack_varargs_int, L_ARG, nargs+1, TOP, MAX_INT_REGISTERS(ct) - reg.ints, rax
            dasm_put(Dst, 1290, 32 + 8*(reg.ints), MAX_INT_REGISTERS(ct) - reg.ints, nargs+1);
        }

        //| mov rax, rsp
        //| add rax, reg.off
        //| call_rrrrrr extern unpack_varargs_stack_skip, L_ARG, nargs+1, TOP, MAX_INT_REGISTERS(ct) - reg.ints, MAX_FLOAT_REGISTERS(ct) - reg.floats, rax
        dasm_put(Dst, 1320, reg.off, MAX_FLOAT_REGISTERS(ct) - reg.floats, MAX_INT_REGISTERS(ct) - reg.ints, nargs+1);

        reg.floats = MAX_FLOAT_REGISTERS(ct);
        reg.ints = MAX_INT_REGISTERS(ct);
    }

    //| mov64 rcx, perr
    //| mov eax, dword [rcx]
    //| call_r extern SetLastError, rax
    dasm_put(Dst, 1354, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    /* remove the stack space to call local functions */
    //| add rsp, 32
    dasm_put(Dst, 1368);

    switch (reg.floats) {
    case 8:
        //| movq xmm7, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+7)]
        dasm_put(Dst, 1373, 8*(MAX_INT_REGISTERS(ct)+7));
    case 7:
        //| movq xmm6, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+6)]
        dasm_put(Dst, 1382, 8*(MAX_INT_REGISTERS(ct)+6));
    case 6:
        //| movq xmm5, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+5)]
        dasm_put(Dst, 1391, 8*(MAX_INT_REGISTERS(ct)+5));
    case 5:
        //| movq xmm4, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+4)]
        dasm_put(Dst, 1400, 8*(MAX_INT_REGISTERS(ct)+4));
    case 4:
        //| movq xmm3, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+3)]
        dasm_put(Dst, 1409, 8*(MAX_INT_REGISTERS(ct)+3));
    case 3:
        //| movq xmm2, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+2)]
        dasm_put(Dst, 1418, 8*(MAX_INT_REGISTERS(ct)+2));
    case 2:
        //| movq xmm1, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+1)]
        dasm_put(Dst, 1427, 8*(MAX_INT_REGISTERS(ct)+1));
    case 1:
        //| movq xmm0, qword [rsp + 8*(MAX_INT_REGISTERS(ct))]
        dasm_put(Dst, 1436, 8*(MAX_INT_REGISTERS(ct)));
    case 0:
        break;
    }

    switch (reg.ints) {
    case 6:
        //| mov r9, [rsp + 8*5]
        dasm_put(Dst, 1445, 8*5);
    case 5:
        //| mov r8, [rsp + 8*4]
        dasm_put(Dst, 1452, 8*4);
    case 4:
        //| mov rcx, [rsp + 8*3]
        dasm_put(Dst, 1459, 8*3);
    case 3:
        //| mov rdx, [rsp + 8*2]
        dasm_put(Dst, 1466, 8*2);
    case 2:
        //| mov rsi, [rsp + 8*1]
        dasm_put(Dst, 1473, 8*1);
    case 1:
        //| mov rdi, [rsp]
        dasm_put(Dst, 1480);
    case 0:
        break;
    }

    //| add rsp, REGISTER_STACK_SPACE(ct)
    dasm_put(Dst, 1485, REGISTER_STACK_SPACE(ct));

    if (ct->has_var_arg) {
        /* al stores an upper limit on the number of float register, note that
         * its allowed to be more than the actual number of float registers used as
         * long as its 0-8 */
        //| mov al, 8
        dasm_put(Dst, 1490);
    }

    //| call extern FUNCTION
    //| sub rsp, 48 // 32 to be able to call local functions, 16 so we can store some local variables
    dasm_put(Dst, 1493);

    /* note on windows X86 the stack may be only aligned to 4 (stdcall will
     * have popped a multiple of 4 bytes), but we don't need 16 byte alignment on
     * that platform
     */

    lua_rawgeti(L, ct_usr, 0);
    mbr_ct = (const struct ctype*) lua_touserdata(L, -1);

    if (mbr_ct->pointers || mbr_ct->is_reference || mbr_ct->type == INTPTR_TYPE) {
        lua_getuservalue(L, -1);
        num_upvals = lua_gettop(L) - orig_top;
        //| mov [rsp+32], rax // save the pointer
        //| get_errno
        //| call_rrp extern push_cdata, L_ARG, lua_upvalueindex(num_upvals), mbr_ct
        //| mov rcx, [rsp+32]
        //| mov [rax], rcx // *(void**) cdata = val
        //| lua_return_arg
        dasm_put(Dst, 1503, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals));

    } else {
        switch (mbr_ct->type) {
        case FUNCTION_PTR_TYPE:
            lua_getuservalue(L, -1);
            num_upvals = lua_gettop(L) - orig_top;
            //| mov [rsp+32], rax // save the function pointer
            //| get_errno
            //| call_rrp extern push_cdata, L_ARG, lua_upvalueindex(num_upvals), mbr_ct
            //| mov rcx, [rsp+32]
            //| mov [rax], rcx // *(cfunction**) cdata = val
            //| lua_return_arg
            dasm_put(Dst, 1503, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals));
            break;

        case INT64_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                //| lua_return_ulong
                dasm_put(Dst, 1563, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));
            } else {
                //| lua_return_long
                dasm_put(Dst, 1563, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));
            }
            break;

        case COMPLEX_FLOAT_TYPE:
            lua_getuservalue(L, -1);
            num_upvals = lua_gettop(L) - orig_top;
            //| // complex floats are returned as two floats packed into xmm0
            //| movq qword [rsp+32], xmm0
            //|
            //| get_errno
            //| call_rrp extern push_cdata, L_ARG, lua_upvalueindex(num_upvals), mbr_ct
            //|
            //| // ((complex_float*) cdata) = val
            //| mov rcx, [rsp+32]
            //| mov [rax], rcx
            //|
            //| lua_return_arg
            dasm_put(Dst, 1615, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals));
            break;

        case COMPLEX_DOUBLE_TYPE:
            lua_getuservalue(L, -1);
            num_upvals = lua_gettop(L) - orig_top;
            //| // complex doubles are returned as xmm0 and xmm1
            //| movq qword [rsp+40], xmm1
            //| movq qword [rsp+32], xmm0
            //|
            //| get_errno
            //| call_rrp extern push_cdata, L_ARG, lua_upvalueindex(num_upvals), mbr_ct
            //|
            //| // ((complex_double*) cdata)->real = val0
            //| // ((complex_double*) cdata)->imag = val1
            //| mov rcx, [rsp+40]
            //| mov [rax+8], rcx
            //| mov rcx, [rsp+32]
            //| mov [rax], rcx
            //|
            //|
            //| lua_return_arg
            dasm_put(Dst, 1676, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals));
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            //| lua_return_void
            dasm_put(Dst, 1752, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));
            break;

        case BOOL_TYPE:
            lua_pop(L, 1);
            //| lua_return_bool
            dasm_put(Dst, 1784, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));
            break;

        case INT8_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                //| movzx eax, al
                dasm_put(Dst, 1081);
            } else {
                //| movsx eax, al
                dasm_put(Dst, 1085);
            }
            //| lua_return_int
            dasm_put(Dst, 1837, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));
            break;

        case INT16_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                //| movzx eax, ax
                dasm_put(Dst, 1089);
            } else {
                //| movsx eax, ax
                dasm_put(Dst, 1093);
            }
            //| lua_return_int
            dasm_put(Dst, 1837, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));
            break;

        case INT32_TYPE:
        case ENUM_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                //| lua_return_uint
                dasm_put(Dst, 1887, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));
            } else {
                //| lua_return_int
                dasm_put(Dst, 1837, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));
            }
            break;

        case FLOAT_TYPE:
            lua_pop(L, 1);
            //| cvtss2sd xmm0, xmm0
            //| lua_return_double
            dasm_put(Dst, 1937, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));
            break;

        case DOUBLE_TYPE:
            lua_pop(L, 1);
            //| lua_return_double
            dasm_put(Dst, 1942, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));
            break;

        default:
            luaL_error(L, "NYI: call return type");
        }
    }

    {
        cfunction f = compile(Dst, L, func, LUA_NOREF);
        /* add a callback as an upval so that the jitted code gets cleaned up when
         * the function gets gc'd */
        push_callback(L, f, func);
        num_upvals = lua_gettop(L) - orig_top;
        lua_pushcclosure(L, (lua_CFunction) f, num_upvals);
    }
}

