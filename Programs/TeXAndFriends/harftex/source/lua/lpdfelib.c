/* loslibext.c

    This file is part of LuaTeX.

    LuaTeX is free software; you can redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    LuaTeX is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
    A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
    details.

    You should have received a copy of the GNU General Public License along with
    LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

/*tex

    This file hosts the encapsulated \PDF\ support code used for inclusion and
    access from \LUA.

*/

# include "ptexlib.h"

/*tex

    We need to avoid collision with some defines in |cpascal.h|.

*/

# undef lpdfelib_orig_input
# undef lpdfelib_orig_output

# ifdef input
# define lpdfelib_orig_input input
# undef input
# endif

# ifdef output
# define lpdfelib_orig_output output
# undef output
# endif

# include "luapplib/pplib.h"

# include "image/epdf.h"

# ifdef lpdfelib_orig_input
# define input  lpdfelib_orig_input
# undef lpdfelib_orig_input
# endif

# ifdef lpdfelib_orig_output
# define output  lpdfelib_orig_output
# undef lpdfelib_orig_output
# endif

# include "lua/luatex-api.h"

/*tex

    We start with some housekeeping. Dictionaries, arrays, streams and references
    get userdata, while strings, names, integers, floats and booleans become regular
    \LUA\ objects. We need to define a few metatable identifiers too.

*/

# define PDFE_METATABLE            "luatex.pdfe"
# define PDFE_METATABLE_DICTIONARY "luatex.pdfe.dictionary"
# define PDFE_METATABLE_ARRAY      "luatex.pdfe.array"
# define PDFE_METATABLE_STREAM     "luatex.pdfe.stream"
# define PDFE_METATABLE_REFERENCE  "luatex.pdfe.reference"

typedef struct {
    ppdoc *document;
    boolean open;
    boolean isfile;
    char *memstream;
    int pages;
    int index;
} pdfe_document ;

typedef struct {
    ppdict *dictionary;
} pdfe_dictionary;

typedef struct {
    pparray *array;
} pdfe_array;

typedef struct {
    ppstream *stream;
    int decode;
    int open;
} pdfe_stream;

typedef struct {
 /* ppref *reference; */
    ppxref *xref;
    int onum;
} pdfe_reference;

/*tex

    We need to check if we have the right userdata. A similar warning is issued
    when encounter a problem. We don't exit.

*/

static void pdfe_invalid_object_warning(const char * detail)
{
    formatted_warning("pdfe lib","lua <pdfe %s> expected",detail);
}

static pdfe_document *check_isdocument(lua_State * L, int n)
{
    pdfe_document *p = (pdfe_document *)lua_touserdata(L, n);
    if (p != NULL && lua_getmetatable(L, n)) {
        lua_get_metatablelua(luatex_pdfe);
        if (!lua_rawequal(L, -1, -2)) {
            p = NULL;
        }
        lua_pop(L, 2);
        if (p != NULL) {
            return p;
        }
    }
    pdfe_invalid_object_warning("document");
    return NULL;
}

static pdfe_dictionary *check_isdictionary(lua_State * L, int n)
{
    pdfe_dictionary *p = (pdfe_dictionary *)lua_touserdata(L, n);
    if (p != NULL && lua_getmetatable(L, n)) {
        lua_get_metatablelua(luatex_pdfe_dictionary);
        if (!lua_rawequal(L, -1, -2)) {
            p = NULL;
        }
        lua_pop(L, 2);
        if (p != NULL) {
            return p;
        }
    }
    pdfe_invalid_object_warning("dictionary");
    return NULL;
}

static pdfe_array *check_isarray(lua_State * L, int n)
{
    pdfe_array *p = (pdfe_array *)lua_touserdata(L, n);
    if (p != NULL && lua_getmetatable(L, n)) {
        lua_get_metatablelua(luatex_pdfe_array);
        if (!lua_rawequal(L, -1, -2)) {
            p = NULL;
        }
        lua_pop(L, 2);
        if (p != NULL) {
            return p;
        }
    }
    pdfe_invalid_object_warning("array");
    return NULL;
}

static pdfe_stream *check_isstream(lua_State * L, int n)
{
    pdfe_stream *p = (pdfe_stream *)lua_touserdata(L, n);
    if (p != NULL && lua_getmetatable(L, n)) {
        lua_get_metatablelua(luatex_pdfe_stream);
        if (!lua_rawequal(L, -1, -2)) {
            p = NULL;
        }
        lua_pop(L, 2);
        if (p != NULL) {
            return p;
        }
    }
    pdfe_invalid_object_warning("stream");
    return NULL;
}

static pdfe_reference *check_isreference(lua_State * L, int n)
{
    pdfe_reference *p = (pdfe_reference *)lua_touserdata(L, n);
    if (p != NULL && lua_getmetatable(L, n)) {
        lua_get_metatablelua(luatex_pdfe_reference);
        if (!lua_rawequal(L, -1, -2)) {
            p = NULL;
        }
        lua_pop(L, 2);
        if (p != NULL) {
            return p;
        }
    }
    pdfe_invalid_object_warning("reference");
    return NULL;
}

/*tex

    Reporting the type of a userdata is just a sequence of tests till we find the
    right one. We return nothing is it is no pdfe type.

    \starttyping
    t = pdfe.type(<pdfe document|dictionary|array|reference|stream>)
    \stoptyping

*/

# define check_type(field,meta,name) do { \
    lua_get_metatablelua(luatex_##meta); \
    if (lua_rawequal(L, -1, -2)) { \
        lua_pushstring(L,name); \
        return 1; \
    } \
    lua_pop(L, 1); \
} while (0)

static int pdfelib_type(lua_State * L)
{
    void *p = lua_touserdata(L, 1);
    if (p != NULL && lua_getmetatable(L, 1)) {
        check_type(document,  pdfe,           "pdfe");
        check_type(dictionary,pdfe_dictionary,"pdfe.dictionary");
        check_type(array,     pdfe_array,     "pdfe.array");
        check_type(reference, pdfe_reference, "pdfe.reference");
        check_type(stream,    pdfe_stream,    "pdfe.stream");
    }
    return 0;
}

/*tex

    The \type {tostring} metamethods are similar and report a pdfe type plus a
    pointer value, as is rather usual in \LUA.

*/

