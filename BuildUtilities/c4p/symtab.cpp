/**
 * @file symtab.cpp
 * @author Christian Schenk
 * @brief Symbol table
 *
 * @copyright Copyright © 1991-2022 Christian Schenk
 *
 * This file is part of C4P.
 *
 * C4P is licensed under GNU General Public License version 2 or any later
 * version.
 */

#include <limits>

#include <cctype>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "common.h"
#include "gram.h"
#include "output.h"

using namespace std;

void new_keyword(const char* , unsigned);
void new_variable(const char*, const char*);
void new_function(const char*, const char*);
void new_procedure(const char*);
void new_build_in(const char*, unsigned);

#define NIL_SYMBOL (MAX_SYMBOLS + 1)

namespace
{
    char char_pool[CHAR_POOL_SIZE];
    STRING_PTR string_ptr;
    STRING_PTR string_mark;
    SYMBOL_PTR hash_table[HASH_PRIME];
    symbol_t symbol_table[MAX_SYMBOLS];
    SYMBOL_PTR symbol_ptr;
    SYMBOL_PTR symbol_mark;
    unsigned long counter;
}

STRING_PTR new_string(const char* s)
{
    int l = strlen(s) + 1;
    STRING_PTR ret = string_ptr;
    if (string_ptr + l > CHAR_POOL_SIZE)
    {
        c4p_error("internal error: must increase `CHAR_POOL_SIZE'");
    }
    strcpy(&char_pool[string_ptr], s);
    string_ptr += l;
    return ret;
}

const char* get_string(STRING_PTR s)
{
    if (s >= string_ptr)
    {
        c4p_error("internal error: get_string: string pointer out of range");
    }
    return &char_pool[s];
}

SYMBOL_PTR hash_func(const char* id)
{
    const char* p = id;
    unsigned long h = 0;
    unsigned long g;
    for (; *p; ++p)
    {
        h = (h << 4) + *p;
        g = h & 0xf000000;
        if (g)
        {
            h ^= (g >> 24);
            h |= 0x0ffffff;
        }
    }
    return h % HASH_PRIME;
}

symbol_t* new_symbol_instance(const char* id)
{
    SYMBOL_PTR h = hash_func(id);
    SYMBOL_PTR front = hash_table[h];
    if (symbol_ptr == MAX_SYMBOLS)
    {
        c4p_error("internal error: must increase `MAX_SYMBOLS'");
    }
    symbol_t* new_symbol = &symbol_table[symbol_ptr];
    hash_table[h] = symbol_ptr;
    new_symbol->s_kind = UNDEFINED_IDENTIFIER;
    new_symbol->s_next = front;
    new_symbol->s_type = UNKNOWN_TYPE;
    new_symbol->s_key = symbol_ptr;
    new_symbol->s_repr = id;
    new_symbol->s_flags = 0;
    new_symbol->s_translated_type = 0;
    ++symbol_ptr;
    return new_symbol;
}

symbol_t* insert(const char* id, SYMBOL_PTR h)
{
    SYMBOL_PTR front = hash_table[h];
    if (symbol_ptr == MAX_SYMBOLS)
    {
        c4p_error("internal error: must increase `MAX_SYMBOLS'");
    }
    symbol_t* new_symbol = &symbol_table[symbol_ptr];
    hash_table[h] = symbol_ptr;
    new_symbol->s_kind = UNDEFINED_IDENTIFIER;
    new_symbol->s_block_level = block_level;
    new_symbol->s_next = front;
    new_symbol->s_type = UNKNOWN_TYPE;
    new_symbol->s_key = symbol_ptr;
    new_symbol->s_repr = &char_pool[string_ptr];
    new_symbol->s_flags = 0;
    new_string(id);
    ++symbol_ptr;
    return new_symbol;
}

symbol_t* lookup(const char* id)
{
    SYMBOL_PTR h = hash_func(id);
    SYMBOL_PTR s = hash_table[h];
    symbol_t* possible_result = nullptr;
    for (; s < NIL_SYMBOL; s = symbol_table[s].s_next)
    {
        if (strcmp(symbol_table[s].s_repr, id) == 0) /* FIXME: ignore case */
        {
            possible_result = &symbol_table[s];
            if (possible_result->s_kind == C_RESERVED)
            {
                // RECURSION
                return lookup(const_cast<const char*>(reinterpret_cast<char*>(possible_result->s_type_ptr)));
            }
            if (possible_result->s_kind != FIELD_IDENTIFIER)
            {
                return possible_result;
            }
        }
    }
    if (possible_result != nullptr)
    {
        return possible_result;
    }
    return insert(id, h);
}

