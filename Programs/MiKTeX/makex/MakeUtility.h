/* MakeUtility.h:                                       -*- C++ -*-

   Copyright (C) 1998-2018 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.
   
   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

#pragma once

#if defined(_MSC_VER)
#  pragma warning (disable: 4786)
#endif

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

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <log4cxx/logger.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/xml/domconfigurator.h>

#define OUT__ (stdoutStderr ? cerr : cout)

#define VA_START(arglist, format)               \
va_start(arglist, format);                      \
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

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;
using namespace std::string_literals;

#define T_(x) MIKTEXTEXT(x)
#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#if !defined(UNUSED_ALWAYS)
#  define UNUSED_ALWAYS(x) static_cast<void>(x)
#endif

inline bool HasPrefix(const string& s1, const string& s2)
{
  return PathName::Compare(s1, s2, s2.length()) == 0;
}

class ProcessOutputTrash :
  public IRunProcessCallback
{
public:
  bool MIKTEXTHISCALL OnProcessOutput(const void* output, size_t n) override
  {
    UNUSED_ALWAYS(output);
    UNUSED_ALWAYS(n);
    return true;
  }
};

class ProcessOutputStderr :
  public IRunProcessCallback
{
public:
  bool MIKTEXTHISCALL OnProcessOutput(const void* output, size_t n) override
  {
    cerr.write((const char*)output, n);
    return true;
  }
};

extern log4cxx::LoggerPtr logger;

class MakeUtility :
  public Application
{
public:
  void Init(const Session::InitInfo& initInfo, std::vector<char*>& args) override
  {
    Application::Init(initInfo, args);
    session = GetSession();
  }

public:
  virtual void Run(int argc, const char** argv) = 0;

protected:
  virtual void Usage() = 0;

protected:
  virtual void CreateDestinationDirectory() = 0;

protected:
  virtual void HandleOption(int ch, const char* optArg, bool& handled) = 0;

private:
  void GetShortOptions(const struct option* longOptions, string& shortOptions)
  {
    shortOptions = "";
    for (const struct option* opt = longOptions; opt->name != nullptr; ++opt)
    {
      if (isprint(opt->val))
      {
        shortOptions += static_cast<char>(opt->val);
        if (opt->has_arg == required_argument)
        {
          shortOptions += ':';
        }
        else if (opt->has_arg == optional_argument)
        {
          shortOptions += "::";
        }
      }
    }
  }
  
protected:
  void GetOptions(int argc, const char** argv, const struct option* longOptions, int& optionIndex)
  {
    string shortOptions;
    GetShortOptions(longOptions, shortOptions);

    int c;
    int idx;
    optind = 0;
    while ((c = getopt_long(argc, const_cast<char*const*>(argv), shortOptions.c_str(), longOptions, &idx)) != EOF)
    {
      switch (c)
      {
      case 'A':
        session->SetAdminMode(true);
        break;
      case 'D':
        this->EnableInstaller(TriState::False);
        break;
      case 'E':
        this->EnableInstaller(TriState::True);
        break;
      case 'h':
        Usage();
        throw 0;
        break;
      case 'n':
        printOnly = true;
        break;
      case 'd':
        debug = true;
        break;
      case 'v':
        verbose = true;
        break;
      case 'q':
        quiet = true;
        break;
      case 'V':
        ShowVersion();
        throw 0;
        break;
      default:
        {
          bool handled = true;
          HandleOption(c, optarg, handled);
          if (!handled)
            {
              FatalError(T_("Unknown command-line option."));
            }
          break;
        }
      }
    }
    
    optionIndex = optind;
  }
  
protected:
  bool RunProcess(const char* exeName, const std::vector<std::string>& arguments, const PathName& workingDirectory)
  {
    // find the executable; make sure it contains no blanks
    PathName exe;
    if (!session->FindFile(exeName, FileType::EXE, exe))
    {
      FatalError(fmt::format(T_("The application file {0} could not be found."), Q_(exeName)));
    }

    std::vector<std::string> allArgs{ exeName };
    switch (GetEnableInstaller())
    {
    case TriState::False:
      allArgs.push_back("--miktex-disable-installer");
      break;
    case TriState::True:
      allArgs.push_back("--miktex-enable-installer");
      break;
    default:
      break;
    }
    allArgs.insert(allArgs.end(), arguments.begin(), arguments.end());

    Message(T_("Running %s..."), Q_(exeName));
    LOG4CXX_INFO(logger, "running: " << CommandLineBuilder(allArgs).ToString());
    PrintOnly("%s", CommandLineBuilder(allArgs).ToString().c_str());
    
    // run the program
    int exitCode = 0;
    if (!(printOnly || find(allArgs.begin(), allArgs.end(), "--print-only") != allArgs.end()))
    {
      ProcessOutputTrash trash;
      ProcessOutputStderr toStderr;
      IRunProcessCallback* callback = nullptr;
      if (quiet)
      {
        callback = &trash;
      }
      else if (stdoutStderr)
        {
          callback = &toStderr;
        }
      if (!Process::Run(exe, allArgs, callback, &exitCode, workingDirectory.GetData()))
      {
        FatalError(fmt::format(T_("The application file {0} could not be started."), Q_(exeName)));
      }
    }

    return exitCode == 0;
  }

protected:
  bool RunMETAFONT(const char* name, const char* mode, const char* mag, const PathName& workingDirectory)
  {
    vector<string> arguments;
    arguments.push_back("--undump="s + "mf");
    arguments.push_back("\\mode:="s + (mode == nullptr ? "ljfour" : mode) + ";");
    if (mag != nullptr)
    {
      arguments.push_back("\\mag:="s + mag + ";");
    }
    if (!debug)
    {
      arguments.push_back("nonstopmode;");
    }
    arguments.push_back("input "s + name);
    if (RunProcess(MIKTEX_MF_EXE, arguments, workingDirectory))
    {
      return true;
    }
    Verbose(T_("METAFONT failed for some reason"));
    PathName pathLogFile = name;
    pathLogFile.AppendExtension(".log");
    AutoFILE pLogFile(File::Open(pathLogFile, FileMode::Open, FileAccess::Read));
    string line;
    bool noError = true;
    size_t nStrangePaths = 0;
    while (noError && Utils::ReadUntilDelim(line, '\n', pLogFile.Get()))
    {
      if (line[0] != '!')
      {
        continue;
      }
      if (strncmp(line.c_str(), "! Strange path", 14) == 0)
      {
        ++nStrangePaths;
        continue;
      }
      noError = false;
    }
    if (noError)
    {
      Verbose(T_("ignoring %u strange path(s)"), nStrangePaths);
    }
    pLogFile.Reset();
    return noError;
  }
  
protected:
  void Install(const PathName& source, const PathName& dest)
  {
    PrintOnly("cp %s %s", Q_(source), Q_(dest));
    PrintOnly("initexmf --update-fndb");
    if (!printOnly)
    {
      File::Copy(source, dest, { FileCopyOption::ReplaceExisting, FileCopyOption::UpdateFndb });
    }
  }

protected:
  PathName CreateDirectoryFromTemplate(const string& templ)
  {
    PathName path;
    const char* lpszTemplate = templ.c_str();
    if (lpszTemplate[0] == '%'
        && lpszTemplate[1] == 'R'
        && IsDirectoryDelimiter(lpszTemplate[2]))
    {
      path = session->GetSpecialPath(SpecialPath::DataRoot);
      path /= lpszTemplate + 3;
    }
    else
    {
      path = lpszTemplate;
    }

    if (!Directory::Exists(path))
    {
      PrintOnly(CommandLineBuilder("mkdir", path.ToString()).ToString().c_str());
      if (!printOnly)
      {
        Directory::Create(path);
      }
    }
    return path;
  }

protected:
  void ShowVersion()
  {
    OUT__
      << MiKTeX::Core::Utils::MakeProgramVersionString(MiKTeX::Core::Utils::GetExeName(), MiKTeX::Core::VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0)) << "\n"
      << "Copyright (C) 1998-2018 Christian Schenk" << "\n"
      << "This is free software; see the source for copying conditions.  There is NO" << "\n"
      << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << "\n";
  }

protected:
  void Verbose(const char* format, ...)
  {
    if (printOnly)
    {
      return;
    }
    string s;
    va_list arglist;
    VA_START(arglist, format);
    s = StringUtil::FormatStringVA(format, arglist);
    VA_END(arglist);
    LOG4CXX_INFO(logger, s);
    if (verbose && !quiet)
    {
      cout << s << "\n";
    }
  }

protected:
  void Message(const char* format, ...)
  {
    if (printOnly)
    {
      return;
    }
    string s;
    va_list arglist;
    VA_START(arglist, format);
    s = StringUtil::FormatStringVA(format, arglist);
    VA_END(arglist);
    LOG4CXX_INFO(logger, s);
    if (!quiet)
    {
      cout << s << "\n";
    }
  }

protected:
  void PrintOnly(const char* format, ...)
  {
    if (!printOnly)
    {
      return;
    }
    va_list arglist;
    VA_START(arglist, format);
    OUT__ << StringUtil::FormatStringVA(format, arglist) << "\n";
    VA_END(arglist);
  }

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
  std::shared_ptr<MiKTeX::Core::Session> session;
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
void HandleOption(int ch, const char* optArg, bool& handled)            \
{                                                                       \
  UNUSED_ALWAYS(optArg);                                                \
  switch (ch)                                                           \
  {

#define OPTION_ENTRY_TRUE(ch, var)              \
    case ch:                                    \
      var = true;                               \
      break;

#define OPTION_ENTRY_SET(ch, var)               \
    case ch:                                    \
      var = optArg;                             \
      break;

#define OPTION_ENTRY_STRING(ch, var, size)              \
    case ch:                                            \
      StringUtil::CopyString(var, size, optArg);        \
      break;

#define OPTION_ENTRY(ch, action)                \
    case ch:                                    \
      action;                                   \
      break;

#define END_OPTION_MAP()                        \
    default:                                    \
      handled = false;                          \
      break;                                    \
    }                                           \
}
