/* vim: ts=4 sw=4 sts=4 et tw=78
 * Portions copyright (c) 2015-present, Facebook, Inc. All rights reserved.
 * Portions copyright (c) 2011 James R. McKaskill.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
#include "ffi.h"

#include <unistd.h>

static cfunction compile(Dst_DECL, lua_State* L, cfunction func, int ref);

static void* reserve_code(struct jit* jit, lua_State* L, size_t sz);
static void commit_code(struct jit* jit, void* p, size_t sz);

static void debug_int(int f)
{
    printf("Here from debugger int = [%d]\n", f);
    return;
}

static void debug_double(double f)
{
    printf("Here from debugger float = [%lf]\n", f);
    return;
}

static void debug_float(float f)
{
    printf("Here from debugger float = [%f]\n", f);
    return;
}

static void debug_statement()
{
    printf("Here from debugger\n");
    return;
}

static void debug_statement_with_arg(void * p)
{
    printf("Here from debugger inp = [%p]\n", p);
    return;
}

static void push_int(lua_State* L, int val)
{ lua_pushinteger(L, val); }

static void push_uint(lua_State* L, unsigned int val)
{ lua_pushinteger(L, val); }

static void push_float(lua_State* L, float val)
{ lua_pushnumber(L, val); }

#ifndef _WIN32
static int GetLastError(void)
{ return errno; }
static void SetLastError(int err)
{ errno = err; }
#endif

#ifdef NDEBUG
#define shred(a,b,c)
#else
#define shred(p,s,e) memset((uint8_t*)(p)+(s),0xCC,(e)-(s))
#endif


#ifdef _WIN64
#include "dynasm/dasm_x86.h"
#include "call_x64win.h"
#elif defined __amd64__
#include "dynasm/dasm_x86.h"
#include "call_x64.h"
#elif defined __arm__ || defined __arm || defined __ARM__ || defined __ARM || defined ARM || defined _ARM_ || defined ARMV4I || defined _M_ARM
#include "dynasm/dasm_arm.h"
#include "call_arm.h"
#elif defined __aarch64__
#include "dynasm/dasm_arm64.h"
#include "call_arm64.h"
#else
#include "dynasm/dasm_x86.h"
#include "call_x86.h"
#endif

#ifndef _MSC_VER
struct __attribute__((__packed__)) jit_head {
    size_t size;
    int ref;
    uint8_t padding[4];
    uint8_t jump[JUMP_SIZE];
};
#else
#pragma pack(push,1)
struct jit_head {
    size_t size;
    int ref;
    uint8_t padding[4];
    uint8_t jump[JUMP_SIZE];
};
#pragma pack(pop)
#endif
#define LINKTABLE_MAX_SIZE (sizeof(extnames) / sizeof(extnames[0]) * (JUMP_SIZE))
#define LINKTABLE_MAX_NUM (sizeof(extnames) / sizeof(extnames[0]))

static void dump_externs(struct jit* jit)
{

#ifdef ARCH_ARM64
    struct page* page = jit->pages[jit->pagenum-1];
    unsigned int * ip = (unsigned int*)(page+1);
    printf("Offset = [%ld]\n", page->off);
    printf("size = [%ld]\n", page->off);
    for (int i = 0, func_ind = 0; (i < LINKTABLE_MAX_NUM-1) ; i++, func_ind++) {
        printf("holder = [%p] func address = [%p]\n", ((char*)ip), *(cfunction*)((char*)ip));
        ip += 2;
        printf("[%p]:ext(%d) = [%0X]\n", (ip), func_ind, *(ip));
        printf("[%p]:ext(%d) = [%0X]\n", (ip+1), func_ind, *(ip+1));
        printf("[%p]:ext(%d) = [%0X]\n", (ip+2), func_ind, *(ip+2));
        ip+=4;
    }
#endif
    return;
}

static void dump_code(struct jit* jit, char * ccode)
{
#ifdef ARCH_ARM64
    struct page* page = jit->pages[jit->pagenum-1];
    unsigned char * code = (unsigned char*)(page+1);
	printf("JUMP_SIZE = [%zu] LINKTABLE_ELEMENT_SIZE=[%zu], LINKTABLE_MAX_NUM=[%ld]\n",
			(size_t)JUMP_SIZE, LINKTABLE_MAX_SIZE/LINKTABLE_MAX_NUM, LINKTABLE_MAX_NUM);
	/* The array extnames has (const char *)0 as the last element, which will not 
	 * have an entry in code. Thus we have to traverse through LINKTABLE_MAX_NUM -1
	 * elements or LINKTABLE_MAX_SIZE - JUMP_SIZE bytes to skip the link table */
    code += LINKTABLE_MAX_SIZE-JUMP_SIZE;
    struct jit_head *jh1 = (struct jit_head *)code; /* This is the jit_head from compile_globals */
    code = (unsigned char*)((struct jit_head *)code+1); /* This is the jit_head from compile_globals */
	/* Incidentally the generated code was less than the available size on first page, and thus
	 * we did not have to adjust for 2nd page onwards */
    struct jit_head *jh2 = (struct jit_head *)code;
    code = (unsigned char*)((struct jit_head *)code+1);
    printf("jh_size = %zu  jh->size = %zu\n", sizeof(struct jit_head), jh2->size);
    printf("jh->ref = %d\n", jh2->ref);
    printf("jh->func = %p content = %p\n", (void*)jh2->jump, *(cfunction*)((void*)jh2->jump));
    printf("jmpadr = [%p]\n", (void*)(*(uint64_t *)((char*)(jh2->jump)+0)));
    printf("ins[1] = [%X]\n", *(unsigned int *)((char*)(jh2->jump)+8));
    printf("ins[2] = [%X]\n", *(unsigned int *)((char*)(jh2->jump)+12));
    printf("ins[3] = [%X]\n", *(unsigned int *)((char*)(jh2->jump)+16));
    //printf("computed code = [%p]\n", code); CODE STARTS HERE IN THE PAGE
    unsigned int * ip = (unsigned int*)ccode;
    for (int i = 0; (char*)&ip[i] < ((char*)page + page->off); i++) {
        printf("[%p]:ins(%02d) = [%0X]\n", &(ip[i]), i, ip[i]);
    }