void mark_symbol_table()
{
    symbol_mark = symbol_ptr;
}

void mark_string_table()
{
    string_mark = string_ptr;
}

void unmark_symbol_table()
{
    SYMBOL_PTR h = HASH_PRIME;
    SYMBOL_PTR s;
    while (h--)
    {
        s = hash_table[h];
        while (s >= symbol_mark && s != NIL_SYMBOL)
        {
            if (symbol_table[s].s_kind == CONSTANT_IDENTIFIER)
            {
                cppout.out_s("#undef " + std::string(symbol_table[s].s_repr) + "\n");
            }
            s = symbol_table[s].s_next;
        }
        hash_table[h] = s;
    }
    symbol_ptr = symbol_mark;
}

void unmark_string_table()
{
    string_ptr = string_mark;
}

void symtab_init()
{
    for (unsigned i = 0; i < HASH_PRIME; ++i)
    {
        hash_table[i] = NIL_SYMBOL;
    }
    array_node* arr;

    /* Pascal keywords */
    new_keyword("and", AND);
    new_keyword("array", ARRAY);
    new_keyword("begin", BEGIn);
    new_keyword("case", CASE);
    new_keyword("const", CONST);
    new_keyword("div", DIV);
    new_keyword("do", DO);
    new_keyword("downto", DOWNTO);
    new_keyword("else", ELSE);
    new_keyword("end", END);
    new_keyword("file", FILe);
    new_keyword("for", FOR);
    new_keyword("forward", FORWARD);
    new_keyword("function", FUNCTION);
    new_keyword("goto", GOTO);
    new_keyword("if", IF);
    new_keyword("in", IN);
    new_keyword("label", LABEL);
    new_keyword("mod", MOD);
    new_keyword("nil", NIL);
    new_keyword("not", NOT);
    new_keyword("of", OF);
    new_keyword("or", OR);
    new_keyword("others", OTHERS);
    new_keyword("packed", PACKED);
    new_keyword("procedure", PROCEDURE);
    new_keyword("program", PROGRAM);
    new_keyword("record", RECORD);
    new_keyword("repeat", REPEAT);
    new_keyword("set", SET);
    new_keyword("then", THEN);
    new_keyword("to", TO);
    new_keyword("type", TYPE);
    new_keyword("until", UNTIL);
    new_keyword("var", VAR);
    new_keyword("while", WHILE);
    new_keyword("with", WITH);

    /* hide C/C++ keywords */
    new_mapping("auto", "c4p_auto");
    new_mapping("break", "c4p_break");
    new_mapping("bool", "c4p_bool");
    new_mapping("class", "c4p_class");
    new_mapping("continue", "c4p_continue");
    new_mapping("default", "c4p_default");
    new_mapping("double", "c4p_double");
    new_mapping("enum", "c4p_enum");
    new_mapping("float", "c4p_float");
    new_mapping("free", "c4p_free");
    new_mapping("int", "c4p_int");
    new_mapping("long", "c4p_long");
    new_mapping("main", "c4p_main");
    new_mapping("nullptr", "c4p_nullptr");
    new_mapping("register", "c4p_register");
    new_mapping("return", "c4p_return");
    new_mapping("short", "c4p_short");
    new_mapping("signed", "c4p_signed");
    new_mapping("sizeof", "c4p_sizeof");
    new_mapping("static", "c4p_static");
    new_mapping("struct", "c4p_struct");
    new_mapping("switch", "c4p_switch");
    new_mapping("template", "c4p_template");
    new_mapping("typedef", "c4p_typedef");
    new_mapping("union", "c4p_union");
    new_mapping("unsigned", "c4p_unsigned");
    new_mapping("void", "c4p_void");
    new_mapping("volatile", "c4p_volatile");

    /* hide ambigous names */
    new_mapping("byte", "c4p_byte");
    new_mapping("link", "c4p_link");
    new_mapping("index", "c4p_index");

    /* Pascal types */
    new_type("integer", INTEGER_TYPE, nullptr, "C4P_integer");
    new_type("longinteger", LONG_INTEGER_TYPE, nullptr, "C4P_longinteger");
    new_type("char", CHARACTER_TYPE, nullptr, nullptr);
    new_type("boolean", BOOLEAN_TYPE, nullptr, "C4P_boolean");
    new_type("real", REAL_TYPE, nullptr, "C4P_real");
    new_type("longreal", LONG_REAL_TYPE, nullptr, "C4P_longreal");
    new_type("text", FILE_NODE, new_type_node(FILE_NODE, CHARACTER_TYPE, 0), "C4P_text");

    new_type("c4pstring", STRING_TYPE, nullptr, nullptr);
    arr = reinterpret_cast<array_node*>(new_type_node(ARRAY_NODE, 0L, 0L));
    arr->component_type = STRING_TYPE;
    new_type("c4pstrarr", ARRAY_NODE, arr, nullptr);
    new_type("c4pjmpbuf", STRING_TYPE, nullptr, nullptr);

    new_variable("input", "text");
    new_variable("output", "text");
    new_variable("c4perroroutput", "text");

    new_variable("c4pargc", "integer");
    new_variable("c4pargv", "c4pstrarr");
    new_variable("c4pplen", "integer");
    new_variable("c4ppline", "c4pstring");

    new_variable("c4phour", "integer");
    new_variable("c4pminute", "integer");
    new_variable("c4psecond", "integer");
    new_variable("c4pday", "integer");
    new_variable("c4pmonth", "integer");
    new_variable("c4pyear", "integer");

    new_variable("c4pcur", "integer"); /* SEEK_CUR */
    new_variable("c4pend", "integer"); /* SEEK_END */
    new_variable("c4pset", "integer"); /* SEEK_SET */
    new_variable("c4prmode", "integer"); /* "r" */
    new_variable("c4pwmode", "integer"); /* "w" */
    new_variable("c4prbmode", "integer"); /* "rb" */
    new_variable("c4pwbmode", "integer"); /* "wb" */

    new_mapping("abs", "c4p_abs");
    new_function("c4p_abs", "integer");
    new_function("chr", "char");
    new_function("eof", "boolean");
    new_function("eoln", "boolean");
    new_function("odd", "boolean");
    new_function("ord", "integer");
    new_function("round", "integer");
    new_function("trunc", "integer");

    new_procedure("get");
    new_procedure("put");
    new_procedure("reset");
    new_procedure("rewrite");

    new_function("c4pferror", "integer"); /* ferror () */
    new_function("c4pfopen", "boolean"); /* fopen () */
    new_function("c4ptryfopen", "boolean"); /* fopen () */
    new_function("c4pftell", "integer"); /* ftell () */
    new_function("c4pinteger", "integer"); /* (integer) */
    new_function("c4pptr", "integer"); /* & */
    new_function("c4psetjmp", "integer"); /* setjmp () */
    new_function("c4pstrlen", "integer");

    new_procedure("c4pbufwrite"); /* fwrite () */
    new_procedure("c4p_break");  /* fflush () */
    new_procedure("c4pexit");    /* exit () */
    new_procedure("c4pfclose");  /* fclose () */
    new_procedure("c4pfopen");   /* fopen () */
    new_procedure("c4pfseek");   /* fseek () */
    new_procedure("c4parrcpy");  /* memcpy () */
    new_procedure("c4pmemcpy");  /* memcpy () */
    new_procedure("c4pstrcpy");  /* strcpy () */
    new_procedure("c4plongjmp"); /* longjmp () */
    new_procedure("c4pgetc");    /* getc () */
    new_procedure("c4pputc");    /* putc () */
    new_procedure("c4pincr");    /* ++ */
    new_procedure("c4pdecr");    /* -- */
    new_procedure("c4pmget");
    new_procedure("c4preturn");
    new_procedure("c4pbreakloop"); /* break */

    new_build_in("read", READ);
    new_build_in("readln", READLN);
    new_build_in("write", WRITE);
    new_build_in("writeln", WRITELN);

    new_constant("false", "boolean", 0);
    new_constant("true", "boolean", 1);
    new_constant("maxint", "integer", std::numeric_limits<C4P_integer>::max());
}

