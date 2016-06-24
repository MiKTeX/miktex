/* w2cemu.cpp: web2c compatibility functions

   Copyright (C) 2010-2016 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#include <miktex/texmfapp.defaults.h>

using namespace std;
using namespace MiKTeX;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

namespace {
  PathName outputDirectory;
}

MIKTEXSTATICFUNC(void) TranslateModeString(const char * lpszMode, FileMode & mode, FileAccess & access, bool & isTextFile)
{
  if (Utils::Equals(lpszMode, "r"))
  {
    mode = FileMode::Open;
    access = FileAccess::Read;
    isTextFile = true;
  }
  else if (Utils::Equals(lpszMode, "w"))
  {
    mode = FileMode::Create;
    access = FileAccess::Write;
    isTextFile = true;
  }
  else if (Utils::Equals(lpszMode, "rb"))
  {
    mode = FileMode::Open;
    access = FileAccess::Read;
    isTextFile = false;
  }
  else if (Utils::Equals(lpszMode, "wb"))
  {
    mode = FileMode::Create;
    access = FileAccess::Write;
    isTextFile = false;
  }
  else if (Utils::Equals(lpszMode, "ab"))
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

MIKTEXSTATICFUNC(FILE *) TryFOpen(const char * lpszFileName, const char * lpszMode)
{
  shared_ptr<Session> session = Session::Get();
  FileMode mode(FileMode::Open);
  FileAccess access(FileAccess::Read);
  bool isTextFile;
  TranslateModeString(lpszMode, mode, access, isTextFile);
  return session->TryOpenFile(lpszFileName, mode, access, isTextFile);
}

MIKTEXW2CCEEAPI(int) Web2C::OpenInput(char * lpszFileName, FILE ** ppfile, kpse_file_format_type format, const char * lpszMode)
{
  char * lpszPath = miktex_kpathsea_find_file(kpse_def, lpszFileName, format, 0);
  if (lpszPath == nullptr)
  {
    return 0;
  }
  try
  {
    *ppfile = TryFOpen(lpszPath, lpszMode);
  }
  catch (const exception &)
  {
    MIKTEX_FREE(lpszPath);
    throw;
  }
  if (*ppfile != nullptr)
  {
    StringUtil::CopyString(lpszFileName, BufferSizes::MaxPath, lpszPath);
  }
  MIKTEX_FREE(lpszPath);
  return *ppfile == nullptr ? 0 : 1;
}

MIKTEXW2CCEEAPI(void) Web2C::RecordFileName(const char * lpszPath, FileAccess access)
{
  shared_ptr<Session> session = Session::Get();
  if (miktex_web2c_recorder_enabled)
  {
    session->StartFileInfoRecorder();
  }
  session->RecordFileInfo(lpszPath, access);
}

MIKTEXW2CCEEAPI(void) miktex_web2c_record_file_name(const char * lpszPath, int reading)
{
  Web2C::RecordFileName(lpszPath, reading ? FileAccess::Read : FileAccess::Write);
}

MIKTEXW2CCEEAPI(void) Web2C::ChangeRecorderFileName(const char * lpszName)
{
  shared_ptr<Session> session = Session::Get();
  PathName path(GetOutputDirectory(), lpszName);
  path.SetExtension(".fls");
  session->SetRecorderPath(path);
}

MIKTEXW2CCEEAPI(void) miktex_web2c_change_recorder_file_name(const char * lpszPath)
{
  Web2C::ChangeRecorderFileName(lpszPath);
}

MIKTEXW2CDATA(const char *) miktex_web2c_version_string = WEB2CVERSION;

MIKTEXW2CDATA(boolean) miktex_web2c_recorder_enabled = 0;

MIKTEXW2CCEEAPI(void) Web2C::SetOutputDirectory(const PathName & path)
{
  shared_ptr<Session> session = Session::Get();
  outputDirectory = path;
  outputDirectory.MakeAbsolute();
  if (!Directory::Exists(outputDirectory))
  {
    if (session->GetConfigValue(nullptr, MIKTEX_REGVAL_CREATE_OUTPUT_DIRECTORY, texmfapp::CreateOutputDirectory().c_str()) == "t")
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
  session->AddInputDirectory(outputDirectory.Get(), true);
}

MIKTEXW2CCEEAPI(void) miktex_web2c_set_output_directory(const char * lpszPath)
{
  Web2C::SetOutputDirectory(lpszPath);
}

MIKTEXW2CCEEAPI(PathName) Web2C::GetOutputDirectory()
{
  return outputDirectory;
}

MIKTEXW2CCEEAPI(const char *) miktex_web2c_get_output_directory()
{
  return outputDirectory.Empty() ? nullptr : outputDirectory.Get();
}

MIKTEXW2CDATA(char *) miktex_web2c_fullnameoffile = nullptr;

MIKTEXW2CCEEAPI(void) Web2C::GetSecondsAndMicros(integer * pSeconds, integer * pMicros)
{
#if defined(MIKTEX_WINDOWS)
  unsigned long clock = GetTickCount();
  *pSeconds = clock / 1000;
  *pMicros = clock % 1000;
#else
  struct timeval tv;
  gettimeofday(&tv, 0);
  *pSeconds = tv.tv_sec;
  *pMicros = tv.tv_usec;
#endif
}

/* Implementation found in texk/web2c/lib/zround.c
   zround.c: round R to the nearest whole number.  This is supposed to
   implement the predefined Pascal round function.  Public domain. */
MIKTEXW2CCEEAPI(integer) miktex_zround(double r)
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

MIKTEXW2CEXPORT MIKTEXNORETURN void MIKTEXCEECALL miktex_uexit(int status)
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

MIKTEXW2CCEEAPI(void) miktex_setupboundvariable(integer * pVar, const char * lpszVarName, integer dflt)
{
  shared_ptr<Session> session = Session::Get();
  int ret = session->GetConfigValue(nullptr, lpszVarName, dflt);
  if (ret >= 0)
  {
    *pVar = ret;
  }
}

MIKTEXW2CEXPORT MIKTEXNORETURN void MIKTEXCEECALL miktex_usagehelp(const char ** lpszLines, const char * lpszBugEmail)
{
  for (; *lpszLines != nullptr; ++lpszLines)
  {
    cout << *lpszLines << endl;
  }
  if (lpszBugEmail == nullptr)
  {
    cout << endl << "Visit miktex.org for bug reports." << endl;
  }
  else
  {
    cout << endl << StringUtil::FormatString("Email bug reports to %s.", lpszBugEmail) << endl;
  }
  throw 0;
}
