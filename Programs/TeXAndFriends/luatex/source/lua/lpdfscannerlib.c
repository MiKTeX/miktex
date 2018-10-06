/* lpdfscannerlib.c

   Copyright 2013 Taco Hoekwater <taco@luatex.org>

   This file is part of LuaTeX.

   LuaTeX is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   LuaTeX is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU General Public License along
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

/*tex

    The scanner can read from a string or stream. Streams can be given directly as
    |ppstream| object or as a |pparray| of streams. Here is an example of usage:

    \starttyping
    local operatortable = { }

    operatortable.Do = function(scanner,info)
        local resources = info.resources
        if resources then
            local val     = scanner:pop()
            local name    = val[2]
            local xobject = resources.XObject
            print(info.space .. "Uses XObject " .. name)
            local resources = xobject.Resources
            if resources then
                local newinfo =  {
                    space     = info.space .. "  ",
                    resources = resources,
                }
                pdfscanner.scan(entry, operatortable, newinfo)
            end
        end
    end

    local function Analyze(filename)
        local doc = pdfe.open(filename)
        if doc then
            local pages = doc.Pages
            for i=1,#pages do
                local page = pages[i]
                local info = {
                  space     = "  " ,
                  resources = page.Resources,
                }
                print("Page " .. i)
                pdfscanner.scan(page.Contents,operatortable,info)
                pdfscanner.scan(page.Contents(),operatortable,info)
            end
        end
    end

    Analyze("foo.pdf")
    \stoptyping

*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "luapplib/pplib.h"

#include <lua/luatex-api.h>

#define SCANNER "pdfscanner"

#define MAXOPERANDS 1000

typedef enum {
    pdf_integer = 1,
    pdf_real,
    pdf_boolean,
    pdf_name,
    pdf_operator,
    pdf_string,
    pdf_startarray,
    pdf_stoparray,
    pdf_startdict,
    pdf_stopdict,
} pdf_token_type;

typedef struct Token {
    pdf_token_type type;
    double value;
    char *string;
} Token;

typedef struct ObjectList {
    struct ObjectList *next;
    ppstream *stream;
} ObjectList;

typedef struct scannerdata {
    int _ininlineimage;
    int _nextoperand;
    Token **_operandstack;
    ppstream *_stream;
    ObjectList *_streams;
    const char *buffer;
    size_t position;
    size_t size;
    int uses_stream;
} scannerdata;

#define PDFE_METATABLE_ARRAY  "luatex.pdfe.array"
#define PDFE_METATABLE_STREAM "luatex.pdfe.stream"

typedef struct {
    void *d;
    /*tex reference to |PdfDocument|, or |NULL| */
    void *pd;
    /*tex counter to detect |PDFDoc| change */
    unsigned long pc;
} udstruct;

static void clear_operand_stack(scannerdata * self, int from);
static Token *_parseToken(scannerdata * self, int c);
static void push_token(lua_State * L, scannerdata * self);

static void *priv_xmalloc(size_t size)
{
    void *new_mem = (void *) malloc(size);
    if (new_mem == NULL) {
        luaL_error(Luas, "no room for <pdfscanned> stream");
    }
    return new_mem;
}

static void *priv_xrealloc(void *old_ptr, size_t size)
{
    void *new_mem = (void *) realloc(old_ptr, size);
    if (new_mem == NULL) {
        luaL_error(Luas, "no room for <pdfscanned> stream");
    }
    return new_mem;
}

#define xreallocarray(ptr,type,size) ((type*)priv_xrealloc(ptr,(size+1)*sizeof(type)))

#define INITBUFSIZE 64

#define define_buffer(a) \
  char *a = (char *)priv_xmalloc (INITBUFSIZE); \
  int a##_size = INITBUFSIZE; \
  int a##index = 0; \
  memset (a,0,INITBUFSIZE)

#define check_overflow(a, wsize) do { \
    if (wsize >= a##_size) { \
        int nsize = a##_size + a##_size / 4; \
        a = (char *) xreallocarray(a, char, (unsigned) nsize); \
        memset (a+a##_size, 0, a##_size / 4); \
        a##_size = nsize; \
    } \
} while (0)


