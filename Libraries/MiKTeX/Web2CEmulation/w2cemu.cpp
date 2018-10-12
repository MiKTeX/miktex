/* w2cemu.cpp: web2c compatibility functions

   Copyright (C) 2010-2018 Christian Schenk

   This file is part of the MiKTeX W2CEMU Library.

   The MiKTeX W2CEMU Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX W2CEMU Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX W2CEMU Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include <cstdlib>

#if defined(MIKTEX_UNIX)
#include <sys/time.h>
#endif

#include <iostream>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/ConfigNames>
#include <miktex/Core/Directory>
#include <miktex/Core/Registry>
#include <miktex/TeXAndFriends/TeXApp>
#include <miktex/TeXAndFriends/WebAppInputLine>
#include <miktex/Util/StringUtil>
#include <miktex/Version>

#include <string>

#include "internal.h"

using namespace MiKTeX;
using namespace MiKTeX::Core;
using namespace MiKTeX::TeXAndFriends;
using namespace MiKTeX::Util;
using namespace std;

namespace {
  PathName outputDirectory;
}

static void TranslateModeString(const char* modeString, FileMode& mode, FileAccess& access, bool& isTextFile)
{
  if (Utils::Equals(modeString, "r"))
  {
    mode = FileMode::Open;
    access = FileAccess::Read;
    isTextFile = true;
  }
  else if (Utils::Equals(modeString, "w"))
  {
    mode = FileMode::Create;
    access = FileAccess::Write;
    isTextFile = true;
  }
  else if (Utils::Equals(modeString, "rb"))
  {
    mode = FileMode::Open;
    access = FileAccess::Read;
    isTextFile = false;
  }
  else if (Utils::Equals(modeString, "wb"))
  {
    mode = FileMode::Create;
    access = FileAccess::Write;
    isTextFile = false;
  }
  else if (Utils::Equals(modeString, "ab"))
  {
    mode = FileMode::Append;
    access = FileAccess::Write;
    isTextFile = false;
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }
}

static FILE* TryFOpen(const char* path, const char* modeString)
{
  shared_ptr<Session> session = Session::Get();
  FileMode mode(FileMode::Open);
  FileAccess access(FileAccess::Read);
  bool isTextFile;
  TranslateModeString(modeString, mode, access, isTextFile);
  return session->TryOpenFile(path, mode, access, isTextFile);
}

int Web2C::OpenInput(FILE** ppfile, kpse_file_format_type format, const char* modeString)
{
  PathName fileName(WebAppInputLine::GetWebAppInputLine()->GetNameOfFile());
  char* path = miktex_kpathsea_find_file(kpse_def, fileName.GetData(), format, 0);
  if (path == nullptr)
  {
    return 0;
  }
  try
  {
    *ppfile = TryFOpen(path, modeString);
  }
  catch (const exception&)
  {
    MIKTEX_FREE(path);
    throw;
  }
  if (*ppfile != nullptr)
  {
    WebAppInputLine::GetWebAppInputLine()->SetNameOfFile(path);
  }
  MIKTEX_FREE(path);
  return *ppfile == nullptr ? 0 : 1;
}

void Web2C::RecordFileName(const char* path, FileAccess access)
{
  shared_ptr<Session> session = Session::Get();
  if (miktex_web2c_recorder_enabled)
  {
    session->StartFileInfoRecorder();
  }
  session->RecordFileInfo(path, access);
}

void miktex_web2c_record_file_name(const char* path, int reading)
{
  Web2C::RecordFileName(path, reading ? FileAccess::Read : FileAccess::Write);
}

void Web2C::ChangeRecorderFileName(const char* fileName)
{
  shared_ptr<Session> session = Session::Get();
  PathName path(GetOutputDirectory(), fileName);
  path.AppendExtension(".fls");
  session->SetRecorderPath(path);
}

void miktex_web2c_change_recorder_file_name(const char* path)
{
  Web2C::ChangeRecorderFileName(path);
}

void Web2C::SetOutputDirectory(const PathName& path)
{
  shared_ptr<Session> session = Session::Get();
  outputDirectory = path;
  outputDirectory.MakeAbsolute();
  if (!Directory::Exists(outputDirectory))
  {
    if (session->GetConfigValue(MIKTEX_CONFIG_SECTION_TEXANDFRIENDS, MIKTEX_CONFIG_VALUE_CREATEOUTPUTDIRECTORY).GetString() == "t")
    {
      Directory::Create(outputDirectory);
    }
    else
    {
      MIKTEX_FATAL_CRT_ERROR_2(T_("The specified directory does not exist."), "directory", outputDirectory.ToString());
    }
  }
#if 0 // TODO
  if (auxDirectory[0] == 0)
  {
    auxDirectory = outputDirectory;
  }
#endif
  session->AddInputDirectory(outputDirectory, true);
}

void miktex_web2c_set_output_directory(const char* path)
{
  Web2C::SetOutputDirectory(path);
}

PathName Web2C::GetOutputDirectory()
{
  return outputDirectory;
}

const char* miktex_web2c_get_output_directory()
{
  return outputDirectory.Empty() ? nullptr : outputDirectory.GetData();
}

void Web2C::GetSecondsAndMicros(int* seconds, int* micros)
{
#if defined(MIKTEX_WINDOWS)
  SYSTEMTIME systemTime;
  GetSystemTime(&systemTime);
  FILETIME fileTime;
  if (!SystemTimeToFileTime(&systemTime, &fileTime))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("SystemTimeToFileTime");
  }
  ULARGE_INTEGER ularge;
  ularge.LowPart = fileTime.dwLowDateTime;
  ularge.HighPart = fileTime.dwHighDateTime;
  ULONGLONG epoch = 116444736000000000;
  *seconds = (ularge.QuadPart - epoch) / 10000000;
  *micros = systemTime.wMilliseconds * 1000;
#else
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  *seconds = tv.tv_sec;
  *micros = tv.tv_usec;
#endif
}

/* Implementation found in texk/web2c/lib/zround.c
   zround.c: round R to the nearest whole number.  This is supposed to
   implement the predefined Pascal round function.  Public domain. */