void new_keyword(const char* keyword, unsigned number)
{
    symbol_t* sym = new_symbol_instance(keyword);
    sym->s_block_level = 0;
    sym->s_kind = PASCAL_KEYWORD;
    sym->s_type = (pascal_type)number; // fixme
}

void new_type(const char* type_name, pascal_type type, void* type_ptr, const char* translation)
{
    symbol_t* sym = new_symbol_instance(type_name);
    sym->s_block_level = 0;
    sym->s_kind = TYPE_IDENTIFIER;
    sym->s_type = type;
    sym->s_type_ptr = type_ptr;
    sym->s_translated_type = translation;
    sym->s_flags |= S_PREDEFINED;
}

void new_variable(const char* var_name, const char* type)
{
    symbol_t* sym = new_symbol_instance(var_name);
    symbol_t* tsym = lookup(type);
    sym->s_block_level = 0;
    sym->s_kind = VARIABLE_IDENTIFIER;
    sym->s_type = tsym->s_type;
    sym->s_type_ptr = tsym->s_type_ptr;
    sym->s_flags |= S_PREDEFINED;
}

void new_function(const char* func_name, const char* result_type)
{
    symbol_t* sym = new_symbol_instance(func_name);
    sym->s_block_level = 0;
    sym->s_kind = FUNCTION_IDENTIFIER;
    sym->s_type = PROTOTYPE_NODE;
    sym->s_type_ptr = new_type_node(PROTOTYPE_NODE, sym, 0, lookup(result_type));
}

