/* vim: ts=4 sw=4 sts=4 et tw=78
 * Portions copyright (c) 2015-present, Facebook, Inc. All rights reserved.
 * Portions copyright (c) 2011 James R. McKaskill.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
# include <lua.h>
# include <lauxlib.h>
# include <lualib.h>
}
# define EXTERN_C extern "C"
#else
# include <lua.h>
# include <lauxlib.h>
# include <lualib.h>
# define EXTERN_C extern
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#endif

#if __STDC_VERSION__+0 >= 199901L

#if ( defined( _WIN32) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__bsdi__) || defined(__DragonFly__))
/* We should include something equivalent to */
/* complex.h                                 */
/* e.g. Mingw GCC >=10 should have complex functions builtin */
/* and we can check them with the macro __has_include */
#if (__GNUC__ >= 10)
# if defined __has_include
#  if __has_include (<complex.h>)
#    include <complex.h>
#    define HAVE_COMPLEX
#    define HAVE_LONG_DOUBLE
#  endif
# endif
#endif 
#else
#include <complex.h>
#define HAVE_COMPLEX
#define HAVE_LONG_DOUBLE
#endif
#endif


#ifndef NDEBUG
#define DASM_CHECKS
#endif

struct jit;
#define Dst_DECL	struct jit* Dst
#define Dst_REF		(Dst->ctx)
#define DASM_EXTERN(a,b,c,d) get_extern(a,b,c,d)

#include "dynasm/dasm_proto.h"

#if defined LUA_FFI_BUILD_AS_DLL
# define EXPORT __declspec(dllexport)
#elif defined __GNUC__
# define EXPORT __attribute__((visibility("default")))
#else
# define EXPORT
#endif
#ifdef __ANDROID_API__
#include <android/log.h>
#define LOGE(msg, ...) __android_log_print(ANDROID_LOG_ERROR,"FFI",msg,##__VA_ARGS__)
#define LOGV(msg, ...) __android_log_print(ANDROID_LOG_VERBOSE,"FFI",msg,##__VA_ARGS__)
#define LOGI(msg, ...) __android_log_print(ANDROID_LOG_INFO,"FFI",msg,##__VA_ARGS__)
#define LOGW(msg, ...) __android_log_print(ANDROID_LOG_WARN,"FFI",msg,##__VA_ARGS__)
#endif
/* architectures */
#if defined _WIN32 && defined UNDER_CE
# define OS_CE
#elif defined _WIN32
# define OS_WIN
#elif defined __APPLE__ && defined __MACH__
# define OS_OSX
#elif defined __ANDROID_API__
#define OS_ANDROID
#elif defined __linux__
# define OS_LINUX
#elif defined __FreeBSD__ || defined __OpenBSD__ || defined __NetBSD__
# define OS_BSD
#elif defined unix || defined __unix__ || defined __unix || defined _POSIX_VERSION || defined _XOPEN_VERSION
# define OS_POSIX
#endif

/* architecture */
#if defined __i386__ || defined _M_IX86
# define ARCH_X86
#elif defined __amd64__ || defined _M_X64
# define ARCH_X64
#elif defined __arm__ || defined __ARM__ || defined ARM || defined __ARM || defined __arm
# define ARCH_ARM
#elif defined __aarch64__
# define ARCH_ARM64
#elif defined __powerpc64__
# define ARCH_PPC64
#else
# define UNSUPPORTED_ARCH /*error*/
#endif

/* See ffi.c: replace luaopen_ffi with a stub    */
/* if FFI_ENABLE_LUATEX_INTERFACE is not defined */
#if (defined ARCH_X86 || defined ARCH_X64) && (defined OS_CE || defined OS_WIN || defined OS_LINUX || defined OS_BSD || defined OS_POSIX || defined OS_OSX) || ( defined ARCH_ARM64 && (defined OS_LINUX ||defined OS_POSIX ))
#define FFI_ENABLE_LUATEX_INTERFACE
#endif 
/* for the moment */
#if (defined __CYGWIN__)
#undef FFI_ENABLE_LUATEX_INTERFACE
#endif 


#if defined ARCH_X86 || defined ARCH_X64
#define ALLOW_MISALIGNED_ACCESS
#endif