#endif
    return;
}

static cfunction compile(struct jit* jit, lua_State* L, cfunction func, int ref)
{
    struct jit_head* code;
    size_t codesz;
    int err;

    if ((err = dasm_checkstep(jit, -1)) !=0 ) {
        char buf[32];
        sprintf(buf, "%x", err);
        printf("%s:%d\n", __FILE__, __LINE__);
        luaL_error(L, "dasm_link error %s", buf);
    }
    if ((err = dasm_link(jit, &codesz)) != 0) {
        char buf[32];
        sprintf(buf, "%x", err);
        printf("%s:%d\n", __FILE__, __LINE__);
        luaL_error(L, "dasm_link error %s", buf);
    }

    codesz += sizeof(struct jit_head);
    code = (struct jit_head*) reserve_code(jit, L, codesz);
	//printf("%s:%d reserved code = [%p] jump=[%p] size=[%zu]\n", __FILE__, __LINE__, code, code->jump, codesz);
    code->ref = ref;
    code->size = codesz;
    compile_extern_jump(jit, L, func, code->jump);

    if ((err = dasm_encode(jit, code+1)) != 0) {
        char buf[32];
        sprintf(buf, "%x", err);
        commit_code(jit, code, 0);
        luaL_error(L, "dasm_encode error %s", buf);
    }

    commit_code(jit, code, codesz);
	//sys_icache_invalidate(code, codesz);
    //dump_externs(jit);
    //dump_code(jit, (char*)(code+1));
    return (cfunction) (code+1);
}

typedef uint8_t jump_t[JUMP_SIZE];