# define define_to_string(field,what) \
static int pdfelib_tostring_##field(lua_State * L) { \
    pdfe_##field *p = check_is##field(L, 1); \
    if (p != NULL) { \
        lua_pushfstring(L, "<" what " %p>", (ppdoc *) p->field); \
        return 1; \
    } \
    return 0; \
}

define_to_string(document,  "pdfe")
define_to_string(dictionary,"pdfe.dictionary")
define_to_string(array,     "pdfe.array")
define_to_string(stream,    "pdfe.stream")

static int pdfelib_tostring_reference(lua_State * L) { \
    pdfe_reference *p = check_isreference(L, 1); \
    if (p != NULL) { \
        lua_pushfstring(L, "<pdfe.reference " "%d>",  p->onum); \
        return 1; \
    } \
    return 0; \
}

/*tex

    The pushers look rather similar. We have two variants, one that just pushes
    the object, and another that also pushes some extra information.

*/

# define pdfe_push_dictionary do { \
    pdfe_dictionary *d = (pdfe_dictionary *)lua_newuserdata(L, sizeof(pdfe_dictionary));	\
    luaL_getmetatable(L, PDFE_METATABLE_DICTIONARY); \
    lua_setmetatable(L, -2); \
    d->dictionary = dictionary; \
} while(0)

static int pushdictionary(lua_State * L, ppdict *dictionary)
{
    if (dictionary != NULL) {
        pdfe_push_dictionary;
        lua_pushinteger(L,dictionary->size);
        return 2;
    }
    return 0;
}

static int pushdictionaryonly(lua_State * L, ppdict *dictionary)
{
    if (dictionary != NULL) {
        pdfe_push_dictionary;
        return 1;
    }
    return 0;
}

# define pdfe_push_array do { \
    pdfe_array *a = (pdfe_array *)lua_newuserdata(L, sizeof(pdfe_array));	\
    luaL_getmetatable(L, PDFE_METATABLE_ARRAY); \
    lua_setmetatable(L, -2); \
    a->array = array; \
  } while (0)

static int pusharray(lua_State * L, pparray * array)
{
    if (array != NULL) {
        pdfe_push_array;
        lua_pushinteger(L,array->size);
        return 2;
    }
    return 0;
}

static int pusharrayonly(lua_State * L, pparray * array)
{
    if (array != NULL) {
        pdfe_push_array;
        return 1;
    }
    return 0;
}

# define pdfe_push_stream do { \
    pdfe_stream *s = (pdfe_stream *)lua_newuserdata(L, sizeof(pdfe_stream));	\
    luaL_getmetatable(L, PDFE_METATABLE_STREAM); \
    lua_setmetatable(L, -2); \
    s->stream = stream; \
    s->open = 0; \
    s->decode = 0; \
} while(0)

static int pushstream(lua_State * L, ppstream * stream)
{
    if (stream != NULL) {
        pdfe_push_stream;
        if (pushdictionary(L, stream->dict) > 0)
            return 3;
        else
            return 1;
    }
    return 0;
}

static int pushstreamonly(lua_State * L, ppstream * stream)
{
    if (stream != NULL) {
        pdfe_push_stream;
        if (pushdictionaryonly(L, stream->dict) > 0)
            return 2;
        else
            return 1;
    }
    return 0;
}

# define pdfe_push_reference do { \
    pdfe_reference *r = (pdfe_reference *)lua_newuserdata(L, sizeof(pdfe_reference));	\
    luaL_getmetatable(L, PDFE_METATABLE_REFERENCE); \
    lua_setmetatable(L, -2); \
    r->xref = reference->xref; \
    r->onum = reference->number; \
  } while (0)

static int pushreference(lua_State * L, ppref * reference)
{
    if (reference != NULL && reference->number != 0) {
        pdfe_push_reference;
        lua_pushinteger(L,reference->number);
        return 2;
    }
    return 0;
}

/*tex

    The next function checks for the type and then pushes the matching data on
    the stack.

    \starttabulate[|c|l|l|l|]
    \BC type       \BC meaning    \BC value            \BC detail \NC \NR
    \NC \type {0}  \NC none       \NC nil              \NC \NC \NR
    \NC \type {1}  \NC null       \NC nil              \NC \NC \NR
    \NC \type {2}  \NC boolean    \NC boolean          \NC \NC \NR
    \NC \type {3}  \NC boolean    \NC integer          \NC \NC \NR
    \NC \type {4}  \NC number     \NC float            \NC \NC \NR
    \NC \type {5}  \NC name       \NC string           \NC \NC \NR
    \NC \type {6}  \NC string     \NC string           \NC type \NC \NR
    \NC \type {7}  \NC array      \NC arrayobject      \NC size \NC \NR
    \NC \type {8}  \NC dictionary \NC dictionaryobject \NC size \NC \NR
    \NC \type {9}  \NC stream     \NC streamobject     \NC dictionary size \NC \NR
    \NC \type {10} \NC reference  \NC integer          \NC \NC \NR
    \LL
    \stoptabulate

    A name and string can be distinguished by the extra type value that a string
    has.

*/

static int pushvalue(lua_State * L, ppobj *object)
{
    switch (object->type) {
        case PPNONE:
        case PPNULL:
            lua_pushnil(L);
            return 1;
            break;
        case PPBOOL:
            lua_pushboolean(L,object->integer);
            return 1;
            break;
        case PPINT:
            lua_pushinteger(L, object-> integer);
            return 1;
            break;
        case PPNUM:
            lua_pushnumber(L, object->number);
            return 1;
            break;
        case PPNAME:
            lua_pushstring(L, (const char *) ppname_decoded(object->name));
            return 1;
            break;
        case PPSTRING:
            lua_pushlstring(L,(const char *) object->string, ppstring_size((void *)object->string));
            lua_pushboolean(L, ppstring_hex((void *)object->string));
            return 2;
            break;
        case PPARRAY:
            return pusharray(L, object->array);
            break;
        case PPDICT:
            return pushdictionary(L, object->dict);
            break;
        case PPSTREAM:
            return pushstream(L, object->stream);
            break;
        case PPREF:
            return pushreference(L, object->ref);
            break;
    }
    return 0;
}

