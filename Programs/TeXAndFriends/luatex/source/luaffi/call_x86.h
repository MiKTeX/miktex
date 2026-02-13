/*
** This file has been pre-processed with DynASM.
** https://luajit.org/dynasm.html
** DynASM version 1.5.0, DynASM x86 version 1.4.0
** DO NOT EDIT! The original file is in "call_x86.dasc".
*/

/* vim: ts=4 sw=4 sts=4 et tw=78
 * Portions copyright (c) 2015-present, Facebook, Inc. All rights reserved.
 * Portions copyright (c) 2011 James R. McKaskill.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
//|.if X64
//|.arch x64
//|.else
//|.arch x86
#if DASM_VERSION != 10500
#error "Version mismatch between DynASM and included encoding engine"
#endif
//|.endif

//|.actionlist build_actionlist
static const unsigned char build_actionlist[1915] = {
  139,141,233,255,139,141,233,139,149,233,255,137,132,253,36,233,255,137,132,
  253,36,233,137,148,253,36,233,255,221,133,233,255,217,133,233,255,252,243,
  15,126,133,233,255,252,243,15,90,133,233,255,221,156,253,36,233,255,217,156,
  253,36,233,255,102,15,214,132,253,36,233,255,252,242,15,90,192,102,15,214,
  132,253,36,233,255,252,242,15,90,192,102,15,126,132,253,36,233,255,85,137,
  229,87,129,252,236,239,255,137,77,252,248,137,85,252,244,255,191,237,255,
  199,68,36,8,237,199,68,36,4,237,137,60,36,232,251,1,0,255,199,68,36,8,237,
  199,68,36,4,252,255,252,255,252,255,252,255,137,60,36,232,251,1,0,255,199,
  68,36,8,237,199,68,36,4,237,137,60,36,232,251,1,0,199,68,36,8,237,199,68,
  36,4,252,255,252,255,252,255,252,255,137,60,36,232,251,1,1,255,137,8,199,
  68,36,4,252,254,252,255,252,255,252,255,137,60,36,232,251,1,2,255,199,68,
  36,8,237,199,68,36,4,0,0,0,0,137,60,36,232,251,1,1,255,137,8,137,80,4,255,
  137,8,255,102,15,214,0,255,217,24,255,217,88,4,255,221,24,255,221,88,8,255,
  221,92,36,4,137,60,36,232,251,1,3,255,15,182,201,137,76,36,4,137,60,36,232,
  251,1,4,255,15,182,201,255,15,190,201,255,137,76,36,4,137,60,36,232,251,1,
  5,255,15,183,201,255,15,191,201,255,137,76,36,4,137,60,36,232,251,1,6,255,
  199,68,36,12,0,0,0,0,199,68,36,8,237,199,68,36,4,237,137,60,36,232,251,1,
  7,255,199,68,36,8,237,199,68,36,4,252,254,252,255,252,255,252,255,137,60,
  36,232,251,1,0,199,68,36,12,237,199,68,36,8,252,255,252,255,252,255,252,255,
  199,68,36,4,252,254,252,255,252,255,252,255,137,60,36,232,251,1,8,137,68,
  36,32,199,68,36,4,252,252,252,255,252,255,252,255,137,60,36,232,251,1,9,139,
  68,36,32,255,199,68,36,8,237,199,68,36,4,252,254,252,255,252,255,252,255,
  137,60,36,232,251,1,0,199,68,36,12,237,199,68,36,8,252,255,252,255,252,255,
  252,255,199,68,36,4,252,254,252,255,252,255,252,255,137,60,36,232,251,1,10,
  137,68,36,32,199,68,36,4,252,252,252,255,252,255,252,255,137,60,36,232,251,
  1,9,139,68,36,32,255,199,68,36,4,252,254,252,255,252,255,252,255,137,60,36,
  232,251,1,9,255,199,68,36,4,252,255,252,255,252,255,252,255,137,60,36,232,
  251,1,11,255,199,68,36,4,252,255,252,255,252,255,252,255,137,60,36,232,251,
  1,12,255,137,68,36,32,199,68,36,4,252,253,252,255,252,255,252,255,137,60,
  36,232,251,1,9,139,68,36,32,255,199,68,36,4,252,255,252,255,252,255,252,255,
  137,60,36,232,251,1,13,255,199,68,36,4,252,255,252,255,252,255,252,255,137,
  60,36,232,251,1,14,255,137,68,36,32,137,84,36,36,199,68,36,4,252,253,252,
  255,252,255,252,255,137,60,36,232,251,1,9,139,68,36,32,139,84,36,36,255,199,
  68,36,4,252,255,252,255,252,255,252,255,137,60,36,232,251,1,15,137,68,36,
  32,199,68,36,4,252,253,252,255,252,255,252,255,137,60,36,232,251,1,9,139,
  68,36,32,255,199,68,36,4,252,255,252,255,252,255,252,255,137,60,36,232,251,
  1,16,255,221,92,36,32,199,68,36,4,252,253,252,255,252,255,252,255,137,60,
  36,232,251,1,9,221,68,36,32,255,199,68,36,4,252,255,252,255,252,255,252,255,
  137,60,36,232,251,1,17,137,68,36,32,137,84,36,36,199,68,36,4,252,253,252,
  255,252,255,252,255,137,60,36,232,251,1,9,139,68,36,32,139,84,36,36,255,199,
  68,36,4,252,255,252,255,252,255,252,255,137,60,36,232,251,1,18,102,15,214,
  68,36,32,102,15,214,76,36,40,199,68,36,4,252,253,252,255,252,255,252,255,
  137,60,36,232,251,1,9,252,243,15,126,68,36,32,252,243,15,126,76,36,40,255,
  139,141,233,199,68,36,8,252,255,252,255,252,255,252,255,137,124,36,4,137,
  12,36,232,251,1,18,131,252,236,4,199,68,36,4,252,253,252,255,252,255,252,
  255,137,60,36,232,251,1,9,255,139,125,252,252,137,252,236,93,194,236,255,
  85,137,229,87,86,139,189,233,131,252,236,16,137,60,36,232,251,1,19,137,198,
  129,252,248,239,255,15,141,244,248,102,184,0,0,199,68,36,4,237,137,60,36,
  232,251,1,20,248,2,15,142,244,247,102,184,0,0,199,68,36,4,237,137,60,36,232,
  251,1,20,255,15,141,244,247,102,184,0,0,199,68,36,4,237,137,60,36,232,251,
  1,20,255,248,1,255,193,224,4,41,196,129,252,236,239,255,199,68,36,8,237,199,
  68,36,4,0,0,0,0,137,60,36,232,251,1,1,131,252,236,16,255,199,68,36,12,237,
  199,68,36,8,237,199,68,36,4,237,137,60,36,232,251,1,8,255,199,68,36,12,237,
  199,68,36,8,237,199,68,36,4,237,137,60,36,232,251,1,21,255,199,68,36,12,237,
  199,68,36,8,237,199,68,36,4,237,137,60,36,232,251,1,10,255,199,68,36,4,237,
  137,60,36,232,251,1,12,255,15,182,192,255,15,190,192,255,15,183,192,255,15,
  191,192,255,199,68,36,4,237,137,60,36,232,251,1,12,131,252,248,0,15,149,208,
  15,182,192,255,199,68,36,4,237,137,60,36,232,251,1,11,255,199,68,36,4,237,
  137,60,36,232,251,1,15,255,199,68,36,4,237,137,60,36,232,251,1,13,255,199,
  68,36,4,237,137,60,36,232,251,1,14,255,199,68,36,4,237,137,60,36,232,251,
  1,16,255,199,68,36,4,237,137,60,36,232,251,1,18,255,252,243,15,126,193,255,
  141,132,253,36,233,131,252,236,4,199,68,36,8,237,137,124,36,4,137,4,36,232,
  251,1,18,255,199,68,36,4,237,137,60,36,232,251,1,17,255,199,68,36,4,237,137,
  60,36,232,251,1,17,137,4,36,217,4,36,255,137,20,36,217,4,36,255,137,224,129,
  192,239,137,68,36,12,137,116,36,8,199,68,36,4,237,137,60,36,232,251,1,22,
  255,185,237,139,1,137,4,36,232,251,1,23,255,131,196,32,255,139,148,253,36,
  233,255,139,12,36,255,129,196,239,255,232,251,1,24,131,252,236,48,255,137,
  68,36,32,232,251,1,25,185,237,137,1,199,68,36,8,237,199,68,36,4,237,137,60,
  36,232,251,1,1,139,76,36,32,137,8,184,1,0,0,0,139,117,252,248,139,125,252,
  252,137,252,236,93,195,255,137,68,36,32,232,251,1,25,185,237,137,1,139,68,
  36,32,137,68,36,4,137,60,36,232,251,1,26,184,1,0,0,0,139,117,252,248,139,
  125,252,252,137,252,236,93,195,255,137,84,36,36,137,68,36,32,232,251,1,25,
  185,237,137,1,199,68,36,8,237,199,68,36,4,0,0,0,0,137,60,36,232,251,1,1,139,
  76,36,36,139,84,36,32,137,72,4,137,16,184,1,0,0,0,139,117,252,248,139,125,
  252,252,137,252,236,93,195,255,137,68,36,32,137,84,36,36,232,251,1,25,185,
  237,137,1,199,68,36,8,237,199,68,36,4,237,137,60,36,232,251,1,1,139,76,36,
  32,137,8,139,76,36,36,137,72,4,184,1,0,0,0,139,117,252,248,139,125,252,252,
  137,252,236,93,195,255,131,252,236,4,232,251,1,25,185,237,137,1,184,1,0,0,
  0,139,117,252,248,139,125,252,252,137,252,236,93,195,255,232,251,1,25,185,
  237,137,1,184,0,0,0,0,139,117,252,248,139,125,252,252,137,252,236,93,195,
  255,15,182,192,137,68,36,32,232,251,1,25,185,237,137,1,139,68,36,32,137,68,
  36,4,137,60,36,232,251,1,4,184,1,0,0,0,139,117,252,248,139,125,252,252,137,
  252,236,93,195,255,137,68,36,32,232,251,1,25,185,237,137,1,139,68,36,32,137,
  68,36,4,137,60,36,232,251,1,5,184,1,0,0,0,139,117,252,248,139,125,252,252,
  137,252,236,93,195,255,137,68,36,32,232,251,1,25,185,237,137,1,139,68,36,
  32,137,68,36,4,137,60,36,232,251,1,6,184,1,0,0,0,139,117,252,248,139,125,
  252,252,137,252,236,93,195,255,221,92,36,4,232,251,1,25,185,237,137,1,137,
  60,36,232,251,1,3,184,1,0,0,0,139,117,252,248,139,125,252,252,137,252,236,
  93,195,255
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
  "unpack_varargs_stack",
  "SetLastError",
  "FUNCTION",
  "GetLastError",
  "lua_pushinteger",
  (const char *)0
};

//|.if not X64
//|.define RET_H, edx // for int64_t returns
//|.define RET_L, eax
//|.endif

//|.if X64
//|.define L_ARG, r12
//|.define TOP, r13
//|.else
//|.define L_ARG, rdi
//|.define TOP, rsi
//|.endif

//|.if X64WIN
//|
//|.macro call_rrrp, func, arg0, arg1, arg2, arg3
//| mov64 r9, arg3
//| mov r8, arg2
//| mov rdx, arg1
//| mov rcx, arg0
//| call func
//|.endmacro
//|.macro call_rrrr, func, arg0, arg1, arg2, arg3
//| mov r9, arg3
//| mov r8, arg2
//| mov rdx, arg1
//| mov rcx, arg0
//| call func
//|.endmacro
//|
//|.macro call_rrp, func, arg0, arg1, arg2
//| mov64 r8, arg2
//| mov rdx, arg1
//| mov rcx, arg0
//| call func
//|.endmacro
//|.macro call_rrr, func, arg0, arg1, arg2
//| mov r8, arg2
//| mov rdx, arg1
//| mov rcx, arg0
//| call func
//|.endmacro
//|
//|.macro call_rp, func, arg0, arg1
//| mov64 rdx, arg1
//| mov rcx, arg0
//| call func
//|.endmacro
//|.macro call_rr, func, arg0, arg1
//| mov rdx, arg1
//| mov rcx, arg0
//| call func
//|.endmacro
//|
//|.macro call_r, func, arg0
//| mov rcx, arg0
//| call func
//|.endmacro
//|
//|.elif X64
//|
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
//|
//|.else
//| // define the 64bit registers to the 32 bit counterparts, so the common
//| // code can use r*x for all pointers
//|.define rax, eax
//|.define rcx, ecx
//|.define rdx, edx
//|.define rsp, esp
//|.define rbp, ebp
//|.define rdi, edi
//|.define rsi, esi
//|.define mov64, mov
//|
//|.macro call_rrrr, func, arg0, arg1, arg2, arg3
//| mov dword [rsp+12], arg3
//| mov dword [rsp+8], arg2
//| mov dword [rsp+4], arg1
//| mov dword [rsp], arg0
//| call func
//|.endmacro
//|.macro call_rrr, func, arg0, arg1, arg2
//| mov dword [rsp+8], arg2
//| mov dword [rsp+4], arg1
//| mov dword [rsp], arg0
//| call func
//|.endmacro
//|.macro call_rr, func, arg0, arg1
//| mov dword [rsp+4], arg1
//| mov dword [rsp], arg0
//| call func
//|.endmacro
//|.macro call_r, func, arg0
//| mov dword [rsp], arg0
//| call func
//|.endmacro
//|
//|.define call_rrrp, call_rrrr
//|.define call_rrp, call_rrr
//|.define call_rp, call_rr
//|
//|.endif

//|.macro epilog
//|.if X64
//| mov TOP, [rbp-16]
//| mov L_ARG, [rbp-8]
//|.else
//| mov TOP, [rbp-8]
//| mov L_ARG, [rbp-4]
//|.endif
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
//|.if X64
//| movq qword [rsp+32], xmm0
//|.else
//| fstp qword [rsp+4] // note get_errno doesn't require any stack on x86
//|.endif
//|
//| get_errno
//|
//|.if X64WIN
//| movq xmm1, qword [rsp+32]
//| mov rcx, L_ARG
//|.elif X64
//| movq xmm0, qword [rsp+32]
//| mov rdi, L_ARG
//|.else
//| mov [rsp], L_ARG
//|.endif
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
//|.if X64WIN
//| // use the provided shadow space for int registers above prev rbp and
//| // return address
//| mov [rbp+16], rcx
//| mov [rbp+24], rdx
//| mov [rbp+32], r8
//| mov [rbp+40], r9
//| // use the extra space we added for float registers
//| // -16 to store underneath previous value of L_ARG
//| movq qword [rbp-16], xmm0
//| movq qword [rbp-24], xmm1
//| movq qword [rbp-32], xmm2
//| movq qword [rbp-40], xmm3
//|.elif X64
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
//|.else
//| // fastcall, -8 to store underneath previous value of L_ARG
//| mov [rbp-8], ecx
//| mov [rbp-12], edx
//|.endif
//|.endmacro

#if defined _WIN64 || defined __amd64__
#define JUMP_SIZE 14
#else
#define JUMP_SIZE 4
#endif

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
     * for 32 bit we only set the function ptr as it can always fit in a 32
     * bit displacement
     */