/* #ifdef OS_WIN */
/* #define ALWAYS_INLINE __forceinline */
/* #elif defined(__GNUC__) */
/* #define ALWAYS_INLINE __attribute__ ((always_inline)) */
/* #else */
/* #define ALWAYS_INLINE inline */
/* #endif */

#define ALWAYS_INLINE inline


EXTERN_C EXPORT int luaopen_ffi(lua_State* L);

static int lua_absindex2(lua_State* L, int idx) {
    return (LUA_REGISTRYINDEX <= idx && idx < 0)
         ? lua_gettop(L) + idx + 1
         : idx;
}
/* use our own version of lua_absindex such that lua_absindex(L, 0) == 0 */
#define lua_absindex(L, idx) lua_absindex2(L, idx)
#if LUA_VERSION_NUM>501
static void (lua_call)(lua_State *L, int nargs, int nresults)
{
    lua_call(L, nargs, nresults);
}
#endif

#if LUA_VERSION_NUM == 501

/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
static void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  for (; l && l->name; l++) {  /* fill the table with given functions */
    int i;
    for (i = 0; i < nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -nup);
    lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_pop(L, nup);  /* remove upvalues */
}
#define lua_setuservalue lua_setfenv
#define lua_getuservalue lua_getfenv
#define lua_rawlen lua_objlen
static char* luaL_prepbuffsize(luaL_Buffer* B, size_t sz) {
    if (sz > LUAL_BUFFERSIZE) {
        luaL_error(B->L, "string too long");
    }
    return luaL_prepbuffer(B);
}

static ALWAYS_INLINE void lua_rawgetp(lua_State *L, int idx,void* p){
    lua_pushlightuserdata(L, p);
    lua_rawget(L, idx);
}

#elif LUA_VERSION_NUM >= 503
static void (lua_remove)(lua_State *L, int idx) {
    lua_remove(L, idx);
}
#endif

#ifdef _WIN32

#   ifdef UNDER_CE
        static void* DoLoadLibraryA(const char* name) {
          wchar_t buf[MAX_PATH];
          int sz = MultiByteToWideChar(CP_UTF8, 0, name, -1, buf, 512);
          if (sz > 0) {
            buf[sz] = 0;
            return LoadLibraryW(buf);
          } else {
            return NULL;
          }
        }
#       define LoadLibraryA DoLoadLibraryA
#   else
#       define GetProcAddressA GetProcAddress
#   endif

#   define LIB_FORMAT_1 "%s.dll"
#   define AllocPage(size) VirtualAlloc(NULL, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE)
#   define FreePage(data, size) VirtualFree(data, 0, MEM_RELEASE)
#   define EnableExecute(data, size) do {DWORD old; VirtualProtect(data, size, PAGE_EXECUTE, &old); FlushInstructionCache(GetCurrentProcess(), data, size);} while (0)
#   define EnableWrite(data, size) do {DWORD old; VirtualProtect(data, size, PAGE_READWRITE, &old);} while (0)

#else
#ifdef OS_OSX
#   define LIB_FORMAT_1 "%s.dylib"
#   define LIB_FORMAT_2 "lib%s.dylib"
#else
#   define LIB_FORMAT_1 "%s.so"
#   define LIB_FORMAT_2 "lib%s.so"
#endif
#if defined (__GNUC__)
#define GCC_VERSION ((__GNUC__*10000) + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__)
#endif
//Only arm need cache flush
#if defined(OS_ANDROID)|| (defined(__GNUC__) && GCC_VERSION>=40300)
#define CacheFlush(st,size)  __builtin___clear_cache((char*)(st),(size)+(char*)(st))
#elif defined(ARCH_ARM64)||defined(ARCH_ARM)
#if defined(OS_OSX)
 void sys_icache_invalidate(void *start, size_t len);
 #define CacheFlush(st,size) sys_icache_invalidate((char*)(st),size)
#else
  #define CacheFlush(st,size) __clear_cache((char*)(st),(size)+(char*)(st))
# endif
#else
#define CacheFlush(st,size)
#endif
#   define LoadLibraryA(name) dlopen(name, RTLD_LAZY | RTLD_GLOBAL)
#   define GetProcAddressA(lib, name) dlsym(lib, name)
#   define FreeLibrary(lib) dlclose(lib)
#   define AllocPage(size) mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0)
#   define FreePage(data, size) munmap(data, size)
#   define EnableExecute(data, size) do{mprotect(data, size, PROT_READ|PROT_EXEC);CacheFlush(data,size);}while(0)
#   define EnableWrite(data, size) mprotect(data, size, PROT_READ|PROT_WRITE)
#endif



