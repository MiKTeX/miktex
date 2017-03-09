/* vim: ts=4 sw=4 sts=4 et tw=78
 * Portions copyright (c) 2015-present, Facebook, Inc. All rights reserved.
 * Portions copyright (c) 2011 James R. McKaskill.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
#include "ffi.h"

static cfunction compile(Dst_DECL, lua_State* L, cfunction func, int ref);

static void* reserve_code(struct jit* jit, lua_State* L, size_t sz);
static void commit_code(struct jit* jit, void* p, size_t sz);

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
#else
#include "dynasm/dasm_x86.h"
#include "call_x86.h"
#endif

struct jit_head {
    size_t size;
    int ref;
    uint8_t jump[JUMP_SIZE];
};

#define LINKTABLE_MAX_SIZE (sizeof(extnames) / sizeof(extnames[0]) * (JUMP_SIZE))

static cfunction compile(struct jit* jit, lua_State* L, cfunction func, int ref)
{
    struct jit_head* code;
    size_t codesz;
    int err;

    dasm_checkstep(jit, -1);
    if ((err = dasm_link(jit, &codesz)) != 0) {
        char buf[32];
        sprintf(buf, "%x", err);
        luaL_error(L, "dasm_link error %s", buf);
    }

    codesz += sizeof(struct jit_head);
    code = (struct jit_head*) reserve_code(jit, L, codesz);
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

    /* compensate for room taken up for the offset so that we can work rip
     * relative */
    addr += BRANCH_OFF;

    /* see if we can fit the offset in the branch displacement, if not use the
     * jump instruction */
    off = *(uint8_t**) jmp - addr;

    if (MIN_BRANCH <= off && off <= MAX_BRANCH) {
        return (int32_t) off;
    } else {
        return (int32_t)(jmp + sizeof(uint8_t*) - addr);
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
        ADDFUNC(NULL, check_int32);
        ADDFUNC(NULL, check_uint32);
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
        page = jit->pages[jit->pagenum-1];
        EnableWrite(page, page->size);
    }

    return (uint8_t*) page + page->off;
}

static void commit_code(struct jit* jit, void* code, size_t sz)
{
    struct page* page = jit->pages[jit->pagenum-1];
    page->off += sz;
    EnableExecute(page, page->size);
    {
#if 0
        FILE* out = fopen("\\Hard Disk\\out.bin", "wb");
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