static scannerdata *scanner_push(lua_State * L)
{
    scannerdata *a = (scannerdata *) lua_newuserdata(L, sizeof(scannerdata));
    luaL_getmetatable(L, SCANNER);
    lua_setmetatable(L, -2);
    return a;
}

static scannerdata *scanner_check(lua_State * L, int index)
{
    scannerdata *bar;
    luaL_checktype(L, index, LUA_TUSERDATA);
    bar = (scannerdata *) luaL_checkudata(L, index, SCANNER);
    if (bar == NULL)
        luaL_argerror(L, index, SCANNER " expected");
    return bar;
}

static void free_token(Token * token)
{
    if (token->string) {
        free(token->string);
    }
    free(token);
}

static void clear_operand_stack(scannerdata * self, int from)
{
    int i = self->_nextoperand - 1;
    while (i >= from) {
        if (self->_operandstack[i]) {
            free_token(self->_operandstack[i]);
            self->_operandstack[i] = NULL;
        }
        i--;
    }
    self->_nextoperand = from;
}

static void push_operand(scannerdata * self, Token * token)
{
    if (self->_nextoperand + 1 > MAXOPERANDS) {
        fprintf(stderr, "out of operand stack space");
        exit(1);
    }
    self->_operandstack[self->_nextoperand++] = token;
}

static Token *new_operand(pdf_token_type c)
{
    Token *token = (Token *) priv_xmalloc(sizeof(Token));
    memset(token, 0, sizeof(Token));
    token->type = c;
    return token;
}

static void _nextStream(scannerdata * self)
{
    ObjectList *rover = NULL;
    if (self->uses_stream && self->buffer != NULL) {
        if (self->uses_stream) {
            ppstream_done(self->_stream);
        } else {
            free(self->_stream);
        }
    }
    rover = self->_streams;
    self->_stream = rover->stream;
    if (self->uses_stream) {
        self->buffer = (const char *) ppstream_all(self->_stream, &self->size, 1);
    }
    self->position = 0;
    self->_streams = rover->next;
    free(rover);
}

static int streamGetChar(scannerdata * self)
{
    int i = EOF;
    if (self->position < self->size) {
        const char c = self->buffer[self->position];
        ++self->position;
        i = (int) c;
    }
    if (i < 0 && self->_streams) {
        _nextStream(self);
        i = streamGetChar(self);
    }
    return i;
}

static int streamLookChar(scannerdata * self)
{
    int i = EOF;
    if (self->position < self->size) {
        const char c = self->buffer[self->position];
        /*not |++self->position;| */
        i = (int) c;
    }
    if (i < 0 && self->_streams) {
        _nextStream(self);
        i = streamGetChar(self);
    }
    return i;
}

static void streamReset(scannerdata * self)
{
    if (self->uses_stream) {
        self->buffer = (const char *) ppstream_all(self->_stream, &self->size, 1);
    }
    self->position = 0;
}

static void streamClose(scannerdata * self)
{
    if (self->uses_stream) {
        ppstream_done(self->_stream);
    } else {
        free(self->_stream);
    }
    self->buffer = NULL;
    self->_stream = NULL;
}

/*tex end of stream interface */

static Token *_parseSpace(scannerdata * self)
{
    return _parseToken(self, streamGetChar(self));
}

static Token *_parseString(scannerdata * self, int c)
{
    int level;
    Token *token = NULL;
    define_buffer(found);
    level = 1;
    while (1) {
        c = streamGetChar(self);
        if (c == '(') {
            level = level + 1;
        } else if (c == ')') {
            level = level - 1;
            if (level < 1)
            break;
        } else if (c == '\\') {
            int next = streamGetChar(self);
            if (next == '(' || next == ')' || next == '\\') {
                c = next;
            } else if (next == '\n' || next == '\r') {
                c = '\0';
            } else if (next == 'n') {
                c = '\n';
            } else if (next == 'r') {
                c = '\r';
            } else if (next == 't') {
                c = '\t';
            } else if (next == 'b') {
                c = '\b';
            } else if (next == 'f') {
                c = '\f';
            } else if (next >= '0' && next <= '7') {
                int next2;
                next = next - '0';
                next2 = streamLookChar(self);
                if (next2 >= '0' && next2 <= '7') {
                    int next3;
                    next2 = streamGetChar(self);
                    next2 = next2 - '0';
                    next3 = streamLookChar(self);
                    if (next3 >= '0' && next3 <= '7') {
                    next3 = streamGetChar(self);
                    next3 = next3 - '0';
                    c = (next * 64 + next2 * 8 + next3);
                    } else {
                    c = (next * 8 + next2);
                    }
                } else {
                    c = next;
                }
            } else {
                c = next;
            }
        }
        check_overflow(found, foundindex);
        if (c >= 0) {
            found[foundindex++] = c;
        }
    }
    token = new_operand(pdf_string);
    token->value = foundindex;
    token->string = found;
    return token;
}