// Since arm jump range is always small, so bl extern is hardly compiled
// as an direct offset to the destination. Unlike global extern functions which
//will be re-use many times, FUNCTION stub is only use one time, therefore,
// no need to keep it for arm
#if defined(ARCH_ARM64)|| defined(ARCH_ARM)
#define NO_FUNCTION_EXTERN
#endif

#ifndef bool
#define bool _Bool
#endif

struct token;

struct parser {
    int line;
    const char* next;
    const char* prev;
    unsigned align_mask;
};

struct page {
    size_t size;
    size_t off;
    size_t freed;
};

struct jit {
    lua_State* L;
    //int32_t last_errno;
    dasm_State* ctx;
    size_t pagenum;
#ifndef NO_FUNCTION_EXTERN
    int function_extern;
#endif
    struct page** pages;
    size_t align_page_size;
    void** globals;
    uint8_t* default_functions;
    void* lua_dll;
    //void* kernel32_dll;
};

#define ALIGN_DOWN(PTR, MASK) \
  (((uintptr_t) (PTR)) & (~ ((uintptr_t) (MASK)) ))
#define ALIGN_UP(PTR, MASK) \
  (( ((uintptr_t) (PTR)) + ((uintptr_t) (MASK)) ) & (~ ((uintptr_t) (MASK)) ))

/* struct cdata/struct ctype */

#define PTR_ALIGN_MASK (sizeof(void*) - 1)
#define FUNCTION_ALIGN_MASK (sizeof(void (*)()) - 1)
#define DEFAULT_ALIGN_MASK 7

#ifdef OS_OSX
/* TODO: figure out why the alignof trick doesn't work on OS X */
#define ALIGNED_DEFAULT 7
#elif defined __GNUC__
#define ALIGNED_DEFAULT (__alignof__(void*) - 1)
#else
#define ALIGNED_DEFAULT PTR_ALIGN_MASK
#endif

#ifdef ARCH_ARM
#define ALIGNED_MAX 7
#else//for arm64 x86 x64
#define ALIGNED_MAX 15
#endif


extern int jit_key;
extern int ctype_mt_key;
extern int cdata_mt_key;
extern int cmodule_mt_key;
extern int callback_mt_key;
extern int constants_key;
extern int types_key;
extern int gc_key;
extern int callbacks_key;
extern int functions_key;
extern int abi_key;
extern int next_unnamed_key;
extern int niluv_key;
extern int asmname_key;

int equals_upval(lua_State* L, int idx, int* key);
void push_upval(lua_State* L, int* key);
void set_upval(lua_State* L, int* key);
struct jit* get_jit(lua_State* L);

/* both ctype and cdata are stored as userdatas
 *
 * usr value is a table shared between the related subtypes which has:
 * name -> member ctype (for structs and unions)
 * +ves -> member ctype - in memory order (for structs)
 * +ves -> argument ctype (for function prototypes)
 * 0 -> return ctype (for function prototypes)
 * light userdata -> misc
 */

enum {
    C_CALL,
    STD_CALL,
    FAST_CALL,
};

enum {
    INVALID_TYPE,
    VOID_TYPE,
    FLOAT_TYPE,
    DOUBLE_TYPE,
    LONG_DOUBLE_TYPE,
    COMPLEX_FLOAT_TYPE,
    COMPLEX_DOUBLE_TYPE,
    COMPLEX_LONG_DOUBLE_TYPE,
    BOOL_TYPE,
    INT8_TYPE,
    INT16_TYPE,
    INT32_TYPE,
    INT64_TYPE,
    INTPTR_TYPE,
    ENUM_TYPE,
    UNION_TYPE,
    STRUCT_TYPE,
    FUNCTION_TYPE,
    FUNCTION_PTR_TYPE,
};

#define IS_CHAR_UNSIGNED (((char) -1) > 0)
#define IS_COMPLEX(type) ((type) == COMPLEX_FLOAT_TYPE || (type) == COMPLEX_DOUBLE_TYPE)

#define POINTER_BITS 2
#define POINTER_MAX ((1 << POINTER_BITS) - 1)

#define ALIGNOF(S) ((int) ((char*) &S.v - (char*) &S - 1))