int get_extern(struct jit* jit, uint8_t* addr, int idx, int type)
{
    struct page* page = jit->pages[jit->pagenum-1];
    jump_t* jumps = (jump_t*) (page+1);
    struct jit_head* h = (struct jit_head*) ((uint8_t*) page + page->off);
    uint8_t* jmp;
    ptrdiff_t off;

    if (idx == jit->function_extern) {
       jmp = h->jump;
    } else {
       jmp = jumps[idx];
    }

	if (type) {
		/* compensate for room taken up for the offset so that we can work rip
		 * relative */
		addr += BRANCH_OFF;

		/* see if we can fit the offset in the branch displacement, if not use the
		 * jump instruction placed via compile_extern_jump */
		off = *(uint8_t**) jmp - addr;

		if (MIN_BRANCH <= off && off <= MAX_BRANCH) {
			//printf("%s:%d offset = [%x]\n", __FILE__, __LINE__, (int32_t) off);
			return (int32_t) off;
		} else {
			//printf("%s:%d offset = [%x]\n", __FILE__, __LINE__, (int32_t)(jmp + sizeof(uint8_t*) - addr));
			return (int32_t)(jmp + sizeof(uint8_t*) - addr);
		}
	}
	else {
		printf("%s:%d THIS CONDITION IS NOT PROVIDED FOR\n", __FILE__, __LINE__);
		return (uint32_t) (uintptr_t)jmp;
	}
}