static Token *_parseNumber(scannerdata * self, int c)
{
    double value = 0;
    pdf_token_type type = pdf_integer;
    int isfraction = 0;
    int isnegative = 0;
    int i = 0;
    Token *token = NULL;
    if (c == '-') {
        isnegative = 1;
        c = streamGetChar(self);
    }
    if (c == '.') {
        type = pdf_real;
        isfraction = 1;
    } else {
        value = c - '0';
    }
    c = streamLookChar(self);
    if ((c >= '0' && c <= '9') || c == '.') {
    c = streamGetChar(self);
    while (1) {
        if (c == '.') {
            type = pdf_real;
            isfraction = 1;
        } else {
            i = c - '0';
            if (isfraction > 0) {
                value = value + (i / (pow(10.0, isfraction)));
                isfraction = isfraction + 1;
            } else {
                value = (value * 10) + i;
            }
        }
        c = streamLookChar(self);
        if (!((c >= '0' && c <= '9') || c == '.'))
            break;
        c = streamGetChar(self);
    }
    }
    if (isnegative) {
        value = -value;
    }
    token = new_operand(type);
    token->value = value;
    return token;
}

static Token *_parseName(scannerdata * self, int c)
{
    Token *token = NULL;
    define_buffer(found);
    c = streamGetChar(self);
    while (1) {
        check_overflow(found, foundindex);
        found[foundindex++] = c;
        c = streamLookChar(self);
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t' ||
            c == '/' || c == '[' || c == '(' || c == '<')
            break;
        c = streamGetChar(self);
    }
    token = new_operand(pdf_name);
    token->string = found;
    token->value = strlen(found);
    return token;
}

#define hexdigit(c)	\
  (c>= '0' && c<= '9') ? (c - '0') : ((c>= 'A' && c<= 'F') ? (c - 'A' + 10) : (c - 'a' + 10))

static Token *_parseHexstring(scannerdata * self, int c)
{
    int isodd = 1;
    int hexval = 0;
    Token *token = NULL;
    define_buffer(found);
    while (c != '>') {
    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
        if (isodd == 1) {
            int v = hexdigit(c);
            hexval = 16 * v;
        } else {
            hexval += hexdigit(c);
            check_overflow(found, foundindex);
            found[foundindex++] = hexval;
        }
        isodd = (isodd == 1 ? 0 : 1);
    }
        c = streamGetChar(self);
    }
    token = new_operand(pdf_string);
    token->value = foundindex;
    token->string = found;
    return token;
}

#define pdf_isspace(a) (a == '\0' || a == ' ' || a == '\n' || a == '\r' || a == '\t' || a == '\v')

/*tex this is rather horrible */