integer miktex_zround(double r)
{
  integer i;

  /* R can be outside the range of an integer if glue is stretching or
     shrinking a lot.  We can't do any better than returning the largest
     or smallest integer possible in that case.  It doesn't seem to make
     any practical difference.  Here is a sample input file which
     demonstrates the problem, from phil@cs.arizona.edu:
        \documentstyle{article}
        \begin{document}
        \begin{flushleft}
        $\hbox{} $\hfill
        \filbreak
        \eject

     djb@silverton.berkeley.edu points out we should testing against
     TeX's largest or smallest integer (32 bits), not the machine's.  So
     we might as well use a floating-point constant, and avoid potential
     compiler bugs (also noted by djb, on BSDI).  */
  if (r > 2147483647.0)
    i = 2147483647;
  /* should be ...8, but atof bugs are too common */
  else if (r < -2147483647.0)
    i = -2147483647;
  /* Admittedly some compilers don't follow the ANSI rules of casting
     meaning truncating toward zero; but it doesn't matter enough to do
     anything more complicated here.  */
  else if (r >= 0.0)
    i = (integer)(r + 0.5);
  else
    i = (integer)(r - 0.5);

  return i;
}

void miktex_uexit(int status)
{
  int final_code;
  if (status == 0)
  {
    final_code = EXIT_SUCCESS;
  }
  else if (status == 1)
  {
    final_code = EXIT_FAILURE;
  }
  else
  {
    final_code = status;
  }
  throw final_code;
}

void miktex_setupboundvariable(integer* var, const char* varName, integer dflt)
{
  shared_ptr<Session> session = Session::Get();
  int ret = session->GetConfigValue("", varName, dflt).GetInt();
  if (ret >= 0)
  {
    *var = ret;
  }
}

void miktex_usagehelp(const char** lines, const char* bugEmail)
{
  for (; *lines != nullptr; ++lines)
  {
    cout << *lines << endl;
  }
  if (bugEmail == nullptr)
  {
    cout << endl << "Visit miktex.org for bug reports." << endl;
  }
  else
  {
    cout << endl << StringUtil::FormatString("Email bug reports to %s.", bugEmail) << endl;
  }
  throw 0;
}

char* Web2C::GetCurrentFileName()
{
  return xstrdup(WebAppInputLine::GetWebAppInputLine()->GetFoundFileFq().GetData());
}

int Web2C::RunSystemCommand(const char* cmd)
{
  int exitCode;
  return (int)TeXApp::GetTeXApp()->Write18(cmd, exitCode);
}

const char* miktex_web2c_version_string = WEB2CVERSION;
boolean miktex_web2c_recorder_enabled = 0;
char* miktex_web2c_fullnameoffile = nullptr;