/*tex

    We need to start someplace when we traverse a document's tree. There are
    three places:

    \starttyping
    catalogdictionary = getcatalog(documentobject)
    trailerdictionary = gettrailer(documentobject)
    infodictionary    = getinfo   (documentobject)
    \stoptyping

*/

static int pdfelib_getcatalog(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p == NULL)
        return 0;
    return pushdictionaryonly(L,ppdoc_catalog(p->document));
}

static int pdfelib_gettrailer(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p == NULL)
        return 0;
    return pushdictionaryonly(L,ppdoc_trailer(p->document));
}

static int pdfelib_getinfo(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p == NULL)
        return 0;
    return pushdictionaryonly(L,ppdoc_info(p->document));
}

/*tex

    We have three more helpers.

    \starttyping
    [key,] type, value, detail = getfromdictionary(dictionaryobject,name|index)
           type, value, detail = getfromarray     (arrayobject,index)
    [key,] type, value, detail = getfromstream    (streamobject,name|index)
    \stoptyping

*/

static int pdfelib_getfromarray(lua_State * L)
{
    pdfe_array *a = check_isarray(L, 1);
    if (a != NULL) {
        int index = luaL_checkint(L, 2) - 1;
        if (index < a->array->size) {
            ppobj *object = pparray_at(a->array,index);
            if (object != NULL) {
                lua_pushinteger(L,(int) object->type);
                return 1 + pushvalue(L,object);
            }
        }
    }
    return 0;
}

static int pdfelib_getfromdictionary(lua_State * L)
{
    pdfe_dictionary *d = check_isdictionary(L, 1);
    if (d != NULL) {
        if (lua_type(L,2) == LUA_TSTRING) {
            const char *name = luaL_checkstring(L, 2);
            ppobj *object = ppdict_get_obj(d->dictionary,name);
            if (object != NULL) {
                lua_pushinteger(L,(int) object->type);
                return 1 + pushvalue(L,object);
            }
        } else {
            int index = luaL_checkint(L, 2) - 1;
            if (index < d->dictionary->size) {
                ppobj *object = ppdict_at(d->dictionary,index);
                if (object != NULL) {
                    ppname key = ppdict_key(d->dictionary,index);
                    lua_pushstring(L,(const char *) key);
                    lua_pushinteger(L,(int) object->type);
                    return 2 + pushvalue(L,object);
                }
            }
        }
    }
    return 0;
}

static int pdfelib_getfromstream(lua_State * L)
{
    pdfe_stream *s = (pdfe_stream *)lua_touserdata(L, 1);
    if (s != NULL) {
        ppdict *d = s->stream->dict;
        if (lua_type(L,2) == LUA_TSTRING) {
            const char *name = luaL_checkstring(L, 2);
            ppobj *object = ppdict_get_obj(d,name);
            if (object != NULL) {
                lua_pushinteger(L,(int) object->type);
                return 1 + pushvalue(L,object);
            }
        } else {
            int index = luaL_checkint(L, 2) - 1;
            if (index < d->size) {
                ppobj *object = ppdict_at(d,index);
                if (object != NULL) {
                    ppname key = ppdict_key(d,index);
                    lua_pushstring(L,(const char *) key);
                    lua_pushinteger(L,(int) object->type);
                    return 2 + pushvalue(L,object);
                }
            }
        }
    }
    return 0;
}

/*tex

    An indexed table with all entries in an array can be fetched with::

    \starttyping
    t = arraytotable(arrayobject)
    \stoptyping

    An hashed table with all entries in an dictionary can be fetched with::

    \starttyping
    t = dictionarytotable(arrayobject)
    \stoptyping

*/

static void pdfelib_totable(lua_State * L, ppobj * object, int flat)
{
    int n = pushvalue(L,object);
    if (flat && n < 2) {
        return;
    }
    /* [value] [extra] [more] */
    lua_createtable(L,n+1,0);
    if (n == 1) {
        /* value { nil, nil } */
        lua_insert(L,-2);
        /* { nil, nil } value */
        lua_rawseti(L,-2,2);
        /* { nil , value } */
    } else if (n == 2) {
        /* value extra { nil, nil, nil } */
        lua_insert(L,-3);
        /* { nil, nil, nil } value extra */
        lua_rawseti(L,-3,3);
        /* { nil, nil, extra } value */
        lua_rawseti(L,-2,2);
        /* { nil, value, extra } */
    } else if (n == 3) {
        /* value extra more { nil, nil, nil, nil } */
        lua_insert(L,-4);
        /* { nil, nil, nil, nil, nil } value extra more */
        lua_rawseti(L,-4,4);
        /* { nil, nil, nil, more } value extra */
        lua_rawseti(L,-3,3);
        /* { nil, nil, extra, more } value */
        lua_rawseti(L,-2,2);
        /* { nil, value, extra, more } */
    }
    lua_pushinteger(L,(int) object->type);
    /* { nil, [value], [extra], [more] } type */
    lua_rawseti(L,-2,1);
    /* { type, [value], [extra], [more] } */
    return;
}

static int pdfelib_arraytotable(lua_State * L)
{
    pdfe_array *a = check_isarray(L, 1);
    if (a != NULL) {
        int flat = lua_isboolean(L,2);
        int i = 0;
        int j = 0;
        lua_createtable(L,i,0);
        /* table */
        for (i=0;i<a->array->size;i++) {
            ppobj *object = pparray_at(a->array,i);
            if (object != NULL) {
                pdfelib_totable(L,object,flat);
                /* table { type, [value], [extra], [more] } */
                lua_rawseti(L,-2,++j);
                /* table[i] = { type, [value], [extra], [more] } */
            }
        }
        return 1;
    }
    return 0;
}

static int pdfelib_dictionarytotable(lua_State * L)
{
    pdfe_dictionary *d = check_isdictionary(L, 1);
    if (d != NULL) {
        int flat = lua_isboolean(L,2);
        int i = 0;
        lua_createtable(L,0,i);
        /* table */
        for (i=0;i<d->dictionary->size;i++) {
            ppobj *object = ppdict_at(d->dictionary,i);
            if (object != NULL) {
                ppname key = ppdict_key(d->dictionary,i);
                lua_pushstring(L,(const char *) key);
                /* table key */
                pdfelib_totable(L,object,flat);
                /* table key { type, [value], [extra], [more] } */
                lua_rawset(L,-3);
                /* table[key] = { type, [value], [extra] } */
            }
        }
        return 1;
    }
    return 0;
}