static Token *_parseInlineImage(scannerdata * self, int c)
{
    Token *token = NULL;
    define_buffer(found);
    if (c == ' ') {
        /*tex first space can be ignored */
        c = streamGetChar(self);
    }
    check_overflow(found, foundindex);
    found[foundindex++] = c;
    while (1) {
        c = streamLookChar(self);
        if (c == 'E'
            && (found[foundindex - 1] == '\n'
            || found[foundindex - 1] == '\r')) {
            c = streamGetChar(self);
            check_overflow(found, foundindex);
            found[foundindex++] = c;
            c = streamLookChar(self);
            if (c == 'I') {
                c = streamGetChar(self);
                check_overflow(found, foundindex);
                found[foundindex++] = c;
                c = streamLookChar(self);
                if (pdf_isspace(c)) {
                    /*tex |I| */
                    found[--foundindex] = '\0';
                    /*tex |E| */
                    found[--foundindex] = '\0';
                    /*tex remove end-of-line before |EI| */
                    if (found[foundindex - 1] == '\n') {
                        found[--foundindex] = '\0';
                    }
                    if (found[foundindex - 1] == '\r') {
                        found[--foundindex] = '\0';
                    }
                    break;
                } else {
                    c = streamGetChar(self);
                    check_overflow(found, foundindex);
                    found[foundindex++] = c;
                }
            } else {
                c = streamGetChar(self);
                check_overflow(found, foundindex);
                found[foundindex++] = c;
            }
        } else {
            c = streamGetChar(self);
            check_overflow(found, foundindex);
            found[foundindex++] = c;
        }
    }
    token = new_operand(pdf_string);
    token->value = foundindex;
    token->string = found;
    return token;
}

static Token *_parseOperator(scannerdata * self, int c)
{
    define_buffer(found);
    while (1) {
        check_overflow(found, foundindex);
        found[foundindex++] = c;
        c = streamLookChar(self);
        if ((c < 0) || (c == ' ' || c == '\n' || c == '\r' || c == '\t' ||
                c == '/' || c == '[' || c == '(' || c == '<'))
            break;
        c = streamGetChar(self);
    }
    /*tex |print| (found) */
    if (strcmp(found, "ID") == 0) {
        self->_ininlineimage = 1;
    }
    if (strcmp(found, "false") == 0) {
        Token *token = new_operand(pdf_boolean);
        token->value = 0;
        free(found);
        return token;
    } else if (strcmp(found, "true") == 0) {
        Token *token = new_operand(pdf_boolean);
        token->value = 1.0;
        free(found);
        return token;
    } else {
        Token *token = new_operand(pdf_operator);
        token->string = found;
        return token;
    }
}

static Token *_parseComment(scannerdata * self, int c)
{
    do {
        c = streamGetChar(self);
    } while (c != '\n' && c != '\r' && c != -1);
    return _parseToken(self, streamGetChar(self));
}

static Token *_parseLt(scannerdata * self, int c)
{
    c = streamGetChar(self);
    if (c == '<') {
        return new_operand(pdf_startdict);
    } else {
        return _parseHexstring(self, c);
    }
}

static Token *_parseGt(scannerdata * self, int c)
{
    c = streamGetChar(self);
    if (c == '>') {
        return new_operand(pdf_stopdict);
    } else {
        fprintf(stderr, "stray > in stream");
        return NULL;
    }
}

static Token *_parseError(int c)
{
    fprintf(stderr, "stray %c [%d] in stream", c, c);
    return NULL;
}

static Token *_parseStartarray(void)
{
    return new_operand(pdf_startarray);
}

static Token *_parseStoparray(void)
{
    return new_operand(pdf_stoparray);
}

static Token *_parseToken(scannerdata * self, int c)
{
    if (self->_ininlineimage == 1) {
        self->_ininlineimage = 2;
        return _parseInlineImage(self, c);
    } else if (self->_ininlineimage == 2) {
        Token *token = NULL;
        self->_ininlineimage = 0;
        token = new_operand(pdf_operator);
        token->string = strdup("EI");
        return token;
    }
    if (c < 0)
        return NULL;
    switch (c) {
        case '(':
            return _parseString(self, c);
            break;
        case ')':
            return _parseError(c);
            break;
        case '[':
            return _parseStartarray();
            break;
        case ']':
            return _parseStoparray();
            break;
        case '/':
            return _parseName(self, c);
            break;
        case '<':
            return _parseLt(self, c);
            break;
        case '>':
            return _parseGt(self, c);
            break;
        case '%':
            return _parseComment(self, c);
            break;
        case ' ':
        case '\r':
        case '\n':
        case '\t':
            return _parseSpace(self);
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
        case '.':
            return _parseNumber(self, c);
            break;
        default:
            if (c <= 127) {
                return _parseOperator(self, c);
            } else {
                return _parseError(c);
            }
    }
}