#if defined _WIN64 || defined __amd64__
    *(cfunction*) code = func;
    code[8] = 0xFF; /* FF /4 operand for jmp */
    code[9] = 0x25; /* RIP displacement */
    *(int32_t*) &code[10] = -14;
#else
    *(cfunction*) code = func;
#endif
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

#if !defined _WIN64 && !defined __amd64__
    lua_rawgeti(L, usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);
    if (!mt->pointers && !mt->is_reference && mt->type == COMPLEX_DOUBLE_TYPE) {
        ret += sizeof(void*);
    }
    lua_pop(L, 1);
#endif

    return ret;
}

#ifdef _WIN64
#define MAX_REGISTERS(ct) 4 /* rcx, rdx, r8, r9 */

#elif defined __amd64__
#define MAX_INT_REGISTERS(ct) 6 /* rdi, rsi, rdx, rcx, r8, r9 */
#define MAX_FLOAT_REGISTERS(ct) 8 /* xmm0-7 */

#else
#define MAX_INT_REGISTERS(ct) ((ct)->calling_convention == FAST_CALL ? 2 /* ecx, edx */ : 0)
#define MAX_FLOAT_REGISTERS(ct) 0
#endif

struct reg_alloc {
#ifdef _WIN64
    int regs;
    int is_float[4];
    int is_int[4];
#else
    int floats;
    int ints;
#endif
    int off;
};

