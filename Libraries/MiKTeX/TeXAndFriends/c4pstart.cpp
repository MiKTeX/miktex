/* c4pstart.cpp: C4P statup code

   Copyright (C) 1996-2020 Christian Schenk

   This file is part of the MiKTeX TeXMF Library.

   The MiKTeX TeXMF Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX TeXMF Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX TeXMF Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include <miktex/App/Application>

#if defined(MIKTEX_TEXMF_SHARED)
#  define C4PEXPORT MIKTEXDLLEXPORT
#else
#  define C4PEXPORT
#endif
#define C1F0C63F01D5114A90DDF8FC10FF410B
#include "miktex/C4P/C4P.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::App;
using namespace MiKTeX::Core;

using namespace C4P;

struct Runtime
{
  ~Runtime()
  {
    ClearCommandLine();
  }
  void ClearCommandLine()
  {
    for (char* lpsz : argumentVector)
    {
      free(lpsz);
    }
    argumentVector.clear();
    commandLine = "";
  }
  time_t startUpTime = static_cast<time_t>(-1);
  struct tm startUpTimeStructLocal;
  struct tm startUpTimeStructUtc;
  bool startUpTimeUseUtc;
  C4P_text standardTextFiles[3];
  vector<char*> argumentVector;
  string commandLine;
  string programName;
};

class C4P::ProgramBase::impl
{
public:
  bool isRunning = false;
public:
  Application* parent = nullptr;
#if defined(MIKTEX_WINDOWS)
public:
  bool utf8ConsoleIssue = false;
#endif
public:
  Runtime runtime;
};

C4P::ProgramBase::ProgramBase() :
  pimpl(make_unique<impl>())
{
}

C4P::ProgramBase::~ProgramBase() noexcept
{
  try
  {
    if (pimpl->isRunning)
    {
      this->Finish();
    }
  }
  catch (const std::exception&)
  {
  }
}

void C4P::ProgramBase::Initialize(const char* programName, int argc, char* argv[])
{
  MIKTEX_ASSERT_STRING(programName);
  pimpl->runtime.programName = programName;
  if (pimpl->runtime.startUpTime == static_cast<time_t>(-1))
  {
    string sde;
    string fsd;
    time_t now;
    bool useUtc;
    if (Utils::GetEnvironmentString("FORCE_SOURCE_DATE", fsd) && fsd == "1" && Utils::GetEnvironmentString("SOURCE_DATE_EPOCH", sde))
    {
      now = Utils::ToTimeT(sde);
      useUtc = true;
    }
    else
    {
      now = time(nullptr);
      useUtc = false;
    }
    SetStartUpTime(now, useUtc);
  }
  vector<string> args;
  for (int idx = 1; idx < argc; ++idx)
  {
    args.push_back(argv[idx]);
  }
  MakeCommandLine(args);
  pimpl->runtime.standardTextFiles[0].Attach(stdin, false);
  pimpl->runtime.standardTextFiles[1].Attach(stdout, false);
  pimpl->runtime.standardTextFiles[2].Attach(stderr, false);
  *input = '\n';
  *output = '\0';
  *c4perroroutput = '\0';
  pimpl->isRunning = true;
}

C4PTHISAPI(void) C4P::ProgramBase::Finish()
{
#if defined(MIKTEX_WINDOWS)
  if (pimpl->utf8ConsoleIssue)
  {
    pimpl->parent->Warning("some characters could not be written to the console window; run 'chcp 65001' to fix this");
    pimpl->utf8ConsoleIssue = false;
  }
#endif
  pimpl->runtime.ClearCommandLine();
  pimpl->runtime.programName = "";
}

C4PTHISAPI(void) C4P::ProgramBase::SetParent(Application* parent)
{
  pimpl->parent = parent;
}

void C4P::ProgramBase::SetStartUpTime(time_t time, bool useUtc)
{
  pimpl->runtime.startUpTime = time;
  pimpl->runtime.startUpTimeUseUtc = useUtc;
#if defined(_MSC_VER) && (_MSC_VER) >= 1400
  if (_localtime64_s(&pimpl->runtime.startUpTimeStructLocal, &pimpl->runtime.startUpTime) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("_localtime_s");
  }
  if (_gmtime64_s(&pimpl->runtime.startUpTimeStructUtc, &pimpl->runtime.startUpTime) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("_gmtime64_s");
  }
#else
  struct tm* localTm = localtime(&pimpl->runtime.startUpTime);
  if (localTm == nullptr)
  {
    MIKTEX_FATAL_CRT_ERROR("localtime");
  }
  pimpl->runtime.startUpTimeStructLocal = *localTm;
  struct tm* utcTm = gmtime(&pimpl->runtime.startUpTime);
  if (utcTm == nullptr)
  {
    MIKTEX_FATAL_CRT_ERROR("gmtime");
  }
  pimpl->runtime.startUpTimeStructUtc = *utcTm;
#endif
}

time_t C4P::ProgramBase::GetStartUpTime()
{
  return pimpl->runtime.startUpTime;
}

int C4P::ProgramBase::MakeCommandLine(const vector<string>& args)
{
  pimpl->runtime.ClearCommandLine();
  pimpl->runtime.argumentVector.push_back(strdup(Utils::GetExeName().c_str()));
  for (const string& arg : args)
  {
    pimpl->runtime.argumentVector.push_back(strdup(arg.c_str()));
    pimpl->runtime.commandLine += ' ';
    pimpl->runtime.commandLine += arg;
  }
  return 0;
}

const char* C4P::ProgramBase::GetCmdLine()
{
  return pimpl->runtime.commandLine.c_str();
}

int C4P::ProgramBase::GetArgC()
{
  return static_cast<int>(pimpl->runtime.argumentVector.size());
}

const char** C4P::ProgramBase::GetArgV()
{
  return const_cast<const char**>(&pimpl->runtime.argumentVector[0]);
}

unsigned C4P::ProgramBase::GetYear()
{
  const tm& tmData = pimpl->runtime.startUpTimeUseUtc ? pimpl->runtime.startUpTimeStructUtc : pimpl->runtime.startUpTimeStructLocal;
  return tmData.tm_year + 1900;
}

unsigned C4P::ProgramBase::GetMonth()
{
  const tm& tmData = pimpl->runtime.startUpTimeUseUtc ? pimpl->runtime.startUpTimeStructUtc : pimpl->runtime.startUpTimeStructLocal;
  return tmData.tm_mon + 1;
}

unsigned C4P::ProgramBase::GetDay()
{
  const tm& tmData = pimpl->runtime.startUpTimeUseUtc ? pimpl->runtime.startUpTimeStructUtc : pimpl->runtime.startUpTimeStructLocal;
  return tmData.tm_mday;
}

unsigned C4P::ProgramBase::GetHour()
{
  const tm& tmData = pimpl->runtime.startUpTimeUseUtc ? pimpl->runtime.startUpTimeStructUtc : pimpl->runtime.startUpTimeStructLocal;
  return tmData.tm_hour;
}

unsigned C4P::ProgramBase::GetMinute()
{
  const tm& tmData = pimpl->runtime.startUpTimeUseUtc ? pimpl->runtime.startUpTimeStructUtc : pimpl->runtime.startUpTimeStructLocal;
  return tmData.tm_min;
}

unsigned C4P::ProgramBase::GetSecond()
{
  const tm& tmData = pimpl->runtime.startUpTimeUseUtc ? pimpl->runtime.startUpTimeStructUtc : pimpl->runtime.startUpTimeStructLocal;
  return tmData.tm_sec;
}

C4P::C4P_text* C4P::ProgramBase::GetStdFilePtr(unsigned idx)
{
  MIKTEX_EXPECT(idx < sizeof(pimpl->runtime.standardTextFiles) / sizeof(pimpl->runtime.standardTextFiles[0]));
  return &pimpl->runtime.standardTextFiles[idx];
}

void C4P::ProgramBase::WriteChar(int ch, FILE* file)
{
#if defined(MIKTEX_WINDOWS)
  if (static_cast<unsigned char>(ch) > 127 )
  {
    int fd = fileno(file);
    if (fd < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("fileno");
    }
    int fdStdOut = (stdout != nullptr ? fileno(stdout) : -1);
    int fdStdErr = (stderr != nullptr ? fileno(stderr) : -1);
    if ((fd == fdStdOut || fd == fdStdErr) && isatty(fd) != 0 && GetConsoleOutputCP() != 65001)
    {
      pimpl->utf8ConsoleIssue = true;
      ch = '?';
    }
  }
#endif
  ch = putc(ch, file);
  if (ch == EOF)
  {
    MIKTEX_FATAL_CRT_ERROR("putc");
  }
}
