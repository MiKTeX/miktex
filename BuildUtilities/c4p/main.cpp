/* main.c: Pascal-to-C Translator                       -*- C++ -*-

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

#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <climits>
#include <cstring>

#include <getopt.h>

#include "common.h"

#include "c4p-version.h"

using namespace std;

namespace {
  string myname;
}

bool auto_exit_label_flag;
bool chars_are_unsigned;
bool dll_flag;
bool macroizing;
bool one_c_file;
bool verbose_flag;
int auto_exit_label;
int n_fast_vars;
string base_class_name;
string c_ext;
string c_file_name;
string class_include;
string class_name;
string class_name_scope;
string def_filename;
string entry_name;
string h_file_name;
string include_filename;
string pascal_file_name;
vector<string> using_namespace;
string var_name_prefix;
string var_struct_base_class_name;
string var_struct_class_name;
string var_struct_name;
unsigned c_file_line_count;
unsigned curly_brace_level;
unsigned extra_indent;
unsigned max_lines_per_c_file;
string name_space;
bool emit_optimize_pragmas;
bool legacy_flag;
string integer_literal_suffix;
bool relational_cast_expressions = false;
bool other_cast_expressions = true;

int yyparse();
extern int yylineno;
extern char * yytext;

int yywrap()
{
  return 1;
}

void yyerror(const char * unused)
{
  fprintf(stderr, T_("%s:%d: parse error before `%s'\n"), pascal_file_name.c_str(), yylineno, yytext);
  exit(1);
}

void c4p_error(const char * fmt, ...)
{
  fprintf(stderr, "%s:%d: ", pascal_file_name.c_str(), yylineno);
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(2);
}

void c4p_warning(const char * fmt, ...)
{
  if (verbose_flag)
  {
    fprintf(stderr, T_("\n%s:%d: warning: "), pascal_file_name.c_str(), yylineno);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
  }
}

void banner()
{
  printf(T_("This is C4P version %s\n"), MIKTEX_COMPONENT_VERSION_STR);
}

void usage()
{
  puts(T_("\
Usage: c4p [options] filename\n\
Options:\n\
  -1 FILENAME, --one FILENAME\n\
  --auto-exit-label=NUM\n\
  --base-class=CLASS\n\
  --chars-are-unsigned\n\
  --class=CLASS\n\
  --class-include=FILENAME\n\
  --constant NAME=VALUE\n\
  --emit-optimize-pragmas\n\
  --entry-name=NAME\n\
  --declare-c-type=NAME\n\
  --var-name-prefix=PREFIX\n\
  --var-struct=NAME\n\
  --var-struct-base-class=NAME\n\
  --var-struct-class=NAME\n\
  -C, --c-plus-plus\n\
  --header-file FILE\n\
  -h, --help\n\
  -i, --include-filename FILENAME\n\
  -l NUM, --lines NUM\n\
  -p FILENAME, --output-prefix FILENAME\n\
  -r NAME, --rename NAME\n\
  -f VAR, --fast-var VAR\n\
  -V, --version"));
}

#define OPT_DLL 1
#define OPT_CLASS 2
#define OPT_BASE_CLASS 3
#define OPT_CLASS_INCLUDE 4
#define OPT_VAR_STRUCT 5
#define OPT_DEF_FILENAME 6
#define OPT_HEADER_FILE 7
#define OPT_AUTO_EXIT_LABEL 8
#define OPT_VAR_STRUCT_CLASS 9
#define OPT_VAR_STRUCT_BASE_CLASS 10
#define OPT_ENTRY_NAME 11
#define OPT_USING_NAMESPACE 12
#define OPT_VAR_NAME_PREFIX 13
#define OPT_CHARS_ARE_UNSIGNED 14
#define OPT_DECLARE_C_TYPE 15
#define OPT_NAMESPACE 16
#define OPT_EMIT_OPTIMIZE_PRAGMAS 17
#define OPT_CONSTANT 18

namespace {
  const struct option longopts[] =
  {
    "auto-exit-label", required_argument, nullptr, OPT_AUTO_EXIT_LABEL,
    "base-class", required_argument, nullptr, OPT_BASE_CLASS,
    "chars-are-unsigned", no_argument, nullptr, OPT_CHARS_ARE_UNSIGNED,
    "class", required_argument, nullptr, OPT_CLASS,
    "class-include", required_argument, nullptr, OPT_CLASS_INCLUDE,
    "c-plus-plus", no_argument, nullptr, 'C',
    "declare-c-type", required_argument, nullptr, OPT_DECLARE_C_TYPE,
    "constant", required_argument, nullptr, OPT_CONSTANT,
    "def-filename", required_argument, nullptr, OPT_DEF_FILENAME,
    "dll", no_argument, nullptr, OPT_DLL,
    "emit-optimize-pragmas", no_argument, nullptr, OPT_EMIT_OPTIMIZE_PRAGMAS,
    "entry-name", required_argument, nullptr, OPT_ENTRY_NAME,
    "fast-var", required_argument, nullptr, 'f',
    "header-file", required_argument, nullptr, OPT_HEADER_FILE,
    "help", no_argument, nullptr, 'h',
    "include-filename", required_argument, nullptr, 'i',
    "lines", required_argument, nullptr, 'l',
    "namespace", required_argument, nullptr, OPT_NAMESPACE,
    "one", optional_argument, nullptr, '1',
    "output-prefix", required_argument, nullptr, 'p',
    "rename", required_argument, nullptr, 'r',
    "using-namespace", required_argument, nullptr, OPT_USING_NAMESPACE,
    "var-name-prefix", required_argument, nullptr, OPT_VAR_NAME_PREFIX,
    "var-struct", required_argument, nullptr, OPT_VAR_STRUCT,
    "var-struct-class", required_argument, nullptr, OPT_VAR_STRUCT_CLASS,
    "var-struct-base-class", required_argument, nullptr, OPT_VAR_STRUCT_BASE_CLASS,
    "verbose", no_argument, nullptr, 'v',
    "version", no_argument, nullptr, 'V',
    nullptr, 0, nullptr, 0,
  };
}

void option_handler(int argc, char ** argv)
{
  myname = argv[0];
  max_lines_per_c_file = 1000;
  auto_exit_label = -1;

  int opt;
  string new_name;

  optind = 0;
  while ((opt = getopt_long(argc, argv, "C1:hi:l:r:Vf:", longopts, nullptr)) != EOF)
  {
    switch (opt)
    {
    case 'h':
      usage();
      exit(0);
      break;
    case 'v':
      verbose_flag = true;
      break;
    case OPT_CLASS:
      class_name = optarg;
      class_name_scope = optarg;
      class_name_scope += "::";
      break;
    case OPT_DEF_FILENAME:
      def_filename = optarg;
      break;
    case OPT_BASE_CLASS:
      base_class_name = optarg;
      break;
    case OPT_CHARS_ARE_UNSIGNED:
      chars_are_unsigned = true;
      break;
    case OPT_CLASS_INCLUDE:
      class_include = optarg;
      break;
    case OPT_DECLARE_C_TYPE:
      new_type(optarg, UNKNOWN_TYPE, nullptr, nullptr);
      break;
    case OPT_CONSTANT:
      {
        auto nv = ParseNameValue(optarg);
        if (nv.second.empty())
        {
          fprintf(stderr, T_("%s: missing constant value\n"), myname.c_str());
          exit(1);
        }
        if (nv.second[0] == '"')
        {
          if (nv.second.length() < 2 || nv.second.back() != '"')
          {
            fprintf(stderr, T_("%s: bad string value\n"), myname.c_str());
            exit(1);
          }
          new_string_constant(strdup(nv.first.c_str()), strdup(nv.second.substr(1, nv.second.length() - 2).c_str()));
        }
        else
        {
          new_constant(strdup(nv.first.c_str()), "integer", std::stoi(nv.second));
        }
      }
    case OPT_DLL:
      dll_flag = true;
      break;
    case OPT_EMIT_OPTIMIZE_PRAGMAS:
      emit_optimize_pragmas = true;
      break;
    case OPT_ENTRY_NAME:
      entry_name = optarg;
      break;
    case OPT_VAR_STRUCT:
      var_struct_name = optarg;
      break;
    case OPT_VAR_STRUCT_CLASS:
      var_struct_class_name = optarg;
      break;
    case OPT_VAR_STRUCT_BASE_CLASS:
      var_struct_base_class_name = optarg;
      break;
    case OPT_HEADER_FILE:
      h_file_name = optarg;
      break;
    case OPT_AUTO_EXIT_LABEL:
      auto_exit_label = atoi(optarg);
      break;
    case OPT_USING_NAMESPACE:
      using_namespace.push_back(optarg);
      break;
    case OPT_VAR_NAME_PREFIX:
      var_name_prefix = optarg;
      break;
    case OPT_NAMESPACE:
      name_space = optarg;
      break;
    case 'C':
      c_ext = ".cc";
      break;
    case 'i':
      include_filename = optarg;
      break;
    case 'l':
      max_lines_per_c_file = std::stoi(optarg);
      break;
    case 'r':
      new_name = "c4p_";
      new_name += optarg;
      new_mapping(optarg, new_name.c_str());
      break;
    case 'f':
      new_fast_var(optarg);
      break;
    case 'p':
      c_file_name = optarg;
      break;
    case 'V':
      banner();
      exit(0);
      break;
    case '1':
      one_c_file = true;
      if (optarg)
      {
        c_file_name = optarg;
      }
      break;
    default:
      usage();
      exit(1);
    }
  }

  if (var_struct_class_name.empty())
  {
    var_struct_class_name = "c4pdata";
  }

  if (entry_name.empty())
  {
    entry_name = "c4pmain";
  }

  if (optind == argc - 1)
  {
    pascal_file_name = argv[optind];
  }

  if (legacy_flag)
  {
    integer_literal_suffix = "l";
    relational_cast_expressions = true;
    other_cast_expressions = true;
  }
}

int main(int argc, char ** argv)
{
  symtab_init();

  option_handler(argc, argv);

  if (pascal_file_name.empty())
  {
    fprintf(stderr, T_("%s: no input file specified\n"), myname.c_str());
    exit(4);
  }

  if (freopen(pascal_file_name.c_str(), "r", stdin) == nullptr)
  {
    fprintf(stderr, T_("%s: can't open %s\n"), myname.c_str(), pascal_file_name.c_str());
    exit(5);
  }

  yyparse();

  return 0;
}