/*tex

    All pages are collected with:

    \starttyping
    { { dict, size, objnum }, ... } = pagestotable(document)
    \stoptyping

*/

static int pdfelib_pagestotable(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p != NULL) {
        ppdoc *d = p->document;
        ppref *r = NULL;
        int i = 0;
        int j = 0;
        lua_createtable(L,ppdoc_page_count(d),0);
        /* pages[1..n] */
        for (r = ppdoc_first_page(d), i = 1; r != NULL; r = ppdoc_next_page(d), ++i) {
            lua_createtable(L,3,0);
            if (ppref_obj(r) != NULL) {
                pushdictionary(L,ppref_obj(r)->dict);
                /* table dictionary n */
                lua_rawseti(L,-3,2);
                /* table dictionary */
                lua_rawseti(L,-2,1);
                /* table */
                lua_pushinteger(L,r->number);
                /* table reference */
                lua_rawseti(L,-2,3);
                /* table */
                lua_rawseti(L,-2,++j);
                /* pages[i] = { dictionary, size, objnum } */
            }
        }
        return 1;
    }
    return 0;
}

/*tex

    Streams can be fetched on one go:

    \starttyping
    string, n = readwholestream(streamobject,decode)
    \stoptyping

*/

static int pdfelib_readwholestream(lua_State * L)
{
    pdfe_stream *s = check_isstream(L, 1);
    if (s != NULL) {
        uint8_t *b = NULL;
        int decode = 0;
        size_t n = 0;
        if (s->open > 0) {
            ppstream_done(s->stream);
            s->open = 0;
            s->decode = 0;
        }
        if (lua_gettop(L) > 1 && lua_isboolean(L, 2)) {
            decode = lua_toboolean(L, 2);
        }
        b = ppstream_all(s->stream,&n,decode);
        lua_pushlstring(L, (const char *) b, n);
        lua_pushinteger(L, (int) n);
        ppstream_done(s->stream);
        return 2;
    }
    return 0;
}

/*tex

    Alternatively streams can be fetched stepwise:

    \starttyping
    okay = openstream(streamobject,[decode])
    string, n = readfromstream(streamobject)
    closestream(streamobject)
    \stoptyping

*/

static int pdfelib_openstream(lua_State * L)
{
    pdfe_stream *s = check_isstream(L, 1);
    if (s != NULL) {
        if (s->open == 0) {
            if (lua_gettop(L) > 1) {
                s->decode = lua_isboolean(L, 2);
            }
            s->open = 1;
        }
        lua_pushboolean(L,1);
        return 1;
    }
    return 0;
}

static int pdfelib_closestream(lua_State * L)
{
    pdfe_stream *s = check_isstream(L, 1);
    if (s != NULL) {
        if (s->open >0) {
            ppstream_done(s->stream);
            s->open = 0;
            s->decode = 0;
        }
    }
    return 0;
}

static int pdfelib_readfromstream(lua_State * L)
{
    pdfe_stream *s = check_isstream(L, 1);
    if (s != NULL) {
        size_t n = 0;
        uint8_t *d = NULL;
        if (s->open == 1) {
            d = ppstream_first(s->stream,&n,s->decode);
            s->open = 2;
        } else if (s->open == 2) {
            d = ppstream_next(s->stream,&n);
        } else {
            return 0;
        }
        lua_pushlstring(L, (const char *) d, n);
        lua_pushinteger(L, (int) n);
        return 2;
    }
    return 0;
}

/*tex

    There are two methods for opening a document: files and strings.

    \starttyping
    documentobject = open(filename)
    documentobject = new(string,length)
    \stoptyping

    Closing happens with:

    \starttyping
    close(documentobject)
    \stoptyping

    When the \type {new} function gets a peudo filename as third argument,
    no user data will be created but the stream is accessible as image.

*/

static int pdfelib_open(lua_State * L)
{
    const char *filename = luaL_checkstring(L, 1);
    ppdoc *d = ppdoc_load(filename);
    if (d == NULL) {
        formatted_warning("pdfe lib","no valid pdf file '%s'",filename);
    } else {
        pdfe_document *p = (pdfe_document *) lua_newuserdata(L, sizeof(pdfe_document));
        luaL_getmetatable(L, PDFE_METATABLE);
        lua_setmetatable(L, -2);
        p->document = d;
        p->open = true;
        p->isfile = true;
        p->memstream = NULL;
        return 1;
    }
    return 0;
}

static int pdfelib_new(lua_State * L)
{
    const char *docstream = NULL;
    char *memstream = NULL ;
    unsigned long long streamsize;
    switch (lua_type(L, 1)) {
        case LUA_TSTRING:
            /* stream as Lua string */
            docstream = luaL_checkstring(L, 1);
            break;
        case LUA_TLIGHTUSERDATA:
            /* stream as sequence of bytes */
            docstream = (const char *) lua_touserdata(L, 1);
            break;
        default:
            luaL_error(L, "bad <pdfe> argument: string or lightuserdata expected");
            break;
    }
    if (docstream == NULL) {
        luaL_error(L, "bad <pdfe> document");
    }
    /* size of the stream */
    streamsize = (unsigned long long) luaL_checkint(L, 2);
    memstream = xmalloc((unsigned) (streamsize + 1));
    if (! memstream) {
        luaL_error(L, "no room for <pdfe> stream");
    }
    memcpy(memstream, docstream, (streamsize + 1));
    memstream[streamsize]='\0';
    if (lua_gettop(L) == 2) {
        /* we stay at the lua end */
        ppdoc *d = ppdoc_mem(memstream, streamsize);
        if (d == NULL) {
            normal_warning("pdfe lib","no valid pdf mem stream");
        } else {
            pdfe_document *p = (pdfe_document *) lua_newuserdata(L, sizeof(pdfe_document));
            luaL_getmetatable(L, PDFE_METATABLE);
            lua_setmetatable(L, -2);
            p->document = d;
            p->open = true;
            p->isfile = false;
            p->memstream = memstream;
            return 1;
        }
    } else {
        /* pseudo file name */
        PdfDocument *pdf_doc;
        const char *file_id = luaL_checkstring(L, 3);
        if (file_id == NULL) {
            luaL_error(L, "<pdfe> stream has an invalid id");
        }
        if (strlen(file_id) > STREAM_FILE_ID_LEN ) {
            /* a limit to the length of the string */
            luaL_error(L, "<pdfe> stream has a too long id");
        }
        pdf_doc = refMemStreamPdfDocument(memstream, streamsize, file_id);
        if (pdf_doc != NULL) {
            lua_pushstring(L,pdf_doc->file_path);
            return 1;
        } else {
            /* pplib does this: xfree(memstream); */
        }
    }
    return 0;
}

