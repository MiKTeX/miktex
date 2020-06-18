/* misc.c: C4P utilities                                -*- C++ -*-

   Copyright (C) 1991-2018 Christian Schenk

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
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <climits>
#include <memory>

#include "c4p-version.h"
#include "common.h"
#include "output.h"

using namespace std;

namespace {
  unsigned c_file_count;
  unsigned c_file_number;
  FILE * name_file;
  std::string current_fast_vars;
}

const size_t MY_PATH_MAX = 8192;

void generate_file_header()
{
  cppout.out_s("/* generated from " + pascal_file_name + " by C4P version " + MIKTEX_COMPONENT_VERSION_STR + " */\n");
}

void open_name_file()
{
  char name_file_name[MY_PATH_MAX];
  sprintf(name_file_name, "%s.nam", prog_symbol->s_repr);
#if 0
  if ((name_file = fopen(name_file_name, "w")) == 0)
  {
    c4p_error("can't open manifest file %s", name_file_name);
  }
#endif
}

void close_name_file()
{
#if 0
  fclose(name_file);
#endif
}

void open_header_file()
{
  char szHeaderFileName[MY_PATH_MAX];
  if (h_file_name.length() > 0)
  {
    strcpy(szHeaderFileName, h_file_name.c_str());
  }
  else if (one_c_file && c_file_name.length() > 0)
  {
    sprintf(szHeaderFileName, "%s.h", c_file_name.c_str());
  }
  else
  {
    sprintf(szHeaderFileName, "%s.h", prog_symbol->s_repr);
  }
  cppout.open_file(H_FILE_NUM, szHeaderFileName);
  cppout.redir_file(H_FILE_NUM);
  generate_file_header();
  cppout.out_s("#ifndef C4P_HEADER_GUARD_" + std::string(prog_symbol->s_repr) + "\n");
  cppout.out_s("#define C4P_HEADER_GUARD_" + std::string(prog_symbol->s_repr) + "\n");
  cppout.out_s("#ifdef HAVE_C4P_PRE_H\n");
  cppout.out_s("#include \"c4p_pre.h\"\n");
  cppout.out_s("#endif\n");
  cppout.out_s("#include <miktex/C4P/C4P>\n");
  cppout.out_s("#ifdef HAVE_C4P_POST_H\n");
  cppout.out_s("#include \"c4p_post.h\"\n");
  cppout.out_s("#endif\n");
  cppout.out_s("#ifdef __cplusplus\nusing namespace C4P;\n#endif\n");
  cppout.out_s("#ifndef C4PEXTERN\n#define C4PEXTERN\n#endif\n");
  if (!def_filename.empty())
  {
    cppout.out_s("#include <" + def_filename + ">\n");
  }
  if (class_name.empty())
  {
    cppout.out_s("#ifdef __cplusplus\n");
    cppout.out_s("extern \"C++\" {\n");
    cppout.out_s("#endif\n");
  }
  else
  {
    if (!class_include.empty())
    {
      cppout.out_s("#include <" + class_include + ">\n");
    }
    if (base_class_name.empty())
    {
      cppout.out_s("class " + class_name + " : public C4P::ProgramBase {\n");
    }
    else
    {
      cppout.out_s("class " + class_name + " : public " + base_class_name + " {\n");
    }
    cppout.out_s("public:\n");
  }
}

void close_header_file()
{
  cppout.redir_file(H_FILE_NUM);
  if (class_name.length() == 0)
  {
    cppout.out_s("#ifdef __cplusplus\n");
    cppout.out_s("}\n");
    cppout.out_s("#endif\n");
  }
  else
  {
    cppout.out_s("};\n");
  }
  cppout.out_s("\n#endif\n");
  cppout.close_file(H_FILE_NUM);
}

void open_def_file()
{
  cppout.open_file(DEF_FILE_NUM, def_filename.c_str());
  cppout.redir_file(DEF_FILE_NUM);
  generate_file_header();
  cppout.out_s("#ifndef C4P_DEF_GUARD_" + std::string(prog_symbol->s_repr) + "\n");
  cppout.out_s("#define C4P_DEF_GUARD_" + std::string(prog_symbol->s_repr) + "\n");
}

void close_def_file()
{
  cppout.redir_file(DEF_FILE_NUM);
  cppout.out_s("\n#endif\n");
  cppout.close_file(DEF_FILE_NUM);
}

void begin_new_c_file(const char *file_name, int is_main)

{
  char cfile_name[MY_PATH_MAX];

  sprintf(cfile_name, "%s%s", file_name, c_ext.length() > 0 ? c_ext.c_str() : ".c");

#if 0
  printf("creating %s\n", cfile_name);
#endif

  if (c_file_count++ == 0)
  {
    cppout.open_file(C_FILE_NUM, cfile_name);
  }
  else
  {
    cppout.reopen_file(C_FILE_NUM, cfile_name);
  }

  cppout.redir_file(C_FILE_NUM);
  generate_file_header();

  if (using_namespace.size() > 0)
  {
    cppout.out_s("#ifdef __cplusplus\n");
    for (const string & ns :  using_namespace)
    {
      int count = 0;
      string s = ns;
      do
      {
        string::size_type end = s.find("::");
        cppout.out_s("namespace " + s.substr(0, end) + " { ");
        ++count;
        if (end == string::npos)
        {
          s = "";
        }
        else
        {
          s = s.substr(end + 2);
        }
      } while (!s.empty());
      for (; count != 0; --count)
      {
        cppout.out_s(" }");
      }
      cppout.out_s("using namespace " + ns + ";\n");
    }
    cppout.out_s("#endif\n");
  }

  if (class_name.empty())
  {
    cppout.out_s("#define C4PEXTERN" + std::string(is_main ? "" : " extern") + "\n\n");
  }
  if (!h_file_name.empty())
  {
    cppout.out_s("#include \"" + h_file_name + "\"\n\n");
  }
  else if (!c_file_name.empty())
  {
    cppout.out_s("#include \"" + c_file_name + ".h\"\n\n");
  }
  else
  {
    cppout.out_s("#include \"" + std::string(prog_symbol->s_repr) + ".h\"\n\n");
  }
  if (!include_filename.empty())
  {
    cppout.out_s("#include \"" + include_filename + "\"\n\n");
  }

  cppout.out_s("#ifdef _MSC_VER\n");
  cppout.out_s("#pragma warning ( disable : 4102 4390)\n");
  cppout.out_s("#endif\n\n");
}