#ifdef _WIN64
#define REGISTER_STACK_SPACE(ct) (4*8)
#elif defined __amd64__
#define REGISTER_STACK_SPACE(ct) (14*8)
#else
#define REGISTER_STACK_SPACE(ct) ALIGN_UP(((ct)->calling_convention == FAST_CALL ? 2*4 : 0), 15)
#endif

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
#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        //| mov rcx, [rbp + 16 + 8*reg->regs]
        dasm_put(Dst, 0, 16 + 8*reg->regs);
        reg->regs++;
    }
#elif __amd64__
    if (reg->ints < MAX_INT_REGISTERS(ct)) {
        //| mov rcx, [rbp - 80 - 8*reg->ints]
        dasm_put(Dst, 0, - 80 - 8*reg->ints);
        reg->ints++;
    }
#else
    if (!is_int64 && reg->ints < MAX_INT_REGISTERS(ct)) {
        //| mov ecx, [rbp - 8 - 4*reg->ints]
        dasm_put(Dst, 0, - 8 - 4*reg->ints);
        reg->ints++;
    }
#endif
    else if (is_int64) {
        //|.if X64
        //| mov rcx, [rbp + reg->off]
        //|.else
        //| mov rcx, [rbp + reg->off]
        //| mov rdx, [rbp + reg->off + 4]
        //|.endif
        dasm_put(Dst, 4, reg->off, reg->off + 4);
        reg->off += 8;
    } else {
        //| mov ecx, [rbp + reg->off]
        dasm_put(Dst, 0, reg->off);
#if defined __amd64__ || defined _WIN64
		/* The parameters to a function on stack are always 8 byte aligned. */
        reg->off += 8;
#else
        reg->off += 4;
#endif
    }
}

static void add_int(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_int64)
{
#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        //| mov [rsp + 32 + 8*(reg->regs)], rax
        dasm_put(Dst, 11, 32 + 8*(reg->regs));
        reg->is_int[reg->regs++] = 1;
    }
#elif __amd64__
    if (reg->ints < MAX_INT_REGISTERS(ct)) {
        //| mov [rsp + 32 + 8*reg->ints], rax
        dasm_put(Dst, 11, 32 + 8*reg->ints);
        reg->ints++;
    }
#else
    if (!is_int64 && reg->ints < MAX_INT_REGISTERS(ct)) {
        //| mov [rsp + 32 + 4*reg->ints], rax
        dasm_put(Dst, 11, 32 + 4*reg->ints);
        reg->ints++;
    }
#endif
    else {
#if defined _WIN64 || defined __amd64__
        if (reg->off % 8 != 0) {
            reg->off += 8 - (reg->off % 8);
        }
#endif
        if (is_int64) {
            //|.if X64
            //| mov [rsp + reg->off], rax
            //|.else
            //| mov [rsp + reg->off], RET_L
            //| mov [rsp + reg->off + 4], RET_H
            //|.endif
            dasm_put(Dst, 17, reg->off, reg->off + 4);
            reg->off += 8;
        } else {
            //| mov [rsp+reg->off], eax
            dasm_put(Dst, 11, reg->off);
            reg->off += 4;
        }
    }
}

