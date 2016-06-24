/* miktex/Core/CommandLineBuilder.h:                    -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(CE888532B67E451F851FB7072C37704A)
#define CE888532B67E451F851FB7072C37704A

#include <miktex/Core/config.h>

#include <cstddef>

#include <string>
#include <vector>

#include "Debug.h"
#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

class Argv
{
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL Argv();

public:
  virtual MIKTEXCOREEXPORT MIKTEXTHISCALL ~Argv();

public:
  MIKTEXCORETHISAPI(void) Build(const std::string & fileName, const std::string & arguments);

public:
  MIKTEXCORETHISAPI(void) Append(const std::string & arguments);

public:
  const char * const * GetArgv() const
  {
    return &argv[0];
  }

public:
  int GetArgc() const
  {
    MIKTEX_ASSERT(argv.size() > 0);
    return static_cast<int>(argv.size() - 1);
  }

public:
  const char * operator[] (std::size_t idx) const
  {
    MIKTEX_ASSERT(idx < argv.size());
    return argv[idx];
  }

private:
  std::vector<char *> argv;
};

enum class OptionConvention
{
  None,
  GNU,
  Xt,
  DOS
};

enum class QuotingConvention
{
  None,
  Whitespace,
  Bat
};

class CommandLineBuilder
{
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL CommandLineBuilder();

public:
  virtual MIKTEXCOREEXPORT MIKTEXTHISCALL ~CommandLineBuilder();

public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL CommandLineBuilder(const CommandLineBuilder & other);

public:
  MIKTEXCORETHISAPI(CommandLineBuilder &) operator= (const CommandLineBuilder & other);

public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL CommandLineBuilder(const std::string & argument);

public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL CommandLineBuilder(const std::string & argument1, const std::string & argument2);

public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL CommandLineBuilder(const std::string & argument1, const std::string & argument2, const std::string & argument3);

public:
  MIKTEXCORETHISAPI(void) SetOptionConvention(OptionConvention optionConvention);

public:
  MIKTEXCORETHISAPI(void) SetQuotingConvention(QuotingConvention quotingConvention);

public:
  MIKTEXCORETHISAPI(void) Clear();

public:
  MIKTEXCORETHISAPI(void) AppendArgument(const std::string & argument);

public:
  void AppendArgument(const PathName & argument)
  {
    AppendArgument(argument.ToString());
  }

public:
  void AppendArgument(const char * argument)
  {
    MIKTEX_ASSERT_STRING(argument);
    AppendArgument(std::string(argument));
  }

public:
  MIKTEXCORETHISAPI(void) AppendUnquoted(const std::string & text);

public:
  MIKTEXCORETHISAPI(void) AppendArguments(int argc, const char * const * argv);

public:
  MIKTEXCORETHISAPI(void) AppendArguments(const std::vector<std::string> & argv);

public:
  MIKTEXCORETHISAPI(void) AppendArguments(const Argv & argv);

public:
  MIKTEXCORETHISAPI(void) AppendOption(const std::string & name, const std::string & value);

public:
  void AppendOption(const std::string & name)
  {
    AppendOption(name, std::string());
  }

public:
  void AppendOption(const std::string & name, const PathName & value)
  {
    AppendOption(name, value.ToString());
  }

public:
  void AppendOption(const std::string & name, const char * value)
  {
    MIKTEX_ASSERT_STRING(value);
    AppendOption(name, std::string(value));
  }

public:
  MIKTEXCORETHISAPI(void) AppendRedirection(const PathName & path, std::string direction);

public:
  void AppendStdoutRedirection(const PathName & path, bool append)
  {
    return AppendRedirection(path, append ? ">>" : ">");
  }

public:
  void AppendStdoutRedirection(const PathName & path)
  {
    return AppendStdoutRedirection(path, false);
  }

public:
  void AppendStdinRedirection(const PathName & path)
  {
    return AppendRedirection(path, "<");
  }

public:
  MIKTEXCORETHISAPI(std::string) ToString() const;

private:
  void * p = nullptr;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