static int scanner_scan(lua_State * L)
{
    Token *token;
    scannerdata *self;
    if (lua_gettop(L) != 3) {
        return 0;
    }
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    self = scanner_push(L);
    memset(self, 0, sizeof(scannerdata));
    self->_operandstack = (Token **) priv_xmalloc(MAXOPERANDS * sizeof(Token));
    memset(self->_operandstack, 0, (MAXOPERANDS * sizeof(Token)));
    /*tex stack slot 4 = self */
    self->uses_stream = 1;
    if (lua_type(L, 1) == LUA_TSTRING) {
      /*tex
            We could make a temporary copy on the stack (or in the registry)
            which saves memory.
      */
      char *buf = NULL;
      const char *s = lua_tolstring(L, 1, &self->size);
      if (s==NULL){
	fprintf(stderr,"fatal: cannot convert the token to string.");
	exit(1);
      }
      buf = priv_xmalloc(self->size+1);
      buf[self->size]='\0';
      self->uses_stream = 0;
      memcpy(buf,s,self->size);
      self->buffer = buf;
    } else if (lua_type(L, 1) == LUA_TTABLE) {
        udstruct *uin;
        void *ud;
        int i = 1;
        while (1) {
            lua_rawgeti(L, 1, i);
            if (lua_type(L, -1) == LUA_TUSERDATA) {
                ud = luaL_checkudata(L, -1, PDFE_METATABLE_STREAM);
                if (ud != NULL) {
                    ObjectList *rover = NULL;
                    ObjectList *item = NULL;
                    uin = (udstruct *) ud;
                    rover = self->_streams;
                    item = (ObjectList *) priv_xmalloc(sizeof(ObjectList));
                    item->stream = ((ppstream *) uin->d);
                    item->next = NULL;
                    if (!rover) {
                        rover = item;
                        self->_streams = rover;
                    } else {
                        while (rover->next)
                            rover = rover->next;
                        rover->next = item;
                    }
                }
            } else {
                ObjectList *rover = self->_streams;
                self->_stream = rover->stream;
                self->_streams = rover->next;
                free(rover);
                lua_pop(L, 1);
                break;
            }
            lua_pop(L, 1);
            i++;
        }
    } else {
        udstruct *uin;
        void *ud;
        luaL_checktype(L, 1, LUA_TUSERDATA);
        ud = luaL_checkudata(L, 1, PDFE_METATABLE_STREAM);
        if (ud != NULL) {
            uin = (udstruct *) ud;
            self->_stream = ((ppstream *) uin->d);
        } else {
            ud = luaL_checkudata(L, 1, PDFE_METATABLE_ARRAY);
            if (ud != NULL) {
            ObjectList *rover = NULL;
            pparray *array = NULL;
            int count;
            int i;
            uin = (udstruct *) ud;
            array = (pparray *) uin->d;
            count = array->size;
            for (i = 0; i < count; i++) {
                ppobj *obj = pparray_at(array, i);
                if (obj->type == PPSTREAM) {
                    ObjectList *rover = self->_streams;
                    ObjectList *item =
                        (ObjectList *)
                        priv_xmalloc(sizeof(ObjectList));
                    item->stream = obj->stream;
                    item->next = NULL;
                    if (!rover) {
                        rover = item;
                        self->_streams = rover;
                    } else {
                        while (rover->next)
                        rover = rover->next;
                        rover->next = item;
                    }
                }
            }
            rover = self->_streams;
            self->_stream = rover->stream;
            self->_streams = rover->next;
            }
        }
    }
    streamReset(self);
    token = _parseToken(self, streamGetChar(self));
    while (token) {
        if (token->type == pdf_operator) {
            lua_pushstring(L, token->string);
            free_token(token);
            /*tex fetch operator table */
            lua_rawget(L, 2);
            if (lua_isfunction(L, -1)) {
                lua_pushvalue(L, 4);
                lua_pushvalue(L, 3);
                (void) lua_call(L, 2, 0);
            } else {
                /*tex nil */
                lua_pop(L, 1);
            }
            clear_operand_stack(self, 0);
        } else {
            push_operand(self, token);
        }
        if (self->uses_stream) {
            if (!self->_stream) {
                break;
            }
        } else {
            if (self->buffer == NULL) {
                break;
            }
        }
        token = _parseToken(self, streamGetChar(self));
    }
    /*tex wrap up */
    if (self->_stream) {
        streamClose(self);
    }
    clear_operand_stack(self, 0);
    free(self->_operandstack);
    return 0;
}