static void get_float(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_double)
{
#if !defined _WIN64 && !defined __amd64__
    assert(MAX_FLOAT_REGISTERS(ct) == 0);
    if (is_double) {
        //| fld qword [rbp + reg->off]
        dasm_put(Dst, 28, reg->off);
        reg->off += 8;
    } else {
        //| fld dword [rbp + reg->off]
        dasm_put(Dst, 32, reg->off);
        reg->off += 4;
    }
#else
    int off;

#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        off = -16 - 8*reg->regs;
        reg->regs++;
    }
#else
    if (reg->floats < MAX_FLOAT_REGISTERS(ct)) {
        off = -16 - 8*reg->floats;
        reg->floats++;
    }
#endif
    else {
        off = reg->off;
        reg->off += is_double ? 8 : 4;
    }

    if (is_double) {
        //| movq xmm0, qword [rbp + off]
        dasm_put(Dst, 36, off);
    } else {
        //| cvtss2sd xmm0, dword [rbp + off]
        dasm_put(Dst, 43, off);
    }
#endif
}

static void add_float(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_double)
{
#if !defined _WIN64 && !defined __amd64__
    assert(MAX_FLOAT_REGISTERS(ct) == 0);
    if (is_double) {
        //| fstp qword [rsp + reg->off]
        dasm_put(Dst, 50, reg->off);
        reg->off += 8;
    } else {
        //| fstp dword [rsp + reg->off]
        dasm_put(Dst, 56, reg->off);
        reg->off += 4;
    }
#else

#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        if (is_double) {
            //| movq qword [rsp + 32 + 8*(reg->regs)], xmm0
            dasm_put(Dst, 62, 32 + 8*(reg->regs));
        } else {
            //| cvtsd2ss xmm0, xmm0
            //| movq qword [rsp + 32 + 8*(reg->regs)], xmm0
            dasm_put(Dst, 70, 32 + 8*(reg->regs));
        }
        reg->is_float[reg->regs++] = 1;
    }
#else
    if (reg->floats < MAX_FLOAT_REGISTERS(ct)) {
        if (is_double) {
            //| movq qword [rsp + 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats)], xmm0
            dasm_put(Dst, 62, 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats));
        } else {
            //| cvtsd2ss xmm0, xmm0
            //| movq qword [rsp + 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats)], xmm0
            dasm_put(Dst, 70, 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats));
        }
        reg->floats++;
    }
#endif

    else if (is_double) {
        //| movq qword [rsp + reg->off], xmm0
        dasm_put(Dst, 62, reg->off);
        reg->off += 8;
    } else {
        //| cvtsd2ss xmm0, xmm0
        //| movd dword [rsp + reg->off], xmm0
        dasm_put(Dst, 83, reg->off);
        reg->off += 4;
    }
#endif
}

#if defined _WIN64 || defined __amd64__
#define add_pointer(jit, ct, reg) add_int(jit, ct, reg, 1)
#define get_pointer(jit, ct, reg) get_int(jit, ct, reg, 1)
#else
#define add_pointer(jit, ct, reg) add_int(jit, ct, reg, 0)
#define get_pointer(jit, ct, reg) get_int(jit, ct, reg, 0)
#endif

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
#ifdef _WIN64
    reg.off = 16 + REGISTER_STACK_SPACE(ct); /* stack registers are above the shadow space */
#elif __amd64__
    reg.off = 16;
#else
    reg.off = 8;
#endif

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
    //|.if X64
    //| // 8 to realign, 16 for return vars, 32 for local calls, rest to save registers
    //| sub rsp, 8 + 16 + 32 + REGISTER_STACK_SPACE(ct)
    //| save_registers
    //|.else
    //| // 4 to realign, 16 for return vars, 32 for local calls, rest to save registers
    //| sub rsp, 4 + 16 + 32 + REGISTER_STACK_SPACE(ct)
    dasm_put(Dst, 96, 4 + 16 + 32 + REGISTER_STACK_SPACE(ct));
    if (ct->calling_convention == FAST_CALL) {
        //| save_registers
        dasm_put(Dst, 105);
    }
    //|.endif

    // hardcode the lua_State* value into the assembly
    //| mov64 L_ARG, L
    dasm_put(Dst, 114, L);

    /* get the upval table */
    //| call_rrr extern lua_rawgeti, L_ARG, LUA_REGISTRYINDEX, ref
    dasm_put(Dst, 117, ref, LUA_REGISTRYINDEX);

    /* get the lua function */
    lua_pushvalue(L, fidx);
    lua_rawseti(L, -2, ++num_upvals);
    assert(num_upvals == CALLBACK_FUNC_USR_IDX);
    //| call_rrr extern lua_rawgeti, L_ARG, -1, num_upvals
    dasm_put(Dst, 135, num_upvals);

#if !defined _WIN64 && !defined __amd64__
    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);
    if (!mt->pointers && !mt->is_reference && mt->type == COMPLEX_DOUBLE_TYPE) {
        hidden_arg_off = reg.off;
        reg.off += sizeof(void*);
    }
    lua_pop(L, 1);
#else
    (void) hidden_arg_off;