/*

    There is no garbage collection needed as the library itself manages the
    objects. Normally objects don't take much space. Streams use buffers so (I
    assume) that they are not persistent. The only collector is in the parent
    object (the document).

*/

static int pdfelib_free(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p != NULL && p->open) {
        if (p->document != NULL) {
            ppdoc_free(p->document);
            p->document = NULL;
        }
        if (p->memstream != NULL) {
         /* pplib does this: xfree(p->memstream); */
            p->memstream = NULL;
        }
        p->open = false;
    }
    return 0;
}

static int pdfelib_close(lua_State * L)
{
    return pdfelib_free(L);
}

/*tex

    A document is can be uncrypted with:

    \starttyping
    status = unencrypt(documentobject,user,owner)
    \stoptyping

    Instead of a password \type {nil} can be passed, so there are three possible
    useful combinations.

*/

static int pdfelib_unencrypt(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p != NULL) {
        size_t u = 0;
        size_t o = 0;
        const char* user = NULL;
        const char* owner = NULL;
        int top = lua_gettop(L);
        if (top > 1) {
            if (lua_type(L,2) == LUA_TSTRING) {
                user = lua_tolstring(L, 2, &u);
            } else {
                /*tex we're not too picky but normally it will be nil or false */
            }
            if (top > 2) {
                if (lua_type(L,3) == LUA_TSTRING) {
                    owner = lua_tolstring(L, 3, &o);
                } else {
                    /*tex we're not too picky but normally it will be nil or false */
                }
            }
            lua_pushinteger(L, (int) ppdoc_crypt_pass(p->document,user,u,owner,o));
            return 1;
        }
    }
    lua_pushinteger(L, (int) PPCRYPT_FAIL);
    return 1;
}

/*tex

    There are a couple of ways to get information about the document:

    \starttyping
    n             = getsize       (documentobject)
    major, minor  = getversion    (documentobject)
    status        = getstatus     (documentobject)
    n             = getnofobjects (documentobject)
    n             = getnofpages   (documentobject)
    bytes, waste  = getmemoryusage(documentobject)
    \stoptyping

*/

static int pdfelib_getsize(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p == NULL)
        return 0;
    lua_pushinteger(L,(int) ppdoc_file_size(p->document));
    return 1;
}


static int pdfelib_getversion(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p == NULL) {
        return 0;
    } else {
        int minor;
        int major = ppdoc_version_number(p->document, &minor);
        lua_pushinteger(L,(int) major);
        lua_pushinteger(L,(int) minor);
        return 2;
    }
}

static int pdfelib_getstatus(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p == NULL)
        return 0;
    lua_pushinteger(L,(int) ppdoc_crypt_status(p->document));
    return 1;
}

static int pdfelib_getnofobjects(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p == NULL)
        return 0;
    lua_pushinteger(L,(int) ppdoc_objects(p->document));
    return 1;
}

static int pdfelib_getnofpages(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p == NULL)
        return 0;
    lua_pushinteger(L,(int) ppdoc_page_count(p->document));
    return 1;
}

static int pdfelib_getmemoryusage(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p != NULL) {
        size_t w = 0;
        size_t m = ppdoc_memory(p->document,&w);
        lua_pushinteger(L,(int) m);
        lua_pushinteger(L,(int) w);
        return 2;
    }
    return 0;
}

/*
    A specific page dictionary can be filtered with the next command. So, there
    is no need to parse the document page tree (with these \type {kids} arrays).

    \starttyping
    dictionaryobject = getpage(documentobject,pagenumber)
    \stoptyping

*/

static int pushpage(lua_State * L, ppdoc * d, int page)
{
    if (page <= 0 || page > ppdoc_page_count(d)) {
        return 0;
    } else {
        ppref *pp = ppdoc_page(d,page);
        return pushdictionaryonly(L, ppref_obj(pp)->dict);
    }
}

static int pdfelib_getpage(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p == NULL) {
        return 0;
    } else {
        return pushpage(L, p->document, luaL_checkint(L, 2));
    }
}

static int pushpages(lua_State * L, ppdoc * d)
{
    int i = 0;
    ppref *r;
    lua_createtable(L,ppdoc_page_count(d),0);
    /* pages[1..n] */
    for (r = ppdoc_first_page(d), i = 1; r != NULL; r = ppdoc_next_page(d), ++i) {
        pushdictionaryonly(L,ppref_obj(r)->dict);
        lua_rawseti(L,-2,i);
    }
    return 1 ;
}

static int pdfelib_getpages(lua_State * L)
{
    pdfe_document *p = check_isdocument(L, 1);
    if (p == NULL) {
        return 0;
    } else {
        return pushpages(L, p->document);
    }
}

/*tex

    The boundingbox (\type {MediaBox) and similar boxes can be available in a
    (page) doctionary but also in a parent object. Therefore a helper is
    available that does the (backtracked) lookup.

    \starttyping
    { lx, ly, rx, ry } = getbox(dictionaryobject)
    \stoptyping

*/

static int pdfelib_getbox(lua_State * L)
{
    if (lua_gettop(L) > 1 && lua_type(L,2) == LUA_TSTRING) {
        pdfe_dictionary *p = check_isdictionary(L, 1);
        if (p != NULL) {
            const char *key = lua_tostring(L,2);
            pprect box;
            pprect *r;
            box.lx = box.rx = box.ly = box.ry = 0;
            r = ppdict_get_box(p->dictionary,key,&box);
            if (r != NULL) {
                lua_createtable(L,4,0);
                lua_pushnumber(L,r->lx);
                lua_rawseti(L,-2,1);
                lua_pushnumber(L,r->ly);
                lua_rawseti(L,-2,2);
                lua_pushnumber(L,r->rx);
                lua_rawseti(L,-2,3);
                lua_pushnumber(L,r->ry);
                lua_rawseti(L,-2,4);
                return 1;
            }
        }
    }
    return 0;
}

