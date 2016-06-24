/* c4pstart.cpp: C4P statup code

   Copyright (C) 1996-2016 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

struct ProgramData
{
  ~ProgramData()
  {
    ClearCommandLine();
  }
  void ClearCommandLine()
  {
    for (char * lpsz : argumentVector)
    {
      free(lpsz);
    }
    argumentVector.clear();
    commandLine = "";
  }
  time_t startUpTime = static_cast<time_t>(-1);
  struct tm startUpTimeStruct;
  C4P_text standardTextFiles[3];
  vector<char *> argumentVector;
  string commandLine;
  string programName;
};

namespace {
  ProgramData programData;
}

C4PCEEAPI(int) C4P::MakeCommandLine(const vector<string> & args)
{
  MIKTEX_API_BEGIN("C4P::MakeCommandLine");
  programData.ClearCommandLine();
  programData.argumentVector.push_back(strdup(Utils::GetExeName().c_str()));
  for (const string & arg : args)
  {
    programData.argumentVector.push_back(strdup(arg.c_str()));
    programData.commandLine += ' ';
    programData.commandLine += arg;
  }
  return 0;
  MIKTEX_API_END("C4P::MakeCommandLine");
}

C4PCEEAPI(void) C4P::SetStartUpTime(time_t time)
{
  MIKTEX_API_BEGIN("C4P::SetStartUpTime");
  programData.startUpTime = time;
#if defined(_MSC_VER) && (_MSC_VER) >= 1400
  if (_localtime64_s(&programData.startUpTimeStruct, &programData.startUpTime) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("_localtime_s");
  }
#else
  struct tm * p = localtime(&programData.startUpTime);
  if (p == nullptr)
  {
    MIKTEX_FATAL_CRT_ERROR("localtime");
  }
  programData.startUpTimeStruct = *p;
#endif
  MIKTEX_API_END("C4P::SetStartUpTime");
}

C4PCEEAPI(time_t) C4P::GetStartUpTime()
{
  MIKTEX_API_BEGIN("C4P::GetStartUpTime");
  return programData.startUpTime;
  MIKTEX_API_END("C4P::GetStartUpTime");
}

C4P::Program::Program(const char * lpszProgramName, int argc, const char ** argv)
{
  MIKTEX_API_BEGIN("C4P::StartUp");
  MIKTEX_ASSERT_STRING(lpszProgramName);
  programData.programName = lpszProgramName;
  if (programData.startUpTime == static_cast<time_t>(-1))
  {
    SetStartUpTime(time(nullptr));
  }
  vector<string> args;
  for (int idx = 1; idx < argc; ++idx)
  {
    args.push_back(argv[idx]);
  }
  MakeCommandLine(args);
  programData.standardTextFiles[0].Attach(stdin, false);
  programData.standardTextFiles[1].Attach(stdout, false);
  programData.standardTextFiles[2].Attach(stderr, false);
  *input = '\n';
  *output = '\0';
  *c4perroroutput = '\0';
  running = true;
  MIKTEX_API_END("C4P::StartUp");
}

C4PTHISAPI(void) C4P::Program::Finish()
{
  programData.ClearCommandLine();
  programData.programName = "";
}

C4PCEEAPI(unsigned) C4P::GetYear()
{
  MIKTEX_API_BEGIN("C4P::GetYear");
  return programData.startUpTimeStruct.tm_year + 1900;
  MIKTEX_API_END("C4P::GetYear");
}

C4PCEEAPI(unsigned) C4P::GetMonth()
{
  MIKTEX_API_BEGIN("C4P::GetMonth");
  return programData.startUpTimeStruct.tm_mon + 1;
  MIKTEX_API_END("C4P::GetMonth");
}

C4PCEEAPI(unsigned) C4P::GetDay()
{
  MIKTEX_API_BEGIN("C4P::GetDay");
  return programData.startUpTimeStruct.tm_mday;
  MIKTEX_API_END("C4P::GetDay");
}

C4PCEEAPI(unsigned) C4P::GetHour()
{
  MIKTEX_API_BEGIN("C4P::GetHour");
  return programData.startUpTimeStruct.tm_hour;
  MIKTEX_API_END("C4P::GetHour");
}

C4PCEEAPI(unsigned) C4P::GetMinute()
{
  MIKTEX_API_BEGIN("C4P::GetMinute");
  return programData.startUpTimeStruct.tm_min;
  MIKTEX_API_END("C4P::GetMinute");
}

C4PCEEAPI(unsigned) C4P::GetSecond()
{
  MIKTEX_API_BEGIN("C4P::GetSecond");
  return programData.startUpTimeStruct.tm_sec;
  MIKTEX_API_END("C4P::GetSecond");
}

C4PCEEAPI(C4P::C4P_text *) C4P::GetStdFilePtr(unsigned idx)
{
  MIKTEX_API_BEGIN("C4P::GetStdFilePtr");
  MIKTEX_ASSERT(idx < sizeof(programData.standardTextFiles) / sizeof(programData.standardTextFiles[0]));
  return &programData.standardTextFiles[idx];
  MIKTEX_API_END("C4P::GetStdFilePtr");
}

C4PCEEAPI(int) C4P::GetArgC()
{
  MIKTEX_API_BEGIN("C4P::GetArgC");
  return programData.argumentVector.size();
  MIKTEX_API_END("C4P::GetArgC");
}

C4PCEEAPI(const char **) C4P::GetArgV()
{
  MIKTEX_API_BEGIN("C4P::GetArgV");
  return const_cast<const char **>(&programData.argumentVector[0]);
  MIKTEX_API_END("C4P::GetArgV");
}

C4PCEEAPI(const char *) C4P::GetCmdLine()
{
  MIKTEX_API_BEGIN("C4P::GetCmdLine");
  return programData.commandLine.c_str();
  MIKTEX_API_END("C4P::GetCmdLine");
}

C4PCEEAPI(char *) C4P::GetProgName(char * lpszProgName)
{
  MIKTEX_API_BEGIN("C4P::GetProgName");
  StringUtil::CopyString(lpszProgName, BufferSizes::MaxPath, programData.programName.c_str());
  return lpszProgName;
  MIKTEX_API_END("C4P::GetProgName");
}
