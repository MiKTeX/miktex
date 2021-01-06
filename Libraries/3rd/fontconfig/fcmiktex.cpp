/* fcmiktex.cpp:                                        -*- C++ -*-

   Copyright (C) 2007-2021 Christian Schenk

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

#include <config.h>

#include <cstdarg>
#include <cstdlib>

#if defined(MIKTEX_WINDOWS)
#include <io.h>
#endif

#include <exception>
#include <memory>
#include <set>

#include <miktex/Core/Exceptions>
#include <miktex/Util/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Session>
#include <miktex/Core/Utils>

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

extern "C" const char* miktex_fontconfig_path()
{
  try
  {
    shared_ptr<Session> session = Session::Get();
    static PathName path;
    if (path.Empty())
    {
      path = session->GetSpecialPath(SpecialPath::ConfigRoot);
      path /= MIKTEX_PATH_FONTCONFIG_CONFIG_DIR;
    }
    return path.GetData();
  }
  catch (const MiKTeXException& e)
  {
    Utils::PrintException(e);
    exit(1);
  }
  catch (const exception& e)
  {
    Utils::PrintException(e);
    exit(1);
  }
}

extern "C" const char* miktex_fc_cachedir()
{
  try
  {
    shared_ptr<Session> session = Session::Get();
    static PathName path;
    if (path.Empty())
    {
      path = session->GetSpecialPath(SpecialPath::DataRoot);
      path /= MIKTEX_PATH_FONTCONFIG_CACHE_DIR;
    }
    return path.GetData();
  }
  catch (const MiKTeXException& e)
  {
    Utils::PrintException(e);
    exit(1);
  }
  catch (const exception& e)
  {
    Utils::PrintException(e);
    exit(1);
  }
}

extern "C" const char* miktex_fc_default_fonts()
{
  try
  {
    static string fontPath;
    if (fontPath.empty())
    {
#if defined(MIKTEX_WINDOWS)
      PathName path;
      UINT l = GetWindowsDirectoryA(path.GetData(), static_cast<UINT>(path.GetCapacity()));
      if (l == 0 || l >= path.GetCapacity())
      {
        path = "C:/wInDoWs";
      }
      path /= "Fonts";
      fontPath = path.GetData();
#endif
#if defined(__APPLE__)
      // TODO
      // https://support.apple.com/en-us/HT201722
      // ~/Library/Fonts/
      // /Library/Fonts/
      // /Network/Library/Fonts/
      // /System/Library/Fonts/
      // /System Folder/Fonts/
#endif
    }
    return fontPath.c_str();
  }
  catch (const MiKTeXException& e)
  {
    Utils::PrintException(e);
    exit(1);
  }
  catch (const exception& e)
  {
    Utils::PrintException(e);
    exit(1);
  }
}

extern "C" const char* miktex_fontconfig_file()
{
  return MIKTEX_FONTS_CONF;
}

extern "C" int miktex_get_fontconfig_config_dirs(char** pPaths, int nPaths)
{
  try
  {
    shared_ptr<Session> session = Session::Get();
    unsigned nConfigDirs = session->GetNumberOfTEXMFRoots();
    if (pPaths != nullptr)
    {
      MIKTEX_ASSERT_BUFFER(pPaths, (nPaths + nConfigDirs) * sizeof(pPaths[0]));
      for (unsigned idx = 0; idx < nConfigDirs; ++idx, ++nPaths)
      {
        PathName path(session->GetRootDirectoryPath(idx));
        path /= MIKTEX_PATH_FONTCONFIG_CONFIG_DIR;
        pPaths[nPaths] = strdup(path.GetData());
      }
    }
    return static_cast<int>(nConfigDirs);
  }
  catch (const MiKTeXException& e)
  {
    Utils::PrintException(e);
    exit(1);
  }
  catch (const exception& e)
  {
    Utils::PrintException(e);
    exit(1);
  }
}

#if defined(MIKTEX_WINDOWS)
extern "C" void miktex_close_cache_file(int fd, const char* directory)
{
  try
  {
    time_t dirCreationTime, dirAccessTime, dirWriteTime;
    File::GetTimes(PathName(directory), dirCreationTime, dirAccessTime, dirWriteTime);
    time_t cache_mtime = time(nullptr);
    static set<time_t> modificationTimes;
    if (dirWriteTime != static_cast<time_t>(-1) && cache_mtime < dirWriteTime)
    {
      cache_mtime = dirWriteTime;
    }
    do
    {
      cache_mtime += 2;
    } while (modificationTimes.find(cache_mtime) != modificationTimes.end());
    File::SetTimes(fd, cache_mtime, cache_mtime, cache_mtime);
    modificationTimes.insert(cache_mtime);
    if (close(fd) < 0)
    {
      fprintf(stderr, "cannot close file: %s\n", strerror(errno));
    }
  }
  catch (const MiKTeXException& e)
  {
    Utils::PrintException(e);
    exit(1);
  }
  catch (const exception& e)
  {
    Utils::PrintException(e);
    exit(1);
  }
}
#endif

extern "C" void miktex_report_crt_error(const char* message, ...)
{
  va_list arglist;
  va_start(arglist, message);
  vfprintf(stderr, message, arglist);
  va_end(arglist);
  fprintf(stderr, ": %s\n", strerror(errno));
}

extern "C" void miktex_report_problem(const char* message, ...)
{
  va_list arglist;
  va_start(arglist, message);
  vfprintf(stderr, message, arglist);
  va_end(arglist);
  fputc('\n', stderr);
}

extern "C" int miktex_file_delete(const char* path)
{
  try
  {
    File::Delete(PathName(path), { FileDeleteOption::TryHard, FileDeleteOption::UpdateFndb });
    return 1;
  }
  catch (const MiKTeXException& e)
  {
    Utils::PrintException(e);
    return 0;
  }
  catch (const exception& e)
  {
    Utils::PrintException(e);
    return 0;
  }
}