/*tex

    This one is needed when you use the detailed getters and run into an
    object reference. The regular getters resolve this automatically.

    \starttyping
    [dictionary|array|stream]object = getfromreference(referenceobject)
    \stoptyping

*/

static int pdfelib_getfromreference(lua_State * L)
{
    pdfe_reference *r = check_isreference(L, 1);
    if (r != NULL && r->xref != NULL) {
        ppref *rr = ppxref_find(r->xref, (ppuint) r->onum);
        if (rr != NULL) {
            ppobj *o = ppref_obj(rr);
            if (o != NULL) {
                lua_pushinteger(L,o->type);
                return 1 + pushvalue(L,o);
            }
        }
    }
    return 0;
}

/*tex

    Here are some convenient getters:

    \starttyping
    <string>         = getstring    (array|dict|ref,index|key)
    <integer>        = getinteger   (array|dict|ref,index|key)
    <number>         = getnumber    (array|dict|ref,index|key)
    <boolan>         = getboolean   (array|dict|ref,index|key)
    <string>         = getname      (array|dict|ref,index|key)
    <dictionary>     = getdictionary(array|dict|ref,index|key)
    <array>          = getarray     (array|dict|ref,index|key)
    <stream>, <dict> = getstream    (array|dict|ref,index|key)
    \stoptyping

    We report issues when reasonable but are silent when it makes sense. We don't
    error on this because we expect the user code to act reasonable on a return
    value.

*/

# define pdfelib_get_value_check_1 do { \
    if (p == NULL) { \
        if (t == LUA_TSTRING) { \
            normal_warning("pdfe lib","lua <pdfe dictionary> expected"); \
        } else if (t == LUA_TNUMBER) { \
            normal_warning("pdfe lib","lua <pdfe array> expected"); \
        } else { \
          normal_warning("pdfe lib","invalid arguments"); \
        } \
        return 0; \
    } else if (! lua_getmetatable(L, 1)) { \
        normal_warning("pdfe lib","first argument should be a <pde array> or <pde dictionary>"); \
    } \
} while (0)

# define pdfelib_get_value_check_2 \
    normal_warning("pdfe lib","second argument should be integer or string");

/*tex

    The direct fetcher returns the result or |NULL| when there is nothing
    found.

*/

# define pdfelib_get_indirect_o(p) \
    ppref *r = (((pdfe_reference *) p)->xref != NULL) ? ppxref_find(((pdfe_reference *) p)->xref, (ppuint) (((pdfe_reference *) p)->onum)) : NULL; \
    ppobj *o = (r != NULL) ? ppref_obj(r) : NULL; \

# define pdfelib_get_value_direct(get_d,get_a) do {                      \
    int t = lua_type(L,2);                                              \
    void *p = lua_touserdata(L, 1);                                     \
    pdfelib_get_value_check_1;                                          \
    if (t == LUA_TSTRING) {                                             \
        const char *key = lua_tostring(L,-2);                           \
        lua_get_metatablelua(luatex_pdfe_dictionary);                   \
        if (lua_rawequal(L, -1, -2)) {                                  \
            value = get_d(((pdfe_dictionary *) p)->dictionary, key);    \
        } else {                                                        \
            lua_pop(L,1);                                               \
            lua_get_metatablelua(luatex_pdfe_reference);                \
            if (lua_rawequal(L, -1, -2)) {                              \
                pdfelib_get_indirect_o(p);                              \
                if (o != NULL && o->type == PPDICT) {                   \
                    value = get_d((ppdict *)o->dict, key);              \
                }                                                       \
            }                                                           \
        }                                                               \
    } else if (t == LUA_TNUMBER) {                                      \
        size_t index = lua_tointeger(L,-2);                             \
        lua_get_metatablelua(luatex_pdfe_array);                        \
        if (lua_rawequal(L, -1, -2)) {                                  \
            value = get_a(((pdfe_array *) p)->array, index);            \
        } else {                                                        \
            lua_pop(L,1);                                               \
            lua_get_metatablelua(luatex_pdfe_reference);                \
            if (lua_rawequal(L, -1, -2)) {                              \
                pdfelib_get_indirect_o(p);                              \
                if (o != NULL && o->type == PPARRAY) {                  \
                    value = get_a((pparray *) o->array, index);         \
                }                                                       \
            }                                                           \
        }                                                               \
    } else {                                                            \
        pdfelib_get_value_check_2;                                      \
    }                                                                   \
} while (0)

/*tex

    The indirect fetcher passes a pointer to the target variable and returns
    success state.

*/

# define pdfelib_get_value_indirect(get_d,get_a) do {                       \
    int t = lua_type(L,2);                                                  \
    void *p = lua_touserdata(L, 1);                                         \
    pdfelib_get_value_check_1;                                              \
    if (t == LUA_TSTRING) {                                                 \
        const char *key = lua_tostring(L,-2);                               \
        lua_get_metatablelua(luatex_pdfe_dictionary);                       \
        if (lua_rawequal(L, -1, -2)) {                                      \
            okay = get_d(((pdfe_dictionary *) p)->dictionary, key, &value); \
        } else {                                                            \
            lua_pop(L,1);                                                   \
            lua_get_metatablelua(luatex_pdfe_reference);                    \
            if (lua_rawequal(L, -1, -2)) {                                  \
                pdfelib_get_indirect_o(p);                                  \
                if (o != NULL && o->type == PPDICT)                         \
                    okay = get_d(o->dict, key, &value);                     \
            }                                                               \
        }                                                                   \
    } else if (t == LUA_TNUMBER) {                                          \
        size_t index = lua_tointeger(L,-2);                                 \
        lua_get_metatablelua(luatex_pdfe_array);                            \
        if (lua_rawequal(L, -1, -2)) {                                      \
            okay = get_a(((pdfe_array *) p)->array, index, &value);         \
        } else {                                                            \
            lua_pop(L,1);                                                   \
            lua_get_metatablelua(luatex_pdfe_reference);                    \
            if (lua_rawequal(L, -1, -2)) {                                  \
                pdfelib_get_indirect_o(p);                                  \
                if (o != NULL && o->type == PPARRAY)                        \
                    okay = get_a(o->array, index, &value);                  \
            }                                                               \
        }                                                                   \
    } else {                                                                \
        pdfelib_get_value_check_2;                                          \
    }                                                                       \
} while (0)