/* Note: if adding a new member that is associated with a struct/union
 * definition then it needs to be copied over in ctype.c:set_defined for when
 * we create types based off of the declaration alone.
 *
 * Since this is used as a header for every ctype and cdata, and we create a
 * ton of them on the stack, we try and minimise its size.
 */
struct ctype {
    size_t base_size; /* size of the base type in bytes */

    union {
        /* valid if is_bitfield */
        struct {
            /* size of bitfield in bits */
            unsigned bit_size : 7;
            /* offset within the current byte between 0-63 */
            unsigned bit_offset : 6;
        };
        /* Valid if is_array */
        size_t array_size;
        /* Valid for is_variable_struct or is_variable_array. If
         * variable_size_known (only used for is_variable_struct) then this is
         * the total increment otherwise this is the per element increment.
         */
        size_t variable_increment;
    };
    unsigned align_mask : 4; /* as (align bytes - 1) eg 7 gives 8 byte alignment */
    unsigned pointers : POINTER_BITS; /* number of dereferences to get to the base type including +1 for arrays */
    unsigned is_empty :1;
    unsigned const_mask : POINTER_MAX + 1; /* const pointer mask, LSB is current pointer, +1 for the whether the base type is const */
    unsigned type : 5; /* value given by type enum above */
    unsigned is_reference : 1;
    unsigned is_array : 1;
    unsigned is_defined : 1;
    unsigned is_null : 1;
    unsigned calling_convention : 2;
    unsigned has_var_arg : 1;
    unsigned is_variable_array : 1; /* set for variable array types where we don't know the variable size yet */
    unsigned is_variable_struct : 1;
    unsigned variable_size_known : 1; /* used for variable structs after we know the variable size */
    unsigned is_bitfield : 1;
    unsigned has_bitfield : 1;
    unsigned is_jitted : 1;
    unsigned is_packed : 1;
    unsigned is_unsigned : 1;
};

struct member_type{
    struct ctype ct;
    unsigned has_member_name : 1;
    size_t offset;
};
#ifdef _MSC_VER
__declspec(align(16))
#endif
struct cdata {
    const struct ctype type
/* #ifdef __GNUC__ */
/*       __attribute__ ((aligned(16))) */
/* #endif */
      ;
};

typedef void (*cfunction)(void);

#ifdef HAVE_COMPLEX
#if defined(_MSC_VER)
typedef _Dcomplex complex_double;
typedef _Fcomplex complex_float;
static ALWAYS_INLINE complex_double mk_complex_double(double real, double imag) {
      return (complex_double){real , imag };
}
static ALWAYS_INLINE complex_float mk_complex_float(double real, double imag) {
      return (complex_float){real , imag};
}
#else
typedef double complex complex_double;
typedef float complex complex_float;
static ALWAYS_INLINE complex_double mk_complex_double(double real, double imag) {
  return real + imag * 1i;
}
static ALWAYS_INLINE complex_float mk_complex_float(double real, double imag) {
    return real + imag * 1i;
}
#endif

/* AlpineLinux musl doesn't like them */
/* extern float cimagf(complex_float); */
/* extern float crealf(complex_float); */
/* extern double cimag(complex_double); */
/* extern double creal(complex_double); */


#if defined(OS_ANDROID) && __ANDROID_API__<26
#include <math.h>

#define cabs(f) ((double (*)(complex_double))hypot)(f);

static complex_double cpow(complex_double f,complex_double s){
    if(cimag(s)==0){
        double real = creal(s);
        double r=cabs(f);
        r = pow(r, real);
        double theta = atan2(creal(f), cimag(f)) * real;
        return mk_complex_double(r*cos(theta),r*sin(theta));
    }
    double r= cabs(f);
    double theta = atan2(creal(f), cimag(f));
    r = log(r);
    f= s*mk_complex_double(r,theta);
    r = exp(creal(f));
    theta=cimag(f);
    return  mk_complex_double(r*cos(theta),r*sin(theta));
}
#endif
#else
typedef struct {
    double real, imag;
} complex_double;

typedef struct {
    float real, imag;
} complex_float;

static ALWAYS_INLINE complex_double mk_complex_double(double real, double imag) {
    return (complex_double){ real, imag };
}
static ALWAYS_INLINE complex_float mk_complex_float(double real, double imag) {
    return (complex_float){ real, imag };
}