#endif

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
            dasm_put(Dst, 160, num_upvals-1, -i-1, mt);
            get_pointer(Dst, ct, &reg);
            //| mov [rax], rcx
            //| call_rr, extern lua_remove, L_ARG, -2
            dasm_put(Dst, 202);
        } else {
            switch (mt->type) {
            case INT64_TYPE:
                lua_getuservalue(L, -1);
                lua_rawseti(L, -3, ++num_upvals); /* mt */
                lua_pop(L, 1);
                //| call_rrp extern push_cdata, L_ARG, 0, mt
                dasm_put(Dst, 224, mt);
                get_int(Dst, ct, &reg, 1);
                //|.if X64
                //| mov [rax], rcx
                //|.else
                //| mov [rax], ecx
                //| mov [rax+4], edx
                //|.endif
                dasm_put(Dst, 245);
                break;

            case INTPTR_TYPE:
                lua_getuservalue(L, -1);
                lua_rawseti(L, -3, ++num_upvals); /* mt */
                lua_pop(L, 1);
                //| call_rrp extern push_cdata, L_ARG, 0, mt
                dasm_put(Dst, 224, mt);
                get_pointer(Dst, ct, &reg);
                //| mov [rax], rcx
                dasm_put(Dst, 251);
                break;

            case COMPLEX_FLOAT_TYPE:
                lua_pop(L, 1);
#if defined _WIN64 || defined __amd64__
                /* complex floats are two floats packed into a double */
                //| call_rrp extern push_cdata, L_ARG, 0, mt
                dasm_put(Dst, 224, mt);
                get_float(Dst, ct, &reg, 1);
                //| movq qword [rax], xmm0
                dasm_put(Dst, 254);
#else
                /* complex floats are real followed by imag on the stack */
                //| call_rrp extern push_cdata, L_ARG, 0, mt
                dasm_put(Dst, 224, mt);
                get_float(Dst, ct, &reg, 0);
                //| fstp dword [rax]
                dasm_put(Dst, 259);
                get_float(Dst, ct, &reg, 0);
                //| fstp dword [rax+4]
                dasm_put(Dst, 262);
#endif
                break;

            case COMPLEX_DOUBLE_TYPE:
                lua_pop(L, 1);
                //| call_rrp extern push_cdata, L_ARG, 0, mt
                dasm_put(Dst, 224, mt);
                /* real */
                get_float(Dst, ct, &reg, 1);
                //|.if X64
                //| movq qword [rax], xmm0
                //|.else
                //| fstp qword [rax]
                //|.endif
                dasm_put(Dst, 266);
                /* imag */
                get_float(Dst, ct, &reg, 1);
                //|.if X64
                //| movq qword [rax+8], xmm0
                //|.else
                //| fstp qword [rax+8]
                //|.endif
                dasm_put(Dst, 269);
                break;

            case FLOAT_TYPE:
            case DOUBLE_TYPE:
                lua_pop(L, 1);
                get_float(Dst, ct, &reg, mt->type == DOUBLE_TYPE);
                //|.if X64WIN
                //| movq xmm1, xmm0
                //| mov rcx, L_ARG
                //|.elif X64
                //| // for 64bit xmm0 is already set
                //| mov rdi, L_ARG
                //|.else
                //| fstp qword [rsp+4]
                //| mov [rsp], L_ARG
                //|.endif
                //| call extern lua_pushnumber
                dasm_put(Dst, 273);
                break;

            case BOOL_TYPE:
                lua_pop(L, 1);
                get_int(Dst, ct, &reg, 0);
                //| movzx ecx, cl
                //| call_rr extern lua_pushboolean, L_ARG, rcx
                dasm_put(Dst, 285);
                break;

            case INT8_TYPE:
                lua_pop(L, 1);
                get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    //| movzx ecx, cl
                    dasm_put(Dst, 300);
                } else {
                    //| movsx ecx, cl
                    dasm_put(Dst, 304);
                }
                //| call_rr extern push_int, L_ARG, rcx
                dasm_put(Dst, 308);
                break;

            case INT16_TYPE:
                lua_pop(L, 1);
                get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    //| movzx ecx, cx
                    dasm_put(Dst, 320);
                } else {
                    //| movsx ecx, cx
                    dasm_put(Dst, 324);
                }
                //| call_rr extern push_int, L_ARG, rcx
                dasm_put(Dst, 308);
                break;

            case ENUM_TYPE:
            case INT32_TYPE:
                lua_pop(L, 1);
                get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    //| call_rr extern push_uint, L_ARG, rcx
                    dasm_put(Dst, 328);
                } else {
                    //| call_rr extern push_int, L_ARG, rcx
                    dasm_put(Dst, 308);
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
    dasm_put(Dst, 340, (mt->pointers || mt->is_reference || mt->type != VOID_TYPE) ? 1 : 0, nargs);

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
        dasm_put(Dst, 366, num_upvals-1, mt);

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
            dasm_put(Dst, 454, num_upvals-1, mt);
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            //| call_rr extern lua_settop, L_ARG, -2
            dasm_put(Dst, 542);
            break;

        case BOOL_TYPE:
        case INT8_TYPE:
        case INT16_TYPE:
        case INT32_TYPE:
            lua_pop(L, 1);
            if (mt->is_unsigned) {
                //| call_rr extern check_uint32, L_ARG, -1
                dasm_put(Dst, 562);
            } else {
                //| call_rr extern check_int32, L_ARG, -1
                dasm_put(Dst, 582);
            }
            //| mov [rsp+32], eax
            //| call_rr extern lua_settop, L_ARG, -3
            //| mov eax, [rsp+32]
            dasm_put(Dst, 602);
            break;

        case INT64_TYPE:
            lua_pop(L, 1);

            if (mt->is_unsigned) {
                //| call_rr extern check_uint64, L_ARG, -1
                dasm_put(Dst, 630);
            } else {
                //| call_rr extern check_int64, L_ARG, -1
                dasm_put(Dst, 650);
            }

            //|.if X64
            //| mov [rsp+32], rax
            //|.else
            //| mov [rsp+32], RET_L
            //| mov [rsp+36], RET_H
            //|.endif
            //| call_rr extern lua_settop, L_ARG, -3
            //|.if X64
            //| mov rax, [rsp+32]
            //|.else
            //| mov RET_L, [rsp+32]
            //| mov RET_H, [rsp+36]
            //|.endif
            dasm_put(Dst, 670);
            break;

        case INTPTR_TYPE:
            lua_pop(L, 1);
            //| call_rr extern check_uintptr, L_ARG, -1
            //| mov [rsp+32], rax
            //| call_rr extern lua_settop, L_ARG, -3
            //| mov rax, [rsp+32]
            dasm_put(Dst, 706);
            break;

        case FLOAT_TYPE:
        case DOUBLE_TYPE:
            lua_pop(L, 1);
            //| call_rr extern check_double, L_ARG, -1
            //|.if X64
            //| movq qword [rsp+32], xmm0
            //| call_rr extern lua_settop, L_ARG, -3
            dasm_put(Dst, 753);
            if (mt->type == FLOAT_TYPE) {
                //| cvtsd2ss xmm0, qword [rsp+32]
            } else {
                //| movq xmm0, qword [rsp+32]
            }
            //|.else
            //| fstp qword [rsp+32]
            //| call_rr extern lua_settop, L_ARG, -3
            //| fld qword [rsp+32]
            //|.endif
            dasm_put(Dst, 773);
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
            //|.if X64
            //| movq qword [rsp+32], xmm0
            //|.else
            //| mov [rsp+32], eax
            //| mov [rsp+36], edx
            //|.endif
            //|
            //| call_rr extern lua_settop, L_ARG, -3
            //|
            //|.if X64
            //| movq xmm0, qword [rsp+32]
            //|.else
            //| mov eax, [rsp+32]
            //| mov edx, [rsp+36]
            //|.endif
            dasm_put(Dst, 801);
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
#if defined _WIN64 || defined __amd64__
            //| call_rr extern check_complex_double, L_ARG, -1
            //| movq qword [rsp+32], xmm0
            //| movq qword [rsp+40], xmm1
            //| call_rr extern lua_settop, L_ARG, -3
            //| movq xmm0, qword [rsp+32]
            //| movq xmm1, qword [rsp+40]
            dasm_put(Dst, 856);
#else
            //| mov rcx, [rbp + hidden_arg_off]
            //| call_rrr extern check_complex_double, rcx, L_ARG, -1
            //| sub rsp, 4 // to realign from popped hidden arg
            //| call_rr extern lua_settop, L_ARG, -3
            dasm_put(Dst, 921, hidden_arg_off);
#endif
            break;

        default:
            luaL_error(L, "NYI: callback return type");
        }
    }

    //|.if X64
    //| mov L_ARG, [rbp-8]
    //|.else
    //| mov L_ARG, [rbp-4]
    //|.endif
    //| mov rsp, rbp
    //| pop rbp
    //| ret x86_return_size(L, ct_usr, ct)
    dasm_put(Dst, 971, x86_return_size(L, ct_usr, ct));

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

    p = push_cdata(L, ct_usr, ct);
    *(cfunction*) p = func;
    num_upvals = 1;

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
    //|.if X64WIN
    //| mov L_ARG, rcx
    //| sub rsp, 32 // shadow space
    //|.elif X64
    //| mov L_ARG, rdi
    //|.else
    //| mov L_ARG, [rbp + 8]
    //| sub rsp, 16
    //|.endif
    //|
    //| call_r extern lua_gettop, L_ARG
    //| mov TOP, rax // no need for movzxd rax, eax - high word guarenteed to be zero by x86-64
    //| cmp rax, nargs
    dasm_put(Dst, 982, 8, nargs);
    if (!ct->has_var_arg) {
        //| jge >2
        //| too_few_arguments
        //| 2:
        //| jle >1
        //| too_many_arguments
        dasm_put(Dst, 1008, (ptrdiff_t)("too few arguments"), (ptrdiff_t)("too many arguments"));
    } else {
        //| jge >1
        //| too_few_arguments
        dasm_put(Dst, 1051, (ptrdiff_t)("too few arguments"));
    }

    //| 1:
    dasm_put(Dst, 1072);

    /* no need to zero extend eax returned by lua_gettop to rax as x86-64
     * preguarentees that the upper 32 bits will be zero */
    //| shl rax, 4 // reserve 16 bytes per argument - this maintains the alignment mod 16
    //| sub rsp, rax
    //| sub rsp, 32 + REGISTER_STACK_SPACE(ct) // reserve an extra 32 to call local functions
    dasm_put(Dst, 1075, 32 + REGISTER_STACK_SPACE(ct));