void new_build_in(const char* name, unsigned number)
{
    symbol_t* sym = new_symbol_instance(name);
    sym->s_block_level = 0;
    sym->s_kind = BUILD_IN_IDENTIFIER;
    sym->s_type = (pascal_type)number; // fixme
}

void new_procedure(const char* proc_name)
{
    symbol_t* sym = new_symbol_instance(proc_name);
    sym->s_block_level = 0;
    sym->s_kind = PROCEDURE_IDENTIFIER;
    sym->s_type = PROTOTYPE_NODE;
    sym->s_type_ptr = new_type_node(PROTOTYPE_NODE, sym, 0, 0);
}

void new_mapping(const char* name, const char* mapped_name)
{
    symbol_t* sym = lookup(name);
    sym->s_kind = C_RESERVED;
    sym->s_type_ptr = (const_cast<void*>(reinterpret_cast<const void*>(get_string(new_string(mapped_name)))));
}

void new_constant(const char* constant_name, const char* type, C4P_integer ivalue)
{
    symbol_t* sym = new_symbol_instance(constant_name);
    symbol_t* tsym = lookup(type);
    sym->s_block_level = 0;
    sym->s_kind = CONSTANT_IDENTIFIER;
    sym->s_type = tsym->s_type;
    sym->s_value.ivalue = ivalue;
    sym->s_flags |= S_PREDEFINED;
}

void new_string_constant(const char* name, const char* value)
{
    symbol_t* sym = new_symbol_instance(name);
    sym->s_block_level = 0;
    sym->s_kind = CONSTANT_IDENTIFIER;
    sym->s_type = STRING_TYPE;
    sym->s_value.ivalue = new_string(value);
    sym->s_flags |= S_PREDEFINED;
}

symbol_t* new_pseudo_symbol()
{
    char pseudo_name[33];
    sprintf(pseudo_name, "c4p_P%lu", counter++);
    return lookup(pseudo_name);
}

symbol_t* define_symbol(symbol_t* sym, unsigned kind, unsigned block_level, pascal_type type, void* type_ptr, value_t* value)
{
    if (sym->s_kind != UNDEFINED_IDENTIFIER)
    {
        if (block_level == sym->s_block_level && sym->s_kind != FIELD_IDENTIFIER)
        {
            c4p_error(fmt::format("`{0}' already defined", sym->s_repr));
        }
        if (sym->s_kind == CONSTANT_IDENTIFIER)
        {
            /* FIXME: should #undefine symbol
                     out_form ("#undef %s\n", sym -> s_repr);
            */
        }
        sym = new_symbol_instance(sym->s_repr);
    }
    sym->s_kind = kind;
    sym->s_block_level = block_level;
    sym->s_type = type;
    sym->s_type_ptr = type_ptr;
    if (value)
    {
        sym->s_value = *value;
    }
    return sym;
}
