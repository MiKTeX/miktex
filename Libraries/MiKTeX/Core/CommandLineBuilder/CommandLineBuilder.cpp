/* CommandLine.cpp: command-line builder

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/CommandLineBuilder.h"

using namespace MiKTeX::Core;
using namespace std;

Argv::Argv()
{
  argv.push_back(nullptr);
}

Argv::~Argv()
{
  try
  {
    for (vector<char*>::iterator it = argv.begin(); it != argv.end() && *it != nullptr; ++it)
    {
      MIKTEX_FREE(*it);
    }
  }
  catch (const exception &)
  {
  }
}

void Argv::Build(const string & fileName, const string & arguments)
{
  MIKTEX_ASSERT(argv.size() == 1);
  argv.clear();
  argv.push_back(MIKTEX_STRDUP(fileName.c_str()));
  argv.push_back(nullptr);
  Append(arguments);
}

struct Data
{
  string str;
  string optionIndicator;
  string valueIndicator;
  string needsQuoting;
};

#define pData reinterpret_cast<Data*>(this->p)

CommandLineBuilder::CommandLineBuilder() :
  p(new Data)
{
  SetOptionConvention(OptionConvention::None);
  SetQuotingConvention(QuotingConvention::Whitespace);
}

CommandLineBuilder::CommandLineBuilder(const CommandLineBuilder & other) :
  CommandLineBuilder()
{
  this->operator= (other);
}

CommandLineBuilder & CommandLineBuilder::operator= (const CommandLineBuilder & other)
{
  *pData = *reinterpret_cast<const Data *>(other.p);
  return *this;
}

CommandLineBuilder::CommandLineBuilder(const string & argument) :
  CommandLineBuilder()
{
  AppendArgument(argument);
}

CommandLineBuilder::CommandLineBuilder(const string & argument1, const string & argument2) :
  CommandLineBuilder()
{
  AppendArgument(argument1);
  AppendArgument(argument2);
}

CommandLineBuilder::CommandLineBuilder(const string & argument1, const string & argument2, const string & argument3) :
  CommandLineBuilder()
{
  AppendArgument(argument1);
  AppendArgument(argument2);
  AppendArgument(argument3);
}

CommandLineBuilder::~CommandLineBuilder()
{
  try
  {
    delete pData;
  }
  catch (const exception &)
  {
  }
  p = nullptr;
}

void CommandLineBuilder::SetOptionConvention(OptionConvention optionConvention)
{
  switch (optionConvention)
  {
  case OptionConvention::None:
    pData->optionIndicator = "";
    pData->valueIndicator = "";
    break;
  case OptionConvention::Xt:
    pData->optionIndicator = "-";
    pData->valueIndicator = "=";
    break;
  case OptionConvention::GNU:
    pData->optionIndicator = "--";
    pData->valueIndicator = "=";
    break;
  case OptionConvention::DOS:
    pData->optionIndicator = "/";
    pData->valueIndicator = ":";
    break;
  }
}

void CommandLineBuilder::SetQuotingConvention(QuotingConvention quotingConvention)
{
  switch (quotingConvention)
  {
  case QuotingConvention::None:
    pData->needsQuoting = "";
    break;
  case QuotingConvention::Whitespace:
    pData->needsQuoting = " \t";
    break;
  case QuotingConvention::Bat:
    pData->needsQuoting = " \t,;=";
    break;
  }
}

void CommandLineBuilder::Clear()
{
  pData->str = "";
}

void CommandLineBuilder::AppendUnquoted(const string & text)
{
  if (!pData->str.empty())
  {
    pData->str += ' ';
  }
  pData->str += text;
}

void CommandLineBuilder::AppendArgument(const string & argument)
{
  if (!pData->str.empty())
  {
    pData->str += ' ';
  }
  bool needsQuoting = argument.empty() || argument.find_first_of(pData->needsQuoting) != string::npos;
  if (needsQuoting)
  {
    pData->str += '"';
  }
  pData->str += argument;
  if (needsQuoting)
  {
    pData->str += '"';
  }
}

void CommandLineBuilder::AppendArguments(int argc, const char * const * argv)
{
  for (int idx = 0; idx < argc; ++idx)
  {
    AppendArgument(string(argv[idx]));
  }
}

void CommandLineBuilder::AppendArguments(const vector<string> & argv)
{
  for (const string & arg : argv)
  {
    AppendArgument(arg);
  }
}

void CommandLineBuilder::AppendArguments(const Argv & argv)
{
  if (argv.GetArgc() > 0)
  {
    AppendArguments(argv.GetArgc() - 1, argv.GetArgv() + 1);
  }
}

void CommandLineBuilder::AppendOption(const string & name, const string & value)
{
  if (!pData->str.empty())
  {
    pData->str += ' ';
  }
  pData->str += pData->optionIndicator;
  pData->str += name;
  if (!value.empty())
  {
    pData->str += pData->valueIndicator;
    bool needsQuoting = value.find_first_of(pData->needsQuoting) != string::npos;
    if (needsQuoting)
    {
      pData->str += '"';
    }
    pData->str += value;
    if (needsQuoting)
    {
      pData->str += '"';
    }
  }
}

void CommandLineBuilder::AppendRedirection(const PathName & path_, string direction)
{
#if defined(MIKTEX_WINDOWS)
  string path = PathName(path_).ToDos().ToString();
#else
  string path = path_.ToString();
#endif
  pData->str += direction;
  bool needsQuoting = path.find_first_of(pData->needsQuoting) != string::npos;
  if (needsQuoting)
  {
    pData->str += '"';
  }
  pData->str += path;
  if (needsQuoting)
  {
    pData->str += '"';
  }
}

string CommandLineBuilder::ToString() const
{
  return pData->str;
}