static void* reserve_code(struct jit* jit, lua_State* L, size_t sz)
{
    struct page* page;
    size_t off = (jit->pagenum > 0) ? jit->pages[jit->pagenum-1]->off : 0;
    size_t size = (jit->pagenum > 0) ? jit->pages[jit->pagenum-1]->size : 0;

    if (off + sz >= size) {
        int i;
        uint8_t* pdata;
        cfunction func;

        /* need to create a new page */
        jit->pages = (struct page**) realloc(jit->pages, (++jit->pagenum) * sizeof(jit->pages[0]));

        size = ALIGN_UP(sz + LINKTABLE_MAX_SIZE + sizeof(struct page), jit->align_page_size);

        page = (struct page*) AllocPage(size);
        jit->pages[jit->pagenum-1] = page;
        pdata = (uint8_t*) page;
        page->size = size;
        page->off = sizeof(struct page);

        lua_newtable(L);

#define ADDFUNC(DLL, NAME) \
        lua_pushliteral(L, #NAME); \
        func = DLL ? (cfunction) GetProcAddressA(DLL, #NAME) : NULL; \
        func = func ? func : (cfunction) &NAME; \
        lua_pushcfunction(L, (lua_CFunction) func); \
        lua_rawset(L, -3)

        ADDFUNC(NULL, check_double);
        ADDFUNC(NULL, check_float);
        ADDFUNC(NULL, check_uint64);
        ADDFUNC(NULL, check_int64);
        //ADDFUNC(NULL, check_int16);
        //ADDFUNC(NULL, check_uint16);
        ADDFUNC(NULL, check_int32);
        ADDFUNC(NULL, check_uint32);
        //ADDFUNC(NULL, check_int8);
        //ADDFUNC(NULL, check_uint8);
        ADDFUNC(NULL, check_uintptr);
        ADDFUNC(NULL, check_enum);
        ADDFUNC(NULL, check_typed_pointer);
        ADDFUNC(NULL, check_typed_cfunction);
        ADDFUNC(NULL, check_complex_double);
        ADDFUNC(NULL, check_complex_float);
        ADDFUNC(NULL, unpack_varargs_stack);
        ADDFUNC(NULL, unpack_varargs_stack_skip);
        ADDFUNC(NULL, unpack_varargs_reg);
        ADDFUNC(NULL, unpack_varargs_float);
        ADDFUNC(NULL, unpack_varargs_int);
        ADDFUNC(NULL, push_cdata);
        ADDFUNC(NULL, push_int);
        ADDFUNC(NULL, push_uint);
        ADDFUNC(NULL, lua_pushinteger);
        ADDFUNC(NULL, push_float);
        ADDFUNC(NULL, debug_int);
        ADDFUNC(NULL, debug_float);
        ADDFUNC(NULL, debug_double);
        ADDFUNC(NULL, debug_statement);
        ADDFUNC(NULL, debug_statement_with_arg);
        ADDFUNC(jit->kernel32_dll, SetLastError);
        ADDFUNC(jit->kernel32_dll, GetLastError);
        ADDFUNC(jit->lua_dll, luaL_error);
        ADDFUNC(jit->lua_dll, lua_pushnumber);
        ADDFUNC(jit->lua_dll, lua_pushboolean);
        ADDFUNC(jit->lua_dll, lua_gettop);
        ADDFUNC(jit->lua_dll, lua_rawgeti);
        ADDFUNC(jit->lua_dll, lua_pushnil);
        ADDFUNC(jit->lua_dll, lua_callk);
        ADDFUNC(jit->lua_dll, lua_settop);
        ADDFUNC(jit->lua_dll, lua_remove);
#undef ADDFUNC

        for (i = 0; extnames[i] != NULL; i++) {

            if (strcmp(extnames[i], "FUNCTION") == 0) {
                shred(pdata + page->off, 0, JUMP_SIZE);
                jit->function_extern = i;

            } else {
                lua_getfield(L, -1, extnames[i]);
                func = (cfunction) lua_tocfunction(L, -1);

                if (func == NULL) {
                    luaL_error(L, "internal error: missing link for %s", extnames[i]);
                }

                compile_extern_jump(jit, L, func, pdata + page->off);
                lua_pop(L, 1);
            }

            page->off += JUMP_SIZE;
        }

        page->freed = page->off;
        lua_pop(L, 1);

    } else {
		//printf("%s:%d\n", __FILE__, __LINE__);
        page = jit->pages[jit->pagenum-1];
        EnableWrite(page, page->size);
    }

	//printf("%s:%d page = [%p] page+off = [%p] pagenum=[%zu]\n", __FILE__, __LINE__, page, (char*)(((char*)page)+page->off+sizeof(struct jit_head)), jit->pagenum);

    return (uint8_t*) page + page->off;
}

#ifdef OS_OSX //{
#include <libkern/OSCacheControl.h>
#endif //}
#ifdef OS_LINUX //{
#ifdef __clang__ //{
#ifdef __cplusplus //{
extern "C" void __clear_cache(void *, void*);
#else //}{
extern void __clear_cache(void *, void*);
#endif //}
#else //}{
///#include <sys/cachectl.h>
extern void __builtin___clear_cache(void *begin, void *end);
#endif //}
#endif //}

static void commit_code(struct jit* jit, void* code, size_t sz)
{
    struct page* page = jit->pages[jit->pagenum-1];
    page->off += sz;
	//mprotect(page, page->size, PROT_NONE);
    EnableExecute(page, page->size);
#ifdef OS_OSX //{
	sys_icache_invalidate(page, page->size);
#else //}{
#ifdef OS_LINUX //{
#ifdef __clang__ //{
	{
		const char *start = (const char *)page;
		const char *end = (const char *)page + page->size;
		__clear_cache((void*)(start), (void*)(end));
	}
#else //}{
	{
		const char *start = (const char *)page;
		const char *end = (const char *)page + page->size;
		__builtin___clear_cache((void*)(start), (void*)(end));
	}
#endif //}
#endif //}
#endif //}

    {
#if 0
        FILE* out = fopen("out.bin", "wb");
        fwrite(page, page->off, 1, out);
        fclose(out);
#endif
    }
}

/* push_func_ref pushes a copy of the upval table embedded in the compiled
 * function func.
 */
void push_func_ref(lua_State* L, cfunction func)
{
    struct jit_head* h = ((struct jit_head*) func) - 1;
    lua_rawgeti(L, LUA_REGISTRYINDEX, h->ref);
}

void free_code(struct jit* jit, lua_State* L, cfunction func)
{
    size_t i;
    struct jit_head* h = ((struct jit_head*) func) - 1;
    for (i = 0; i < jit->pagenum; i++) {
        struct page* p = jit->pages[i];

        if ((uint8_t*) h < (uint8_t*) p || (uint8_t*) p + p->size <= (uint8_t*) h) {
            continue;
        }

        luaL_unref(L, LUA_REGISTRYINDEX, h->ref);

        EnableWrite(p, p->size);
        p->freed += h->size;

        shred(h, 0, h->size);

        if (p->freed < p->off) {
            EnableExecute(p, p->size);
            return;
        }

        FreePage(p, p->size);
        memmove(&jit->pages[i], &jit->pages[i+1], (jit->pagenum - (i+1)) * sizeof(jit->pages[0]));
        jit->pagenum--;
        return;
    }

    assert(!"couldn't find func in the jit pages");
}