static int scanner_done(lua_State * L)
{
    int c;
    scannerdata *self = scanner_check(L, 1);
    while ((c = streamGetChar(self)) >= 0);
    return 0;
}

/*tex here are the stack popping functions, and their helpers */

static void operandstack_backup(scannerdata * self)
{
    int i = self->_nextoperand - 1;
    int balance = 0;
    int backupstart = 0;
    int backupstop = self->_operandstack[i]->type;
    if (backupstop == pdf_stopdict) {
        backupstart = pdf_startdict;
    } else if (backupstop == pdf_stoparray) {
        backupstart = pdf_startarray;
    } else {
        return;
    }
    for (; i >= 0; i--) {
        if (self->_operandstack[i]->type == backupstop) {
            balance++;
        } else if (self->_operandstack[i]->type == backupstart) {
            balance--;
        }
        if (balance == 0) {
            break;
        }
    }
    self->_nextoperand = i + 1;
}

static void push_array(lua_State * L, scannerdata * self)
{
    /*tex nesting tracking */
    int balance = 1;
    /*tex \LUA\ array index */
    int index = 1;
    Token *token = self->_operandstack[self->_nextoperand++];
    lua_newtable(L);
    while (token) {
    if (token->type == pdf_stoparray)
        balance--;
    if (token->type == pdf_startarray)
        balance++;
    if (!balance) {
        break;
    } else {
        push_token(L, self);
        lua_rawseti(L, -2, index++);
    }
    token = self->_operandstack[self->_nextoperand++];
    }
}


static void push_dict(lua_State * L, scannerdata * self)
{
    /*tex nesting tracking */
    int balance = 1;
    /*tex toggle between \LUA\ value and \LUA\ key */
    int needskey = 1;
    Token *token = self->_operandstack[self->_nextoperand++];
    lua_newtable(L);
    while (token) {
    if (token->type == pdf_stopdict)
        balance--;
    if (token->type == pdf_startdict)
        balance++;
    if (!balance) {
        break;
    } else if (needskey) {
        lua_pushlstring(L, token->string, token->value);
        needskey = 0;
    } else {
        push_token(L, self);
        needskey = 1;
        lua_rawset(L, -3);
    }
    token = self->_operandstack[self->_nextoperand++];
    }
}

const char *typenames[pdf_stopdict + 1] = {
    "unknown", "integer", "real", "boolean", "name", "operator",
    "string", "array", "array", "dict", "dict"
};

static void push_token(lua_State * L, scannerdata * self)
{
    Token *token = self->_operandstack[self->_nextoperand - 1];
    lua_createtable(L, 2, 0);
    lua_pushstring(L, typenames[token->type]);
    lua_rawseti(L, -2, 1);
    if (token->type == pdf_string || token->type == pdf_name) {
        lua_pushlstring(L, token->string, token->value);
    } else if (token->type == pdf_real || token->type == pdf_integer) {
        /*tex This is an integer or float. */
        lua_pushnumber(L, token->value);
    } else if (token->type == pdf_boolean) {
        lua_pushboolean(L, (int) token->value);
    } else if (token->type == pdf_startarray) {
        push_array(L, self);
    } else if (token->type == pdf_startdict) {
        push_dict(L, self);
    } else {
        lua_pushnil(L);
    }
    lua_rawseti(L, -2, 2);
}