static int pdfelib_getstring(lua_State * L)
{
    if (lua_gettop(L) > 1) {
        ppstring value = NULL;
        pdfelib_get_value_direct(ppdict_rget_string,pparray_rget_string);
        if (value != NULL) {
            lua_pushstring(L,(const char *) value);
            return 1;
        }
    }
    return 0;
}

static int pdfelib_getinteger(lua_State * L)
{
    if (lua_gettop(L) > 1) {
        ppint value = 0;
        int okay = 0;
        pdfelib_get_value_indirect(ppdict_rget_int,pparray_rget_int);
        if (okay) {
            lua_pushinteger(L,(int) value);
            return 1;
        }
    }
    return 0;
}

static int pdfelib_getnumber(lua_State * L)
{
    if (lua_gettop(L) > 1) {
        ppnum value = 0;
        int okay = 0;
        pdfelib_get_value_indirect(ppdict_rget_num,pparray_rget_num);
        if (okay) {
            lua_pushnumber(L,value);
            return 1;
        }
    }
    return 0;
}

static int pdfelib_getboolean(lua_State * L)
{
    if (lua_gettop(L) > 1) {
        int value = 0;
        int okay = 0;
        pdfelib_get_value_indirect(ppdict_rget_bool,pparray_rget_bool);
        if (okay) {
            lua_pushboolean(L,value);
            return 1;
        }
    }
    return 0;
}

static int pdfelib_getname(lua_State * L)
{
    if (lua_gettop(L) > 1) {
        ppname value = NULL;
        pdfelib_get_value_direct(ppdict_rget_name,pparray_rget_name);
        if (value != NULL) {
            lua_pushstring(L,(const char *) ppname_decoded(value));
            return 1;
        }
    }
    return 0;
}

static int pdfelib_getdictionary(lua_State * L)
{
    if (lua_gettop(L) > 1) {
        ppdict * value = NULL;
        pdfelib_get_value_direct(ppdict_rget_dict,pparray_rget_dict);
        if (value != NULL) {
            return pushdictionaryonly(L,value);
        }
    }
    return 0;
}

static int pdfelib_getarray(lua_State * L)
{
    if (lua_gettop(L) > 1) {
        pparray * value = NULL;
        pdfelib_get_value_direct(ppdict_rget_array,pparray_rget_array);
        if (value != NULL) {
            return pusharrayonly(L,value);
        }
    }
    return 0;
}

static int pdfelib_getstream(lua_State * L)
{
    if (lua_gettop(L) > 1) {
        ppobj * value = NULL;
        pdfelib_get_value_direct(ppdict_rget_obj,pparray_rget_obj);
        if (value != NULL && value->type == PPSTREAM) {
            return pushstreamonly(L,(ppstream *) value->stream);
        }
    }
    return 0;
}

/*tex

    The generic pushed that does a similar job as the previous getters acts upon
    the type.

*/

static int pdfelib_pushvalue(lua_State * L, ppobj *object)
{
    switch (object->type) {
        case PPNONE:
        case PPNULL:
            lua_pushnil(L);
            break;
        case PPBOOL:
            lua_pushboolean(L, object->integer);
            break;
        case PPINT:
            lua_pushinteger(L, object->integer);
            break;
        case PPNUM:
            lua_pushnumber(L, object->number);
            break;
        case PPNAME:
            lua_pushstring(L, (const char *) ppname_decoded(object->name));
            break;
        case PPSTRING:
            lua_pushlstring(L,(const char *) object->string, ppstring_size((void *)object->string));
            break;
        case PPARRAY:
            return pusharrayonly(L, object->array);
            break;
        case PPDICT:
            return pushdictionary(L, object->dict);
            break;
        case PPSTREAM:
            return pushstream(L, object->stream);
            break;
        case PPREF:
            pushreference(L, object->ref);
            break;
        default:
            lua_pushnil(L);
            break;
    }
    return 1;
}

/*tex

    Finally we arrived at the acessors for the userdata objects. The use
    previously defined helpers.

*/

static int pdfelib_access(lua_State * L)
{
    if (lua_type(L,2) == LUA_TSTRING) {
        pdfe_document *p = (pdfe_document *)lua_touserdata(L, 1);
        const char *s = lua_tostring(L,2);
        if (lua_key_eq(s,catalog) || lua_key_eq(s,Catalog)) {
            return pushdictionaryonly(L,ppdoc_catalog(p->document));
        } else if (lua_key_eq(s,info) || lua_key_eq(s,Info)) {
            return pushdictionaryonly(L,ppdoc_info(p->document));
        } else if (lua_key_eq(s,trailer) || lua_key_eq(s,Trailer)) {
            return pushdictionaryonly(L,ppdoc_trailer(p->document));
        } else if (lua_key_eq(s,pages) || lua_key_eq(s,Pages)) {
            return pushpages(L,p->document);
        }
    }
    return 0;
}

static int pdfelib_array_access(lua_State * L)
{
    if (lua_type(L,2) == LUA_TNUMBER) {
        pdfe_array *p = (pdfe_array *)lua_touserdata(L, 1);
        ppint index = lua_tointeger(L,2) - 1;
        ppobj *o = pparray_rget_obj(p->array,index);
        if (o != NULL) {
            return pdfelib_pushvalue(L,o);
        }
    }
    return 0;
}

static int pdfelib_dictionary_access(lua_State * L)
{
    pdfe_dictionary *p = (pdfe_dictionary *)lua_touserdata(L, 1);
    if (lua_type(L,2) == LUA_TSTRING) {
        const char *key = lua_tostring(L,2);
        ppobj *o = ppdict_rget_obj(p->dictionary,key);
        if (o != NULL) {
            return pdfelib_pushvalue(L,o);
        }
    } else if (lua_type(L,2) == LUA_TNUMBER) {
        ppint index = lua_tointeger(L,2) - 1;
        ppobj *o = ppdict_at(p->dictionary,index);
        if (o != NULL) {
            return pdfelib_pushvalue(L,o);
        }
    }
    return 0;
}

