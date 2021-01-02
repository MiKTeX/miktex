/* MakeUtility.h:                                       -*- C++ -*-

   Copyright (C) 1998-2021 Christian Schenk

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

#include <iostream>
#include <string>
#include <vector>

#include <getopt.h>

#include <log4cxx/logger.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/App/Application>

#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Directory>
#include <miktex/Core/File>
#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/Session>

#include <miktex/Util/StringUtil>

#define OUT__ (stdoutStderr ? std::cerr : std::cout)

#define T_(x) MIKTEXTEXT(x)
#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#if !defined(UNUSED_ALWAYS)
#  define UNUSED_ALWAYS(x) static_cast<void>(x)
#endif

inline bool HasPrefix(const std::string& s1, const std::string& s2)
{
  return MiKTeX::Core::PathName::Compare(MiKTeX::Core::PathName(s1), MiKTeX::Core::PathName(s2), s2.length()) == 0;
}

class ProcessOutputTrash :
  public MiKTeX::Core::IRunProcessCallback
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
  public MiKTeX::Core::IRunProcessCallback
{
public:
  bool MIKTEXTHISCALL OnProcessOutput(const void* output, size_t n) override
  {
    std::cerr.write((const char*)output, n);
    return true;
  }
};

extern log4cxx::LoggerPtr logger;

class MakeUtility :
  public MiKTeX::App::Application
{
public:
  void Init(const MiKTeX::Core::Session::InitInfo& initInfo, std::vector<char*>& args) override
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
  void GetShortOptions(const struct option* longOptions, std::string& shortOptions)
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
    std::string shortOptions;
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
        this->EnableInstaller(MiKTeX::Configuration::TriState::False);
        break;
      case 'E':
        this->EnableInstaller(MiKTeX::Configuration::TriState::True);
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
  bool RunProcess(const char* exeName, const std::vector<std::string>& arguments, const MiKTeX::Core::PathName& workingDirectory)
  {
    // find the executable; make sure it contains no blanks
    MiKTeX::Core::PathName exe;
    if (!session->FindFile(exeName, MiKTeX::Core::FileType::EXE, exe))
    {
      FatalError(fmt::format(T_("The application file {0} could not be found."), Q_(exeName)));
    }

    std::vector<std::string> allArgs{ exeName };

    if (session->IsAdminMode())
    {
      allArgs.push_back("--miktex-admin");
    }

    switch (GetEnableInstaller())
    {
    case MiKTeX::Configuration::TriState::False:
      allArgs.push_back("--miktex-disable-installer");
      break;
    case MiKTeX::Configuration::TriState::True:
      allArgs.push_back("--miktex-enable-installer");
      break;
    default:
      break;
    }

    allArgs.push_back("--miktex-disable-maintenance");
    allArgs.push_back("--miktex-disable-diagnose");

    allArgs.insert(allArgs.end(), arguments.begin(), arguments.end());

    Message(fmt::format(T_("Running {0}..."), Q_(exeName)));
    LOG4CXX_INFO(logger, "running: " << MiKTeX::Core::CommandLineBuilder(allArgs).ToString());
    PrintOnly(MiKTeX::Core::CommandLineBuilder(allArgs).ToString());
    
    // run the program
    int exitCode = 0;
    if (!(printOnly || find(allArgs.begin(), allArgs.end(), "--print-only") != allArgs.end()))
    {
      ProcessOutputTrash trash;
      ProcessOutputStderr toStderr;
      MiKTeX::Core::IRunProcessCallback* callback = nullptr;
      if (quiet)
      {
        callback = &trash;
      }
      else if (stdoutStderr)
        {
          callback = &toStderr;
        }
      if (!MiKTeX::Core::Process::Run(exe, allArgs, callback, &exitCode, workingDirectory.GetData()))
      {
        FatalError(fmt::format(T_("The application file {0} could not be started."), Q_(exeName)));
      }
    }

    return exitCode == 0;
  }

protected:
  bool RunMETAFONT(const char* name, const char* mode, const char* mag, const MiKTeX::Core::PathName& workingDirectory)
  {
    std::vector<std::string> arguments;
    arguments.push_back(fmt::format("--undump={}", "mf"));
    arguments.push_back(fmt::format("\\mode:={};", mode == nullptr ? "ljfour" : mode));
    if (mag != nullptr)
    {
      arguments.push_back(fmt::format("\\mag:={};", mag));
    }
    if (!debug)
    {
      arguments.push_back("nonstopmode;");
    }
    arguments.push_back(fmt::format("input {}", name));
    if (RunProcess(MIKTEX_MF_EXE, arguments, workingDirectory))
    {
      return true;
    }
    Verbose(T_("METAFONT failed for some reason"));
    MiKTeX::Core::PathName pathLogFile = workingDirectory / MiKTeX::Core::PathName(name);
    pathLogFile.AppendExtension(".log");
    bool noError = true;
    size_t nStrangePaths = 0;
    std::ifstream stream = MiKTeX::Core::File::CreateInputStream(pathLogFile);
    std::string line;
    while (noError && std::getline(stream, line))
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
    stream.close();
    if (noError)
    {
      Verbose(fmt::format(T_("ignoring {0} strange path(s)"), nStrangePaths));
    }
    return noError;
  }
  
protected:
  void Install(const MiKTeX::Core::PathName& source, const MiKTeX::Core::PathName& dest)
  {
    PrintOnly(fmt::format("cp {} {}", Q_(source), Q_(dest)));
    PrintOnly("initexmf --update-fndb");
    if (!printOnly)
    {
      Verbose(fmt::format(T_("Installing {0}..."), Q_(dest)));
      MiKTeX::Core::File::Copy(source, dest, { MiKTeX::Core::FileCopyOption::ReplaceExisting, MiKTeX::Core::FileCopyOption::UpdateFndb });
    }
  }

protected:
  MiKTeX::Core::PathName CreateDirectoryFromTemplate(const std::string& templ)
  {
    MiKTeX::Core::PathName path;
    const char* lpszTemplate = templ.c_str();
    if (lpszTemplate[0] == '%'
        && lpszTemplate[1] == 'R'
        && MiKTeX::Util::PathNameUtil::IsDirectoryDelimiter(lpszTemplate[2]))
    {
      path = session->GetSpecialPath(MiKTeX::Core::SpecialPath::DataRoot);
      path /= lpszTemplate + 3;
    }
    else
    {
      path = lpszTemplate;
    }

    if (!MiKTeX::Core::Directory::Exists(path))
    {
      PrintOnly(MiKTeX::Core::CommandLineBuilder("mkdir", path.ToString()).ToString());
      if (!printOnly)
      {
        MiKTeX::Core::Directory::Create(path);
      }
    }
    return path;
  }

protected:
  void ShowVersion()
  {
    OUT__
      << MiKTeX::Core::Utils::MakeProgramVersionString(MiKTeX::Core::Utils::GetExeName(), MiKTeX::Core::VersionNumber(MIKTEX_COMPONENT_VERSION_STR)) << "\n"
      << "\n"
      << MIKTEX_COMP_COPYRIGHT_STR << "\n"
      << "\n"
      << "This is free software; see the source for copying conditions.  There is NO" << "\n"
      << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << "\n";
  }

protected:
  void Verbose(const std::string& s)
  {
    if (printOnly)
    {
      return;
    }
    LOG4CXX_INFO(logger, s);
    if (verbose && !quiet)
    {
      std::cout << s << "\n";
    }
  }

protected:
  void Message(const std::string& s)
  {
    if (printOnly)
    {
      return;
    }
    LOG4CXX_INFO(logger, s);
    if (!quiet)
    {
      std::cout << s << "\n";
    }
  }

protected:
  void PrintOnly(const std::string& s)
  {
    if (!printOnly)
    {
      return;
    }
    OUT__ << s << "\n";
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
  MiKTeX::Core::PathName destinationDirectory;

protected:
  std::string name;

protected:
  std::shared_ptr<MiKTeX::Core::Session> session;
};

#define COMMON_OPTIONS                                          \
  {"admin",        no_argument,            0,      'A'},        \
  {"debug",        no_argument,            0,      'd'},        \
  {"disable-installer", no_argument,       0,      'D'},        \
  {"enable-installer", no_argument,        0,      'E'},        \
  {"help",         no_argument,            0,      'h'},        \
  {"print-only",   no_argument,            0,      'n'},        \
  {"quiet",        no_argument,            0,      'q'},        \
  {"verbose",      no_argument,            0,      'v'},        \
  {"version",      no_argument,            0,      'V'}

#define BEGIN_OPTION_MAP(cls)                                           \
void HandleOption(int ch, const char* optArg, bool& handled) override   \
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

#define OPTION_ENTRY_STRING(ch, var, size)                     \
    case ch:                                                   \
      MiKTeX::Util::StringUtil::CopyString(var, size, optArg); \
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