void check_c_file_size()
{
  char name[MY_PATH_MAX];
  if (c_file_count == 0 || (!one_c_file && c_file_line_count + 20 > max_lines_per_c_file))
  {
    ++c_file_number;
    if (one_c_file)
    {
      if (!c_file_name.empty())
      {
        begin_new_c_file(c_file_name.c_str(), 1);
        return;
      }
      else
      {
        sprintf(name, "%s", prog_symbol->s_repr);
      }
    }
    else
    {
      sprintf(name, "%s%03u",
        (c_file_name.length() > 0 ? c_file_name.c_str() : prog_symbol->s_repr),
        c_file_number);
    }
    begin_new_c_file(name, one_c_file);
  }
}

void forget_fast_vars()
{
  current_fast_vars = "";
}

void remember_fast_var(const char * s)
{
  if (strstr(current_fast_vars.c_str(), s) == nullptr)
  {
    current_fast_vars += ' ';
    current_fast_vars += s;
  }
}

void declare_fast_var_macro(unsigned routine_handle)
{
  unique_ptr<char[]> buf(new char[current_fast_vars.length() + 1]);
  strcpy(buf.get(), current_fast_vars.c_str());
  char * v = strtok(buf.get(), " ");
  cppout.redir_file(H_FILE_NUM);
  cppout.out_s("\n#define C4P_FAST_VARS_" + std::to_string(routine_handle) + " ");
  macroizing = true;
  while (v != nullptr)
  {
    symbol_t * sym = lookup(v);
    if (sym != nullptr)
    {
      cppout.out_s("register " +
                   std::string(sym->s_translated_type ? sym->s_translated_type : "FIXME") +
                   std::string(" ") +
                   std::string(sym->s_type == ARRAY_NODE ? "*" : "") +
                   std::string("_c4p_fast_") + std::string(sym->s_repr) +
                   std::string("_") + std::to_string(routine_handle) +
                   std::string(" = ") + std::string(sym->s_repr) +
                   std::string(";\n"));
    }
    v = strtok(nullptr, " ");
  }
  macroizing = false;
  cppout.out_s("\n");
  cppout.redir_file(C_FILE_NUM);
}

void begin_routine(prototype_node * proto, unsigned handle)
{
  cppout.redir_file(DEF_FILE_NUM);
  cppout.out_s("#define C4P_HANDLE_" + std::string(proto->name->s_repr) + " " + std::to_string(handle) + "\n");
  cppout.redir_file(C_FILE_NUM);
  check_c_file_size();
  cppout.out_s("\n");
  ++block_level;
  mark_symbol_table();
  mark_string_table();
  mark_type_table();
  if (emit_optimize_pragmas)
  {
    cppout.out_s("#if defined (C4P_OPT_" + std::string(proto->name->s_repr) + ") && ! defined (C4P_OPT_" + std::to_string(handle) + ")\n");
    cppout.out_s("#define C4P_OPT_" + std::to_string(handle) + " C4P_OPT_" + std::string(proto->name->s_repr) + "\n");
    cppout.out_s("#endif\n");
    cppout.out_s("#ifdef C4P_OPT_" + std::to_string(handle) + "\n");
    cppout.out_s("#pragma optimize (C4P_OPT_" + std::to_string(handle) + ", on)\n");
    cppout.out_s("#endif\n");
    cppout.out_s("#ifdef C4P_NOOPT_" + std::string(proto->name->s_repr) + "\n");
    cppout.out_s("#pragma optimize (\"\", off)\n");
    cppout.out_s("#endif\n");
  }
  generate_routine_head(proto);
  cppout.out_s("\n");
  cppout.out_s("{\n");
  ++curly_brace_level;
  if (n_fast_vars)
  {
    cppout.out_s("C4P_FAST_VARS_" + std::to_string(handle) + "\n");
    forget_fast_vars();
  }
#if 0
  fprintf(name_file, "%u %s\n", handle, proto->name->s_repr);
#endif
  auto_exit_label_flag = (auto_exit_label >= 0);
}

void end_routine(unsigned handle)
{
  unmark_type_table();
  unmark_string_table();
  unmark_symbol_table();
  --block_level;
  --curly_brace_level;
  if (auto_exit_label_flag)
  {
    cppout.out_s("\nc4p_L" + std::to_string(auto_exit_label)  + ": ;\n");
    auto_exit_label_flag = false;
  }
  cppout.out_s("}\n");
  if (n_fast_vars)
  {
    declare_fast_var_macro(handle);
  }
  if (emit_optimize_pragmas)
  {
    cppout.out_s("#pragma optimize (\"\", on)\n");
  }
}

char * strcpye(char * s1, const char * s2)
{
  while ((*s1++ = *s2++) != 0)
  {
    ;
  }
  return --s1;
}
