/* copystart.cpp: MiKTeX copy starter

   Copyright (C) 2001-2016 Christian Schenk

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
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <miktex/Core/Environment>
#include <miktex/Core/File>
#include <miktex/Core/MD5>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Session>
#include <miktex/Util/StringUtil>

#define UNUSED_ALWAYS(x) (x)

#define WU_(x) MiKTeX::Util::CharBuffer<char>(x).Get()
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).Get()

#define W(x) W_(x)
#define W_(x) L#x
#define LOC() W(__LINE__)
#define GET_INTERNAL_ERROR() L"Internal error (" LOC()  L")"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

void CopyStart(const wchar_t * lpszFileName, const wchar_t * lpszArgs)
{
  shared_ptr<Session> session = Session::Create(Session::InitInfo("copystart"));

  MD5 digest = MD5::FromFile(StringUtil::WideCharToUTF8(lpszFileName).c_str());

  // set MIKTEX_BINDIR
  Utils::SetEnvironmentString(MIKTEX_ENV_BIN_DIR, session->GetMyLocation().Get());

  // set MIKTEX_*STARTUPFILE
  if (session->IsMiKTeXPortable())
  {
    PathName startupConfig(session->GetSpecialPath(SpecialPath::InstallRoot));
    startupConfig /= MIKTEX_PATH_STARTUP_CONFIG_FILE;
    Utils::SetEnvironmentString(MIKTEX_ENV_COMMON_STARTUP_FILE, startupConfig.Get());
    Utils::SetEnvironmentString(MIKTEX_ENV_USER_STARTUP_FILE, startupConfig.Get());
  }

  PathName pathExe;

  bool alreadyCopied = false;

  // create the destination file name
  pathExe.SetToTempDirectory();
  pathExe /= PathName(lpszFileName).GetFileNameWithoutExtension();
  pathExe.SetExtension(".tmp");
  if (File::Exists(pathExe))
  {
    if (MD5::FromFile(pathExe.Get()) == digest)
    {
      alreadyCopied = true;
    }
    else
    {
      File::Delete(pathExe, { FileDeleteOption::TryHard });
    }
  }

  if (!alreadyCopied)
  {
    // copy the executable
    File::Copy(lpszFileName, pathExe);
    if (session->RunningAsAdministrator())
    {
      // schedule the removal of the destination file
      session->ScheduleFileRemoval(pathExe.Get());
    }
  }

  // start the executable
  Process::Start(pathExe.Get(), StringUtil::WideCharToUTF8(lpszArgs).c_str());
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t * lpCmdLine, int nCmdShow)
{
  UNUSED_ALWAYS(hInstance);
  UNUSED_ALWAYS(hPrevInstance);

  try
  {
    wchar_t szMyPath[MAX_PATH];

    if (GetModuleFileNameW(nullptr, szMyPath, MAX_PATH) == 0)
    {
      throw GET_INTERNAL_ERROR();
    }

    wchar_t szMyDrive[MAX_PATH];
    wchar_t szMyDir[MAX_PATH];
    wchar_t szMyName[MAX_PATH];
    wchar_t szMySuffix[MAX_PATH];

    if (_wsplitpath_s(szMyPath, szMyDrive, MAX_PATH, szMyDir, MAX_PATH, szMyName, MAX_PATH, szMySuffix, MAX_PATH) != 0)
    {
      throw GET_INTERNAL_ERROR();
    }

    wcscat_s(szMyName, MAX_PATH, szMySuffix);

    wstring exe;

    if (wcscmp(szMyName, UW_(MIKTEX_COPYSTART_EXE)) == 0 || wcscmp(szMyName, UW_(MIKTEX_COPYSTART_ADMIN_EXE)) == 0)
    {
      if (lpCmdLine == nullptr || *lpCmdLine == 0)
      {
        throw GET_INTERNAL_ERROR();
      }

      bool quoted = (*lpCmdLine == L'"');

      wchar_t endChar = (quoted ? L'"' : L' ');

      if (quoted)
      {
        ++lpCmdLine;
        if (*lpCmdLine == 0)
        {
          throw GET_INTERNAL_ERROR();
        }
      }

      for (; *lpCmdLine != endChar && *lpCmdLine != 0; ++lpCmdLine)
      {
        exe += *lpCmdLine;
      }

      if (quoted && *lpCmdLine == 0)
      {
        throw GET_INTERNAL_ERROR();
      }

      if (quoted)
      {
        ++lpCmdLine;
      }
    }
    else
    {
      exe = szMyDrive;
      exe += szMyDir;
      exe += L"internal\\";
      exe += szMyName;
    }

    CopyStart(exe.c_str(), lpCmdLine);

    return 0;
  }
  catch (const exception & e)
  {
    UNUSED_ALWAYS(e);
    MessageBoxW(nullptr, UW_(e.what()), L"MiKTeX", MB_OK | MB_ICONSTOP);
    return 1;
  }
  catch (const wchar_t * lpszMessage)
  {
    MessageBoxW(nullptr, lpszMessage, L"MiKTeX", MB_OK | MB_ICONSTOP);
    return 1;
  }
}