static ALWAYS_INLINE double creal(complex_double c) {
    return c.real;
}
static ALWAYS_INLINE float crealf(complex_float c) {
    return c.real;
}

static ALWAYS_INLINE double cimag(complex_double c) {
    return c.imag;
}
static ALWAYS_INLINE float cimagf(complex_float c) {
    return c.imag;
}

#include "math.h"
static complex_double cpow(complex_double f, complex_double s) {
    if (s.imag == 0) {
        double real = s.real;
        double r = hypot(f.real, f.imag);
        r = pow(r, real);
        double theta = atan2(f.imag, f.real) * real;
        return (complex_double){ r*cos(theta),r*sin(theta) };
    }
    double r= hypot(f.real, f.imag);
    double theta = atan2(f.imag, f.real);
    r = log(r);
    f = (complex_double) { r * s.real - theta * s.imag ,r * s.imag + s.real*theta};
    r = exp(f.real);
    return (complex_double) {r*cos(f.imag),r*sin(f.imag)};
}

#endif

#define CALLBACK_FUNC_USR_IDX 1

void set_defined(lua_State* L, int ct_usr, struct ctype* ct);
struct ctype* push_ctype(lua_State* L, int ct_usr, const struct ctype* ct);
struct member_type* push_member_type(lua_State* L, int ct_usr, const struct member_type* mt);
void* push_cdata(lua_State* L, int ct_usr, const struct ctype* ct); /* called from asm */
void push_callback(lua_State* L, cfunction luafunc, cfunction cfunc);
void check_ctype(lua_State* L, int idx, struct ctype* ct);
void* to_cdata(lua_State* L, int idx, struct ctype* ct);
const struct ctype* get_ctype(lua_State* L, int idx);
void* check_cdata(lua_State* L, int idx, struct ctype* ct);
size_t ctype_size(lua_State* L, const struct ctype* ct);

int parse_type(lua_State* L, struct parser* P, struct ctype* type);
void parse_argument(lua_State *L, struct parser *P, int ct_usr, struct ctype *type,
                    struct token *name,
                    struct parser *asmname, bool ignore_name);
void push_type_name(lua_State* L, int usr, const struct ctype* ct);

int push_user_mt(lua_State* L, int ct_usr, const struct ctype* ct);

int ffi_cdef(lua_State* L);

void push_func_ref(lua_State* L, cfunction func);
void free_code(struct jit* jit, lua_State* L, cfunction func);
int x86_return_size(lua_State* L, int usr, const struct ctype* ct);
void compile_function(lua_State* L, cfunction f, int ct_usr, const struct ctype* ct);
cfunction compile_callback(lua_State* L, int fidx, int ct_usr, const struct ctype* ct);
void compile_globals(struct jit* jit, lua_State* L);
int get_extern(struct jit* jit, uint8_t* addr, int idx, int type);

/* WARNING: assembly needs to be updated for prototype changes of these functions */
double check_double(lua_State* L, int idx);
float check_float(lua_State* L, int idx);
uint64_t check_uint64(lua_State* L, int idx);
int64_t check_int64(lua_State* L, int idx);
int32_t check_int32(lua_State* L, int idx);
uint32_t check_uint32(lua_State* L, int idx);
uintptr_t check_uintptr(lua_State* L, int idx);
int32_t check_enum(lua_State* L, int idx, int to_usr, const struct ctype* tt);
void* check_struct(lua_State*L,int idx, int to_usr,const struct ctype* ct);
/* these two will always push a value so that we can create structs/functions on the fly */
void* check_typed_pointer(lua_State* L, int idx, int to_usr, const struct ctype* tt);
cfunction check_typed_cfunction(lua_State* L, int idx, int to_usr, const struct ctype* tt);
complex_double check_complex_double(lua_State* L, int idx);
complex_float check_complex_float(lua_State* L, int idx);

void unpack_varargs_stack(lua_State* L, int first, int last, char* to);
void unpack_varargs_reg(lua_State* L, int first, int last, char* to);

void unpack_varargs_stack_skip(lua_State* L, int first, int last, int ints_to_skip, int floats_to_skip, char* to);
void unpack_varargs_float(lua_State* L, int first, int last, int max, char* to);
void unpack_varargs_int(lua_State* L, int first, int last, int max, char* to);

int unpack_varargs_bound(lua_State* L, int first, char* to,char* end);
