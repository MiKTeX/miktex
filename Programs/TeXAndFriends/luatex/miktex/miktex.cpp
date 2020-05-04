/* luatex/miktex/miktex.cpp:

   Copyright (C) 2016-2020 Christian Schenk

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

#include <miktex/App/Application>
#include <miktex/Core/ConfigNames>
#include <miktex/Core/Directory>
#include <miktex/Core/FileType>
#include <miktex/Core/Paths>
#include <miktex/KPSE/Emulation>
#include <miktex/Util/PathNameUtil>

#include <string>

#include "luatex.h"

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

void miktex_enable_installer(int onOff)
{
  Application::GetApplication()->EnableInstaller(onOff ? TriState::True : TriState::False);
}

int miktex_open_format_file(const char* fileName_, FILE** ppFile, int renew)
{
  MIKTEX_ASSERT_STRING(fileName_);
  MIKTEX_ASSERT(ppFile != nullptr);

  shared_ptr<Session> session = Session::Get();

  PathName fileName(fileName_);

  if (!fileName.HasExtension())
  {
    fileName.SetExtension(".fmt");
  }

  PathName path;

  std::string dumpName = fileName.GetFileNameWithoutExtension().ToString();

  Session::FindFileOptionSet findFileOptions;

  findFileOptions += Session::FindFileOption::Create;

  if (renew)
  {
    findFileOptions += Session::FindFileOption::Renew;
  }

  if (!session->FindFile(fileName.ToString(), FileType::FMT, findFileOptions, path))
  {
    MIKTEX_FATAL_ERROR_2(T_("The memory dump file could not be found."), "fileName", fileName.ToString());
  }

#if 1
  if (!renew)
  {
    time_t modificationTime = File::GetLastWriteTime(path);
    time_t lastAdminMaintenance = session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_ADMIN_MAINTENANCE, ConfigValue("0")).GetTimeT();
    renew = lastAdminMaintenance > modificationTime;
    if (!renew && !session->IsAdminMode())
    {
      time_t lastUserMaintenance = session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_USER_MAINTENANCE, ConfigValue("0")).GetTimeT();
      renew = lastUserMaintenance > modificationTime;
    }
    if (renew)
    {
      // RECURSION
      return miktex_open_format_file(fileName_, ppFile, true);
    }
  }
#endif

  *ppFile = session->OpenFile(path, FileMode::Open, FileAccess::Read, false);

  session->PushAppName(dumpName);

  return true;
}

char* miktex_program_basename(const char* lpszArgv0)
{
  PathName argv0(lpszArgv0);
  std::string baseName = argv0.GetFileNameWithoutExtension().ToString();
  size_t prefixLen = strlen(MIKTEX_PREFIX);
  if (baseName.compare(0, prefixLen, MIKTEX_PREFIX) == 0)
  {
    baseName = baseName.substr(prefixLen);
  }
  return xstrdup(baseName.c_str());
}

int miktex_is_output_file(const char* path)
{
  PathName path_(path);
  if (path_.HasExtension(".gz"))
  {
    path_.SetExtension(nullptr);
  }
  return path_.HasExtension(".dvi")
    || path_.HasExtension(".pdf")
    || path_.HasExtension(".synctex");
}

PathName auxDirectory;

void miktex_set_aux_directory(const char* path)
{
  auxDirectory = path;
  auxDirectory.MakeAbsolute();
  shared_ptr<Session> session = Session::Get();
  if (!Directory::Exists(auxDirectory))
  {
    if (session->GetConfigValue(MIKTEX_CONFIG_SECTION_TEXANDFRIENDS, MIKTEX_CONFIG_VALUE_CREATEAUXDIRECTORY).GetString() == "t")
    {
      Directory::Create(auxDirectory);
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("The specified auxiliary directory does not exist."), "directory", auxDirectory.ToString());
    }
  }
  session->AddInputDirectory(auxDirectory, true);
}

const char* miktex_get_aux_directory()
{
  if (auxDirectory.Empty())
  {
    return nullptr;
  }
  else
  {
    return auxDirectory.GetData();
  }
}

void miktex_invoke_editor(const char* filename, int lineno)
{
  Application::GetApplication()->InvokeEditor(PathName(filename), lineno, FileType::TEX, PathName());
}

void miktex_show_library_versions()
{
  Application::GetApplication()->ShowLibraryVersions();
}

void miktex_add_include_directory(const char* path)
{
  if (Directory::Exists(PathName(path)))
  {
    PathName includeDirectory(path);
    includeDirectory.MakeAbsolute();
    shared_ptr<Session> session = Session::Get();
    session->AddInputDirectory(includeDirectory, true);
  }
}

int miktex_is_fully_qualified_path(const char* path)
{
  return PathNameUtil::IsFullyQualifiedPath(path);
}

void miktex_convert_to_unix(char* path)
{
  for (char* ch = path; *ch != 0; ++ch)
  {
    if (*ch == '\\')
    {
      *ch = '/';
    }
  }
}