static int pdfelib_stream_access(lua_State * L)
{
    pdfe_stream *p = (pdfe_stream *)lua_touserdata(L, 1);
    if (lua_type(L,2) == LUA_TSTRING) {
        const char *key = lua_tostring(L,2);
        ppobj *o = ppdict_rget_obj(p->stream->dict,key);
        if (o != NULL) {
            return pdfelib_pushvalue(L,o);
        }
    } else if (lua_type(L,2) == LUA_TNUMBER) {
        ppint index = lua_tointeger(L,2) - 1;
        ppobj *o = ppdict_at(p->stream->dict,index);
        if (o != NULL) {
            return pdfelib_pushvalue(L,o);
        }
    }
    return 0;
}

/*tex

    The length metamethods are defined last.

*/

static int pdfelib_array_size(lua_State * L)
{
    pdfe_array *p = (pdfe_array *)lua_touserdata(L, 1);
    lua_pushinteger(L,p->array->size);
    return 1;
}

static int pdfelib_dictionary_size(lua_State * L)
{
    pdfe_dictionary *p = (pdfe_dictionary *)lua_touserdata(L, 1);
    lua_pushinteger(L,p->dictionary->size);
    return 1;
}

static int pdfelib_stream_size(lua_State * L)
{
    pdfe_stream *p = (pdfe_stream *)lua_touserdata(L, 1);
    lua_pushinteger(L,p->stream->dict->size);
    return 1;
}

/*tex

    We now initialize the main interface. We might add few more informational
    helpers but this is it.

*/

static const struct luaL_Reg pdfelib[] = {
    /* management */
    { "type",                    pdfelib_type },
    { "open",                    pdfelib_open },
    { "new",                     pdfelib_new },
    { "close",                   pdfelib_close },
    { "unencrypt",               pdfelib_unencrypt },
    /* statistics */
    { "getversion",              pdfelib_getversion },
    { "getstatus",               pdfelib_getstatus },
    { "getsize",                 pdfelib_getsize },
    { "getnofobjects",           pdfelib_getnofobjects },
    { "getnofpages",             pdfelib_getnofpages },
    { "getmemoryusage",          pdfelib_getmemoryusage },
    /* getters */
    { "getcatalog",              pdfelib_getcatalog },
    { "gettrailer",              pdfelib_gettrailer },
    { "getinfo",                 pdfelib_getinfo },
    { "getpage",                 pdfelib_getpage },
    { "getpages",                pdfelib_getpages },
    { "getbox",                  pdfelib_getbox },
    { "getfromreference",        pdfelib_getfromreference },
    { "getfromdictionary",       pdfelib_getfromdictionary },
    { "getfromarray",            pdfelib_getfromarray },
    { "getfromstream",           pdfelib_getfromstream },
    /* collectors */
    { "dictionarytotable",       pdfelib_dictionarytotable },
    { "arraytotable",            pdfelib_arraytotable },
    { "pagestotable",            pdfelib_pagestotable },
    /* more getters */
    { "getstring",               pdfelib_getstring },
    { "getinteger",              pdfelib_getinteger },
    { "getnumber",               pdfelib_getnumber },
    { "getboolean",              pdfelib_getboolean },
    { "getname",                 pdfelib_getname },
    { "getdictionary",           pdfelib_getdictionary },
    { "getarray",                pdfelib_getarray },
    { "getstream",               pdfelib_getstream },
    /* streams */
    { "readwholestream",         pdfelib_readwholestream },
    /* not really needed */
    { "openstream",              pdfelib_openstream },
    { "readfromstream",          pdfelib_readfromstream },
    { "closestream",             pdfelib_closestream },
    /* done */
    { NULL,                      NULL}
};

/*tex

    The user data metatables are defined as follows. Watch how only the
    document needs a garbage collector.

*/

static const struct luaL_Reg pdfelib_m[] = {
    { "__tostring", pdfelib_tostring_document },
    { "__gc",       pdfelib_free },
    { "__index",    pdfelib_access },
    { NULL,         NULL}
};

static const struct luaL_Reg pdfelib_m_dictionary[] = {
    { "__tostring", pdfelib_tostring_dictionary },
    { "__index",    pdfelib_dictionary_access },
    { "__len",      pdfelib_dictionary_size },
    { NULL,         NULL}
};

static const struct luaL_Reg pdfelib_m_array[] = {
    { "__tostring", pdfelib_tostring_array },
    { "__index",    pdfelib_array_access },
    { "__len",      pdfelib_array_size },
    { NULL,         NULL}
};

static const struct luaL_Reg pdfelib_m_stream[] = {
    { "__tostring", pdfelib_tostring_stream },
    { "__index",    pdfelib_stream_access },
    { "__len",      pdfelib_stream_size },
    { "__call",     pdfelib_readwholestream },
    { NULL,         NULL}
};

static const struct luaL_Reg pdfelib_m_reference[] = {
    { "__tostring", pdfelib_tostring_reference },
    { NULL,         NULL}
};

/*tex

    Finally we hav earrived at the main initialiser that will be called as part
    of \LUATEX's initializer.

*/

/*tex

    Here we hook in the error handler.

*/

static void pdfelib_message(const char *message, void *alien)
{
    normal_warning("pdfe",message);
}

int luaopen_pdfe(lua_State * L)
{
    /*tex First the four userdata object get their metatables defined. */

    luaL_newmetatable(L, PDFE_METATABLE_DICTIONARY);
    luaL_openlib(L, NULL, pdfelib_m_dictionary, 0);

    luaL_newmetatable(L, PDFE_METATABLE_ARRAY);
    luaL_openlib(L, NULL, pdfelib_m_array, 0);

    luaL_newmetatable(L, PDFE_METATABLE_STREAM);
    luaL_openlib(L, NULL, pdfelib_m_stream, 0);

    luaL_newmetatable(L, PDFE_METATABLE_REFERENCE);
    luaL_openlib(L, NULL, pdfelib_m_reference, 0);

    /*tex Then comes the main (document) metatable: */

    luaL_newmetatable(L, PDFE_METATABLE);
    luaL_openlib(L, NULL, pdfelib_m, 0);

    /*tex Last the library opens up itself to the world. */

    luaL_openlib(L, "pdfe", pdfelib, 0);

    pplog_callback(pdfelib_message, stderr);

    return 1;
}