static int scanner_popsingular(lua_State * L, int token_type)
{
    Token *token = NULL;
    /*tex this keeps track of how much of the operand stack needs deleting: */
    int clear = 0;
    scannerdata *self = scanner_check(L, 1);
    if (self->_nextoperand == 0) {
        return 0;
    }
    clear = self->_nextoperand - 1;
    token = self->_operandstack[self->_nextoperand - 1];
    if (token == NULL || (token->type != token_type)) {
        return 0;
    }
    /*tex
        The simple cases can be written out directly, but dicts and
        arrays are better done via the recursive function.
    */
    if (token_type == pdf_stoparray || token_type == pdf_stopdict) {
        operandstack_backup(self);
        clear = self->_nextoperand - 1;
        push_token(L, self);
        lua_rawgeti(L, -1, 2);
    } else if (token_type == pdf_real || token_type == pdf_integer) {
        /*tex the number can be an integer or float */
        lua_pushnumber(L, token->value);
    } else if (token_type == pdf_boolean) {
        lua_pushboolean(L, (int) token->value);
    } else if (token_type == pdf_name || token_type == pdf_string) {
        lua_pushlstring(L, token->string, token->value);
    } else {
        return 0;
    }
    clear_operand_stack(self, clear);
    return 1;
}

static int scanner_popanything(lua_State * L)
{
    Token *token = NULL;
    /*tex how much of the operand stack needs deleting: */
    int clear = 0;
    int token_type;
    scannerdata *self = scanner_check(L, 1);
    if (self->_nextoperand == 0) {
        return 0;
    }
    clear = self->_nextoperand - 1;
    token = self->_operandstack[self->_nextoperand - 1];
    if (token == NULL) {
        return 0;
    }
    token_type = token->type;
    /*tex
        The simple cases can be written out directly, but dicts and
        arrays are better done via the recursive function.
    */
    if (token_type == pdf_stoparray || token_type == pdf_stopdict) {
        operandstack_backup(self);
        clear = self->_nextoperand - 1;
        push_token(L, self);
    } else {
        push_token(L, self);
    }
    clear_operand_stack(self, clear);
    return 1;
}

static int scanner_popnumber(lua_State * L)
{
    if (scanner_popsingular(L, pdf_real))
        return 1;
    if (scanner_popsingular(L, pdf_integer))
        return 1;
    lua_pushnil(L);
    return 1;
}

static int scanner_popboolean(lua_State * L)
{
    if (scanner_popsingular(L, pdf_boolean))
        return 1;
    lua_pushnil(L);
    return 1;
}

static int scanner_popstring(lua_State * L)
{
    if (scanner_popsingular(L, pdf_string))
        return 1;
    lua_pushnil(L);
    return 1;
}

static int scanner_popname(lua_State * L)
{
    if (scanner_popsingular(L, pdf_name))
        return 1;
    lua_pushnil(L);
    return 1;
}

static int scanner_poparray(lua_State * L)
{
    if (scanner_popsingular(L, pdf_stoparray))
        return 1;
    lua_pushnil(L);
    return 1;
}

static int scanner_popdictionary(lua_State * L)
{
    if (scanner_popsingular(L, pdf_stopdict))
        return 1;
    lua_pushnil(L);
    return 1;
}

static int scanner_popany(lua_State * L)
{
    if (scanner_popanything(L))
        return 1;
    lua_pushnil(L);
    return 1;
}

static const luaL_Reg scannerlib_meta[] = {
    {0, 0}
};

static const struct luaL_Reg scannerlib_m[] = {
    { "done",          scanner_done },
    { "pop",           scanner_popany },
    { "popnumber",     scanner_popnumber },
    { "popname",       scanner_popname },
    { "popstring",     scanner_popstring },
    { "poparray",      scanner_poparray },
    { "popdictionary", scanner_popdictionary },
    { "popboolean",    scanner_popboolean },
    /*tex For old times sake: */
    { "popNumber",     scanner_popnumber },
    { "popName",       scanner_popname },
    { "popString",     scanner_popstring },
    { "popArray",      scanner_poparray },
    { "popDict",       scanner_popdictionary },
    { "popBool",       scanner_popboolean },
    /*tex Sentinel: */
    { NULL,            NULL }
};

static const luaL_Reg scannerlib[] = {
    { "scan", scanner_scan },
    /*tex Sentinel: */
    { NULL,   NULL }
};

LUALIB_API int luaopen_pdfscanner(lua_State * L)
{
    luaL_newmetatable(L, SCANNER);
    luaL_openlib(L, 0, scannerlib_meta, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_openlib(L, NULL, scannerlib_m, 0);
    luaL_openlib(L, "pdfscanner", scannerlib, 0);
    return 1;
}
