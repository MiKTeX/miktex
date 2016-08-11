/* common.cpp: common stuff

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

#include "internal.h"

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("makex"));

class ProcessOutputTrash :
  public IRunProcessCallback
{
public:
  virtual bool MIKTEXTHISCALL OnProcessOutput(const void * pOutput, size_t n)
  {
    UNUSED_ALWAYS(pOutput);
    UNUSED_ALWAYS(n);
    return true;
  }
};

class ProcessOutputStderr :
  public IRunProcessCallback
{
public:
  virtual bool MIKTEXTHISCALL OnProcessOutput(const void * pOutput, size_t n)
  {
    cerr.write((const char *)pOutput, n);
    return true;
  }
};

bool MakeUtility::RunMETAFONT(const char * lpszName, const char * lpszMode, const char * lpszMag, const PathName & workingDirectory)
{
  string arguments;
  arguments += "--undump=mf";
  switch (GetEnableInstaller())
  {
  case TriState::False:
    arguments += " --disable-installer";
    break;
  case TriState::True:
    arguments += " --enable-installer";
    break;
  default:
    break;
  }
  arguments += " \"\\mode:=";
  arguments += (lpszMode == nullptr ? "ljfour" : lpszMode);
  arguments += ";\"";
  if (lpszMag != nullptr)
  {
    arguments += " \"\\mag:=";
    arguments += lpszMag;
    arguments += ";\"";
  }
  if (!debug)
  {
    arguments += " nonstopmode;";
  }
  arguments += " \"input ";
  arguments += lpszName;
  arguments += '"';
  if (RunProcess(MIKTEX_MF_EXE, arguments.c_str(), workingDirectory))
  {
    return true;
  }
  Verbose(T_("METAFONT failed for some reason"));
  PathName pathLogFile(nullptr, lpszName, ".log");
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

bool MakeUtility::RunProcess(const char * lpszExeName, const string & arguments, const PathName & workingDirectory)
{
  // find the executable; make sure it contains no blanks
  PathName exe;
  if (!session->FindFile(lpszExeName, FileType::EXE, exe))
  {
    FatalError(T_("The application file %s could not be found."), Q_(lpszExeName));
  }

  Message(T_("Running %s..."), Q_(lpszExeName));
  PrintOnly("%s %s", Q_(lpszExeName), arguments.c_str());

  // run the program
  int exitCode = 0;
  if (!printOnly || arguments.find("--print-only") != string::npos)
  {
    ProcessOutputTrash trash;
    ProcessOutputStderr toStderr;
    IRunProcessCallback * callback = nullptr;
    if (quiet)
    {
      callback = &trash;
    }
    else if (stdoutStderr)
    {
      callback = &toStderr;
    }
    if (!Process::Run(exe, arguments, callback, &exitCode, workingDirectory.GetData()))
    {
      FatalError(T_("The application file %s could not be started."), Q_(lpszExeName));
    }
  }

  return exitCode == 0;
}

void MakeUtility::Init(const Session::InitInfo & initInfoArg)
{
  Session::InitInfo initInfo = initInfoArg;
  if (adminMode)
  {
    initInfo.SetOptions({ Session::InitOption::AdminMode });
  }
  Application::Init(initInfo);
}

void MakeUtility::ShowVersion()
{
  OUT__
    << Utils::MakeProgramVersionString(Utils::GetExeName().c_str(), VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0)) << endl
    << "Copyright (C) 1998-2016 Christian Schenk" << endl
    << "This is free software; see the source for copying conditions.  There is NO" << endl
    << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
}

void MakeUtility::Verbose(const char * lpszFormat, ...)
{
  if (printOnly)
  {
    return;
  }
  string s;
  va_list arglist;
  VA_START(arglist, lpszFormat);
  s = StringUtil::FormatString(lpszFormat, arglist);
  VA_END(arglist);
  LOG4CXX_INFO(logger, s);
  if (verbose && !quiet)
  {
    cout << s << endl;
  }
}

void MakeUtility::Message(const char * lpszFormat, ...)
{
  if (printOnly)
  {
    return;
  }
  string s;
  va_list arglist;
  VA_START(arglist, lpszFormat);
  s = StringUtil::FormatString(lpszFormat, arglist);
  VA_END(arglist);
  LOG4CXX_INFO(logger, s);
  if (!quiet)
  {
    cout << s << endl;
  }
}

void MakeUtility::PrintOnly(const char * lpszFormat, ...)
{
  if (!printOnly)
  {
    return;
  }
  va_list arglist;
  VA_START(arglist, lpszFormat);
  OUT__ << StringUtil::FormatString(lpszFormat, arglist) << endl;
  VA_END(arglist);
}

void MakeUtility::Install(const PathName & source, const PathName & dest)
{
  PrintOnly("cp %s %s", Q_(source), Q_(dest));
  PrintOnly("initexmf --update-fndb");
  if (!printOnly)
  {
    File::Copy(source, dest, { FileCopyOption::ReplaceExisting, FileCopyOption::UpdateFndb });
  }
}

void GetShortOptions(const struct option * pLongOptions, string & shortOptions)
{
  shortOptions = "";
  for (const struct option * opt = pLongOptions; opt->name != nullptr; ++opt)
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

void MakeUtility::GetOptions(int argc, const char ** argv, const struct option * pLongOptions, int & optionIndex)
{
  string shortOptions;
  GetShortOptions(pLongOptions, shortOptions);

  int c;
  int idx;
  optind = 0;
  while ((c = getopt_long(argc, const_cast<char * const *>(argv), shortOptions.c_str(), pLongOptions, &idx)) != EOF)
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

PathName MakeUtility::CreateDirectoryFromTemplate(const string & templ)
{
  PathName path;
  const char * lpszTemplate = templ.c_str();
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
