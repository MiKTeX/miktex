/* internal.h:                                          -*- C++ -*-

   Copyright (C) 1998-2016 Christian Schenk

   This file is part of the MiKTeX Maker Library.

   The MiKTeX Maker Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Maker Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Maker Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if defined(_MSC_VER)
#  pragma warning (disable: 4786)
#endif

#include "makex-version.h"

#include <array>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <getopt.h>

#include <miktex/App/Application>
#include <miktex/Core/AutoResource>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Debug>
#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Core/Fndb>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/Registry>
#include <miktex/Core/Session>
#include <miktex/Core/TemporaryDirectory>
#include <miktex/Util/StringUtil>
#include <miktex/Util/Tokenizer>

#include <log4cxx/logger.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/xml/domconfigurator.h>

#define MKTEXAPI(func) extern "C" MIKTEXDLLEXPORT int MIKTEXCEECALL func

#define OUT__ (stdoutStderr ? cerr : cout)

#define VA_START(arglist, lpszFormat   )        \
va_start(arglist, lpszFormat);                  \
try                                             \
{

#define VA_END(arglist)                         \
}                                               \
catch(...)                                      \
{                                               \
  va_end(arglist);                              \
  throw;                                        \
}                                               \
va_end(arglist);

#include "makex.h"

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

#define T_(x) MIKTEXTEXT(x)
#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

#if !defined(UNUSED_ALWAYS)
#  define UNUSED_ALWAYS(x) static_cast<void>(x)
#endif

inline bool HasPrefix(const char * lpsz1, const char * lpsz2)
{
  return PathName::Compare(lpsz1, lpsz2, strlen(lpsz2)) == 0;
}

class MakeUtility :
  public Application
{
public:
  void Init(const Session::InitInfo & initInfo) override;

public:
  virtual void Run(int argc, const char ** argv) = 0;

protected:
  virtual void Usage() = 0;

protected:
  virtual void CreateDestinationDirectory() = 0;

protected:
  virtual void HandleOption(int ch, const char * lpszOptArg, bool & handled) = 0;

protected:
  void GetOptions(int argc, const char ** argv, const struct option * pLongOptions, int & index);

protected:
  bool RunProcess(const char * lpszExeName, const std::string & arguments, const PathName & workingDirectory);

protected:
  bool RunMETAFONT(const char * lpszName, const char * lpszMode, const char * lpszMag, const PathName & workingDirectory);

protected:
  void Install(const PathName & source, const PathName & dest);

protected:
  PathName CreateDirectoryFromTemplate(const string & templ);

protected:
  void ShowVersion();

protected:
  void Verbose(const char * lpszFormat, ...);

protected:
  void Message(const char * lpszFormat, ...);

protected:
  void PrintOnly(const char * lpszFormat, ...);

protected:
  bool quiet = false;

protected:
  bool verbose = false;

protected:
  bool debug = false;

protected:
  bool printOnly = false;

protected:
  bool stdoutStderr = true;

protected:
  PathName destinationDirectory;

protected:
  string name;

protected:
  bool adminMode = false;
};

#define COMMON_OPTIONS                                          \
    "admin",        no_argument,            0,      'A',        \
    "debug",        no_argument,            0,      'd',        \
    "disable-installer", no_argument,       0,      'D',        \
    "enable-installer", no_argument,        0,      'E',        \
    "help",         no_argument,            0,      'h',        \
    "print-only",   no_argument,            0,      'n',        \
    "quiet",        no_argument,            0,      'q',        \
    "verbose",      no_argument,            0,      'v',        \
    "version",      no_argument,            0,      'V'

#define BEGIN_OPTION_MAP(cls)                                           \
void HandleOption(int ch, const char * lpszOptArg, bool & bHandled)     \
{                                                                       \
  UNUSED_ALWAYS(lpszOptArg);                                            \
  switch (ch)                                                           \
  {

#define OPTION_ENTRY_TRUE(ch, var)              \
    case ch:                                    \
      var = true;                               \
      break;

#define OPTION_ENTRY_SET(ch, var)               \
    case ch:                                    \
      var = lpszOptArg;                         \
      break;

#define OPTION_ENTRY_STRING(ch, var, size)              \
    case ch:                                            \
      StringUtil::CopyString (var, size, lpszOptArg);   \
      break;

#define OPTION_ENTRY(ch, action)                \
    case ch:                                    \
      action;                                   \
      break;

#define END_OPTION_MAP()                        \
    default:                                    \
      bHandled = false;                         \
      break;                                    \
    }                                           \
}
