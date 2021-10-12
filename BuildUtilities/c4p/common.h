/* common.h: common definitions for C4P                 -*- C++ -*-

   Copyright (C) 1991-2021 Christian Schenk

   This file is part of C4P.

   C4P is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
   
   C4P is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.
   
   You should have received a copy of the GNU General Public License
   along with C4P; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#pragma once

#include <stack>
#include <string>
#include <vector>

#define T_(x) x

enum pascal_type
{
  UNKNOWN_TYPE,
  INTEGER_TYPE,
  LONG_INTEGER_TYPE,
  BOOLEAN_TYPE,
  CHARACTER_TYPE,
  REAL_TYPE,
  LONG_REAL_TYPE,
  SUBRANGE_NODE,
  ARRAY_NODE,
  RECORD_NODE,
  STRING_TYPE,
  POINTER_NODE,
  NAMED_TYPE_NODE,
  FUNCTION_TYPE,
  FIELD_LIST_NODE,
  VARIANT_FIELD_LIST_NODE,
  RECORD_SECTION_NODE,
  VARIANT_NODE,
  DECLARATOR_NODE,
  PARAMETER_NODE,
  PROTOTYPE_NODE,
  FILE_NODE,
};

const size_t BUF_SIZE = 8192;
const size_t CHAR_POOL_SIZE = 20000;
const size_t TYPE_TABLE_SIZE = 20000;
const size_t HASH_PRIME = 211;
const size_t MAX_SYMBOLS = 10000;

typedef unsigned long DWORD;
typedef unsigned short WORD;

// must be in sync with miktex/C4P/C4p.h
typedef int C4P_integer;
#if defined(C4P_REAL_IS_DOUBLE)
typedef double C4P_real;
#else
typedef float C4P_real;
#endif

union value_t
{
  C4P_integer ivalue;
  C4P_real fvalue;
};

typedef WORD SYMBOL_PTR;
typedef WORD STRING_PTR;

struct symbol_t
{
  unsigned s_kind;
  SYMBOL_PTR s_key;
  const char * s_repr;
  pascal_type s_type;
  void * s_type_ptr;
  unsigned s_block_level;
  value_t s_value;
  const char * s_translated_type;
  SYMBOL_PTR s_next;
  unsigned s_flags;
#define S_BY_REFERENCE 0x0001
#define S_FAST 0x0002
#define S_PREDEFINED 0x0004
};

const size_t C_FILE_NUM = 0;
const size_t H_FILE_NUM = 1;
const size_t DEF_FILE_NUM = 2;

const size_t NO_MARK = 12345;

struct file_node
{
  pascal_type type;
  void * type_ptr;
};

struct declarator_node
{
  symbol_t * name;
  declarator_node * next;
};

struct parameter_node
{
  const char * name;
  symbol_t * type_symbol;
  bool by_reference;
  parameter_node * next;
};

struct prototype_node
{
  symbol_t * name;
  parameter_node * formal_parameter;
  symbol_t * result_type;
};

struct named_type_node
{
  symbol_t * name;
};

struct array_node
{
  pascal_type component_type;
  void * component_type_ptr;
  C4P_integer lower_bound;
  C4P_integer upper_bound;
};

struct pointer_node
{
  pascal_type component_type;
  void * component_type_ptr;
  array_node * array_node_ptr;
};

struct subrange_node
{
  C4P_integer lower_bound;
  C4P_integer upper_bound;
};

struct record_section_node
{
  declarator_node * name;
  pascal_type type;
  void * type_ptr;
  record_section_node * next;
};

struct variant_field_list_node;

struct variant_node
{
  symbol_t * pseudo_name;
  variant_field_list_node * variant_field_list;
};

struct variant_field_list_node
{
  symbol_t * pseudo_name;
  record_section_node * fixed_part;
  variant_node * variant_part;
  variant_field_list_node * next;
};

struct field_list_node
{
  record_section_node * fixed_part;
  variant_node * variant_part;
};

struct record_node
{
  field_list_node * field_list;
};

void c4p_error(const char *, ...);
void c4p_warning(const char *, ...);
char * strcpye(char *, const char *);
void * new_type_node(pascal_type, ...);
void declare_var_list(declarator_node *, unsigned, unsigned, pascal_type, void *);
void define_type(symbol_t *, unsigned, pascal_type, void *);
pascal_type translate_field_name(const char *, record_node *, void **);
void generate_routine_head(prototype_node *);
void mark_type_table();
void unmark_type_table();
pascal_type coerce(pascal_type, pascal_type);
void open_name_file();
void close_name_file();
void begin_routine(prototype_node *, unsigned);
void end_routine(unsigned);
void add_loner(const char *);
void begin_new_c_file(const char *, int);
void open_header_file();
void close_header_file();
void open_def_file();
void close_def_file();
void symtab_init();
void new_mapping(const char *, const char *);
symbol_t * new_symbol_instance(const char *);
symbol_t * lookup(const char *);
void mark_symbol_table();
void mark_string_table();
void unmark_symbol_table();
void unmark_string_table();
STRING_PTR new_string(const char * s);
const char * get_string(STRING_PTR);
symbol_t * new_pseudo_symbol();
symbol_t * define_symbol(symbol_t *, unsigned, unsigned, pascal_type, void *, value_t *);
bool is_fast_var(const char *);
void new_fast_var(const char *);
const char * subrange(C4P_integer, C4P_integer);
void declare_fast_var_macro(unsigned);
void forget_fast_vars();
void remember_fast_var(const char *);
void new_type(const char *, pascal_type, void *, const char *);
void new_constant(const char* name, const char* type, C4P_integer value);
void new_string_constant(const char* name, const char* value);
std::pair<std::string, std::string> ParseNameValue(const std::string& s);

extern symbol_t * prog_symbol;
extern unsigned curly_brace_level;
extern unsigned block_level;
extern int n_fast_vars;
extern unsigned extra_indent;
extern unsigned c_file_line_count;
extern unsigned max_lines_per_c_file;
extern std::string pascal_file_name;
extern std::string c_file_name;
extern std::string h_file_name;
extern std::string include_filename;
extern std::string c_ext;
extern bool one_c_file;
extern bool dll_flag;
extern std::string class_name;
extern std::string class_name_scope;
extern std::string base_class_name;
extern std::string class_include;
extern bool macroizing;
extern std::string var_struct_name;
extern std::string var_struct_class_name;
extern std::string var_struct_base_class_name;
extern std::string entry_name;
extern std::string def_filename;
extern int auto_exit_label;
extern bool auto_exit_label_flag;
extern std::vector<std::string> using_namespace;
extern std::string var_name_prefix;
extern bool chars_are_unsigned;
extern std::string name_space;
extern bool emit_optimize_pragmas;
extern bool legacy_flag;
extern std::string integer_literal_suffix;
extern bool relational_cast_expressions;
extern bool other_cast_expressions;

int yyparse();
void yyerror(const char *);
int yylex();

extern char * yytext;
extern "C" int yywrap();