#if !defined _WIN64 && !defined __amd64__
    /* Returned complex doubles require a hidden first parameter where the
     * data is stored, which is popped by the calling code. */
    lua_rawgeti(L, ct_usr, 0);
    mbr_ct = (const struct ctype*) lua_touserdata(L, -1);
    if (!mbr_ct->pointers && !mbr_ct->is_reference && mbr_ct->type == COMPLEX_DOUBLE_TYPE) {
        /* we can allocate more space for arguments as long as no add_*
         * function has been called yet, mbr_ct will be added as an upvalue in
         * the return processing later */
        //| call_rrp extern push_cdata, L_ARG, 0, mbr_ct
        //| sub rsp, 16
        dasm_put(Dst, 1085, mbr_ct);
        add_pointer(Dst, ct, &reg);
    }
    lua_pop(L, 1);
#endif

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, (int) i);
        mbr_ct = (const struct ctype*) lua_touserdata(L, -1);

        if (mbr_ct->pointers || mbr_ct->is_reference) {
            lua_getuservalue(L, -1);
            num_upvals += 2;
            //| call_rrrp extern check_typed_pointer, L_ARG, i, lua_upvalueindex(num_upvals), mbr_ct
            dasm_put(Dst, 1110, mbr_ct, lua_upvalueindex(num_upvals), i);
            add_pointer(Dst, ct, &reg);
        } else {
            switch (mbr_ct->type) {
            case FUNCTION_PTR_TYPE:
                lua_getuservalue(L, -1);
                num_upvals += 2;
                //| call_rrrp extern check_typed_cfunction, L_ARG, i, lua_upvalueindex(num_upvals), mbr_ct
                dasm_put(Dst, 1133, mbr_ct, lua_upvalueindex(num_upvals), i);
                add_pointer(Dst, ct, &reg);
                break;

            case ENUM_TYPE:
                lua_getuservalue(L, -1);
                num_upvals += 2;
                //| call_rrrp, extern check_enum, L_ARG, i, lua_upvalueindex(num_upvals), mbr_ct
                dasm_put(Dst, 1156, mbr_ct, lua_upvalueindex(num_upvals), i);
                add_int(Dst, ct, &reg, 0);
                break;

            case INT8_TYPE:
                //| call_rr extern check_int32, L_ARG, i
                dasm_put(Dst, 1179, i);
                if (mbr_ct->is_unsigned) {
                    //| movzx eax, al
                    dasm_put(Dst, 1192);
                } else {
                    //| movsx eax, al
                    dasm_put(Dst, 1196);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INT16_TYPE:
                //| call_rr extern check_int32, L_ARG, i
                dasm_put(Dst, 1179, i);
                if (mbr_ct->is_unsigned) {
                    //| movzx eax, ax
                    dasm_put(Dst, 1200);
                } else {
                    //| movsx eax, ax
                    dasm_put(Dst, 1204);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case BOOL_TYPE:
                //| call_rr extern check_int32, L_ARG, i
                //| cmp eax, 0
                //| setne al
                //| movzx eax, al
                dasm_put(Dst, 1208, i);
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INT32_TYPE:
                if (mbr_ct->is_unsigned) {
                    //| call_rr extern check_uint32, L_ARG, i
                    dasm_put(Dst, 1231, i);
                } else {
                    //| call_rr extern check_int32, L_ARG, i
                    dasm_put(Dst, 1179, i);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INTPTR_TYPE:
                //| call_rr extern check_uintptr, L_ARG, i
                dasm_put(Dst, 1244, i);
                add_pointer(Dst, ct, &reg);
                lua_pop(L, 1);
                break;

            case INT64_TYPE:
                if (mbr_ct->is_unsigned) {
                    //| call_rr extern check_uint64, L_ARG, i
                    dasm_put(Dst, 1257, i);
                } else {
                    //| call_rr extern check_int64, L_ARG, i
                    dasm_put(Dst, 1270, i);
                }
                add_int(Dst, ct, &reg, 1);
                lua_pop(L, 1);
                break;

            case DOUBLE_TYPE:
                //| call_rr extern check_double, L_ARG, i
                dasm_put(Dst, 1283, i);
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
                dasm_put(Dst, 1296, i);
                add_float(Dst, ct, &reg, 1);
                //| movq xmm0, xmm1
                dasm_put(Dst, 1309);
                add_float(Dst, ct, &reg, 1);
#else
                //| lea rax, [rsp+reg.off]
                //| sub rsp, 4
                //| call_rrr extern check_complex_double, rax, L_ARG, i
                dasm_put(Dst, 1315, reg.off, i);
                reg.off += 16;
#endif
                lua_pop(L, 1);
                break;

            case FLOAT_TYPE:
                //| call_rr extern check_double, L_ARG, i
                dasm_put(Dst, 1283, i);
                add_float(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case COMPLEX_FLOAT_TYPE:
#if defined _WIN64 || defined __amd64__
                //| call_rr extern check_complex_float, L_ARG, i
                dasm_put(Dst, 1341, i);
                /* complex floats are two floats packed into a double */
                add_float(Dst, ct, &reg, 1);
#else
                /* returned complex floats use eax and edx */
                //| call_rr extern check_complex_float, L_ARG, i
                //| mov [rsp], eax
                //| fld dword [rsp]
                dasm_put(Dst, 1354, i);
                add_float(Dst, ct, &reg, 0);
                //| mov [rsp], edx
                //| fld dword [rsp]
                dasm_put(Dst, 1373);
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
#ifdef _WIN64
        //|.if X64WIN
        if (reg.regs < MAX_REGISTERS(ct)) {
            assert(reg.regs == nargs);
            //| cmp TOP, MAX_REGISTERS(ct)
            //| jle >1
            //| // unpack onto stack
            //| mov rax, rsp
            //| add rax, 32 + 8*MAX_REGISTERS(ct)
            //| call_rrrr extern unpack_varargs_stack, L_ARG, MAX_REGISTERS(ct)+1, TOP, rax
            //| // unpack to registers
            //| mov rax, rsp
            //| add rax, 32 + 8*(reg.regs)
            //| call_rrrr extern unpack_varargs_reg, L_ARG, nargs+1, MAX_REGISTERS(ct), rax
            //| jmp >2
            //|1:
            //| // unpack just to registers
            //| mov rax, rsp
            //| add rax, 32 + 8*(reg.regs)
            //| call_rrrr extern unpack_varargs_reg, L_ARG, nargs+1, TOP, rax
            //|2:
        } else {
            //| // unpack just to stack
            //| mov rax, rsp
            //| add rax, reg.off
            //| call_rrrr extern unpack_varargs_stack, L_ARG, nargs+1, TOP, rax
        }

        for (i = nargs; i < MAX_REGISTERS(ct); i++) {
            reg.is_int[i] = reg.is_float[i] = 1;
        }
        reg.regs = MAX_REGISTERS(ct);
#elif defined __amd64__
        //|.elif X64
        if (reg.floats < MAX_FLOAT_REGISTERS(ct)) {
            //| mov rax, rsp
            //| add rax, 32 + 8*(MAX_INT_REGISTERS(ct) + reg.floats)
            //| call_rrrrr extern unpack_varargs_float, L_ARG, nargs+1, TOP, MAX_FLOAT_REGISTERS(ct) - reg.floats, rax
        }

        if (reg.ints < MAX_INT_REGISTERS(ct)) {
            //| mov rax, rsp
            //| add rax, 32 + 8*(reg.ints)
            //| call_rrrrr extern unpack_varargs_int, L_ARG, nargs+1, TOP, MAX_INT_REGISTERS(ct) - reg.ints, rax
        }

        //| mov rax, rsp
        //| add rax, reg.off
        //| call_rrrrrr extern unpack_varargs_stack_skip, L_ARG, nargs+1, TOP, MAX_INT_REGISTERS(ct) - reg.ints, MAX_FLOAT_REGISTERS(ct) - reg.floats, rax

        reg.floats = MAX_FLOAT_REGISTERS(ct);
        reg.ints = MAX_INT_REGISTERS(ct);
#else
        //|.else
        //| mov rax, rsp
        //| add rax, reg.off
        //| call_rrrr extern unpack_varargs_stack, L_ARG, nargs+1, TOP, rax
        //|.endif
        dasm_put(Dst, 1380, reg.off, nargs+1);
#endif
    }

    //| mov64 rcx, perr
    //| mov eax, dword [rcx]
    //| call_r extern SetLastError, rax
    dasm_put(Dst, 1406, perr);

    /* remove the stack space to call local functions */
    //|.if X32WIN
    //| add rsp, 28 // SetLastError will have already popped 4
    //|.else
    //| add rsp, 32
    //|.endif
    dasm_put(Dst, 1418);

#ifdef _WIN64
    //|.if X64WIN
    switch (reg.regs) {
    case 4:
        if (reg.is_float[3]) {
            //| movq xmm3, qword [rsp + 8*3]
        }
        if (reg.is_int[3]) {
            //| mov r9, [rsp + 8*3]
        }
    case 3:
        if (reg.is_float[2]) {
            //| movq xmm2, qword [rsp + 8*2]
        }
        if (reg.is_int[2]) {
            //| mov r8, [rsp + 8*2]
        }
    case 2:
        if (reg.is_float[1]) {
            //| movq xmm1, qword [rsp + 8*1]
        }
        if (reg.is_int[1]) {
            //| mov rdx, [rsp + 8*1]
        }
    case 1:
        if (reg.is_float[0]) {
            //| movq xmm0, qword [rsp]
        }
        if (reg.is_int[0]) {
            //| mov rcx, [rsp]
        }
    case 0:
        break;
    }

    /* don't remove the space for the registers as we need 32 bytes of register overflow space */
    assert(REGISTER_STACK_SPACE(ct) == 32);

#elif defined __amd64__
    //|.elif X64
    switch (reg.floats) {
    case 8:
        //| movq xmm7, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+7)]
    case 7:
        //| movq xmm6, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+6)]
    case 6:
        //| movq xmm5, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+5)]
    case 5:
        //| movq xmm4, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+4)]
    case 4:
        //| movq xmm3, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+3)]
    case 3:
        //| movq xmm2, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+2)]
    case 2:
        //| movq xmm1, qword [rsp + 8*(MAX_INT_REGISTERS(ct)+1)]
    case 1:
        //| movq xmm0, qword [rsp + 8*(MAX_INT_REGISTERS(ct))]
    case 0:
        break;
    }

    switch (reg.ints) {
    case 6:
        //| mov r9, [rsp + 8*5]
    case 5:
        //| mov r8, [rsp + 8*4]
    case 4:
        //| mov rcx, [rsp + 8*3]
    case 3:
        //| mov rdx, [rsp + 8*2]
    case 2:
        //| mov rsi, [rsp + 8*1]
    case 1:
        //| mov rdi, [rsp]
    case 0:
        break;
    }

    //| add rsp, REGISTER_STACK_SPACE(ct)
#else
    //|.else
    if (ct->calling_convention == FAST_CALL) {
        switch (reg.ints) {
        case 2:
            //| mov edx, [rsp + 4]
            dasm_put(Dst, 1422, 4);
        case 1:
            //| mov ecx, [rsp]
            dasm_put(Dst, 1428);
        case 0:
            break;
        }

        //| add rsp, REGISTER_STACK_SPACE(ct)
        dasm_put(Dst, 1432, REGISTER_STACK_SPACE(ct));
    }
    //|.endif
#endif

#ifdef __amd64__
    if (ct->has_var_arg) {
        /* al stores an upper limit on the number of float register, note that
         * its allowed to be more than the actual number of float registers used as
         * long as its 0-8 */
        //|.if X64 and not X64WIN
        //| mov al, 8
        //|.endif
    }
#endif

    //| call extern FUNCTION
    //| sub rsp, 48 // 32 to be able to call local functions, 16 so we can store some local variables
    dasm_put(Dst, 1436);

    /* note on windows X86 the stack may be only aligned to 4 (stdcall will
     * have popped a multiple of 4 bytes), but we don't need 16 byte alignment on
     * that platform
     */

    lua_rawgeti(L, ct_usr, 0);
    mbr_ct = (const struct ctype*) lua_touserdata(L, -1);

    if (mbr_ct->pointers || mbr_ct->is_reference || mbr_ct->type == INTPTR_TYPE) {
        lua_getuservalue(L, -1);
        num_upvals += 2;
        //| mov [rsp+32], rax // save the pointer
        //| get_errno
        //| call_rrp extern push_cdata, L_ARG, lua_upvalueindex(num_upvals), mbr_ct
        //| mov rcx, [rsp+32]
        //| mov [rax], rcx // *(void**) cdata = val
        //| lua_return_arg
        dasm_put(Dst, 1445, perr, mbr_ct, lua_upvalueindex(num_upvals));

    } else {
        switch (mbr_ct->type) {
        case FUNCTION_PTR_TYPE:
            lua_getuservalue(L, -1);
            num_upvals += 2;
            //| mov [rsp+32], rax // save the function pointer
            //| get_errno
            //| call_rrp extern push_cdata, L_ARG, lua_upvalueindex(num_upvals), mbr_ct
            //| mov rcx, [rsp+32]
            //| mov [rax], rcx // *(cfunction**) cdata = val
            //| lua_return_arg
            dasm_put(Dst, 1445, perr, mbr_ct, lua_upvalueindex(num_upvals));
            break;

        case INT64_TYPE:
#if LUA_VERSION_NUM >= 503
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                //| lua_return_ulong
                dasm_put(Dst, 1499, perr);
            } else {
                //| lua_return_long
                dasm_put(Dst, 1499, perr);
            }
#else
            num_upvals++;
            //| // save the return value
            //|.if X64
            //| mov [rsp+32], rax
            //|.else
            //| mov [rsp+36], edx // high
            //| mov [rsp+32], eax // low
            //|.endif
            //|
            //| get_errno
            //| call_rrp extern push_cdata, L_ARG, 0, mbr_ct
            //|
            //| // *(int64_t*) cdata = val
            //|.if X64
            //| mov rcx, [rsp+32]
            //| mov [rax], rcx
            //|.else
            //| mov rcx, [rsp+36]
            //| mov rdx, [rsp+32]
            //| mov [rax+4], rcx
            //| mov [rax], rdx
            //|.endif
            //|
            //| lua_return_arg
            dasm_put(Dst, 1545, perr, mbr_ct);
#endif
            break;

        case COMPLEX_FLOAT_TYPE:
            lua_getuservalue(L, -1);
            num_upvals += 2;
            //|.if X64
            //| // complex floats are returned as two floats packed into xmm0
            //| movq qword [rsp+32], xmm0
            //|.else
            //| // complex floats are returned as floats in eax and edx
            //| mov [rsp+32], eax
            //| mov [rsp+36], edx
            //|.endif
            //|
            //| get_errno
            //| call_rrp extern push_cdata, L_ARG, lua_upvalueindex(num_upvals), mbr_ct
            //|
            //| // ((complex_float*) cdata) = val
            //|.if X64
            //| mov rcx, [rsp+32]
            //| mov [rax], rcx
            //|.else
            //| mov ecx, [rsp+32]
            //| mov [rax], ecx
            //| mov ecx, [rsp+36]
            //| mov [rax+4], ecx
            //|.endif
            //|
            //| lua_return_arg
            dasm_put(Dst, 1613, perr, mbr_ct, lua_upvalueindex(num_upvals));
            break;

        case COMPLEX_DOUBLE_TYPE:
            lua_getuservalue(L, -1);
            num_upvals += 2;
            //|.if X64
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
            //|.else
            //| // On 32 bit we have already handled this by pushing a new cdata
            //| // and handing the cdata ptr in as the hidden first param, but
            //| // still need to add mbr_ct as an upval as its used earlier.
            //| // Hidden param was popped by called function, we need to realign.
            //| sub rsp, 4
            //| get_errno
            //|.endif
            //|
            //| lua_return_arg
            dasm_put(Dst, 1678, perr);
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            //| lua_return_void
            dasm_put(Dst, 1709, perr);
            break;

        case BOOL_TYPE:
            lua_pop(L, 1);
            //| lua_return_bool
            dasm_put(Dst, 1736, perr);
            break;

        case INT8_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                //| movzx eax, al
                dasm_put(Dst, 1192);
            } else {
                //| movsx eax, al
                dasm_put(Dst, 1196);
            }
            //| lua_return_int
            dasm_put(Dst, 1785, perr);
            break;

        case INT16_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                //| movzx eax, ax
                dasm_put(Dst, 1200);
            } else {
                //| movsx eax, ax
                dasm_put(Dst, 1204);
            }
            //| lua_return_int
            dasm_put(Dst, 1785, perr);
            break;

        case INT32_TYPE:
        case ENUM_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                //| lua_return_uint
                dasm_put(Dst, 1831, perr);
            } else {
                //| lua_return_int
                dasm_put(Dst, 1785, perr);
            }
            break;

        case FLOAT_TYPE:
            lua_pop(L, 1);
            //|.if X64
            //| cvtss2sd xmm0, xmm0
            //|.endif
            //| lua_return_double
            dasm_put(Dst, 1877, perr);
            break;

        case DOUBLE_TYPE:
            lua_pop(L, 1);
            //| lua_return_double
            dasm_put(Dst, 1877, perr);
            break;

        default:
            luaL_error(L, "NYI: call return type");
        }
    }

    assert(lua_gettop(L) == top + num_upvals);
    {
        cfunction f = compile(Dst, L, func, LUA_NOREF);
        /* add a callback as an upval so that the jitted code gets cleaned up when
         * the function gets gc'd */
        push_callback(L, f, func);
        lua_pushcclosure(L, (lua_CFunction) f, num_upvals+1);
    }
}

