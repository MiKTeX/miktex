/* CommandLine.cpp: command-line builder

   Copyright (C) 1996-2017 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/CommandLineBuilder.h"

#include "ArgvImpl.h"

using namespace MiKTeX::Core;
using namespace std;

Argv::Argv() :
  pimpl(new impl{ {nullptr} })
{
}

Argv::Argv(Argv&& rhs) :
  pimpl(std::move(rhs.pimpl))
{
}

Argv::~Argv() noexcept
{
  try
  {
    if (pimpl != nullptr)
    {
      for (vector<char*>::iterator it = pimpl->argv.begin(); it != pimpl->argv.end() && *it != nullptr; ++it)
      {
        MIKTEX_FREE(*it);
      }
    }
  }
  catch (const exception&)
  {
  }
}

Argv::Argv(const string& fileName, const string& arguments) :
  pimpl(new impl{ { MIKTEX_STRDUP(fileName.c_str()), nullptr } })
{
  Append(arguments);
}

Argv::Argv(const vector<string>& arguments) :
  pimpl(new impl{ { nullptr } })
{
  MIKTEX_ASSERT(!pimpl->argv.empty());
  pimpl->argv.pop_back();
  MIKTEX_ASSERT(pimpl->argv.empty());
  for(const string& arg : arguments)
  {
    pimpl->argv.push_back(MIKTEX_STRDUP(arg.c_str()));
  }
  pimpl->argv.push_back(nullptr);
}

const char* const* Argv::GetArgv() const
{
  return &pimpl->argv[0];
}

int Argv::GetArgc() const
{
  MIKTEX_ASSERT(pimpl->argv.size() > 0);
  return static_cast<int>(pimpl->argv.size() - 1);
}

const char* Argv::operator[] (size_t idx) const
{
  MIKTEX_ASSERT(idx < pimpl->argv.size());
  return pimpl->argv[idx];
}

vector<string> Argv::ToStringVector() const
{
  vector<string> result;
  for (size_t idx = 0; idx < GetArgc(); ++idx)
  {
    result.push_back(this->operator[](idx));
  }
  return result;
}

class CommandLineBuilder::impl
{
public:
  string str;
public:
  string optionIndicator;
public:
  string valueIndicator;
public:
  string needsQuoting;
};

CommandLineBuilder::CommandLineBuilder() :
  pimpl(new impl{})
{
  SetOptionConvention(OptionConvention::None);
  SetQuotingConvention(QuotingConvention::Whitespace);
}

CommandLineBuilder::CommandLineBuilder(const CommandLineBuilder& other) :
  CommandLineBuilder()
{
  *pimpl = *other.pimpl;
}

CommandLineBuilder& CommandLineBuilder::operator= (const CommandLineBuilder& other)
{
  *pimpl = *other.pimpl;
  return *this;
}

CommandLineBuilder::CommandLineBuilder(const string& argument) :
  CommandLineBuilder()
{
  AppendArgument(argument);
}

CommandLineBuilder::~CommandLineBuilder() noexcept
{
}

CommandLineBuilder::CommandLineBuilder(const string& argument1, const string& argument2) :
  CommandLineBuilder()
{
  AppendArgument(argument1);
  AppendArgument(argument2);
}

CommandLineBuilder::CommandLineBuilder(const string& argument1, const string& argument2, const string& argument3) :
  CommandLineBuilder()
{
  AppendArgument(argument1);
  AppendArgument(argument2);
  AppendArgument(argument3);
}

CommandLineBuilder::CommandLineBuilder(const vector<string>& arguments) :
  CommandLineBuilder()
{
  AppendArguments(arguments);
}

void CommandLineBuilder::SetOptionConvention(OptionConvention optionConvention)
{
  switch (optionConvention)
  {
  case OptionConvention::None:
    pimpl->optionIndicator = "";
    pimpl->valueIndicator = "";
    break;
  case OptionConvention::Xt:
    pimpl->optionIndicator = "-";
    pimpl->valueIndicator = "=";
    break;
  case OptionConvention::GNU:
    pimpl->optionIndicator = "--";
    pimpl->valueIndicator = "=";
    break;
  case OptionConvention::DOS:
    pimpl->optionIndicator = "/";
    pimpl->valueIndicator = ":";
    break;
  }
}

void CommandLineBuilder::SetQuotingConvention(QuotingConvention quotingConvention)
{
  switch (quotingConvention)
  {
  case QuotingConvention::None:
    pimpl->needsQuoting = "";
    break;
  case QuotingConvention::Whitespace:
    pimpl->needsQuoting = " \t";
    break;
  case QuotingConvention::Bat:
    pimpl->needsQuoting = " \t,;=";
    break;
  }
}

void CommandLineBuilder::Clear()
{
  pimpl->str = "";
}

void CommandLineBuilder::AppendUnquoted(const string& text)
{
  if (!pimpl->str.empty())
  {
    pimpl->str += ' ';
  }
  pimpl->str += text;
}

void CommandLineBuilder::AppendArgument(const string& argument)
{
  if (!pimpl->str.empty())
  {
    pimpl->str += ' ';
  }
  bool needsQuoting = argument.empty() || argument.find_first_of(pimpl->needsQuoting) != string::npos;
  if (needsQuoting)
  {
    pimpl->str += '"';
  }
  pimpl->str += argument;
  if (needsQuoting)
  {
    pimpl->str += '"';
  }
}

void CommandLineBuilder::AppendArguments(int argc, const char* const* argv)
{
  for (int idx = 0; idx < argc; ++idx)
  {
    AppendArgument(string(argv[idx]));
  }
}

void CommandLineBuilder::AppendArguments(const vector<string>& argv)
{
  for (const string& arg : argv)
  {
    AppendArgument(arg);
  }
}

void CommandLineBuilder::AppendArguments(const Argv& argv)
{
  if (argv.GetArgc() > 0)
  {
    AppendArguments(argv.GetArgc() - 1, argv.GetArgv() + 1);
  }
}

void CommandLineBuilder::AppendOption(const string& name, const string& value)
{
  if (!pimpl->str.empty())
  {
    pimpl->str += ' ';
  }
  pimpl->str += pimpl->optionIndicator;
  pimpl->str += name;
  if (!value.empty())
  {
    pimpl->str += pimpl->valueIndicator;
    bool needsQuoting = value.find_first_of(pimpl->needsQuoting) != string::npos;
    if (needsQuoting)
    {
      pimpl->str += '"';
    }
    pimpl->str += value;
    if (needsQuoting)
    {
      pimpl->str += '"';
    }
  }
}

void CommandLineBuilder::AppendRedirection(const PathName& path_, string direction)
{
#if defined(MIKTEX_WINDOWS)
  string path = PathName(path_).ToDos().ToString();
#else
  string path = path_.ToString();
#endif
  pimpl->str += direction;
  bool needsQuoting = path.find_first_of(pimpl->needsQuoting) != string::npos;
  if (needsQuoting)
  {
    pimpl->str += '"';
  }
  pimpl->str += path;
  if (needsQuoting)
  {
    pimpl->str += '"';
  }
}

string CommandLineBuilder::ToString() const
{
  return pimpl->str;
}
